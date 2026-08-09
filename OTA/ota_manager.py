#!/usr/bin/env python3
"""Desktop OTA package manager and HTTP server controller."""

from __future__ import annotations

import hashlib
import json
import queue
import re
import shutil
import socket
import threading
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

from server import FIRMWARE_ROOT, VALID_TARGETS, create_server


ROOT = Path(__file__).resolve().parent
VERSION_PATTERN = re.compile(r"^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def lan_ipv4() -> str:
    probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        probe.connect(("8.8.8.8", 80))
        return probe.getsockname()[0]
    except OSError:
        try:
            return socket.gethostbyname(socket.gethostname())
        except OSError:
            return "127.0.0.1"
    finally:
        probe.close()


class OtaManagerApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("HVAC Replica OTA Manager")
        self.geometry("1080x720")
        self.minsize(920, 620)
        self.protocol("WM_DELETE_WINDOW", self.on_close)

        self.http_server = None
        self.server_thread: threading.Thread | None = None
        self.log_queue: queue.Queue[str] = queue.Queue()

        self.target_var = tk.StringVar(value="HU7")
        self.version_var = tk.StringVar(value="0.5.0")
        self.file_var = tk.StringVar()
        self.file_size_var = tk.StringVar(value="-")
        self.sha_var = tk.StringVar(value="-")
        self.host_var = tk.StringVar(value="0.0.0.0")
        self.port_var = tk.StringVar(value="8080")
        self.server_url_var = tk.StringVar(value=f"http://{lan_ipv4()}:8080")
        self.server_state_var = tk.StringVar(value="Stopped")

        self.configure_style()
        self.build_ui()
        self.refresh_packages()
        self.after(100, self.drain_logs)

    def configure_style(self) -> None:
        style = ttk.Style(self)
        if "vista" in style.theme_names():
            style.theme_use("vista")
        style.configure("Title.TLabel", font=("Segoe UI", 18, "bold"))
        style.configure("Section.TLabelframe.Label", font=("Segoe UI", 11, "bold"))
        style.configure("State.TLabel", font=("Segoe UI", 10, "bold"))
        style.configure("Primary.TButton", font=("Segoe UI", 10, "bold"))

    def build_ui(self) -> None:
        outer = ttk.Frame(self, padding=16)
        outer.pack(fill=tk.BOTH, expand=True)
        outer.columnconfigure(0, weight=1)
        outer.rowconfigure(3, weight=1)

        title_row = ttk.Frame(outer)
        title_row.grid(row=0, column=0, sticky="ew", pady=(0, 12))
        title_row.columnconfigure(0, weight=1)
        ttk.Label(title_row, text="OTA Manager", style="Title.TLabel").grid(row=0, column=0, sticky="w")
        ttk.Label(title_row, textvariable=self.server_state_var, style="State.TLabel").grid(row=0, column=1, padx=(8, 0))

        content = ttk.Frame(outer)
        content.grid(row=1, column=0, sticky="nsew")
        content.columnconfigure(0, weight=1)
        content.columnconfigure(1, weight=1)

        self.build_package_panel(content)
        self.build_server_panel(content)
        self.build_package_list(outer)
        self.build_log_panel(outer)

    def build_package_panel(self, parent: ttk.Frame) -> None:
        panel = ttk.LabelFrame(parent, text=" Firmware Package ", style="Section.TLabelframe", padding=12)
        panel.grid(row=0, column=0, sticky="nsew", padx=(0, 6))
        panel.columnconfigure(1, weight=1)

        ttk.Label(panel, text="Target").grid(row=0, column=0, sticky="w", pady=4)
        target = ttk.Combobox(
            panel, textvariable=self.target_var, values=sorted(VALID_TARGETS),
            state="readonly", width=16
        )
        target.grid(row=0, column=1, sticky="ew", pady=4)

        ttk.Label(panel, text="Version").grid(row=1, column=0, sticky="w", pady=4)
        ttk.Entry(panel, textvariable=self.version_var).grid(row=1, column=1, sticky="ew", pady=4)

        ttk.Label(panel, text="Firmware").grid(row=2, column=0, sticky="w", pady=4)
        file_row = ttk.Frame(panel)
        file_row.grid(row=2, column=1, sticky="ew", pady=4)
        file_row.columnconfigure(0, weight=1)
        ttk.Entry(file_row, textvariable=self.file_var, state="readonly").grid(row=0, column=0, sticky="ew")
        ttk.Button(file_row, text="Browse...", command=self.choose_firmware).grid(row=0, column=1, padx=(6, 0))

        ttk.Label(panel, text="Size").grid(row=3, column=0, sticky="w", pady=4)
        ttk.Label(panel, textvariable=self.file_size_var).grid(row=3, column=1, sticky="w", pady=4)

        ttk.Label(panel, text="SHA-256").grid(row=4, column=0, sticky="nw", pady=4)
        sha_label = ttk.Label(panel, textvariable=self.sha_var, wraplength=400)
        sha_label.grid(row=4, column=1, sticky="w", pady=4)

        ttk.Button(
            panel, text="Register Firmware", style="Primary.TButton",
            command=self.register_firmware
        ).grid(row=5, column=0, columnspan=2, sticky="ew", pady=(12, 0))

    def build_server_panel(self, parent: ttk.Frame) -> None:
        panel = ttk.LabelFrame(parent, text=" HTTP Server ", style="Section.TLabelframe", padding=12)
        panel.grid(row=0, column=1, sticky="nsew", padx=(6, 0))
        panel.columnconfigure(1, weight=1)

        ttk.Label(panel, text="Bind Host").grid(row=0, column=0, sticky="w", pady=4)
        self.host_entry = ttk.Entry(panel, textvariable=self.host_var)
        self.host_entry.grid(row=0, column=1, sticky="ew", pady=4)

        ttk.Label(panel, text="Port").grid(row=1, column=0, sticky="w", pady=4)
        self.port_entry = ttk.Entry(panel, textvariable=self.port_var)
        self.port_entry.grid(row=1, column=1, sticky="ew", pady=4)

        ttk.Label(panel, text="HU URL").grid(row=2, column=0, sticky="w", pady=4)
        ttk.Entry(panel, textvariable=self.server_url_var, state="readonly").grid(
            row=2, column=1, sticky="ew", pady=4
        )

        button_row = ttk.Frame(panel)
        button_row.grid(row=3, column=0, columnspan=2, sticky="ew", pady=(12, 0))
        button_row.columnconfigure(0, weight=1)
        button_row.columnconfigure(1, weight=1)
        self.start_button = ttk.Button(
            button_row, text="Start Server", style="Primary.TButton", command=self.start_server
        )
        self.start_button.grid(row=0, column=0, sticky="ew", padx=(0, 4))
        self.stop_button = ttk.Button(button_row, text="Stop Server", command=self.stop_server, state="disabled")
        self.stop_button.grid(row=0, column=1, sticky="ew", padx=(4, 0))

        ttk.Label(
            panel,
            text="HU7 and this PC must use the same LAN. Allow TCP 8080 on Windows Private network.",
            wraplength=420
        ).grid(row=4, column=0, columnspan=2, sticky="w", pady=(14, 0))

    def build_package_list(self, parent: ttk.Frame) -> None:
        panel = ttk.LabelFrame(parent, text=" Registered Packages ", style="Section.TLabelframe", padding=8)
        panel.grid(row=2, column=0, sticky="ew", pady=(12, 6))
        panel.columnconfigure(0, weight=1)

        columns = ("target", "version", "file", "size", "sha256")
        self.package_tree = ttk.Treeview(panel, columns=columns, show="headings", height=5)
        headings = {
            "target": "Target", "version": "Version", "file": "File",
            "size": "Size", "sha256": "SHA-256"
        }
        widths = {"target": 80, "version": 100, "file": 220, "size": 100, "sha256": 420}
        for column in columns:
            self.package_tree.heading(column, text=headings[column])
            self.package_tree.column(column, width=widths[column], anchor="w")
        self.package_tree.grid(row=0, column=0, sticky="ew")
        scrollbar = ttk.Scrollbar(panel, orient=tk.VERTICAL, command=self.package_tree.yview)
        scrollbar.grid(row=0, column=1, sticky="ns")
        self.package_tree.configure(yscrollcommand=scrollbar.set)

        ttk.Button(panel, text="Refresh", command=self.refresh_packages).grid(
            row=1, column=0, sticky="e", pady=(6, 0)
        )

    def build_log_panel(self, parent: ttk.Frame) -> None:
        panel = ttk.LabelFrame(parent, text=" Server Log ", style="Section.TLabelframe", padding=8)
        panel.grid(row=3, column=0, sticky="nsew", pady=(6, 0))
        panel.columnconfigure(0, weight=1)
        panel.rowconfigure(0, weight=1)

        self.log_text = tk.Text(panel, height=10, state=tk.DISABLED, font=("Consolas", 9), wrap=tk.NONE)
        self.log_text.grid(row=0, column=0, sticky="nsew")
        scrollbar = ttk.Scrollbar(panel, orient=tk.VERTICAL, command=self.log_text.yview)
        scrollbar.grid(row=0, column=1, sticky="ns")
        self.log_text.configure(yscrollcommand=scrollbar.set)
        ttk.Button(panel, text="Clear Log", command=self.clear_log).grid(
            row=1, column=0, sticky="e", pady=(6, 0)
        )

    def choose_firmware(self) -> None:
        filename = filedialog.askopenfilename(
            title="Select ESP32 firmware",
            filetypes=(("ESP32 binary", "*.bin"), ("All files", "*.*"))
        )
        if not filename:
            return
        path = Path(filename)
        self.file_var.set(str(path))
        self.file_size_var.set(f"{path.stat().st_size:,} bytes")
        self.sha_var.set(sha256_file(path))

    def register_firmware(self) -> None:
        source = Path(self.file_var.get())
        target = self.target_var.get().strip().upper()
        version = self.version_var.get().strip()
        if target not in VALID_TARGETS:
            messagebox.showerror("Invalid Target", "Select a valid target.")
            return
        if not VERSION_PATTERN.fullmatch(version):
            messagebox.showerror("Invalid Version", "Use a version such as 1.0.1 or 1.0.1-beta.1.")
            return
        if not source.is_file() or source.suffix.lower() != ".bin":
            messagebox.showerror("Invalid Firmware", "Select an existing .bin file.")
            return

        target_dir = FIRMWARE_ROOT / target
        target_dir.mkdir(parents=True, exist_ok=True)
        destination = target_dir / f"{target}-{version}.bin"
        firmware_temp = target_dir / f".{destination.name}.tmp"
        manifest_path = target_dir / "manifest.json"
        manifest_temp = target_dir / ".manifest.json.tmp"
        try:
            shutil.copyfile(source, firmware_temp)
            firmware_temp.replace(destination)
            digest = sha256_file(destination)
            manifest = {"target": target, "version": version, "file": destination.name}
            manifest_temp.write_text(
                json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8"
            )
            manifest_temp.replace(manifest_path)
        except OSError as error:
            firmware_temp.unlink(missing_ok=True)
            manifest_temp.unlink(missing_ok=True)
            messagebox.showerror("Registration Failed", str(error))
            return

        self.file_size_var.set(f"{destination.stat().st_size:,} bytes")
        self.sha_var.set(digest)
        self.queue_log(f"Registered {target} {version}: {destination.name}")
        self.refresh_packages()
        messagebox.showinfo("Registered", f"{target} {version} firmware registered.")

    def refresh_packages(self) -> None:
        for item in self.package_tree.get_children():
            self.package_tree.delete(item)
        for target in sorted(VALID_TARGETS):
            manifest_path = FIRMWARE_ROOT / target / "manifest.json"
            if not manifest_path.is_file():
                continue
            try:
                manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
                firmware = FIRMWARE_ROOT / target / str(manifest["file"])
                size = firmware.stat().st_size
                digest = sha256_file(firmware)
                self.package_tree.insert(
                    "", tk.END,
                    values=(target, manifest["version"], firmware.name, f"{size:,}", digest)
                )
            except (OSError, KeyError, json.JSONDecodeError) as error:
                self.package_tree.insert("", tk.END, values=(target, "INVALID", str(error), "-", "-"))

    def start_server(self) -> None:
        if self.http_server is not None:
            return
        host = self.host_var.get().strip() or "0.0.0.0"
        try:
            port = int(self.port_var.get())
            if not 1 <= port <= 65535:
                raise ValueError
            self.http_server = create_server(host, port, self.queue_log)
        except (ValueError, OSError) as error:
            self.http_server = None
            messagebox.showerror("Server Start Failed", str(error))
            return

        self.server_url_var.set(f"http://{lan_ipv4()}:{port}")
        self.server_thread = threading.Thread(target=self.http_server.serve_forever, daemon=True)
        self.server_thread.start()
        self.server_state_var.set("Running")
        self.start_button.configure(state="disabled")
        self.stop_button.configure(state="normal")
        self.host_entry.configure(state="disabled")
        self.port_entry.configure(state="disabled")
        self.queue_log(f"Server started: {self.server_url_var.get()}")

    def stop_server(self) -> None:
        server = self.http_server
        if server is None:
            return
        self.http_server = None
        server.shutdown()
        server.server_close()
        if self.server_thread is not None:
            self.server_thread.join(timeout=2)
        self.server_thread = None
        self.server_state_var.set("Stopped")
        self.start_button.configure(state="normal")
        self.stop_button.configure(state="disabled")
        self.host_entry.configure(state="normal")
        self.port_entry.configure(state="normal")
        self.queue_log("Server stopped")

    def queue_log(self, message: str) -> None:
        self.log_queue.put(message)

    def drain_logs(self) -> None:
        messages: list[str] = []
        while True:
            try:
                messages.append(self.log_queue.get_nowait())
            except queue.Empty:
                break
        if messages:
            self.log_text.configure(state=tk.NORMAL)
            self.log_text.insert(tk.END, "\n".join(messages) + "\n")
            self.log_text.see(tk.END)
            self.log_text.configure(state=tk.DISABLED)
        self.after(100, self.drain_logs)

    def clear_log(self) -> None:
        self.log_text.configure(state=tk.NORMAL)
        self.log_text.delete("1.0", tk.END)
        self.log_text.configure(state=tk.DISABLED)

    def on_close(self) -> None:
        if self.http_server is not None:
            self.stop_server()
        self.destroy()


if __name__ == "__main__":
    OtaManagerApp().mainloop()
