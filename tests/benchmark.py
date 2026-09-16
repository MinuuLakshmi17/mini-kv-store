#!/usr/bin/env python3
import socket
import sys
import time

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 6380
OPERATIONS = int(sys.argv[3]) if len(sys.argv) > 3 else 10000


def command(sock, text):
    sock.sendall((text + "\n").encode())
    data = b""
    while not data.endswith(b"\n"):
        chunk = sock.recv(4096)
        if not chunk:
            raise RuntimeError("server disconnected")
        data += chunk
    return data


def run(label, sock, template):
    start = time.perf_counter()
    for i in range(OPERATIONS):
        response = command(sock, template.format(i=i))
        if not response:
            raise RuntimeError("empty response")
    elapsed = time.perf_counter() - start
    ops = OPERATIONS / elapsed
    print(f"{label}: {OPERATIONS} ops in {elapsed:.6f} s -> {ops:.2f} ops/s")


s = socket.create_connection((HOST, PORT), timeout=5)
run("SET", s, "SET bench{i} value{i}")
run("GET", s, "GET bench{i}")
command(s, "QUIT")
s.close()
