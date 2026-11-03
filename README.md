# C++ Low-Latency Trading Engine

A C++20 project focused on building a low-latency trading engine, starting with an order book and gradually evolving into a benchmarked, multithreaded and performance-optimized exchange simulator.

The primary goal of this project is to develop strong practical skills in modern C++, data structures, memory management, concurrency and performance engineering.

## Project Objectives

- Build an order book using price-time priority.
- Implement a matching engine for buy and sell orders.
- Explore efficient data structures and memory management.
- Understand multithreading, atomic operations and lock-free programming.
- Measure and optimize execution performance.
- Develop a realistic exchange simulator.

## Technology Stack

| Technology | Purpose |
|---|---|
| C++20 | Core programming language |
| GCC / MinGW | C++ compiler |
| CMake | Build system configuration |
| Git | Version control |
| GitHub | Source code hosting |
| VS Code | Development environment |

## Project Roadmap

| Phase | Description | Status |
|---|---|---|
| 00 | Foundation and Project Setup | In Progress |
| 01 | Order Book | Planned |
| 02 | Matching Engine | Planned |
| 03 | Testing and Correctness | Planned |
| 04 | Market Simulator | Planned |
| 05 | Memory Engineering | Planned |
| 06 | Multithreading | Planned |
| 07 | Atomic Operations | Planned |
| 08 | Lock-Free Programming | Planned |
| 09 | Performance Engineering | Planned |
| 10 | Advanced Exchange Simulation | Planned |
| 11 | Production-Quality Polish | Planned |

## Project Structure

```text
low-latency-trading-engine/
│
├── CMakeLists.txt
├── README.md
├── .gitignore
│
├── include/          # Header files
├── src/              # Source files
├── tests/            # Unit and correctness tests
├── benchmarks/       # Performance benchmarks
│
└── build/             # Generated build files (not committed)
```

## Prerequisites

- C++20-compatible compiler (GCC/MinGW recommended for the current setup).
- CMake 3.21 or newer.
- Git.
- Visual Studio Code or another C++ development environment.

## Building the Project

### 1. Clone the repository

```bash
git clone https://github.com/yuvrajsoni-sys/low-latency-trading-engine.git
cd low-latency-trading-engine
```

### 2. Configure and build Debug

```powershell
cmake -S . -B build-debug -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

Run the executable:

```powershell
.\build-debug\trading_engine.exe
```

### 3. Configure and build Release

```powershell
cmake -S . -B build-release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

Run:

```powershell
.\build-release\trading_engine.exe
```

## Development Principles

The project will follow these principles:

1. **Correctness first:** Validate functionality before optimization.
2. **Understand the fundamentals:** Study the data structures and algorithms behind each implementation.
3. **Measure performance:** Use benchmarks rather than assumptions.
4. **Optimize carefully:** Identify bottlenecks before making changes.
5. **Write maintainable code:** Keep the project modular and documented.

## Performance Philosophy

The central approach to performance engineering is:

**Measure → Identify Bottleneck → Optimize → Measure Again**

Performance improvements will be evaluated using benchmarks as the project develops.

## Current Development Status

The initial project structure and CMake build configuration are being established.

The order book and matching engine are planned components and have not yet been implemented.

Further development, testing and benchmark results will be documented as each phase is completed.

---

*This project is being developed as a practical exploration of modern C++, low-latency systems and exchange infrastructure.*