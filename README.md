# Mini KV Store

A Redis-inspired, in-memory key-value database written from scratch in C11/POSIX on Linux.

The project implements a TCP server, concurrent client handling, a custom hash table, command parsing, key expiration, append-only persistence, restart/replay, and a command-line client.

The goal was to build a small database system from first principles while focusing on data structures, networking, concurrency, memory management, persistence, and systems-level programming in C.

---

## Features

- In-memory key-value storage
- Custom hash table implementation
- Separate chaining for collision handling
- Automatic hash-table resizing and rehashing
- TCP client/server architecture
- POSIX-threaded concurrent clients
- Line-based text protocol
- Command parsing and dispatch
- Key expiration (TTL)
- Absolute expiration timestamps
- Append-only file (AOF) persistence
- Persistence replay after server restart
- Interactive command-line client
- Unit and integration testing
- Concurrent-client testing
- Performance benchmarking

---

## Supported Commands

| Command | Description | Example |
|---|---|---|
| `PING` | Test server responsiveness | `PING` |
| `SET` | Store or update a key | `SET name Minuu Shanmugam` |
| `GET` | Retrieve a value | `GET name` |
| `DEL` | Delete a key | `DEL name` |
| `EXISTS` | Check whether a key exists | `EXISTS name` |
| `EXPIRE` | Set a TTL in seconds | `EXPIRE name 60` |
| `EXPIREAT` | Set expiration using a Unix timestamp | `EXPIREAT name 1790000000` |
| `QUIT` | Close the client connection | `QUIT` |

Values may contain spaces.

### Example

```text
SET name Minuu Shanmugam
OK

GET name
Minuu Shanmugam

SET university Binghamton University
OK

GET university
Binghamton University

EXISTS name
1

DEL name
1

GET name
(nil)

Architecture

The system is organized into separate modules so that storage, networking, protocol handling, command processing, and persistence remain independent.

                         ┌──────────────────────┐
                         │    CLI Client        │
                         │   cli_client.c       │
                         └──────────┬───────────┘
                                    │
                                  TCP
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │     TCP Server       │
                         │      server.c        │
                         └──────────┬───────────┘
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │ Client Handler       │
                         │ client_handler.c     │
                         └──────────┬───────────┘
                                    │
                         ┌──────────┴───────────┐
                         │                      │
                         ▼                      ▼
                ┌─────────────────┐    ┌─────────────────┐
                │ Protocol Layer  │    │ Command Layer   │
                │  protocol.c     │    │  commands.c     │
                └─────────────────┘    └────────┬────────┘
                                                │
                                                ▼
                                      ┌──────────────────┐
                                      │   Hash Table     │
                                      │ hashtable.c/.h   │
                                      └────────┬─────────┘
                                               │
                                               ▼
                                      ┌──────────────────┐
                                      │  AOF Persistence │
                                      │ persistence.c/.h │
                                      └──────────────────┘

Project Structure

mini-kv-store/
│
├── src/
│   ├── server.c
│   ├── client_handler.c
│   ├── client_handler.h
│   ├── commands.c
│   ├── commands.h
│   ├── hashtable.c
│   ├── hashtable.h
│   ├── persistence.c
│   ├── persistence.h
│   ├── protocol.c
│   ├── protocol.h
│   └── cli_client.c
│
├── tests/
│   ├── benchmark.py
│   ├── test_all.c
│   ├── test_hashtable.c
│   └── test_integration.py
│
├── bin/
│   └── compiled binaries (ignored by Git)
│
├── Makefile
├── .gitignore
└── README.md

Core Design Decisions
1. Custom Hash Table

The database uses a custom hash table instead of a library-provided map.

The implementation uses:

FNV-1a 64-bit hashing
Separate chaining
Dynamically allocated keys and values
Automatic resizing
Rehashing when the table reaches approximately 75% load

Separate chaining was selected because it provides straightforward collision handling and deletion while keeping the implementation easy to reason about.

2. Dynamic Resizing

The hash table automatically grows when its load factor exceeds the configured threshold.

The resize process:

Allocate a larger bucket array.
Traverse the existing entries.
Recalculate each entry's bucket index.
Relink the existing nodes into the new buckets.
Release the old bucket array.

Entries themselves do not need to be recreated during rehashing.

3. Concurrency

The server accepts multiple TCP clients and handles clients using POSIX threads.

The shared in-memory store is protected so concurrent clients cannot corrupt the hash table during operations.

This allows multiple clients to perform operations against the same database instance.

4. Key Expiration

Keys can have a time-to-live.

For example:

SET session active
EXPIRE session 5

The key remains accessible while its expiration time has not been reached.

After expiration:

GET session
(nil)

Expiration metadata is kept at the application/database layer rather than inside the basic hash-table implementation.

5. Append-Only Persistence

Mutating operations are written to an append-only file.

The persistence layer allows the server to reconstruct its in-memory state after a restart.

The replay process reads the persisted commands and rebuilds the database.

Expiration timestamps are persisted so that already-expired keys are not incorrectly restored as permanent entries.

Protocol

The server uses a simple line-based text protocol.

Each command is terminated with a newline:

SET key value\n
GET key\n
DEL key\n
EXISTS key\n
EXPIRE key 60\n

Responses are also newline terminated.

Example:

Client                         Server
  │                              │
  │──── SET name Minuu ─────────>│
  │<────────── OK ───────────────│
  │                              │
  │──── GET name ───────────────>│
  │<──── Minuu Shanmugam ───────│

The protocol is intentionally simple so the focus remains on the underlying storage, networking, concurrency, and persistence mechanisms.

Building

The project requires:

Linux/Unix environment
GCC
C11 support
POSIX threads
Python 3 for integration tests and benchmarking

Build everything with:

make

Or perform a clean rebuild:

make clean && make

The binaries are generated under:

bin/

Running the Server

Start the server with:

./bin/mini-kv-server

The server listens on:

127.0.0.1:6380

Expected output:

mini-kv-server listening on port 6380

Running the CLI

In another terminal:

./bin/mini-kv-cli

The client connects to the local server.

Example session:

Connected to 127.0.0.1:6380. Type commands; QUIT exits.

SET name Minuu Shanmugam
OK

GET name
Minuu Shanmugam

SET university Binghamton University
OK

GET university
Binghamton University

SET program Computer Science
OK

GET program
Computer Science

Testing

The project contains both unit-level and end-to-end tests.

Run the complete test suite with:

make test

The test suite covers:

Hash-table creation and destruction
SET
GET
UPDATE
EXISTS
DELETE
Hash-table resizing
Rehashing
Command handling
PING
TTL operations
Expiration
Invalid commands
Persistence
Replay after restart
TCP communication
Concurrent clients
End-to-end server behavior
Current Test Results

The complete test suite currently reports:

Suite: 3/3 test groups passed
70 checks passed
0 checks failed

Integration test: PASS
  TCP protocol: PASS
  commands: PASS
  TTL expiration: PASS
  concurrent clients: PASS (20 clients)
  persistence + restart replay: PASS
Performance Benchmark

The repository includes:

tests/benchmark.py

The benchmark performs sequential TCP requests from a single client and measures end-to-end request/response throughput.

The default benchmark performs:

10,000 SET operations
10,000 GET operations
Measured Result

Benchmark performed on the project's Linux development environment (remote02) using 10,000 operations per workload:

SET: 10000 ops in 12.866409 s -> 777.22 ops/s
GET: 10000 ops in 0.372855 s -> 26646.38 ops/s

These measurements represent a single sequential client and include TCP request/response overhead.

They should be interpreted as a baseline rather than a maximum theoretical throughput measurement.

Run the benchmark with:

python3 tests/benchmark.py

The server must be running before executing the benchmark.

Example TTL Workflow

Start the server:

./bin/mini-kv-server

Connect with the CLI:

./bin/mini-kv-cli

Then:

SET name Minuu Shanmugam
OK

EXPIRE name 5
OK

GET name
Minuu Shanmugam

After approximately five seconds:

GET name
(nil)
Persistence Workflow

The server writes mutating operations to its append-only persistence file.

A typical persistence workflow is:

Client
  │
  │ SET / DEL / EXPIRE
  ▼
Command Processor
  │
  ├──────────────► Hash Table
  │
  └──────────────► Append-Only File

After restarting the server:

AOF
 │
 ▼
Replay
 │
 ▼
In-Memory Hash Table

This allows previously persisted state to be reconstructed.

Error Handling

The implementation validates:

Missing command arguments
Invalid commands
Invalid TTL values
Missing keys
Malformed requests
Failed socket operations
Memory allocation failures where applicable
Persistence errors

The server is designed to keep individual client errors isolated rather than terminating the entire server process.

Technical Concepts Demonstrated

This project brings together several systems-programming concepts:

Data Structures
Hash tables
Hash functions
Separate chaining
Dynamic resizing
Load factors
Rehashing
Systems Programming
C11
Manual memory management
POSIX APIs
File I/O
Error handling
Networking
TCP sockets
Client/server architecture
Connection handling
Request/response protocols
Concurrency
POSIX threads
Shared-state synchronization
Multiple simultaneous clients
Database Concepts
In-memory storage
TTL-based expiration
Append-only persistence
State reconstruction/replay
Software Engineering
Modular architecture
Unit testing
Integration testing
Performance measurement
Make-based builds
Git version control
Limitations

This project intentionally keeps the implementation small and educational.

Current limitations include:

Single-process server architecture
In-memory dataset
Simple line-based protocol
Append-only persistence rather than a full database storage engine
No replication
No clustering
No authentication
No TLS
No LRU eviction policy

These provide natural directions for future development.

Future Improvements

Possible extensions include:

LRU cache eviction
RESP-compatible protocol
Pub/Sub
Snapshot persistence
Replication
More advanced benchmarking
Connection pooling
Metrics and monitoring
Authentication
TLS support
Containerization
Kubernetes deployment
Learning Objectives

The project was developed to gain practical experience with:

Designing data structures from scratch.
Building network services in C.
Managing concurrent access to shared state.
Implementing persistence and recovery.
Designing a modular systems architecture.
Writing automated tests for low-level software.
Measuring real system performance rather than relying on theoretical estimates.
License

This project is intended for educational and portfolio use.
