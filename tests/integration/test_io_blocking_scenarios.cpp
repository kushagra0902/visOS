#include "kernel/kernel.hpp"
#include "kernel/simulation_engine.hpp"
#include "core/program_metadata.hpp"
#include "core/syscall.hpp"
#include "core/syscall_profile.hpp"
#include "util/logger.hpp"

#include <gtest/gtest.h>

using namespace visos;

class IOBlockingTest : public ::testing::Test {
protected:
    void SetUp() override { Logger::setEnabled(false); }
};

// Process that blocks on I/O must eventually reach TERMINATED
TEST_F(IOBlockingTest, SingleProcessWithOneIOBlock) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata meta;
    meta.program_name = "IOProc";
    meta.estimated_cpu_time = 5;
    meta.syscall_profile.addSyscall(2, Syscall{SyscallType::IoRead, 4});

    auto id = kernel.submitProgram(std::move(meta));
    kernel.runProgram(id);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    const auto& pcb = kernel.allProcesses()[0];
    EXPECT_EQ(pcb->state(), ProcessState::Terminated);
    EXPECT_EQ(pcb->cpuTimeUsed(), 5);
}

// Process with two sequential I/O blocks
TEST_F(IOBlockingTest, SingleProcessWithTwoIOBlocks) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata meta;
    meta.program_name = "DoubleIO";
    meta.estimated_cpu_time = 8;
    meta.syscall_profile.addSyscall(2, Syscall{SyscallType::IoRead,  3});
    meta.syscall_profile.addSyscall(5, Syscall{SyscallType::IoWrite, 2});

    auto id = kernel.submitProgram(std::move(meta));
    kernel.runProgram(id);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    const auto& pcb = kernel.allProcesses()[0];
    EXPECT_EQ(pcb->state(), ProcessState::Terminated);
    EXPECT_EQ(pcb->cpuTimeUsed(), 8);
}

// CPU-bound companion runs during the I/O block of the first process
TEST_F(IOBlockingTest, IOBlockAllowsOtherProcessesToRun) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata m1, m2;
    m1.program_name = "IOProc";
    m1.estimated_cpu_time = 6;
    m1.syscall_profile.addSyscall(2, Syscall{SyscallType::IoRead, 4});

    m2.program_name = "CPUProc";
    m2.estimated_cpu_time = 3;

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));

    kernel.runProgram(id1);
    kernel.runProgram(id2);

    SimulationEngine engine(kernel, {1, 200});
    engine.run();

    for (const auto& p : kernel.allProcesses()) {
        EXPECT_EQ(p->state(), ProcessState::Terminated);
    }
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 6);
    EXPECT_EQ(kernel.allProcesses()[1]->cpuTimeUsed(), 3);
}

// Three-process mix: one pure CPU, one single I/O, one double I/O
TEST_F(IOBlockingTest, ThreeProcessMixedWorkload) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::RoundRobin, 3});

    ProgramMetadata m1, m2, m3;
    m1.program_name = "CPU";
    m1.estimated_cpu_time = 5;

    m2.program_name = "SingleIO";
    m2.estimated_cpu_time = 8;
    m2.syscall_profile.addSyscall(3, Syscall{SyscallType::IoRead, 4});

    m3.program_name = "DoubleIO";
    m3.estimated_cpu_time = 10;
    m3.syscall_profile.addSyscall(2, Syscall{SyscallType::IoRead,  3});
    m3.syscall_profile.addSyscall(7, Syscall{SyscallType::IoWrite, 2});

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));
    auto id3 = kernel.submitProgram(std::move(m3));

    kernel.runProgram(id1);
    kernel.runProgram(id2);
    kernel.runProgram(id3);

    SimulationEngine engine(kernel, {1, 500});
    engine.run();

    for (const auto& p : kernel.allProcesses()) {
        EXPECT_EQ(p->state(), ProcessState::Terminated)
            << "Process " << p->pid() << " did not terminate";
    }

    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 5);
    EXPECT_EQ(kernel.allProcesses()[1]->cpuTimeUsed(), 8);
    EXPECT_EQ(kernel.allProcesses()[2]->cpuTimeUsed(), 10);
}

// I/O duration longer than remaining CPU time — process must still terminate
TEST_F(IOBlockingTest, LongIODurationDoesNotPreventTermination) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata meta;
    meta.program_name = "LongIO";
    meta.estimated_cpu_time = 4;
    meta.syscall_profile.addSyscall(1, Syscall{SyscallType::IoRead, 20});

    auto id = kernel.submitProgram(std::move(meta));
    kernel.runProgram(id);

    SimulationEngine engine(kernel, {1, 200});
    engine.run();

    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 4);
}

// IoWrite syscall is handled identically to IoRead
TEST_F(IOBlockingTest, IoWriteSyscallBlocks) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata meta;
    meta.program_name = "WriteProc";
    meta.estimated_cpu_time = 5;
    meta.syscall_profile.addSyscall(2, Syscall{SyscallType::IoWrite, 3});

    auto id = kernel.submitProgram(std::move(meta));
    kernel.runProgram(id);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 5);
}

// Verify determinism: two identical I/O scenarios produce the same tick count
TEST_F(IOBlockingTest, IOScenarioDeterminism) {
    auto runOnce = []() -> Tick {
        Logger::setEnabled(false);
        Kernel kernel(Kernel::Config{SchedulingPolicy::RoundRobin, 3});

        ProgramMetadata m1, m2;
        m1.program_name = "P1";
        m1.estimated_cpu_time = 6;
        m1.syscall_profile.addSyscall(2, Syscall{SyscallType::IoRead, 3});

        m2.program_name = "P2";
        m2.estimated_cpu_time = 4;

        kernel.runProgram(kernel.submitProgram(std::move(m1)));
        kernel.runProgram(kernel.submitProgram(std::move(m2)));

        SimulationEngine engine(kernel, {1, 200});
        engine.run();
        return kernel.currentTick();
    };

    EXPECT_EQ(runOnce(), runOnce());
}
