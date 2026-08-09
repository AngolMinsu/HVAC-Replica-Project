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


def version_from_filename(target: str, path: Path) -> str:
    prefix = f"{target}-"
    if path.name.startswith(prefix) and path.name.lower().endswith(".bin"):
        return path.name[len(prefix):-4]
    return ""


def binary_kind(path: Path) -> str:
    try:
        with path.open("rb") as firmware:
            first_byte = firmware.read(1)
    except OSError:
        return "Unreadable"
    return "ESP32 App" if first_byte == b"\xE9" else "Non-ESP32"


def validate_firmware_file(target: str, path: Path) -> str | None:
    if not path.is_file() or path.suffix.lower() != ".bin":
        return "Select an existing .bin file."
    lowered = path.name.lower()
    if any(token in lowered for token in (".merged.bin", ".bootloader.bin", ".partitions.bin")):
        return "Use the application .ino.bin, not merged/bootloader/partitions binary."
    if target in {"HU7", "MKBD"} and binary_kind(path) != "ESP32 App":
        return "HU7/MKBD requires an ESP32 application binary with 0xE9 image header."
    return None


def write_manifest(target: str, version: str, firmware: Path) -> None:
    target_dir = FIRMWARE_ROOT / target
    target_dir.mkdir(parents=True, exist_ok=True)
    manifest_path = target_dir / "manifest.json"
    manifest_temp = target_dir / ".manifest.json.tmp"
    manifest = {"target": target, "version": version, "file": firmware.name}
    manifest_temp.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    manifest_temp.replace(manifest_path)

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
        self.package_items: dict[str, tuple[str, Path]] = {}

        self.target_var = tk.StringVar(value="HU7")
        self.version_var = tk.StringVar(value="0.5.3")
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
        panel = ttk.LabelFrame(parent, text=" Stored Binary Files ", style="Section.TLabelframe", padding=8)
        panel.grid(row=2, column=0, sticky="ew", pady=(12, 6))
        panel.columnconfigure(0, weight=1)

        columns = ("active", "target", "version", "file", "kind", "size", "sha256")
        self.package_tree = ttk.Treeview(panel, columns=columns, show="headings", height=6, selectmode="browse")
        headings = {
            "active": "Active", "target": "Target", "version": "Version", "file": "File",
            "kind": "Type", "size": "Size", "sha256": "SHA-256"
        }
        widths = {
            "active": 55, "target": 70, "version": 90, "file": 210,
            "kind": 95, "size": 85, "sha256": 360
        }
        for column in columns:
            self.package_tree.heading(column, text=headings[column])
            self.package_tree.column(column, width=widths[column], anchor="w")
        self.package_tree.grid(row=0, column=0, sticky="ew")
        self.package_tree.bind("<<TreeviewSelect>>", self.on_package_selected)
        scrollbar = ttk.Scrollbar(panel, orient=tk.VERTICAL, command=self.package_tree.yview)
        scrollbar.grid(row=0, column=1, sticky="ns")
        self.package_tree.configure(yscrollcommand=scrollbar.set)

        button_row = ttk.Frame(panel)
        button_row.grid(row=1, column=0, sticky="e", pady=(6, 0))
        ttk.Button(button_row, text="Set Active", command=self.set_selected_active).grid(row=0, column=0, padx=(0, 6))
        ttk.Button(button_row, text="Delete Selected", command=self.delete_selected_package).grid(row=0, column=1, padx=(0, 6))
        ttk.Button(button_row, text="Refresh", command=self.refresh_packages).grid(row=0, column=2)
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
        validation_error = validate_firmware_file(target, source)
        if validation_error:
            messagebox.showerror("Invalid Firmware", validation_error)
            return

        target_dir = FIRMWARE_ROOT / target
        target_dir.mkdir(parents=True, exist_ok=True)
        destination = target_dir / f"{target}-{version}.bin"
        firmware_temp = target_dir / f".{destination.name}.tmp"
        try:
            if source.resolve() != destination.resolve():
                shutil.copyfile(source, firmware_temp)
                firmware_temp.replace(destination)
            write_manifest(target, version, destination)
            digest = sha256_file(destination)
        except OSError as error:
            firmware_temp.unlink(missing_ok=True)
            messagebox.showerror("Registration Failed", str(error))
            return

        self.file_var.set(str(destination))
        self.file_size_var.set(f"{destination.stat().st_size:,} bytes")
        self.sha_var.set(digest)
        self.queue_log(f"Registered and activated {target} {version}: {destination.name}")
        self.refresh_packages()
        messagebox.showinfo("Registered", f"{target} {version} firmware registered and activated.")
    def refresh_packages(self) -> None:
        for item in self.package_tree.get_children():
            self.package_tree.delete(item)
        self.package_items.clear()

        for target in sorted(VALID_TARGETS):
            target_dir = FIRMWARE_ROOT / target
            manifest_path = target_dir / "manifest.json"
            active_file = ""
            active_version = ""
            if manifest_path.is_file():
                try:
                    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
                    active_file = str(manifest.get("file", ""))
                    active_version = str(manifest.get("version", ""))
                except (OSError, json.JSONDecodeError):
                    active_file = ""

            if not target_dir.is_dir():
                continue
            for firmware in sorted(target_dir.glob("*.bin"), key=lambda item: item.name.lower()):
                try:
                    size = firmware.stat().st_size
                    digest = sha256_file(firmware)
                    is_active = firmware.name == active_file
                    version = active_version if is_active else version_from_filename(target, firmware)
                    item_id = self.package_tree.insert(
                        "", tk.END,
                        values=("YES" if is_active else "", target, version or "-", firmware.name,
                                binary_kind(firmware), f"{size:,}", digest),
                    )
                    self.package_items[item_id] = (target, firmware)
                except OSError as error:
                    item_id = self.package_tree.insert(
                        "", tk.END, values=("", target, "INVALID", firmware.name, "Unreadable", "-", str(error))
                    )
                    self.package_items[item_id] = (target, firmware)

    def selected_package(self) -> tuple[str, Path] | None:
        selection = self.package_tree.selection()
        if not selection:
            return None
        return self.package_items.get(selection[0])

    def on_package_selected(self, _event=None) -> None:
        selected = self.selected_package()
        if selected is None:
            return
        target, firmware = selected
        version = version_from_filename(target, firmware)
        values = self.package_tree.item(self.package_tree.selection()[0], "values")
        if len(values) >= 3 and values[2] not in ("", "-", "INVALID"):
            version = str(values[2])
        self.target_var.set(target)
        if version:
            self.version_var.set(version)
        self.file_var.set(str(firmware))
        try:
            self.file_size_var.set(f"{firmware.stat().st_size:,} bytes")
            self.sha_var.set(sha256_file(firmware))
        except OSError as error:
            self.file_size_var.set("-")
            self.sha_var.set(str(error))

    def set_selected_active(self) -> None:
        selected = self.selected_package()
        if selected is None:
            messagebox.showerror("No Selection", "Select a binary file first.")
            return
        target, firmware = selected
        version = self.version_var.get().strip()
        if not VERSION_PATTERN.fullmatch(version):
            messagebox.showerror("Invalid Version", "Enter the version for the selected binary.")
            return
        validation_error = validate_firmware_file(target, firmware)
        if validation_error:
            messagebox.showerror("Invalid Firmware", validation_error)
            return
        try:
            write_manifest(target, version, firmware)
        except OSError as error:
            messagebox.showerror("Activation Failed", str(error))
            return
        self.queue_log(f"Activated {target} {version}: {firmware.name}")
        self.refresh_packages()

    def delete_selected_package(self) -> None:
        selected = self.selected_package()
        if selected is None:
            messagebox.showerror("No Selection", "Select a binary file first.")
            return
        target, firmware = selected
        if not messagebox.askyesno(
            "Delete Firmware", f"Delete {target}/{firmware.name}?\nThis cannot be undone."
        ):
            return
        manifest_path = FIRMWARE_ROOT / target / "manifest.json"
        try:
            if manifest_path.is_file():
                manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
                if manifest.get("file") == firmware.name:
                    manifest_path.unlink()
            firmware.unlink()
        except (OSError, json.JSONDecodeError) as error:
            messagebox.showerror("Delete Failed", str(error))
            return
        if Path(self.file_var.get()) == firmware:
            self.file_var.set("")
            self.file_size_var.set("-")
            self.sha_var.set("-")
        self.queue_log(f"Deleted {target}: {firmware.name}")
        self.refresh_packages()
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
