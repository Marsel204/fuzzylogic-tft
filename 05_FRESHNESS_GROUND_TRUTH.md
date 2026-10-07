# 05. Freshness Ground Truth Analysis for Freshwater Fish

## 1. Overview of Freshness Ground-Truth Metrics

To train, calibrate, and validate any non-destructive sensing model—including an Electrical Impedance Spectroscopy (EIS) fuzzy inference system—a rigorous, quantitative, and legally codified ground truth of "freshness" is mandatory.

In the academic literature, food quality and freshness are quantified via six primary analytical methodologies:
1. Chemical Volatile Indicators (Total Volatile Basic Nitrogen, TVB-N; Trimethylamine, TMA)
2. Microbiological Quantification (Total Viable Count, TVC; Aerobic Plate Count, APC)
3. Biochemical Nucleotide Catabolism (K-value, ATP breakdown index)
4. Physicochemical Measurements (Tissue pH, electrical conductivity)
5. Sensory Evaluation (Quality Index Method, QIM; Torry score)
6. Storage Duration (Elapsed days post-harvest)

---

## 2. Comparative Evaluation of Freshness Ground Truths for Fish

### 2.1. Total Volatile Basic Nitrogen (TVB-N)
- **Biochemical Mechanism:** Following harvest, endogenous proteases and proliferating spoilage bacteria (*Pseudomonas*, *Shewanella*, *Enterobacteriaceae*) degrade fish muscle proteins and amine compounds. This catabolism produces volatile nitrogenous bases, primarily ammonia ($\text{NH}_3$), dimethylamine (DMA), and trimethylamine (TMA).
- **Measurement Method:** Standard Conway microdiffusion method, semi-micro Kjeldahl steam distillation, or acidimetric titration (codified by AOAC and ISO standards).
- **Legal & Regulatory Codification:**
  - **Freshwater Fish (Crucian Carp, Grass Carp, Tilapia):** Chinese National Food Safety Standard **GB 2733-2015**:
    - **Fresh (Grade A):** $\text{TVB-N} \le 15.0\ \text{mg N / 100g}$
    - **Secondary Fresh / Acceptable:** $15.0 < \text{TVB-N} \le 20.0\ \text{mg N / 100g}$
    - **Spoiled / Unfit for Consumption:** $\text{TVB-N} > 20.0\ \text{mg N / 100g}$
  - **Marine Fish (Sea Bream, Cod, Sea Bass):** European Commission Regulation **EC No 2074/2005** / Directive **95/149/EC**:
    - Fresh: $\le 20.0\ \text{mg N / 100g}$
    - Acceptable Limit: $25.0 - 35.0\ \text{mg N / 100g}$ (species-dependent)
    - Spoiled: $> 35.0\ \text{mg N / 100g}$
- **Correlation with EIS:** Exceptionally high in peer-reviewed studies:
  - Sun et al. (2018): $R^2 = 0.86$ between EIS parameters and TVB-N in Crucian carp `[P04]`.
  - Sun et al. (2021): $r = 0.91$ between EIS fusion parameter and TVB-N `[P05]`.
  - Pérez-Esteve et al. (2014): $R^2 = 0.72$ for TVB-N prediction in sea bream `[P06]`.
  - Liang et al. (2020): $R^2 = 0.89$ in tilapia fillets `[P13]`.
- **Verdict:** **PRIMARY REFERENCE CHEMICAL GROUND TRUTH.**

---

### 2.2. Total Viable Count (TVC) / Aerobic Plate Count (APC)
- **Biochemical Mechanism:** Measures the total density of viable mesophilic and psychrotrophic bacterial colony-forming units per gram ($\log_{10}\ \text{CFU/g}$) on fish muscle.
- **Measurement Method:** Standard serial dilution, plate agar incubation (PCA) at 30°C for 48 h or 20°C for 72 h (ISO 4833).
- **Regulatory Standard (ICMSF):**
  - **Fresh:** $< 5.0\ \log_{10}\ \text{CFU/g}$
  - **Acceptable:** $5.0 - 6.0\ \log_{10}\ \text{CFU/g}$
  - **Rejection / Spoilage Threshold:** $7.0\ \log_{10}\ \text{CFU/g}$ (International Commission on Microbiological Specifications for Foods, ICMSF).
- **Correlation with EIS:** Strong ($R^2 = 0.82 - 0.88$). Bacterial growth directly correlates with cellular membrane breakdown and electrolyte accumulation.
- **Verdict:** **PRIMARY REFERENCE MICROBIOLOGICAL GROUND TRUTH.**

---

### 2.3. K-Value (ATP Catabolism Index)
- **Biochemical Mechanism:** Immediately post-mortem, cellular ATP is hydrolyzed sequentially:
  $$\text{ATP} \to \text{ADP} \to \text{AMP} \to \text{IMP} \to \text{HxR (Inosine)} \to \text{Hx (Hypoxanthine)}$$
  The K-value quantifies the percentage of unphosphorylated catabolites:
  $$K = \frac{[\text{HxR}] + [\text{Hx}]}{[\text{ATP}] + [\text{ADP}] + [\text{AMP}] + [\text{IMP}] + [\text{HxR}] + [\text{Hx}]} \times 100\%$$
- **Freshness Tiers:**
  - Sashimi Grade / Very Fresh: $K < 20\%$
  - Good Freshness: $20\% \le K < 40\%$
  - Acceptable: $40\% \le K < 60\%$
  - Spoiled: $K \ge 60\%$
- **Measurement Method:** High-Performance Liquid Chromatography (HPLC) or enzymatic test strips.
- **Correlation with EIS:** Strong in early post-mortem storage ($r = -0.92$ in Shinji et al. 2026 `[P09]`).
- **Verdict:** Excellent for early autolysis, but HPLC requires expensive, specialized chromatography instrumentation.

---

### 2.4. Muscle Tissue pH
- **Biochemical Mechanism:** Post-mortem glycolysis initially converts glycogen into lactic acid, causing muscle pH to drop from ~7.0 to ~6.2–6.4 (rigor mortis). Subsequently, as spoilage bacteria proliferate and produce basic volatile compounds (ammonia, TMA), pH reverses direction and climbs to >6.8–7.2.
- **Limitations:** **Non-monotonic response.** A pH of 6.6 could represent either very early fresh meat (on the downward glycolytic slope) or late-stage spoiled meat (on the upward alkaline slope). Therefore, pH alone cannot serve as an unambiguous single freshness ground truth.

---

### 2.5. Storage Duration (Elapsed Time) vs. True Freshness
> [!CAUTION]
> **CRITICAL SCIENTIFIC PRINCIPLE:** Storage time (days post-harvest) MUST NOT be equated directly with freshness!

1. **Temperature Confounding:** Spoilage is an enzymatic and microbial reaction whose kinetic rate obeys the Arrhenius equation. Storage at 4°C results in an 8-day shelf life, storage on melting ice (0°C) extends shelf life to 12–14 days, while mild temperature abuse at 15°C causes complete spoilage within 36–48 hours!
2. **Cold-Chain Interruptions:** A fish stored for 2 days that experienced a 6-hour exposure to 20°C may be far more spoiled than a fish stored for 5 days under unbroken 0°C ice.
3. **Conclusion:** Storage time is merely an **experimental coordinate** under strictly controlled isothermal conditions, NOT a physical freshness attribute. True freshness must always be defined by **TVB-N ($\text{mg/100g}$)** and **TVC ($\log_{10}\ \text{CFU/g}$)**.

---

## 3. Freshness Mapping Scheme for Proposed Fuzzy Output

The 0–100% Freshness Score output of our fuzzy inference system is mapped directly onto codified TVB-N and TVC thresholds for freshwater fish (GB 2733-2015 and ICMSF):

| Fuzzy Output Category | Fuzzy Score Universe | Codified TVB-N (mg N / 100g) | Codified TVC ($\log_{10}\ \text{CFU/g}$) | Biochemical Tissue State | Commercial Quality Action |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FRESH** | **65% – 100%** | $\le 15.0$ | $< 5.0$ | Intact cell membranes; negligible proteolysis; fresh smell. | Premium retail sale; raw/steamed consumption. |
| **ACCEPTABLE** | **35% – 65%** | $15.0 - 20.0$ | $5.0 - 6.8$ | Partial autolytic membrane breakdown; moderate bacterial count. | Secondary fresh; cooked consumption immediately; discard soon. |
| **SPOILED** | **0% – 35%** | $> 20.0$ | $\ge 7.0$ | Severe cellular lysis; high volatile bases; strong off-odor; slime. | **REJECTED**: Unfit for human consumption. Legal condemnation. |
