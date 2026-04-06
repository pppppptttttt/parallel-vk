#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

setup_case "$@"
start_server "${SCRIPT_DIR}/scenarios/wait_done.json"
run_client_sync
assert_client_exit 0
wait_server 0
assert_transcript_equals $'HELLO 1000000\n'
print_ok "wait_done"