#!/usr/bin/env bash
# Copyright (c) 2026 MeshForge Project
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# PR 6.5: Notarize and staple MeshForge.app.
# Run AFTER sign_mac.sh.
#
# Usage:
#   ./scripts/notarize_mac.sh <path-to-MeshForge.app> <apple-id> <team-id> <keychain-profile>
#
# Example:
#   ./scripts/notarize_mac.sh build/MeshForge.app dev@acme.com TEAMID1234 meshforge-notary
#
# Setup keychain profile once (stores credentials securely):
#   xcrun notarytool store-credentials meshforge-notary \
#     --apple-id dev@acme.com \
#     --team-id TEAMID1234 \
#     --password <app-specific-password>

set -euo pipefail

if [[ $# -lt 4 ]]; then
    echo "Usage: $0 <path-to-MeshForge.app> <apple-id> <team-id> <keychain-profile>"
    exit 1
fi

APP_PATH="$1"
APPLE_ID="$2"
TEAM_ID="$3"
KEYCHAIN_PROFILE="$4"
ZIP_PATH="/tmp/meshforge_notarize_$$.zip"

if [[ ! -d "${APP_PATH}" ]]; then
    echo "ERROR: '${APP_PATH}' not found."
    exit 1
fi

echo "==> Zipping ${APP_PATH} → ${ZIP_PATH}"
ditto -c -k --keepParent "${APP_PATH}" "${ZIP_PATH}"

echo "==> Submitting to Apple notary service"
xcrun notarytool submit "${ZIP_PATH}" \
    --apple-id "${APPLE_ID}" \
    --team-id "${TEAM_ID}" \
    --keychain-profile "${KEYCHAIN_PROFILE}" \
    --wait

echo "==> Stapling ticket to ${APP_PATH}"
xcrun stapler staple "${APP_PATH}"

echo "==> Verifying Gatekeeper acceptance"
spctl --assess --verbose "${APP_PATH}"

rm -f "${ZIP_PATH}"
echo "==> Done. '${APP_PATH}' is notarized and stapled."
