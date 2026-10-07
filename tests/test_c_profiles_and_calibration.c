/**
 * @file test_c_profiles_and_calibration.c
 * @brief Comprehensive verification of all 7 MBG profiles and calibration math in C
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "../firmware/src/mbg_fuzzy_core.h"
#include "../firmware/src/mbg_profiles.h"
#include "../firmware/src/mbg_calibration.h"

static void test_profiles_and_overrides(void) {
    /* 1. Ayam Broiler Fresh */
    MbgInferenceResult r_ayam_fresh = mbg_evaluate_solid(MBG_PROFILE_AYAM_BROILER, 2.45f, -15.5f, 4.0f, 2.0f);
    assert(r_ayam_fresh.freshness_score >= 80.0f);
    assert(r_ayam_fresh.classification == MBG_CLASS_FRESH);
    assert(strcmp(r_ayam_fresh.sni_status, "MEMENUHI SYARAT") == 0);
    printf("PASS: Ayam Broiler Fresh (Score: %.1f)\n", r_ayam_fresh.freshness_score);

    /* 2. Ayam Broiler Freeze-Thaw */
    MbgInferenceResult r_ayam_ft = mbg_evaluate_solid(MBG_PROFILE_AYAM_BROILER, 1.08f, -2.5f, 4.0f, 1.5f);
    assert(fabsf(r_ayam_ft.freshness_score - 52.0f) < 1.0f);
    assert(r_ayam_ft.freeze_thaw_detected == true);
    assert(strstr(r_ayam_ft.diagnosis, "FREEZE-THAW") != NULL);
    printf("PASS: Ayam Broiler Freeze-Thaw (Score: %.1f, Diag: %s)\n", r_ayam_ft.freshness_score, r_ayam_ft.diagnosis);

    /* 3. Ayam Broiler Spoiled */
    MbgInferenceResult r_ayam_spoil = mbg_evaluate_solid(MBG_PROFILE_AYAM_BROILER, 1.10f, -2.0f, 5.0f, 35.0f);
    assert(r_ayam_spoil.freshness_score <= 20.0f);
    assert(r_ayam_spoil.classification == MBG_CLASS_SPOILED);
    printf("PASS: Ayam Broiler Spoiled (Score: %.1f)\n", r_ayam_spoil.freshness_score);

    /* 4. Daging Sapi Fresh & Anisotropy */
    const MbgProfileConfig* sapi_cfg = mbg_get_profile_config(MBG_PROFILE_DAGING_SAPI);
    assert(sapi_cfg->anisotropy_safe == false);
    assert(strstr(sapi_cfg->orientation_notice, "Transverse") != NULL);
    MbgInferenceResult r_sapi_fresh = mbg_evaluate_solid(MBG_PROFILE_DAGING_SAPI, 2.10f, -14.0f, 4.0f, 3.0f);
    assert(r_sapi_fresh.freshness_score >= 80.0f);
    printf("PASS: Daging Sapi Fresh & Anisotropy Checked (Score: %.1f)\n", r_sapi_fresh.freshness_score);

    /* 5. Ikan Tongkol Histamine Hazard */
    MbgInferenceResult r_tongkol = mbg_evaluate_solid(MBG_PROFILE_IKAN_TONGKOL, 1.45f, -5.5f, 15.0f, 22.0f);
    assert(r_tongkol.histamine_hazard_alert == true);
    assert(r_tongkol.freshness_score <= 20.0f);
    assert(strstr(r_tongkol.diagnosis, "HISTAMIN") != NULL);
    printf("PASS: Ikan Tongkol Histamine Hazard (Score: %.1f, Alert: %d)\n", r_tongkol.freshness_score, r_tongkol.histamine_hazard_alert);

    /* 6. Tempe Kedelai Fresh & Over-fermentation */
    MbgInferenceResult r_tempe_fresh = mbg_evaluate_solid(MBG_PROFILE_TEMPE_KEDELAI, 2.10f, -15.0f, 4.0f, 2.5f);
    assert(r_tempe_fresh.freshness_score >= 80.0f);
    assert(r_tempe_fresh.classification == MBG_CLASS_FRESH);

    MbgInferenceResult r_tempe_spoil = mbg_evaluate_solid(MBG_PROFILE_TEMPE_KEDELAI, 1.12f, -2.0f, 4.0f, 32.0f);
    assert(r_tempe_spoil.freshness_score <= 20.0f);
    assert(r_tempe_spoil.classification == MBG_CLASS_SPOILED);
    assert(strstr(r_tempe_spoil.diagnosis, "TEMPE BUSUK") != NULL);
    printf("PASS: Tempe Kedelai Fresh & Busuk Checked (Fresh: %.1f, Busuk: %.1f)\n",
           r_tempe_fresh.freshness_score, r_tempe_spoil.freshness_score);

    /* 7. Tahu Putih Fresh & Liquefaction */
    MbgInferenceResult r_tahu_fresh = mbg_evaluate_solid(MBG_PROFILE_TAHU_PUTIH, 1.85f, -13.0f, 4.0f, 1.8f);
    assert(r_tahu_fresh.freshness_score >= 80.0f);

    MbgInferenceResult r_tahu_spoil = mbg_evaluate_solid(MBG_PROFILE_TAHU_PUTIH, 1.08f, -1.5f, 4.0f, 24.0f);
    assert(r_tahu_spoil.freshness_score <= 20.0f);
    assert(strstr(r_tahu_spoil.diagnosis, "LIQUEFACTION") != NULL);
    printf("PASS: Tahu Putih Fresh & Liquefaction Checked (Fresh: %.1f, Spoil: %.1f)\n",
           r_tahu_fresh.freshness_score, r_tahu_spoil.freshness_score);

    /* 8. Susu Sapi Segar */
    MbgInferenceResult r_susu_fresh = mbg_evaluate_liquid(4.8f, 6.60f, 4.0f);
    assert(r_susu_fresh.freshness_score >= 80.0f);
    assert(r_susu_fresh.classification == MBG_CLASS_FRESH);

    MbgInferenceResult r_susu_sour = mbg_evaluate_liquid(8.5f, 4.75f, 4.0f);
    assert(r_susu_sour.freshness_score <= 20.0f);
    assert(strstr(r_susu_sour.diagnosis, "ACID SOURING") != NULL);
    printf("PASS: Susu Sapi Segar & Acid Souring (Fresh: %.1f, Sour: %.1f)\n",
           r_susu_fresh.freshness_score, r_susu_sour.freshness_score);
}

static void test_calibration_math(void) {
    /* Complex division test */
    MbgComplex a = { 4.0f, 2.0f };
    MbgComplex b = { 2.0f, 0.0f };
    MbgComplex div = mbg_complex_div(a, b);
    assert(fabsf(div.real - 2.0f) < 1e-5f);
    assert(fabsf(div.imag - 1.0f) < 1e-5f);

    /* Modulus test */
    MbgComplex z = { 3.0f, 4.0f };
    assert(fabsf(mbg_complex_modulus(z) - 5.0f) < 1e-5f);

    /* Phase test */
    MbgComplex z_45 = { 1.0f, 1.0f };
    assert(fabsf(mbg_complex_phase_deg(z_45) - 45.0f) < 1e-3f);

    /* AD5933 Margo Eq. 4 extractor: Zx = (Xv / Xi) * K */
    MbgComplex xv = { 100.0f, 0.0f };
    MbgComplex xi = { 50.0f, 0.0f };
    MbgComplex k  = { 500.0f, 0.0f };
    MbgComplex zx = mbg_extract_ad5933_impedance(xv, xi, k);
    assert(fabsf(zx.real - 1000.0f) < 1e-3f);
    assert(fabsf(zx.imag - 0.0f) < 1e-3f);

    /* Liquid conductivity test */
    /* sigma = (130 / 260) * 10 = 5.0 mS/cm */
    float sigma = mbg_calculate_liquid_conductivity(260.0f, 130.0f);
    assert(fabsf(sigma - 5.0f) < 1e-3f);

    /* Temperature compensation */
    float z_comp = mbg_compensate_tissue_temp(1000.0f, 14.0f, 4.0f, 0.0156f);
    assert(z_comp > 1000.0f);

    printf("PASS: test_calibration_math\n");
}

int main(void) {
    printf("Running C Profiles and Calibration Tests...\n");
    test_profiles_and_overrides();
    test_calibration_math();
    MbgInferenceResult invalid = mbg_evaluate_solid(MBG_PROFILE_AYAM_BROILER, NAN, -15.0f, 4.0f, 2.0f);
    assert(invalid.classification == MBG_CLASS_INDETERMINATE);
    assert(isnan(invalid.freshness_score));
    assert(!invalid.freeze_thaw_detected);
    MbgInferenceResult missing = mbg_evaluate_solid(MBG_PROFILE_AYAM_BROILER, 1.1f, -2.0f, 4.0f, NAN);
    assert(missing.classification == MBG_CLASS_INDETERMINATE);
    MbgInferenceResult uncovered = mbg_evaluate_liquid(4.8f, 7.8f, 4.0f);
    assert(uncovered.classification == MBG_CLASS_INDETERMINATE);
    assert(isnan(uncovered.freshness_score));
    printf("ALL C PROFILES AND CALIBRATION TESTS PASSED!\n");
    return 0;
}
