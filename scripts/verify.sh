#!/usr/bin/env bash
# Copyright (c) 2026 MeshForge Project
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Local CI gate for MeshForge. Every PR must pass this before opening.
# Runs in order; stops on first failure.
#
# Usage:
#   ./scripts/verify.sh [build_dir]
#
# Defaults to 'build_verify' in the repo root if no argument is given.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${1:-${REPO_ROOT}/build_verify}"

fail() {
    echo ""
    echo "FAILED: $1"
    exit 1
}

# ─── Step 1: Brand scrub ────────────────────────────────────────────────────
echo "==> [1/5] Brand scrub check..."
BRAND_HITS=$(grep -riP "\b(bambu|orca|softfever|prusaslicer)\b" \
    --include="*.cpp" --include="*.h" --include="*.mm" \
    --exclude-dir=deps \
    "${REPO_ROOT}/src/" \
    | grep -iv "copyright\|license\|based on\|adapted from\|originally" \
    || true)

if [[ -n "${BRAND_HITS}" ]]; then
    echo "Brand scrub FAILED. Remaining violations:"
    echo "${BRAND_HITS}"
    fail "brand scrub"
fi
echo "    Brand scrub: PASS"

# ─── Step 2: CMake configure ────────────────────────────────────────────────
echo "==> [2/5] CMake configure (Debug, SLIC3R_NETWORK=OFF)..."

HOST_ARCH="$(uname -m)"
OSX_ARCH_FLAG=""
if [[ "$(uname -s)" == "Darwin" ]]; then
    OSX_ARCH_FLAG="-DCMAKE_OSX_ARCHITECTURES=${HOST_ARCH}"
fi

cmake -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSLIC3R_NETWORK=OFF \
    -DBUILD_TESTS=ON \
    ${OSX_ARCH_FLAG} \
    "${REPO_ROOT}" \
    > "${BUILD_DIR}/cmake_configure.log" 2>&1 \
    || fail "CMake configure (see ${BUILD_DIR}/cmake_configure.log)"
echo "    CMake configure: PASS"

# ─── Step 3: Full build ─────────────────────────────────────────────────────
echo "==> [3/5] Full build..."
NCPU=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
ninja -C "${BUILD_DIR}" -j"${NCPU}" \
    > "${BUILD_DIR}/build.log" 2>&1 \
    || fail "Build (see ${BUILD_DIR}/build.log)"
echo "    Build: PASS"

# ─── Step 4: Catch2 unit tests ──────────────────────────────────────────────
echo "==> [4/5] Unit tests..."
ctest --test-dir "${BUILD_DIR}" -L unit --output-on-failure \
    || fail "unit tests"
echo "    Unit tests: PASS"

# ─── Step 5: Undo/redo canary ───────────────────────────────────────────────
echo "==> [5/5] Undo/redo canary..."
ctest --test-dir "${BUILD_DIR}" -R test_undo_redo_baseline --output-on-failure \
    || fail "undo/redo canary"
echo "    Undo/redo canary: PASS"

# ─── Summary ────────────────────────────────────────────────────────────────
echo ""
echo "ALL CHECKS PASSED"
