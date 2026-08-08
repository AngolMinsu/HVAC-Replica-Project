#!/usr/bin/env python3
"""Small LAN OTA server for the HVAC Replica head unit."""

from __future__ import annotations

import argparse
import hashlib
import json
import mimetypes
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, quote, unquote, urlparse

ROOT = Path(__file__).resolve().parent
FIRMWARE_ROOT = ROOT / "firmware"
LOG_ROOT = ROOT / "logs"
VALID_TARGETS = {"HU7", "MKBD", "GW", "BMS"}


def json_bytes(value: object) -> bytes:
    return json.dumps(value, ensure_ascii=False, separators=(",", ":")).encode("utf-8")


def safe_child(root: Path, *parts: str) -> Path | None:
    candidate = root.joinpath(*parts).resolve()
    try:
        candidate.relative_to(root.resolve())
    except ValueError:
        return None
    return candidate


def load_manifest(target: str) -> tuple[dict[str, object] | None, str | None]:
    if target not in VALID_TARGETS:
        return None, "unsupported target"
    target_dir = FIRMWARE_ROOT / target
    manifest_path = target_dir / "manifest.json"
    if not manifest_path.is_file():
        return None, "manifest not found"
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None, "invalid manifest"
    if manifest.get("target") != target:
        return None, "manifest target mismatch"
    version = manifest.get("version")
    filename = manifest.get("file")
    if not isinstance(version, str) or not version.strip():
        return None, "manifest version missing"
    if not isinstance(filename, str) or not filename.strip():
        return None, "manifest file missing"
    firmware_path = safe_child(target_dir, filename)
    if firmware_path is None or not firmware_path.is_file():
        return None, "firmware file not found"
    digest = hashlib.sha256()
    with firmware_path.open("rb") as firmware:
        for block in iter(lambda: firmware.read(1024 * 1024), b""):
            digest.update(block)
    return {
        "target": target,
        "version": version.strip(),
        "file": firmware_path.name,
        "size": firmware_path.stat().st_size,
        "sha256": digest.hexdigest(),
        "url": f"/firmware/{quote(target)}/{quote(firmware_path.name)}",
    }, None


class OtaRequestHandler(BaseHTTPRequestHandler):
    server_version = "HVAC-OTA/1.0"

    def send_json(self, status: int, value: object) -> None:
        payload = json_bytes(value)
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(payload)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(payload)

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/api/health":
            self.send_json(200, {"status": "ok"})
            return
        if parsed.path == "/api/firmware/latest":
            target = parse_qs(parsed.query).get("target", [""])[0].upper()
            manifest, error = load_manifest(target)
            if error:
                self.send_json(404, {"error": error, "target": target})
                return
            self.send_json(200, manifest)
            return
        if parsed.path.startswith("/firmware/"):
            parts = [unquote(part) for part in parsed.path.split("/")[2:]]
            if len(parts) != 2 or parts[0] not in VALID_TARGETS:
                self.send_json(404, {"error": "firmware not found"})
                return
            firmware_path = safe_child(FIRMWARE_ROOT, *parts)
            if firmware_path is None or not firmware_path.is_file():
                self.send_json(404, {"error": "firmware not found"})
                return
            self.send_response(200)
            self.send_header("Content-Type", mimetypes.guess_type(firmware_path.name)[0] or "application/octet-stream")
            self.send_header("Content-Length", str(firmware_path.stat().st_size))
            self.send_header("Content-Disposition", f'attachment; filename="{firmware_path.name}"')
            self.end_headers()
            with firmware_path.open("rb") as firmware:
                for block in iter(lambda: firmware.read(64 * 1024), b""):
                    self.wfile.write(block)
            return
        self.send_json(404, {"error": "not found"})

    def do_POST(self) -> None:
        if urlparse(self.path).path != "/api/ota/result":
            self.send_json(404, {"error": "not found"})
            return
        try:
            length = int(self.headers.get("Content-Length", "0"))
            if length <= 0 or length > 64 * 1024:
                raise ValueError
            event = json.loads(self.rfile.read(length).decode("utf-8"))
            if not isinstance(event, dict):
                raise ValueError
        except (ValueError, UnicodeDecodeError, json.JSONDecodeError):
            self.send_json(400, {"error": "invalid JSON body"})
            return
        event["serverTime"] = datetime.now(timezone.utc).isoformat()
        LOG_ROOT.mkdir(parents=True, exist_ok=True)
        with (LOG_ROOT / "ota-results.jsonl").open("a", encoding="utf-8") as output:
            output.write(json.dumps(event, ensure_ascii=False) + "\n")
        self.send_json(202, {"accepted": True})

    def log_message(self, fmt: str, *args: object) -> None:
        print(f"[{self.log_date_time_string()}] {self.address_string()} {fmt % args}")


def main() -> None:
    parser = argparse.ArgumentParser(description="HVAC Replica local OTA server")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()
    server = ThreadingHTTPServer((args.host, args.port), OtaRequestHandler)
    print(f"OTA server: http://{args.host}:{args.port}")
    server.serve_forever()


if __name__ == "__main__":
    main()
