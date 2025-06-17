/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_MCP_H10.cpp
  @brief MCP-H10 Failiy unit for M5UnitUnified
 */

#include "unit_MCP_H10.hpp"
#include <M5Utility.hpp>

using namespace m5::utility::mmh3;
using namespace m5::unit::types;
using namespace m5::unit::mcp_h10;

namespace m5 {
namespace unit {

// class UnitMCP_H10
const char UnitMCP_H10::name[] = "UnitMCP_H10";
const types::uid_t UnitMCP_H10::uid{"UnitMCP_H10"_mmh3};
const types::attr_t UnitMCP_H10::attr{attribute::AccessGPIO};

bool UnitMCP_H10::begin()
{
    auto ssize = stored_size();
    assert(ssize && "stored_size must be greater than zero");
    if (ssize != _data->capacity()) {
        _data.reset(new m5::container::CircularBuffer<Data>(ssize));
        if (!_data) {
            M5_LIB_LOGE("Failed to allocate");
            return false;
        }
    }

    if (_cfg.calib_vzero != 0.0f) {
        setCalibration(_cfg.calib_vzero);
    }
    return pinModeRX(gpio::Mode::Analog) && (_cfg.start_periodic ? startPeriodicMeasurement(_cfg.interval_ms) : true);
}

void UnitMCP_H10::update(const bool force)
{
    _updated = false;
    if (inPeriodic()) {
        elapsed_time_t at{m5::utility::millis()};
        if (force || !_latest || at >= _latest + _interval) {
            Data d{};
            _updated = read_measurement(d);
            if (_updated) {
                _latest = at;
                _data->push_back(d);
            }
        }
    }
}

bool UnitMCP_H10::start_periodic_measurement(const uint32_t interval)
{
    if (inPeriodic()) {
        return false;
    }

    _periodic = true;
    _interval = interval;
    _latest   = 0;
    return _periodic;
}

bool UnitMCP_H10::stop_periodic_measurement()
{
    _periodic = false;
    return !_periodic;
}

bool UnitMCP_H10::measureSingleshot(Data& d)
{
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }
    return read_measurement(d);
}

//
bool UnitMCP_H10::read_measurement(Data& d)
{
    uint16_t raw{};
    constexpr float M5_VREF{3.6f};

    if (readAnalogRX(raw)) {
        float v = raw * M5_VREF / 4095.f;
        v -= _calib_zero_diff;
        v = std::fmin(maximumVoltage(), std::fmax(minimumVoltage(), v));

        d.raw     = raw;
        d.voltage = v;
        d.k       = coefficient();
        d.b       = offset();
        return true;
    }
    return false;
}

// class UnitMCP_H10_B200KPPN
const char UnitMCP_H10_B200KPPN::name[] = "UnitMCP_H10_B200KPPN";
const types::uid_t UnitMCP_H10_B200KPPN::uid{"UnitMCP_H10_B200KPPN"_mmh3};
const types::attr_t UnitMCP_H10_B200KPPN::attr{attribute::AccessGPIO};

}  // namespace unit
}  // namespace m5
