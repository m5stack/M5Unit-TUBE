/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example of using via UnitPbHub

  Core ---> PbHub ----> ch:3 UnitTubePressure
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedTUBE.h>
#include <M5UnitUnifiedHUB.h>  // UnitPbHub

using namespace m5::unit::mcp_h10;

namespace {
auto& lcd = M5.Display;

m5::unit::UnitUnified Units;
m5::unit::UnitPbHub hub;
m5::unit::UnitTubePressure unit;

bool render_pressure(const float pressure)
{
    static int32_t prev_bw{0};

    auto w     = lcd.width();
    auto h     = lcd.height() / 4;
    int32_t bw = w * ((pressure + 100) / 300.f);
    if (bw == prev_bw) {
        return false;
    }

    lcd.startWrite();

    if (prev_bw > bw) {
        lcd.fillRect(bw, h, prev_bw - bw, h, 0);
    } else {
        lcd.fillRect(prev_bw, h, bw - prev_bw, h, TFT_YELLOW);
    }
    lcd.drawRect(0, h, w, h, TFT_WHITE);
    lcd.drawFastVLine(w / 3, h, h, TFT_RED);  // 0 Ka
    lcd.setCursor(0, h * 2 + 1);
    lcd.printf("Pres:%7.2f Ka", pressure);

    lcd.endWrite();

    prev_bw = bw;

    return true;
}

}  // namespace

void setup()
{
    M5.begin();

    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    //
    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Wire.end();
    Wire.begin(pin_num_sda, pin_num_scl, 400000U);

    if (!hub.add(unit, 3) ||      // PbHub ch:3 -> UnitTubePressure
        !Units.add(hub, Wire) ||  // Core -> PbHub
        !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.fillScreen(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    lcd.startWrite();
    lcd.fillScreen(0);

    auto w = lcd.width();
    auto h = lcd.height() / 4;
    lcd.fillRect(0, h, w / 3, h, TFT_YELLOW);
    lcd.drawRect(0, h, w, h, TFT_WHITE);
    lcd.drawFastVLine(w / 3, h, h, TFT_RED);  // 0 Ka
    lcd.setCursor(0, h * 2 + 1);
    lcd.printf("Pres:%7.2f Ka", 0.0f);

    lcd.endWrite();
}

void loop()
{
    M5.update();
    auto touch = M5.Touch.getDetail();
    Units.update();

    if (unit.updated()) {
        auto p = unit.pressure();
        M5.Log.printf(">Pressure:%.2f\n>V:%f\n>Raw:%u\n", p, unit.oldest().voltage, unit.oldest().raw);
        render_pressure(p);
    }

    /*
      Measure single and calibrate
      To be done with no pressure applied
     */
    if (M5.BtnA.wasClicked() || touch.wasClicked()) {
        static bool single{};
        single = !single;

        if (single) {
            unit.stopPeriodicMeasurement();
            Data d{};
            if (unit.measureSingleshot(d)) {
                M5.Log.printf("Single: Pressure:%.2f V:%f Raw:%u\n", d.pressure(), d.voltage, d.raw);
                M5.Log.printf("Calibrate: Pressure 0 = %f V\n", d.voltage);
                unit.setCalibration(d.voltage);
            }
        } else {
            unit.startPeriodicMeasurement(100);
        }
    }
}
