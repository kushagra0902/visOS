#include "kernel/kernel.hpp"
#include "kernel/simulation_engine.hpp"
#include "core/program_metadata.hpp"
#include "core/syscall.hpp"
#include "util/logger.hpp"

#include <gtest/gtest.h>

using namespace visos;

class PriorityScenarioTest : public ::testing::Test {
protected:
    void SetUp() override { Logger::setEnabled(false); }

    ProgramMetadata makeProgram(const std::string& name, Tick cpuTime, int priority = 0) {
        ProgramMetadata m;
        m.program_name = name;
        m.estimated_cpu_time = cpuTime;
        return m;
    }
};

// High-priority process admitted while low-priority is running preempts it
// on the next quantum boundary.
TEST_F(PriorityScenarioTest, AllProcessesTerminate) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::Priority, 4});
    auto m1 = makeProgram("Low", 6);
    auto m2 = makeProgram("High", 4);

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));

    // Submit both with priorities
    kernel.runProgram(id1);
    kernel.runProgram(id2);

    kernel.allProcesses()[0]->setPriority(1);
    kernel.allProcesses()[1]->setPriority(5);

    SimulationEngine engine(kernel, {1, 200});
    engine.run();

    for (const auto& p : kernel.allProcesses()) {
        EXPECT_EQ(p->state(), ProcessState::Terminated);
    }
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 6);
    EXPECT_EQ(kernel.allProcesses()[1]->cpuTimeUsed(), 4);
}

TEST_F(PriorityScenarioTest, EqualPriorityFallsBackToFCFS) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::Priority, 100});
    auto m1 = makeProgram("P1", 3);
    auto m2 = makeProgram("P2", 3);

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));

    kernel.runProgram(id1);
    kernel.runProgram(id2);
    // Both have default priority 0

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    // Both must terminate
    for (const auto& p : kernel.allProcesses()) {
        EXPECT_EQ(p->state(), ProcessState::Terminated);
        EXPECT_EQ(p->cpuTimeUsed(), 3);
    }
}

TEST_F(PriorityScenarioTest, ThreeProcessesDifferentPriorities) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::Priority, 4});
    auto m1 = makeProgram("Low",  4);
    auto m2 = makeProgram("Med",  4);
    auto m3 = makeProgram("High", 4);

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));
    auto id3 = kernel.submitProgram(std::move(m3));

    kernel.runProgram(id1);
    kernel.runProgram(id2);
    kernel.runProgram(id3);

    kernel.allProcesses()[0]->setPriority(1);
    kernel.allProcesses()[1]->setPriority(5);
    kernel.allProcesses()[2]->setPriority(10);

    SimulationEngine engine(kernel, {1, 200});
    engine.run();

    for (const auto& p : kernel.allProcesses()) {
        EXPECT_EQ(p->state(), ProcessState::Terminated);
        EXPECT_EQ(p->cpuTimeUsed(), 4);
    }
}

TEST_F(PriorityScenarioTest, SingleProcessPriority) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::Priority, 4});
    auto m = makeProgram("Alone", 5);
    auto id = kernel.submitProgram(std::move(m));
    kernel.runProgram(id);
    kernel.allProcesses()[0]->setPriority(99);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 5);
}

TEST_F(PriorityScenarioTest, PriorityWithIOBlocking) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::Priority, 4});

    ProgramMetadata m1, m2;
    m1.program_name = "HighIO";
    m1.estimated_cpu_time = 6;
    m1.syscall_profile.addSyscall(2, Syscall{SyscallType::IoRead, 3});

    m2.program_name = "LowCPU";
    m2.estimated_cpu_time = 4;

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));

    kernel.runProgram(id1);
    kernel.runProgram(id2);

    kernel.allProcesses()[0]->setPriority(10);
    kernel.allProcesses()[1]->setPriority(1);

    SimulationEngine engine(kernel, {1, 200});
    engine.run();

    for (const auto& p : kernel.allProcesses()) {
        EXPECT_EQ(p->state(), ProcessState::Terminated);
    }
    // High-priority process must eventually complete its 6 CPU ticks
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 6);
    EXPECT_EQ(kernel.allProcesses()[1]->cpuTimeUsed(), 4);
}
