/**
 * @file test_c_fuzzy_core.c
 * @brief Unit tests for C99 fuzzy math core
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "../firmware/src/mbg_fuzzy_core.h"

static void test_triangular_mf(void) {
    MbgTriangularMF tri = { 10.0f, 20.0f, 30.0f };
    assert(fabsf(mbg_eval_triangular(&tri, 5.0f) - 0.0f) < 1e-5f);
    assert(fabsf(mbg_eval_triangular(&tri, 10.0f) - 0.0f) < 1e-5f);
    assert(fabsf(mbg_eval_triangular(&tri, 15.0f) - 0.5f) < 1e-5f);
    assert(fabsf(mbg_eval_triangular(&tri, 20.0f) - 1.0f) < 1e-5f);
    assert(fabsf(mbg_eval_triangular(&tri, 25.0f) - 0.5f) < 1e-5f);
    assert(fabsf(mbg_eval_triangular(&tri, 30.0f) - 0.0f) < 1e-5f);
    assert(fabsf(mbg_eval_triangular(&tri, 35.0f) - 0.0f) < 1e-5f);
    printf("PASS: test_triangular_mf\n");
}

static void test_trapezoidal_mf(void) {
    MbgTrapezoidalMF trap = { 10.0f, 20.0f, 30.0f, 40.0f };
    assert(fabsf(mbg_eval_trapezoidal(&trap, 5.0f) - 0.0f) < 1e-5f);
    assert(fabsf(mbg_eval_trapezoidal(&trap, 15.0f) - 0.5f) < 1e-5f);
    assert(fabsf(mbg_eval_trapezoidal(&trap, 25.0f) - 1.0f) < 1e-5f);
    assert(fabsf(mbg_eval_trapezoidal(&trap, 35.0f) - 0.5f) < 1e-5f);
    assert(fabsf(mbg_eval_trapezoidal(&trap, 45.0f) - 0.0f) < 1e-5f);

    /* Left shoulder test */
    MbgTrapezoidalMF left = { 0.0f, 0.0f, 15.0f, 35.0f };
    assert(fabsf(mbg_eval_trapezoidal(&left, 0.0f) - 1.0f) < 1e-5f);
    assert(fabsf(mbg_eval_trapezoidal(&left, 10.0f) - 1.0f) < 1e-5f);
    assert(fabsf(mbg_eval_trapezoidal(&left, 25.0f) - 0.5f) < 1e-5f);

    /* Right shoulder test */
    MbgTrapezoidalMF right = { 65.0f, 85.0f, 100.0f, 100.0f };
    assert(fabsf(mbg_eval_trapezoidal(&right, 100.0f) - 1.0f) < 1e-5f);
    assert(fabsf(mbg_eval_trapezoidal(&right, 90.0f) - 1.0f) < 1e-5f);
    assert(fabsf(mbg_eval_trapezoidal(&right, 75.0f) - 0.5f) < 1e-5f);

    printf("PASS: test_trapezoidal_mf\n");
}

static void test_defuzzification(void) {
    /* Pure fresh activation */
    float score_fresh = mbg_defuzzify_centroid(0.0f, 0.0f, 1.0f);
    assert(score_fresh >= 85.0f && score_fresh <= 95.0f);

    /* Pure spoiled activation */
    float score_spoiled = mbg_defuzzify_centroid(1.0f, 0.0f, 0.0f);
    assert(score_spoiled >= 10.0f && score_spoiled <= 16.0f);
    assert(isnan(mbg_defuzzify_centroid(0.0f, 0.0f, 0.0f)));
    assert(isnan(mbg_defuzzify_centroid(NAN, 0.0f, 0.0f)));

    /* Pure acceptable activation */
    float score_acc = mbg_defuzzify_centroid(0.0f, 1.0f, 0.0f);
    assert(fabsf(score_acc - 50.0f) < 1.0f);

    printf("PASS: test_defuzzification (Fresh: %.2f, Spoiled: %.2f, Acc: %.2f)\n",
           score_fresh, score_spoiled, score_acc);
}

int main(void) {
    printf("Running C Fuzzy Core Unit Tests...\n");
    test_triangular_mf();
    test_trapezoidal_mf();
    test_defuzzification();
    printf("ALL C FUZZY CORE TESTS PASSED!\n");
    return 0;
}
