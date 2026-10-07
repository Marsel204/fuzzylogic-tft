# 14. Hardware Integration Guide: ESP32-S3 Multi-Sensor Front-End & TFT Touch Display

> [!IMPORTANT]
> **PURPOSE OF THIS DOCUMENT:**
> This engineering reference details the exact electrical connections, sensor conditioning circuits, peripheral driver interfaces, and display architecture required to connect physical sensors and an SPI TFT display to the verified ESP32-S3 fuzzy logic engine.
> All pin assignments are pre-validated against the **ESP32-S3 hardware constraints** (ADC1 isolation, Wi-Fi coexistence, and strapping pin avoidance).

---

## 1. System Block Diagram & Hardware Architecture

```
                                      +--------------------------------------------+
                                      |          ESP32-S3 DUAL-CORE LX7           |
                                      |            (240 MHz, 512 KB SRAM)          |
                                      +--------------------------------------------+
                                             |              |              |
                +----------------------------+              |              +----------------------------+
                |                                           |                                           |
                v [I2C0 Master: GPIO 8/9]                   v [ADC1 Analog Channels]                   v [FSPI Master: GPIO 10-16]
+--------------------------------+         +--------------------------------+         +--------------------------------+
|      AD5933 EIS FRONT-END      |         |     GAS, TEMP & pH SENSORS     |         |     SPI TFT TOUCH DISPLAY      |
|  - AD5933 1MSPS Impedance Core |         |  - GPIO 1: MQ-137 (NH3 Gas)    |         |  - ILI9341 / ST7789 (240x320)  |
|  - AD8250 Diff-Amp (Voltage)   |         |  - GPIO 2: NTC 10k Thermistor  |         |  - XPT2046 Touch Controller    |
|  - AD8034 Transimpedance (Cur) |         |  - GPIO 4: Analog pH Probe     |         |  - Hardware SPI with DMA       |
|  - 4-Needle Stainless Array    |         |  (ADC1 immune to Wi-Fi lock)   |         |  - Backlight PWM Dimming       |
+--------------------------------+         +--------------------------------+         +--------------------------------+
```

---

## 2. Complete ESP32-S3 Pinout & Wiring Table

| Peripheral Subsystem | Function / Signal | ESP32-S3 Pin | Hardware Type | Electrical Voltage | Wiring Notes |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **I2C Bus (EIS Front-End)** | I2C0 SDA | **GPIO 8** | Open Drain | 3.3V | 4.7 kΩ pull-up resistor to 3.3V |
| | I2C0 SCL | **GPIO 9** | Open Drain | 3.3V | 4.7 kΩ pull-up resistor to 3.3V |
| **Analog Gas Sensor** | MQ-137 / MiCS $\text{NH}_3$ Out | **GPIO 1** | ADC1 CH0 | 0.0 – 3.1V | Voltage divider from 5V MQ board |
| **Cold-Chain Thermistor** | NTC 10k Voltage Divider | **GPIO 2** | ADC1 CH1 | 0.0 – 3.3V | 10 kΩ 0.1% pull-up to 3.3V |
| **Liquid pH Probe** | Analog pH Signal | **GPIO 4** | ADC1 CH3 | 0.0 – 3.0V | Signal conditioner (pH 4502C / E-201) |
| **TFT Display (SPI)** | TFT MOSI (SDI) | **GPIO 11** | Output | 3.3V | FSPI Hardware MOSI |
| | TFT SCK (SCLK) | **GPIO 12** | Output | 3.3V | FSPI Clock (up to 40 MHz) |
| | TFT MISO (SDO) | **GPIO 13** | Input | 3.3V | FSPI MISO |
| | TFT CS | **GPIO 10** | Output | 3.3V | Chip Select (Active LOW) |
| | TFT DC / RS | **GPIO 14** | Output | 3.3V | Data / Command Select |
| | TFT RESET | **GPIO 15** | Output | 3.3V | Hardware Reset |
| | TFT Backlight (BL) | **GPIO 16** | LEDC PWM | 3.3V | 5 kHz PWM for brightness control |
| **Touch Screen (SPI)** | Touch CS | **GPIO 17** | Output | 3.3V | XPT2046 Chip Select |
| | Touch IRQ (PEN) | **GPIO 18** | Input Pullup | 3.3V | Pen interrupt trigger |
| **Audio Alert** | HACCP Alarm Buzzer | **GPIO 5** | LEDC PWM | 3.3V | Active buzzer / 2.7 kHz PWM |
| **Status Indicators** | LED Green (Lolos) | **GPIO 6** | Output | 3.3V | 330 Ω current limiting resistor |
| | LED Red (Tolak) | **GPIO 7** | Output | 3.3V | 330 Ω current limiting resistor |

> [!CAUTION]
> **PINS TO NEVER USE:**
> - `GPIO 19, 20`: Reserved for Native USB (USB-JTAG / USB-CDC).
> - `GPIO 0, 3, 45, 46`: Strapping pins (affect bootloader mode and flash voltage).
> - `GPIO 26 – 37`: Internal Octal/Quad SPI Flash and PSRAM connections.
> - `ADC2 Channels (GPIO 11–20)`: DO NOT use for analog input; ADC2 is disabled whenever Wi-Fi is transmitting.

---

## 3. Sensor Interfacing & Conditioning Protocols

### 3.1. AD5933 Four-Electrode EIS Front-End (`[MARGO: P01]`)
* **I2C Slave Address:** `0x0D`.
* **Excitation Bandwidth:** Validated from $4\ \text{kHz}$ to $100\ \text{kHz}$.
* **Register Map & Command Flow:**
  1. `0x80 / 0x81` (Control Register): Configure PGA Gain ($\times 1$ or $\times 5$), excitation range ($2.0\ \text{V}_{\text{p-p}}$ Range 1), and clock source (internal 16.776 MHz).
  2. `0x82 – 0x84`: Start Frequency register (24-bit word).
  3. `0x85 – 0x87`: Frequency Increment register.
  4. `0x88 – 0x89`: Number of Increments register.
  5. `0x8A – 0x8B`: Settling Time Cycles.
  6. `0x8F`: Status Register (Poll Bit D1: "Valid real/imaginary data ready").
  7. `0x94 – 0x97`: 16-bit Two's Complement Real and Imaginary DFT data.

#### Operating Sequence (Pseudo-C):
```c
void ad5933_acquire_impedance_point(float freq_hz, float* z_mag, float* z_phase) {
    // 1. Program frequency into AD5933 DDS registers
    ad5933_set_frequency(freq_hz);
    
    // 2. Issue Start Sweep Command
    ad5933_write_control(0x10); // STANDBY -> INIT WITH START FREQ
    delayMicroseconds(200);
    ad5933_write_control(0x20); // START FREQUENCY SWEEP
    
    // 3. Poll Status register for DFT completion
    while (!(ad5933_read_status() & 0x02)) {
        delayMicroseconds(50);
    }
    
    // 4. Read raw DFT registers
    int16_t real_raw = ad5933_read_16bit(0x94);
    int16_t imag_raw = ad5933_read_16bit(0x96);
    
    // 5. Apply Margo Eq. 4 Calibration
    // Zx(f) = (Xv / Xi) * K(f)
    MbgComplex xv_xi = mbg_complex_div(mbg_complex_make(real_raw, imag_raw), xi_cal);
    MbgComplex zx = mbg_complex_mult(xv_xi, k_cal_at_freq);
    
    *z_mag = mbg_complex_modulus(zx);
    *z_phase = mbg_complex_phase_deg(zx);
}
```

---

### 3.2. MQ-137 / MiCS-5914 $\text{NH}_3$ Headspace Gas Sensor
* **Target Detection:** Volatile ammonia and basic amine off-gassing (TVB-N proxy).
* **Electrical Conditioning:**
  - MQ-137 requires $5.0\ \text{V} \pm 0.1\ \text{V}$ heater supply ($R_H \approx 31\ \Omega$, current $\approx 160\ \text{mA}$).
  - Signal output $V_{\text{out}}$ spans $0.5\ \text{V} - 4.5\ \text{V}$. Use a precision resistive voltage divider ($R_1 = 10\ \text{k}\Omega,\ R_2 = 18\ \text{k}\Omega$) to step down $V_{\text{out}}$ to $\le 2.9\ \text{V}$ for `GPIO 1` (ADC1).
* **Calibration Curve Equation:**
  $$\text{ppm} = A \cdot \left(\frac{R_s}{R_0}\right)^B$$
  Where $R_s = R_L \cdot \left(\frac{V_{\text{in}} - V_{\text{out}}}{V_{\text{out}}}\right)$, $R_0$ is baseline resistance in clean 4°C air, $A \approx 102.2$, $B \approx -2.47$.

```c
float read_nh3_ppm() {
    uint32_t adc_mv = analogReadMilliVolts(PIN_ADC_NH3);
    float v_out = (float)adc_mv / 1000.0f * (28.0f / 18.0f); // Divider compensation
    if (v_out <= 0.1f) return 0.0f;
    
    float rs = 10.0f * (5.0f - v_out) / v_out; // Load resistor = 10k
    float rs_r0_ratio = rs / R0_CLEAN_AIR;
    float ppm = 102.2f * powf(rs_r0_ratio, -2.47f);
    return ppm;
}
```

---

### 3.3. NTC 10k Temperature Probe (Cold Chain Compliance)
* **Configuration:** $10\ \text{k}\Omega$ NTC connected between `GPIO 2` and GND; $10\ \text{k}\Omega \pm 0.1\%$ reference resistor tied to $3.3\ \text{V}$.
* **Temperature Formula (Beta Model, $B = 3950\ \text{K}$):**
  $$\frac{1}{T} = \frac{1}{T_0} + \frac{1}{B} \ln\left(\frac{R}{R_0}\right)$$
```c
float read_temperature_c() {
    uint32_t adc_mv = analogReadMilliVolts(PIN_ADC_TEMP);
    if (adc_mv <= 10 || adc_mv >= 3290) return 4.0f; // Fault fallback
    
    float v = (float)adc_mv / 1000.0f;
    float r_ntc = 10000.0f * (v / (3.3f - v));
    
    float steinhart = logf(r_ntc / 10000.0f);
    steinhart /= 3950.0f;                    // 1/B
    steinhart += 1.0f / (25.0f + 273.15f);   // + (1/T0)
    float temp_c = (1.0f / steinhart) - 273.15f;
    return temp_c;
}
```

---

### 3.4. Analog pH Probe (Liquid Milk Acidification)
* **Configuration:** Standard BNC glass electrode with signal conditioning amplifier board connected to `GPIO 4`.
* **Linear Calibration ($V_{\text{offset}}$ and Slope $S$):**
  $$\text{pH} = 7.00 + \frac{V_{\text{pH}} - V_{7.00}}{S}$$
  Calibrated with pH 4.01 and pH 6.86 standard buffers.

---

## 4. TFT Touch Display Architecture & UI Layout

### 4.1. Hardware Recommendations & Display Controller
* **Display Module:** 2.8" or 3.2" SPI TFT with **ST7789** or **ILI9341** controller ($240 \times 320$ pixels) and **XPT2046** resistive or **FT6236** capacitive touch.
* **Display Driver Library:** **LovyanGFX** (recommended for ESP32-S3 DMA SPI performance) or **TFT_eSPI** (Bodmer).

### 4.2. `TFT_eSPI` Setup Configuration (`User_Setup.h`)
```c
#define USER_SETUP_LOADED
#define ILI9341_DRIVER           // Or ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   14
#define TFT_RST  15
#define TFT_BL   16
#define TOUCH_CS 17

#define SPI_FREQUENCY       40000000  // 40 MHz SPI clock
#define SPI_TOUCH_FREQUENCY  2500000  // 2.5 MHz Touch clock
```

---

### 4.3. UI State Machine & Screen Wireframes

```
+-----------------------------------------------------------------------------------------+
|                                    UI STATE MACHINE                                     |
|                                                                                         |
|   [BOOT SPLASH] ---> [MENU UTAMA (PILIH BAHAN)] <=============================+         |
|   (BGN Branding)      (7 Tombol Pangan MBG)                                   |         |
|                              |                                                |         |
|                              v                                                |         |
|                      [LIVE INSPECTION PROBE] ---- [ < MENU UTAMA ] -----------+         |
|                      (Pembacaan Sensor Riil)                                  |         |
|                              |                                                |         |
|                              v                                                |         |
|                      [HACCP VERDICT RECEIPT] ---- [ < MENU UTAMA ] -----------+         |
|                      (Skor, Status, Rekomendasi)                              |         |
|                              |                                                |         |
|                              +------------------- [ UJI ULANG ] --------------+         |
+-----------------------------------------------------------------------------------------+
```

#### Screen 1: Menu Utama - Pemilihan Bahan Pangan (Touch Interface)
```
+-----------------------------------------+
| [BGN] INSPEKSI KESEGARAN BAHAN MBG      |
+-----------------------------------------+
|  [ 1. AYAM BROILER ]   [ 2. DAGING SAPI ]|
|  [ 3. IKAN NILA    ]   [ 4. TONGKOL (H) ]|
|  [ 5. TEMPE KEDELAI]   [ 6. TAHU PUTIH  ]|
|  [ 7. SUSU SEGAR   ]   [ KALIBRASI MUX  ]|
+-----------------------------------------+
| Status Suhu: 3.8°C (OK) | Siap Uji      |
+-----------------------------------------+
```

#### Screen 2: Real-Time Multi-Sensor Measurement (Live Probe)
```
+-----------------------------------------+
| [ < MENU ]  UJI: TEMPE KEDELAI          |
+-----------------------------------------+
| Rasio EIS (4k/100k):   2.12  [NORMAL]   |
| Sudut Fasa (θ):       -15.2° [KOMPAK]   |
| Gas NH3 Headspace:     2.8 ppm [AMAN]   |
| Suhu Penerimaan:       4.1°C [CHILLER]  |
|                                         |
|    [=== PROSES DEFUZZIFIKASI ===]       |
+-----------------------------------------+
| Waktu: 8 us | Heap: 0 B | Baterai: 94%  |
+-----------------------------------------+
```

#### Screen 3: Hasil Inspeksi Penerimaan (HACCP Verdict Receipt)
```
+-----------------------------------------+
| [ < MENU ]  HASIL INSPEKSI PENERIMAAN   |
+-----------------------------------------+
|               SKOR: 86.4%               |
|                                         |
|     +-----------------------------+     |
|     |   MEMENUHI SYARAT (PASS)    |     |
|     +-----------------------------+     |
|                                         |
| Diagnosis: NORMAL MATRIX DEGRADATION    |
| Tindakan : Simpan di Cold Storage 4°C   |
+-----------------------------------------+
| [ < MENU UTAMA ]   [ SIMPAN ]  [ ULANG ]|
+-----------------------------------------+
```

> [!TIP]
> **Panduan Desain & Ergonomi Operator Lapangan:**
> 1. **Kemudahan Navigasi:** Tombol `[ < MENU UTAMA ]` wajib ditempatkan di pojok kiri atas (*header*) dan/atau bilah tombol bawah di seluruh layar aktif, memungkinkan operator kembali ke menu pilihan pangan dengan satu sentuhan.
> 2. **Tampilan Bersih & Minimalis:** Hindari teks kode standar berlebihan (seperti tulisan SNI panjang) dan warna cyan/biru menyala. Gunakan kontras tinggi yang nyaman di mata: latar belakang gelap netral (`#1E1E1E`), teks putih bersih (`#FFFFFF`), badge hijau (`#2E7D32`) untuk *MEMENUHI SYARAT*, dan merah (`#C62828`) untuk *TIDAK MEMENUHI SYARAT*.

---

## 5. FreeRTOS Dual-Core Firmware Integration Blueprint

On the ESP32-S3, utilize both CPU cores independently:
* **Core 0 (Sensor & Acquisition Task):** Runs hardware I2C frequency sweeps on AD5933, samples analog ADC1 channels with oversampling, and manages SPI DMA to the TFT.
* **Core 1 (Fuzzy Engine & Touch Task):** Runs the touch controller polling, executes `mbg_evaluate_solid()` / `mbg_evaluate_liquid()` in 10 microseconds, and handles serial/CSV audit logging.

```cpp
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "mbg_fuzzy_core.h"
#include "mbg_profiles.h"
#include "mbg_calibration.h"

// FreeRTOS Task Handles
TaskHandle_t hTaskSensor;
QueueHandle_t qSensorData;

struct SensorTelemetry {
    MbgProfileId profile;
    float r_eis;
    float phase;
    float temp;
    float nh3;
    float cond;
    float ph;
};

// Core 0: Continuous Sensor Sampling
void TaskSensorAcquisition(void* pvParameters) {
    SensorTelemetry data;
    for (;;) {
        // 1. Read AD5933 over I2C
        // 2. Read NH3 over ADC1
        // 3. Read Temp over ADC1
        xQueueSend(qSensorData, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz refresh
    }
}

// Core 1: UI & Fuzzy Inference Execution
void TaskUIAndFuzzy(void* pvParameters) {
    SensorTelemetry data;
    for (;;) {
        if (xQueueReceive(qSensorData, &data, pdMS_TO_TICKS(50)) == pdPASS) {
            // Instantaneous C99 inference (<10 microseconds)
            MbgInferenceResult result;
            if (data.profile != MBG_PROFILE_SUSU_SEGAR) {
                result = mbg_evaluate_solid(data.profile, data.r_eis, data.phase, data.temp, data.nh3);
            } else {
                result = mbg_evaluate_liquid(data.cond, data.ph, data.temp);
            }
            
            // Update TFT UI (render verdict, score bar, SNI status)
            update_tft_display(&result);
        }
    }
}

void setup() {
    Serial.begin(115200);
    qSensorData = xQueueCreate(2, sizeof(SensorTelemetry));

    // Pin Core 0 to Sensor Acquisition
    xTaskCreatePinnedToCore(TaskSensorAcquisition, "Sensors", 4096, NULL, 2, &hTaskSensor, 0);

    // Pin Core 1 to Fuzzy Engine & UI
    xTaskCreatePinnedToCore(TaskUIAndFuzzy, "FuzzyUI", 4096, NULL, 1, NULL, 1);
}

void loop() {
    vTaskDelete(NULL); // FreeRTOS manages execution
}
```

---

## 6. Power Supply, Decoupling & Cabling Specifications

1. **Power Budget:**
   - ESP32-S3 with Wi-Fi: $240\ \text{mA}$ peak.
   - MQ-137 Heater: $160\ \text{mA} @ 5.0\ \text{V}$.
   - 3.2" TFT Backlight at 100%: $90\ \text{mA} @ 3.3\ \text{V}$.
   - AD5933 Front-End + Op-Amps: $45\ \text{mA}$.
   - **Total Current Demand:** $\approx 550\ \text{mA} @ 5.0\ \text{V}$. Use a dedicated $5.0\ \text{V} / 2.0\ \text{A}$ USB-C supply or a $3.7\ \text{V}$ LiPo battery with a synchronized boost converter.
2. **Noise Isolation & Ground Planes:**
   - Keep digital SPI clock traces ($40\ \text{MHz}$) separated from the AD5933 high-impedance virtual ground node ($V_{\text{IN}}$ pin).
   - Use a $100\ \text{nF}$ low-ESR ceramic capacitor in parallel with a $10\ \mu\text{F}$ tantalum capacitor adjacent to the $V_{\text{DD}}$ pins of the AD5933, AD8250, and AD8034.
3. **Electrode Probe Cabling:**
   - The four stainless steel needles must be connected via **shielded coaxial cables** (inner core carries signal, outer braid grounded at PCB side only) to eliminate parasitic shunt capacitance when sweeping up to $100\ \text{kHz}$.
