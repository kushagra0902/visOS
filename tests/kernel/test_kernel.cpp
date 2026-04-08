#include "kernel/kernel.hpp"
#include "core/program_metadata.hpp"
#include "util/logger.hpp"

#include <gtest/gtest.h>

using namespace visos;

class KernelTest : public ::testing::Test {
protected:
    Kernel makeKernel(SchedulingPolicy policy = SchedulingPolicy::RoundRobin,
                      int quantum = 4) {
        return Kernel(Kernel::Config{policy, quantum});
    }

    ProgramMetadata makeProgram(const std::string& name, Tick cpu_time) {
        ProgramMetadata meta;
        meta.program_name = name;
        meta.estimated_cpu_time = cpu_time;
        return meta;
    }
};

TEST_F(KernelTest, SubmitAndRunProgram) {
    Logger::setEnabled(false);
    auto kernel = makeKernel();
    auto id = kernel.submitProgram(makeProgram("Test", 5));
    kernel.runProgram(id);

    EXPECT_EQ(kernel.allProcesses().size(), 1u);
    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Ready);
    EXPECT_TRUE(kernel.hasActiveProcesses());
}

TEST_F(KernelTest, RunNonExistentProgramThrows) {
    Logger::setEnabled(false);
    auto kernel = makeKernel();
    EXPECT_THROW(kernel.runProgram(999), std::runtime_error);
}

TEST_F(KernelTest, MultipleProcesses) {
    Logger::setEnabled(false);
    auto kernel = makeKernel();
    auto id1 = kernel.submitProgram(makeProgram("A", 5));
    auto id2 = kernel.submitProgram(makeProgram("B", 3));

    kernel.runProgram(id1);
    kernel.runProgram(id2);

    EXPECT_EQ(kernel.allProcesses().size(), 2u);
}

TEST_F(KernelTest, FirstTickDispatchesProcess) {
    Logger::setEnabled(false);
    auto kernel = makeKernel();
    auto id = kernel.submitProgram(makeProgram("Test", 5));
    kernel.runProgram(id);

    kernel.notifyTick(1);

    // After first tick, the process should have been dispatched and executed
    auto& processes = kernel.allProcesses();
    EXPECT_EQ(processes.size(), 1u);
    // It should be Running (dispatched by CPU idle handler)
    // and have some CPU time used
}

TEST_F(KernelTest, ShortProcessTerminates) {
    Logger::setEnabled(false);
    auto kernel = makeKernel(SchedulingPolicy::FCFS);
    auto id = kernel.submitProgram(makeProgram("Short", 2));
    kernel.runProgram(id);

    // Tick 1: CPU idle -> dispatch (no execution this tick)
    kernel.notifyTick(1);
    // Tick 2: execute tick 1 of 2
    kernel.notifyTick(2);
    // Tick 3: execute tick 2 of 2 -> process finishes
    kernel.notifyTick(3);

    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
    EXPECT_FALSE(kernel.hasActiveProcesses());
}
