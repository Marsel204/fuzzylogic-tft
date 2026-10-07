"""
Fuzzy Freshness Model
Literature-Grounded Mamdani Fuzzy Inference System for Food Freshness Detection
Supports 5 Indonesian Free Nutritious Meals (MBG) Ingredient Profiles:
1. Daging Ayam Broiler (SNI 3924:2009)
2. Daging Sapi (SNI 3932:2008)
3. Ikan Air Tawar - Nila & Lele (SNI 2729:2021)
4. Ikan Laut Pelagis - Tongkol/Cakalang (SNI 2729:2021 Annex B Histamin)
5. Susu Sapi Segar (SNI 3141.1:2011)

Classification: [LITERATURE] / [MARGO] / [SNI-STANDARD] / [ENGINEERING-INFERENCE]
"""

from typing import Dict, Any, List, Optional
import numpy as np
from membership_functions import TriangularMF, TrapezoidalMF
from feature_processing import MBG_PROFILES

class FreshnessFuzzyModel:
    """
    Literature-grounded Mamdani Multi-Sensor Fuzzy Inference System.
    """

    def __init__(self, ingredient_profile: str = "ikan_nila_lele", is_calibrated: bool = False):
        if ingredient_profile not in MBG_PROFILES:
            raise ValueError(f"Unknown ingredient_profile: {ingredient_profile}. Available: {list(MBG_PROFILES.keys())}")

        self.ingredient_profile = ingredient_profile
        self.profile_config = MBG_PROFILES[ingredient_profile]
        self.is_liquid_mode = (self.profile_config["mode"] == "liquid_colloid")
        self.requires_experimental_calibration = not is_calibrated
        self.output_name = "freshness"

        self.setup_universes_and_mfs()
        self.setup_rules()

    def setup_universes_and_mfs(self):
        """Configure input/output universes based on selected profile mode."""
        self.output_universe = (0.0, 100.0)
        self.output_mfs = {
            "SPOILED": TrapezoidalMF(0.0, 0.0, 15.0, 35.0, name="SPOILED"),
            "ACCEPTABLE": TriangularMF(25.0, 50.0, 75.0, name="ACCEPTABLE"),
            "FRESH": TrapezoidalMF(65.0, 85.0, 100.0, 100.0, name="FRESH")
        }

        if not self.is_liquid_mode:
            # Solid Muscle Mode (Ayam, Sapi, Ikan Nila, Ikan Tongkol)
            # Profile-specific ratio tuning:
            if self.ingredient_profile == "ayam_broiler":
                # Shinji et al. (2026): Fresh ~ 2.45, Day 10 ~ 1.15
                ratio_mfs = {
                    "LOW": TrapezoidalMF(0.8, 0.8, 1.20, 1.45, name="LOW"),
                    "MEDIUM": TriangularMF(1.30, 1.75, 2.20, name="MEDIUM"),
                    "HIGH": TrapezoidalMF(1.95, 2.30, 3.20, 3.20, name="HIGH")
                }
                nh3_high_thresh = 25.0
            elif self.ingredient_profile == "daging_sapi":
                # Damez et al. (2008), Liu et al. (2022): Fresh ~ 1.9-2.2, Day 5 ~ 1.15
                ratio_mfs = {
                    "LOW": TrapezoidalMF(0.8, 0.8, 1.18, 1.40, name="LOW"),
                    "MEDIUM": TriangularMF(1.25, 1.60, 2.05, name="MEDIUM"),
                    "HIGH": TrapezoidalMF(1.85, 2.15, 3.20, 3.20, name="HIGH")
                }
                nh3_high_thresh = 20.0
            elif self.ingredient_profile == "ikan_tongkol":
                # Pelagic fish with high histidine content
                ratio_mfs = {
                    "LOW": TrapezoidalMF(0.8, 0.8, 1.15, 1.40, name="LOW"),
                    "MEDIUM": TriangularMF(1.25, 1.65, 2.10, name="MEDIUM"),
                    "HIGH": TrapezoidalMF(1.90, 2.30, 3.20, 3.20, name="HIGH")
                }
                nh3_high_thresh = 15.0  # Stricter ammonia limit for histamine control
            elif self.ingredient_profile == "tempe_kedelai":
                # Intact Rhizopus mycelium network (fresh ~ 1.9-2.3, over-fermented/lysed ~ 1.1)
                ratio_mfs = {
                    "LOW": TrapezoidalMF(0.8, 0.8, 1.18, 1.40, name="LOW"),
                    "MEDIUM": TriangularMF(1.25, 1.65, 2.05, name="MEDIUM"),
                    "HIGH": TrapezoidalMF(1.85, 2.15, 3.20, 3.20, name="HIGH")
                }
                nh3_high_thresh = 20.0
            elif self.ingredient_profile == "tahu_putih":
                # Coagulated soy protein gel (coherent ~ 1.7-2.1, hydrolyzed gel ~ 1.1)
                ratio_mfs = {
                    "LOW": TrapezoidalMF(0.8, 0.8, 1.15, 1.35, name="LOW"),
                    "MEDIUM": TriangularMF(1.20, 1.50, 1.85, name="MEDIUM"),
                    "HIGH": TrapezoidalMF(1.65, 1.95, 3.20, 3.20, name="HIGH")
                }
                nh3_high_thresh = 18.0
            else:
                # Default Freshwater Fish (Nila / Lele / Carp)
                ratio_mfs = {
                    "LOW": TrapezoidalMF(0.8, 0.8, 1.15, 1.40, name="LOW"),
                    "MEDIUM": TriangularMF(1.25, 1.65, 2.10, name="MEDIUM"),
                    "HIGH": TrapezoidalMF(1.90, 2.30, 3.2, 3.2, name="HIGH")
                }
                nh3_high_thresh = 25.0

            self.inputs = {
                "impedance_ratio": {
                    "universe": (0.8, 3.2),
                    "mfs": ratio_mfs
                },
                "phase_angle": {
                    "universe": (-25.0, 0.0),
                    "mfs": {
                        "FLAT": TrapezoidalMF(-5.0, -3.0, 0.0, 0.0, name="FLAT"),
                        "MODERATE": TriangularMF(-13.0, -8.5, -4.0, name="MODERATE"),
                        "PROMINENT": TrapezoidalMF(-25.0, -25.0, -15.0, -9.0, name="PROMINENT")
                    }
                },
                "temperature": {
                    "universe": (0.0, 25.0),
                    "mfs": {
                        "COLD": TrapezoidalMF(0.0, 0.0, 4.0, 7.0, name="COLD"),
                        "MILD": TriangularMF(5.0, 10.0, 16.0, name="MILD"),
                        "WARM": TrapezoidalMF(13.0, 18.0, 25.0, 25.0, name="WARM")
                    }
                },
                "nh3_ppm": {
                    "universe": (0.0, 100.0),
                    "mfs": {
                        "LOW": TrapezoidalMF(0.0, 0.0, 4.0, 8.0, name="LOW"),
                        "MEDIUM": TriangularMF(6.0, nh3_high_thresh * 0.6, nh3_high_thresh),
                        "HIGH": TrapezoidalMF(nh3_high_thresh * 0.8, nh3_high_thresh * 1.2, 100.0, 100.0, name="HIGH")
                    }
                }
            }
        else:
            # Liquid Colloid Mode (Susu Sapi Segar)
            # SNI 3141.1:2011: Fresh conductivity 4.0-5.8 mS/cm, pH 6.3-6.8
            self.inputs = {
                "conductivity_mscm": {
                    "universe": (3.0, 15.0),
                    "mfs": {
                        "NORMAL": TrapezoidalMF(3.0, 3.5, 5.2, 6.0, name="NORMAL"),
                        "ELEVATED": TriangularMF(5.5, 6.8, 7.8, name="ELEVATED"),
                        "HIGH": TrapezoidalMF(7.2, 8.2, 15.0, 15.0, name="HIGH")
                    }
                },
                "ph_value": {
                    "universe": (3.5, 8.0),
                    "mfs": {
                        "ACIDIC": TrapezoidalMF(3.5, 3.5, 5.2, 5.8, name="ACIDIC"),
                        "NORMAL": TriangularMF(5.7, 6.55, 7.1, name="NORMAL"),
                        "ALKALINE": TrapezoidalMF(7.0, 7.5, 8.0, 8.0, name="ALKALINE")
                    }
                },
                "temperature": {
                    "universe": (0.0, 25.0),
                    "mfs": {
                        "COLD": TrapezoidalMF(0.0, 0.0, 4.0, 7.0, name="COLD"),
                        "MILD": TriangularMF(5.0, 10.0, 16.0, name="MILD"),
                        "WARM": TrapezoidalMF(13.0, 18.0, 25.0, 25.0, name="WARM")
                    }
                }
            }

    def setup_rules(self):
        """Define rule bases for solid vs liquid operating modes."""
        if not self.is_liquid_mode:
            # Solid Muscle Mode Rules (11 Rules)
            self.rules = [
                {"id": 1, "conditions": {"impedance_ratio": "HIGH", "phase_angle": "PROMINENT", "temperature": "COLD"}, "consequent": "FRESH"},
                {"id": 2, "conditions": {"impedance_ratio": "HIGH", "phase_angle": "PROMINENT", "temperature": "MILD"}, "consequent": "ACCEPTABLE"},
                {"id": 3, "conditions": {"impedance_ratio": "HIGH", "phase_angle": "MODERATE", "temperature": "COLD"}, "consequent": "FRESH"},
                {"id": 4, "conditions": {"impedance_ratio": "MEDIUM", "phase_angle": "MODERATE", "temperature": "COLD"}, "consequent": "ACCEPTABLE"},
                {"id": 5, "conditions": {"impedance_ratio": "MEDIUM", "phase_angle": "PROMINENT", "temperature": "COLD"}, "consequent": "FRESH"},
                {"id": 6, "conditions": {"impedance_ratio": "MEDIUM", "phase_angle": "FLAT"}, "consequent": "SPOILED"},
                {"id": 7, "conditions": {"impedance_ratio": "LOW"}, "consequent": "SPOILED"},
                {"id": 8, "conditions": {"impedance_ratio": "LOW", "phase_angle": "FLAT"}, "consequent": "SPOILED"},
                {"id": 9, "conditions": {"phase_angle": "FLAT", "temperature": "WARM"}, "consequent": "SPOILED"},
                {"id": 10, "conditions": {"impedance_ratio": "MEDIUM", "temperature": "WARM"}, "consequent": "SPOILED"},
                {"id": 11, "conditions": {"impedance_ratio": "HIGH", "temperature": "WARM"}, "consequent": "ACCEPTABLE"},
            ]
        else:
            # Liquid Milk Rules (SNI 3141.1:2011)
            self.rules = [
                {"id": 1, "conditions": {"conductivity_mscm": "NORMAL", "ph_value": "NORMAL", "temperature": "COLD"}, "consequent": "FRESH"},
                {"id": 2, "conditions": {"conductivity_mscm": "NORMAL", "ph_value": "NORMAL", "temperature": "MILD"}, "consequent": "ACCEPTABLE"},
                {"id": 3, "conditions": {"conductivity_mscm": "NORMAL", "ph_value": "NORMAL", "temperature": "WARM"}, "consequent": "ACCEPTABLE"},
                {"id": 4, "conditions": {"conductivity_mscm": "ELEVATED", "ph_value": "NORMAL", "temperature": "COLD"}, "consequent": "ACCEPTABLE"},
                {"id": 5, "conditions": {"ph_value": "ACIDIC"}, "consequent": "SPOILED"},
                {"id": 6, "conditions": {"conductivity_mscm": "HIGH"}, "consequent": "SPOILED"},
                {"id": 7, "conditions": {"conductivity_mscm": "ELEVATED", "temperature": "WARM"}, "consequent": "SPOILED"},
                {"id": 8, "conditions": {"conductivity_mscm": "HIGH", "ph_value": "ACIDIC"}, "consequent": "SPOILED"},
            ]

    def _indeterminate_result(self, inputs: Dict[str, Any], diagnosis: str) -> Dict[str, Any]:
        """No freshness or compliance verdict is available for this inspection."""
        return {
            "profile": self.ingredient_profile,
            "indonesian_name": self.profile_config["indonesian_name"],
            "sni_standard": self.profile_config["sni_standard"],
            "sni_status": "BELUM DAPAT DITENTUKAN (ULANGI PENGUKURAN)",
            "freshness_score": None,
            "classification": "INDETERMINATE",
            "diagnosis": diagnosis,
            "histamine_hazard_alert": False,
            "inputs": inputs,
            "requires_experimental_calibration": self.requires_experimental_calibration
        }

    def evaluate(
        self,
        impedance_ratio: float,
        phase_angle: float,
        temperature: float,
        nh3_ppm: Optional[float] = None,
        defuzz_points: int = 200
    ) -> Dict[str, Any]:
        """
        Evaluate freshness for solid muscle tissues with NH3 multi-sensor cross-validation.
        """
        if self.is_liquid_mode:
            raise TypeError("Model configured for liquid milk ('susu_segar'). Call evaluate_liquid() instead.")

        inputs = {
            "impedance_ratio": float(impedance_ratio),
            "phase_angle": float(phase_angle),
            "temperature": float(temperature),
            "nh3_ppm": float(nh3_ppm) if nh3_ppm is not None else None
        }
        if any(v is None or not np.isfinite(v) for v in inputs.values()):
            return self._indeterminate_result(inputs, "MISSING OR INVALID SENSOR READINGS")
        if inputs["impedance_ratio"] <= 0 or inputs["nh3_ppm"] < 0:
            return self._indeterminate_result(inputs, "INVALID SENSOR READINGS")

        # Check special cross-sensor conditions
        diagnosis = "NORMAL TISSUE DEGRADATION"
        histamine_alert = False

        # 1. Freeze-Thaw Cross-Validation:
        # Cell membranes ruptured (R_eis < 1.25), but NH3 is low (< 5.0 ppm) and cold storage
        # Only applies to animal muscle (beef, chicken, fish); not fermented tempe or tofu
        is_freeze_thaw = (
            inputs["impedance_ratio"] <= 1.25
            and inputs["nh3_ppm"] <= 5.0
            and inputs["temperature"] <= 8.0
            and self.ingredient_profile not in ["tempe_kedelai", "tahu_putih"]
        )

        # 2. Ammonia High Override:
        # If volatile basic nitrogen is high, it is unequivocally spoiled regardless of deep core impedance
        nh3_high = (inputs["nh3_ppm"] >= self.profile_config.get("nh3_limit_ppm", 25.0))

        # 3. Tongkol Scombroid Histamine Risk:
        if self.ingredient_profile == "ikan_tongkol":
            if inputs["nh3_ppm"] >= 15.0 or (inputs["temperature"] >= 10.0 and inputs["impedance_ratio"] <= 1.80):
                histamine_alert = True
                diagnosis = "BAHAYA HISTAMIN (SCOMBROID POISONING RISK)"

        # Standard Fuzzification
        fuzzified: Dict[str, Dict[str, float]] = {}
        for var_name, var_info in self.inputs.items():
            fuzzified[var_name] = {}
            val = inputs[var_name]
            for term, mf in var_info["mfs"].items():
                fuzzified[var_name][term] = mf.evaluate(val)

        # Rule evaluation
        consequent_activations = {"SPOILED": 0.0, "ACCEPTABLE": 0.0, "FRESH": 0.0}
        active_rules = []

        for rule in self.rules:
            weights = []
            for var_name, term in rule["conditions"].items():
                weights.append(fuzzified[var_name][term])
            activation = min(weights) if weights else 0.0

            if activation > 0:
                active_rules.append({"rule_id": rule["id"], "activation": activation, "consequent": rule["consequent"]})
                consequent_activations[rule["consequent"]] = max(consequent_activations[rule["consequent"]], activation)

        # Defuzzification
        y_grid = np.linspace(self.output_universe[0], self.output_universe[1], defuzz_points)
        aggregated_mf = np.zeros_like(y_grid)
        for term, activation in consequent_activations.items():
            if activation > 0:
                mf_vals = self.output_mfs[term].evaluate_array(y_grid)
                clipped = np.minimum(activation, mf_vals)
                aggregated_mf = np.maximum(aggregated_mf, clipped)

        sum_weights = np.sum(aggregated_mf)
        freshness_score = float(np.sum(y_grid * aggregated_mf) / sum_weights) if sum_weights > 1e-9 else float("nan")

        # Apply Cross-Sensor Diagnostics Overrides
        if is_freeze_thaw:
            diagnosis = "FREEZE-THAW DAMAGE (CHEMICALLY SAFE / NON-MICROBIAL)"
            freshness_score = 52.0  # Safe for cooked consumption, not fresh
        elif nh3_high or histamine_alert:
            freshness_score = min(freshness_score, 20.0) if np.isfinite(freshness_score) else 20.0
            if histamine_alert:
                pass
            elif self.ingredient_profile == "tempe_kedelai":
                diagnosis = "OVER-FERMENTATION / PROTEOLYTIC SPOILAGE (HIGH AMMONIA / TEMPE BUSUK)"
            elif self.ingredient_profile == "tahu_putih":
                diagnosis = "BACTERIAL GEL LIQUEFACTION / FOUL SPOILAGE"
            else:
                diagnosis = "BACTERIAL PROTEOLYSIS (HIGH VOLATILE AMINES)"

        if not np.isfinite(freshness_score):
            return self._indeterminate_result(inputs, "NO MATCHING FUZZY RULES")

        # Classification & SNI Status
        if freshness_score >= 65.0:
            classification = "FRESH"
            sni_status = "MEMENUHI SYARAT"
        elif freshness_score <= 35.0:
            classification = "SPOILED"
            sni_status = "TIDAK MEMENUHI SYARAT"
        else:
            classification = "ACCEPTABLE"
            sni_status = "MEMENUHI SYARAT KELAYAKAN (SEGERA OLAH)"

        return {
            "profile": self.ingredient_profile,
            "indonesian_name": self.profile_config["indonesian_name"],
            "sni_standard": self.profile_config["sni_standard"],
            "sni_status": sni_status,
            "freshness_score": round(freshness_score, 2),
            "classification": classification,
            "diagnosis": diagnosis,
            "histamine_hazard_alert": histamine_alert,
            "inputs": inputs,
            "requires_experimental_calibration": self.requires_experimental_calibration
        }

    def evaluate_liquid(
        self,
        conductivity_mscm: float,
        ph_value: Optional[float],
        temperature: float,
        defuzz_points: int = 200
    ) -> Dict[str, Any]:
        """
        Evaluate freshness for liquid dairy (Susu Sapi Segar) using Tetrapolar Conductivity + pH.
        Mapped to SNI 3141.1:2011.
        """
        if not self.is_liquid_mode:
            raise TypeError("evaluate_liquid() is only valid for liquid milk ('susu_segar').")

        inputs = {
            "conductivity_mscm": float(conductivity_mscm),
            "ph_value": float(ph_value) if ph_value is not None else None,
            "temperature": float(temperature)
        }
        if any(v is None or not np.isfinite(v) for v in inputs.values()):
            return self._indeterminate_result(inputs, "MISSING OR INVALID SENSOR READINGS")
        if inputs["conductivity_mscm"] <= 0 or not 0 <= inputs["ph_value"] <= 14:
            return self._indeterminate_result(inputs, "INVALID SENSOR READINGS")

        # Fuzzification
        fuzzified: Dict[str, Dict[str, float]] = {}
        for var_name, var_info in self.inputs.items():
            fuzzified[var_name] = {}
            val = inputs[var_name]
            for term, mf in var_info["mfs"].items():
                fuzzified[var_name][term] = mf.evaluate(val)

        # Rule evaluation
        consequent_activations = {"SPOILED": 0.0, "ACCEPTABLE": 0.0, "FRESH": 0.0}
        active_rules = []
        for rule in self.rules:
            weights = []
            for var_name, term in rule["conditions"].items():
                weights.append(fuzzified[var_name][term])
            activation = min(weights) if weights else 0.0
            if activation > 0:
                active_rules.append({"rule_id": rule["id"], "activation": activation, "consequent": rule["consequent"]})
                consequent_activations[rule["consequent"]] = max(consequent_activations[rule["consequent"]], activation)

        # Defuzzification
        y_grid = np.linspace(self.output_universe[0], self.output_universe[1], defuzz_points)
        aggregated_mf = np.zeros_like(y_grid)
        for term, activation in consequent_activations.items():
            if activation > 0:
                mf_vals = self.output_mfs[term].evaluate_array(y_grid)
                clipped = np.minimum(activation, mf_vals)
                aggregated_mf = np.maximum(aggregated_mf, clipped)

        sum_weights = np.sum(aggregated_mf)
        freshness_score = float(np.sum(y_grid * aggregated_mf) / sum_weights) if sum_weights > 1e-9 else float("nan")

        # Diagnosis
        diagnosis = "NORMAL FRESH MILK"
        if inputs["ph_value"] < 5.80 or inputs["conductivity_mscm"] > 7.2:
            freshness_score = min(freshness_score, 18.0) if np.isfinite(freshness_score) else 18.0
            diagnosis = "ACID SOURING (FERMENTASI ASAM LAKTAT / GUMPALAN)"

        if not np.isfinite(freshness_score):
            return self._indeterminate_result(inputs, "NO MATCHING FUZZY RULES")

        # Classification & SNI 3141.1 Status
        if freshness_score >= 65.0:
            classification = "FRESH"
            sni_status = "MEMENUHI SYARAT"
        elif freshness_score <= 35.0:
            classification = "SPOILED"
            sni_status = "TIDAK MEMENUHI SYARAT"
        else:
            classification = "ACCEPTABLE"
            sni_status = "MEMENUHI SYARAT KELAYAKAN (SEGERA PASTEURISASI)"

        return {
            "profile": self.ingredient_profile,
            "indonesian_name": self.profile_config["indonesian_name"],
            "sni_standard": self.profile_config["sni_standard"],
            "sni_status": sni_status,
            "freshness_score": round(freshness_score, 2),
            "classification": classification,
            "diagnosis": diagnosis,
            "inputs": inputs,
            "requires_experimental_calibration": self.requires_experimental_calibration
        }

    def evaluate_sugeno(self, impedance_ratio: float, phase_angle: float, temperature: float) -> float:
        """Sugeno singleton inference helper for ANFIS benchmark."""
        inputs = {"impedance_ratio": float(impedance_ratio), "phase_angle": float(phase_angle), "temperature": float(temperature)}
        singletons = {"SPOILED": 15.0, "ACCEPTABLE": 50.0, "FRESH": 88.0}
        rule_weights, rule_outputs = [], []
        for rule in self.rules:
            weights = [self.inputs[v]["mfs"][t].evaluate(inputs[v]) for v, t in rule["conditions"].items() if v in inputs]
            w = min(weights) if weights else 0.0
            if w > 0:
                rule_weights.append(w)
                rule_outputs.append(singletons[rule["consequent"]])
        return float(np.average(rule_outputs, weights=rule_weights)) if rule_weights and sum(rule_weights) > 0 else 50.0

    def get_metadata(self) -> Dict[str, Any]:
        """Return profile metadata, SNI standards, and probe constraints."""
        meta = {
            "ingredient_profile": self.ingredient_profile,
            "indonesian_name": self.profile_config["indonesian_name"],
            "sni_standard": self.profile_config["sni_standard"],
            "mode": self.profile_config["mode"],
            "requires_experimental_calibration": self.requires_experimental_calibration,
            "probe_orientation_requirement": self.profile_config.get("probe_orientation_requirement", "Standard insertion depth 8 mm"),
            "calibration_notice": "Requires empirical calibration on final physical probe before deployment."
        }
        return meta
