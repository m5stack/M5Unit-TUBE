/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitUnifiedTUBE.hpp
  @brief Main header of M5Unit-TUBE using M5UnitUnified

  @mainpage M5Unit-TUBE
  Library for UnitTUBE using M5UnitUnified.
*/
#ifndef M5_UNIT_UNIFIED_TUBE_HPP
#define M5_UNIT_UNIFIED_TUBE_HPP

#include "unit/unit_MCP_H10.hpp"

/*!
  @namespace m5
  @brief Top level namespace of M5stack
 */
namespace m5 {

/*!
  @namespace unit
  @brief Unit-related namespace
 */
namespace unit {
//! @brief Alias for UnitTunePressure
using UnitTubePressure = m5::unit::UnitMCP_H10_B200KPPN;

}  // namespace unit
}  // namespace m5
#endif
