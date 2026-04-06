#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

setup_case "$@"
start_server "${SCRIPT_DIR}/scenarios/sigterm_during_bruteforce.json"

start_client_async
sleep 0.5
kill -TERM "${CLIENT_PID}"

wait_client 0
wait_server 0

assert_transcript_equals $'HELLO 1000000\n'
assert_file_contains "${CLIENT_STDOUT}" "bruteforce aborted"

print_ok "sigterm_during_bruteforce"
