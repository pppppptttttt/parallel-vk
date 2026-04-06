#!/usr/bin/env bash
set -euo pipefail

setup_case() {
    if [[ $# -ne 3 ]]; then
        echo "usage: <script> <client_binary> <python_bin> <server_script>" >&2
        return 2
    fi

    CLIENT_BIN="$1"
    PYTHON_BIN="$2"
    SERVER_SCRIPT="$3"

    CASE_TMPDIR="$(mktemp -d)"
    PORT_FILE="${CASE_TMPDIR}/port.txt"
    TRANSCRIPT_FILE="${CASE_TMPDIR}/transcript.txt"
    SERVER_STDOUT="${CASE_TMPDIR}/server.stdout"
    SERVER_STDERR="${CASE_TMPDIR}/server.stderr"
    CLIENT_STDOUT="${CASE_TMPDIR}/client.stdout"
    CLIENT_STDERR="${CASE_TMPDIR}/client.stderr"
    HASHRATE_FILE="${CASE_TMPDIR}/hashrate.txt"

    printf '1000000\n' > "${HASHRATE_FILE}"

    CLIENT_PID=""
    SERVER_PID=""

    trap cleanup_case EXIT
}

cleanup_case() {
    if [[ -n "${CLIENT_PID:-}" ]]; then
        if kill -0 "${CLIENT_PID}" 2>/dev/null; then
            kill "${CLIENT_PID}" 2>/dev/null || true
        fi
        wait "${CLIENT_PID}" 2>/dev/null || true
    fi

    if [[ -n "${SERVER_PID:-}" ]]; then
        if kill -0 "${SERVER_PID}" 2>/dev/null; then
            kill "${SERVER_PID}" 2>/dev/null || true
        fi
        wait "${SERVER_PID}" 2>/dev/null || true
    fi

    if [[ -n "${CASE_TMPDIR:-}" && -d "${CASE_TMPDIR}" ]]; then
        rm -rf "${CASE_TMPDIR}"
    fi
}

start_server() {
    local scenario_file="$1"

    "${PYTHON_BIN}" "${SERVER_SCRIPT}" \
        "${scenario_file}" \
        "${PORT_FILE}" \
        "${TRANSCRIPT_FILE}" \
        >"${SERVER_STDOUT}" \
        2>"${SERVER_STDERR}" &
    SERVER_PID=$!

    for _ in $(seq 1 100); do
        if [[ -s "${PORT_FILE}" ]]; then
            return 0
        fi
        sleep 0.05
    done

    echo "[FAIL] server did not publish port in time" >&2
    if [[ -f "${SERVER_STDERR}" ]]; then
        cat "${SERVER_STDERR}" >&2 || true
    fi
    return 1
}

run_client_sync() {
    set +e
    "${CLIENT_BIN}" \
        127.0.0.1 \
        "$(cat "${PORT_FILE}")" \
        "${HASHRATE_FILE}" \
        >"${CLIENT_STDOUT}" \
        2>"${CLIENT_STDERR}"
    CLIENT_RC=$?
    set -e
}

start_client_async() {
    "${CLIENT_BIN}" \
        127.0.0.1 \
        "$(cat "${PORT_FILE}")" \
        "${HASHRATE_FILE}" \
        >"${CLIENT_STDOUT}" \
        2>"${CLIENT_STDERR}" &
    CLIENT_PID=$!
}

wait_client() {
    local expected_rc="$1"

    if [[ -z "${CLIENT_PID:-}" ]]; then
        echo "[FAIL] client process was not started asynchronously" >&2
        return 1
    fi

    set +e
    wait "${CLIENT_PID}"
    CLIENT_RC=$?
    set -e
    CLIENT_PID=""

    assert_client_exit "${expected_rc}"
}

wait_server() {
    local expected_rc="$1"

    if [[ -z "${SERVER_PID:-}" ]]; then
        echo "[FAIL] server process was not started" >&2
        return 1
    fi

    set +e
    wait "${SERVER_PID}"
    local server_rc=$?
    set -e
    SERVER_PID=""

    if [[ "${server_rc}" -ne "${expected_rc}" ]]; then
        echo "[FAIL] server exit code: got ${server_rc}, expected ${expected_rc}" >&2
        echo "--- server stdout ---" >&2
        cat "${SERVER_STDOUT}" >&2 || true
        echo "--- server stderr ---" >&2
        cat "${SERVER_STDERR}" >&2 || true
        return 1
    fi
}

assert_client_exit() {
    local expected_rc="$1"

    if [[ "${CLIENT_RC:-999}" -ne "${expected_rc}" ]]; then
        echo "[FAIL] client exit code: got ${CLIENT_RC:-unset}, expected ${expected_rc}" >&2
        echo "--- client stdout ---" >&2
        cat "${CLIENT_STDOUT}" >&2 || true
        echo "--- client stderr ---" >&2
        cat "${CLIENT_STDERR}" >&2 || true
        return 1
    fi
}

assert_file_contains() {
    local file_path="$1"
    local needle="$2"

    if ! grep -Fq "${needle}" "${file_path}"; then
        echo "[FAIL] file ${file_path} does not contain: ${needle}" >&2
        echo "--- file content ---" >&2
        cat "${file_path}" >&2 || true
        return 1
    fi
}

assert_transcript_equals() {
    local expected_text="$1"
    local expected_file="${CASE_TMPDIR}/expected_transcript.txt"

    printf '%s' "${expected_text}" > "${expected_file}"

    if ! diff -u "${expected_file}" "${TRANSCRIPT_FILE}" >&2; then
        echo "[FAIL] transcript mismatch" >&2
        return 1
    fi
}

print_ok() {
    local test_name="$1"
    echo "[OK] ${test_name}"
}