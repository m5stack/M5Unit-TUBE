/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitPaHub
*/
#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <unit/unit_MCP_H10.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::mcp_h10;
using namespace m5::unit::types;

const ::testing::Environment* global_fixture = ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

constexpr uint32_t STORED_SIZE{8};

class TestMCP_H10_B200KPPN : public GPIOComponentTestBase<UnitMCP_H10_B200KPPN, bool> {
protected:
    virtual UnitMCP_H10_B200KPPN* get_instance() override
    {
        auto ptr         = new m5::unit::UnitMCP_H10_B200KPPN();
        auto ccfg        = ptr->component_config();
        ccfg.stored_size = STORED_SIZE;
        ptr->component_config(ccfg);
        return ptr;
    }
    virtual bool is_using_hal() const override
    {
        return GetParam();
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestMCP_H10_B200KPPN, ::testing::Values(false, true));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestMCP_H10_B200KPPN,::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestMCP_H10_B200KPPN, ::testing::Values(false));

namespace {
template <class U>
elapsed_time_t test_periodic(U* unit, const uint32_t times, const uint32_t measure_duration = 0)
{
    auto tm         = unit->interval();
    auto timeout_at = m5::utility::millis() + 10 * 1000;

    do {
        unit->update();
        if (unit->updated()) {
            break;
        }
        std::this_thread::yield();
    } while (!unit->updated() && m5::utility::millis() <= timeout_at);
    // timeout
    if (!unit->updated()) {
        return 0;
    }

    //
    uint32_t measured{};
    auto start_at = m5::utility::millis();
    timeout_at    = start_at + (times * (tm + measure_duration) * 2);

    do {
        unit->update();
        measured += unit->updated() ? 1 : 0;
        if (measured >= times) {
            break;
        }
        // std::this_thread::yield();
        m5::utility::delay(1);

    } while (measured < times && m5::utility::millis() <= timeout_at);
    return (measured == times) ? m5::utility::millis() - start_at : 0;
    // return (measured == times) ? unit->updatedMillis() - start_at : 0;
}
}  // namespace

TEST_P(TestMCP_H10_B200KPPN, Periodic)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->inPeriodic());
    EXPECT_FALSE(unit->startPeriodicMeasurement(120));
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    constexpr uint32_t it{150};

    EXPECT_TRUE(unit->startPeriodicMeasurement(it));
    auto elapsed = test_periodic(unit.get(), STORED_SIZE, it);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    EXPECT_NE(elapsed, 0);
    EXPECT_GE(elapsed, STORED_SIZE * it);

    //
    EXPECT_EQ(unit->available(), STORED_SIZE);
    EXPECT_FALSE(unit->empty());
    EXPECT_TRUE(unit->full());

    uint32_t cnt{STORED_SIZE / 2};
    while (cnt-- && unit->available()) {
        EXPECT_TRUE(std::isfinite(unit->pressure()));
        EXPECT_FLOAT_EQ(unit->pressure(), unit->oldest().pressure());
        EXPECT_FALSE(unit->empty());
        unit->discard();
    }
    EXPECT_EQ(unit->available(), STORED_SIZE / 2);
    EXPECT_FALSE(unit->empty());
    EXPECT_FALSE(unit->full());

    unit->flush();
    EXPECT_EQ(unit->available(), 0);
    EXPECT_TRUE(unit->empty());
    EXPECT_FALSE(unit->full());

    EXPECT_FALSE(std::isfinite(unit->pressure()));
}

TEST_P(TestMCP_H10_B200KPPN, Singleshot)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->inPeriodic());

    Data d{};
    EXPECT_FALSE(unit->measureSingleshot(d));

    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    uint32_t count{8};
    while (count--) {
        EXPECT_TRUE(unit->measureSingleshot(d));
        EXPECT_TRUE(std::isfinite(d.pressure()));
    }
}
