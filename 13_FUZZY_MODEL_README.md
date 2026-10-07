# 13. Fuzzy Logic Freshness Model: Software Documentation and Hardware Interface

## 1. Overview

This directory contains the Python implementation of the literature-grounded fuzzy logic freshness model for freshwater fish (Crucian carp / Tilapia).

The code is strictly designed according to Test-Driven Development (TDD) principles and engineered to support both **pre-hardware synthetic simulation** and **direct hardware drop-in replacement** once the AD5933 physical four-electrode instrument is fabricated.

---

## 2. Architecture & File Structure

```
├── membership_functions.py    # Core fuzzy membership function classes (Triangular, Trapezoidal, Gaussian)
├── feature_processing.py      # Feature extraction (R_eis, Py, temperature compensation, AD5933 Eq. 4 interface)
├── fuzzy_model.py             # Mamdani Fuzzy Inference System (Centroid defuzzification, Sugeno ANFIS benchmark)
├── example_inference.py       # Demonstration runner, scenario evaluator, and plot generator
├── tests/
│   ├── test_membership.py    # Unit tests for membership function formulas and bounds
│   ├── test_features.py      # Unit tests for feature extraction and temperature compensation
│   └── test_fuzzy_model.py   # Unit tests for fuzzy inference rules and boundary conditions
├── membership_functions/     # Generated membership function plots
├── rule_analysis/            # Generated 3D fuzzy control surfaces and rule activation diagrams
├── literature_figures/       # Literature dynamics vs synthetic simulation and Margo concepts
└── simulation_results/       # Sensitivity analysis and Mamdani vs Sugeno comparisons
```

---

## 3. Installation and Verification

### Prerequisites
Python 3.10+ with the packages listed in `requirements.txt`.

```bash
python3 -m pip install -r requirements.txt

# Run the complete test suite
pytest tests/ -v

# Run the inference demonstration and generate all plots
python3 example_inference.py
```

`make test` builds and runs the C tests and the Python regression suite, including host tests of the display firmware's serial commands. Set `PYTHON=/path/to/python` when using a virtual environment or a different interpreter. The host firmware tests require a C++17 compiler.

Every required sensor must have a measured value. Missing NH3 or pH readings remain `None` in extracted features. Missing/nonfinite sensor readings and inputs that activate no rules return `classification="INDETERMINATE"`, `freshness_score=None`, and a request to repeat the measurement, without a compliance verdict. The C API uses `MBG_CLASS_INDETERMINATE` and a `NAN` score for the same conditions; callers should check the classification before displaying the score.

---

## 4. Hardware Transition Architecture (AD5933 Interface)

The software is structured so that the fuzzy model does **not** need to be modified when real hardware arrives.

### Simulation Phase (Current Workflow)
```
Synthetic Parameters (R_eis, θ, T) 
            ↓
    [fuzzy_model.py] 
            ↓
  Freshness Score (0–100%)
```

### Hardware Deployment Phase (Future Workflow)
```
    [AD5933 DDS Core + Analog Front-End]
            ↓
   Measured DFT Coefficients: Xv(f), Xi(f)
            ↓
   Calibration Vector K(f) [MARGO: Eq. 5]
            ↓
   [feature_processing.py: extract_ad5933_impedance()]
            ↓
   Complex Impedance Spectrum Zx(f)
            ↓
   [feature_processing.py: AD5933SpectralFeatureExtractor]
   - Extract R_eis = |Z(4kHz)| / |Z(100kHz)|
   - Extract θ(10kHz) = arg(Zx(10kHz))
   - Acquire T (°C) from onboard thermistor
            ↓
   [fuzzy_model.py: FreshnessFuzzyModel.evaluate()]
            ↓
   Calibrated Freshness Score (0–100%)
```

### Hardware Code Snippet:
```python
from feature_processing import AD5933SpectralFeatureExtractor, extract_ad5933_impedance
from fuzzy_model import FreshnessFuzzyModel

# 1. Acquire raw DFT registers from AD5933 over I2C
# Xv = real_v + 1j * imag_v (Voltage channel from AD8250 via MUX)
# Xi = real_i + 1j * imag_i (Current channel from AD8034 via MUX)
# K  = cal_real + 1j * cal_imag (From reference calibration circuit)
z_complex_sweep = [extract_ad5933_impedance(xv, xi, k) for xv, xi, k in raw_measurements]

# 2. Extract evidence-based features
extractor = AD5933SpectralFeatureExtractor(ingredient_profile="ikan_nila_lele")
features = extractor.extract_features_from_spectrum(
    frequencies, z_complex_sweep, temperature_c=measured_temp, nh3_ppm=measured_nh3
)

# 3. Evaluate Fuzzy Inference System
model = FreshnessFuzzyModel(is_calibrated=True)
result = model.evaluate(
    impedance_ratio=features["impedance_ratio"],
    phase_angle=features["phase_angle"],
    temperature=features["temperature"],
    nh3_ppm=features["nh3_ppm"]
)

if result["classification"] == "INDETERMINATE":
    print(result["sni_status"], result["diagnosis"])
else:
    print(f"Freshness Score: {result['freshness_score']}% | Tier: {result['classification']}")
```

---

## 5. Critical Calibration Safeguards

The model class includes an explicit metadata flag:
```python
model = FreshnessFuzzyModel()
print(model.get_metadata()["requires_experimental_calibration"])  # True
```
When running with uncalibrated parameters, the output explicitly warns that numerical boundaries require wet-lab calibration on the target probe geometry before commercial deployment.
