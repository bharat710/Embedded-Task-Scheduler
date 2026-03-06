# Embedded Task Scheduler

A lightweight embedded task scheduler implemented in **C** with **POSIX** simulation. Manages multiple concurrent tasks using configurable scheduling policies, driven by a software timer.

---

## System Architecture

```
Timer / System Clock
        │
        ▼
  ┌─────────────┐
  │  Scheduler   │──── Round Robin / Priority-Based
  │    Core      │
  └──────┬──────┘
         │
         ▼
  ┌─────────────┐
  │  Task Queue  │   (Task Control Blocks)
  │  / Task Table│
  └──────┬──────┘
         │
         ▼
  ┌─────────────┐
  │   Task       │   LED · Sensor · Logger · Motor · Comms
  │  Execution   │
  └─────────────┘
```

## Features

| Feature                  |
|--------------------------|
| Task Control Block (TCB) |
| Task creation / deletion | 
| Task state management    | 
| Round Robin scheduling   | 
| Priority-Based scheduling| 
| Periodic task execution  | 
| Software timer           |
| Cooperative mutex        | 
| Coloured terminal output | 
| CLI-configurable policy  | 

## Project Structure

```
Project/
├── Makefile
├── README.md
└── src/
    ├── main.c                  # Entry point, CLI parsing, task registration
    ├── scheduler/
    │   ├── scheduler.h         # TCB, scheduler structs, public API
    │   ├── scheduler.c         # Scheduler core: RR, Priority, task mgmt
    │   ├── sync.h              # Mutex primitives
    │   └── sync.c              # Mutex implementation
    ├── tasks/
    │   ├── tasks.h             # Example task declarations
    │   └── tasks.c             # LED, sensor, logging, motor, comms tasks
    └── timer/
        ├── timer.h             # Software timer API
        └── timer.c             # Timer implementation (usleep-based)
```

## System Requirements

- **OS**: macOS or Linux
- **Compiler**: GCC or Clang with C11 support
- **Tools**: GNU Make

## How to Compile

```bash
make            # Build the project
make clean      # Remove build artifacts
```

The compiled binary is placed at `build/scheduler`.

## How to Run

```bash
# Round Robin scheduling (default), 40 ticks
./build/scheduler rr 40

# Priority-Based scheduling, 30 ticks
./build/scheduler priority 30

# Show help
./build/scheduler --help
```

### Shortcut Targets

```bash
make run            # Build & run with Round Robin, 40 ticks
make run-rr         # Same as above
make run-priority   # Build & run with Priority-Based, 40 ticks
```

## Registered Demo Tasks

| Task             | Priority | Period | Description              |
|------------------|----------|--------|--------------------------|
| LED Toggle       | 2        | 10 ms  | Simulates blinking LED   |
| Sensor Read      | 1        | 20 ms  | Random temperature value |
| Data Logger      | 3        | 15 ms  | Sequential log entries   |
| Motor Control    | 0 (high) | 25 ms  | Random PWM duty cycle    |
| Communication    | 4        | 30 ms  | Sequential packet sends  |

## Scheduling Policies

### Round Robin
Tasks cycle in order: A → B → C → D → E → A → … Only READY tasks are selected. Provides fair execution time.

### Priority-Based
The READY task with the **lowest priority number** (highest priority) runs first. Ties broken by task ID. Motor Control (priority 0) runs before Communication (priority 4).

## Task States

```
READY ──► RUNNING   (scheduler selects task)
RUNNING ──► READY   (time slice expires)
RUNNING ──► WAITING (blocked on mutex)
WAITING ──► READY   (mutex released)
Any ──► SUSPENDED   (explicitly suspended)
SUSPENDED ──► READY (explicitly resumed)
```

