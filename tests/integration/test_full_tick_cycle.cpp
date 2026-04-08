#include "kernel/kernel.hpp"
#include "kernel/simulation_engine.hpp"
#include "core/program_metadata.hpp"
#include "core/syscall_profile.hpp"
#include "util/logger.hpp"

#include <gtest/gtest.h>

using namespace visos;

class FullTickCycleTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::setEnabled(false);
    }
};

TEST_F(FullTickCycleTest, ProcessGoesFromNewToTerminated) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata meta;
    meta.program_name = "SimpleProg";
    meta.estimated_cpu_time = 3;

    auto id = kernel.submitProgram(std::move(meta));
    kernel.runProgram(id);

    // Process should be in READY state after runProgram
    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Ready);

    SimulationEngine engine(kernel, {1, 50});
    engine.run();

    // After completion, should be TERMINATED
    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 3);
}

TEST_F(FullTickCycleTest, BlockingAndWakeup) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata meta;
    meta.program_name = "IOProg";
    meta.estimated_cpu_time = 5;
    meta.syscall_profile.addSyscall(2, Syscall{SyscallType::IoRead, 3});

    auto id = kernel.submitProgram(std::move(meta));
    kernel.runProgram(id);

    SimulationEngine engine(kernel, {1, 50});
    engine.run();

    auto& pcb = kernel.allProcesses()[0];
    EXPECT_EQ(pcb->state(), ProcessState::Terminated);
    EXPECT_EQ(pcb->cpuTimeUsed(), 5);
}

TEST_F(FullTickCycleTest, DeterministicExecution) {
    // Run the same scenario twice, verify identical results
    auto runScenario = []() -> Tick {
        Logger::setEnabled(false);
        Kernel kernel(Kernel::Config{SchedulingPolicy::RoundRobin, 2});

        ProgramMetadata m1, m2;
        m1.program_name = "A";
        m1.estimated_cpu_time = 4;
        m2.program_name = "B";
        m2.estimated_cpu_time = 3;

        auto id1 = kernel.submitProgram(std::move(m1));
        auto id2 = kernel.submitProgram(std::move(m2));
        kernel.runProgram(id1);
        kernel.runProgram(id2);

        SimulationEngine engine(kernel, {1, 100});
        engine.run();
        return kernel.currentTick();
    };

    Tick run1 = runScenario();
    Tick run2 = runScenario();
    EXPECT_EQ(run1, run2);
}
