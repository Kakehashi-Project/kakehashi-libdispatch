#!/usr/bin/env bash
# Copyright 2026 Kakehashi Project
# SPDX-License-Identifier: Apache-2.0
# Test alias argument/result delegation, not guest scheduler execution.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDK="$(xcrun --sdk macosx --show-sdk-path)"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

for arch in arm64 x86_64; do
	# Link only libc's SDK child, never the libSystem umbrella: otherwise a
	# missing alias could silently bind to Apple's dispatch implementation.
	xcrun clang -arch "$arch" -Wall -Wextra -Werror -nostdlib \
		-Wl,-e,_main -Wl,-dead_strip "$SDK/usr/lib/system/libsystem_c.tbd" \
		"$ROOT/tests/kakehashi_v2_aliases.c" "$ROOT/src/kakehashi_compat.c" \
		-o "$OUT/aliases-$arch"
	if xcrun nm -u "$OUT/aliases-$arch" | grep -q 'dispatch_'; then
		echo "error: alias test must define every dispatch operation locally" >&2
		exit 1
	fi
	"$OUT/aliases-$arch"
done
