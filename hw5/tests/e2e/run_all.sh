#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/../.." && pwd)"

CLIENT_BIN="${1:-}"
PYTHON_BIN="${2:-python3}"
SERVER_SCRIPT="${SCRIPT_DIR}/e2e_server.py"

if [[ -z "${CLIENT_BIN}" ]]; then
    BUILD_DIR="${PROJECT_ROOT}/build"
    cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}"
    cmake --build "${BUILD_DIR}"
    CLIENT_BIN="${BUILD_DIR}/hw5"
fi

if [[ ! -x "${CLIENT_BIN}" ]]; then
    echo "[FAIL] client binary not found or not executable: ${CLIENT_BIN}" >&2
    exit 1
fi

tests=(
    test_hello_done.sh
    test_wait_done.sh
    test_task_found.sh
    test_task_not_found.sh
    test_invalid_server_message.sh
    test_invalid_task_range.sh
    test_sigterm_during_bruteforce.sh
)

for test_script in "${tests[@]}"; do
    echo "[RUN] ${test_script}"
    bash "${SCRIPT_DIR}/${test_script}" "${CLIENT_BIN}" "${PYTHON_BIN}" "${SERVER_SCRIPT}"
done

echo "[OK] all e2e tests passed"