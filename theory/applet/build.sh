#!/usr/bin/env bash
#
# Build (and optionally upload) the Proteus Infon Applet.
#
# Usage:
#   ./build.sh              # build only
#   ./build.sh --upload     # build, then rsync dist/ to the server
#   ./build.sh -u           # same as --upload
#   ./build.sh --upload --dest user@host:/path   # override destination
#
# Environment:
#   DEPLOY_DEST   overrides the default rsync destination
#
set -euo pipefail

cd "$(dirname "$0")"

DEFAULT_DEST="bruce@infomage.com"
DEST="${DEPLOY_DEST:-$DEFAULT_DEST}"
UPLOAD=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    -u|--upload) UPLOAD=1 ;;
    --dest) DEST="$2"; shift ;;
    -h|--help)
      grep '^#' "$0" | sed 's/^# \{0,1\}//'
      exit 0 ;;
    *) echo "Unknown argument: $1" >&2; exit 1 ;;
  esac
  shift
done

if [[ -d node_modules ]]; then
  echo ">> node_modules present, skipping install."
elif [[ -f package-lock.json ]]; then
  echo ">> Installing dependencies (npm ci)..."
  npm ci
else
  echo ">> Installing dependencies (npm install)..."
  npm install
fi

echo ">> Building..."
npm run build

if [[ "$UPLOAD" -eq 1 ]]; then
  echo ">> Uploading dist/ -> $DEST"
 # rsync -avz --delete dist/ "$DEST/"
#  echo ">> Upload complete."
else
  echo ">> Build complete. Run with --upload to deploy to $DEST"
fi
