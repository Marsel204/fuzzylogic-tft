# 10. Experimental Validation Protocol for AD5933 Prototype

## 1. Objective

To provide an exhaustive, statistically rigorous protocol for experimentally validating the literature-grounded fuzzy freshness model once the physical four-electrode AD5933 measurement hardware is fabricated.

This protocol will:
1. Empirically calibrate the numerical membership boundaries of the fuzzy model on the specific probe geometry.
2. Quantify prediction accuracy against legally codified chemical (TVB-N) and microbiological (TVC) ground truths.
3. Establish operational limits under cold-chain temperature abuse scenarios.

---

## 2. Experimental Design & Sample Sizing

### 2.1. Biological Sample Specification
- **Target Species:** Freshwater Tilapia (*Oreochromis niloticus*) or Crucian Carp (*Carassius auratus*).
- **Procurement:** Live harvested or freshly slaughtered within $< 2\ \text{hours}$ post-mortem from a verified aquaculture supplier.
- **Sample Uniformity:** Weight $500 \pm 50\ \text{g}$, length $25 \pm 2\ \text{cm}$. Scaled, gutted, washed with sterile deionized water, and fileted.
- **Sample Size ($N$):** 
  - To achieve statistical power $(1 - \beta) = 0.90$ at significance level $\alpha = 0.05$ with an anticipated effect size $R^2 \ge 0.80$, minimum sample size is $N = 45$.
  - To account for inter-batch variance, **$N = 90$ fish fillets** (across 3 independent biological batches of 30 fish each, sampled across different harvesting weeks) will be tested.

### 2.2. Storage Regimes & Testing Cohorts
Fillets will be sealed in sterile polyethylene bags (simulating commercial retail packaging) and split into three parallel cohorts:
1. **Cohort A (Refrigerated Baseline, $N = 40$):** Maintained in a high-precision incubator at $4.0 \pm 0.5^\circ\text{C}$ for 10 days.
2. **Cohort B (Melting Ice Preservation, $N = 25$):** Stored in polystyrene boxes with crushed melting ice at $0.5 \pm 0.5^\circ\text{C}$ for 14 days.
3. **Cohort C (Thermal Abuse, $N = 25$):** Maintained at mild abuse temperature $15.0 \pm 0.5^\circ\text{C}$ for 4 days to validate temperature compensation.

### 2.3. Sampling Timeline
- **Cohort A (4°C):** Measured every 24 hours (Days 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10).
- **Cohort B (0°C):** Measured every 48 hours (Days 0, 2, 4, 6, 8, 10, 12, 14).
- **Cohort C (15°C):** Measured every 8 hours (0, 8, 16, 24, 32, 40, 48, 72, 96 hours).

---

## 3. EIS Hardware & Electrode Probe Specification

### 3.1. Analog Front-End & AD5933 Configuration `[MARGO]`
- **Measurement Architecture:** AD5933 integrated with Margo et al. (2013) four-electrode analog extension.
- **Excitation Signal:** Sinusoidal, $V_{\text{exc}} = 50\ \text{mVpp}$ (attenuated by factor 10 to avoid nonlinear polarization), DC-coupled high-pass filtered (cutoff 10 Hz).
- **Frequency Sweep:** Logarithmic sweep from **1.0 kHz to 100.0 kHz** (30 discrete frequency points per sweep).
- **System Calibration:** Prior to each experimental run, calibration vector $K(f)$ is obtained using the standard three-element resistor/capacitor test circuit ($Z_x = 470\ \Omega$, $Z_i = 2.2\ \text{k}\Omega$) as detailed in Margo et al. (2013, Eq. 5).

### 3.2. Tetrapolar Needle Probe & Insertion Jig
- **Electrode Material:** Medical-grade 316L stainless steel needles (passivated) or platinum-iridium wire (diameter $\phi = 0.8\ \text{mm}$).
- **Electrode Geometry:** Linear 4-pin in-line array:
  - Outer current-injection electrodes: E1, E4 (spacing $L_{\text{outer}} = 15.0\ \text{mm}$).
  - Inner voltage-sensing electrodes: E2, E3 (spacing $L_{\text{inner}} = 5.0\ \text{mm}$).
- **Mechanical Jig:** A vertical mechanical micrometer stand to guarantee:
  1. Constant penetration depth of exactly **$8.0 \pm 0.2\ \text{mm}$** into the thickest dorsal muscle.
  2. Fixed orientation strictly **perpendicular to the dorsal spine** to standardize muscle fiber anisotropy.
- **Temperature Sensing:** High-precision miniature thermistor (NTC 10k, accuracy $\pm 0.1^\circ\text{C}$) integrated into the probe head, inserted 8 mm alongside electrodes.

---

## 4. Reference Ground-Truth Laboratory Assays

At each sampling time point, following EIS acquisition, destructive assays are performed immediately on the identical fillet:

1. **Total Volatile Basic Nitrogen (TVB-N):**
   - Assayed via semi-micro Kjeldahl steam distillation according to standard ISO / GB 5009.228-2016.
   - 10.0 g of minced muscle homogenized with 100 mL perchloric acid; distilled with magnesium oxide (MgO); titrated with 0.01 mol/L hydrochloric acid.
   - Expressed in $\text{mg N / 100g}$. Tested in triplicate.
2. **Total Viable Count (TVC):**
   - 10.0 g of aseptic muscle homogenized in 90 mL sterile peptone water (0.1%).
   - Serial 10-fold dilutions plated on Plate Count Agar (PCA). Incubated at $30.0^\circ\text{C}$ for 48 hours.
   - Expressed as $\log_{10}\ \text{CFU/g}$.
3. **Muscle pH:**
   - 5.0 g muscle homogenized in 45 mL deionized water; measured via calibrated digital pH meter with glass electrode.

---

## 5. Statistical Validation & Performance Evaluation

### 5.1. Data Partitioning
- **Training & Calibration Set (60%):** Batches 1 & 2 used to calibrate membership boundaries ($a, b, c, d$) and optimize rule weights.
- **Independent Test Set (40%):** Batch 3 held out entirely; tested strictly after model parameters are locked.
- **Cross-Validation:** 5-fold stratified cross-validation on the training set to prevent overfitting.

### 5.2. Evaluation Metrics

#### A. Continuous Freshness Prediction (Fuzzy Score vs. Standardized TVB-N Index):
To evaluate the continuous 0–100% score against the continuous TVB-N degradation trajectory:
- **Mean Absolute Error (MAE):**
  $$\text{MAE} = \frac{1}{N} \sum_{i=1}^{N} |y_i - \hat{y}_i|$$
- **Root Mean Square Error (RMSE):**
  $$\text{RMSE} = \sqrt{\frac{1}{N} \sum_{i=1}^{N} (y_i - \hat{y}_i)^2}$$
- **Coefficient of Determination ($R^2$):**
  $$R^2 = 1 - \frac{\sum (y_i - \hat{y}_i)^2}{\sum (y_i - \bar{y})^2}$$

#### B. Regulatory Spoilage Classification:
Samples categorized into discrete regulatory tiers based on TVB-N:
- **FRESH:** $\text{TVB-N} \le 15.0\ \text{mg/100g}$
- **ACCEPTABLE:** $15.0 < \text{TVB-N} \le 20.0\ \text{mg/100g}$
- **SPOILED:** $\text{TVB-N} > 20.0\ \text{mg/100g}$

Metrics evaluated:
- **Overall Multi-Class Classification Accuracy**
- **Per-Class Precision, Recall, and F1-Score**
- **Confusion Matrix**
- **False Negative Rate for Spoilage:** Crucial food safety metric: proportion of spoiled fish incorrectly predicted as Fresh or Acceptable (Target: $< 1.0\%$).
