#!/usr/bin/env bash
# Copyright (c) 2026 MeshForge Project
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Run a libFuzzer harness from tests/fuzz/.
# NOT part of the PR gate (too slow). Run explicitly before parser PRs.
#
# Usage:
#   ./scripts/run_fuzz.sh <target> [libfuzzer-args...]
#
# Examples:
#   ./scripts/run_fuzz.sh fuzz_stl -max_total_time=300
#   ./scripts/run_fuzz.sh fuzz_3mf -max_total_time=300 -rss_limit_mb=2048
#
# Prerequisites:
#   - Build with Clang + ASan + libfuzzer:
#       cmake -B build_fuzz -G Ninja \
#         -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
#         -DSLIC3R_ASAN=ON -DSLIC3R_NETWORK=OFF
#       ninja -C build_fuzz <target>

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_FUZZ_DIR="${REPO_ROOT}/build_fuzz"

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <target> [libfuzzer-args...]"
    echo "Available targets (after build):"
    ls "${BUILD_FUZZ_DIR}/tests/fuzz/" 2>/dev/null | grep "^fuzz_" || echo "  (none built yet)"
    exit 1
fi

TARGET="$1"
shift

BINARY="${BUILD_FUZZ_DIR}/tests/fuzz/${TARGET}"

if [[ ! -x "${BINARY}" ]]; then
    echo "ERROR: '${BINARY}' not found or not executable."
    echo "Build fuzz targets first:"
    echo "  cmake -B ${BUILD_FUZZ_DIR} -G Ninja \\"
    echo "    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \\"
    echo "    -DSLIC3R_ASAN=ON -DSLIC3R_NETWORK=OFF ${REPO_ROOT}"
    echo "  ninja -C ${BUILD_FUZZ_DIR} ${TARGET}"
    exit 1
fi

CORPUS_DIR="${REPO_ROOT}/tests/fuzz/corpus/${TARGET}"
mkdir -p "${CORPUS_DIR}"

echo "==> Running ${TARGET} with corpus at ${CORPUS_DIR}"
echo "==> Extra args: $*"
exec "${BINARY}" "${CORPUS_DIR}" "$@"
