# 06. Fuzzy Model Architecture and Design Specification

## 1. System Architecture Overview

The proposed model implements an interpretable, literature-grounded **Fuzzy Inference System (FIS)** to estimate the freshness of freshwater fish prior to the availability of custom EIS hardware.

The design strictly obeys the principle of **parsimony (Occam's razor)**: it employs the absolute minimum number of input variables necessary to capture the underlying biophysical and environmental dynamics without inventing uncalibrated complexity.

```
+-----------------------------------------------------------------------------------------+
|                               FUZZY FRESHNESS ARCHITECTURE                              |
|                                                                                         |
|  [Input 1: Impedance Ratio R_eis]  ---> [Fuzzification: LOW, MEDIUM, HIGH]              |
|   |Z(4kHz)| / |Z(100kHz)|                                         |                     |
|                                                                   v                     |
|  [Input 2: Phase Angle θ] --------> [Fuzzification: FLAT, MODERATE, PROMINENT]          |
|   deg at 10 kHz (Tetrapolar)                                      |                     |
|                                                                   +-> [RULE BASE]       |
|  [Input 3: Temperature T] --------> [Fuzzification: COLD, MILD, WARM]  (11 Rules)       |
|   °C (Thermal Covariate)                                          |       |             |
|                                                                   |       v             |
|                                                                   +-> [Aggregation]     |
|                                                                           |             |
|                                                                           v             |
|  [Freshness Score: 0 - 100%] <----- [Centroid Defuzzification] <----------+             |
|   (SPOILED, ACCEPTABLE, FRESH)                                                          |
+-----------------------------------------------------------------------------------------+
```

---

## 2. Selection & Biophysical Justification of Fuzzy Inputs

### Input 1: Dual-Frequency Impedance Ratio ($R_{\text{EIS}} = |Z_{\text{LF}}| / |Z_{\text{HF}}|$)
- **Universe of Discourse:** $[0.8, 3.2]$ (Dimensionless)
- **Frequencies:** $f_{\text{LF}} = 4.0\ \text{kHz}$, $f_{\text{HF}} = 100.0\ \text{kHz}$
- **Classification:** `[LITERATURE: P02, P08, P09, P13]`
- **Biophysical Justification:**
  In biological tissue, the cell membrane behaves as a frequency-dependent dielectric barrier.
  - At 4 kHz, current is restricted to extracellular pathways due to high membrane capacitive reactance; $|Z_{\text{LF}}|$ is high.
  - At 100 kHz, membranes are shunted; current traverses both intracellular and extracellular fluids; $|Z_{\text{HF}}|$ is low.
  - As cell membranes rupture during autolysis and bacterial proteolysis, $|Z_{\text{LF}}|$ drops by 50–70% while $|Z_{\text{HF}}|$ changes only slightly.
  - **Why Ratio Instead of Raw Ohms:** Because measured impedance scales with the geometric cell constant ($|Z| = K_{\text{cell}} / \sigma$), taking the ratio **cancels out the unknown electrode geometry $K_{\text{cell}}$**:
    $$R_{\text{EIS}} = \frac{|Z(f_{\text{LF}})|}{|Z(f_{\text{HF}})|} = \frac{\sigma(f_{\text{HF}})}{\sigma(f_{\text{LF}})}$$
  - In fresh fish, $R_{\text{EIS}} \ge 2.2$. In spoiled fish with lysed membranes, $R_{\text{EIS}} \to 1.05$.

### Input 2: Tetrapolar Phase Angle at Characteristic Frequency ($\theta$)
- **Universe of Discourse:** $[-25.0^\circ, 0.0^\circ]$
- **Frequency:** $f_{\text{char}} \approx 10.0\ \text{kHz}$
- **Classification:** `[MARGO: P01; LITERATURE: P04, P07]`
- **Biophysical Justification:**
  - In a tetrapolar configuration (`[MARGO]`), voltage-sensing electrodes draw zero current, eliminating the massive negative phase artifact of the electrode-electrolyte interface ($\theta \approx -75^\circ$) observed in bipolar setups.
  - Consequently, measured phase angle reflects **true tissue dielectric reactance** $(\omega C_m)$.
  - In fresh tissue, intact membrane capacitance creates a distinct negative phase shift: $\theta(10\ \text{kHz}) \approx -12^\circ$ to $-20^\circ$.
  - In spoiled tissue, loss of membrane capacitance collapses the reactive phase shift toward $0^\circ$ (purely resistive fluid conductor).

### Input 3: Sample / Storage Temperature ($T$)
- **Universe of Discourse:** $[0.0, 25.0]\ ^\circ\text{C}$
- **Classification:** `[LITERATURE: P02, P10, P11]`
- **Biophysical Justification:**
  1. **Direct Electrical Drift:** Biological tissues exhibit a temperature coefficient of approximately **$-1.56\%/^\circ\text{C}$**. Without temperature awareness, warm fresh tissue could be misclassified as deteriorated tissue.
  2. **Kinetic Spoilage Acceleration:** Storage temperature dictates the exponential growth rate of spoilage bacteria (Arrhenius kinetics). Mild temperature abuse (e.g. 15°C) invalidates cold-chain stability.

---

## 3. Evaluation of Fuzzy Inference Architectures

| Architecture | Principle of Operation | Strengths for this Project | Weaknesses / Limitations | Prototype Selection |
| :--- | :--- | :--- | :--- | :--- |
| **Mamdani** | Antecedents and consequents are both represented as linguistic fuzzy sets. Rule outputs are clipped/scaled and aggregated into a combined fuzzy region; defuzzified via Centroid (COG). | **1. High interpretability:** Consequents ("FRESH", "SPOILED") map intuitively to human/commercial categories.<br>**2. Ideal for sparse data:** Does not require thousands of training points to fit regression coefficients.<br>**3. Direct literature translation:** Expert food science rules translate directly into linguistic IF-THEN logic. | Higher computational overhead due to numerical integration during defuzzification. | **SELECTED FOR INITIAL PROTOTYPE** |
| **Takagi-Sugeno (TS)** | Antecedents are fuzzy sets; consequents are crisp functions of the inputs (e.g., constants $z = c$ or linear functions $z = a x_1 + b x_2 + c$). Defuzzified via weighted average. | **1. Highly computationally efficient:** Eliminates integration overhead on microcontrollers.<br>**2. Native compatibility with ANFIS:** Can be tuned automatically via gradient descent when experimental data become available. | Consequents lack intuitive linguistic membership curves; linear output polynomials require empirical training data that do not yet exist. | **Design includes Sugeno comparison for future ANFIS migration.** |
| **Tsukamoto** | Consequents are monotonic membership functions; each rule's output is calculated as an exact crisp inverse value. | Deterministic reasoning with simple defuzzification. | Monotonicity restriction severely limits modeling of complex non-monotonic biological transitions (e.g., intermediate secondary freshness). | **REJECTED** |

---

## 4. Defuzzification Formulation: Centroid (Center of Gravity)

For the Mamdani system, the **Centroid (Center of Gravity, COG)** method is selected:

$$\text{Freshness Score} = \frac{\int_{0}^{100} y \cdot \mu_{\text{agg}}(y) \, dy}{\int_{0}^{100} \mu_{\text{agg}}(y) \, dy}$$

In discrete implementation over $N = 200$ evaluation points:
$$\text{Freshness Score} = \frac{\sum_{i=1}^{N} y_i \cdot \mu_{\text{agg}}(y_i)}{\sum_{i=1}^{N} \mu_{\text{agg}}(y_i)}$$

### Justification for Centroid over Mean of Maxima (MOM):
- **Smooth Continuity:** As physical impedance and temperature fluctuate smoothly, Centroid produces a smooth, continuous output curve, avoiding the step-function discontinuities common in MOM or Bisector methods.
- **Sensitivity:** Retains proportional influence from overlapping active rules, correctly reflecting partial freshness degradation.
