#include "core/program_metadata.hpp"
#include "core/syscall_profile.hpp"
#include "core/syscall.hpp"

#include <gtest/gtest.h>

using namespace visos;

TEST(ProgramMetadataTest, DefaultConstruction) {
    ProgramMetadata meta;
    meta.program_name = "TestProgram";
    meta.estimated_cpu_time = 10;
    EXPECT_EQ(meta.program_name, "TestProgram");
    EXPECT_EQ(meta.estimated_cpu_time, 10);
    EXPECT_TRUE(meta.syscall_profile.empty());
}

TEST(SyscallProfileTest, AddAndRetrieveSyscall) {
    SyscallProfile profile;
    profile.addSyscall(5, Syscall{SyscallType::IoRead, 3});

    auto result = profile.getSyscallAt(5);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->type, SyscallType::IoRead);
    EXPECT_EQ(result->duration, 3);
}

TEST(SyscallProfileTest, NoSyscallAtTick) {
    SyscallProfile profile;
    profile.addSyscall(5, Syscall{SyscallType::IoRead, 3});

    EXPECT_FALSE(profile.getSyscallAt(3).has_value());
    EXPECT_FALSE(profile.getSyscallAt(6).has_value());
}

TEST(SyscallProfileTest, EmptyProfile) {
    SyscallProfile profile;
    EXPECT_TRUE(profile.empty());
    EXPECT_FALSE(profile.getSyscallAt(0).has_value());
}
