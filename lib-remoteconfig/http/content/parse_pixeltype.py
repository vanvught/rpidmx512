#!/usr/bin/env python3
"""
Parse pixeltype.h and convert the `kTypeInfo[]` table to JSON.

Rules applied:
- led_map: remove leading 'k'  -> "kGRB" becomes "GRB"
- protocol_type:
    - "kSpi" becomes "spi"
    - "kRtz" becomes "rtz"
- for SPI entries:
    - omit low_code
    - omit high_code
- for RTZ entries:
    - omit default_hz
    - omit max_hz

Usage:
    python3 parse_pixeltype.py pixeltype.h
    python3 parse_pixeltype.py pixeltype.h -o typeinfo.json
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


SPI_RE = re.compile(
    r'''
    MakeSpiTypeInfo
    \s*\(
        \s*"(?P<name>[^"]+)"\s*,
        \s*LedCount::k(?P<led_count>\d+)\s*,
        \s*(?P<default_hz>\d+)\s*,
        \s*(?P<max_hz>\d+)\s*
    \)
    ''',
    re.VERBOSE | re.MULTILINE,
)

RTZ_RE = re.compile(
    r'''
    MakeRtzTypeInfo
    \s*\(
        \s*"(?P<name>[^"]+)"\s*,
        \s*LedMap::(?P<led_map>k[A-Za-z0-9_]+)\s*,
        \s*LedCount::k(?P<led_count>\d+)\s*,
        \s*(?P<high_code>0x[0-9A-Fa-f]+|\d+)\s*
    \)
    ''',
    re.VERBOSE | re.MULTILINE,
)


def strip_comments(text: str) -> str:
    text = re.sub(r'//.*?$', '', text, flags=re.MULTILINE)
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
    return text


def extract_ktypeinfo_block(text: str) -> str:
    marker = 'inline constexpr TypeInfo kTypeInfo[] ='
    start = text.find(marker)
    if start == -1:
        raise ValueError("Could not find 'inline constexpr TypeInfo kTypeInfo[] ='")

    brace_start = text.find('{', start)
    if brace_start == -1:
        raise ValueError("Could not find opening '{' for kTypeInfo[]")

    depth = 0
    for i in range(brace_start, len(text)):
        ch = text[i]
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                return text[brace_start + 1:i]

    raise ValueError("Could not find closing '}' for kTypeInfo[]")


def normalize_led_map(value: str) -> str:
    return value[1:] if value.startswith("k") else value


def parse_entries(block: str) -> list[dict]:
    entries: list[dict] = []

    for raw_line in block.splitlines():
        line = raw_line.strip().rstrip(',')
        if not line:
            continue

        spi_match = SPI_RE.fullmatch(line)
        if spi_match:
            entries.append(
                {
                    "name": spi_match.group("name"),
                    "protocol_type": "spi",
                    "led_count": int(spi_match.group("led_count")),
                    "default_hz": int(spi_match.group("default_hz")),
                    "max_hz": int(spi_match.group("max_hz")),
                    "led_map": "RGB",
                }
            )
            continue

        rtz_match = RTZ_RE.fullmatch(line)
        if rtz_match:
            entries.append(
                {
                    "name": rtz_match.group("name"),
                    "protocol_type": "rtz",
                    "led_count": int(rtz_match.group("led_count")),
                    "low_code": 0xC0,
                    "high_code": int(rtz_match.group("high_code"), 0),
                    "led_map": normalize_led_map(rtz_match.group("led_map")),
                }
            )
            continue

        raise ValueError(f"Unrecognized kTypeInfo entry:\n{raw_line}")

    return entries


def default_output_path(input_path: Path) -> Path:
    return input_path.with_suffix(".json")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert pixel::kTypeInfo[] from a C++ header into JSON."
    )
    parser.add_argument("header", type=Path, help="Path to pixeltype.h")
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="Output JSON file path (default: same name, .json suffix)",
    )
    args = parser.parse_args()

    if not args.header.is_file():
        print(f"Error: file not found: {args.header}", file=sys.stderr)
        return 1

    try:
        text = args.header.read_text(encoding="utf-8")
        text = strip_comments(text)
        block = extract_ktypeinfo_block(text)
        entries = parse_entries(block)
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    output = {
        #"type_count": len(entries),
        "type_info": entries,
    }

    output_path = args.output or default_output_path(args.header)
    output_path.write_text(json.dumps(output, indent=2), encoding="utf-8")

    print(f"Wrote {len(entries)} entries to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())