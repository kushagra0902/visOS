#include "kernel/kernel.hpp"
#include "kernel/simulation_engine.hpp"
#include "core/program_metadata.hpp"
#include "core/syscall_profile.hpp"
#include "core/syscall.hpp"
#include "core/scheduling_policy.hpp"
#include "util/logger.hpp"

#include <iostream>
#include <string>

using namespace visos;

namespace {

void printUsage() {
    std::cout << "VisOS - Interactive Operating System Simulator\n"
              << "================================================\n\n"
              << "Usage: visos_cli [options]\n"
              << "  --policy <fcfs|rr|priority>  Scheduling policy (default: rr)\n"
              << "  --quantum <int>              Time quantum for RR/Priority (default: 4)\n"
              << "  --quiet                      Disable tick-by-tick logging\n"
              << "  --help                       Show this help\n\n";
}

void printSummary(const Kernel& kernel) {
    std::cout << "\n========== SIMULATION SUMMARY ==========\n";
    for (const auto& pcb : kernel.allProcesses()) {
        std::cout << "PID " << pcb->pid()
                  << " | State: " << to_string(pcb->state())
                  << " | CPU Time Used: " << pcb->cpuTimeUsed()
                  << " | Priority: " << pcb->priority()
                  << "\n";
    }
    std::cout << "Total ticks: " << kernel.currentTick() << "\n";
    std::cout << "=========================================\n";
}

SchedulingPolicy parsePolicy(const std::string& s) {
    if (s == "fcfs") return SchedulingPolicy::FCFS;
    if (s == "rr") return SchedulingPolicy::RoundRobin;
    if (s == "priority") return SchedulingPolicy::Priority;
    std::cerr << "Unknown policy: " << s << ". Using Round Robin.\n";
    return SchedulingPolicy::RoundRobin;
}

} // namespace

int main(int argc, char* argv[]) {
    SchedulingPolicy policy = SchedulingPolicy::RoundRobin;
    int quantum = 4;
    bool quiet = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "--policy" && i + 1 < argc) {
            policy = parsePolicy(argv[++i]);
        } else if (arg == "--quantum" && i + 1 < argc) {
            quantum = std::stoi(argv[++i]);
        } else if (arg == "--quiet") {
            quiet = true;
        }
    }

    Logger::setEnabled(!quiet);

    std::cout << "VisOS Simulation\n";
    std::cout << "Policy: " << to_string(policy)
              << " | Quantum: " << quantum << "\n\n";

    // Configure and create kernel
    Kernel::Config config{policy, quantum};
    Kernel kernel(config);

    // --- Demo: Submit sample programs ---

    // Program 1: Short CPU-bound process (5 ticks, no syscalls)
    ProgramMetadata prog1;
    prog1.program_name = "Calculator";
    prog1.estimated_cpu_time = 5;
    prog1.required_resources = {64, 0};

    // Program 2: Medium process with a blocking I/O at tick 3
    ProgramMetadata prog2;
    prog2.program_name = "FileReader";
    prog2.estimated_cpu_time = 8;
    prog2.required_resources = {128, 1};
    prog2.syscall_profile.addSyscall(3, Syscall{SyscallType::IoRead, 4});

    // Program 3: Long process with two blocking calls
    ProgramMetadata prog3;
    prog3.program_name = "WebServer";
    prog3.estimated_cpu_time = 12;
    prog3.required_resources = {256, 2};
    prog3.syscall_profile.addSyscall(4, Syscall{SyscallType::IoRead, 3});
    prog3.syscall_profile.addSyscall(9, Syscall{SyscallType::IoWrite, 2});

    // Submit and run all programs
    auto id1 = kernel.submitProgram(std::move(prog1));
    auto id2 = kernel.submitProgram(std::move(prog2));
    auto id3 = kernel.submitProgram(std::move(prog3));

    kernel.runProgram(id1);
    kernel.runProgram(id2);
    kernel.runProgram(id3);

    // Run the simulation
    SimulationEngine::Config engine_config;
    engine_config.max_ticks = 200;
    SimulationEngine engine(kernel, engine_config);
    engine.run();

    // Print summary
    printSummary(kernel);

    return 0;
}
