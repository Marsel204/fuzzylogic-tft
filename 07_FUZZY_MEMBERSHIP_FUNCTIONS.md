# 07. Fuzzy Membership Function Specifications

> [!IMPORTANT]
> **BOUNDARY CALIBRATION DISCLOSURE:**
> In accordance with the mandate of this study, all numerical membership parameters specified below are derived from normalized literature trends (dual-frequency ratios and tetrapolar phase measurements in fish).
> However, **exact numerical crossover points for a custom 4-electrode sensor head REQUIRE EXPERIMENTAL CALIBRATION**. 
> They must NOT be treated as universal physical constants.

---

## 1. Input 1: Impedance Ratio ($R_{\text{EIS}} = |Z_{\text{LF}}| / |Z_{\text{HF}}|$)

- **Universe of Discourse:** $[0.8, 3.2]$ (Dimensionless)
- **Frequencies:** $f_{\text{LF}} = 4.0\ \text{kHz}$, $f_{\text{HF}} = 100.0\ \text{kHz}$
- **Linguistic Terms:** `LOW`, `MEDIUM`, `HIGH`
- **Source:** `[P08, P09, P13]` (Pliquett 2003, Shinji 2026, Liang 2020)

### Mathematical Definitions:

#### Term: `LOW`
- **Shape:** Left-Shoulder Trapezoidal: $\text{trapmf}(x; 0.8, 0.8, 1.15, 1.40)$
- **Formula:**
  $$\mu_{\text{LOW}}(x) = \begin{cases} 
  1.0, & x \le 1.15 \\ 
  \frac{1.40 - x}{1.40 - 1.15}, & 1.15 < x < 1.40 \\ 
  0.0, & x \ge 1.40 
  \end{cases}$$
- **Biophysical Meaning:** Represents ruptured, lysed cell membranes where low-frequency impedance has collapsed to near high-frequency levels ($R_{\text{EIS}} \to 1.0$). Characteristic of advanced autolysis and bacterial proteolysis (TVB-N $> 20$ mg/100g).
- **Confidence:** **Medium-High** (Well-established that ratio approaches 1.0 in fully damaged tissue).
- **Calibration Status:** Inflection slope ($1.15 - 1.40$) **REQUIRES EXPERIMENTAL CALIBRATION** on target probe.

#### Term: `MEDIUM`
- **Shape:** Symmetrical Triangular: $\text{trimf}(x; 1.25, 1.65, 2.10)$
- **Formula:**
  $$\mu_{\text{MEDIUM}}(x) = \begin{cases} 
  0.0, & x \le 1.25 \text{ or } x \ge 2.10 \\ 
  \frac{x - 1.25}{1.65 - 1.25}, & 1.25 < x < 1.65 \\ 
  \frac{2.10 - x}{2.10 - 1.65}, & 1.65 \le x < 2.10 
  \end{cases}$$
- **Biophysical Meaning:** Represents progressive cell membrane degradation, increasing electrolyte permeability, and secondary freshness (TVB-N $15 - 20$ mg/100g).
- **Confidence:** **Medium** (Reflects intermediate transition observed in Days 3–5).
- **Calibration Status:** Peak parameter ($1.65$) **REQUIRES EXPERIMENTAL CALIBRATION**.

#### Term: `HIGH`
- **Shape:** Right-Shoulder Trapezoidal: $\text{trapmf}(x; 1.90, 2.30, 3.20, 3.20)$
- **Formula:**
  $$\mu_{\text{HIGH}}(x) = \begin{cases} 
  0.0, & x \le 1.90 \\ 
  \frac{x - 1.90}{2.30 - 1.90}, & 1.90 < x < 2.30 \\ 
  1.0, & x \ge 2.30 
  \end{cases}$$
- **Biophysical Meaning:** Represents intact, viable cellular membranes providing strong capacitive barriers to low-frequency current. Corresponds to fresh fish (TVB-N $< 15$ mg/100g, TVC $< 5.0\ \log_{10}$ CFU/g).
- **Confidence:** **High** (Fresh fish muscle consistently displays ratios $> 2.0$).
- **Calibration Status:** Threshold boundary ($2.30$) **REQUIRES EXPERIMENTAL CALIBRATION**.

---

## 2. Input 2: Tetrapolar Phase Angle ($\theta$)

- **Universe of Discourse:** $[-25.0^\circ, 0.0^\circ]$ (Degrees)
- **Frequency:** $f_{\text{char}} \approx 10.0\ \text{kHz}$ (Tetrapolar bulk measurement)
- **Linguistic Terms:** `FLAT`, `MODERATE`, `PROMINENT`
- **Source:** `[P01, P04, P07, P13]` (Margo 2013, Sun 2018, Fuentes 2013, Liang 2020)

### Mathematical Definitions:

#### Term: `FLAT`
- **Shape:** Right-Shoulder Trapezoidal: $\text{trapmf}(x; -5.0, -3.0, 0.0, 0.0)$
- **Formula:**
  $$\mu_{\text{FLAT}}(\theta) = \begin{cases} 
  0.0, & \theta \le -5.0 \\ 
  \frac{\theta - (-5.0)}{-3.0 - (-5.0)}, & -5.0 < \theta < -3.0 \\ 
  1.0, & -3.0 \le \theta \le 0.0 
  \end{cases}$$
- **Biophysical Meaning:** Loss of membrane capacitance; tissue behaves as a purely resistive electrolyte solution ($\theta \to 0^\circ$). Signifies advanced cellular breakdown.
- **Confidence:** **High** (Confirmed by tetrapolar biological impedance theory).
- **Calibration Status:** Boundary ($-3.0^\circ$) **REQUIRES EXPERIMENTAL CALIBRATION**.

#### Term: `MODERATE`
- **Shape:** Triangular: $\text{trimf}(x; -13.0, -8.5, -4.0)$
- **Formula:**
  $$\mu_{\text{MODERATE}}(\theta) = \begin{cases} 
  0.0, & \theta \le -13.0 \text{ or } \theta \ge -4.0 \\ 
  \frac{\theta - (-13.0)}{-8.5 - (-13.0)}, & -13.0 < \theta < -8.5 \\ 
  \frac{-4.0 - \theta}{-4.0 - (-8.5)}, & -8.5 \le \theta < -4.0 
  \end{cases}$$
- **Biophysical Meaning:** Intermediate membrane reactance; typical of partially autolyzed tissue.
- **Confidence:** **Medium**.
- **Calibration Status:** Peak parameter ($-8.5^\circ$) **REQUIRES EXPERIMENTAL CALIBRATION**.

#### Term: `PROMINENT`
- **Shape:** Left-Shoulder Trapezoidal: $\text{trapmf}(x; -25.0, -25.0, -15.0, -9.0)$
- **Formula:**
  $$\mu_{\text{PROMINENT}}(\theta) = \begin{cases} 
  1.0, & \theta \le -15.0 \\ 
  \frac{-9.0 - \theta}{-9.0 - (-15.0)}, & -15.0 < \theta < -9.0 \\ 
  0.0, & \theta \ge -9.0 
  \end{cases}$$
- **Biophysical Meaning:** High membrane capacitive reactance in intact white muscle tissue ($\theta \approx -12^\circ$ to $-20^\circ$).
- **Confidence:** **Medium-High**.
- **Calibration Status:** Transition boundary ($-9.0^\circ$) **REQUIRES EXPERIMENTAL CALIBRATION**.

---

## 3. Input 3: Temperature ($T$)

- **Universe of Discourse:** $[0.0, 25.0]\ ^\circ\text{C}$
- **Linguistic Terms:** `COLD`, `MILD`, `WARM`
- **Source:** `[P02, P10, P11]` (Zhao 2017, Damez 2008, Liu 2022)

### Mathematical Definitions:

#### Term: `COLD`
- **Shape:** Left-Shoulder Trapezoidal: $\text{trapmf}(T; 0.0, 0.0, 4.0, 7.0)$
- **Biophysical Meaning:** Proper commercial refrigeration and melting ice storage (0–4°C). Bacterial proliferation is minimized.
- **Confidence:** **High** (Codified cold-chain standard).

#### Term: `MILD`
- **Shape:** Triangular: $\text{trimf}(T; 5.0, 10.0, 16.0)$
- **Biophysical Meaning:** Mild temperature abuse (chill failure, room transfer). Spoilage rates accelerate significantly.
- **Confidence:** **High**.

#### Term: `WARM`
- **Shape:** Right-Shoulder Trapezoidal: $\text{trapmf}(T; 13.0, 18.0, 25.0, 25.0)$
- **Biophysical Meaning:** Ambient room temperature. Psychrotrophic and mesophilic bacteria proliferate exponentially; severe spoilage risk.
- **Confidence:** **High**.

---

## 4. Output: Freshness Score ($0 - 100\%$)

- **Universe of Discourse:** $[0.0, 100.0]\%$
- **Linguistic Terms:** `SPOILED`, `ACCEPTABLE`, `FRESH`
- **Ground Truth Mapping:** Codified TVB-N (GB 2733-2015) and TVC (ICMSF).

### Mathematical Definitions:

#### Term: `SPOILED`
- **Shape:** Left-Shoulder Trapezoidal: $\text{trapmf}(y; 0.0, 0.0, 15.0, 35.0)$
- **Ground Truth:** $\text{TVB-N} > 20.0\ \text{mg N/100g}$, $\text{TVC} \ge 7.0\ \log_{10}\ \text{CFU/g}$.
- **Confidence:** **High** (Strict legal rejection limit).

#### Term: `ACCEPTABLE`
- **Shape:** Symmetrical Triangular: $\text{trimf}(y; 25.0, 50.0, 75.0)$
- **Ground Truth:** $\text{TVB-N} = 15.0 - 20.0\ \text{mg N/100g}$, $\text{TVC} = 5.0 - 6.8\ \log_{10}\ \text{CFU/g}$.
- **Confidence:** **High** (Secondary freshness standard).

#### Term: `FRESH`
- **Shape:** Right-Shoulder Trapezoidal: $\text{trapmf}(y; 65.0, 85.0, 100.0, 100.0)$
- **Ground Truth:** $\text{TVB-N} \le 15.0\ \text{mg N/100g}$, $\text{TVC} < 5.0\ \log_{10}\ \text{CFU/g}$.
- **Confidence:** **High** (Grade A fresh standard).
