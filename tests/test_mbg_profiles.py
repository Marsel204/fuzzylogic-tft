import pytest
import numpy as np
from fuzzy_model import FreshnessFuzzyModel

def test_mbg_profile_initialization():
    profiles = [
        "ayam_broiler", "daging_sapi", "ikan_nila_lele", "ikan_tongkol", "susu_segar",
        "tempe_kedelai", "tahu_putih"
    ]
    for p in profiles:
        model = FreshnessFuzzyModel(ingredient_profile=p)
        assert model.ingredient_profile == p
        assert "sni_standard" in model.profile_config

    with pytest.raises(ValueError):
        FreshnessFuzzyModel(ingredient_profile="makanan_tidak_dikenal")

def test_chicken_freshness_inference():
    model = FreshnessFuzzyModel(ingredient_profile="ayam_broiler")
    # Fresh chicken breast at 4°C with low ammonia
    res_fresh = model.evaluate(impedance_ratio=2.40, phase_angle=-15.0, temperature=4.0, nh3_ppm=2.0)
    assert res_fresh["freshness_score"] >= 70.0
    assert res_fresh["classification"] == "FRESH"
    assert "SNI 3924:2009" in res_fresh["sni_standard"]
    assert res_fresh["sni_status"] == "MEMENUHI SYARAT"

    # Spoiled chicken with high ammonia
    res_spoiled = model.evaluate(impedance_ratio=1.10, phase_angle=-2.0, temperature=4.0, nh3_ppm=35.0)
    assert res_spoiled["freshness_score"] <= 30.0
    assert res_spoiled["classification"] == "SPOILED"
    assert res_spoiled["sni_status"] == "TIDAK MEMENUHI SYARAT"

def test_beef_anisotropy_and_freshness():
    model = FreshnessFuzzyModel(ingredient_profile="daging_sapi")
    meta = model.get_metadata()
    assert "anisotropy" in meta["probe_orientation_requirement"].lower()

    # Fresh beef at 4°C
    res_fresh = model.evaluate(impedance_ratio=2.10, phase_angle=-14.0, temperature=4.0, nh3_ppm=3.0)
    assert res_fresh["freshness_score"] >= 65.0
    assert res_fresh["classification"] == "FRESH"
    assert "SNI 3932:2008" in res_fresh["sni_standard"]

    # Spoiled beef
    res_spoiled = model.evaluate(impedance_ratio=1.12, phase_angle=-1.5, temperature=4.0, nh3_ppm=28.0)
    assert res_spoiled["freshness_score"] <= 30.0
    assert res_spoiled["classification"] == "SPOILED"

def test_tongkol_histamine_hazard_trigger():
    model = FreshnessFuzzyModel(ingredient_profile="ikan_tongkol")
    # Temperature abuse with elevated ammonia on marine pelagic fish
    res_hazard = model.evaluate(impedance_ratio=1.50, phase_angle=-6.0, temperature=15.0, nh3_ppm=22.0)
    assert res_hazard["freshness_score"] <= 35.0
    assert res_hazard["histamine_hazard_alert"] is True
    assert "BAHAYA HISTAMIN" in res_hazard["diagnosis"]

def test_milk_lactic_acidification_and_ph():
    model = FreshnessFuzzyModel(ingredient_profile="susu_segar")
    assert model.is_liquid_mode is True

    # Fresh cow's milk (normal conductivity ~ 4.8 mS/cm, normal pH 6.6, 4°C)
    res_fresh = model.evaluate_liquid(conductivity_mscm=4.8, ph_value=6.6, temperature=4.0)
    assert res_fresh["freshness_score"] >= 70.0
    assert res_fresh["classification"] == "FRESH"
    assert "SNI 3141.1:2011" in res_fresh["sni_standard"]
    assert res_fresh["sni_status"] == "MEMENUHI SYARAT"

    # Souring / curdled milk (surging conductivity ~ 8.2 mS/cm, acidic pH 4.8)
    res_sour = model.evaluate_liquid(conductivity_mscm=8.2, ph_value=4.8, temperature=4.0)
    assert res_sour["freshness_score"] <= 25.0
    assert res_sour["classification"] == "SPOILED"
    assert "ACID SOURING" in res_sour["diagnosis"]
    assert res_sour["sni_status"] == "TIDAK MEMENUHI SYARAT"

def test_freeze_thaw_resolution_via_nh3():
    model = FreshnessFuzzyModel(ingredient_profile="ayam_broiler")
    # Frozen-thawed chicken: ruptured membranes (R_eis = 1.08), but zero microbial ammonia (1.5 ppm) at 4°C
    res_ft = model.evaluate(impedance_ratio=1.08, phase_angle=-2.5, temperature=4.0, nh3_ppm=1.5)
    # Model should recognize freeze-thaw damage rather than outright bacterial condemnation
    assert "FREEZE-THAW" in res_ft["diagnosis"]
    assert 40.0 <= res_ft["freshness_score"] <= 65.0

def test_tempe_freshness_and_ammonia_inference():
    model = FreshnessFuzzyModel(ingredient_profile="tempe_kedelai")
    meta = model.get_metadata()
    assert "SNI 3142:2018" in meta["sni_standard"]

    # Fresh tempe: tight Rhizopus mycelia, low ammonia, 4°C
    res_fresh = model.evaluate(impedance_ratio=2.10, phase_angle=-15.0, temperature=4.0, nh3_ppm=2.5)
    assert res_fresh["freshness_score"] >= 68.0
    assert res_fresh["classification"] == "FRESH"
    assert res_fresh["sni_status"] == "MEMENUHI SYARAT"

    # Over-fermented / spoiled tempe: lysed mycelium, ammonia surge (tempe busuk)
    res_spoiled = model.evaluate(impedance_ratio=1.12, phase_angle=-2.0, temperature=4.0, nh3_ppm=32.0)
    assert res_spoiled["freshness_score"] <= 30.0
    assert res_spoiled["classification"] == "SPOILED"
    assert res_spoiled["sni_status"] == "TIDAK MEMENUHI SYARAT"
    assert "AMMONIA" in res_spoiled["diagnosis"] or "PROTEOLYSIS" in res_spoiled["diagnosis"]

def test_tahu_freshness_and_gel_liquefaction_inference():
    model = FreshnessFuzzyModel(ingredient_profile="tahu_putih")
    meta = model.get_metadata()
    assert "SNI 01-3144-1998" in meta["sni_standard"]

    # Fresh tofu: coherent protein gel network, clean headspace
    res_fresh = model.evaluate(impedance_ratio=1.85, phase_angle=-13.0, temperature=4.0, nh3_ppm=1.8)
    assert res_fresh["freshness_score"] >= 68.0
    assert res_fresh["classification"] == "FRESH"
    assert res_fresh["sni_status"] == "MEMENUHI SYARAT"

    # Spoiled tofu: bacterial gel liquefaction, sour/ammoniacal off-odors
    res_spoiled = model.evaluate(impedance_ratio=1.08, phase_angle=-1.5, temperature=4.0, nh3_ppm=24.0)
    assert res_spoiled["freshness_score"] <= 30.0
    assert res_spoiled["classification"] == "SPOILED"
    assert res_spoiled["sni_status"] == "TIDAK MEMENUHI SYARAT"
