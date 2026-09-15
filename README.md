# Mini KV Store

A Redis-style in-memory key-value store written from scratch in C11 using POSIX APIs.

## Project Status

🚧 Currently under development.

## Goals

This project is designed to demonstrate systems programming concepts including:

- TCP networking
- Concurrent client handling
- Custom hash tables
- Text-based network protocols
- Thread synchronization
- Key expiration and TTL
- Append-only persistence
- Client-server architecture
- Load testing and benchmarking

## Planned Architecture

```text
              TCP
Client ──────────────────► Server
                              │
                              ▼
                       Protocol Parser
                              │
                              ▼
                       Command Dispatch
                         /          \
                        ▼            ▼
                 Hash Table     Persistence Log
                        │
                        ▼
                  In-Memory Data