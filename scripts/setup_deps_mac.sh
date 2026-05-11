#!/usr/bin/env bash
# Copyright (c) 2026 MeshForge Project
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Build all vendored dependencies for MeshForge on macOS.
# Run once on a fresh machine. Subsequent builds are fast with a warm ccache.
#
# Usage:
#   ./scripts/setup_deps_mac.sh [arm64|x86_64]
#
# Defaults to the current host architecture if no argument is given.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# --- Architecture ---
HOST_ARCH="$(uname -m)"
TARGET_ARCH="${1:-${HOST_ARCH}}"

if [[ "${TARGET_ARCH}" != "arm64" && "${TARGET_ARCH}" != "x86_64" ]]; then
    echo "ERROR: Unknown architecture '${TARGET_ARCH}'. Pass 'arm64' or 'x86_64'."
    exit 1
fi

echo "==> Building dependencies for ${TARGET_ARCH}"

# --- Homebrew tools ---
echo "==> Installing system tools via Homebrew..."
brew install \
    cmake \
    ninja \
    gettext \
    glew \
    mbedtls \
    libtool \
    autoconf \
    automake \
    pkg-config \
    texinfo

# ccache is strongly recommended; first-time build is ~60–90 min without it.
if ! command -v ccache &>/dev/null; then
    echo "==> Installing ccache (highly recommended)..."
    brew install ccache
fi

# --- ccache environment ---
export CCACHE_DIR="${HOME}/.ccache"
export CMAKE_C_COMPILER_LAUNCHER=ccache
export CMAKE_CXX_COMPILER_LAUNCHER=ccache

# --- Build ---
DEPS_BUILD_DIR="${REPO_ROOT}/deps/build_${TARGET_ARCH}"
echo "==> Configuring deps (output: ${DEPS_BUILD_DIR})"

cmake -S "${REPO_ROOT}/deps" \
      -B "${DEPS_BUILD_DIR}" \
      -G Ninja \
      -DCMAKE_OSX_ARCHITECTURES="${TARGET_ARCH}" \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=11.3

echo "==> Building deps (this takes 60–90 min on first run)..."
cmake --build "${DEPS_BUILD_DIR}" -j"$(sysctl -n hw.ncpu)"

echo ""
echo "==> Dependencies built successfully in ${DEPS_BUILD_DIR}"
echo "==> To build MeshForge, run:"
echo "      cmake -B build -G Ninja \\"
echo "            -DCMAKE_OSX_ARCHITECTURES=${TARGET_ARCH} \\"
echo "            -DCMAKE_OSX_DEPLOYMENT_TARGET=11.3 \\"
echo "            -DDEP_BUILD_DIR=${DEPS_BUILD_DIR} \\"
echo "            -DSLIC3R_NETWORK=OFF"
echo "      ninja -C build -j\$(sysctl -n hw.ncpu)"
