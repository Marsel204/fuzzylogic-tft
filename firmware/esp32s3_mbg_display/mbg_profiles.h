/**
 * @file mbg_profiles.h
 * @brief MBG 7-Ingredient Profiles and Complete Multi-Sensor Inference Engine
 * Zero-Dynamic-Allocation, MISRA-C Compliant for ESP32-S3
 */

#ifndef MBG_PROFILES_H
#define MBG_PROFILES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mbg_fuzzy_core.h"

typedef enum {
    MBG_PROFILE_AYAM_BROILER = 0,
    MBG_PROFILE_DAGING_SAPI,
    MBG_PROFILE_IKAN_NILA_LELE,
    MBG_PROFILE_IKAN_TONGKOL,
    MBG_PROFILE_TEMPE_KEDELAI,
    MBG_PROFILE_TAHU_PUTIH,
    MBG_PROFILE_SUSU_SEGAR,
    MBG_PROFILE_COUNT
} MbgProfileId;

typedef enum {
    MBG_MODE_SOLID_MATRIX = 0,
    MBG_MODE_LIQUID_COLLOID = 1
} MbgOperatingMode;

typedef struct {
    MbgProfileId id;
    const char* code_name;
    const char* indonesian_name;
    const char* sni_standard;
    MbgOperatingMode mode;
    float f_lf;
    float f_hf;
    float f_char;
    float nh3_limit_ppm;
    bool anisotropy_safe;
    const char* orientation_notice;
} MbgProfileConfig;

/**
 * @brief Get profile configuration by ID
 */
const MbgProfileConfig* mbg_get_profile_config(MbgProfileId profile_id);

/**
 * @brief Find profile ID by string code name
 */
MbgProfileId mbg_profile_from_string(const char* name);

/**
 * @brief Evaluate freshness for solid muscle / plant matrices
 * Supports: Ayam, Sapi, Ikan Nila, Ikan Tongkol, Tempe, Tahu
 * @param profile_id Selected MBG ingredient profile
 * @param impedance_ratio Dual-frequency ratio R_eis = |Z_lf| / |Z_hf|
 * @param phase_angle Characteristic phase angle in degrees (e.g. -15.0 deg)
 * @param temperature_c Temperature in Celsius (e.g. 4.0 C)
 * @param nh3_ppm Measured ammonia concentration in ppm; NAN when unavailable
 * @return MbgInferenceResult crisp score, diagnosis, and SNI status
 */
MbgInferenceResult mbg_evaluate_solid(
    MbgProfileId profile_id,
    float impedance_ratio,
    float phase_angle,
    float temperature_c,
    float nh3_ppm
);

/**
 * @brief Evaluate freshness for liquid dairy (Susu Sapi Segar)
 * Mapped to SNI 3141.1:2011
 * @param conductivity_mscm Bulk electrolytic conductivity in mS/cm
 * @param ph_value Measured milk pH (nominal 6.30 - 6.80); NAN when unavailable
 * @param temperature_c Temperature in Celsius (nominal 4.0 C)
 * @return MbgInferenceResult crisp score, diagnosis, and SNI status
 */
MbgInferenceResult mbg_evaluate_liquid(
    float conductivity_mscm,
    float ph_value,
    float temperature_c
);

#ifdef __cplusplus
}
#endif

#endif /* MBG_PROFILES_H */
