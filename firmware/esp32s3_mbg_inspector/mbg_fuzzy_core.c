/**
 * @file mbg_fuzzy_core.c
 * @brief Zero-Dynamic-Allocation Fuzzy Logic Math Engine
 */

#include "mbg_fuzzy_core.h"
#include <math.h>

/* Static Consequent Membership Function Definitions */
static const MbgTrapezoidalMF MF_OUT_SPOILED    = { 0.0f,  0.0f, 15.0f,  35.0f };
static const MbgTriangularMF  MF_OUT_ACCEPTABLE = { 25.0f, 50.0f, 75.0f };
static const MbgTrapezoidalMF MF_OUT_FRESH      = { 65.0f, 85.0f, 100.0f, 100.0f };

float mbg_eval_triangular(const MbgTriangularMF* mf, float x) {
    if (!isfinite(x)) return 0.0f;
    if (x <= mf->a || x >= mf->c) {
        return 0.0f;
    }
    if (x == mf->b) {
        return 1.0f;
    }
    if (x < mf->b) {
        float denom = mf->b - mf->a;
        return (denom > 1e-7f) ? (x - mf->a) / denom : 0.0f;
    } else {
        float denom = mf->c - mf->b;
        return (denom > 1e-7f) ? (mf->c - x) / denom : 0.0f;
    }
}

float mbg_eval_trapezoidal(const MbgTrapezoidalMF* mf, float x) {
    if (!isfinite(x)) return 0.0f;
    if (mf->a == mf->b && x <= mf->b) {
        return 1.0f;
    }
    if (mf->c == mf->d && x >= mf->c) {
        return 1.0f;
    }
    if (x <= mf->a || x >= mf->d) {
        return 0.0f;
    }
    if (x >= mf->b && x <= mf->c) {
        return 1.0f;
    }
    if (x < mf->b) {
        float denom = mf->b - mf->a;
        return (denom > 1e-7f) ? (x - mf->a) / denom : 1.0f;
    } else {
        float denom = mf->d - mf->c;
        return (denom > 1e-7f) ? (mf->d - x) / denom : 1.0f;
    }
}

float mbg_defuzzify_centroid(float spoiled_act, float acceptable_act, float fresh_act) {
    if (!isfinite(spoiled_act) || !isfinite(acceptable_act) || !isfinite(fresh_act)) {
        return NAN;
    }
    float numerator = 0.0f;
    float denominator = 0.0f;

    /* Step size for 100 grid points over [0.0, 100.0] */
    const float step = 100.0f / (float)(MBG_DEFUZZ_POINTS - 1);

    for (int i = 0; i < MBG_DEFUZZ_POINTS; ++i) {
        float y = (float)i * step;

        /* Evaluate each consequent membership */
        float u_spoiled = mbg_eval_trapezoidal(&MF_OUT_SPOILED, y);
        float u_acc     = mbg_eval_triangular(&MF_OUT_ACCEPTABLE, y);
        float u_fresh   = mbg_eval_trapezoidal(&MF_OUT_FRESH, y);

        /* Apply Mamdani Min-clipping */
        float clip_spoiled = (spoiled_act < u_spoiled) ? spoiled_act : u_spoiled;
        float clip_acc     = (acceptable_act < u_acc) ? acceptable_act : u_acc;
        float clip_fresh   = (fresh_act < u_fresh) ? fresh_act : u_fresh;

        /* Max union aggregation */
        float agg = clip_spoiled;
        if (clip_acc > agg) agg = clip_acc;
        if (clip_fresh > agg) agg = clip_fresh;

        numerator += y * agg;
        denominator += agg;
    }

    if (denominator > 1e-7f) {
        return numerator / denominator;
    }
    return NAN; /* No inference is available when no rule triggered. */
}
