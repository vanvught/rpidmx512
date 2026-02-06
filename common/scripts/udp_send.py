#!/usr/bin/env python3
"""
udp_send.py

Matches the behavior of the provided udp_send.c:
- bind local UDP port 10501 on INADDR_ANY
- send up to 512 bytes read from stdin to <ip>:10501
- if first byte is '?', wait up to 1 second for a reply and print it

Stand-alone:
  python3 udp_send.py <ip_address> < payload.bin

As a module:
  from udp_send import send_and_maybe_recv
"""

from __future__ import annotations

import socket
import sys
from typing import Optional, Tuple

BUFLEN = 512
PORT = 10501
DEFAULT_TIMEOUT_SEC = 1.0


def _set_reuse_opts(sock: socket.socket) -> None:
    # Mirrors the C code: SO_REUSEADDR and SO_REUSEPORT when present.
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Not available on all OSes / Python builds.
    if hasattr(socket, "SO_REUSEPORT"):
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        except OSError:
            # Some platforms define it but don't allow setting it for UDP in all cases.
            pass


def send_and_maybe_recv(
    ip_address: str,
    data: bytes,
    *,
    port: int = PORT,
    local_port: int = PORT,
    timeout_sec: float = DEFAULT_TIMEOUT_SEC,
    buf_len: int = BUFLEN,
) -> Tuple[int, Optional[bytes]]:
    """
    Sends `data` via UDP to (ip_address, port) from local bind (0.0.0.0, local_port).

    Returns:
      (sent_len, reply_bytes_or_None)

    If data starts with b'?', it will attempt to receive one reply up to `timeout_sec`.
    If timeout happens, reply is None.
    """
    if not isinstance(data, (bytes, bytearray, memoryview)):
        raise TypeError("data must be bytes-like")

    payload = bytes(data[:buf_len])
    if len(payload) == 0:
        return 0, None

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    try:
        _set_reuse_opts(sock)
        sock.settimeout(timeout_sec)

        # bind like the C code
        sock.bind(("0.0.0.0", local_port))

        sent = sock.sendto(payload, (ip_address, port))

        reply: Optional[bytes] = None
        if payload[:1] == b"?":
            try:
                # MSG_WAITALL isn't necessary in Python for datagrams; recvfrom returns one datagram.
                reply, _addr = sock.recvfrom(buf_len)
            except socket.timeout:
                reply = None

        return sent, reply
    finally:
        sock.close()


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(f"Usage: {argv[0]} ip_address", file=sys.stderr)
        return 2

    ip_address = argv[1]

    # Read up to BUFLEN bytes from stdin (binary)
    data = sys.stdin.buffer.read(BUFLEN)

    try:
        _sent, reply = send_and_maybe_recv(ip_address, data)
    except OSError as e:
        # Similar to perror()+exit(1)
        print(f"udp_send.py: {e}", file=sys.stderr)
        return 1

    if data[:1] == b"?" and reply is not None:
        # C code treats reply as text and prints it.
        # We'll write raw bytes to stdout to avoid encoding surprises.
        sys.stdout.buffer.write(reply)

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

