/**
 * @file mbg_tft_display.ino
 * @brief Indonesian MBG Freshness Inspector - TFT Touch Display Firmware
 * Integrates 320x240 SPI TFT, C99 Fuzzy Logic, and Realistic Multi-Sensor Simulator.
 *
 * Hardware Compatibility:
 *  - ESP32-S3 (FSPI: MOSI=11, SCK=12, MISO=13, CS=10, DC=14, RST=15, BL=16, Touch=17/18)
 *  - ESP32 CYD Guition JC2432W328C (ILI9341/ST7789, CST820/XPT2046)
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Preferences.h>
#include "mbg_tft_setup.h"
#include <TFT_eSPI.h>

#include "mbg_fuzzy_core.h"
#include "mbg_profiles.h"
#include "mbg_calibration.h"
#include "mbg_mock_sensors.h"
#include "mbg_ui_engine.h"
#include "mbg_cst_touch.h"

// =========================================================================
// HARDWARE PIN DEFINITIONS
// =========================================================================
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  #define PIN_TFT_BL       16
  #define PIN_LED_PASS     6   // Green LED
  #define PIN_LED_FAIL     7   // Red LED
  #define PIN_BUZZER       5   // HACCP Buzzer
  #define PIN_TP_SDA       8
  #define PIN_TP_SCL       9
  #define LED_ACTIVE       HIGH
  #define LED_INACTIVE     LOW
#else
  // Fallback for standard ESP32 / CYD (Guition JC2432W328C)
  #define PIN_TFT_BL       21
  #define PIN_TFT_BL2      27
  #define PIN_TP_RST       25
  #define PIN_LED_PASS     16  // Green RGB on CYD (Active LOW)
  #define PIN_LED_FAIL     4   // Red RGB on CYD (Active LOW)
  #define PIN_LED_BLUE     17
  #define PIN_SD_CS        5
  #define PIN_BUZZER       -1
  #define PIN_TP_SDA       33
  #define PIN_TP_SCL       32
  #define LED_ACTIVE       LOW
  #define LED_INACTIVE     HIGH
#endif

// =========================================================================
// GLOBAL STATE & OBJECTS
// =========================================================================
TFT_eSPI tft = TFT_eSPI();
MbgUiEngine uiEngine(tft);

MbgProfileId currentProfile = MBG_PROFILE_AYAM_BROILER;
MbgSensorReading currentReading;
MbgInferenceResult currentResult;
uint32_t lastInferenceMicros = 0;
bool hasSelectedProfile = false;

struct MbgInspectionSnapshot {
  bool valid;
  MbgProfileId profile;
  MbgSensorReading reading;
  MbgInferenceResult result;
  uint32_t timestampMs;
  uint32_t inferenceMicros;
};
MbgInspectionSnapshot lastInspection = {};

uint32_t lastProbeUpdateMs = 0;
uint32_t splashStartMs = 0;
bool autoDemoMode = false;
uint32_t lastDemoStepMs = 0;
int demoStepIndex = 0;
bool touchWasDown = false;
bool touchCalibrated = false;

void calibrateDisplayTouch(bool force = false) {
#if defined(TOUCH_CS)
  Preferences preferences;
  uint16_t calibration[5] = {};
  bool storageReady = preferences.begin("mbg-touch", false);
  bool saved = storageReady && preferences.getBytesLength("landscape") == sizeof(calibration) &&
               preferences.getBytes("landscape", calibration, sizeof(calibration)) == sizeof(calibration);
  if (force || !saved || calibration[1] == 0 || calibration[3] == 0) {
    tft.fillScreen(0xF7DF);
    tft.setTextColor(0x1947, 0xF7DF);
    tft.drawCentreString("KALIBRASI SENTUH", 160, 70, 2);
    tft.drawCentreString("Sentuh tanda di empat sudut layar", 160, 104, 1);
    Serial.println("Kalibrasi XPT2046: sentuh empat tanda sudut layar.");
    tft.calibrateTouch(calibration, 0x2BE6, 0xF7DF, 15);
    if (storageReady && preferences.putBytes("landscape", calibration, sizeof(calibration)) != sizeof(calibration)) {
      Serial.println("Kalibrasi aktif; penyimpanan gagal, ulangi setelah restart.");
    }
  }
  tft.setTouch(calibration);
  touchCalibrated = true;
  if (storageReady) preferences.end();
#else
  (void)force;
#endif
}

// =========================================================================
// TOUCH CONTROLLER DETECTION & READING
// =========================================================================
#define CST816_I2C_ADDR 0x15

bool readTouchCoords(uint16_t &tx, uint16_t &ty) {
#if defined(MBG_TFT_NO_TOUCH)
  return false;
#else
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  #if defined(TOUCH_CS)
  if (touchCalibrated && tft.getTouch(&tx, &ty)) {
    return true;
  }
  #endif
  return false;
#else
  // Guition JC2432W328C Capacitive Touch CST820 / CST816 over I2C
  Wire.beginTransmission(CST816_I2C_ADDR);
  Wire.write(0x02);
  if (Wire.endTransmission(false) != 0) return false;

  if (Wire.requestFrom((uint8_t)CST816_I2C_ADDR, (uint8_t)5) != 5) return false;
  uint8_t data[5];
  for (uint8_t &value : data) value = Wire.read();
  return mbgCstLandscapeTouch(data, tx, ty);
#endif
#endif
}

// =========================================================================
// AUDIT LOG & SERIAL RECEIPT GENERATOR
// =========================================================================
void printAuditLogReceipt() {
  if (!lastInspection.valid) {
    Serial.println("Belum ada hasil inspeksi. Pilih bahan dan jalankan evaluasi.");
    return;
  }
  const MbgProfileConfig* cfg = mbg_get_profile_config(lastInspection.profile);
  const MbgInferenceResult& result = lastInspection.result;
  const MbgSensorReading& reading = lastInspection.reading;
  const char* ingName = cfg ? cfg->indonesian_name : "BAHAN MBG";
  const char* sniStd = cfg ? cfg->sni_standard : "SNI STANDAR";

  Serial.println();
  Serial.println("==================================================================");
  Serial.println("  BADAN GIZI NASIONAL (BGN) - AUDIT RECEIPT INSPEKSI CCP-1");
  Serial.println("==================================================================");
  Serial.printf("  Bahan Pangan : %s\n", ingName);
  Serial.printf("  Standar SNI  : %s\n", sniStd);
  if (result.classification == MBG_CLASS_INDETERMINATE) {
    Serial.println("  Skor Mutu    : -- (ulangi pengukuran)");
  } else {
    Serial.printf("  Skor Mutu    : %.1f %%\n", result.freshness_score);
  }
  Serial.printf("  Status CCP-1 : %s\n", result.sni_status);
  Serial.printf("  Diagnosis    : %s\n", result.diagnosis);
  Serial.printf("  Waktu Uji    : %u mikrodetik\n", (unsigned int)lastInspection.inferenceMicros);

  if (cfg && cfg->mode == MBG_MODE_LIQUID_COLLOID) {
    Serial.printf("  Sensor Probe : pH=%.2f | Konduktivitas=%.2f mS/cm | Suhu=%.1f C\n",
                  reading.ph_val, reading.cond_mscm, reading.temp_c);
  } else {
    Serial.printf("  Sensor Probe : R_eis=%.2f | Fasa=%.1f deg | NH3=%.2f ppm | Suhu=%.1f C\n",
                  reading.r_eis, reading.phase_deg, reading.nh3_ppm, reading.temp_c);
  }

  if (result.histamine_hazard_alert) {
    Serial.println("  [PERINGATAN BAHAYA] Terdeteksi akumulasi histamin ikan scombroid!");
  }
  Serial.println("------------------------------------------------------------------");
  char scoreJson[16];
  if (result.classification == MBG_CLASS_INDETERMINATE) {
    snprintf(scoreJson, sizeof(scoreJson), "null");
  } else {
    snprintf(scoreJson, sizeof(scoreJson), "%.1f", result.freshness_score);
  }
  Serial.printf("JSON: {\"timestamp_ms\":%lu,\"ingredient\":\"%s\",\"sni\":\"%s\",\"score\":%s,\"verdict\":\"%s\",\"latency_us\":%u}\n",
                (unsigned long)lastInspection.timestampMs, ingName, sniStd, scoreJson, result.sni_status, (unsigned int)lastInspection.inferenceMicros);
  Serial.println("==================================================================");
  Serial.println();
}

// =========================================================================
// RUN FULL COMPUTATIONAL BENCHMARK (12 SCENARIOS)
// =========================================================================
void runFullComputationalBenchmark() {
  Serial.println();
  Serial.println("==================================================================");
  Serial.println("BENCHMARK KOMPUTASI ESP32 (7 BAHAN PANGAN MBG):");
  Serial.println("==================================================================");

  struct BenchItem {
    MbgProfileId profile;
    float r_eis; float phase; float temp; float nh3; float cond; float ph;
    const char* label;
  } items[] = {
    { MBG_PROFILE_AYAM_BROILER,   2.45f, -15.5f, 4.0f,  2.0f, 0.0f, 0.0f,  "Ayam Broiler Segar" },
    { MBG_PROFILE_AYAM_BROILER,   1.08f,  -2.5f, 4.0f,  1.5f, 0.0f, 0.0f,  "Ayam Beku-Lumer (Freeze-Thaw)" },
    { MBG_PROFILE_AYAM_BROILER,   1.10f,  -2.0f, 5.0f, 35.0f, 0.0f, 0.0f,  "Ayam Busuk Bakteri (High NH3)" },
    { MBG_PROFILE_DAGING_SAPI,    2.10f, -14.0f, 4.0f,  3.0f, 0.0f, 0.0f,  "Daging Sapi Segar" },
    { MBG_PROFILE_IKAN_NILA_LELE, 2.50f, -16.0f, 3.0f,  1.0f, 0.0f, 0.0f,  "Ikan Nila Segar" },
    { MBG_PROFILE_IKAN_TONGKOL,   1.45f,  -5.5f,15.0f, 22.0f, 0.0f, 0.0f,  "Ikan Tongkol (Histamin Alert!)" },
    { MBG_PROFILE_TEMPE_KEDELAI,  2.10f, -15.0f, 4.0f,  2.5f, 0.0f, 0.0f,  "Tempe Kedelai Segar" },
    { MBG_PROFILE_TEMPE_KEDELAI,  1.12f,  -2.0f, 4.0f, 32.0f, 0.0f, 0.0f,  "Tempe Busuk (Over-fermented)" },
    { MBG_PROFILE_TAHU_PUTIH,     1.85f, -13.0f, 4.0f,  1.8f, 0.0f, 0.0f,  "Tahu Putih Segar" },
    { MBG_PROFILE_TAHU_PUTIH,     1.08f,  -1.5f, 4.0f, 24.0f, 0.0f, 0.0f,  "Tahu Basi (Likuifaksi Lendir)" },
    { MBG_PROFILE_SUSU_SEGAR,     0.00f,   0.0f, 4.0f,  0.0f, 4.8f, 6.60f, "Susu Sapi Segar" },
    { MBG_PROFILE_SUSU_SEGAR,     0.00f,   0.0f, 4.0f,  0.0f, 8.5f, 4.75f, "Susu Asam Basi (Menggumpal)" }
  };

  size_t count = sizeof(items) / sizeof(items[0]);
  for (size_t i = 0; i < count; ++i) {
    uint32_t t0 = micros();
    MbgInferenceResult r;
    if (items[i].profile != MBG_PROFILE_SUSU_SEGAR) {
      r = mbg_evaluate_solid(items[i].profile, items[i].r_eis, items[i].phase, items[i].temp, items[i].nh3);
    } else {
      r = mbg_evaluate_liquid(items[i].cond, items[i].ph, items[i].temp);
    }
    uint32_t el = micros() - t0;
    Serial.printf("[%02d] %-32s | Skor: %5.1f%% | Status: %-22s | Waktu: %2u us\n",
                  (int)(i + 1), items[i].label, r.freshness_score, r.sni_status, (unsigned int)el);
  }
  Serial.println("==================================================================");
}

// =========================================================================
// FUZZY INFERENCE EVALUATION TRIGGER
// =========================================================================
void executeFuzzyEvaluation() {
  if (!hasSelectedProfile) {
    Serial.println("Pilih bahan pangan sebelum evaluasi.");
    return;
  }
  mbg_mock_sensors_sample(currentProfile, &currentReading);
  uint32_t tStart = micros();
  const MbgProfileConfig* cfg = mbg_get_profile_config(currentProfile);

  if (cfg && cfg->mode == MBG_MODE_LIQUID_COLLOID) {
    currentResult = mbg_evaluate_liquid(currentReading.cond_mscm, currentReading.ph_val, currentReading.temp_c);
  } else {
    currentResult = mbg_evaluate_solid(currentProfile, currentReading.r_eis, currentReading.phase_deg,
                                       currentReading.temp_c, currentReading.nh3_ppm);
  }
  lastInferenceMicros = micros() - tStart;
  lastInspection = {true, currentProfile, currentReading, currentResult, millis(), lastInferenceMicros};

  // Set hardware LEDs
  if (currentResult.classification == MBG_CLASS_FRESH) {
    digitalWrite(PIN_LED_PASS, LED_ACTIVE);
    digitalWrite(PIN_LED_FAIL, LED_INACTIVE);
  } else if (currentResult.classification == MBG_CLASS_SPOILED ||
             currentResult.classification == MBG_CLASS_INDETERMINATE) {
    digitalWrite(PIN_LED_PASS, LED_INACTIVE);
    digitalWrite(PIN_LED_FAIL, LED_ACTIVE);
  } else {
    digitalWrite(PIN_LED_PASS, LED_ACTIVE);
    digitalWrite(PIN_LED_FAIL, LED_ACTIVE);
  }

  // Render Verdict Screen
  uiEngine.setState(UI_STATE_HACCP_VERDICT);
  uiEngine.renderHaccpVerdict(cfg, currentResult, lastInferenceMicros);
  printAuditLogReceipt();
}

// =========================================================================
// SERIAL COMMAND PARSER
// =========================================================================
void printSerialHelp() {
  Serial.println("\n--- MBG Freshness Terminal Interactive Serial Commands ---");
  Serial.println(" '1'..'7' : Pilih Bahan (1:Ayam, 2:Sapi, 3:Nila, 4:Tongkol, 5:Tempe, 6:Tahu, 7:Susu)");
  Serial.println(" 'f'      : Simulasi SEGAR (Fresh Standard)");
  Serial.println(" 's'      : Simulasi BUSUK (Spoiled Bacterial)");
  Serial.println(" 'z'      : Simulasi BEKU-LUMER (Freeze-Thaw Cell Rupture)");
  Serial.println(" 'h'      : Simulasi HISTAMIN RISK (Tongkol Scombroid Alert)");
  Serial.println(" 'e'/' '  : EVALUASI CCP-1 (Jalankan Defuzzifikasi & Tampilkan Hasil)");
  Serial.println(" 'r'      : KEMBALI ke Menu Pemilihan Bahan Pangan");
  Serial.println(" 'a'      : Cetak Ulang AUDIT RECEIPT JSON");
  Serial.println(" 'b'      : Jalankan 12-Skenario Computational BENCHMARK");
  Serial.println(" 'd'      : Toggle AUTO-DEMO Cycle Mode");
#if defined(TOUCH_CS)
  Serial.println(" 't'      : Kalibrasi ulang layar sentuh (empat sudut)");
#endif
  Serial.println(" '?'      : Tampilkan Bantuan Ini");
  Serial.println("----------------------------------------------------------\n");
}

void handleSerialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c <= '7') {
      autoDemoMode = false;
      currentProfile = (MbgProfileId)(c - '1');
      hasSelectedProfile = true;
      uiEngine.setState(UI_STATE_LIVE_PROBE);
      mbg_mock_sensors_sample(currentProfile, &currentReading);
      uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
      Serial.printf(">> Dipilih: %s\n", mbg_get_profile_config(currentProfile)->indonesian_name);
    } else if (c == 'f' || c == 'F') {
      mbg_mock_sensors_set_scenario(MOCK_SCENARIO_FRESH);
      Serial.println(">> Skenario diset: SEGAR (FRESH)");
      if (uiEngine.getState() == UI_STATE_LIVE_PROBE) {
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
      }
    } else if (c == 's' || c == 'S') {
      mbg_mock_sensors_set_scenario(MOCK_SCENARIO_SPOILED);
      Serial.println(">> Skenario diset: BUSUK (SPOILED)");
      if (uiEngine.getState() == UI_STATE_LIVE_PROBE) {
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
      }
    } else if (c == 'z' || c == 'Z') {
      mbg_mock_sensors_set_scenario(MOCK_SCENARIO_FREEZE_THAW);
      Serial.println(">> Skenario diset: BEKU-LUMER (FREEZE-THAW)");
      if (uiEngine.getState() == UI_STATE_LIVE_PROBE) {
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
      }
    } else if (c == 'h' || c == 'H') {
      mbg_mock_sensors_set_scenario(MOCK_SCENARIO_HISTAMINE_RISK);
      Serial.println(">> Skenario diset: HISTAMIN RISK");
      if (uiEngine.getState() == UI_STATE_LIVE_PROBE) {
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
      }
    } else if (c == 'e' || c == 'E' || c == ' ') {
      executeFuzzyEvaluation();
    } else if (c == 'r' || c == 'R') {
      hasSelectedProfile = false;
      uiEngine.setState(UI_STATE_INGREDIENT_SELECT);
      uiEngine.renderIngredientSelect();
      Serial.println(">> Kembali ke Menu Utama.");
    } else if (c == 'a' || c == 'A') {
      printAuditLogReceipt();
    } else if (c == 'b' || c == 'B') {
      runFullComputationalBenchmark();
    } else if (c == 'd' || c == 'D') {
      autoDemoMode = !autoDemoMode;
      Serial.printf(">> Auto-Demo Cycle Mode: %s\n", autoDemoMode ? "ON" : "OFF");
    } else if (c == 't' || c == 'T') {
      autoDemoMode = false;
      hasSelectedProfile = false;
      calibrateDisplayTouch(true);
      uiEngine.setState(UI_STATE_INGREDIENT_SELECT);
      uiEngine.renderIngredientSelect();
    } else if (c == '?') {
      printSerialHelp();
    }
  }
}

// =========================================================================
// ARDUINO SETUP & LOOP
// =========================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  // Pin setup
  #if PIN_TFT_BL >= 0
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH); // Backlight on
  #endif

  #if defined(PIN_TFT_BL2) && (PIN_TFT_BL2 >= 0)
  pinMode(PIN_TFT_BL2, OUTPUT);
  digitalWrite(PIN_TFT_BL2, HIGH); // Backlight 2 on (CYD)
  #endif

  #if defined(PIN_TP_RST) && (PIN_TP_RST >= 0)
  pinMode(PIN_TP_RST, OUTPUT);
  digitalWrite(PIN_TP_RST, LOW);
  delay(20);
  digitalWrite(PIN_TP_RST, HIGH);
  delay(50);
  #endif

  pinMode(PIN_LED_PASS, OUTPUT);
  pinMode(PIN_LED_FAIL, OUTPUT);
  digitalWrite(PIN_LED_PASS, LED_INACTIVE);
  digitalWrite(PIN_LED_FAIL, LED_INACTIVE);
#if defined(PIN_LED_BLUE)
  pinMode(PIN_LED_BLUE, OUTPUT);
  digitalWrite(PIN_LED_BLUE, LED_INACTIVE);
  // Keep the unused SD device deselected on the shared display SPI bus.
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
#endif

  // I2C for Capacitive Touch CST820
  Wire.begin(PIN_TP_SDA, PIN_TP_SCL);
  Wire.setTimeOut(30);
#if defined(MBG_TFT_GUITION) && !defined(MBG_TFT_NO_TOUCH)
  Wire.beginTransmission(CST816_I2C_ADDR);
  bool touchPresent = Wire.endTransmission() == 0;
  Serial.printf("[MBG] GUITION CST touch 0x15: %s (SDA=33, SCL=32)\n",
                touchPresent ? "detected" : "not detected; use Serial controls");
#endif

  // TFT Display Init
  tft.init();
  uiEngine.init();
  calibrateDisplayTouch();
  mbg_mock_sensors_init();

  Serial.println();
  Serial.println("==================================================================");
  Serial.println("  BADAN GIZI NASIONAL (BGN) - SATUAN PELAYANAN PEMENUHAN GIZI (SPPG)");
#if defined(MBG_TFT_GUITION)
  Serial.println("  GUITION JC2432W328C - ESP32 LIGHT TFT TERMINAL");
#else
  Serial.println("  ESP32-S3 EMBEDDED MULTI-SENSOR FRESHNESS INSPECTION TERMINAL");
#endif
  Serial.println("  Display: 320x240 SPI TFT | Engine: Verified C99 Fuzzy Logic");
  Serial.println("==================================================================");
  printSerialHelp();

  // Show Boot Splash
  uiEngine.setState(UI_STATE_BOOT_SPLASH);
  uiEngine.renderBootSplash();
  splashStartMs = millis();
}

void loop() {
  uint32_t now = millis();

  // 1. Process Serial Commands
  handleSerialCommands();

  // 2. Touch Screen Handling
  uint16_t tx, ty;
  bool touching = readTouchCoords(tx, ty);
  if (touching && !touchWasDown) {
    MbgUiState state = uiEngine.getState();

    if (state == UI_STATE_BOOT_SPLASH) {
      uiEngine.setState(UI_STATE_INGREDIENT_SELECT);
      uiEngine.renderIngredientSelect();
    } else if (state == UI_STATE_INGREDIENT_SELECT) {
      int item = uiEngine.hitTestIngredient(tx, ty);
      if (item >= 0 && item < 7) {
        autoDemoMode = false;
        currentProfile = (MbgProfileId)item;
        hasSelectedProfile = true;
        uiEngine.setState(UI_STATE_LIVE_PROBE);
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
      } else if (item == 7) {
        // Demo cycle toggle
        autoDemoMode = !autoDemoMode;
        uiEngine.showFlashToast(autoDemoMode ? "DEMO MODE: ON" : "DEMO MODE: OFF");
        delay(300);
        uiEngine.renderIngredientSelect();
      }
    } else if (state == UI_STATE_LIVE_PROBE) {
      if (uiEngine.hitTestProbeBack(tx, ty)) {
        autoDemoMode = false;
        hasSelectedProfile = false;
        uiEngine.setState(UI_STATE_INGREDIENT_SELECT);
        uiEngine.renderIngredientSelect();
      } else if (uiEngine.hitTestProbeEval(tx, ty)) {
        executeFuzzyEvaluation();
      } else if (uiEngine.hitTestScenarioToggle(tx, ty)) {
        mbg_mock_sensors_next_scenario();
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
      }
    } else if (state == UI_STATE_HACCP_VERDICT) {
      if (uiEngine.hitTestVerdictRepeat(tx, ty)) {
        autoDemoMode = false;
        uiEngine.setState(UI_STATE_LIVE_PROBE);
        mbg_mock_sensors_sample(currentProfile, &currentReading);
        uiEngine.renderLiveProbe(currentProfile, currentReading, 0);
      } else if (uiEngine.hitTestVerdictMenu(tx, ty)) {
        autoDemoMode = false;
        hasSelectedProfile = false;
        uiEngine.setState(UI_STATE_INGREDIENT_SELECT);
        uiEngine.renderIngredientSelect();
      } else if (uiEngine.hitTestVerdictAudit(tx, ty)) {
        printAuditLogReceipt();
        uiEngine.showFlashToast("LOG DIKIRIM KE SERIAL", TFT_GREEN);
        delay(400);
        const MbgProfileConfig* cfg = mbg_get_profile_config(currentProfile);
        uiEngine.renderHaccpVerdict(cfg, currentResult, lastInferenceMicros);
      }
    }
  }
  touchWasDown = touching; // One action per press; require release before another action.

  // 3. Splash Screen Auto-Advance (after 2.5 seconds)
  if (uiEngine.getState() == UI_STATE_BOOT_SPLASH && (now - splashStartMs > 2500)) {
    uiEngine.setState(UI_STATE_INGREDIENT_SELECT);
    uiEngine.renderIngredientSelect();
  }

  // 4. Live Sensor Update in Probe Mode (4 Hz; redraw only changed cards)
  if (uiEngine.getState() == UI_STATE_LIVE_PROBE && (now - lastProbeUpdateMs >= 250)) {
    lastProbeUpdateMs = now;
    mbg_mock_sensors_sample(currentProfile, &currentReading);
    uiEngine.renderLiveProbe(currentProfile, currentReading, lastInferenceMicros);
  }

  // 5. Auto Demo Mode Stepping
  if (autoDemoMode && (now - lastDemoStepMs >= 3000)) {
    lastDemoStepMs = now;
    demoStepIndex = (demoStepIndex + 1) % 12;

    const MbgProfileId demoProfiles[12] = {
      MBG_PROFILE_AYAM_BROILER, MBG_PROFILE_AYAM_BROILER, MBG_PROFILE_AYAM_BROILER,
      MBG_PROFILE_DAGING_SAPI, MBG_PROFILE_IKAN_NILA_LELE, MBG_PROFILE_IKAN_TONGKOL,
      MBG_PROFILE_TEMPE_KEDELAI, MBG_PROFILE_TEMPE_KEDELAI, MBG_PROFILE_TAHU_PUTIH,
      MBG_PROFILE_TAHU_PUTIH, MBG_PROFILE_SUSU_SEGAR, MBG_PROFILE_SUSU_SEGAR
    };
    const MockScenarioType demoScenarios[12] = {
      MOCK_SCENARIO_FRESH, MOCK_SCENARIO_FREEZE_THAW, MOCK_SCENARIO_SPOILED,
      MOCK_SCENARIO_FRESH, MOCK_SCENARIO_FRESH, MOCK_SCENARIO_HISTAMINE_RISK,
      MOCK_SCENARIO_FRESH, MOCK_SCENARIO_SPOILED, MOCK_SCENARIO_FRESH,
      MOCK_SCENARIO_SPOILED, MOCK_SCENARIO_FRESH, MOCK_SCENARIO_SPOILED
    };

    currentProfile = demoProfiles[demoStepIndex];
    hasSelectedProfile = true;
    mbg_mock_sensors_set_scenario(demoScenarios[demoStepIndex]);
    mbg_mock_sensors_sample(currentProfile, &currentReading);
    executeFuzzyEvaluation();
  }

  delay(20);
}
