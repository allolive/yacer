#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Write to the CoreELEC mirror the builds are pinned to (MIRROR_REPO, by default
# allolive/CoreELEC): it follows CoreELEC/CoreELEC, and keeps every commit a
# build was pinned to under a tag, so upstream rewriting its history cannot take
# one away. CI writes with the mirror's deploy key, in MIRROR_KEY; the key is
# written to a file only for the push and removed after it.
#
#   yacer-mirror.sh push <sha> <ref> [--force]   point <ref> at <sha>
#   yacer-mirror.sh delete <ref>
#   yacer-mirror.sh has <ref>                    exit 0 if the mirror has it
set -euo pipefail

MIRROR_REPO=${MIRROR_REPO:-allolive/CoreELEC}
UPSTREAM=https://github.com/CoreELEC/CoreELEC
READ=https://github.com/$MIRROR_REPO
WRITE=git@github.com:$MIRROR_REPO.git

die() { echo "yacer-mirror: $*" >&2; exit 1; }

with_key() {
  [ -n "${MIRROR_KEY:-}" ] || die "no MIRROR_KEY - the mirror's deploy key"
  local key
  key=$(mktemp)
  trap 'rm -f "$key"' RETURN
  printf '%s\n' "$MIRROR_KEY" > "$key"
  GIT_SSH_COMMAND="ssh -i $key -o IdentitiesOnly=yes -o StrictHostKeyChecking=accept-new" "$@"
}

case "${1:-}" in
  has)
    [ $# -eq 2 ] || die "usage: has <ref>"
    git ls-remote --exit-code "$READ" "$2" >/dev/null
    ;;
  push)
    [ $# -ge 3 ] || die "usage: push <sha> <ref> [--force]"
    sha=$2 ref=$3 force=${4:-}
    [[ "$sha" =~ ^[0-9a-f]{40}$ ]] || die "not a full commit sha: $sha"
    work=$(mktemp -d)
    trap 'rm -rf "$work"' EXIT
    git -C "$work" init -q
    # the mirror has it unless upstream has moved on since; then upstream has it
    git -C "$work" fetch -q --depth=1 "$READ" "$sha" 2>/dev/null \
      || git -C "$work" fetch -q --depth=1 "$UPSTREAM" "$sha" \
      || die "${sha:0:10} is neither in the mirror nor upstream"
    with_key git -C "$work" push -q ${force:+--force} "$WRITE" "$sha:$ref"
    ;;
  delete)
    [ $# -eq 2 ] || die "usage: delete <ref>"
    work=$(mktemp -d)
    trap 'rm -rf "$work"' EXIT
    git -C "$work" init -q
    with_key git -C "$work" push -q "$WRITE" ":$2"
    ;;
  *)
    die "usage: yacer-mirror.sh push <sha> <ref> [--force] | delete <ref> | has <ref>"
    ;;
esac
