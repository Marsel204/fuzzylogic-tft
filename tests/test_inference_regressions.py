"""Regressions for unavailable sensors, failed inference, and CLI verdicts."""
import csv
import math
from pathlib import Path
import subprocess

import numpy as np
import pytest

from feature_processing import AD5933SpectralFeatureExtractor
from fuzzy_model import FreshnessFuzzyModel
from test_cross_language_verification import lib, CLASS_MAP

ROOT = Path(__file__).resolve().parents[1]


def assert_indeterminate(py_result, c_result):
    assert py_result["classification"] == CLASS_MAP[c_result.classification] == "INDETERMINATE"
    assert py_result["freshness_score"] is None
    assert math.isnan(c_result.freshness_score)
    assert "MEMENUHI SYARAT" not in py_result["sni_status"]
    assert "MEMENUHI SYARAT" not in c_result.sni_status.decode()
    assert not c_result.freeze_thaw_detected


def test_missing_nh3_does_not_imply_freeze_thaw():
    model = FreshnessFuzzyModel("ayam_broiler")
    assert_indeterminate(model.evaluate(1.1, -2, 4), lib.mbg_evaluate_solid(0, 1.1, -2, 4, float("nan")))
    assert model.evaluate(1.1, -2, 4, 6)["classification"] == "SPOILED"
    assert model.evaluate(1.1, -2, 4, 0)["freshness_score"] == 52


@pytest.mark.parametrize("index", range(4))
@pytest.mark.parametrize("invalid", [float("nan"), float("inf"), -float("inf")])
def test_nonfinite_solid_readings_never_pass(index, invalid):
    values = [2.45, -15, 4, 2]
    values[index] = invalid
    assert_indeterminate(FreshnessFuzzyModel("ayam_broiler").evaluate(*values), lib.mbg_evaluate_solid(0, *values))


@pytest.mark.parametrize("index", range(3))
@pytest.mark.parametrize("invalid", [float("nan"), float("inf"), -float("inf")])
def test_nonfinite_liquid_readings_never_pass(index, invalid):
    values = [4.8, 6.6, 4]
    values[index] = invalid
    assert_indeterminate(FreshnessFuzzyModel("susu_segar").evaluate_liquid(*values), lib.mbg_evaluate_liquid(*values))


@pytest.mark.parametrize("mode", ["solid", "liquid"])
def test_uncovered_inputs_have_no_compliance_verdict(mode):
    if mode == "solid":
        model = FreshnessFuzzyModel("ayam_broiler")
        py_result = model.evaluate(2.45, -2, 4, 6)
        c_result = lib.mbg_evaluate_solid(0, 2.45, -2, 4, 6)
    else:
        model = FreshnessFuzzyModel("susu_segar")
        py_result = model.evaluate_liquid(4.8, 7.8, 4)
        c_result = lib.mbg_evaluate_liquid(4.8, 7.8, 4)
    assert_indeterminate(py_result, c_result)
    assert py_result["diagnosis"] == "NO MATCHING FUZZY RULES"


def test_feature_extractor_preserves_missing_cross_sensors():
    spectrum = np.array([1100 + 0j, 800 * np.exp(-2j * np.pi / 180), 1000 + 0j])
    features = AD5933SpectralFeatureExtractor("ayam_broiler").extract_features_from_spectrum(
        np.array([2000., 10000., 100000.]), spectrum, 4)
    assert features["nh3_ppm"] is None
    result = FreshnessFuzzyModel("ayam_broiler").evaluate(
        features["impedance_ratio"], features["phase_angle"], features["temperature"], features["nh3_ppm"])
    assert result["classification"] == "INDETERMINATE"
    milk = AD5933SpectralFeatureExtractor("susu_segar").extract_features_from_spectrum(
        np.array([10000.]), np.array([1300 / 4.8 + 0j]), 20)
    assert milk["ph_value"] is None
    result = FreshnessFuzzyModel("susu_segar").evaluate_liquid(
        milk["conductivity_mscm"], milk["ph_value"], milk["temperature"])
    assert result["classification"] == "INDETERMINATE"


def test_high_ammonia_override_still_rejects_an_uncovered_eis_combination():
    py_result = FreshnessFuzzyModel("ayam_broiler").evaluate(2.45, -2, 4, 35)
    c_result = lib.mbg_evaluate_solid(0, 2.45, -2, 4, 35)
    assert py_result["classification"] == CLASS_MAP[c_result.classification] == "SPOILED"
    assert py_result["freshness_score"] == c_result.freshness_score == 20


@pytest.fixture(scope="module")
def cli():
    subprocess.run(["make", "all"], cwd=ROOT, check=True, capture_output=True)
    return ROOT / "bin" / "mbg_inspect"


@pytest.mark.parametrize("args", [
    ["--profile", "ayam_broiler", "--ratio", "nan", "--nh3", "2"],
    ["--profile", "ayam_broiler", "--ratio", "1.1", "--phase", "-2"],
    ["--profile", "susu_segar", "--cond", "4.8", "--ph", "7.8"],
    ["--profile", "susu_segar", "--cond", "4.8"],
])
def test_cli_does_not_record_a_passing_score_for_invalid_inspection(cli, tmp_path, args):
    output = subprocess.check_output([str(cli), *args], cwd=tmp_path, text=True)
    assert "BELUM DAPAT DITENTUKAN" in output
    assert "MEMENUHI SYARAT" not in output
    with (tmp_path / "audit_logs" / "receiving_inspection.csv").open() as file:
        row = list(csv.DictReader(file))[0]
    assert row["Freshness_Score"] == ""
