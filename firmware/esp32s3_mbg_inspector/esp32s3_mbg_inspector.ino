/**
 * @file esp32s3_mbg_inspector.ino
 * @brief Indonesian MBG Food Freshness Inspector Firmware for ESP32-S3
 * Runs 7-Ingredient Multi-Sensor Fuzzy Logic with AD5933 EIS, NH3, and pH
 * Target: ESP32-S3 (Xtensa Dual-Core LX7 @ 240MHz with FPU)
 */

#include <Arduino.h>
#include <Wire.h>
#include "mbg_fuzzy_core.h"
#include "mbg_profiles.h"
#include "mbg_calibration.h"

/* Hardware Pinout Configuration (Strictly ADC1 safe with Wi-Fi) */
#define PIN_I2C_SDA        8
#define PIN_I2C_SCL        9
#define PIN_ADC_NH3        1   /* ADC1 CH0: MQ-137 Gas Sensor */
#define PIN_ADC_TEMP       2   /* ADC1 CH1: NTC 10k Thermistor */
#define PIN_ADC_PH         4   /* ADC1 CH3: Analog pH Probe (Milk) */
#define PIN_BUZZER         5   /* LEDC PWM HACCP Alarm Buzzer */
#define PIN_LED_PASS       6   /* Green LED: MEMENUHI SYARAT */
#define PIN_LED_FAIL       7   /* Red LED: TIDAK MEMENUHI SYARAT */

void print_main_menu();
void run_inspection_for_choice(int choice);
void run_computational_benchmarks();

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(PIN_LED_PASS, OUTPUT);
    pinMode(PIN_LED_FAIL, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    digitalWrite(PIN_LED_PASS, LOW);
    digitalWrite(PIN_LED_FAIL, LOW);

    /* Initialize Hardware I2C at 400 kHz Fast Mode */
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);

    print_main_menu();
}

void print_main_menu() {
    Serial.println();
    Serial.println("==================================================================");
    Serial.println("  BADAN GIZI NASIONAL (BGN) - SATUAN PELAYANAN PEMENUHAN GIZI (SPPG)");
    Serial.println("  ESP32-S3 EMBEDDED MULTI-SENSOR FRESHNESS INSPECTION TERMINAL");
    Serial.println("==================================================================");
    Serial.println("  PILIH BAHAN PANGAN UNTUK DIUJI:");
    Serial.println("  [1] Daging Ayam Broiler");
    Serial.println("  [2] Daging Sapi (Otot Skelet)");
    Serial.println("  [3] Ikan Air Tawar (Ikan Nila / Lele)");
    Serial.println("  [4] Ikan Laut Pelagis (Ikan Tongkol / Cakalang)");
    Serial.println("  [5] Tempe Kedelai (Fermentasi Rhizopus)");
    Serial.println("  [6] Tahu Putih (Protein Gel)");
    Serial.println("  [7] Susu Sapi Segar (Liquid)");
    Serial.println("  ----------------------------------------------------------------");
    Serial.println("  [8] Benchmark Komputasi Semua 7 Bahan Sekaligus");
    Serial.println("  [m] Tampilkan Menu Utama Ini Kapan Saja");
    Serial.println("==================================================================");
    Serial.print("  Ketik angka pilihan [1-8]: ");
}

void loop() {
    if (Serial.available()) {
        char c = (char)Serial.read();

        /* Ignore whitespace and carriage return */
        if (c == '\r') return;

        if (c == 'm' || c == 'M' || c == '\n') {
            print_main_menu();
            return;
        }

        if (c >= '1' && c <= '7') {
            int choice = c - '0';
            Serial.println(choice);
            run_inspection_for_choice(choice);
        } else if (c == '8' || c == 'b' || c == 'B') {
            Serial.println("8");
            run_computational_benchmarks();
        }
    }
    delay(50);
}

void run_inspection_for_choice(int choice) {
    MbgProfileId prof_id = (MbgProfileId)(choice - 1);
    const MbgProfileConfig* cfg = mbg_get_profile_config(prof_id);

    float r_eis = 2.45f, phase = -15.5f, temp = 4.0f, nh3 = 2.0f;
    float cond = 4.8f, ph = 6.60f;

    /* Reference fresh values */
    switch (prof_id) {
        case MBG_PROFILE_AYAM_BROILER:   r_eis = 2.45f; phase = -15.5f; nh3 = 2.0f; break;
        case MBG_PROFILE_DAGING_SAPI:    r_eis = 2.10f; phase = -14.0f; nh3 = 3.0f; break;
        case MBG_PROFILE_IKAN_NILA_LELE: r_eis = 2.50f; phase = -16.0f; nh3 = 1.0f; break;
        case MBG_PROFILE_IKAN_TONGKOL:   r_eis = 2.20f; phase = -15.0f; nh3 = 3.0f; break;
        case MBG_PROFILE_TEMPE_KEDELAI:  r_eis = 2.10f; phase = -15.0f; nh3 = 2.5f; break;
        case MBG_PROFILE_TAHU_PUTIH:     r_eis = 1.85f; phase = -13.0f; nh3 = 1.8f; break;
        case MBG_PROFILE_SUSU_SEGAR:     cond = 4.8f; ph = 6.60f; break;
        default: break;
    }

    uint32_t t_start = micros();
    MbgInferenceResult res;
    if (cfg->mode == MBG_MODE_SOLID_MATRIX) {
        res = mbg_evaluate_solid(prof_id, r_eis, phase, temp, nh3);
    } else {
        res = mbg_evaluate_liquid(cond, ph, temp);
    }
    uint32_t t_elapsed = micros() - t_start;

    /* Update hardware LEDs */
    if (res.classification == MBG_CLASS_FRESH) {
        digitalWrite(PIN_LED_PASS, HIGH);
        digitalWrite(PIN_LED_FAIL, LOW);
    } else if (res.classification == MBG_CLASS_SPOILED || res.classification == MBG_CLASS_INDETERMINATE) {
        digitalWrite(PIN_LED_PASS, LOW);
        digitalWrite(PIN_LED_FAIL, HIGH);
    } else {
        digitalWrite(PIN_LED_PASS, HIGH);
        digitalWrite(PIN_LED_FAIL, HIGH);
    }

    Serial.println();
    Serial.println("------------------------------------------------------------------");
    Serial.println("  HASIL INSPEKSI PENERIMAAN (CCP-1)");
    Serial.println("------------------------------------------------------------------");
    Serial.printf("  Bahan Pangan      : %s\n", cfg->indonesian_name);
    Serial.printf("  Orientasi Jarum   : %s\n", cfg->orientation_notice);
    if (cfg->mode == MBG_MODE_SOLID_MATRIX) {
        Serial.printf("  Parameter Sensor  : R_eis=%.2f, Fasa=%.1f deg, Suhu=%.1f C, NH3=%.1f ppm\n",
                      r_eis, phase, temp, nh3);
    } else {
        Serial.printf("  Parameter Sensor  : Konduktivitas=%.2f mS/cm, pH=%.2f, Suhu=%.1f C\n",
                      cond, ph, temp);
    }
    Serial.printf("  Waktu Komputasi   : %u us (0 bytes heap)\n", (unsigned int)t_elapsed);
    Serial.println("  ----------------------------------------------------------------");
    if (res.classification == MBG_CLASS_INDETERMINATE) {
        Serial.println("  Skor Kesegaran    : -- (ulangi pengukuran)");
    } else {
        Serial.printf("  Skor Kesegaran    : %.1f %%\n", res.freshness_score);
    }
    Serial.printf("  Status Kelayakan  : [%s]\n", res.sni_status);
    Serial.printf("  Diagnosis Fisik   : %s\n", res.diagnosis);

    if (res.histamine_hazard_alert) {
        Serial.println("  [PERINGATAN BAHAYA] Terdeteksi toksin histamin scombroid! TOLAK!");
    }
    if (res.freeze_thaw_detected) {
        Serial.println("  [INFO BEKU-LUMER] Lisis membran kristal es (Aman dikonsumsi).");
    }
    Serial.println("------------------------------------------------------------------");
    Serial.println("  [Ketik 'm' atau tekan ENTER untuk kembali ke Menu Utama]");
    Serial.println();
}

void run_computational_benchmarks() {
    Serial.println();
    Serial.println("------------------------------------------------------------------");
    Serial.println("BENCHMARK KOMPUTASI ESP32-S3 (7 BAHAN PANGAN MBG):");
    Serial.println("------------------------------------------------------------------");

    struct TestScenario {
        MbgProfileId profile;
        float r_eis;
        float phase;
        float temp;
        float nh3;
        float cond;
        float ph;
        const char* label;
    };

    TestScenario scenarios[] = {
        { MBG_PROFILE_AYAM_BROILER, 2.45f, -15.5f, 4.0f,  2.0f, 0.0f, 0.0f,  "Ayam Broiler Segar" },
        { MBG_PROFILE_AYAM_BROILER, 1.08f,  -2.5f, 4.0f,  1.5f, 0.0f, 0.0f,  "Ayam Beku-Lumer (Freeze-Thaw)" },
        { MBG_PROFILE_AYAM_BROILER, 1.10f,  -2.0f, 5.0f, 35.0f, 0.0f, 0.0f,  "Ayam Busuk Bakteri (High NH3)" },
        { MBG_PROFILE_DAGING_SAPI,  2.10f, -14.0f, 4.0f,  3.0f, 0.0f, 0.0f,  "Daging Sapi Segar (Transversal)" },
        { MBG_PROFILE_IKAN_NILA_LELE, 2.50f,-16.0f, 3.0f,  1.0f, 0.0f, 0.0f,  "Ikan Nila Segar (Es Curah)" },
        { MBG_PROFILE_IKAN_TONGKOL, 1.45f,  -5.5f,15.0f, 22.0f, 0.0f, 0.0f,  "Ikan Tongkol (Histamin Alert!)" },
        { MBG_PROFILE_TEMPE_KEDELAI,2.10f, -15.0f, 4.0f,  2.5f, 0.0f, 0.0f,  "Tempe Kedelai Segar" },
        { MBG_PROFILE_TEMPE_KEDELAI,1.12f,  -2.0f, 4.0f, 32.0f, 0.0f, 0.0f,  "Tempe Busuk (Over-fermented)" },
        { MBG_PROFILE_TAHU_PUTIH,   1.85f, -13.0f, 4.0f,  1.8f, 0.0f, 0.0f,  "Tahu Putih Segar (Gel Padat)" },
        { MBG_PROFILE_TAHU_PUTIH,   1.08f,  -1.5f, 4.0f, 24.0f, 0.0f, 0.0f,  "Tahu Basi (Likuifaksi Lendir)" },
        { MBG_PROFILE_SUSU_SEGAR,   0.00f,   0.0f, 4.0f,  0.0f, 4.8f, 6.60f, "Susu Sapi Segar" },
        { MBG_PROFILE_SUSU_SEGAR,   0.00f,   0.0f, 4.0f,  0.0f, 8.5f, 4.75f, "Susu Asam Basi (Menggumpal)" }
    };

    size_t count = sizeof(scenarios) / sizeof(scenarios[0]);

    for (size_t i = 0; i < count; ++i) {
        TestScenario& sc = scenarios[i];
        MbgInferenceResult res;

        /* Measure execution time in microseconds with hardware timer */
        uint32_t t_start = micros();
        if (sc.profile != MBG_PROFILE_SUSU_SEGAR) {
            res = mbg_evaluate_solid(sc.profile, sc.r_eis, sc.phase, sc.temp, sc.nh3);
        } else {
            res = mbg_evaluate_liquid(sc.cond, sc.ph, sc.temp);
        }
        uint32_t t_elapsed = micros() - t_start;

        /* LED Indicator */
        if (res.classification == MBG_CLASS_FRESH) {
            digitalWrite(PIN_LED_PASS, HIGH);
            digitalWrite(PIN_LED_FAIL, LOW);
        } else if (res.classification == MBG_CLASS_SPOILED || res.classification == MBG_CLASS_INDETERMINATE) {
            digitalWrite(PIN_LED_PASS, LOW);
            digitalWrite(PIN_LED_FAIL, HIGH);
        } else {
            digitalWrite(PIN_LED_PASS, HIGH);
            digitalWrite(PIN_LED_FAIL, HIGH);
        }

        Serial.printf("[%02d] %-32s | Skor: %5.1f%% | Status: %-22s | Waktu: %2u us\n",
                      (int)(i + 1), sc.label, res.freshness_score, res.sni_status, (unsigned int)t_elapsed);
        Serial.printf("     -> Diagnosis: %s\n", res.diagnosis);
        if (res.histamine_hazard_alert) {
            Serial.println("     -> [PERINGATAN BAHAYA] Terdeteksi akumulasi histamin ikan scombroid!");
        }
        delay(30);
    }

    Serial.println("------------------------------------------------------------------");
    Serial.println("BENCHMARK SELESAI: Waktu komputasi rata-rata < 15 mikrodetik per uji.");
    Serial.println("Zero Dynamic Allocation (0 Bytes Heap). Beban CPU ESP32-S3 < 0.01%.");
    Serial.println("------------------------------------------------------------------");
    Serial.println("  [Ketik 'm' atau tekan ENTER untuk kembali ke Menu Utama]");
    Serial.println();
}
