#!/usr/bin/env python3
"""
do-tftp.py

- Sends UDP control messages from local port 10501 to <ip>:10501
- Runs system 'tftp' to upload the board-specific uImage (.gz preferred if present)
- Sends stop + reboot, then polls until '?list#' returns something
- Shows a spinner + elapsed time while waiting for reboot

Board selection:
- --board zero|one
- default can be inferred from script name (do-tftp-zero.py / do-tftp-one.py)
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time
from typing import Optional

from udp_send import send_and_maybe_recv

CTRL_PORT = 10501
LOCAL_PORT = 10501
TIMEOUT_SEC = 1.0

BLUE = "\033[34m"
CYAN = "\033[36m"
RESET = "\033[0m"


def _decode_reply(reply: bytes) -> str:
    return reply.decode("utf-8", errors="replace").rstrip("\r\n\0")


def _udp_send_text(ip: str, msg: str, *, print_reply: bool = True) -> Optional[str]:
    data = msg.encode("ascii", errors="strict")

    _sent, reply = send_and_maybe_recv(
        ip,
        data,
        port=CTRL_PORT,
        local_port=LOCAL_PORT,
        timeout_sec=TIMEOUT_SEC,
    )

    if reply:
        text = _decode_reply(reply)
        if print_reply:
            print(text)
        return text

    return None


def _run_tftp_put(ip: str, filename: str) -> None:
    script = f"binary\nput {filename}\nquit\n"
    try:
        subprocess.run(
            ["tftp", ip],
            input=script.encode("ascii"),
            stdout=sys.stdout,
            stderr=sys.stderr,
            check=True,
        )
    except FileNotFoundError:
        raise SystemExit("Error: 'tftp' command not found. Install a tftp client and try again.")
    except subprocess.CalledProcessError as e:
        raise SystemExit(f"Error: tftp failed with exit code {e.returncode}")


def _wait_for_online(ip: str) -> str:
    spinner = "|/-\\"
    i = 0
    start = time.monotonic()

    while True:
        online = _udp_send_text(ip, "?list#", print_reply=False) or ""
        if online != "":
            sys.stdout.write("\r" + (" " * 60) + "\r")
            sys.stdout.flush()
            return online

        elapsed = time.monotonic() - start
        sys.stdout.write(f"\rWaiting for reboot {spinner[i % len(spinner)]}  {elapsed:5.1f}s")
        sys.stdout.flush()
        i += 1
        time.sleep(0.2)


def _infer_default_board_from_argv0(argv0: str) -> str:
    name = os.path.basename(argv0).lower()
    if "zero" in name:
        return "zero"
    if "one" in name:
        return "one"
    # sensible default if called as plain do-tftp.py
    return "zero"


def _stem_for_board(board: str) -> str:
    # Keep this explicit and boring (you said no plans for other boards).
    if board == "zero":
        return "orangepi_zero.uImage"
    if board == "one":
        return "orangepi_one.uImage"
    raise ValueError(f"Unsupported board '{board}'")


def _choose_filename(stem: str) -> str:
    gz = f"{stem}.gz"
    return gz if os.path.isfile(gz) else stem


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument("ip", help="Target IP address")
    parser.add_argument(
        "--board",
        choices=("zero", "one"),
        default=_infer_default_board_from_argv0(argv[0]),
        help="Board type (default inferred from script name, otherwise 'zero')",
    )
    ns = parser.parse_args(argv[1:])

    ip = ns.ip
    stem = _stem_for_board(ns.board)

    filename = _choose_filename(stem)
    if not os.path.isfile(filename):
        gz = f"{stem}.gz"
        print(f"Error: neither '{gz}' nor '{stem}' exists in the current directory.", file=sys.stderr)
        return 1

    # Start TFTP mode
    _udp_send_text(ip, "!tftp#1", print_reply=False)
    _udp_send_text(ip, "?tftp#")

    # Upload
    _run_tftp_put(ip, filename)

    # Stop TFTP mode
    _udp_send_text(ip, "!tftp#0", print_reply=False)
    _udp_send_text(ip, "?tftp#")

    print(f"{BLUE}Rebooting...{RESET}")
    _udp_send_text(ip, "?reboot##")

    online = _wait_for_online(ip)
    print(f"{CYAN}[{online}]{RESET}")

    version = _udp_send_text(ip, "?version#")
    if version:
        print(f"{CYAN}{version}{RESET}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

