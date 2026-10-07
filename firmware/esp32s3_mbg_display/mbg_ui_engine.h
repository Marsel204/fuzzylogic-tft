/**
 * @file mbg_ui_engine.h
 * @brief High-Performance TFT UI Engine for Indonesian MBG Freshness Inspector
 * 320x240 Landscape Layout with High-Visibility Industrial HACCP Widgets
 */

#ifndef MBG_UI_ENGINE_H
#define MBG_UI_ENGINE_H

#include <Arduino.h>
#include "mbg_tft_setup.h"
#include <TFT_eSPI.h>
#include "mbg_fuzzy_core.h"
#include "mbg_profiles.h"
#include "mbg_mock_sensors.h"

typedef enum {
    UI_STATE_BOOT_SPLASH = 0,
    UI_STATE_INGREDIENT_SELECT = 1,
    UI_STATE_LIVE_PROBE = 2,
    UI_STATE_HACCP_VERDICT = 3
} MbgUiState;

struct MbgUiRect {
    int x, y, w, h;
    bool contains(uint16_t px, uint16_t py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};
inline MbgUiRect mbgIngredientRect(int index) {
    return {8 + (index % 2) * 156, 52 + (index / 2) * 38, 148, 34};
}

class MbgUiEngine {
public:
    MbgUiEngine(TFT_eSPI& tftRef);

    void init();
    void setState(MbgUiState newState);
    MbgUiState getState() const { return m_state; }

    /* Screen Renders */
    void renderBootSplash();
    void renderIngredientSelect();
    void renderLiveProbe(MbgProfileId profile, const MbgSensorReading& reading, uint32_t evalMicros = 0);
    void renderHaccpVerdict(const MbgProfileConfig* cfg, const MbgInferenceResult& result, uint32_t evalMicros);

    /* Touch Hit-Testing */
    int hitTestIngredient(uint16_t x, uint16_t y);
    bool hitTestProbeEval(uint16_t x, uint16_t y);
    bool hitTestProbeBack(uint16_t x, uint16_t y);
    bool hitTestVerdictAudit(uint16_t x, uint16_t y);
    bool hitTestVerdictRepeat(uint16_t x, uint16_t y);
    bool hitTestVerdictMenu(uint16_t x, uint16_t y);
    bool hitTestScenarioToggle(uint16_t x, uint16_t y);

    /* Helper notification */
    void showFlashToast(const char* message, uint16_t color = TFT_YELLOW);

    /* Clean UI Widgets */
    void drawPill(int x, int y, int w, int h, const char* text, uint16_t fg, uint16_t bg, uint16_t outline = 0);
    void drawProgressBar(int x, int y, int w, int h, float progressRatio, uint16_t fillColor);

private:
    TFT_eSPI& m_tft;
    MbgUiState m_state;
    MbgProfileId m_activeProfile;
    bool m_dirty;
    MockScenarioType m_scenario;
    char m_values[4][16];
    char m_badges[4][16];

    /* Base draw methods */
    void drawHeader(const char* title, const char* badgeText = nullptr, uint16_t badgeColor = TFT_SKYBLUE);
    void drawSensorCard(int x, int y, int w, int h, const char* label, const char* value, const char* badge, uint16_t badgeColor, float rangeProgress);
};

#endif /* MBG_UI_ENGINE_H */
