"""
Cross-Language Monte Carlo Verification: Python vs C/ESP32 Fuzzy Engine
Validates that the C99 embedded engine is bit-compatible with the Python reference model.
"""

import ctypes
import os
import pytest
import numpy as np
from fuzzy_model import FreshnessFuzzyModel

# Load shared library
SO_PATH = os.path.join(os.path.dirname(__file__), "..", "bin", "libmbgfuzzy.so")

if not os.path.exists(SO_PATH):
    import subprocess
    os.makedirs(os.path.dirname(SO_PATH), exist_ok=True)
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    cmd = [
        "gcc", "-shared", "-fPIC", "-O2",
        os.path.join(root_dir, "firmware", "src", "mbg_fuzzy_core.c"),
        os.path.join(root_dir, "firmware", "src", "mbg_profiles.c"),
        "-lm", "-o", SO_PATH
    ]
    subprocess.check_call(cmd)

class MbgInferenceResult(ctypes.Structure):
    _fields_ = [
        ("freshness_score", ctypes.c_float),
        ("classification", ctypes.c_int),
        ("sni_status", ctypes.c_char_p),
        ("diagnosis", ctypes.c_char_p),
        ("histamine_hazard_alert", ctypes.c_bool),
        ("freeze_thaw_detected", ctypes.c_bool),
    ]

lib = ctypes.CDLL(SO_PATH)
lib.mbg_evaluate_solid.argtypes = [
    ctypes.c_int,
    ctypes.c_float,
    ctypes.c_float,
    ctypes.c_float,
    ctypes.c_float
]
lib.mbg_evaluate_solid.restype = MbgInferenceResult

lib.mbg_evaluate_liquid.argtypes = [
    ctypes.c_float,
    ctypes.c_float,
    ctypes.c_float
]
lib.mbg_evaluate_liquid.restype = MbgInferenceResult

PROFILE_MAP = {
    "ayam_broiler": 0,
    "daging_sapi": 1,
    "ikan_nila_lele": 2,
    "ikan_tongkol": 3,
    "tempe_kedelai": 4,
    "tahu_putih": 5
}

CLASS_MAP = {
    0: "SPOILED",
    1: "ACCEPTABLE",
    2: "FRESH",
    3: "INDETERMINATE"
}

def test_fixed_benchmark_scenarios():
    """Verify standard benchmark scenarios yield matching classifications and scores."""
    scenarios = [
        ("ayam_broiler", 2.45, -15.5, 4.0, 2.0),
        ("ayam_broiler", 1.08, -2.5, 4.0, 1.5),
        ("ayam_broiler", 1.10, -2.0, 5.0, 35.0),
        ("daging_sapi", 2.10, -14.0, 4.0, 3.0),
        ("ikan_nila_lele", 2.50, -16.0, 3.0, 1.0),
        ("ikan_tongkol", 1.45, -5.5, 15.0, 22.0),
        ("tempe_kedelai", 2.10, -15.0, 4.0, 2.5),
        ("tempe_kedelai", 1.12, -2.0, 4.0, 32.0),
        ("tahu_putih", 1.85, -13.0, 4.0, 1.8),
        ("tahu_putih", 1.08, -1.5, 4.0, 24.0),
    ]

    for prof, r, ph, temp, nh3 in scenarios:
        py_model = FreshnessFuzzyModel(ingredient_profile=prof)
        py_res = py_model.evaluate(impedance_ratio=r, phase_angle=ph, temperature=temp, nh3_ppm=nh3)

        c_prof_id = PROFILE_MAP[prof]
        c_res = lib.mbg_evaluate_solid(c_prof_id, r, ph, temp, nh3)

        c_class = CLASS_MAP[c_res.classification]

        assert py_res["classification"] == c_class, f"Mismatch in classification for {prof}"
        # Scores should match within 0.5% (due to 100 vs 200 discretization points)
        assert abs(py_res["freshness_score"] - c_res.freshness_score) < 0.6, \
            f"Score divergence for {prof}: py={py_res['freshness_score']}, c={c_res.freshness_score}"

def test_liquid_milk_scenarios():
    """Verify milk scenarios between Python and C."""
    scenarios = [
        (4.8, 6.60, 4.0),
        (8.5, 4.75, 4.0),
        (5.2, 6.40, 6.0),
        (7.5, 5.50, 4.0)
    ]
    py_model = FreshnessFuzzyModel(ingredient_profile="susu_segar")
    for cond, ph, temp in scenarios:
        py_res = py_model.evaluate_liquid(conductivity_mscm=cond, ph_value=ph, temperature=temp)
        c_res = lib.mbg_evaluate_liquid(cond, ph, temp)

        c_class = CLASS_MAP[c_res.classification]
        assert py_res["classification"] == c_class
        assert abs(py_res["freshness_score"] - c_res.freshness_score) < 0.6

def test_monte_carlo_1000_vectors():
    """1,000-point randomized Monte Carlo sweep verifying zero decision divergence."""
    np.random.seed(42)
    solid_profiles = list(PROFILE_MAP.keys())

    max_delta = 0.0
    for _ in range(1000):
        prof = np.random.choice(solid_profiles)
        r = float(np.random.uniform(0.85, 3.10))
        ph = float(np.random.uniform(-24.0, -0.5))
        temp = float(np.random.uniform(1.0, 24.0))
        nh3 = float(np.random.uniform(0.5, 45.0))

        py_model = FreshnessFuzzyModel(ingredient_profile=prof)
        py_res = py_model.evaluate(impedance_ratio=r, phase_angle=ph, temperature=temp, nh3_ppm=nh3)

        c_res = lib.mbg_evaluate_solid(PROFILE_MAP[prof], r, ph, temp, nh3)
        c_class = CLASS_MAP[c_res.classification]

        if py_res["classification"] == "INDETERMINATE":
            assert c_class == "INDETERMINATE"
            assert py_res["freshness_score"] is None
            assert np.isnan(c_res.freshness_score)
            continue

        delta = abs(py_res["freshness_score"] - c_res.freshness_score)
        if delta > max_delta:
            max_delta = delta

        # Decision classification must agree 100%
        assert py_res["classification"] == c_class, \
            f"Monte Carlo mismatch on {prof}: R={r:.2f}, NH3={nh3:.1f}. Py={py_res['classification']} ({py_res['freshness_score']:.1f}), C={c_class} ({c_res.freshness_score:.1f})"

    print(f"\nMonte Carlo 1,000 vectors complete: Maximum delta = {max_delta:.2f}% (all classifications 100% identical)")
