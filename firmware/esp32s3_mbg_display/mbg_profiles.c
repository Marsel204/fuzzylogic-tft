/**
 * @file mbg_profiles.c
 * @brief MBG 7-Ingredient Profiles & Complete Inference Implementation
 */

#include "mbg_profiles.h"
#include <string.h>
#include <math.h>

/* Master registry of MBG ingredient configurations */
static const MbgProfileConfig MBG_PROFILES[MBG_PROFILE_COUNT] = {
    {
        MBG_PROFILE_AYAM_BROILER,
        "ayam_broiler",
        "Daging Ayam Broiler (Karkas/Fillet)",
        "SNI 3924:2009 (Mutu Karkas dan Daging Ayam)",
        MBG_MODE_SOLID_MATRIX,
        2000.0f, 100000.0f, 10000.0f, 25.0f,
        true,
        "Standard insertion depth 8 mm"
    },
    {
        MBG_PROFILE_DAGING_SAPI,
        "daging_sapi",
        "Daging Sapi (Otot Skelet)",
        "SNI 3932:2008 (Mutu Karkas dan Daging Sapi)",
        MBG_MODE_SOLID_MATRIX,
        1000.0f, 100000.0f, 10000.0f, 20.0f,
        false,
        "Transverse probe orientation required (strictly perpendicular to muscle fibers to avoid 250% anisotropy error)"
    },
    {
        MBG_PROFILE_IKAN_NILA_LELE,
        "ikan_nila_lele",
        "Ikan Air Tawar (Ikan Nila & Ikan Lele)",
        "SNI 2729:2021 (Ikan Segar)",
        MBG_MODE_SOLID_MATRIX,
        4000.0f, 100000.0f, 10000.0f, 25.0f,
        true,
        "Standard insertion depth 8 mm"
    },
    {
        MBG_PROFILE_IKAN_TONGKOL,
        "ikan_tongkol",
        "Ikan Laut Pelagis (Ikan Tongkol / Cakalang)",
        "SNI 2729:2021 (Annex B Histamin Scombridae)",
        MBG_MODE_SOLID_MATRIX,
        4000.0f, 100000.0f, 10000.0f, 15.0f,
        true,
        "Standard insertion depth 8 mm"
    },
    {
        MBG_PROFILE_TEMPE_KEDELAI,
        "tempe_kedelai",
        "Tempe Kedelai (Fermentasi Rhizopus oligosporus)",
        "SNI 3142:2018 (Tempe Kedelai)",
        MBG_MODE_SOLID_MATRIX,
        4000.0f, 100000.0f, 10000.0f, 20.0f,
        true,
        "Standard insertion depth 8 mm"
    },
    {
        MBG_PROFILE_TAHU_PUTIH,
        "tahu_putih",
        "Tahu Putih / Tahu Kedelai (Protein Gel Matrix)",
        "SNI 01-3144-1998 (Tahu)",
        MBG_MODE_SOLID_MATRIX,
        2000.0f, 100000.0f, 10000.0f, 18.0f,
        true,
        "Standard insertion depth 8 mm"
    },
    {
        MBG_PROFILE_SUSU_SEGAR,
        "susu_segar",
        "Susu Sapi Segar (Liquid)",
        "SNI 3141.1:2011 (Susu Segar - Sapi)",
        MBG_MODE_LIQUID_COLLOID,
        10000.0f, 10000.0f, 10000.0f, 0.0f,
        true,
        "Tetrapolar immersion dip cell"
    }
};

const MbgProfileConfig* mbg_get_profile_config(MbgProfileId profile_id) {
    if (profile_id < 0 || profile_id >= MBG_PROFILE_COUNT) {
        return &MBG_PROFILES[MBG_PROFILE_AYAM_BROILER];
    }
    return &MBG_PROFILES[profile_id];
}

MbgProfileId mbg_profile_from_string(const char* name) {
    if (!name) return MBG_PROFILE_AYAM_BROILER;
    for (int i = 0; i < MBG_PROFILE_COUNT; ++i) {
        if (strcmp(MBG_PROFILES[i].code_name, name) == 0) {
            return (MbgProfileId)i;
        }
    }
    return MBG_PROFILE_AYAM_BROILER;
}

static inline float min2(float a, float b) { return (a < b) ? a : b; }
static inline float min3(float a, float b, float c) { return min2(min2(a, b), c); }
static inline float max2(float a, float b) { return (a > b) ? a : b; }

static MbgInferenceResult indeterminate_result(const char* diagnosis) {
    MbgInferenceResult result;
    result.freshness_score = NAN;
    result.classification = MBG_CLASS_INDETERMINATE;
    result.sni_status = "BELUM DAPAT DITENTUKAN (ULANGI PENGUKURAN)";
    result.diagnosis = diagnosis;
    result.histamine_hazard_alert = false;
    result.freeze_thaw_detected = false;
    return result;
}

MbgInferenceResult mbg_evaluate_solid(
    MbgProfileId profile_id,
    float impedance_ratio,
    float phase_angle,
    float temperature_c,
    float nh3_ppm
) {
    if (!isfinite(impedance_ratio) || !isfinite(phase_angle) ||
        !isfinite(temperature_c) || !isfinite(nh3_ppm) ||
        impedance_ratio <= 0.0f || nh3_ppm < 0.0f) {
        return indeterminate_result("MISSING OR INVALID SENSOR READINGS");
    }
    if (profile_id < 0 || profile_id >= MBG_PROFILE_COUNT) {
        profile_id = MBG_PROFILE_AYAM_BROILER;
    }
    const MbgProfileConfig* config = &MBG_PROFILES[profile_id];

    /* 1. Setup Profile-Specific Ratio MFs */
    MbgTrapezoidalMF ratio_low;
    MbgTriangularMF  ratio_med;
    MbgTrapezoidalMF ratio_high;
    float nh3_thresh = config->nh3_limit_ppm;

    switch (profile_id) {
        case MBG_PROFILE_AYAM_BROILER:
            ratio_low  = (MbgTrapezoidalMF){ 0.8f, 0.8f, 1.20f, 1.45f };
            ratio_med  = (MbgTriangularMF){ 1.30f, 1.75f, 2.20f };
            ratio_high = (MbgTrapezoidalMF){ 1.95f, 2.30f, 3.20f, 3.20f };
            break;
        case MBG_PROFILE_DAGING_SAPI:
            ratio_low  = (MbgTrapezoidalMF){ 0.8f, 0.8f, 1.18f, 1.40f };
            ratio_med  = (MbgTriangularMF){ 1.25f, 1.60f, 2.05f };
            ratio_high = (MbgTrapezoidalMF){ 1.85f, 2.15f, 3.20f, 3.20f };
            break;
        case MBG_PROFILE_IKAN_TONGKOL:
            ratio_low  = (MbgTrapezoidalMF){ 0.8f, 0.8f, 1.15f, 1.40f };
            ratio_med  = (MbgTriangularMF){ 1.25f, 1.65f, 2.10f };
            ratio_high = (MbgTrapezoidalMF){ 1.90f, 2.30f, 3.20f, 3.20f };
            break;
        case MBG_PROFILE_TEMPE_KEDELAI:
            ratio_low  = (MbgTrapezoidalMF){ 0.8f, 0.8f, 1.18f, 1.40f };
            ratio_med  = (MbgTriangularMF){ 1.25f, 1.65f, 2.05f };
            ratio_high = (MbgTrapezoidalMF){ 1.85f, 2.15f, 3.20f, 3.20f };
            break;
        case MBG_PROFILE_TAHU_PUTIH:
            ratio_low  = (MbgTrapezoidalMF){ 0.8f, 0.8f, 1.15f, 1.35f };
            ratio_med  = (MbgTriangularMF){ 1.20f, 1.50f, 1.85f };
            ratio_high = (MbgTrapezoidalMF){ 1.65f, 1.95f, 3.20f, 3.20f };
            break;
        default: /* Freshwater fish (Nila / Lele) */
            ratio_low  = (MbgTrapezoidalMF){ 0.8f, 0.8f, 1.15f, 1.40f };
            ratio_med  = (MbgTriangularMF){ 1.25f, 1.65f, 2.10f };
            ratio_high = (MbgTrapezoidalMF){ 1.90f, 2.30f, 3.20f, 3.20f };
            break;
    }

    /* 2. Common Phase & Temperature MFs */
    const MbgTrapezoidalMF phase_flat = { -5.0f, -3.0f, 0.0f, 0.0f };
    const MbgTriangularMF  phase_mod  = { -13.0f, -8.5f, -4.0f };
    const MbgTrapezoidalMF phase_prom = { -25.0f, -25.0f, -15.0f, -9.0f };

    const MbgTrapezoidalMF temp_cold  = { 0.0f, 0.0f, 4.0f, 7.0f };
    const MbgTriangularMF  temp_mild  = { 5.0f, 10.0f, 16.0f };
    const MbgTrapezoidalMF temp_warm  = { 13.0f, 18.0f, 25.0f, 25.0f };

    /* 3. Fuzzification */
    float u_r_low  = mbg_eval_trapezoidal(&ratio_low, impedance_ratio);
    float u_r_med  = mbg_eval_triangular(&ratio_med, impedance_ratio);
    float u_r_high = mbg_eval_trapezoidal(&ratio_high, impedance_ratio);

    float u_ph_flat = mbg_eval_trapezoidal(&phase_flat, phase_angle);
    float u_ph_mod  = mbg_eval_triangular(&phase_mod, phase_angle);
    float u_ph_prom = mbg_eval_trapezoidal(&phase_prom, phase_angle);

    float u_t_cold = mbg_eval_trapezoidal(&temp_cold, temperature_c);
    float u_t_mild = mbg_eval_triangular(&temp_mild, temperature_c);
    float u_t_warm = mbg_eval_trapezoidal(&temp_warm, temperature_c);

    /* 4. Evaluate 11 Rules */
    float act_fresh = 0.0f;
    float act_acc   = 0.0f;
    float act_spoil = 0.0f;

    /* Rule 1: HIGH ratio, PROMINENT phase, COLD temp -> FRESH */
    act_fresh = max2(act_fresh, min3(u_r_high, u_ph_prom, u_t_cold));
    /* Rule 2: HIGH ratio, PROMINENT phase, MILD temp -> ACCEPTABLE */
    act_acc   = max2(act_acc, min3(u_r_high, u_ph_prom, u_t_mild));
    /* Rule 3: HIGH ratio, MODERATE phase, COLD temp -> FRESH */
    act_fresh = max2(act_fresh, min3(u_r_high, u_ph_mod, u_t_cold));
    /* Rule 4: MEDIUM ratio, MODERATE phase, COLD temp -> ACCEPTABLE */
    act_acc   = max2(act_acc, min3(u_r_med, u_ph_mod, u_t_cold));
    /* Rule 5: MEDIUM ratio, PROMINENT phase, COLD temp -> FRESH */
    act_fresh = max2(act_fresh, min3(u_r_med, u_ph_prom, u_t_cold));
    /* Rule 6: MEDIUM ratio, FLAT phase -> SPOILED */
    act_spoil = max2(act_spoil, min2(u_r_med, u_ph_flat));
    /* Rule 7: LOW ratio -> SPOILED */
    act_spoil = max2(act_spoil, u_r_low);
    /* Rule 8: LOW ratio, FLAT phase -> SPOILED */
    act_spoil = max2(act_spoil, min2(u_r_low, u_ph_flat));
    /* Rule 9: FLAT phase, WARM temp -> SPOILED */
    act_spoil = max2(act_spoil, min2(u_ph_flat, u_t_warm));
    /* Rule 10: MEDIUM ratio, WARM temp -> SPOILED */
    act_spoil = max2(act_spoil, min2(u_r_med, u_t_warm));
    /* Rule 11: HIGH ratio, WARM temp -> ACCEPTABLE */
    act_acc   = max2(act_acc, min2(u_r_high, u_t_warm));

    /* 5. Defuzzification */
    float score = mbg_defuzzify_centroid(act_spoil, act_acc, act_fresh);

    /* 6. Special Diagnostics & Cross-Sensor Overrides */
    const char* diagnosis = "NORMAL TISSUE DEGRADATION";
    bool histamine_alert = false;
    bool is_freeze_thaw = false;

    /* Freeze-Thaw check for animal meats only (not tempe or tahu) */
    if (profile_id != MBG_PROFILE_TEMPE_KEDELAI && profile_id != MBG_PROFILE_TAHU_PUTIH) {
        if (impedance_ratio <= 1.25f && nh3_ppm <= 5.0f && temperature_c <= 8.0f) {
            is_freeze_thaw = true;
            diagnosis = "FREEZE-THAW DAMAGE (CHEMICALLY SAFE / NON-MICROBIAL)";
            score = 52.0f;
        }
    }

    /* Ammonia High check */
    bool nh3_high = (nh3_ppm >= nh3_thresh);

    /* Scombroid Histamine check */
    if (profile_id == MBG_PROFILE_IKAN_TONGKOL) {
        if (nh3_ppm >= 15.0f || (temperature_c >= 10.0f && impedance_ratio <= 1.80f)) {
            histamine_alert = true;
            diagnosis = "BAHAYA HISTAMIN (SCOMBROID POISONING RISK)";
        }
    }

    if (!is_freeze_thaw) {
        if (nh3_high || histamine_alert) {
            score = fminf(score, 20.0f);
            if (histamine_alert) {
                /* Handled */
            } else if (profile_id == MBG_PROFILE_TEMPE_KEDELAI) {
                diagnosis = "OVER-FERMENTATION / PROTEOLYTIC SPOILAGE (HIGH AMMONIA / TEMPE BUSUK)";
            } else if (profile_id == MBG_PROFILE_TAHU_PUTIH) {
                diagnosis = "BACTERIAL GEL LIQUEFACTION / FOUL SPOILAGE";
            } else {
                diagnosis = "BACTERIAL PROTEOLYSIS (HIGH VOLATILE AMINES)";
            }
        }
    }

    if (!isfinite(score)) {
        return indeterminate_result("NO MATCHING FUZZY RULES");
    }

    /* 7. Classify & SNI Status */
    MbgClassification classification;
    const char* sni_status;
    if (score >= 65.0f) {
        classification = MBG_CLASS_FRESH;
        sni_status = "MEMENUHI SYARAT";
    } else if (score <= 35.0f) {
        classification = MBG_CLASS_SPOILED;
        sni_status = "TIDAK MEMENUHI SYARAT";
    } else {
        classification = MBG_CLASS_ACCEPTABLE;
        sni_status = is_freeze_thaw ? "MEMENUHI SYARAT KELAYAKAN (SEGERA OLAH)" : "MEMENUHI SYARAT KELAYAKAN";
    }

    MbgInferenceResult result;
    result.freshness_score = roundf(score * 10.0f) / 10.0f;
    result.classification = classification;
    result.sni_status = sni_status;
    result.diagnosis = diagnosis;
    result.histamine_hazard_alert = histamine_alert;
    result.freeze_thaw_detected = is_freeze_thaw;

    return result;
}

MbgInferenceResult mbg_evaluate_liquid(
    float conductivity_mscm,
    float ph_value,
    float temperature_c
) {
    if (!isfinite(conductivity_mscm) || !isfinite(ph_value) ||
        !isfinite(temperature_c) || conductivity_mscm <= 0.0f ||
        ph_value < 0.0f || ph_value > 14.0f) {
        return indeterminate_result("MISSING OR INVALID SENSOR READINGS");
    }
    /* Liquid milk MFs (SNI 3141.1:2011) */
    const MbgTrapezoidalMF cond_norm = { 3.0f, 3.5f, 5.2f, 6.0f };
    const MbgTriangularMF  cond_elev = { 5.5f, 6.8f, 7.8f };
    const MbgTrapezoidalMF cond_high = { 7.2f, 8.2f, 15.0f, 15.0f };

    const MbgTrapezoidalMF ph_acid   = { 3.5f, 3.5f, 5.2f, 5.8f };
    const MbgTriangularMF  ph_norm   = { 5.7f, 6.55f, 7.1f };

    const MbgTrapezoidalMF temp_cold = { 0.0f, 0.0f, 4.0f, 7.0f };
    const MbgTriangularMF  temp_mild = { 5.0f, 10.0f, 16.0f };
    const MbgTrapezoidalMF temp_warm = { 13.0f, 18.0f, 25.0f, 25.0f };

    float u_c_norm = mbg_eval_trapezoidal(&cond_norm, conductivity_mscm);
    float u_c_elev = mbg_eval_triangular(&cond_elev, conductivity_mscm);
    float u_c_high = mbg_eval_trapezoidal(&cond_high, conductivity_mscm);

    float u_ph_acid = mbg_eval_trapezoidal(&ph_acid, ph_value);
    float u_ph_norm = mbg_eval_triangular(&ph_norm, ph_value);

    float u_t_cold = mbg_eval_trapezoidal(&temp_cold, temperature_c);
    float u_t_mild = mbg_eval_triangular(&temp_mild, temperature_c);
    float u_t_warm = mbg_eval_trapezoidal(&temp_warm, temperature_c);

    float act_fresh = 0.0f;
    float act_acc   = 0.0f;
    float act_spoil = 0.0f;

    /* Rule 1: NORMAL cond, NORMAL pH, COLD temp -> FRESH */
    act_fresh = max2(act_fresh, min3(u_c_norm, u_ph_norm, u_t_cold));
    /* Rule 2: NORMAL cond, NORMAL pH, MILD temp -> ACCEPTABLE */
    act_acc   = max2(act_acc, min3(u_c_norm, u_ph_norm, u_t_mild));
    /* Rule 3: NORMAL cond, NORMAL pH, WARM temp -> ACCEPTABLE */
    act_acc   = max2(act_acc, min3(u_c_norm, u_ph_norm, u_t_warm));
    /* Rule 4: ELEVATED cond, NORMAL pH, COLD temp -> ACCEPTABLE */
    act_acc   = max2(act_acc, min3(u_c_elev, u_ph_norm, u_t_cold));
    /* Rule 5: ACIDIC pH -> SPOILED */
    act_spoil = max2(act_spoil, u_ph_acid);
    /* Rule 6: HIGH cond -> SPOILED */
    act_spoil = max2(act_spoil, u_c_high);
    /* Rule 7: ELEVATED cond, WARM temp -> SPOILED */
    act_spoil = max2(act_spoil, min2(u_c_elev, u_t_warm));
    /* Rule 8: HIGH cond, ACIDIC pH -> SPOILED */
    act_spoil = max2(act_spoil, min2(u_c_high, u_ph_acid));

    float score = mbg_defuzzify_centroid(act_spoil, act_acc, act_fresh);

    const char* diagnosis = "NORMAL FRESH MILK";
    if (ph_value < 5.80f || conductivity_mscm > 7.2f) {
        score = fminf(score, 18.0f);
        diagnosis = "ACID SOURING (FERMENTASI ASAM LAKTAT / GUMPALAN)";
    }

    if (!isfinite(score)) {
        return indeterminate_result("NO MATCHING FUZZY RULES");
    }

    MbgClassification classification;
    const char* sni_status;
    if (score >= 65.0f) {
        classification = MBG_CLASS_FRESH;
        sni_status = "MEMENUHI SYARAT";
    } else if (score <= 35.0f) {
        classification = MBG_CLASS_SPOILED;
        sni_status = "TIDAK MEMENUHI SYARAT";
    } else {
        classification = MBG_CLASS_ACCEPTABLE;
        sni_status = "MEMENUHI SYARAT KELAYAKAN (SEGERA PASTEURISASI)";
    }

    MbgInferenceResult result;
    result.freshness_score = roundf(score * 10.0f) / 10.0f;
    result.classification = classification;
    result.sni_status = sni_status;
    result.diagnosis = diagnosis;
    result.histamine_hazard_alert = false;
    result.freeze_thaw_detected = false;

    return result;
}
