/**
 * @file mbg_mock_sensors.c
 * @brief Implementation of Mock Sensor Generator
 */

#include "mbg_mock_sensors.h"
#include <string.h>
#include <math.h>

static MockScenarioType s_current_scenario = MOCK_SCENARIO_FRESH;
static uint32_t s_sample_step = 0;

void mbg_mock_sensors_init(void) {
    s_current_scenario = MOCK_SCENARIO_FRESH;
    s_sample_step = 0;
}

void mbg_mock_sensors_set_scenario(MockScenarioType scenario) {
    if (scenario < MOCK_SCENARIO_COUNT) {
        s_current_scenario = scenario;
    }
}

void mbg_mock_sensors_next_scenario(void) {
    s_current_scenario = (MockScenarioType)((s_current_scenario + 1) % MOCK_SCENARIO_COUNT);
}

const char* mbg_mock_sensors_get_scenario_name(MockScenarioType scenario) {
    switch (scenario) {
        case MOCK_SCENARIO_FRESH:          return "SEGAR (FRESH)";
        case MOCK_SCENARIO_SPOILED:        return "BUSUK (SPOILED)";
        case MOCK_SCENARIO_FREEZE_THAW:    return "BEKU-LUMER (FT)";
        case MOCK_SCENARIO_HISTAMINE_RISK: return "HISTAMIN ALERT";
        default:                           return "CUSTOM";
    }
}

void mbg_mock_sensors_sample(MbgProfileId profile, MbgSensorReading* reading) {
    if (!reading) return;
    s_sample_step++;

    /* Small pseudo-random noise oscillation based on step counter */
    float noise1 = sinf((float)s_sample_step * 0.2f) * 0.03f;
    float noise2 = cosf((float)s_sample_step * 0.3f) * 0.15f;

    reading->profile_id = profile;
    reading->scenario = s_current_scenario;
    reading->scenario_name = mbg_mock_sensors_get_scenario_name(s_current_scenario);

    if (profile == MBG_PROFILE_SUSU_SEGAR) {
        /* Liquid Colloid Model */
        reading->r_eis = 0.0f;
        reading->phase_deg = 0.0f;
        reading->nh3_ppm = 0.0f;

        if (s_current_scenario == MOCK_SCENARIO_FRESH) {
            reading->cond_mscm = 4.80f + noise1;
            reading->ph_val = 6.62f + (noise1 * 0.5f);
            reading->temp_c = 4.0f + noise1;
        } else {
            /* Souring / curdled milk */
            reading->cond_mscm = 8.50f + noise1;
            reading->ph_val = 4.75f - (fabsf(noise1) * 0.3f);
            reading->temp_c = 5.2f + noise1;
        }
    } else {
        /* Solid Muscle / Plant Matrices */
        reading->cond_mscm = 0.0f;
        reading->ph_val = 0.0f;

        if (s_current_scenario == MOCK_SCENARIO_FRESH) {
            reading->temp_c = 4.0f + noise1;
            switch (profile) {
                case MBG_PROFILE_AYAM_BROILER:
                    reading->r_eis = 2.45f + noise1;
                    reading->phase_deg = -15.5f + noise2;
                    reading->nh3_ppm = 2.0f + fabsf(noise1 * 5.0f);
                    break;
                case MBG_PROFILE_DAGING_SAPI:
                    reading->r_eis = 2.10f + noise1;
                    reading->phase_deg = -14.0f + noise2;
                    reading->nh3_ppm = 3.0f + fabsf(noise1 * 4.0f);
                    break;
                case MBG_PROFILE_IKAN_NILA_LELE:
                    reading->r_eis = 2.50f + noise1;
                    reading->phase_deg = -16.0f + noise2;
                    reading->nh3_ppm = 1.0f + fabsf(noise1 * 3.0f);
                    break;
                case MBG_PROFILE_IKAN_TONGKOL:
                    reading->r_eis = 2.20f + noise1;
                    reading->phase_deg = -14.5f + noise2;
                    reading->nh3_ppm = 2.2f + fabsf(noise1 * 4.0f);
                    break;
                case MBG_PROFILE_TEMPE_KEDELAI:
                    reading->r_eis = 2.10f + noise1;
                    reading->phase_deg = -15.0f + noise2;
                    reading->nh3_ppm = 2.5f + fabsf(noise1 * 5.0f);
                    break;
                case MBG_PROFILE_TAHU_PUTIH:
                    reading->r_eis = 1.85f + noise1;
                    reading->phase_deg = -13.0f + noise2;
                    reading->nh3_ppm = 1.8f + fabsf(noise1 * 4.0f);
                    break;
                default:
                    reading->r_eis = 2.0f;
                    reading->phase_deg = -14.0f;
                    reading->nh3_ppm = 2.0f;
                    break;
            }
        } else if (s_current_scenario == MOCK_SCENARIO_FREEZE_THAW) {
            /* Ruptured cell membrane (low EIS) but low NH3 */
            reading->r_eis = 1.08f + noise1;
            reading->phase_deg = -2.5f + noise2;
            reading->temp_c = 4.0f + noise1;
            reading->nh3_ppm = 1.5f + fabsf(noise1 * 2.0f);
        } else if (s_current_scenario == MOCK_SCENARIO_HISTAMINE_RISK) {
            /* High temperature + significant NH3 off-gassing */
            reading->r_eis = 1.45f + noise1;
            reading->phase_deg = -5.5f + noise2;
            reading->temp_c = 15.0f + noise1;
            reading->nh3_ppm = 22.0f + fabsf(noise1 * 10.0f);
        } else {
            /* Bacterial Spoilage (High NH3 & Low EIS) */
            reading->r_eis = 1.10f + noise1;
            reading->phase_deg = -2.0f + noise2;
            reading->temp_c = 5.5f + noise1;
            reading->nh3_ppm = 35.0f + fabsf(noise1 * 12.0f);
        }
    }
}
