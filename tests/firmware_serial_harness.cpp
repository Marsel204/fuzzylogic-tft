// Exercise the real sketch and inference/UI code with only hardware I/O stubbed.
#include <Arduino.h>
#include <cassert>
SerialStub Serial;
#include "../firmware/esp32s3_mbg_display/esp32s3_mbg_display.ino"

static void setTouch(uint16_t x, uint16_t y, bool pressed) {
#if defined(MBG_TFT_GUITION)
    uint16_t rawX = 239 - y;
    Wire.connected = true;
    Wire.data = {uint8_t(pressed ? 1 : 0), uint8_t(0x80 | (rawX >> 8)), uint8_t(rawX),
                 uint8_t(x >> 8), uint8_t(x)};
#else
    tft.touchX = x;
    tft.touchY = y;
    tft.pressed = pressed;
#endif
}

static void press(uint16_t x, uint16_t y) {
    setTouch(x, y, true);
    loop();
    setTouch(x, y, false);
    loop();
}

static void captureScreen(const char* screen) {
    if (strcmp(screen, "boot") == 0) uiEngine.renderBootSplash();
    else if (strcmp(screen, "menu") == 0) uiEngine.renderIngredientSelect();
    else if (strcmp(screen, "probe") == 0 || strcmp(screen, "milk") == 0) {
        currentProfile = strcmp(screen, "milk") == 0 ? MBG_PROFILE_SUSU_SEGAR : MBG_PROFILE_AYAM_BROILER;
        uiEngine.setState(UI_STATE_LIVE_PROBE);
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
    } else {
        mbg_mock_sensors_set_scenario(strcmp(screen, "reject") == 0 ? MOCK_SCENARIO_SPOILED : MOCK_SCENARIO_FRESH);
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        MbgInferenceResult result = strcmp(screen, "unknown") == 0 ? mbg_evaluate_liquid(4.8f, 7.8f, 4.0f) :
            mbg_evaluate_solid(currentProfile, currentReading.r_eis, currentReading.phase_deg, currentReading.temp_c, currentReading.nh3_ppm);
        uiEngine.renderHaccpVerdict(mbg_get_profile_config(currentProfile), result, 1);
    }
    for (const TftDraw& draw : tft.commands) {
        std::printf("DRAW: {\"kind\":\"%s\",\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"r\":%d,\"font\":%d,\"color\":%u,\"background\":%u,\"text\":\"%s\"}\n",
                    draw.kind.c_str(), draw.x, draw.y, draw.w, draw.h, draw.radius, draw.font, draw.color, draw.background, draw.text.c_str());
    }
}

int main(int argc, char** argv) {
    setup();
    if (argc > 2 && strcmp(argv[1], "--screen") == 0) {
        captureScreen(argv[2]);
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--touch-flow") == 0) {
        press(160, 100); // Splash -> menu
        assert(uiEngine.getState() == UI_STATE_INGREDIENT_SELECT);
        press(50, 65); // Chicken
        assert(uiEngine.getState() == UI_STATE_LIVE_PROBE);
        press(270, 15); // Change scenario
        assert(currentReading.scenario == MOCK_SCENARIO_SPOILED);
        press(150, 207); // Evaluate
        assert(uiEngine.getState() == UI_STATE_HACCP_VERDICT);
        assert(currentResult.classification == MBG_CLASS_SPOILED);
        press(240, 215); // Repeat same ingredient
        assert(uiEngine.getState() == UI_STATE_LIVE_PROBE && hasSelectedProfile);
        assert(currentProfile == MBG_PROFILE_AYAM_BROILER);
        press(30, 15); // Menu
        assert(uiEngine.getState() == UI_STATE_INGREDIENT_SELECT && !hasSelectedProfile);
        press(200, 180); // Auto demo
        assert(autoDemoMode);
        stubMillis += 3001;
        loop();
        assert(uiEngine.getState() == UI_STATE_HACCP_VERDICT);
        press(30, 15); // Stop demo
        assert(!autoDemoMode && uiEngine.getState() == UI_STATE_INGREDIENT_SELECT);
        std::puts("TOUCH FLOW PASSED");
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--hold-touch") == 0) {
        uiEngine.setState(UI_STATE_INGREDIENT_SELECT);
        uiEngine.renderIngredientSelect();
        setTouch(200, 180, true);
        loop();
        assert(autoDemoMode);
        loop(); loop();
        assert(autoDemoMode); // The same held finger must not toggle the button repeatedly.
        std::puts("HELD TOUCH PASSED");
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--calibration") == 0) {
#if defined(MBG_TFT_GUITION)
        assert(tft.calibrations == 0); // Capacitive touch must not wait for XPT2046 calibration.
        calibrateDisplayTouch(true);
        assert(tft.calibrations == 0);
#else
        assert(tft.calibrations == 1);
        calibrateDisplayTouch();
        assert(tft.calibrations == 1); // NVS copy reused
        calibrateDisplayTouch(true);
        assert(tft.calibrations == 2);
#endif
        std::puts("CALIBRATION PASSED");
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--cst-input") == 0) {
        uint16_t x = 0, y = 0;
        uint8_t data[5] = {1, 0, 0, 0, 0};
        assert(mbgCstLandscapeTouch(data, x, y) && x == 0 && y == 239);
        data[2] = 239; data[3] = 1; data[4] = 63;
        assert(mbgCstLandscapeTouch(data, x, y) && x == 319 && y == 0);
        data[2] = 240;
        assert(!mbgCstLandscapeTouch(data, x, y));
        data[2] = 0; data[4] = 64;
        assert(!mbgCstLandscapeTouch(data, x, y));
        data[3] = 0; data[4] = 0; data[1] = 0x40; // Explicit finger-up event.
        assert(!mbgCstLandscapeTouch(data, x, y));
        data[1] = 0; data[0] = 0;
        assert(!mbgCstLandscapeTouch(data, x, y));
        data[0] = 2;
        assert(!mbgCstLandscapeTouch(data, x, y));
#if defined(MBG_TFT_GUITION)
        setTouch(150, 207, true);
        assert(readTouchCoords(x, y) && x == 150 && y == 207);
        Wire.bytesToReturn = 4;
        assert(!readTouchCoords(x, y));
        Wire.bytesToReturn = 5; Wire.connected = false;
        assert(!readTouchCoords(x, y));
#endif
        std::puts("CST INPUT PASSED");
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--redraw") == 0) {
        uiEngine.setState(UI_STATE_LIVE_PROBE);
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading);
        size_t original = tft.commands.size();
        uiEngine.renderLiveProbe(currentProfile, currentReading);
        assert(tft.commands.size() == original);
        currentReading.nh3_ppm += 2;
        uiEngine.renderLiveProbe(currentProfile, currentReading);
        assert(tft.commands.size() > original);
        for (size_t i = original; i < tft.commands.size(); ++i) {
            assert(!(tft.commands[i].w == 320 && tft.commands[i].h == 240));
        }
        std::puts("INCREMENTAL REDRAW PASSED");
        return 0;
    }
    Serial.input = argc > 1 ? argv[1] : "";
    handleSerialCommands();
    if (argc > 2 && strcmp(argv[2], "indeterminate") == 0) {
        currentResult = mbg_evaluate_liquid(4.8f, 7.8f, 4.0f);
        lastInspection = {true, MBG_PROFILE_SUSU_SEGAR, currentReading, currentResult, millis(), 0};
        uiEngine.renderHaccpVerdict(mbg_get_profile_config(MBG_PROFILE_SUSU_SEGAR), currentResult, 0);
        printAuditLogReceipt();
    }
}
