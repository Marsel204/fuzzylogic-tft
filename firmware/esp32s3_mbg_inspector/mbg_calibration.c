/**
 * @file mbg_calibration.c
 * @brief Complex Arithmetic and Calibration Equations Implementation
 */

#include "mbg_calibration.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

MbgComplex mbg_complex_mult(MbgComplex a, MbgComplex b) {
    MbgComplex res;
    res.real = a.real * b.real - a.imag * b.imag;
    res.imag = a.real * b.imag + a.imag * b.real;
    return res;
}

MbgComplex mbg_complex_div(MbgComplex a, MbgComplex b) {
    MbgComplex res = { 0.0f, 0.0f };
    float denom = b.real * b.real + b.imag * b.imag;
    if (denom < 1e-12f) {
        return res;
    }
    res.real = (a.real * b.real + a.imag * b.imag) / denom;
    res.imag = (a.imag * b.real - a.real * b.imag) / denom;
    return res;
}

float mbg_complex_modulus(MbgComplex z) {
    return sqrtf(z.real * z.real + z.imag * z.imag);
}

float mbg_complex_phase_deg(MbgComplex z) {
    return atan2f(z.imag, z.real) * (180.0f / (float)M_PI);
}

MbgComplex mbg_extract_ad5933_impedance(MbgComplex xv, MbgComplex xi, MbgComplex k_cal) {
    MbgComplex ratio = mbg_complex_div(xv, xi);
    return mbg_complex_mult(ratio, k_cal);
}

float mbg_calculate_impedance_ratio(float z_lf, float z_hf) {
    if (z_hf <= 1e-7f) return 1.0f;
    return z_lf / z_hf;
}

float mbg_calculate_py_parameter(float z_lf, float z_hf) {
    if (z_hf <= 1e-7f) return 0.0f;
    return (z_lf - z_hf) / z_hf;
}

float mbg_calculate_liquid_conductivity(float z_tetra_modulus, float k_cell) {
    if (z_tetra_modulus <= 1e-7f) return 0.0f;
    float sigma_sm = k_cell / z_tetra_modulus;
    return sigma_sm * 10.0f; /* Convert S/m to mS/cm */
}

float mbg_compensate_tissue_temp(float z_meas, float temp_c, float temp_ref, float alpha) {
    float delta_t = temp_c - temp_ref;
    float factor = 1.0f + alpha * delta_t;
    return z_meas * factor;
}

float mbg_compensate_milk_temp(float sigma_meas, float temp_c, float temp_ref, float beta) {
    float factor = 1.0f + beta * (temp_c - temp_ref);
    if (factor <= 1e-7f) return sigma_meas;
    return sigma_meas / factor;
}
