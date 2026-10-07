/**
 * @file mbg_mock_sensors.h
 * @brief Realistic Mock Sensor Generator for Indonesian MBG Freshness Inspector
 * Simulates AD5933 Four-Electrode EIS, MQ-137 NH3 Gas, NTC 10k Temp, and pH Probe
 */

#ifndef MBG_MOCK_SENSORS_H
#define MBG_MOCK_SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "mbg_profiles.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MOCK_SCENARIO_FRESH = 0,
    MOCK_SCENARIO_SPOILED = 1,
    MOCK_SCENARIO_FREEZE_THAW = 2,
    MOCK_SCENARIO_HISTAMINE_RISK = 3,
    MOCK_SCENARIO_COUNT
} MockScenarioType;

typedef struct {
    MbgProfileId profile_id;
    MockScenarioType scenario;
    float r_eis;        /* Dual-frequency impedance ratio (4k/100k) */
    float phase_deg;    /* Characteristic phase angle in degrees */
    float temp_c;       /* Cold-chain reception temperature in Celsius */
    float nh3_ppm;      /* Headspace ammonia concentration in ppm */
    float cond_mscm;    /* Milk bulk electrolytic conductivity in mS/cm */
    float ph_val;       /* Milk pH probe reading */
    const char* scenario_name;
} MbgSensorReading;

/**
 * @brief Initialize the mock sensor subsystem
 */
void mbg_mock_sensors_init(void);

/**
 * @brief Set current test scenario
 */
void mbg_mock_sensors_set_scenario(MockScenarioType scenario);

/**
 * @brief Cycle to the next scenario
 */
void mbg_mock_sensors_next_scenario(void);

/**
 * @brief Get scenario label
 */
const char* mbg_mock_sensors_get_scenario_name(MockScenarioType scenario);

/**
 * @brief Generate a sensor reading with realistic continuous dynamics and noise
 * @param profile Active ingredient profile
 * @param reading Output struct to populate
 */
void mbg_mock_sensors_sample(MbgProfileId profile, MbgSensorReading* reading);

#ifdef __cplusplus
}
#endif

#endif /* MBG_MOCK_SENSORS_H */
