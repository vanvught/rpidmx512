#!/usr/bin/env python3
"""
reboot.py

Default:
  Reboot once.

Optional:
  --forever / -f   Loop forever 

Usage:
  python3 reboot.py <ip_address>
  python3 reboot.py <ip_address> --forever
"""

from __future__ import annotations

import sys
import time
import argparse
from typing import Optional

from udp_send import send_and_maybe_recv


LIST_CMD = b"?list#"
REBOOT_CMD = b"?reboot##"


def _reply_text(reply: Optional[bytes]) -> str:
    if not reply:
        return ""
    try:
        return reply.decode("utf-8", errors="replace").strip()
    except Exception:
        return repr(reply)


def _color(s: str, code: str) -> str:
    return f"\033[{code}m{s}\033[0m"


def wait_for_list(ip: str, *, timeout_sec: float = 1.0) -> str:
    while True:
        _sent, reply = send_and_maybe_recv(ip, LIST_CMD, timeout_sec=timeout_sec)
        text = _reply_text(reply)
        if text:
            return text
        time.sleep(0.1)


def spinner_wait_for_online(ip: str, *, timeout_sec: float = 1.0) -> str:
    frames = ["|", "/", "-", "\\"]
    i = 0
    last_poll = 0.0
    poll_interval = 0.25

    while True:
        now = time.monotonic()
        if now - last_poll >= poll_interval:
            last_poll = now
            _sent, reply = send_and_maybe_recv(ip, LIST_CMD, timeout_sec=timeout_sec)
            text = _reply_text(reply)
            if text:
                sys.stdout.write("\r" + " " * 60 + "\r")
                sys.stdout.flush()
                return text

        sys.stdout.write(f"\rWaiting for reboot {frames[i % len(frames)]}")
        sys.stdout.flush()
        i += 1
        time.sleep(0.08)


def do_reboot_cycle(ip: str) -> None:
    _sent, reply = send_and_maybe_recv(ip, LIST_CMD, timeout_sec=1.0)
    online = _reply_text(reply)

    print(_color(f"[{online}]", "33"))  # yellow

    if not online:
        online = wait_for_list(ip)
    print(_color(f"[{online}]", "36"))  # cyan

    time.sleep(2)

    send_and_maybe_recv(ip, REBOOT_CMD, timeout_sec=1.0)

    online_after = spinner_wait_for_online(ip)
    print(_color(f"Back online: [{online_after}]", "32"))  # green


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="UDP reboot tool")
    parser.add_argument("ip", help="Target IP address")
    parser.add_argument(
        "-f",
        "--forever",
        action="store_true",
        help="Reboot forever (loop mode)",
    )

    args = parser.parse_args(argv[1:])

    if args.forever:
        while True:
            do_reboot_cycle(args.ip)
    else:
        do_reboot_cycle(args.ip)

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

