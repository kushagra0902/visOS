# VisOS

---

## 1. PROJECT OVERVIEW

### What Is the Project About?

**Simple terms:** VisOS is an educational software that simulates how an operating system manages multiple programs running at the same time. Think of it like a "virtual OS lab" — instead of needing a real operating system kernel, you watch processes being created, scheduled, paused, blocked for I/O, and terminated, all through a visual dashboard.

**Technical terms:** VisOS is a user-space, discrete-event operating system simulator that models the complete process lifecycle — including process scheduling (FCFS, Round Robin, Priority), state transitions (5-state process model), context switching, time quantum enforcement, and I/O blocking/wakeup — through a deterministic, event-driven architecture. It is implemented in C++20 with a CMake build system, features a full Qt6 GUI with Gantt charts, and is validated by 112 GoogleTest unit and integration tests.

### Problem Statement

Operating system internals — scheduling algorithms, process state machines, context switches — are difficult to learn from textbooks alone. Students and engineers need a way to **observe** these mechanisms in action, step through them tick-by-tick, and experiment with different scheduling policies without spinning up real kernels or virtual machines.

### Why This Project Exists (Real-World Need)

1. **Educational gap** — OS concepts are abstract; textbook diagrams don't show you what happens tick-by-tick when Round Robin preempts a process.
2. **Experimentation** — Engineers prototyping new scheduling algorithms need a sandbox where they can plug in a strategy and observe its effects.
3. **Reproducibility** — Real OS kernels are non-deterministic (hardware interrupts, multi-core races). VisOS guarantees identical runs for identical inputs, making it ideal for graded assignments and automated testing.
4. **Accessibility** — No root access, no kernel modules, no VMs. Runs as a regular user-space application.

### Key Objectives

| #   | Objective                                                                                      |
| --- | ---------------------------------------------------------------------------------------------- |
| 1   | Simulate the complete 5-state process lifecycle (New → Ready → Running → Blocked → Terminated) |
| 2   | Implement three scheduling policies: FCFS, Round Robin, Priority                               |
| 3   | Model I/O blocking and automatic wakeup using syscall profiles                                 |
| 4   | Ensure **deterministic simulation** — identical inputs always produce identical results        |
| 5   | Decouple the simulation engine from the UI using the Observer pattern                          |
| 6   | Provide a Qt6 GUI with live process table, Gantt chart, event log, and narrative report        |
| 7   | Achieve comprehensive test coverage with 112 automated tests                                   |
| 8   | Follow a strict 4-layer architecture with no circular dependencies                             |

---

## 2. CORE CONCEPTS & THEORY

### 2.1 Process and Process Control Block (PCB)

**Definition:** A process is a program in execution. Each process is represented by a **Process Control Block (PCB)** — a data structure that stores all the information the OS needs to manage that process.

**In VisOS**, the PCB (`ProcessControlBlock` class) contains:

- **PID** — unique process identifier (`int32_t`)
- **State** — current state from the 5-state model
- **Priority** — integer priority value (higher = more important)
- **CPU time used** — how many ticks the process has executed
- **Estimated CPU time** — total ticks needed to finish
- **Register Set** — program counter, stack pointer, 8 general-purpose registers (simulated)
- **Resource Descriptor** — memory required (MB), I/O devices required

**Key method:** `applyTransition(StateTransition)` — the ONLY way to change a process's state. It validates the transition at compile time using `constexpr` and throws `std::logic_error` if the transition is illegal. This prevents bugs like jumping directly from `NEW` to `RUNNING`.

### 2.2 Process States (5-State Model)

This is the classical 5-state process model from operating systems theory:

```
NEW ──[admit]──▶ READY ──[dispatch]──▶ RUNNING
                  ▲                      │  │  │
                  │                      │  │  │
          [io_complete]       [quantum   │  │  │[exit]
                  │            expired]  │  │  │
                  │                      │  │  ▼
               BLOCKED ◀──[blocking]─────┘  │  TERMINATED
                           [syscall]        │
                  ▲                          │
                  └──────────────────────────┘
```

| State          | Meaning                                                 |
| -------------- | ------------------------------------------------------- |
| **NEW**        | Process created but not yet admitted to the ready queue |
| **READY**      | In memory, waiting for CPU time                         |
| **RUNNING**    | Currently executing on the CPU                          |
| **BLOCKED**    | Waiting for I/O or some event to complete               |
| **TERMINATED** | Finished execution; awaiting cleanup                    |

**6 Valid Transitions:**

| Transition      | From → To            | Trigger                           |
| --------------- | -------------------- | --------------------------------- |
| Admit           | NEW → READY          | Process submitted to kernel       |
| Dispatch        | READY → RUNNING      | Scheduler selects process for CPU |
| QuantumExpired  | RUNNING → READY      | Time slice used up (RR/Priority)  |
| BlockingSyscall | RUNNING → BLOCKED    | Process requests I/O              |
| IoComplete      | BLOCKED → READY      | I/O operation finishes            |
| Exit            | RUNNING → TERMINATED | Process finishes all CPU work     |

**Why enforce transitions?** Allowing invalid transitions (e.g., `BLOCKED → RUNNING`) would corrupt the simulation. VisOS makes `isValidTransition()` a `constexpr` function — the compiler can evaluate it at compile time — and throws `std::logic_error` at runtime for any violation.

### 2.3 CPU Scheduling Algorithms

#### 2.3.1 FCFS (First-Come, First-Served)

- **Type:** Non-preemptive
- **How it works:** Processes are served in the order they arrive. Once a process gets the CPU, it runs until it finishes or blocks for I/O.
- **Quantum:** Effectively infinite (`INT_MAX`)
- **Advantage:** Simple, no starvation in terms of ordering
- **Disadvantage:** **Convoy effect** — short processes stuck behind a long one suffer high waiting time
- **In VisOS:** `FCFSStrategy::selectNext()` simply pops from the front of the ready queue deque. `defaultQuantum()` returns `std::numeric_limits<int>::max()` to prevent preemption.

#### 2.3.2 Round Robin (RR)

- **Type:** Preemptive
- **How it works:** Each process gets a fixed **time quantum** (default 4 ticks). When the quantum expires, the running process is moved back to the READY queue and the next process is dispatched.
- **Advantage:** Fair — every process gets equal CPU time slices
- **Disadvantage:** High context switch overhead if quantum is too small; behaves like FCFS if quantum is too large
- **In VisOS:** `RoundRobinStrategy` stores the `time_quantum_`. Selection is FIFO (front of deque), same as FCFS — the preemption is enforced by the CPU counting down `remaining_quantum_` and raising `QuantumExpired` when it hits zero.

#### 2.3.3 Priority Scheduling

- **Type:** Preemptive (with time quantum)
- **How it works:** The process with the highest priority value gets the CPU. If priorities are equal, FCFS order is used as a tiebreaker.
- **Advantage:** Critical processes get CPU first
- **Disadvantage:** **Starvation** — low-priority processes may never run (VisOS does NOT implement aging)
- **In VisOS:** `PriorityStrategy::selectNext()` uses `std::max_element` to find the process with the highest priority in the ready queue. The ready queue also uses `enqueueByPriority()` which inserts in sorted position.

### 2.4 Context Switching

**Definition:** Saving the state (registers, program counter, etc.) of the currently running process and restoring the state of the next process to be run.

**In VisOS:**

1. CPU raises a `KernelEvent` (QuantumExpired, SyscallBlock, ProcessExit)
2. Kernel calls `cpu_->unloadProcess()` — saves the PCB with its register set, resets CPU to Idle
3. Kernel calls `dispatchNext()` — selects next process via Scheduler, calls `cpu_->loadProcess()` with the new PCB, its syscall profile, and the time quantum
4. Register state is simulated: program counter increments by 1 each tick

### 2.5 Time Quantum

**Definition:** The maximum number of CPU ticks a process can run before being preempted (for preemptive schedulers).

- **FCFS:** Quantum = `INT_MAX` (no preemption)
- **Round Robin:** Quantum = configurable (default 4)
- **Priority:** Quantum = configurable (default 4)

In the CPU's `executeTick()`, `remaining_quantum_` is decremented each tick. When it hits 0, the `QuantumExpired` event fires.

### 2.6 I/O Blocking and Wakeup

**How blocking works:**

1. Each program carries a `SyscallProfile` — a `std::map<Tick, Syscall>` mapping relative CPU ticks to syscalls
2. During `CPU::executeTick()`, after incrementing CPU time, the CPU checks if a syscall is registered at the current CPU time
3. If yes, it raises `SyscallBlock`
4. The Kernel transitions the process to BLOCKED, calculates `wakeup_tick = current_tick + syscall.duration`, and stores it in the blocked queue

**How wakeup works:**

1. At the **start of each tick** (before CPU execution), `Kernel::checkBlockedCompletions()` is called
2. It calls `blocked_queue_.getCompleted(current_tick)` which uses `std::partition` to separate completed from pending entries
3. Completed processes are transitioned BLOCKED → READY and re-enqueued

**Key insight:** Wakeup uses **absolute ticks**, not timers or countdowns. This guarantees determinism.

### 2.7 Deterministic Simulation

**Why it matters:** If the same inputs produce different results each time, you can't grade students or debug scheduling algorithms.

**How VisOS achieves determinism:**

- `SyscallProfile` maps exact CPU ticks to syscalls — no randomness
- No threads or concurrency in the simulation loop
- Processing order within each tick is fixed: `checkBlockedCompletions() → CPU.executeTick() → notifyTickCompleted()`
- The blocked queue's `getCompleted()` uses `std::partition`, which preserves relative order of non-completed elements

### 2.8 Observer Pattern

**Definition:** A design pattern where an object (the subject) maintains a list of dependents (observers) and notifies them automatically of any state changes.

**In VisOS:**

- **Subject:** `Kernel` class
- **Observer interface:** `IKernelObserver` (pure virtual class) with callbacks: `onTickCompleted`, `onProcessCreated`, `onStateTransition`, `onContextSwitch`, `onSchedulerInvoked`, `onSimulationComplete`
- **Concrete observer (GUI):** `KernelObserverAdapter` — a QObject that implements `IKernelObserver` and emits Qt signals. These signals drive the Qt models (ProcessTableModel, QueueModel, GanttModel).

**Why:** The simulation engine has zero knowledge of Qt or GUIs. Any frontend (CLI, web, testing harness) can observe the kernel.

### 2.9 Strategy Pattern

**Definition:** A design pattern that enables selecting an algorithm at runtime by encapsulating each algorithm into a separate class.

**In VisOS:**

- **Interface:** `ISchedulingStrategy` with `selectNext(deque&)` and `defaultQuantum()`
- **Concrete strategies:** `FCFSStrategy`, `RoundRobinStrategy`, `PriorityStrategy`
- **Client:** `Scheduler` holds a `std::unique_ptr<ISchedulingStrategy>` and delegates `selectNext()` to the active strategy
- **Factory:** `Scheduler::createStrategy()` — a static method that creates the right strategy from the `SchedulingPolicy` enum

### 2.10 Event-Driven Architecture

VisOS does NOT use a traditional game loop or polling architecture. Instead:

1. `SimulationEngine` calls `ClockService::tick()`
2. `ClockService` increments the clock and fires a callback to `Kernel::notifyTick()`
3. The Kernel processes events synchronously within that single tick
4. Events flow upward from CPU → Kernel → Observers

This is the **Hollywood Principle** ("don't call us, we'll call you") — the CPU doesn't call Kernel methods directly; it raises events via a `std::function` callback.

### 2.11 Model/View Pattern (Qt)

Qt's Model/View separates data storage (models) from presentation (views):

- `ProcessTableModel` (QAbstractTableModel) stores process rows
- `QueueModel` (QAbstractListModel) stores queue contents
- `GanttModel` stores state-segment history
- Views (`QTableView`, `QListView`, custom `GanttWidget`) render the data

When the model changes (via signals from the `KernelObserverAdapter`), the view automatically updates.

---

## 3. ARCHITECTURE & WORKFLOW

### 3.1 Four-Layer Architecture

```
┌─────────────────────────────────────────────────┐
│  Layer 4: APP (CLI / Qt6 GUI)                   │
│  Configures kernel, submits programs, runs loop │
└──────────────────────┬──────────────────────────┘
                       │ uses
┌──────────────────────▼──────────────────────────┐
│  Layer 3: KERNEL (Orchestration)                │
│  Kernel: central event dispatcher               │
│  SimulationEngine: run/step loop + ClockService │
└──────────────────────┬──────────────────────────┘
                       │ owns & coordinates
┌──────────────────────▼──────────────────────────┐
│  Layer 2: SERVICES (Stateful Components)        │
│  CPU, Scheduler, MemoryManager, SyscallHandler, │
│  ReadyQueue, BlockedQueue, ProgramStore,        │
│  ClockService, TimerInterrupt                   │
└──────────────────────┬──────────────────────────┘
                       │ operates on
┌──────────────────────▼──────────────────────────┐
│  Layer 1: CORE (Domain Types, header-only)      │
│  PCB, ProcessState, KernelEvent, SyscallProfile,│
│  RegisterSet, ProgramMetadata, types.hpp        │
└─────────────────────────────────────────────────┘
```

**Strict dependency rule:** Each layer depends only on the layers below it, NEVER upward. This prevents circular dependencies and makes each layer independently testable.

### 3.2 Step-by-Step Workflow (Per Tick)

1. **User submits programs:** `kernel.submitProgram(metadata)` → stores in `ProgramStore`, returns a `ProgramID`
2. **User runs programs:** `kernel.runProgram(id)` → creates a PCB, transitions NEW → READY, enqueues in ready queue
3. **Simulation starts:** `SimulationEngine::run()` or `step()`
4. **Each tick:**
   ```
   ClockService.tick()                         // increment clock
     → Kernel.notifyTick(current_tick)
       → checkBlockedCompletions()             // scan blocked queue, wake completed I/O processes (BLOCKED → READY)
       → CPU.executeTick()                     // advance 1 tick of the current process
         → increment CPU time used
         → decrement remaining quantum
         → update registers (PC++)
         → check for syscall at this CPU tick  → SyscallBlock event
         → check if process finished           → ProcessExit event
         → check if quantum expired            → QuantumExpired event
         → if CPU is idle                      → CpuIdle event
       → handleEvent(event)                    // Kernel reacts:
         → SyscallBlock:    unload, RUNNING→BLOCKED, move to blocked queue, dispatchNext()
         → QuantumExpired:  unload, RUNNING→READY, re-enqueue, dispatchNext()
         → ProcessExit:     unload, RUNNING→TERMINATED, check if all done
         → CpuIdle:         try dispatchNext()
       → notifyTickCompleted()                 // all IKernelObserver callbacks fire
   ```
5. **Simulation ends:** When all processes are TERMINATED or `max_ticks` is reached

### 3.3 Data Flow

```
Input:                           Processing:                    Output:
─────                           ──────────                     ──────
ProgramMetadata ──submitProgram──▶ ProgramStore                CLI: tick-by-tick log
  ├─ name                        │                             + summary
  ├─ cpu_time                    ▼
  ├─ resources        runProgram──▶ Create PCB                 GUI:
  └─ syscall_profile              │ NEW→READY                  ├─ Process table
                                  │ enqueue                    ├─ Ready/Blocked queues
                                  ▼                            ├─ Gantt chart
                      SimulationEngine.run()                   ├─ Narrative report
                           │                                   ├─ Event log
                      ClockService.tick()                      └─ Process detail dialog
                           │
                      Kernel.notifyTick()
                           │
                      CPU.executeTick() → Events → Handler → State changes
                           │
                      IKernelObserver notifications
```

### 3.4 Module Roles

| Module                                          | Role                                                                   |
| ----------------------------------------------- | ---------------------------------------------------------------------- |
| **ProcessControlBlock**                         | Stores process state; enforces valid transitions                       |
| **CPU**                                         | Executes one tick; raises events via callback                          |
| **Scheduler**                                   | Delegates to the active `ISchedulingStrategy` to pick the next process |
| **MemoryManager**                               | Orchestrates ready queue and blocked queue                             |
| **ReadyQueue**                                  | FIFO deque with optional priority insertion                            |
| **BlockedQueue**                                | Stores (PCB, wakeup_tick) pairs; returns completed entries             |
| **SyscallHandler**                              | Maps syscalls to kernel events (currently all blocking)                |
| **ClockService**                                | Increments the simulation clock; fires tick callback                   |
| **ProgramStore**                                | Stores program metadata by ID for later retrieval                      |
| **PidGenerator**                                | Generates monotonically increasing unique PIDs                         |
| **Kernel**                                      | Central event dispatcher; owns all services; coordinates everything    |
| **SimulationEngine**                            | Drives the clock; provides `run()` and `step()`                        |
| **Logger**                                      | Console logging with enable/disable toggle                             |
| **KernelObserverAdapter**                       | Bridges Kernel observer callbacks to Qt signals                        |
| **ProcessTableModel / QueueModel / GanttModel** | Qt data models                                                         |
| **Widgets**                                     | Qt views: process table, queues, Gantt chart, controls, event log      |

---

## 4. CODE/IMPLEMENTATION BREAKDOWN

### 4.1 Core Layer

#### `types.hpp`

- Defines type aliases: `PID = int32_t`, `ProgramID = int32_t`, `Tick = int64_t`
- **Why `int64_t` for Tick?** Prevents overflow even for extremely long simulations
- **Why type aliases?** Enables changing the underlying type in one place; improves readability

#### `ProcessState` (enum class)

- 5 values: `New`, `Ready`, `Running`, `Blocked`, `Terminated`
- Has a `to_string()` function returning `std::string_view` (zero allocation)
- **`enum class` vs plain `enum`:** `enum class` is type-safe, prevents implicit conversion to int

#### `ProcessControlBlock`

- **`applyTransition()`:** The single mutation point for state. Calls `isValidTransition()` then `targetState()` to compute the new state.
- **`isValidTransition()` is `constexpr`:** The compiler can evaluate it at compile time, enabling static analysis. At runtime, if it returns `false`, a `std::logic_error` is thrown with a detailed message.
- **`isFinished()`:** Returns `cpu_time_used_ >= estimated_cpu_time_` — the exit condition.
- **Alternative considered:** Using a state machine library (e.g., Boost.SML). Rejected because `constexpr` switch is simpler, zero-dependency, and easier to test.

#### `SyscallProfile`

- `std::map<Tick, Syscall>` — maps a **relative CPU tick** to a syscall
- `getSyscallAt(tick)` returns `std::optional<Syscall>` — elegant null handling without pointers
- **Why `std::map` instead of `std::unordered_map`?** Syscalls are rarely more than 2–3 per process; `std::map` preserves ordering and is simpler for debugging.

#### `RegisterSet`

- Simulated register state: `program_counter`, `stack_pointer`, 8 general-purpose registers
- Default-initialized to zero; uses `= default` for comparison operator (C++20 feature)
- **Purpose:** Demonstrates context switching — saving/restoring registers when swapping processes

### 4.2 Services Layer

#### `CPU`

- **Constructor takes `EventCallback`** — a `std::function<void(KernelEvent, PCB*)>`. This is the Callback pattern / Inversion of Control.
- **`executeTick()`:** The heart of the simulation. Order of checks matters:
  1. If idle → raise `CpuIdle`
  2. Increment CPU time, decrement quantum, advance PC
  3. Check for syscall → `SyscallBlock` (priority: blocking I/O must be handled before exit)
  4. Check if finished → `ProcessExit`
  5. Check quantum → `QuantumExpired`
  6. No event → continue next tick
- **`loadProcess()` / `unloadProcess()`:** Context switch primitives. `unloadProcess()` clears all state and returns the PCB.
- **Time complexity:** `executeTick()` is O(log n) for the syscall lookup (`std::map::find`), everything else is O(1).

#### `Scheduler`

- **`selectNextProcess()`:** Accesses the ready queue's internal deque via `const_cast` (necessary because the strategy needs a mutable reference to remove the selected process).
- **`createStrategy()`:** Factory method with a switch on `SchedulingPolicy`.
- **Design note on `const_cast`:** The `ReadyQueue` only exposes a `const` reference to its deque for safety. The Scheduler needs write access for the Strategy to remove the selected process. Alternative: make the ready queue provide a `selectAndRemove()` method.

#### `FCFSStrategy`

- `selectNext()`: `pop_front()` — takes the first process in the queue
- `defaultQuantum()`: Returns `INT_MAX` — effectively no preemption
- **Time complexity:** O(1)

#### `RoundRobinStrategy`

- `selectNext()`: Same as FCFS (`pop_front()`), because the preemption is handled by the CPU countdown, not the selection logic
- `defaultQuantum()`: Returns the configured `time_quantum_`
- **Time complexity:** O(1)

#### `PriorityStrategy`

- `selectNext()`: Uses `std::max_element` to find the highest-priority process, erases it from the deque
- `defaultQuantum()`: Returns the configured `time_quantum_` (priority scheduling is still preemptive in VisOS)
- **Time complexity:** O(n) where n is the number of ready processes

#### `ReadyQueue`

- `std::deque<shared_ptr<PCB>>` — allows efficient front/back operations
- `enqueueByPriority()`: Inserts in sorted order using `std::find_if` — O(n)
- **Why `std::deque` not `std::priority_queue`?** Strategies need to iterate and select arbitrarily; `std::priority_queue` only gives access to the top element.

#### `BlockedQueue`

- `std::vector<BlockedEntry>` where `BlockedEntry = {PCB, wakeup_tick}`
- `getCompleted(tick)`: Uses `std::partition` to split completed vs. pending. Moves completed entries, erases them from the vector.
- **Time complexity:** O(n) per tick where n is the number of blocked processes
- **Why `std::partition` instead of a sorted structure?** Blocked queue is typically small (1–3 entries). Sorting overhead is unnecessary.

#### `MemoryManager`

- Facade over `ReadyQueue` and `BlockedQueue`
- Provides a unified interface: `enqueueReady()`, `moveToBlocked()`, `checkBlockedCompletions()`
- **Note:** Despite the name, it does NOT simulate memory allocation, paging, or fragmentation. It manages process queues.

#### `ClockService`

- Maintains `current_time_` (Tick)
- `tick()` increments by `tick_interval` (default 1) and invokes the callback
- **Purpose:** Decouples the timing mechanism from the kernel. The kernel doesn't know or care about how ticks are generated.

### 4.3 Kernel Layer

#### `Kernel`

- **Central event dispatcher** — NOT a loop. Called externally by `SimulationEngine`.
- **`notifyTick()`:** Called once per tick. Steps: (1) store current tick, (2) wake up blocked processes, (3) execute CPU tick, (4) notify observers.
- **`handleEvent()`:** Switch on `KernelEvent` type. Delegates to `handleSyscallBlock()`, `handleQuantumExpired()`, `handleProcessExit()`, `handleCpuIdle()`.
- **`dispatchNext()`:** Calls `scheduler_->selectNextProcess()`. If a process is available, transitions it READY → RUNNING and loads it onto the CPU.
- **`transitionState()`:** Wraps `pcb.applyTransition()` with logging and observer notification.
- **Observer registration:** `addObserver(shared_ptr<IKernelObserver>)` — uses `shared_ptr` to ensure lifetime management.
- **Owns all services:** `MemoryManager`, `Scheduler`, `CPU`, `SyscallHandler`, `ProgramStore`, `PidGenerator`.

#### `SimulationEngine`

- **`run()`:** While loop: `running_ && hasActiveProcesses() && tick_count_ < max_ticks` → `clock_->tick()`
- **`step()`:** Single tick: `if (hasActiveProcesses) clock_->tick()`
- **`max_ticks` safety:** Prevents infinite loops if a bug creates a deadlock
- **`std::atomic<bool> running_`:** Thread-safe stop flag (though currently single-threaded)

### 4.4 GUI Layer

#### `KernelObserverAdapter`

- Implements `IKernelObserver`, inherits `QObject`
- Converts kernel callbacks to Qt signals (`tickCompleted(Tick)`, `processCreated(...)`, etc.)
- **Bridge pattern** — adapts a non-Qt interface to the Qt signal/slot mechanism

#### Models

- `ProcessTableModel`: `QAbstractTableModel` — columns: PID, Name, State, Priority, CPU Used, Estimated CPU, Memory, I/O
- `QueueModel`: `QAbstractListModel` — shows PIDs with priority or wakeup tick
- `GanttModel`: Custom model — stores state segments per process for the Gantt chart and generates narrative reports

#### Widgets

- `MainWindow`: Top-level widget with tabs, controls, and event log
- `SimulationControlsWidget`: Step/Play/Pause/Reset/Restart buttons, policy dropdown, quantum spinner
- `ProcessTableWidget`: Color-coded table with "ⓘ" info column
- `GanttWidget`: Custom `QPainter` rendering of Gantt chart
- `ProcessDetailDialog`: Pop-up showing PCB snapshot and full state history
- `ProgramSubmissionDialog`: Form for adding custom programs with presets

### 4.5 Utility Layer

#### `Logger`

- Static class with `enabled_` flag
- All logging goes to `std::cout`
- **Why static?** Global logging facility; avoids threading issues with a single boolean guard

#### `PidGenerator`

- `next()` returns `next_pid_++` — monotonically increasing
- `reset()` resets to 1 — used for simulation restart
- **Why not random UUIDs?** Sequential PIDs are easier to read in logs and match real OS behavior

---

## 5. TOOLS & TECHNOLOGIES

### 5.1 C++20

**Why chosen:** Modern features like `constexpr`, `std::optional`, `std::string_view`, `enum class`, defaulted comparison operators, and `[[nodiscard]]` make code safer and more expressive.

| Feature Used                     | Purpose                                               |
| -------------------------------- | ----------------------------------------------------- |
| `constexpr`                      | Compile-time state transition validation              |
| `std::optional`                  | Null-safe syscall lookup                              |
| `std::string_view`               | Zero-copy string operations in `to_string()`          |
| `std::shared_ptr` / `unique_ptr` | Automatic memory management                           |
| `std::function`                  | Type-erased callbacks (CPU → Kernel)                  |
| `enum class`                     | Type-safe enumerations                                |
| `[[nodiscard]]`                  | Compiler warning if return value ignored              |
| `[[maybe_unused]]`               | Suppress warnings for intentionally unused parameters |
| `= default` comparison           | Auto-generated `operator==` for structs               |

**Alternatives:** C, Java, Python.

- C lacks RAII, templates, and strong typing → error-prone for this scale
- Java would work but lacks `constexpr` compile-time evaluation and has GC overhead
- Python would be too slow for 10-process stress tests and lacks static typing

### 5.2 CMake (≥ 3.20)

**Why:** Industry-standard for C++ projects. Supports cross-platform builds, `FetchContent` for dependency management, and fine-grained target configuration.

**Key features used:**

- `FetchContent` for GoogleTest auto-download
- `option()` for conditional build targets (GUI, CLI, tests)
- `target_link_libraries` with proper scoping
- `CMAKE_CXX_STANDARD 20` enforcement

**Alternative:** Meson, Bazel, Make. CMake was chosen because it's the most widely supported in the C++ ecosystem.

### 5.3 Qt6 (≥ 6.7)

**Why:** Mature, cross-platform GUI framework with excellent Model/View architecture.

**Key components used:**

- `QAbstractTableModel` / `QAbstractListModel` — data models
- `QTableView` / `QListView` — data views
- `QPainter` — custom Gantt chart rendering
- `QTimer` — auto-play tick advancement
- Signal/Slot mechanism — loose coupling between components

**Alternatives:** Dear ImGui (simpler but no Model/View), Electron (cross-language overhead), GTK (less mature on macOS/Windows).

### 5.4 GoogleTest v1.14.0

**Why:** De facto standard for C++ unit testing. Supports test fixtures, parameterized tests, and death tests.

**Key features used:**

- `TEST_F` — test fixtures with `SetUp()`/`TearDown()`
- `EXPECT_EQ`, `EXPECT_THROW` — assertion macros
- `--gtest_filter` — selective test execution

**Alternative:** Catch2. GoogleTest was chosen because it's more widely known and auto-fetched via CMake `FetchContent`.

### 5.5 Summary Table

| Tool       | Version     | Purpose         | Alternative     |
| ---------- | ----------- | --------------- | --------------- |
| C++        | C++20       | Core language   | C, Java, Python |
| CMake      | ≥ 3.20      | Build system    | Meson, Bazel    |
| Qt6        | ≥ 6.7       | GUI framework   | ImGui, GTK      |
| GoogleTest | 1.14.0      | Unit testing    | Catch2, doctest |
| GCC/Clang  | ≥ 11 / ≥ 14 | Compiler        | MSVC            |
| Git        | Any         | Version control | —               |

---

## 6. RESULTS & OUTPUT

### 6.1 CLI Output

Running `./visos_cli --policy rr --quantum 4` with the three demo programs produces tick-by-tick output:

```
VisOS Simulation
Policy: ROUND_ROBIN | Quantum: 4

[TICK 1]
  [STATE] PID 1: NEW -> READY
  [STATE] PID 2: NEW -> READY
  [STATE] PID 3: NEW -> READY
  [STATE] PID 1: READY -> RUNNING
[TICK 2]
  ...
========== SIMULATION SUMMARY ==========
PID 1 | State: TERMINATED | CPU Time Used: 5 | Priority: 0
PID 2 | State: TERMINATED | CPU Time Used: 8 | Priority: 0
PID 3 | State: TERMINATED | CPU Time Used: 12 | Priority: 0
Total ticks: 30
=========================================
```

### 6.2 GUI Output

- **Process Table:** Color-coded rows (green=RUNNING, yellow=READY, red=BLOCKED, gray=TERMINATED)
- **Gantt Chart:** Horizontal bars per process, color-coded by state, with tick numbers on the X-axis
- **Narrative Report:** Auto-generated English text describing scheduling decisions
- **Event Log:** Tick-by-tick state transitions and scheduler invocations
- **Queue Panels:** Live ready/blocked queue contents with priority/wakeup tick info

### 6.3 How to Interpret Results

- **Total ticks** = makespan = total simulation time
- **CPU utilization** can be inferred: if CPU was idle for N ticks out of T total, utilization = (T-N)/T
- **Waiting time** = time a process spends in READY state
- **Turnaround time** = time from process creation to termination
- **Lower total ticks** generally means better scheduling for a given workload
- **FCFS** will typically have higher total ticks than RR for mixed workloads due to the convoy effect

### 6.4 Test Results

All **112 tests pass** across 18 test suites covering:

- State machine correctness (11 + 16 tests)
- CPU event generation (8 tests)
- Scheduling strategy selection (4 + 5 tests)
- Queue operations (5 tests)
- Kernel dispatch logic (5 tests)
- Observer callbacks (8 tests)
- Edge cases and stress tests (12 tests)
- Integration scenarios (20 tests)

---

## 7. LIMITATIONS

| Limitation                                                                                                                                                    | Impact                                                             |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| **I/O is a placeholder** — modelled by wakeup ticks, not actual device queues or transfer rates                                                               | Cannot study I/O scheduling algorithms (SCAN, C-LOOK, etc.)        |
| **No memory allocation simulation** — `ResourceDescriptor` records requirements but no paging, segmentation, or fragmentation                                 | Cannot study memory management concepts                            |
| **Single-core only** — one CPU                                                                                                                                | Cannot study multi-core scheduling (SMP, affinity, load balancing) |
| **No aging in Priority scheduling** — low-priority processes can starve forever                                                                               | Unrealistic for production OS study                                |
| **Priority not settable via ProgramMetadata** — must call `setPriority()` post-creation                                                                       | Awkward API for the GUI submission dialog                          |
| **Context switch notification is a no-op** — `onContextSwitch` exists but the kernel unloads old PID before dispatching, so old_pid is always 0               | GUI cannot display context switch pairs accurately                 |
| **No preemptive priority** — a higher-priority process arriving while a lower one is running doesn't immediately preempt it until the current quantum expires | Not true preemptive priority                                       |
| **Single-threaded GUI** — simulation runs on the GUI thread                                                                                                   | Very large simulations (1000s of processes) could freeze the UI    |
| **`const_cast` in Scheduler** — breaks const-correctness to give Strategy write access to the queue                                                           | Code smell; should be refactored                                   |

---

## 8. FUTURE IMPROVEMENTS

### 8.1 Realistic Enhancements

- **Aging for Priority scheduling** — increment priority of waiting processes over time to prevent starvation
- **Shortest Job First (SJF) / Shortest Remaining Time First (SRTF)** — add new scheduling strategies
- **Multi-level Feedback Queue (MLFQ)** — combine multiple queues with different priorities and time quanta
- **Preemptive priority dispatch** — immediately preempt a lower-priority running process when a higher-priority one arrives

### 8.2 Scalability Ideas

- **Multi-core simulation** — multiple CPU instances with load balancing
- **Worker thread model for GUI** — run simulation on a background thread to keep GUI responsive during stress tests
- **Configuration via JSON/YAML files** — define programs and policies in config files instead of code

### 8.3 Industry-Level Improvements

- **Memory simulation** — paging, page replacement (LRU, FIFO, Optimal), fragmentation visualization
- **File system simulation** — disk scheduling algorithms (SCAN, C-LOOK, SSTF)
- **Network I/O simulation** — model network latency and bandwidth
- **Performance metrics dashboard** — CPU utilization, throughput, average waiting time, average turnaround time as real-time graphs
- **Process communication (IPC)** — shared memory, message passing, semaphores
- **Deadlock detection** — resource allocation graphs, Banker's algorithm
- **Plugin API** — allow users to write custom scheduling strategies as shared libraries loaded at runtime

---
