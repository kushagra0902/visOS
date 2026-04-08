#include "kernel/kernel.hpp"
#include "kernel/simulation_engine.hpp"
#include "core/program_metadata.hpp"
#include "util/logger.hpp"

#include <gtest/gtest.h>

using namespace visos;

class RoundRobinScenarioTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::setEnabled(false);
    }
};

TEST_F(RoundRobinScenarioTest, PreemptionOccurs) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::RoundRobin, 2});

    ProgramMetadata m1, m2;
    m1.program_name = "P1";
    m1.estimated_cpu_time = 4;
    m2.program_name = "P2";
    m2.estimated_cpu_time = 4;

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));

    kernel.runProgram(id1);
    kernel.runProgram(id2);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    // Both should terminate
    for (const auto& p : kernel.allProcesses()) {
        EXPECT_EQ(p->state(), ProcessState::Terminated);
    }

    // With quantum=2 and RR:
    // P1 runs 2 ticks, preempted
    // P2 runs 2 ticks, preempted
    // P1 runs 2 ticks, exits
    // P2 runs 2 ticks, exits
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 4);
    EXPECT_EQ(kernel.allProcesses()[1]->cpuTimeUsed(), 4);
}

TEST_F(RoundRobinScenarioTest, ShortProcessFinishesEarly) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::RoundRobin, 4});

    ProgramMetadata m1, m2;
    m1.program_name = "Short";
    m1.estimated_cpu_time = 2;
    m2.program_name = "Long";
    m2.estimated_cpu_time = 6;

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));

    kernel.runProgram(id1);
    kernel.runProgram(id2);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    EXPECT_EQ(kernel.allProcesses()[0]->state(), ProcessState::Terminated);
    EXPECT_EQ(kernel.allProcesses()[1]->state(), ProcessState::Terminated);
    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 2);
    EXPECT_EQ(kernel.allProcesses()[1]->cpuTimeUsed(), 6);
}

TEST_F(RoundRobinScenarioTest, ThreeProcessesWithDifferentLengths) {
    Kernel kernel(Kernel::Config{SchedulingPolicy::RoundRobin, 3});

    ProgramMetadata m1, m2, m3;
    m1.program_name = "P1";
    m1.estimated_cpu_time = 5;
    m2.program_name = "P2";
    m2.estimated_cpu_time = 3;
    m3.program_name = "P3";
    m3.estimated_cpu_time = 7;

    auto id1 = kernel.submitProgram(std::move(m1));
    auto id2 = kernel.submitProgram(std::move(m2));
    auto id3 = kernel.submitProgram(std::move(m3));

    kernel.runProgram(id1);
    kernel.runProgram(id2);
    kernel.runProgram(id3);

    SimulationEngine engine(kernel, {1, 100});
    engine.run();

    for (const auto& p : kernel.allProcesses()) {
        EXPECT_EQ(p->state(), ProcessState::Terminated);
    }

    EXPECT_EQ(kernel.allProcesses()[0]->cpuTimeUsed(), 5);
    EXPECT_EQ(kernel.allProcesses()[1]->cpuTimeUsed(), 3);
    EXPECT_EQ(kernel.allProcesses()[2]->cpuTimeUsed(), 7);
}
