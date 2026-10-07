# 04. EIS Feature Evidence & Support Classification

This document evaluates the scientific validity, biophysical basis, and cross-system transferability of candidate Electrical Impedance Spectroscopy (EIS) features reported in academic literature for food freshness assessment.

## Evidence Classification Hierarchy
- **Class A:** **Strongly Supported.** Validated across multiple independent peer-reviewed studies with clear biophysical mechanisms, strong statistical correlations ($r > 0.85$ or $R^2 > 0.75$), and mathematical independence from electrode geometric artifacts.
- **Class B:** **Moderately Supported.** Statistically significant correlation reported in multiple studies, but sensitive to environmental variables, hardware bandwidth limits, or computational fitting errors.
- **Class C:** **Weakly Supported.** Correlates within a single highly controlled experiment, but fails cross-system comparison due to extreme sensitivity to electrode geometry, sample size, or tissue anisotropy.
- **Class D:** **Unsupported / Insufficient Evidence.** Theoretical concept with contradictory empirical findings, poor reproducibility, or lacking peer-reviewed evidence in whole-food tissues.

---

## Feature Evaluation Matrix

### 1. Dual-Frequency Impedance Ratio ($R_{\text{EIS}} = |Z(f_{\text{LF}})| / |Z(f_{\text{HF}})|$)
- **Evidence Rating:** **Class A (Strongly Supported)**
- **Target Food:** Fish (Carp, Tilapia, Sea Bream), Poultry Breast
- **Why Useful:** 
  Biophysically grounded in $\beta$-dispersion. At low frequency ($f_{\text{LF}} \approx 1 - 4\ \text{kHz}$), cell membranes act as insulating capacitors, forcing current around cells through extracellular pathways (high resistance). At high frequency ($f_{\text{HF}} \approx 100\ \text{kHz}$), membranes short-circuit, allowing current through both intracellular and extracellular fluids (low resistance).
  Crucially, because measured impedance obeys $|Z| = K_{\text{cell}} / \sigma$, the geometric cell constant $K_{\text{cell}} = L/A$ **cancels out in the ratio**:
  $$R_{\text{EIS}} = \frac{|Z(f_{\text{LF}})|}{|Z(f_{\text{HF}})|} = \frac{\sigma(f_{\text{HF}})}{\sigma(f_{\text{LF}})}$$
  In fresh fish, $R_{\text{EIS}} \approx 2.2 - 2.8$. As cell membranes rupture during spoilage, $R_{\text{EIS}}$ drops monotonically toward 1.0.
- **Literature Support:** `[P02, P08, P09, P12, P13]`
  - P08 (Pliquett et al. 2003): Established dual-frequency ratio as fundamental membrane integrity metric.
  - P09 (Shinji et al. 2026): Handheld 2 kHz / 100 kHz ratio correlated strongly with K-value ($r = -0.92, R^2 = 0.85$).
  - P13 (Liang et al. 2020): Tilapia 5 kHz / 100 kHz ratio correlated with TVB-N ($R^2 = 0.89$).
- **Limitations:** Fails if high frequency exceeds instrument bandwidth or is corrupted by stray cable capacitance.

---

### 2. Pliquett $P_y$-Parameter ($P_y = (|Z_{\text{LF}}| - |Z_{\text{HF}}|) / |Z_{\text{HF}}|$)
- **Evidence Rating:** **Class A (Strongly Supported)**
- **Target Food:** Fish, Meat, Poultry
- **Why Useful:** 
  Direct mathematical variant of the impedance ratio: $P_y = R_{\text{EIS}} - 1.0$. Specifically defined by Pliquett et al. (2003) to quantify the viable cell volume fraction. In intact tissue, $P_y > 1.2$; in fully autolyzed/ruptured tissue, $P_y \to 0.0$. Geometry-independent.
- **Literature Support:** `[P02, P08, P12]`
  - P08 (Meat Science 2003): Demonstrated $P_y$ as a universal metric for cell membrane destruction.
  - P02 (Zhao et al. 2017): Validated across multiple meat and fish freshness studies.
- **Limitations:** Identical biophysical dynamics to $R_{\text{EIS}}$, redundant if $R_{\text{EIS}}$ is already utilized.

---

### 3. Tetrapolar Phase Angle at Characteristic Frequency ($\theta(f_{\text{char}})$ at ~10–20 kHz)
- **Evidence Rating:** **Class A (Strongly Supported in Tetrapolar Systems)**
- **Target Food:** Fish (Sea Bream, Carp, Tilapia), Physiological Solutions
- **Why Useful:** 
  Phase angle $\theta = \arctan(\text{Im}(Z) / \text{Re}(Z))$ represents tissue reactance resulting from intact membrane capacitance ($C_m$).
  In a **four-electrode (tetrapolar) configuration**, electrode-electrolyte polarization impedance is eliminated because the instrumentation amplifier draws negligible current (`[MARGO: P01]`). Thus, $\theta$ directly measures true bulk tissue dielectric properties and is **independent of electrode contact area**.
  In fresh fish, $\theta(10\ \text{kHz}) \approx -12^\circ$ to $-20^\circ$. During spoilage, membrane capacitance collapses and the tissue becomes purely resistive ($\theta \to -2^\circ$ to $0^\circ$).
- **Literature Support:** `[P01, P04, P06, P07, P13]`
  - P01 (Margo et al. 2013): Proved tetrapolar phase is flat and uncorrupted by interface polarization compared to bipolar mode (Fig 11).
  - P07 (Fuentes et al. 2013): Phase at 10 kHz discriminated fresh from membrane-damaged fish with >90% accuracy.
  - P04 (Sun et al. 2018): Peak phase shift tracked Cole-Cole relaxation collapse during storage.
- **Limitations:** Extremely sensitive to electrode configuration. In **bipolar (2-electrode)** systems, this feature is completely masked by electrode polarization artifacts ($\theta \approx -70^\circ$). It is only valid in tetrapolar setups!

---

### 4. Temperature-Normalized Impedance ($Z_{\text{norm}}(T)$)
- **Evidence Rating:** **Class A (Strongly Supported)**
- **Target Food:** All biological tissues (Meat, Fish, Fruit)
- **Why Useful:** 
  Electrolyte conductivity increases with temperature by approximately $+1.56\%/^\circ\text{C}$, causing impedance magnitude to decrease by $-1.56\%/^\circ\text{C}$. Normalizing measured impedance to a reference temperature (e.g., $T_{\text{ref}} = 4^\circ\text{C}$) eliminates thermal false-positives:
  $$Z_{\text{ref}} = Z_{\text{meas}} \cdot [1 + \alpha_{\text{temp}} \cdot (T_{\text{meas}} - T_{\text{ref}})]$$
- **Literature Support:** `[P02, P10, P11]`
  - Systematic temperature coefficient studies report $\alpha \approx 0.014 - 0.018\ /^\circ\text{C}$ in animal tissues.
- **Limitations:** Assumes linear temperature dependence within the 0°C to 20°C refrigeration range.

---

### 5. Cole-Cole Equivalent Circuit Parameters ($R_0, R_\infty, C_m, \alpha, f_c$)
- **Evidence Rating:** **Class B (Moderately Supported)**
- **Target Food:** Fish (Carp, Salmon), Beef, Fruit
- **Why Useful:** 
  Fitting complex impedance spectra to the Cole-Cole bioimpedance equation:
  $$Z(\omega) = R_\infty + \frac{R_0 - R_\infty}{1 + (j\omega \tau)^\alpha}$$
  yields direct biophysical parameters: extracellular resistance $R_0$, intracellular resistance fraction $R_\infty$, distribution parameter $\alpha$, and characteristic relaxation frequency $f_c$.
- **Literature Support:** `[P02, P03, P04, P05, P10, P14]`
  - P04 (Sun et al. 2018): Equivalent circuit parameters mapped directly to 3 spoilage stages.
  - P05 (Sun et al. 2021): Multi-parameter fusion achieved 93.3% stage accuracy.
- **Limitations:** 
  1. Requires a broad multi-frequency sweep (at least 20–50 frequency points) and non-linear complex least-squares (NLLS) optimization, which is computationally expensive on microcontroller edge devices.
  2. $R_0$ and $R_\infty$ are absolute resistance values in Ohms, meaning their numerical values cannot be transferred between different probe geometries without calibration.

---

### 6. Raw Impedance Modulus at a Single Frequency ($|Z(f)|$, e.g., at 1 kHz or 10 kHz)
- **Evidence Rating:** **Class C (Weakly Supported across systems; Class B within a single fixed rig)**
- **Target Food:** Fish, Meat
- **Why Useful:** Simple to acquire; drops monotonically during storage within a strictly identical experimental setup.
- **Literature Support:** `[P03, P04, P06, P13]`
- **Critical Limitations:** 
  **NEVER DIRECTLY COMPARABLE BETWEEN PAPERS.** $|Z| = \frac{L}{\sigma A}$. If electrode spacing $L$ changes from 5 mm to 10 mm, $|Z|$ doubles. If needle insertion depth changes from 5 mm to 10 mm, contact area $A$ doubles, halving $|Z|$. Without an identical probe and mechanical insertion jig, numerical thresholds cannot be generalized.

---

### 7. Real Impedance $\text{Re}(Z)$ and Imaginary Impedance $\text{Im}(Z)$ Separately
- **Evidence Rating:** **Class B (Moderately Supported)**
- **Target Food:** Fish, Beef
- **Why Useful:** Directly outputted by AD5933 DFT registers. $\text{Re}(Z)$ tracks conductive dissipation; $\text{Im}(Z)$ tracks capacitive energy storage.
- **Literature Support:** `[P01, P04, P05, P11]`
- **Limitations:** Both scale with the geometric cell constant $K_{\text{cell}}$. Ratioing or phase extraction ($\arctan(\text{Im}/\text{Re})$) is required to remove geometric dependence.

---

### 8. Spectral Slope ($d|Z|/df$ or $\Delta |Z| / \Delta \log f$)
- **Evidence Rating:** **Class B (Moderately Supported)**
- **Target Food:** Fish, Chicken
- **Why Useful:** Measures the steepness of $\beta$-dispersion drop across frequency. In fresh tissue with high membrane capacitance, the slope is steep; in spoiled tissue with ruptured membranes, the impedance curve flattens across frequency.
- **Literature Support:** `[P02, P04, P09]`
- **Limitations:** Sensitive to measurement noise at frequency extremes.

---

### 9. Area Under the Nyquist Spectrum / Morphological Curvature (PEMC)
- **Evidence Rating:** **Class B (Moderately Supported)**
- **Target Food:** Crucian Carp
- **Why Useful:** Proposed by Sun et al. (2017, 2018) as "Peak of Extremum Morphological Characteristic (PEMC)". Integrating the area under the Nyquist semicircle captures the shrinkage of the impedance arc during spoilage.
- **Literature Support:** `[P03, P04]`
- **Limitations:** Computationally complex; requires full sweep and numerical integration; sensitive to baseline shifts.

---

### 10. High-Frequency Conductance ($G$ at $> 500\ \text{kHz}$)
- **Evidence Rating:** **Class D (Unsupported / Insufficient Evidence for AD5933)**
- **Target Food:** Meat
- **Why Useful:** In theory, reflects total intracellular electrolyte concentration.
- **Limitations:** The AD5933 hardware cannot operate above 100 kHz without complex external frequency converters `[MARGO: P01, AD5933-DATASHEET]`. Insufficient peer-reviewed evidence for reliable food freshness estimation at these frequencies.

---

## 3. Summary & Recommended Feature Set for Fuzzy Model

Based on the evidence classification above, the optimal, scientifically defensible feature set for a prototype fuzzy freshness model is:

| Input Variable | Selected Feature | Evidence Class | Justification |
| :--- | :--- | :--- | :--- |
| **Input 1** | **Impedance Ratio ($R_{\text{EIS}} = \|Z(4\text{kHz})\| / \|Z(100\text{kHz})\|$)** | **Class A** | Dimensionless, geometry-invariant, directly measures cellular membrane integrity. |
| **Input 2** | **Phase Angle ($\theta$ at 10 kHz, Tetrapolar)** | **Class A** | Reflects membrane capacitance without electrode polarization artifacts; geometry-independent. |
| **Input 3** | **Temperature ($T$ in °C)** | **Class A** | Eliminates thermal false positives ($-1.56\%/^\circ\text{C}$ biological drift). |
