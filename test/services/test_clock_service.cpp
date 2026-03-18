#include "services/clock_service.hpp"

#include <gtest/gtest.h>

using namespace visos;

TEST(ClockServiceTest, InitialTimeIsZero) {
    ClockService clock({1}, [](Tick) {});
    EXPECT_EQ(clock.currentTime(), 0);
}

TEST(ClockServiceTest, TickAdvancesTime) {
    Tick last_tick = 0;
    ClockService clock({1}, [&last_tick](Tick t) { last_tick = t; });

    clock.tick();
    EXPECT_EQ(clock.currentTime(), 1);
    EXPECT_EQ(last_tick, 1);

    clock.tick();
    EXPECT_EQ(clock.currentTime(), 2);
    EXPECT_EQ(last_tick, 2);
}

TEST(ClockServiceTest, CallbackInvokedEachTick) {
    int count = 0;
    ClockService clock({1}, [&count](Tick) { count++; });

    clock.tick();
    clock.tick();
    clock.tick();
    EXPECT_EQ(count, 3);
}
