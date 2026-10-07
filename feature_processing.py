"""
EIS Feature Extraction and Hardware Interface Module
Literature-Grounded Multi-Sensor Freshness Model
Supports 5 Indonesian Free Meals (MBG) Ingredient Profiles
Classification: [LITERATURE] / [MARGO] / [SNI-STANDARD] / [ENGINEERING-INFERENCE]
"""

from typing import Dict, Any, Union, Optional
import numpy as np

# Master registry of Indonesian Free Meals (MBG) ingredient profiles
MBG_PROFILES: Dict[str, Dict[str, Any]] = {
    "ayam_broiler": {
        "indonesian_name": "Daging Ayam Broiler (Karkas/Fillet)",
        "mode": "solid_muscle",
        "f_lf": 2000.0,     # [LITERATURE: Shinji et al. 2026]
        "f_hf": 100000.0,
        "f_char": 10000.0,
        "sni_standard": "SNI 3924:2009 (Mutu Karkas dan Daging Ayam)",
        "tvc_limit_log": 6.0,  # < 1 x 10^6 CFU/g
        "tvbn_limit": 20.0,    # mg N / 100g
        "nh3_limit_ppm": 25.0,
        "anisotropy_safe": True
    },
    "daging_sapi": {
        "indonesian_name": "Daging Sapi (Otot Skelet)",
        "mode": "solid_muscle",
        "f_lf": 1000.0,     # [LITERATURE: Damez et al. 2008, Liu et al. 2022]
        "f_hf": 100000.0,
        "f_char": 10000.0,
        "sni_standard": "SNI 3932:2008 (Mutu Karkas dan Daging Sapi)",
        "tvc_limit_log": 6.0,
        "tvbn_limit": 20.0,
        "nh3_limit_ppm": 20.0,
        "anisotropy_safe": False,
        "probe_orientation_requirement": "Transverse probe orientation required (strictly perpendicular to muscle fibers to avoid 250% anisotropy error)"
    },
    "ikan_nila_lele": {
        "indonesian_name": "Ikan Air Tawar (Ikan Nila & Ikan Lele)",
        "mode": "solid_muscle",
        "f_lf": 4000.0,     # [LITERATURE: Liang et al. 2020, Sun et al. 2018]
        "f_hf": 100000.0,
        "f_char": 10000.0,
        "sni_standard": "SNI 2729:2021 (Ikan Segar)",
        "tvc_limit_log": 5.7,  # < 5.0 x 10^5 CFU/g
        "tvbn_limit": 20.0,
        "nh3_limit_ppm": 25.0,
        "anisotropy_safe": True
    },
    "ikan_tongkol": {
        "indonesian_name": "Ikan Laut Pelagis (Ikan Tongkol / Cakalang)",
        "mode": "solid_muscle",
        "f_lf": 4000.0,
        "f_hf": 100000.0,
        "f_char": 10000.0,
        "sni_standard": "SNI 2729:2021 (Annex B Histamin Scombridae)",
        "tvc_limit_log": 5.7,
        "tvbn_limit": 25.0,
        "nh3_limit_ppm": 15.0,  # Lower threshold due to rapid scombroid histamine generation
        "histamine_limit_ppm": 100.0,
        "anisotropy_safe": True
    },
    "susu_segar": {
        "indonesian_name": "Susu Sapi Segar (Liquid)",
        "mode": "liquid_colloid",
        "f_meas": 10000.0,   # 10 kHz tetrapolar dip measurement
        "sni_standard": "SNI 3141.1:2011 (Susu Segar - Sapi)",
        "tvc_limit_log": 6.0,  # < 1 x 10^6 CFU/mL
        "ph_nominal_range": (6.30, 6.80),
        "acidity_sh_max": 7.0,  # Soxhlet-Henkel degrees
        "conductivity_fresh_range_mscm": (4.0, 5.8),
        "anisotropy_safe": True
    },
    "tempe_kedelai": {
        "indonesian_name": "Tempe Kedelai (Fermentasi Rhizopus oligosporus)",
        "mode": "solid_muscle",
        "f_lf": 4000.0,     # [LITERATURE & BIO-INFERENCE: intact fungal hyphae capacitance]
        "f_hf": 100000.0,
        "f_char": 10000.0,
        "sni_standard": "SNI 3142:2018 (Tempe Kedelai)",
        "tvc_limit_log": 7.0,
        "tvbn_limit": 20.0,
        "nh3_limit_ppm": 20.0,  # Ammonia threshold for over-ripeness / tempe busuk
        "anisotropy_safe": True
    },
    "tahu_putih": {
        "indonesian_name": "Tahu Putih / Tahu Kedelai (Protein Gel Matrix)",
        "mode": "solid_muscle",
        "f_lf": 2000.0,     # [LITERATURE: Li et al. 2011 J. Food Eng.; low-freq gel boundary 2 kHz]
        "f_hf": 100000.0,
        "f_char": 10000.0,
        "sni_standard": "SNI 01-3144-1998 (Tahu)",
        "tvc_limit_log": 6.0,
        "tvbn_limit": 20.0,
        "nh3_limit_ppm": 18.0,  # Bacterial gel liquefaction and foul odor threshold
        "anisotropy_safe": True
    }
}


def calculate_impedance_ratio(z_lf: float, z_hf: float) -> float:
    """
    Calculate dual-frequency impedance ratio: R_eis = |Z_lf| / |Z_hf|
    [LITERATURE: Shinji et al. 2026; Pliquett et al. 2003; Zhao et al. 2017]
    """
    if z_hf <= 0:
        raise ValueError("High-frequency impedance must be strictly positive.")
    return float(z_lf / z_hf)


def calculate_py_parameter(z_lf: float, z_hf: float) -> float:
    """
    Calculate Pliquett Py-parameter: Py = (|Z_lf| - |Z_hf|) / |Z_hf|
    [LITERATURE: Pliquett et al. 2003]
    """
    if z_hf <= 0:
        raise ValueError("High-frequency impedance must be strictly positive.")
    return float((z_lf - z_hf) / z_hf)


def temperature_compensate_impedance(
    z_measured: float,
    temp_c: float,
    temp_ref: float = 4.0,
    alpha: float = 0.0156
) -> float:
    """
    Temperature compensation for measured tissue impedance.
    [LITERATURE: Biological animal tissue mean temperature coefficient = -1.56%/°C]
    Z_ref = Z_meas * (1 + alpha * (temp_c - temp_ref))
    """
    delta_t = temp_c - temp_ref
    correction_factor = 1.0 + alpha * delta_t
    return float(z_measured * correction_factor)


def calculate_liquid_conductivity(
    z_tetra_modulus: float,
    k_cell: float = 130.0
) -> float:
    """
    Compute bulk electrolytic conductivity from tetrapolar liquid impedance.
    [MARGO: Physiol. Meas. 34 (2013) 391-405, Section 5]

    sigma (S/m) = K_cell (m^-1) / |Z_tetra| (Ohm)
    Returns conductivity in mS/cm: 1 S/m = 10 mS/cm.
    """
    if z_tetra_modulus <= 0:
        raise ValueError("Tetrapolar modulus must be strictly positive.")
    sigma_sm = k_cell / z_tetra_modulus
    return float(sigma_sm * 10.0)  # Convert to mS/cm


def temperature_compensate_milk_conductivity(
    sigma_meas: float,
    temp_c: float,
    temp_ref: float = 20.0,
    beta: float = 0.020
) -> float:
    """
    Temperature compensation for milk electrolytic conductivity (+2.0%/°C).
    [LITERATURE: Mabrook & Petty 2003, Bertemes-Filho 2010]
    sigma_ref = sigma_meas / (1 + beta * (temp_c - temp_ref))
    """
    correction = 1.0 + beta * (temp_c - temp_ref)
    if correction <= 0:
        return sigma_meas
    return float(sigma_meas / correction)


def extract_ad5933_impedance(
    xv: complex,
    xi: complex,
    k_cal: complex
) -> complex:
    """
    Compute unknown impedance from AD5933 two-step tetrapolar acquisition.
    [MARGO: Eq. 4] Zx(f) = (Xv(f) / Xi(f)) * K(f)
    """
    if abs(xi) == 0:
        raise ZeroDivisionError("Current coefficient Xi is zero.")
    return complex((xv / xi) * k_cal)


def normalize_impedance_to_initial(
    z_series: Union[np.ndarray, list],
    z_initial: float
) -> np.ndarray:
    """Normalize impedance trajectory to Day 0 initial baseline: Z(t) / Z(0)"""
    if z_initial <= 0:
        raise ValueError("Initial impedance baseline must be strictly positive.")
    arr = np.asarray(z_series, dtype=float)
    return arr / z_initial


class AD5933SpectralFeatureExtractor:
    """
    Feature extractor interfacing between AD5933 hardware and the fuzzy logic engine.
    Supports MBG species profile presets.
    """
    def __init__(self, ingredient_profile: str = "ikan_nila_lele"):
        if ingredient_profile not in MBG_PROFILES:
            raise ValueError(f"Unknown profile: {ingredient_profile}. Available: {list(MBG_PROFILES.keys())}")

        self.profile = MBG_PROFILES[ingredient_profile]
        self.ingredient_profile = ingredient_profile
        self.mode = self.profile["mode"]

        if self.mode == "solid_muscle":
            self.f_lf = self.profile["f_lf"]
            self.f_hf = self.profile["f_hf"]
            self.f_char = self.profile["f_char"]
        else:
            self.f_meas = self.profile["f_meas"]

    def extract_features_from_spectrum(
        self,
        frequencies: np.ndarray,
        impedance_spectrum: np.ndarray,
        temperature_c: float,
        nh3_ppm: Optional[float] = None,
        ph_value: Optional[float] = None,
        k_cell: float = 130.0
    ) -> Dict[str, Any]:
        """Extract multi-sensor features from calibrated complex impedance spectrum."""
        if self.mode == "solid_muscle":
            idx_lf = int(np.argmin(np.abs(frequencies - self.f_lf)))
            idx_hf = int(np.argmin(np.abs(frequencies - self.f_hf)))
            idx_char = int(np.argmin(np.abs(frequencies - self.f_char)))

            z_lf = float(np.abs(impedance_spectrum[idx_lf]))
            z_hf = float(np.abs(impedance_spectrum[idx_hf]))
            phase_char = float(np.angle(impedance_spectrum[idx_char], deg=True))

            r_eis = calculate_impedance_ratio(z_lf, z_hf)
            py_val = calculate_py_parameter(z_lf, z_hf)

            return {
                "profile": self.ingredient_profile,
                "mode": "solid_muscle",
                "impedance_ratio": r_eis,
                "py_parameter": py_val,
                "phase_angle": phase_char,
                "temperature": float(temperature_c),
                "nh3_ppm": float(nh3_ppm) if nh3_ppm is not None else None,
                "z_lf_magnitude": z_lf,
                "z_hf_magnitude": z_hf,
                "sni_standard": self.profile["sni_standard"]
            }
        else:
            # Liquid mode (Susu Segar)
            idx_meas = int(np.argmin(np.abs(frequencies - self.f_meas)))
            z_mod = float(np.abs(impedance_spectrum[idx_meas]))
            phase_val = float(np.angle(impedance_spectrum[idx_meas], deg=True))
            sigma_mscm = calculate_liquid_conductivity(z_mod, k_cell=k_cell)
            sigma_comp = temperature_compensate_milk_conductivity(sigma_mscm, temp_c=temperature_c)

            return {
                "profile": self.ingredient_profile,
                "mode": "liquid_colloid",
                "conductivity_mscm": sigma_comp,
                "phase_angle": phase_val,
                "temperature": float(temperature_c),
                "ph_value": float(ph_value) if ph_value is not None else None,
                "z_modulus": z_mod,
                "sni_standard": self.profile["sni_standard"]
            }
