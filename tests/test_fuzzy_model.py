import pytest
import numpy as np
from fuzzy_model import FreshnessFuzzyModel

def test_model_initialization():
    model = FreshnessFuzzyModel()
    assert model is not None
    assert "impedance_ratio" in model.inputs
    assert "phase_angle" in model.inputs
    assert "temperature" in model.inputs
    assert model.output_name == "freshness"

def test_fresh_fish_inference():
    model = FreshnessFuzzyModel()
    # High impedance ratio (intact cell membranes), prominent phase angle, cold storage (4°C)
    result = model.evaluate(impedance_ratio=2.4, phase_angle=-14.0, temperature=4.0, nh3_ppm=2.0)
    assert 70.0 <= result["freshness_score"] <= 100.0
    assert result["classification"] == "FRESH"

def test_spoiled_fish_inference():
    model = FreshnessFuzzyModel()
    # Low impedance ratio (~1.0, ruptured cell membranes), collapsed phase angle (-2°), elevated temperature (12°C)
    result = model.evaluate(impedance_ratio=1.05, phase_angle=-2.0, temperature=12.0, nh3_ppm=6.0)
    assert 0.0 <= result["freshness_score"] <= 35.0
    assert result["classification"] == "SPOILED"

def test_moderate_fish_inference():
    model = FreshnessFuzzyModel()
    # Intermediate values
    result = model.evaluate(impedance_ratio=1.5, phase_angle=-8.0, temperature=4.0, nh3_ppm=2.0)
    assert 35.0 <= result["freshness_score"] <= 75.0

def test_temperature_impact():
    model = FreshnessFuzzyModel()
    # At identical EIS values, warmer temperature accelerates spoilage risk and decreases freshness score
    res_cold = model.evaluate(impedance_ratio=1.6, phase_angle=-9.0, temperature=2.0, nh3_ppm=2.0)
    res_warm = model.evaluate(impedance_ratio=1.6, phase_angle=-9.0, temperature=14.0, nh3_ppm=2.0)
    assert res_cold["freshness_score"] > res_warm["freshness_score"]

def test_model_requires_calibration_flag():
    model = FreshnessFuzzyModel()
    # Without in-situ hardware calibration, the model explicitly flags that numerical boundaries require experimental calibration
    metadata = model.get_metadata()
    assert metadata["requires_experimental_calibration"] is True
