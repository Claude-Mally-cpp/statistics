#!/usr/bin/env python3
from __future__ import annotations

import argparse
import pathlib
import re
import sys


PROJECT_ROOT = pathlib.Path(__file__).resolve().parent.parent
CMAKE_LISTS = PROJECT_ROOT / "CMakeLists.txt"
VERSION_PATTERN = re.compile(r"(^\s*VERSION\s+)(\d+)\.(\d+)\.(\d+)(\s*$)", re.MULTILINE)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Bump the project version in CMakeLists.txt."
    )
    parser.add_argument(
        "target",
        help="One of: major, minor, patch, or an explicit version like 1.2.3",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print the version change without rewriting CMakeLists.txt.",
    )
    return parser.parse_args()


def read_current_version(text: str) -> tuple[str, tuple[int, int, int], re.Match[str]]:
    match = VERSION_PATTERN.search(text)
    if match is None:
        raise ValueError("Could not find a project VERSION entry in CMakeLists.txt")

    version_text = ".".join(match.group(index) for index in range(2, 5))
    version = tuple(int(match.group(index)) for index in range(2, 5))
    return version_text, version, match


def resolve_target(target: str, current: tuple[int, int, int]) -> tuple[int, int, int]:
    if target == "major":
        return current[0] + 1, 0, 0
    if target == "minor":
        return current[0], current[1] + 1, 0
    if target == "patch":
        return current[0], current[1], current[2] + 1

    explicit = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", target)
    if explicit is None:
        raise ValueError("Target must be major, minor, patch, or an explicit X.Y.Z version")

    return tuple(int(explicit.group(index)) for index in range(1, 4))


def detect_encoding(raw: bytes) -> str:
    if raw.startswith(b"\xef\xbb\xbf"):
        return "utf-8-sig"
    return "utf-8"


def main() -> int:
    args = parse_args()
    raw = CMAKE_LISTS.read_bytes()
    encoding = detect_encoding(raw)
    text = raw.decode(encoding)
    current_text, current_version, match = read_current_version(text)
    next_version = resolve_target(args.target, current_version)
    next_text = ".".join(str(part) for part in next_version)

    if next_text == current_text:
        print(f"Version unchanged: {current_text}")
        return 0

    print(f"Bumped version: {current_text} -> {next_text}")
    if args.dry_run:
        return 0

    updated = (
        text[: match.start()]
        + f"{match.group(1)}{next_text}{match.group(5)}"
        + text[match.end() :]
    )
    CMAKE_LISTS.write_bytes(updated.encode(encoding))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(2)
