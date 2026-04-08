#include "kernel/kernel.hpp"
#include "kernel/simulation_engine.hpp"
#include "core/program_metadata.hpp"
#include "util/logger.hpp"

#include <gtest/gtest.h>

using namespace visos;

class FCFSScenarioTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::setEnabled(false);
    }
};

TEST_F(FCFSScenarioTest, ThreeProcessesRunInOrder) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata m1, m2, m3;
    m1.program_name = "P1";
    m1.estimated_cpu_time = 3;
    m2.program_name = "P2";
    m2.estimated_cpu_time = 2;
    m3.program_name = "P3";
    m3.estimated_cpu_time = 4;

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));
    auto id3 = kernel.submitProgram(std::move(m3));

    kernel.runProgram(id1);
    kernel.runProgram(id2);
    kernel.runProgram(id3);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    auto& processes = kernel.allProcesses();
    EXPECT_EQ(processes.size(), 3u);

    // All should be terminated
    for (const auto& p : processes) {
        EXPECT_EQ(p->state(), ProcessState::Terminated);
    }

    // In FCFS: P1 runs first (3 ticks), then P2 (2 ticks), then P3 (4 ticks)
    // Total should be 9 ticks of execution
    EXPECT_EQ(processes[0]->cpuTimeUsed(), 3); // P1
    EXPECT_EQ(processes[1]->cpuTimeUsed(), 2); // P2
    EXPECT_EQ(processes[2]->cpuTimeUsed(), 4); // P3
}

TEST_F(FCFSScenarioTest, NoPreemptionInFCFS) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::FCFS, 100});

    ProgramMetadata m1;
    m1.program_name = "LongProcess";
    m1.estimated_cpu_time = 10;

    auto id = kernel.submitProgram(std::move(m1));
    kernel.runProgram(id);

    // Run for exactly 10 ticks + 1 for initial dispatch
    SimulationEngine engine(kernel, {1, 50});
    engine.run();

    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 10);
}
