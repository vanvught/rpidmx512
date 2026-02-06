#!/usr/bin/env python3
"""
do-tftp.py

Python port of the provided bash script, using udp_send.py for the UDP control
messages and invoking the system 'tftp' client (no external dependencies).

Usage:
  python3 do-tftp.py <ip_address> <file_to_put>

Behavior matches the bash script:
- toggles tftp on, waits until device responds and reports not Off
- checks '?list#' for 'TFTP Server' and reboots if missing, then repeats enable
- runs: tftp <ip> (binary, put <file>, quit)
- toggles tftp off, waits until device responds and reports not On
- reboots and waits until '?list#' responds, then prints list and version
"""

from __future__ import annotations

import os
import subprocess
import sys
sys.dont_write_bytecode = True
import time
from typing import Optional

import udp_send  # expects udp_send.py to be importable (same dir or PYTHONPATH)

PORT = 10501
BUFLEN = 512
TIMEOUT_SEC = 1.0


def _udp_cmd(ip: str, cmd: str) -> str:
    """
    Send cmd via udp_send and return reply as text if any, else "".
    Mirrors: echo '...cmd...' | udp_send <ip>
    """
    data = cmd.encode("utf-8", errors="strict")
    _sent, reply = udp_send.send_and_maybe_recv(
        ip, data, port=PORT, local_port=PORT, timeout_sec=TIMEOUT_SEC, buf_len=BUFLEN
    )
    if not reply:
        return ""
    return reply.decode("utf-8", errors="replace").rstrip("\r\n")


def _sleep1() -> None:
    time.sleep(1.0)


def _wait_nonempty_reply(ip: str, query_cmd: str, tick_cmd: Optional[str] = None) -> str:
    """
    Repeatedly:
      optional send tick_cmd (e.g. '!tftp#1' or '!tftp#0')
      query query_cmd (e.g. '?tftp#' or '?list#')
    until reply != "".
    """
    reply = _udp_cmd(ip, query_cmd)
    while reply == "":
        _sleep1()
        if tick_cmd is not None:
            _udp_cmd(ip, tick_cmd)
        reply = _udp_cmd(ip, query_cmd)
    return reply


def _wait_until_not_equal(ip: str, query_cmd: str, bad_value: str, tick_cmd: Optional[str] = None) -> str:
    """
    Wait until query reply is non-empty and != bad_value.
    """
    reply = _wait_nonempty_reply(ip, query_cmd, tick_cmd=tick_cmd)
    while reply == bad_value:
        _sleep1()
        if tick_cmd is not None:
            _udp_cmd(ip, tick_cmd)
        reply = _udp_cmd(ip, query_cmd)
        if reply == "":
            # keep consistent with bash (it loops separately on empty)
            reply = _wait_nonempty_reply(ip, query_cmd, tick_cmd=tick_cmd)
    return reply


def _reboot_and_wait_list(ip: str) -> str:
    print("Rebooting...")
    _udp_cmd(ip, "?reboot##")

    reply = _udp_cmd(ip, "?list#")
    while reply == "":
        time.sleep(0.2)
        reply = _udp_cmd(ip, "?list#")
    return reply


def _ensure_tftp_on(ip: str) -> str:
    # initial kick
    _udp_cmd(ip, "!tftp#1")
    on_line = _udp_cmd(ip, "?tftp#")
    print(f"[{on_line}]")

    # while empty: sleep 1, kick + query
    on_line = _wait_nonempty_reply(ip, "?tftp#", tick_cmd="!tftp#1")
    print(f"[{on_line}]")

    # while "tftp:Off": sleep 1, kick + query
    on_line = _wait_until_not_equal(ip, "?tftp#", "tftp:Off", tick_cmd="!tftp#1")
    print(f"[{on_line}]")
    return on_line


def _ensure_tftp_off(ip: str) -> str:
    _udp_cmd(ip, "!tftp#0")
    on_line = _udp_cmd(ip, "?tftp#")
    print(f"[{on_line}]")

    on_line = _wait_nonempty_reply(ip, "?tftp#", tick_cmd="!tftp#0")
    print(f"[{on_line}]")

    on_line = _wait_until_not_equal(ip, "?tftp#", "tftp:On", tick_cmd="!tftp#0")
    print(f"[{on_line}]")
    return on_line


def _run_tftp_put(ip: str, filename: str) -> None:
    print("tftp...")
    # Equivalent of:
    # tftp $1 << -EOF
    # binary
    # put $2
    # quit
    # -EOF
    script = f"binary\nput {filename}\nquit\n"
    try:
        subprocess.run(
            ["tftp", ip],
            input=script.encode("utf-8"),
            check=True,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )
    except FileNotFoundError:
        raise RuntimeError("System 'tftp' client not found in PATH.")
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"tftp failed with exit code {e.returncode}.")


def main(argv: list[str]) -> int:
    if len(argv) < 3:
        print(f"Usage: {argv[0]} ip_address file", file=sys.stderr)
        return 2

    ip = argv[1]
    filepath = argv[2]

    if not os.path.isfile(filepath):
        # matches bash: echo $2 else exit
        return 1

    print(filepath)

    # Enable TFTP, wait until device reports it isn't Off
    _ensure_tftp_on(ip)

    on_line = _udp_cmd(ip, "?list#")
    print(f"[{on_line}]")

    sub = "TFTP Server"
    if sub not in on_line:
        time.sleep(1.0)
        on_line = _reboot_and_wait_list(ip)

        # After reboot: enable again (with the same wait loops)
        _ensure_tftp_on(ip)

    # Do the transfer
    _run_tftp_put(ip, filepath)

    # Disable TFTP, wait until device reports it isn't On
    _ensure_tftp_off(ip)

    # Reboot again and wait for list to respond
    on_line = _reboot_and_wait_list(ip)
    print(f"[{on_line}]")

    version = _udp_cmd(ip, "?version#")
    print(version, end="" if version.endswith("\n") else "\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

