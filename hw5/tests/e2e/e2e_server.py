#!/usr/bin/env python3

import json
import pathlib
import socket
import sys
import time


def usage() -> None:
    raise SystemExit(
        "usage: e2e_server.py <scenario.json> <port_file> <transcript_file>"
    )


def read_line(reader, transcript):
    line = reader.readline()
    if line == "":
        return None

    line = line.rstrip("\r\n")
    transcript.append(line)
    return line


def main() -> int:
    if len(sys.argv) != 4:
        usage()

    scenario_path = pathlib.Path(sys.argv[1])
    port_file = pathlib.Path(sys.argv[2])
    transcript_file = pathlib.Path(sys.argv[3])

    scenario = json.loads(scenario_path.read_text(encoding="utf-8"))
    timeout = float(scenario.get("timeout", 10.0))
    steps = scenario.get("steps", [])

    transcript = []

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(("127.0.0.1", 0))
    server_socket.listen(1)
    server_socket.settimeout(timeout)

    try:
        port = server_socket.getsockname()[1]
        port_file.write_text(f"{port}\n", encoding="utf-8")

        conn, _ = server_socket.accept()
        conn.settimeout(timeout)

        with conn:
            reader = conn.makefile("r", encoding="utf-8", newline="\n")
            writer = conn.makefile("w", encoding="utf-8", newline="\n")

            for index, step in enumerate(steps):
                action = step["action"]

                if action == "expect":
                    expected = step["line"]
                    actual = read_line(reader, transcript)
                    if actual is None:
                        raise AssertionError(
                            f"step {index}: expected '{expected}', but client closed connection"
                        )
                    if actual != expected:
                        raise AssertionError(
                            f"step {index}: expected '{expected}', got '{actual}'"
                        )

                elif action == "send":
                    writer.write(step["line"] + "\n")
                    writer.flush()

                elif action == "sleep":
                    time.sleep(float(step["seconds"]))

                elif action == "expect_close":
                    actual = read_line(reader, transcript)
                    if actual is not None:
                        raise AssertionError(
                            f"step {index}: expected connection close, got '{actual}'"
                        )

                else:
                    raise ValueError(f"unknown action '{action}' at step {index}")

        return 0
    finally:
        transcript_text = "".join(f"{line}\n" for line in transcript)
        transcript_file.write_text(transcript_text, encoding="utf-8")
        server_socket.close()


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[SERVER ERROR] {exc}", file=sys.stderr)
        raise