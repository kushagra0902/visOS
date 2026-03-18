#include "services/program_store.hpp"
#include "core/program_metadata.hpp"

#include <gtest/gtest.h>

using namespace visos;

TEST(ProgramStoreTest, StoreAndLoad) {
    ProgramStore store;
    ProgramMetadata meta;
    meta.program_name = "TestProg";
    meta.estimated_cpu_time = 10;

    ProgramID id = store.store(std::move(meta));
    EXPECT_GE(id, 1);

    auto loaded = store.load(id);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->program_name, "TestProg");
    EXPECT_EQ(loaded->estimated_cpu_time, 10);
}

TEST(ProgramStoreTest, LoadNonExistent) {
    ProgramStore store;
    EXPECT_FALSE(store.load(999).has_value());
}

TEST(ProgramStoreTest, UniqueIDs) {
    ProgramStore store;
    ProgramMetadata m1, m2;
    m1.program_name = "A";
    m2.program_name = "B";

    auto id1 = store.store(std::move(m1));
    auto id2 = store.store(std::move(m2));
    EXPECT_NE(id1, id2);
}
