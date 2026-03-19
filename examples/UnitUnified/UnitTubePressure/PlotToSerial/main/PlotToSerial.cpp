/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using M5UnitUnified for UnitTubePressure
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedTUBE.h>

using namespace m5::unit::mcp_h10;

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitTubePressure unit;

// Display
bool has_display{};
LGFX_Sprite sprite;

// Layout (calculated in setup)
int32_t sw{}, sh{};         // sprite width, height
int32_t sprite_y{};         // sprite Y offset on lcd
int32_t bar_y{}, bar_h{};   // bar area within sprite
int32_t val_y{}, sub_y{};   // text areas within sprite
int32_t zero_x{};           // 0 kPa marker position
bool small_display{};       // true if sw < 200
const lgfx::IFont* font{};  // font selection

// Pressure range: -100 ~ 200 kPa
constexpr float P_MIN{-100.f};
constexpr float P_MAX{200.f};
constexpr float P_RANGE{P_MAX - P_MIN};

// Palette indices (2-bit: 0~3)
enum : uint8_t {
    C_BG   = 0,  // background + bar background
    C_BAR  = 1,  // pressure bar
    C_MARK = 2,  // zero marker, border, sub text
    C_TEXT = 3,  // labels, value text
};

void init_display()
{
    has_display = (lcd.width() > 0 && lcd.height() > 0 && !lcd.isEPD());
    if (!has_display) {
        return;
    }

    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    sw            = lcd.width();
    sh            = lcd.height();
    small_display = (sw < 200);

    // Font selection based on display size
    if (small_display) {
        font = &fonts::AsciiFont8x16;
    } else {
        font = &fonts::FreeSansBold9pt7b;
    }

    // Layout
    int32_t font_h  = small_display ? 16 : 22;
    int32_t title_h = font_h + 2;
    sprite_y        = title_h;

    bar_y           = 2;  // within sprite
    bar_h           = small_display ? sh / 5 : sh / 4;
    int32_t scale_y = bar_y + bar_h + 2;     // scale labels below bar
    val_y           = scale_y + font_h + 2;  // pressure value below scale
    sub_y           = val_y + font_h + 2;    // voltage/raw

    int32_t sprite_h = sub_y + font_h + 2;

    // 0 kPa position in bar
    zero_x = sw * (0 - P_MIN) / P_RANGE;

    // Create 2-bit palette sprite (4 colors, minimal memory)
    sprite.setPsram(false);
    sprite.setColorDepth(2);
    sprite.createSprite(sw, sprite_h);

    // Setup palette
    sprite.setPaletteColor(C_BG, TFT_BLACK);
    sprite.setPaletteColor(C_BAR, TFT_BLUE);
    sprite.setPaletteColor(C_MARK, 0x7BCFu);  // grey
    sprite.setPaletteColor(C_TEXT, TFT_WHITE);

    sprite.setFont(font);

    // Title on lcd (above sprite area)
    lcd.fillScreen(TFT_BLACK);
    lcd.startWrite();
    lcd.setFont(font);
    lcd.setTextColor(TFT_WHITE);
    lcd.setCursor(0, 0);
    lcd.print(small_display ? "TUBE" : "Tube Pressure");
    lcd.endWrite();
}

void render_pressure(const float pressure, const float voltage, const uint16_t raw)
{
    if (!has_display) {
        return;
    }

    // Draw to sprite
    sprite.fillScreen(C_BG);

    // Bar border
    sprite.drawRect(0, bar_y, sw, bar_h, C_MARK);

    // Pressure bar
    int32_t bw = sw * ((pressure - P_MIN) / P_RANGE);
    bw         = std::max((int32_t)0, std::min(bw, sw));
    if (pressure < 0) {
        sprite.fillRect(bw, bar_y + 1, zero_x - bw, bar_h - 2, C_BAR);
    } else {
        sprite.fillRect(zero_x, bar_y + 1, bw - zero_x, bar_h - 2, C_BAR);
    }

    // 0 kPa marker
    sprite.drawFastVLine(zero_x, bar_y, bar_h, C_TEXT);

    // Scale labels below bar
    int32_t scale_y = bar_y + bar_h + 2;
    sprite.setTextColor((uint8_t)C_MARK);
    sprite.setTextDatum(textdatum_t::top_left);
    sprite.drawString("-100", 2, scale_y);
    sprite.setTextDatum(textdatum_t::top_center);
    sprite.drawString("0", zero_x, scale_y);
    sprite.setTextDatum(textdatum_t::top_right);
    sprite.drawString("200", sw - 2, scale_y);

    // Pressure value
    auto s = m5::utility::formatString("%.1f kPa", pressure);
    sprite.setTextColor((uint8_t)C_TEXT);
    sprite.setTextDatum(textdatum_t::top_left);
    sprite.drawString(s.c_str(), 2, val_y);

    // Voltage and raw (right-aligned on same line for large, next line for small)
    auto s2 = m5::utility::formatString("%.3fV R:%u", voltage, raw);
    sprite.setTextColor((uint8_t)C_MARK);
    if (small_display) {
        sprite.setTextDatum(textdatum_t::top_left);
        sprite.drawString(s2.c_str(), 2, sub_y);
    } else {
        sprite.setTextDatum(textdatum_t::top_right);
        sprite.drawString(s2.c_str(), sw - 2, val_y);
    }

    // Push to display (flicker-free)
    sprite.pushSprite(&lcd, 0, sprite_y);
}

}  // namespace

void setup()
{
    M5.begin();
    M5.setTouchButtonHeightByRatio(100);

    init_display();

    auto pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_b_in);
    auto pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_b_out);
    if (pin_num_gpio_in < 0 || pin_num_gpio_out < 0) {
        M5_LOGW("PortB is not available");
        Wire.end();
        pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
        pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_a_pin2);
    }
    M5_LOGI("getPin: %d,%d", pin_num_gpio_in, pin_num_gpio_out);

    if (!Units.add(unit, pin_num_gpio_in, pin_num_gpio_out) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        if (has_display) {
            lcd.fillScreen(TFT_RED);
        }
        while (true) {
            m5::utility::delay(10000);
        }
    }

    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    render_pressure(0.f, 0.f, 0);
}

void loop()
{
    M5.update();
    Units.update();

    if (unit.updated()) {
        auto p = unit.pressure();
        M5.Log.printf(">Pressure:%.2f\n>V:%f\n>Raw:%u\n", p, unit.oldest().voltage, unit.oldest().raw);
        render_pressure(p, unit.oldest().voltage, unit.oldest().raw);
    }

    /*
      Measure single and calibrate
      To be done with no pressure applied
     */
    if (M5.BtnA.wasClicked()) {
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
