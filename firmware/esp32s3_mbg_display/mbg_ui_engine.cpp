/** 320x240 operator UI. Geometry is shared with touch hit testing. */
#include "mbg_ui_engine.h"
#include <math.h>
#include <string.h>

namespace {
constexpr uint16_t BG = 0xF7DF, CARD = 0xFFFF, BORDER = 0xE73C;
constexpr uint16_t TEXT = 0x1947, MUTED = 0x63B1, ON_ACCENT = 0xFFFF;
constexpr uint16_t GREEN = 0x2BE6, RED = 0xC145, AMBER = 0xA300;
constexpr uint16_t GREEN_BG = 0xD7BB, RED_BG = 0xFF1C, AMBER_BG = 0xFFD8;
const char* const NAMES[7] = {"AYAM BROILER", "DAGING SAPI", "IKAN NILA", "IKAN TONGKOL", "TEMPE", "TAHU", "SUSU SEGAR"};
const char* const SUBTITLES[8] = {"Daging unggas", "Otot sapi", "Ikan air tawar", "Risiko histamin", "Fermentasi kedelai", "Gel kedelai", "Mode cair", "12 kasus bergantian"};
const MbgUiRect MENU_BACK = {8, 4, 48, 24};
const MbgUiRect SCENARIO = {244, 4, 68, 24};
const MbgUiRect EVALUATE = {8, 190, 304, 38};
const MbgUiRect AUDIT = {8, 200, 148, 32};
const MbgUiRect REPEAT = {164, 200, 148, 32};

uint16_t statusColor(MbgClassification value) {
    return value == MBG_CLASS_FRESH ? GREEN : value == MBG_CLASS_SPOILED ? RED : AMBER;
}
const char* scenarioLabel(MockScenarioType value) {
    switch (value) {
        case MOCK_SCENARIO_FRESH: return "SEGAR";
        case MOCK_SCENARIO_FREEZE_THAW: return "LUMER";
        case MOCK_SCENARIO_HISTAMINE_RISK: return "RISIKO";
        default: return "BUSUK";
    }
}
void number(char* output, size_t size, float value, unsigned decimals) {
    if (!isfinite(value)) snprintf(output, size, "--");
    else snprintf(output, size, "%.*f", (int)decimals, value);
}
}

MbgUiEngine::MbgUiEngine(TFT_eSPI& display)
    : m_tft(display), m_state(UI_STATE_BOOT_SPLASH), m_activeProfile(MBG_PROFILE_AYAM_BROILER),
      m_dirty(true), m_scenario(MOCK_SCENARIO_COUNT) {
    memset(m_values, 0, sizeof(m_values));
    memset(m_badges, 0, sizeof(m_badges));
}

void MbgUiEngine::init() {
    m_tft.setRotation(1);
    m_tft.fillScreen(BG);
}
void MbgUiEngine::setState(MbgUiState state) {
    m_state = state;
    m_dirty = true;
}

void MbgUiEngine::drawPill(int x, int y, int w, int h, const char* text, uint16_t fg, uint16_t bg, uint16_t outline) {
    m_tft.fillRoundRect(x, y, w, h, 5, bg);
    if (outline) m_tft.drawRoundRect(x, y, w, h, 5, outline);
    m_tft.setTextColor(fg, bg);
    m_tft.drawCentreString(text, x + w / 2, y + (h - m_tft.fontHeight(1)) / 2, 1);
}
void MbgUiEngine::drawProgressBar(int x, int y, int w, int h, float value, uint16_t color) {
    m_tft.fillRect(x, y, w, h, BORDER);
    if (!isfinite(value)) return;
    int pixels = (int)(w * constrain(value, 0.0f, 1.0f));
    if (pixels > 0) m_tft.fillRect(x, y, pixels, h, color);
}
void MbgUiEngine::drawHeader(const char* title, const char* badge, uint16_t color) {
    m_tft.fillRect(0, 0, 320, 32, BG);
    m_tft.drawFastHLine(8, 31, 304, BORDER);
    m_tft.setTextColor(TEXT, BG);
    m_tft.drawString(title, 12, 7, 2);
    if (badge) drawPill(260, 6, 52, 20, badge, color, CARD);
}

void MbgUiEngine::renderBootSplash() {
    m_tft.fillScreen(BG);
    drawHeader("MBG / INSPEKSI", "DEMO", AMBER);
    m_tft.fillRoundRect(16, 48, 288, 116, 8, CARD);
    m_tft.drawCircle(160, 76, 16, GREEN);
    m_tft.drawLine(151, 76, 158, 83, GREEN);
    m_tft.drawLine(158, 83, 169, 69, GREEN);
    m_tft.setTextColor(TEXT, CARD);
    m_tft.drawCentreString("INSPEKSI KESEGARAN", 160, 105, 2);
    m_tft.setTextColor(MUTED, CARD);
    m_tft.drawCentreString("7 bahan pangan / satu alur pemeriksaan", 160, 132, 1);
    m_tft.setTextColor(AMBER, BG);
    m_tft.drawCentreString("Data sensor simulasi", 160, 181, 2);
    m_tft.setTextColor(MUTED, BG);
    m_tft.drawCentreString("Sentuh layar untuk memulai", 160, 214, 1);
}

void MbgUiEngine::renderIngredientSelect() {
    m_tft.fillScreen(BG);
    drawHeader("PILIH BAHAN", "DEMO", AMBER);
    m_tft.setTextColor(MUTED, BG);
    m_tft.drawString("Pilih bahan untuk memulai pemeriksaan", 10, 37, 1);
    for (int i = 0; i < 8; ++i) {
        MbgUiRect box = mbgIngredientRect(i);
        m_tft.fillRoundRect(box.x, box.y, box.w, box.h, 5, CARD);
        m_tft.drawRoundRect(box.x, box.y, box.w, box.h, 5, BORDER);
        m_tft.fillRect(box.x, box.y + 7, 3, box.h - 14, i == 3 ? RED : i == 7 ? AMBER : GREEN);
        m_tft.setTextColor(TEXT, CARD);
        m_tft.drawString(i == 7 ? "SIKLUS DEMO" : NAMES[i], box.x + 10, box.y + 3, 2);
        m_tft.setTextColor(MUTED, CARD);
        m_tft.drawString(SUBTITLES[i], box.x + 10, box.y + 23, 1);
    }
    m_tft.setTextColor(AMBER, BG);
    m_tft.drawString("Sensor simulasi / hasil untuk demonstrasi", 10, 218, 1);
}

void MbgUiEngine::drawSensorCard(int x, int y, int w, int h, const char* label, const char* value,
                               const char* badge, uint16_t color, float progress) {
    m_tft.fillRoundRect(x, y, w, h, 5, CARD);
    m_tft.drawRoundRect(x, y, w, h, 5, BORDER);
    m_tft.setTextColor(MUTED, CARD);
    m_tft.drawString(label, x + 8, y + 7, 1);
    m_tft.setTextColor(TEXT, CARD);
    m_tft.drawString(value, x + 8, y + 23, 4);
    drawPill(x + w - 60, y + 28, 52, 18, badge, color, BG);
    drawProgressBar(x + 8, y + h - 8, w - 16, 3, progress, color);
}

void MbgUiEngine::renderLiveProbe(MbgProfileId profile, const MbgSensorReading& reading, uint32_t evalMicros) {
    (void)evalMicros;
    bool redraw = m_dirty || profile != m_activeProfile || reading.scenario != m_scenario;
    m_activeProfile = profile;
    m_scenario = reading.scenario;
    const MbgProfileConfig* cfg = mbg_get_profile_config(profile);
    if (redraw) {
        m_tft.fillScreen(BG);
        drawPill(MENU_BACK.x, MENU_BACK.y, MENU_BACK.w, MENU_BACK.h, "< MENU", TEXT, CARD);
        m_tft.setTextColor(TEXT, BG);
        m_tft.drawString(NAMES[profile], 64, 7, 2);
        drawPill(SCENARIO.x, SCENARIO.y, SCENARIO.w, SCENARIO.h, scenarioLabel(reading.scenario),
                 reading.scenario == MOCK_SCENARIO_FRESH ? GREEN : AMBER, CARD);
        m_tft.drawFastHLine(8, 31, 304, BORDER);
        m_tft.fillRoundRect(EVALUATE.x, EVALUATE.y, EVALUATE.w, EVALUATE.h, 6, GREEN);
        m_tft.setTextColor(ON_ACCENT, GREEN);
        m_tft.drawCentreString("EVALUASI KESEGARAN", 160, 201, 2);
        m_tft.setTextColor(AMBER, BG);
        m_tft.drawString("DEMO / sentuh badge untuk ganti skenario", 8, 232, 1);
    }
    char ratioLabel[24];
    snprintf(ratioLabel, sizeof(ratioLabel), "RASIO EIS %dk/%dk", (int)(cfg->f_lf / 1000), (int)(cfg->f_hf / 1000));
    const bool liquid = cfg->mode == MBG_MODE_LIQUID_COLLOID;
    float values[4] = {liquid ? reading.cond_mscm : reading.r_eis,
                       liquid ? reading.ph_val : reading.phase_deg,
                       liquid ? reading.temp_c : reading.nh3_ppm, reading.temp_c};
    const char* labels[4] = {liquid ? "KONDUKTIVITAS mS/cm" : ratioLabel,
                             liquid ? "pH SUSU" : "SUDUT FASA (deg)",
                             liquid ? "SUHU (C)" : "GAS NH3 (ppm)",
                             liquid ? "SUMBER DATA" : "SUHU (C)"};
    const char* badges[4] = {liquid ? (values[0] <= 5.8f ? "NORMAL" : "TINGGI") : (values[0] >= 1.8f ? "UTUH" : "LISIS"),
                             liquid ? (values[1] >= 6.3f && values[1] <= 6.8f ? "NORMAL" : "BERUBAH") : (values[1] <= -10 ? "REAKTIF" : "DATAR"),
                             liquid ? (values[2] <= 7 ? "DINGIN" : "HANGAT") : (values[2] < cfg->nh3_limit_ppm ? "RENDAH" : "TINGGI"),
                             liquid ? "CAIR" : (values[3] <= 7 ? "DINGIN" : "HANGAT")};
    float progress[4] = {liquid ? (values[0] - 3) / 12 : (values[0] - 0.8f) / 2.4f,
                         liquid ? (values[1] - 3.5f) / 4.5f : fabsf(values[1]) / 25,
                         liquid ? values[2] / 25 : values[2] / 40, values[3] / 25};
    for (int i = 0; i < 4; ++i) {
        char value[16];
        if (liquid && i == 3) snprintf(value, sizeof(value), "DEMO");
        else number(value, sizeof(value), values[i], i == 0 || (liquid && i == 1) ? 2 : 1);
        const char* badge = !isfinite(values[i]) && !(liquid && i == 3) ? "ERROR" : badges[i];
        if (redraw || strcmp(value, m_values[i]) || strcmp(badge, m_badges[i])) {
            uint16_t color = strcmp(badge, "ERROR") == 0 || strcmp(badge, "TINGGI") == 0 ? RED : MUTED;
            drawSensorCard(i % 2 ? 164 : 8, i / 2 ? 112 : 40, 148, 66, labels[i], value, badge, color,
                           liquid && i == 3 ? 0 : progress[i]);
            snprintf(m_values[i], sizeof(m_values[i]), "%s", value);
            snprintf(m_badges[i], sizeof(m_badges[i]), "%s", badge);
        }
    }
    m_dirty = false;
}

void MbgUiEngine::renderHaccpVerdict(const MbgProfileConfig* cfg, const MbgInferenceResult& result, uint32_t evalMicros) {
    (void)cfg;
    (void)evalMicros;
    m_tft.fillScreen(BG);
    drawPill(MENU_BACK.x, MENU_BACK.y, MENU_BACK.w, MENU_BACK.h, "< MENU", TEXT, CARD);
    m_tft.setTextColor(TEXT, BG);
    m_tft.drawString("HASIL INSPEKSI", 64, 7, 2);
    drawPill(260, 6, 52, 20, "DEMO", AMBER, CARD);
    m_tft.drawFastHLine(8, 31, 304, BORDER);
    const bool unknown = result.classification == MBG_CLASS_INDETERMINATE || !isfinite(result.freshness_score);
    uint16_t accent = unknown ? AMBER : statusColor(result.classification);
    const char* badge = unknown ? "ULANGI" : result.classification == MBG_CLASS_FRESH ? "SEGAR" :
                        result.classification == MBG_CLASS_ACCEPTABLE ? "SEGERA OLAH" : "TOLAK";
    m_tft.fillRoundRect(8, 40, 304, 50, 6, CARD);
    m_tft.setTextColor(MUTED, CARD);
    m_tft.drawString("SKOR KESEGARAN", 18, 47, 1);
    char score[16];
    if (unknown) snprintf(score, sizeof(score), "--");
    else snprintf(score, sizeof(score), "%.1f%%", result.freshness_score);
    m_tft.setTextColor(TEXT, CARD);
    m_tft.drawString(score, 18, 61, 4);
    drawPill(182, 55, 118, 24, badge, ON_ACCENT, accent);
    // Boundaries match the inference engine: <=35 spoiled, >=65 fresh.
    m_tft.fillRect(8, 97, 106, 5, RED);
    m_tft.fillRect(114, 97, 91, 5, AMBER);
    m_tft.fillRect(205, 97, 107, 5, GREEN);
    if (!unknown) {
        int marker = 8 + (int)(299 * constrain(result.freshness_score / 100, 0.0f, 1.0f));
        m_tft.fillRect(marker, 94, 5, 11, TEXT);
    }
    uint16_t bannerBg = unknown || result.classification == MBG_CLASS_ACCEPTABLE ? AMBER_BG :
                        result.classification == MBG_CLASS_FRESH ? GREEN_BG : RED_BG;
    const char* status = unknown ? "BELUM DAPAT DITENTUKAN" : result.classification == MBG_CLASS_FRESH ? "BAHAN DALAM KONDISI BAIK" :
                         result.classification == MBG_CLASS_ACCEPTABLE ? "PERLU SEGERA DIOLAH" : "TOLAK BAHAN PANGAN";
    m_tft.fillRoundRect(8, 111, 304, 30, 5, bannerBg);
    m_tft.setTextColor(TEXT, bannerBg);
    m_tft.drawCentreString(status, 160, 118, 2);
    m_tft.setTextColor(MUTED, BG);
    const char* action = unknown ? "Periksa sensor dan ulangi pengukuran." : result.classification == MBG_CLASS_FRESH ? "Simpan dalam penyimpanan dingin." :
                         result.classification == MBG_CLASS_ACCEPTABLE ? "Pantau kondisi sebelum pengolahan." : "Pisahkan sampel; jangan digunakan.";
    m_tft.drawString(action, 12, 150, 1);
    const char* diagnosis = unknown ? "Data / aturan belum mencukupi." : result.histamine_hazard_alert ? "Risiko histamin: karantina sampel." :
                            result.freeze_thaw_detected ? "Kerusakan jaringan beku-lumer." :
                            result.diagnosis && strstr(result.diagnosis, "AMMONIA") ? "Kadar amonia tinggi." :
                            result.diagnosis && strstr(result.diagnosis, "ACID SOURING") ? "Perubahan keasaman susu." :
                            result.classification == MBG_CLASS_SPOILED ? "Terdapat indikasi penurunan mutu." : "Pola sensor sesuai penilaian model.";
    m_tft.setTextColor(accent, BG);
    m_tft.drawString(diagnosis, 12, 168, 1);
    m_tft.setTextColor(AMBER, BG);
    m_tft.drawString("DEMO / penilaian dari sensor simulasi", 12, 184, 1);
    m_tft.fillRoundRect(AUDIT.x, AUDIT.y, AUDIT.w, AUDIT.h, 5, CARD);
    m_tft.fillRoundRect(REPEAT.x, REPEAT.y, REPEAT.w, REPEAT.h, 5, GREEN);
    m_tft.setTextColor(TEXT, CARD);
    m_tft.drawCentreString("CETAK LOG", AUDIT.x + AUDIT.w / 2, 208, 2);
    m_tft.setTextColor(ON_ACCENT, GREEN);
    m_tft.drawCentreString("UJI ULANG", REPEAT.x + REPEAT.w / 2, 208, 2);
}

void MbgUiEngine::showFlashToast(const char* message, uint16_t color) {
    m_tft.fillRoundRect(30, 98, 260, 42, 6, CARD);
    m_tft.drawRoundRect(30, 98, 260, 42, 6, color);
    m_tft.setTextColor(TEXT, CARD);
    m_tft.drawCentreString(message, 160, 112, 2);
    m_dirty = true;
}
int MbgUiEngine::hitTestIngredient(uint16_t x, uint16_t y) {
    for (int i = 0; i < 8; ++i) if (mbgIngredientRect(i).contains(x, y)) return i;
    return -1;
}
bool MbgUiEngine::hitTestProbeEval(uint16_t x, uint16_t y) { return EVALUATE.contains(x, y); }
bool MbgUiEngine::hitTestProbeBack(uint16_t x, uint16_t y) { return MENU_BACK.contains(x, y); }
bool MbgUiEngine::hitTestVerdictMenu(uint16_t x, uint16_t y) { return MENU_BACK.contains(x, y); }
bool MbgUiEngine::hitTestVerdictAudit(uint16_t x, uint16_t y) { return AUDIT.contains(x, y); }
bool MbgUiEngine::hitTestVerdictRepeat(uint16_t x, uint16_t y) { return REPEAT.contains(x, y); }
bool MbgUiEngine::hitTestScenarioToggle(uint16_t x, uint16_t y) { return SCENARIO.contains(x, y); }
