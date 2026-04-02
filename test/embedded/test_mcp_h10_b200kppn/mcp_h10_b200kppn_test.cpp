/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitTube
*/
#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <googletest/test_helper.hpp>
#include <unit/unit_MCP_H10.hpp>
#include <cmath>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::mcp_h10;
using namespace m5::unit::types;

constexpr uint32_t STORED_SIZE{8};

class TestMCP_H10_B200KPPN : public GPIOComponentTestBase<UnitMCP_H10_B200KPPN> {
protected:
    virtual UnitMCP_H10_B200KPPN* get_instance() override
    {
        auto ptr         = new m5::unit::UnitMCP_H10_B200KPPN();
        auto ccfg        = ptr->component_config();
        ccfg.stored_size = STORED_SIZE;
        ptr->component_config(ccfg);
        return ptr;
    }
};

TEST_F(TestMCP_H10_B200KPPN, Properties)
{
    SCOPED_TRACE(ustr);

    // MCP-H10-B200KPPN datasheet constants
    EXPECT_FLOAT_EQ(unit->coefficient(), 100.0f);
    EXPECT_FLOAT_EQ(unit->offset(), -110.0f);
    EXPECT_FLOAT_EQ(unit->minimumVoltage(), 0.1f);
    EXPECT_FLOAT_EQ(unit->maximumVoltage(), 3.1f);
    EXPECT_FLOAT_EQ(unit->voltageRange(), 3.0f);
}

TEST_F(TestMCP_H10_B200KPPN, Calibration)
{
    SCOPED_TRACE(ustr);

    // Initially not calibrated
    EXPECT_FALSE(unit->isCalibrated());

    // Set calibration: voltage at zero pressure
    // Ideal zero voltage = -offset / coefficient = 110 / 100 = 1.1V
    unit->setCalibration(1.2f);
    EXPECT_TRUE(unit->isCalibrated());

    // Read with calibration applied
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    Data d{};
    EXPECT_TRUE(unit->measureSingleshot(d));
    EXPECT_TRUE(std::isfinite(d.pressure()));
    float calibrated_pressure = d.pressure();

    // Clear calibration
    unit->clearCalibration();
    EXPECT_FALSE(unit->isCalibrated());

    // Read without calibration
    EXPECT_TRUE(unit->measureSingleshot(d));
    EXPECT_TRUE(std::isfinite(d.pressure()));
    float uncalibrated_pressure = d.pressure();

    // Calibrated and uncalibrated should differ (unless voltage happens to be exactly ideal zero)
    // The calibration offset is 1.2 - 1.1 = 0.1V, so pressure diff = 100 * 0.1 = 10 kPa
    // Allow some tolerance for ADC noise between two reads
    M5_LOGI("Calibrated: %.2f, Uncalibrated: %.2f", calibrated_pressure, uncalibrated_pressure);
    EXPECT_NE(calibrated_pressure, uncalibrated_pressure);
}

TEST_F(TestMCP_H10_B200KPPN, Config)
{
    SCOPED_TRACE(ustr);

    // Default config starts periodic measurement
    EXPECT_TRUE(unit->inPeriodic());

    // Stop and reconfigure
    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    auto cfg           = unit->config();
    cfg.start_periodic = false;
    cfg.interval_ms    = 200;
    cfg.calib_vzero    = 1.15f;
    unit->config(cfg);

    // Verify config is stored
    auto cfg2 = unit->config();
    EXPECT_FALSE(cfg2.start_periodic);
    EXPECT_EQ(cfg2.interval_ms, 200U);
    EXPECT_FLOAT_EQ(cfg2.calib_vzero, 1.15f);
}

TEST_F(TestMCP_H10_B200KPPN, Periodic)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->inPeriodic());
    EXPECT_FALSE(unit->startPeriodicMeasurement(120));
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    constexpr uint32_t it{150};

    EXPECT_TRUE(unit->startPeriodicMeasurement(it));
    auto r = collect_periodic_measurements(unit.get(), STORED_SIZE);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    EXPECT_FALSE(r.timed_out);
    EXPECT_EQ(r.update_count, STORED_SIZE);
    EXPECT_LE(r.median(), r.expected_interval + 1);

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

TEST_F(TestMCP_H10_B200KPPN, Singleshot)
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
