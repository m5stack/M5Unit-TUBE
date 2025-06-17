/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_MCP_H10.hpp
  @brief MCP-H10 Failiy unit for M5UnitUnified
 */
#ifndef M5_UNIT_TUBE_UNIT_MCP_H10_HPP
#define M5_UNIT_TUBE_UNIT_MCP_H10_HPP

#include <M5UnitComponent.hpp>
#include <limits>  // NaN

namespace m5 {
namespace unit {

/*!
  @namespace mcp_h10
  @brief For MCP-H10 family
 */
namespace mcp_h10 {

/*!
  @struct Data
  @brief Measurement data group
 */
struct Data {
    uint16_t raw{};   // Raw data
    float voltage{};  // Calculated voltage

    inline float pressure() const
    {
        return voltage * k + b;
    }

    float k{}, b{};
};

}  // namespace mcp_h10

/*!
  @class m5::unit::UnitMCP_H10
  @brief MCP-H10 family unit base class
  @note  MCP-H10 family spec
| Model             |   Po (kPa) |   Pr (kPa) |   OL (V) |   OH (V) |       k |         B | M5?|
|-------------------|------------|------------|----------|----------|---------|-----------|---|
| MCP-H10-A2KP5P    |        0   |        2.5 |     0.5  |     4.5  |   0.625 |   -0.3125 ||
| MCP-H10-A2KP5PN   |      -2.5  |        2.5 |     0.5  |     4.5  |   1.25  |   -3.125  ||
| MCP-H10-A5KPP     |        0   |        5   |     0.5  |     4.5  |   1.25  |   -0.625  ||
| MCP-H10-A5KPPN    |      -5    |        5   |     0.5  |     4.5  |   2.5   |   -6.25   ||
| MCP-H10-A10KPP    |        0   |       10   |     0.5  |     4.5  |   2.5   |   -1.25   ||
| MCP-H10-A10KPPN   |     -10    |       10   |     0.5  |     4.5  |   5     |  -12.5    ||
| MCP-H10-A40KPP    |        0   |       40   |     0.5  |     4.5  |  10     |   -5      ||
| MCP-H10-A40KPPN   |     -40    |       40   |     0.5  |     4.5  |  20     |  -20      ||
| MCP-H10-A100KPP   |        0   |      100   |     0.5  |     4.5  |  25     |  -12.5    ||
| MCP-H10-A100KPPN  |    -100    |      100   |     0.5  |     4.5  |  50     |  -25      ||
| MCP-H10-A200KPP   |        0   |      200   |     0.5  |     4.5  |  50     |  -12.5    ||
| MCP-H10-A200KPPN  |    -100    |      200   |     0.5  |     4.5  |  75     | -137.5    ||
| MCP-H10-A500KPP   |        0   |      500   |     0.5  |     4.5  | 125     |  -62.5    ||
| MCP-H10-A1000KPP  |        0   |     1000   |     0.5  |     4.5  | 250     | -125      ||
| MCP-H10-A1000KPPN |    -100    |     1000   |     0.5  |     4.5  | 275     | -237.5    ||
| MCP-H10-B2KP5P    |        0   |        2.5 |     0.1  |     3.1  |   0.833 |   -0.083  ||
| MCP-H10-B2KP5PN   |      -2.5  |        2.5 |     0.1  |     3.1  |   1.667 |   -2.667  ||
| MCP-H10-B05KPPN   |      -5    |        5   |     0.1  |     3.1  |   3.333 |   -5.333  ||
| MCP-H10-B05KPP    |        0   |        5   |     0.1  |     3.1  |   1.667 |   -0.167  ||
| MCP-H10-B10KPP    |        0   |       10   |     0.1  |     3.1  |   3.333 |   -0.333  ||
| MCP-H10-B10KPPN   |     -10    |       10   |     0.1  |     3.1  |   6.667 |   -3.333  ||
| MCP-H10-B40KPP    |        0   |       40   |     0.1  |     3.1  |  13.333 |   -1.333  ||
| MCP-H10-B40KPPN   |     -40    |       40   |     0.1  |     3.1  |  26.667 |  -42.667  ||
| MCP-H10-B100KPP   |        0   |      100   |     0.1  |     3.1  |  33.333 |   -3.333  ||
| MCP-H10-B100KPPN  |    -100    |      100   |     0.1  |     3.1  |  66.667 | -106.667  ||
| MCP-H10-B200KPP   |        0   |      200   |     0.1  |     3.1  |  66.667 |   -6.667  ||
| MCP-H10-B200KPPN  |    -100    |      200   |     0.1  |     3.1  | 100     | -110      |TubePressure(SKU:U130)|
| MCP-H10-B500KPP   |        0   |      500   |     0.1  |     3.1  | 166.667 |  -16.667  ||
| MCP-H10-B1000KPP  |        0   |     1000   |     0.1  |     3.1  | 333.333 |  -33.333  ||
| MCP-H10-B1000KPPN |    -100    |     1000   |     0.1  |     3.1  | 366.667 | -136.667  ||
*/
class UnitMCP_H10 : public Component, public PeriodicMeasurementAdapter<UnitMCP_H10, mcp_h10::Data> {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitMCP_H10, 0x00 /* No I2C */);

public:
    /*!
      @brief Constructor
      @param minV OL
      @param minV OH
      @param coefficient k
      @param offset B
     */
    explicit UnitMCP_H10(const float minV, const float maxV, const float coefficient, const float offset)
        : Component(DEFAULT_ADDRESS),
          _data{new m5::container::CircularBuffer<mcp_h10::Data>(1)},
          _minV{minV},
          _maxV{maxV},
          _coefficient{coefficient},
          _offset{offset}
    {
    }
    virtual ~UnitMCP_H10() = default;

    virtual bool begin() override;
    virtual void update(const bool force = false) override;

    /*!
      @struct config_t
      @brief Settings for begin
     */
    struct config_t {
        //! Start periodic measurement on begin?
        bool start_periodic{true};
        //! Interval time if start on begin (ms)
        uint32_t interval_ms{100};
        //! Calibration (Voltage value to be considered as pressure 0)
        float calib_vzero{};
    };

    ///@name Settings for begin
    ///@{
    /*! @brief Gets the configration */
    inline config_t config()
    {
        return _cfg;
    }
    //! @brief Set the configration
    inline void config(const config_t& cfg)
    {
        _cfg = cfg;
    }
    ///@}

    ///@name Properties
    ///@{
    //! @brief coefficient (K)
    inline float coefficient() const
    {
        return _coefficient;
    }
    //! @brief offset (B)
    inline float offset() const
    {
        return _offset;
    }
    //! @brief minimum voltage (OL)
    inline float minimumVoltage() const
    {
        return _minV;
    }
    //! @brief maximum voltage (OH)
    inline float maximumVoltage() const
    {
        return _maxV;
    }
    //! @brief Voltage range
    inline float voltageRange() const
    {
        return maximumVoltage() - minimumVoltage();
    }

    ///@}

    ///@name Measurement data by periodic
    ///@{
    //! @brief Oldest measured pressure (kPA)
    inline float pressure() const
    {
        return !empty() ? oldest().pressure() : std::numeric_limits<float>::quiet_NaN();
    }
    ///@}

    ///@name Periodic measurement
    ///@{
    /*!
      @brief Start periodic measurement
      @param interval Measurement interval (ms)
      @return True if successful
    */
    inline bool startPeriodicMeasurement(const uint32_t interval)
    {
        return PeriodicMeasurementAdapter<UnitMCP_H10, mcp_h10::Data>::startPeriodicMeasurement(interval);
    }
    /*!
      @brief Stop periodic measurement
      @return True if successful
    */
    inline bool stopPeriodicMeasurement()
    {
        return PeriodicMeasurementAdapter<UnitMCP_H10, mcp_h10::Data>::stopPeriodicMeasurement();
    }
    ///@}

    ///@name Single shot measurement
    ///@{
    /*!
      @brief Measurement single shot
      @param[out] data Measuerd data
      @warning During periodic detection runs, an error is returned
    */
    bool measureSingleshot(mcp_h10::Data& d);
    ///@}

    ///@name Calibration (Software)
    ///@{
    inline bool isCalibrated() const
    {
        return _calib_zero_diff != 0.0f;
    }
    /*!
      @brief Set calibration parameter
      @param vzero Voltage value to be considered as pressure 0
    */
    void setCalibration(const float vzero)
    {
        const float ideal_zero_voltage = -offset() / coefficient();
        _calib_zero_diff               = vzero - ideal_zero_voltage;
    }
    //! @brief Disable calibration
    inline void clearCalibration()
    {
        _calib_zero_diff = 0.0f;
    };
    ///@}

protected:
    bool read_measurement(mcp_h10::Data& d);
    bool start_periodic_measurement(const uint32_t interval);
    bool stop_periodic_measurement();

    std::unique_ptr<m5::container::CircularBuffer<mcp_h10::Data>> _data{};

    M5_UNIT_COMPONENT_PERIODIC_MEASUREMENT_ADAPTER_HPP_BUILDER(UnitMCP_H10, mcp_h10::Data);

private:
    float _minV{}, _maxV{}, _coefficient{}, _offset{};
    float _calib_zero_diff{};
    config_t _cfg{};
};

/*!
  @class m5::unit::UnitMCP_H10_B200KPPN
  @brief MCP-H10-B200KPPN (TubePressure Unit)
*/
class UnitMCP_H10_B200KPPN : public UnitMCP_H10 {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitMCP_H10_B200KPPN, 0x00 /* No I2C */);

public:
    explicit UnitMCP_H10_B200KPPN() : UnitMCP_H10(0.1f, 3.1f, 100.f, -110.f)
    {
    }
    virtual ~UnitMCP_H10_B200KPPN() = default;
};

/*!

 */
}  // namespace unit
}  // namespace m5
#endif
