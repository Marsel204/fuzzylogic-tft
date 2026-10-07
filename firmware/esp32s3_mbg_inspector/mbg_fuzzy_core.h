/**
 * @file mbg_fuzzy_core.h
 * @brief Core Embedded Fuzzy Logic Engine for Indonesian MBG Food Inspection
 * Zero-Dynamic-Allocation, MISRA-C Compliant, ESP32-S3 Hardware FPU Accelerated
 */

#ifndef MBG_FUZZY_CORE_H
#define MBG_FUZZY_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define MBG_DEFUZZ_POINTS 200
#define MBG_OUTPUT_MIN    0.0f
#define MBG_OUTPUT_MAX    100.0f

typedef enum {
    MBG_CLASS_SPOILED = 0,
    MBG_CLASS_ACCEPTABLE = 1,
    MBG_CLASS_FRESH = 2,
    MBG_CLASS_INDETERMINATE = 3
} MbgClassification;

typedef struct {
    float a;
    float b;
    float c;
} MbgTriangularMF;

typedef struct {
    float a;
    float b;
    float c;
    float d;
} MbgTrapezoidalMF;

typedef struct {
    float freshness_score;              /* 0.0 to 100.0 %, NAN if indeterminate */
    MbgClassification classification;   /* FRESH, ACCEPTABLE, SPOILED */
    const char* sni_status;             /* Indonesian statutory text */
    const char* diagnosis;              /* Diagnostic reason */
    bool histamine_hazard_alert;        /* Scombroid warning flag */
    bool freeze_thaw_detected;          /* Freeze-thaw cell rupture flag */
} MbgInferenceResult;

/**
 * @brief Fast clamp helper for [0.0, 1.0] membership
 */
static inline float mbg_clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

/**
 * @brief Evaluate triangular membership function at point x
 */
float mbg_eval_triangular(const MbgTriangularMF* mf, float x);

/**
 * @brief Evaluate trapezoidal membership function at point x
 */
float mbg_eval_trapezoidal(const MbgTrapezoidalMF* mf, float x);

/**
 * @brief Centroid defuzzification over 100-point discrete universe [0, 100]
 * @param spoiled_act Activation weight for SPOILED consequent [0, 1]
 * @param acceptable_act Activation weight for ACCEPTABLE consequent [0, 1]
 * @param fresh_act Activation weight for FRESH consequent [0, 1]
 * @return Defuzzified crisp centroid (0.0 - 100.0), or NAN without valid activation
 */
float mbg_defuzzify_centroid(float spoiled_act, float acceptable_act, float fresh_act);

#ifdef __cplusplus
}
#endif

#endif /* MBG_FUZZY_CORE_H */
