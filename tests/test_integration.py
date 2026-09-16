#!/usr/bin/env python3
import socket
import subprocess
import tempfile
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SERVER = ROOT / "bin" / "mini-kv-server"


def command(sock, text):
    sock.sendall((text + "\n").encode())
    data = b""
    while not data.endswith(b"\n"):
        chunk = sock.recv(4096)
        if not chunk:
            raise RuntimeError("server disconnected")
        data += chunk
    return data.decode()


def start_server(cwd, port):
    return subprocess.Popen(
        [str(SERVER), str(port)],
        cwd=cwd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )


def wait_for_server(port, proc):
    deadline = time.time() + 3
    while time.time() < deadline:
        if proc.poll() is not None:
            raise RuntimeError(proc.stdout.read())
        try:
            s = socket.create_connection(("127.0.0.1", port), timeout=0.2)
            s.close()
            return
        except OSError:
            time.sleep(0.05)
    raise RuntimeError("server did not start")


def main():
    port = 6381
    with tempfile.TemporaryDirectory() as td:
        proc = start_server(td, port)
        try:
            wait_for_server(port, proc)
            s = socket.create_connection(("127.0.0.1", port), timeout=2)
            checks = [
                ("PING", "PONG\n"),
                ("SET name Minuu Shanmugam", "OK\n"),
                ("GET name", "Minuu Shanmugam\n"),
                ("EXISTS name", "1\n"),
                ("EXPIRE name 1", "1\n"),
            ]
            for cmd, expected in checks:
                actual = command(s, cmd)
                assert actual == expected, (cmd, actual, expected)
            time.sleep(1.2)
            assert command(s, "GET name") == "(nil)\n"
            assert command(s, "DEL name") == "0\n"
            command(s, "SET program Computer Science")
            command(s, "SET university Binghamton University")
            command(s, "QUIT")
            s.close()

            errors = []
            def worker(i):
                try:
                    c = socket.create_connection(("127.0.0.1", port), timeout=2)
                    assert command(c, f"SET k{i} value-{i}") == "OK\n"
                    assert command(c, f"GET k{i}") == f"value-{i}\n"
                    command(c, "QUIT")
                    c.close()
                except Exception as exc:
                    errors.append((i, exc))

            threads = [threading.Thread(target=worker, args=(i,)) for i in range(20)]
            for t in threads: t.start()
            for t in threads: t.join()
            assert not errors, errors

            proc.terminate()
            proc.wait(timeout=3)
            proc = start_server(td, port)
            wait_for_server(port, proc)
            s = socket.create_connection(("127.0.0.1", port), timeout=2)
            assert command(s, "GET program") == "Computer Science\n"
            assert command(s, "GET university") == "Binghamton University\n"
            assert command(s, "GET k19") == "value-19\n"
            command(s, "QUIT")
            s.close()
            print("Integration test: PASS")
            print("  TCP protocol: PASS")
            print("  commands: PASS")
            print("  TTL expiration: PASS")
            print("  concurrent clients: PASS (20 clients)")
            print("  persistence + restart replay: PASS")
        finally:
            if proc.poll() is None:
                proc.terminate()
                try:
                    proc.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    proc.kill()


if __name__ == "__main__":
    main()
