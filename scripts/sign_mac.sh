#!/usr/bin/env bash
# Copyright (c) 2026 MeshForge Project
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# PR 6.5: Sign MeshForge.app with a Developer ID Application certificate.
#
# Usage:
#   ./scripts/sign_mac.sh <path-to-MeshForge.app> <developer-id>
#
# Example:
#   ./scripts/sign_mac.sh build/MeshForge.app "Developer ID Application: Acme Corp (TEAMID)"
#
# Prerequisites:
#   - Xcode command-line tools installed
#   - Valid Developer ID Application certificate in the keychain
#   - scripts/entitlements.plist present

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENTITLEMENTS="${SCRIPT_DIR}/entitlements.plist"

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 <path-to-MeshForge.app> <developer-id>"
    exit 1
fi

APP_PATH="$1"
DEVELOPER_ID="$2"

if [[ ! -d "${APP_PATH}" ]]; then
    echo "ERROR: '${APP_PATH}' is not a directory / .app bundle."
    exit 1
fi

if [[ ! -f "${ENTITLEMENTS}" ]]; then
    echo "ERROR: '${ENTITLEMENTS}' not found."
    exit 1
fi

echo "==> Signing ${APP_PATH}"
echo "    Identity : ${DEVELOPER_ID}"
echo "    Entitlements: ${ENTITLEMENTS}"

codesign \
    --deep \
    --force \
    --verify \
    --verbose \
    --sign "${DEVELOPER_ID}" \
    --options runtime \
    --entitlements "${ENTITLEMENTS}" \
    "${APP_PATH}"

echo "==> Verifying signature"
codesign --verify --deep --strict --verbose=2 "${APP_PATH}"
spctl --assess --type exec --verbose "${APP_PATH}"

echo "==> Done. '${APP_PATH}' is signed."
