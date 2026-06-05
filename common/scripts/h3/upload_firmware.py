#!/usr/bin/env python3

import sys
import os
import math
import time
import http.client

from udp_send import send_and_maybe_recv

BLUE = "\033[34m"
CYAN = "\033[36m"
RESET = "\033[0m"

CHUNK_SIZE = 1024  # 1 KB

def _decode_reply(reply: bytes) -> str:
    return reply.decode("utf-8", errors="replace").rstrip("\r\n\0")


def _udp_send_text(ip: str, msg: str, *, print_reply: bool = True):
    data = msg.encode("ascii", errors="strict")

    _sent, reply = send_and_maybe_recv(
        ip,
        data,
        port=10501,
        local_port=10501,
        timeout_sec=1.0,
    )

    if reply:
        text = _decode_reply(reply)
        if print_reply:
            print(text)
        return text

    return None

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

def upload_firmware(hostname, filepath):
    filesize = os.path.getsize(filepath)
    filename = os.path.basename(filepath)
    max_chunks = math.ceil(filesize / CHUNK_SIZE)

    conn = http.client.HTTPConnection(hostname, timeout=5)

    try:
        #
        # Step 1: upload_start
        #
        print("Sending upload_start...")

        conn.request(
            "POST",
            "/upload_start",
            headers={
                "X-Upload-Size": str(filesize),
                "X-Upload-Name": filename
            }
        )

        resp = conn.getresponse()

        if resp.status != 200:
            raise Exception(
                f"upload_start failed: {resp.status} {resp.reason}"
            )

        resp.read()

        #
        # Step 2: upload chunks
        #
        print(f"Uploading {filename} ({filesize:,} bytes)")

        with open(filepath, "rb") as f:
            chunk_count = 0
            offset = 0

            while True:
                chunk = f.read(CHUNK_SIZE)

                if not chunk:
                    break

                conn = http.client.HTTPConnection(hostname, timeout=5)

                conn.request(
                    "POST",
                    "/upload",
                    body=chunk,
                    headers={
                        "Content-Type": "application/octet-stream"
                    }
                )

                res = conn.getresponse()

                if res.status != 200:
                    raise Exception(
                        f"Chunk upload failed at offset {offset}: "
                        f"{res.status} {res.reason}"
                    )

                res.read()

                chunk_count += 1
                offset += len(chunk)

                print(
                    f"\rUploaded chunk {chunk_count}/{max_chunks}",
                    end="",
                    flush=True
                )

        print()

        #
        # Step 3: upload_complete
        #
        conn = http.client.HTTPConnection(hostname, timeout=5)

        print("Sending upload_complete...")

        conn.request("POST", "/upload_complete")

        resp = conn.getresponse()

        if resp.status != 200:
            raise Exception(
                f"upload_complete failed: "
                f"{resp.status} {resp.reason}"
            )

        resp.read()

        print("Upload complete.")

        print(f"{BLUE}Rebooting...{RESET}")
        _udp_send_text(hostname, "?reboot##")

        online = _wait_for_online(hostname)
        print(f"{CYAN}[{online}]{RESET}")

        version = _udp_send_text(hostname, "?version#")
        if version:
            print(f"{CYAN}{version}{RESET}")

    except Exception as e:
        print(f"\nError: {e}")

    finally:
        conn.close()


if len(sys.argv) != 3:
    print(
        "Usage: python3 upload_firmware.py "
        "<hostname> <firmware_file>"
    )
    sys.exit(1)

upload_firmware(sys.argv[1], sys.argv[2])