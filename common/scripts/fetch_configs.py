#!/usr/bin/env python3
"""
Download config JSON files listed by /json/config/directory from a remote node,
plus default endpoints: /json/version and /json/list.

This version resolves the host ONCE (e.g. .local via mDNS) and then uses the IP
for all subsequent HTTP requests to avoid repeated resolver delays.

Usage:
  python3 fetch_configs.py <remote_node> <local_directory>
"""

from __future__ import annotations

import argparse
import json
import re
import socket
import sys
import urllib.error
import urllib.request
from pathlib import Path
from typing import Any, Dict, Tuple


def normalize_host(host: str) -> str:
    host = host.strip()
    host = re.sub(r"^https?://", "", host)  # remove scheme if provided
    host = host.split("/")[0]               # remove any path
    host = host.rstrip(".")                 # handle ".local." style
    return host


def resolve_host_once(host: str) -> Tuple[str, str]:
    """
    Resolve host to an IP once.
    Returns (display_host, ip) where display_host is the original host for logging.
    """
    try:
        # getaddrinfo gives us both IPv4/IPv6; prefer IPv4 for embedded nodes.
        infos = socket.getaddrinfo(host, 80, type=socket.SOCK_STREAM)
    except socket.gaierror as e:
        raise RuntimeError(f"DNS/mDNS lookup failed for {host}: {e}") from e

    ipv4 = next((ai for ai in infos if ai[0] == socket.AF_INET), None)
    chosen = ipv4 or infos[0]
    ip = chosen[4][0]
    return host, ip


def http_get_json(url: str, timeout: float = 10.0) -> Any:
    req = urllib.request.Request(
        url,
        headers={
            "Accept": "application/json",
            "User-Agent": "fetch-configs/1.2",
        },
        method="GET",
    )
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            charset = resp.headers.get_content_charset() or "utf-8"
            data = resp.read().decode(charset, errors="replace")
            return json.loads(data)
    except urllib.error.HTTPError as e:
        body = ""
        try:
            body = e.read().decode("utf-8", errors="replace")
        except Exception:
            pass
        raise RuntimeError(f"HTTP {e.code} for {url}\n{body}".strip()) from e
    except urllib.error.URLError as e:
        raise RuntimeError(f"URL error for {url}: {e.reason}") from e
    except json.JSONDecodeError as e:
        raise RuntimeError(f"Invalid JSON from {url}: {e}") from e


def save_json(out_dir: Path, filename: str, payload: Any) -> None:
    path = out_dir / filename
    with path.open("w", encoding="utf-8") as f:
        json.dump(payload, f, indent=4, sort_keys=True)
        f.write("\n")


def fetch_and_save(ip: str, out_dir: Path, url_path: str, out_name: str) -> bool:
    url_path = url_path if url_path.startswith("/") else "/" + url_path
    url = f"http://{ip}{url_path}"
    try:
        payload = http_get_json(url)
        save_json(out_dir, out_name, payload)
        print(f"[OK]  {url} -> {out_name}")
        return True
    except Exception as e:
        print(f"[FAIL] {url}: {e}", file=sys.stderr)
        return False


def main() -> int:
    ap = argparse.ArgumentParser(description="Fetch remote JSON endpoints into a local directory.")
    ap.add_argument("remote_node", help="Hostname/IP (optionally with scheme), e.g. gigadevice_486149.local.")
    ap.add_argument("local_directory", help="Destination directory")
    args = ap.parse_args()

    host = normalize_host(args.remote_node)
    out_dir = Path(args.local_directory).expanduser().resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"Resolving: {host}")
    # Resolve once (fixes repeated 5s delays with .local/mDNS)
    display_host, ip = resolve_host_once(host)

    print(f"Remote: {display_host} (resolved -> {ip})")
    print(f"Output: {out_dir}")
    print("")

    ok = 0
    fail = 0

    # 1) Default endpoints
    defaults = {
        "/json/version": "version.json",
        "/json/list": "list.json",
    }

    for path, out_name in defaults.items():
        if fetch_and_save(ip, out_dir, path, out_name):
            ok += 1
        else:
            fail += 1

    # 2) Directory + config endpoints
    directory_url = f"http://{ip}/json/config/directory"
    try:
        directory = http_get_json(directory_url)
        print(f"[OK]  {directory_url} -> config_directory.json")
        save_json(out_dir, "config_directory.json", directory)
        ok += 1
    except Exception as e:
        print(f"[FAIL] {directory_url}: {e}", file=sys.stderr)
        fail += 1
        print("")
        print(f"Done. OK={ok} FAIL={fail}")
        return 2

    files: Dict[str, Any] = {}
    if isinstance(directory, dict) and isinstance(directory.get("files"), dict):
        files = directory["files"]
    else:
        print(f"[FAIL] Unexpected directory JSON shape from {directory_url}", file=sys.stderr)
        print("")
        print(f"Done. OK={ok} FAIL={fail + 1}")
        return 2

    for path_key in files.keys():
        remote_path = path_key.lstrip("/")
        url_path = f"/json/{remote_path}"
        basename = remote_path.split("/")[-1]
        out_name = f"{basename}.json"

        if fetch_and_save(ip, out_dir, url_path, out_name):
            ok += 1
        else:
            fail += 1

    print("")
    print(f"Done. OK={ok} FAIL={fail}")
    return 0 if fail == 0 else 2


if __name__ == "__main__":
    raise SystemExit(main())
