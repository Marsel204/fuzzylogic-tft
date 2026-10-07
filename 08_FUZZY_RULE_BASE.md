# 08. Fuzzy Rule Base and Scientific Justification

## 1. Rule Base Design Principles

To maintain strict scientific discipline, the rule base avoids exhaustive combinatorial explosion (which would create ungrounded artificial rules). Instead, an **interpretable, compact set of 11 rules** is formulated.

Every rule is strictly classified into one of three epistemic categories:
1. `[LITERATURE-SUPPORTED]`: Directly supported by published experimental findings linking impedance parameters, microbial growth, or chemical freshness.
2. `[PHYSICALLY-MOTIVATED]`: Derived from established laws of thermodynamics, cellular biophysics, or microbiological kinetics (e.g., Arrhenius temperature dependence).
3. `[ENGINEERING-ASSUMPTION]`: Practical operational heuristics introduced to ensure smooth mathematical transitions in regions where specific literature is sparse.

---

## 2. Complete Rule Specifications

### Rule 1: Intact Tissue under Cold Storage
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is HIGH AND } \theta \text{ is PROMINENT AND Temperature is COLD THEN Freshness is FRESH}$$
- **Classification:** `[LITERATURE-SUPPORTED]`
- **Supporting Literature:** `[P04, P08, P09, P13]` (Sun 2018, Pliquett 2003, Shinji 2026, Liang 2020)
- **Scientific Justification:** High dual-frequency ratio ($R_{\text{EIS}} > 2.2$) proves intact cell membranes. Prominent negative phase ($\theta < -12^\circ$) confirms high membrane capacitance. Cold temperature (0–4°C) confirms active cold-chain preservation. Corresponds to Day 0–2 post-harvest fish with TVB-N $< 12$ mg/100g.

---

### Rule 2: Intact Tissue with Mild Thermal Abuse
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is HIGH AND } \theta \text{ is PROMINENT AND Temperature is MILD THEN Freshness is ACCEPTABLE}$$
- **Classification:** `[PHYSICALLY-MOTIVATED]`
- **Scientific Justification:** Although physical cellular membranes have not yet completely autolyzed, mild temperature exposure (5–15°C) dramatically accelerates psychrotrophic bacterial growth. Freshness must be penalized from premium to acceptable because remaining shelf-life is drastically curtailed.

---

### Rule 3: High Ratio with Moderate Phase under Cold Storage
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is HIGH AND } \theta \text{ is MODERATE AND Temperature is COLD THEN Freshness is FRESH}$$
- **Classification:** `[ENGINEERING-ASSUMPTION]`
- **Scientific Justification:** Provides robustness against minor probe contact variations or slight muscle fiber orientation shifts that slightly attenuate phase angle while bulk dual-frequency resistance ratio remains strongly intact.

---

### Rule 4: Intermediate Degradation under Cold Storage
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is MEDIUM AND } \theta \text{ is MODERATE AND Temperature is COLD THEN Freshness is ACCEPTABLE}$$
- **Classification:** `[LITERATURE-SUPPORTED]`
- **Supporting Literature:** `[P04, P05, P06, P13]` (Sun 2018, Sun 2021, Pérez-Esteve 2014, Liang 2020)
- **Scientific Justification:** Corresponds to Days 4–5 under 4°C refrigeration. Partial membrane permeability leakage has occurred; TVB-N is approaching the regulatory secondary freshness threshold ($15 - 20$ mg N/100g). Fish is safe for immediate cooking but cannot be classified as Grade A fresh.

---

### Rule 5: Intermediate Ratio with Prominent Phase under Cold Storage
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is MEDIUM AND } \theta \text{ is PROMINENT AND Temperature is COLD THEN Freshness is FRESH}$$
- **Classification:** `[ENGINEERING-ASSUMPTION]`
- **Scientific Justification:** When tissue capacitance remains strong, a moderate impedance ratio may reflect higher baseline ionic conductivity (e.g. natural mineral variation) rather than true cellular destruction.

---

### Rule 6: Intermediate Ratio with Collapsed Phase Angle
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is MEDIUM AND } \theta \text{ is FLAT THEN Freshness is SPOILED}$$
- **Classification:** `[PHYSICALLY-MOTIVATED]`
- **Scientific Justification:** Phase collapse ($\theta \to 0^\circ$) indicates that membrane capacitance has vanished and cells have lost capacitive charge separation. Even if the resistance ratio appears moderate, the loss of dielectric reactance signals advanced cellular disintegration.

---

### Rule 7: Low Dual-Frequency Ratio
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is LOW THEN Freshness is SPOILED}$$
- **Classification:** `[LITERATURE-SUPPORTED]`
- **Supporting Literature:** `[P03, P04, P08, P09]` (Sun 2017, Sun 2018, Pliquett 2003, Shinji 2026)
- **Scientific Justification:** When $R_{\text{EIS}} \to 1.05$ (low frequency impedance equals high frequency impedance), cell membranes are entirely ruptured. Current encounters zero capacitive impediment. Corresponds universally across all meat and fish studies to spoiled, post-rigor autolyzed tissue.

---

### Rule 8: Low Ratio with Collapsed Phase Angle
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is LOW AND } \theta \text{ is FLAT THEN Freshness is SPOILED}$$
- **Classification:** `[LITERATURE-SUPPORTED]`
- **Supporting Literature:** `[P04, P07, P13]` (Sun 2018, Fuentes 2013, Liang 2020)
- **Scientific Justification:** Concurrence of both primary electrical indicators: zero membrane resistance barrier and zero reactive capacitance. Corresponds to Day 7+ spoiled fish with TVB-N $> 25$ mg/100g and TVC $> 7.0\ \log_{10}$ CFU/g.

---

### Rule 9: Warm Ambient Temperature with Flat Phase
- **Statement:** 
  $$\text{IF } \theta \text{ is FLAT AND Temperature is WARM THEN Freshness is SPOILED}$$
- **Classification:** `[LITERATURE-SUPPORTED]`
- **Supporting Literature:** `[P11, P15]` (Liu 2022, Genç 2023)
- **Scientific Justification:** Elevated temperature combined with complete absence of membrane reactance indicates severe microbial decay and high risk of enteric pathogen growth.

---

### Rule 10: Intermediate Ratio under Warm Ambient Temperature
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is MEDIUM AND Temperature is WARM THEN Freshness is SPOILED}$$
- **Classification:** `[PHYSICALLY-MOTIVATED]`
- **Scientific Justification:** Exposure of partially autolyzed fish muscle to warm ambient temperatures (>15°C) triggers explosive bacterial growth; the product rapidly crosses the TVB-N spoilage limit within hours.

---

### Rule 11: High Ratio under Warm Ambient Temperature
- **Statement:** 
  $$\text{IF } R_{\text{EIS}} \text{ is HIGH AND Temperature is WARM THEN Freshness is ACCEPTABLE}$$
- **Classification:** `[PHYSICALLY-MOTIVATED]`
- **Scientific Justification:** Even if structural cell integrity remains intact, prolonged warm exposure breaks cold-chain safety. The score is capped at ACCEPTABLE (maximum ~50%) to alert the consumer to imminent spoilage.

---

## 3. Epistemic Breakdown of Rule Base

| Epistemic Classification | Number of Rules | Rule IDs | Proportion |
| :--- | :--- | :--- | :--- |
| **`[LITERATURE-SUPPORTED]`** | **5** | Rules 1, 4, 7, 8, 9 | **45.5%** |
| **`[PHYSICALLY-MOTIVATED]`** | **4** | Rules 2, 6, 10, 11 | **36.4%** |
| **`[ENGINEERING-ASSUMPTION]`** | **2** | Rules 3, 5 | **18.1%** |
| **Total** | **11** | All | **100.0%** |
