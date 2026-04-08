#include "kernel/simulation_engine.hpp"
#include "kernel/kernel.hpp"
#include "core/program_metadata.hpp"
#include "util/logger.hpp"

#include <gtest/gtest.h>

using namespace visos;

class SimulationEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::setEnabled(false);
    }
};

TEST_F(SimulationEngineTest, RunSimpleProcess) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});
    auto id = kernel.submitProgram(
        ProgramMetadata{"Simple", ResourceDescriptor{}, 3, SyscallProfile{}});
    kernel.runProgram(id);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 3);
}

TEST_F(SimulationEngineTest, StopPreventsCompletion) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});
    auto id = kernel.submitProgram(
        ProgramMetadata{"Long", ResourceDescriptor{}, 1000, SyscallProfile{}});
    kernel.runProgram(id);

    SimulationEngine engine(kernel, {1, 5});
    engine.run();

    // Should stop at max_ticks
    EXPECT_NE(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
}

TEST_F(SimulationEngineTest, StepMode) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});
    auto id = kernel.submitProgram(
        ProgramMetadata{"Step", ResourceDescriptor{}, 3, SyscallProfile{}});
    kernel.runProgram(id);

    SimulationEngine engine(kernel, {1, 100});
    engine.step();
    // After first step, process should be dispatched
    EXPECT_TRUE(kernel.hasActiveProcesses());
}
