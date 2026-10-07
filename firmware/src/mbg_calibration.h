/**
 * @file mbg_calibration.h
 * @brief Complex Vector Arithmetic and Calibration for AD5933 Front-End
 * Implements Margo et al. (2013) Equations 4 & 5
 */

#ifndef MBG_CALIBRATION_H
#define MBG_CALIBRATION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float real;
    float imag;
} MbgComplex;

/* Complex number operations */
static inline MbgComplex mbg_complex_make(float r, float i) {
    MbgComplex c = { r, i };
    return c;
}

MbgComplex mbg_complex_mult(MbgComplex a, MbgComplex b);
MbgComplex mbg_complex_div(MbgComplex a, MbgComplex b);
float mbg_complex_modulus(MbgComplex z);
float mbg_complex_phase_deg(MbgComplex z);

/**
 * @brief Extract unknown complex impedance from AD5933 two-step tetrapolar acquisition
 * [MARGO: Eq. 4] Zx(f) = (Xv(f) / Xi(f)) * K(f)
 */
MbgComplex mbg_extract_ad5933_impedance(MbgComplex xv, MbgComplex xi, MbgComplex k_cal);

/**
 * @brief Compute dual-frequency ratio R_eis = |Z_lf| / |Z_hf|
 */
float mbg_calculate_impedance_ratio(float z_lf, float z_hf);

/**
 * @brief Compute Pliquett Py parameter: Py = (|Z_lf| - |Z_hf|) / |Z_hf|
 */
float mbg_calculate_py_parameter(float z_lf, float z_hf);

/**
 * @brief Compute bulk liquid conductivity in mS/cm from tetrapolar modulus
 * [MARGO: Section 5] sigma (mS/cm) = (K_cell / |Z|) * 10.0
 */
float mbg_calculate_liquid_conductivity(float z_tetra_modulus, float k_cell);

/**
 * @brief Temperature compensation for animal tissue impedance (-1.56%/C)
 */
float mbg_compensate_tissue_temp(float z_meas, float temp_c, float temp_ref, float alpha);

/**
 * @brief Temperature compensation for bovine milk electrolytic conductivity (+2.0%/C)
 */
float mbg_compensate_milk_temp(float sigma_meas, float temp_c, float temp_ref, float beta);

#ifdef __cplusplus
}
#endif

#endif /* MBG_CALIBRATION_H */
