# File & Directory Description

This file provides a summary of the project structure and folder descriptions for developers and AI agents working on this codebase.

## Directory Structure

```
strukdat-final/
├── .agents/
│   └── rules/
│       └── agents.md             # Rules and constraints for AI agents
├── docs/
│   ├── file_desc.md              # [This File] Description of directory contents
│   ├── system_rules_and_constraints.md # Domain logic, priorities, and system rules
│   ├── ui_implementation.md          # Page-level UI spec and component breakdown
│   └── DESIGN.md                     # Design system (typography, colors, spacing, components)
├── include/                      # C++ Header Files
│   ├── Patient.h                 # Patient model, status enum, priority enum, and comparator
│   ├── QueueSystem.h             # Queue management class declaration
│   └── Benchmark.h               # Benchmark engine declaration
├── src/                          # C++ Source Files
│   ├── QueueSystem.cpp           # Queue system methods implementation
│   ├── Benchmark.cpp             # Performance benchmark tools implementation
│   └── main.cpp                  # Main CLI and JSON CLI runner
├── web/                          # Web UI Application
│   ├── backend/                  # Node.js Express server (spawns C++ executable)
│   │   ├── server.js             # Server API endpoints and runner
│   │   └── package.json          # Backend dependencies
│   └── frontend/                 # React + Vite frontend dashboard
│       ├── package.json          # Frontend dependencies
│       └── src/                  # Styling and React view components
├── kode_uts/                     # Legacy UTS source code
│   └── hospital_queue_system.cpp # Original single-file implementation
└── Makefile                      # Compilation build rules
```

## Description of Key Files

### Configuration & Docs
* **`docs/system_rules_and_constraints.md`**: Outlines business requirements for hospital queues, priority mappings (1 to 4), and status state machines.
* **`docs/ui_implementation.md`**: Page-level UI specification with element-by-element breakdowns for every view, component, and interaction.
* **`docs/DESIGN.md`**: Design system document defining typography (Playfair Display + Inter), color tokens, spacing scale, component specs, and anti-patterns.
* **`.agents/rules/agents.md`**: AI guidelines to restrict automatic Git commits/pushes.

### C++ Modular Backend (`include/` and `src/`)
* **`Patient.h`**: Encapsulates the `Patient` struct containing ID, Name, Service Type, Priority Level, Arrival Time, Queue Number, and Status.
* **`QueueSystem.h` / `QueueSystem.cpp`**: Implements the hospital queue system logic using `std::priority_queue` (min-heap/max-heap depending on priority) and `std::unordered_map` (hash table for O(1) searches). Includes file storage logic.
* **`Benchmark.h` / `Benchmark.cpp`**: Tools for measuring execution times and comparing `std::priority_queue` vs. `std::queue` (FIFO) across different data scales (100, 1000, 10000+ records).
* **`main.cpp`**: Provides two modes of execution:
  1. An interactive CLI menu for terminal usage.
  2. A JSON-based CLI interface (e.g., `./hospital_queue --json '{"action":"insert", "data":...}'`) for Web UI communication.
