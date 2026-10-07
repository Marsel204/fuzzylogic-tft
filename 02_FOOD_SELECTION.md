# 02. Food Selection Analysis for Prototype Development

## 1. Candidate Food Evaluation

To determine the most scientifically defensible food for the initial EIS freshness prototype, six primary candidate food classes were evaluated across ten rigorous criteria:
1. Number of relevant peer-reviewed EIS studies
2. Methodological rigor and quality of literature
3. Strength of biophysical relationship between EIS and tissue freshness
4. Availability and codification of measurable freshness ground truth
5. Standardized storage protocols in literature
6. Sensitivity to and documentation of temperature conditions
7. Availability of numerical impedance data
8. Availability of frequency-dependent spectral data
9. Practical suitability for a benchtop laboratory prototype
10. Scientific suitability for mapping to a continuous 0–100% freshness score

---

### Candidate 1: Freshwater Fish (Crucian Carp, *Carassius auratus* / Tilapia, *Oreochromis niloticus*)
- **EIS Literature Depth:** Extensive and highly focused. Dozens of dedicated peer-reviewed papers in high-impact journals (*Journal of Food Engineering*, *Journal of Food Quality*, *Sensors*, *Journal of Food Process Engineering*).
- **Biophysical Relationship:** Exceptionally direct. Freshwater fish muscle possesses high water content (~78–82%) and delicate cellular structures with thin perimysial connective tissue. Post-mortem autolytic cellular lysis and bacterial proliferation occur rapidly, producing an unambiguous, monotonic collapse of $\beta$-dispersion impedance within 6–8 days at 4°C.
- **Freshness Ground Truth:** Codified by national and international legal standards (e.g., Chinese Standard GB 2733-2015):
  - Total Volatile Basic Nitrogen (TVB-N): Fresh $\le 15$ mg N/100g; Secondary Fresh $15 - 20$ mg/100g; Spoiled $> 20$ mg N/100g.
  - Total Viable Count (TVC): Fresh $< 5.0\ \log_{10}$ CFU/g; Threshold of spoilage $7.0\ \log_{10}$ CFU/g.
- **Storage Protocols & Temperature:** Standardized refrigerated storage at 4°C (or ice storage at 0°C). Temperature coefficients well characterized (~$-1.56\%/^\circ\text{C}$).
- **Spectral Data:** Broad sweeps (100 Hz to 1 MHz) with specific discrete multi-frequency data (1 kHz, 4 kHz, 10 kHz, 100 kHz) and equivalent circuit parameter extractions ($R_e, R_i, C_m, \alpha$) readily available.
- **Laboratory Prototype Suitability:** Excellent. Live or freshly sacrificed whole fish/fillets are readily accessible year-round at low cost; rapid 7-day spoilage cycle allows rapid experimental turnarounds; white dorsal muscle provides uniform, isotropic tissue slabs.
- **Fuzzy Suitability (0–100%):** Outstanding. High dynamic range in impedance ratio ($2.5 \to 1.05$) and phase ($-18^\circ \to -2^\circ$) provides clear linear/sigmoidal mapping to a 0–100% score.

---

### Candidate 2: Marine Fish (Gilthead Sea Bream, *Sparus aurata* / European Sea Bass / Salmon)
- **EIS Literature Depth:** Very high. Pioneering studies from Universitat Politècnica de València (Pérez-Esteve et al. 2014, Fuentes et al. 2013) and Mediterranean seafood research (*Food Control*, *IFSET*).
- **Biophysical Relationship:** Strong, but confounded by high, variable lipid content (e.g., farmed salmon and sea bream can vary between 5% and 18% intramuscular fat depending on diet and season). High lipid content introduces dielectric variability that alters baseline impedance independently of freshness.
- **Freshness Ground Truth:** TVB-N (EU Directive 95/149/EC: limit 25–35 mg N/100g depending on species); Sensory QIM; K-value.
- **Laboratory Suitability:** Good, but higher procurement cost and seasonal variations in fat content reduce repeatability compared to lean freshwater species.
- **Fuzzy Suitability:** Strong, but requires fat-content compensation terms.

---

### Candidate 3: Poultry (Broiler Chicken Breast, *Pectoralis major*)
- **EIS Literature Depth:** Moderate to High. Recent high-quality studies on handheld impedance meters (Shinji et al. 2026) and frozen-thawed discrimination (Wang et al. 2016).
- **Biophysical Relationship:** Moderate to Good. Impedance ratio ($2\ \text{kHz} / 100\ \text{kHz}$) drops from ~2.45 to ~1.15 over 10 days of chilled storage.
- **Freshness Ground Truth:** K-value (ATP breakdown), TVC, pH.
- **Key Limitations:** 
  1. Spoilage in intact poultry breast is predominantly surface microbial proliferation (*Pseudomonas* producing surface slime), whereas deep tissue autolysis occurs at a different kinetic rate. Needle insertion measures internal muscle, potentially missing early surface spoilage.
  2. Susceptibility to post-mortem "woody breast" or "white striping" myopathies alters baseline membrane capacitance.

---

### Candidate 4: Red Meat — Beef (*Longissimus dorsi* / *Semimembranosus*)
- **EIS Literature Depth:** Very high, but primarily focused on **aging/tenderization** rather than microbial spoilage (Damez et al. 2008; Byrne et al. 2001).
- **Biophysical Relationship:** Highly confounded. In beef, industrial practice relies on deliberate cold aging for 14 to 28+ days to allow enzymatic tenderization. Differentiating beneficial enzymatic aging from undesirable microbial spoilage via EIS is complex and non-linear.
- **Structural Anisotropy Problem:** Extreme. Damez et al. (2008) demonstrated that raw beef impedance varies by **200% to 300%** depending on whether needles are aligned parallel or perpendicular to muscle fibers ($Z_\perp / Z_\parallel \approx 2.5$). Any slight tilt or misalignment in a handheld probe creates errors larger than 5 days of aging!
- **Laboratory Suitability:** Poor for initial prototype due to multi-week testing cycles and high cost.

---

### Candidate 5: Red Meat — Pork (*Longissimus dorsi*)
- **EIS Literature Depth:** High foundational literature (Pliquett et al. 2003).
- **Biophysical Relationship:** Severely confounded by post-mortem glycolysis anomalies. Pork frequently suffers from Pale, Soft, Exudative (PSE) or Dark, Firm, Dry (DFD) conditions immediately following slaughter due to acute pre-slaughter stress. PSE meat exhibits massive early cell membrane damage and drip loss, mimicking spoiled meat within 24 hours post-mortem.
- **Fuzzy Suitability:** Poor for early prototypes due to high false-positive spoilage rates from stress defects.

---

### Candidate 6: Fruits (Apples, *Malus domestica* / Bananas / Citrus)
- **EIS Literature Depth:** Moderate to High (Harker & Forbes 1997; Rehman et al. 2018).
- **Biophysical Relationship:** Fundamentally different mechanism. Fruit "freshness" is developmental ripening and physiological senescence (pectin solubilization, starch conversion, cell wall loosening), not microbial spoilage.
- **Freshness Ground Truth:** Firmness (Penetrometer in N), Soluble Solids Content (°Brix), Titratable Acidity. No chemical TVB-N or bacterial count applies.
- **Testing Duration:** Senescence takes 30–90 days in refrigerated storage.
- **Fuzzy Suitability:** Unsuitable for a generic "freshness/spoilage" 0–100% score; requires cultivar-specific ripening indices.

---

## 2. Comparative Food Ranking Table

| Candidate Food | EIS Literature Volume & Quality | Numerical & Spectral Data Availability | Measurable Ground Truth Codification | Structural Homogeneity & Isotropy | Spoilage Dynamics & Lab Cycle | Overall Fuzzy Suitability | Prototype Recommendation |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Freshwater Fish** *(Crucian Carp / Tilapia)* | **High** (Peer-reviewed JFE, JFQ, Sensors) | **High** (Discrete 1k–100k Hz, Cole-Cole $R_e, R_i, C_m$) | **Very High** (Codified TVB-N $\le 15$ to $>20$ mg/100g, TVC) | **High** (Low connective tissue, uniform white myotomes) | **Optimal** (0–8 days at 4°C, rapid cycles) | **Very High** (Monotonic 2.5 $\to$ 1.05 ratio drop) | **RANK 1: RECOMMENDED FOR FIRST PROTOTYPE** |
| **Marine Fish** *(Sea Bream / Sea Bass)* | **High** (Food Control, IFSET) | **High** (1 Hz – 100 kHz spectra available) | **High** (TVB-N EU limits, QIM sensory) | **Moderate** (Inter-batch fat variations 5–18%) | **Good** (0–12 days at 4°C) | **High** (Requires fat normalization) | **RANK 2: Secondary Candidate** |
| **Broiler Chicken Breast** | **Moderate-High** (ASJ 2026, Meat Sci) | **Moderate** (2 kHz & 100 kHz dual ratios) | **High** (K-value, TVC, pH) | **Moderate** (Surface vs core spoilage decoupling) | **Good** (0–10 days at 4°C) | **Moderate-High** (Well-correlated dual ratio) | **RANK 3: Viable Alternative** |
| **Pork** *(Longissimus dorsi)* | **High** (Foundational Meat Sci) | **Moderate** ($P_y$-parameter data available) | **Moderate** (Drip loss, pH, TVC) | **Moderate** (PSE/DFD defects alter baseline) | **Moderate** (Early post-slaughter glycolysis confounds) | **Low-Moderate** (High false-positive risk) | **RANK 4: Not Recommended for Prototype** |
| **Beef** *(Striploin / Semimembranosus)* | **High** (JFE, Agriculture) | **High** (Spectra & anisotropy data) | **Moderate** (Aging tenderness vs TVC spoilage) | **Very Low** (200–300% fiber anisotropy variation) | **Poor** (14–28 days aging cycle) | **Low** (Directional angle swamps freshness signal) | **RANK 5: Not Recommended for Prototype** |
| **Fruit** *(Apple / Banana)* | **Moderate** (JFE, Sensors) | **Moderate** (Cole-Cole plots, high resistance) | **Moderate** (Firmness N, Brix; no TVB-N) | **Moderate** (Cultivar, peel, core variations) | **Poor** (30–60 days storage cycle) | **Low** (Senescence is non-monotonic ripening) | **RANK 6: Unsuitable for Microbial Model** |

---

## 3. Definitive Recommendation and Technical Justification

### Recommendation: **Freshwater Fish Muscle (Crucian Carp, *Carassius auratus*, or Tilapia, *Oreochromis niloticus*)**

### Technical Justification:
1. **Unambiguous Ground Truth Alignment [LITERATURE]:** Freshwater fish has legally enforceable, scientifically validated chemical thresholds:
   - Fresh (Score 70–100%): $\text{TVB-N} \le 15.0\ \text{mg N / 100g}$, $\text{TVC} < 5.0\ \log_{10}\ \text{CFU/g}$.
   - Secondary Fresh / Limit of Acceptability (Score 35–70%): $\text{TVB-N} = 15.0 - 20.0\ \text{mg N / 100g}$.
   - Spoiled (Score 0–35%): $\text{TVB-N} > 20.0\ \text{mg N / 100g}$, $\text{TVC} > 7.0\ \log_{10}\ \text{CFU/g}$.
2. **Predictable Spoilage Kinetics [LITERATURE]:** Unlike beef, which requires weeks of aging, fish muscle spoils predictably within **6 to 8 days under standard 4°C refrigeration**. This permits complete experimental validation cycles in a single working week.
3. **Low Structural Anisotropy [LITERATURE]:** Fish white muscle consists of short, segmented muscle fibers (myotomes) bound by delicate myocommata. It possesses significantly lower collagen and connective tissue content (~3%) compared to mammalian muscle (~10–15%), resulting in much lower directional impedance variation and higher probe repositioning repeatability.
4. **Rich Literature Foundation [LITERATURE]:** The studies by Sun et al. (2017, 2018, 2021) and Liang et al. (2020) provide comprehensive multi-frequency impedance and equivalent circuit parameters ($R_e, R_i, C_m$) specifically for carp and tilapia under controlled storage conditions, directly enabling the design of our fuzzy rule base.

---

## 4. Extension: Indonesian Free Nutritious Meals Program (MBG) Multi-Ingredient Scope

To address institutional school lunch safety in Indonesia under the **Program Makan Bergizi Gratis (MBG)**, the model expands beyond freshwater fish to encompass the 5 primary raw perishables delivered to central catering kitchens (*Satuan Pelayanan MBG*):

1. **Daging Ayam Broiler** (*Broiler Chicken Breast / Fillet*): Primary protein across >70% of MBG menus. Standardized by **SNI 3924:2009**. Evaluated via 4-electrode penetrating probe at 2 kHz / 100 kHz (`[LITERATURE: P09, P12]`).
2. **Daging Sapi** (*Bovine Beef Muscle*): High-nutrition animal protein for stewing/curry (rendang, semur). Standardized by **SNI 3932:2008**. Evaluated at 1 kHz / 100 kHz (`[LITERATURE: P10, P11]`) with mandatory transverse needle insertion to control for muscle fiber anisotropy.
3. **Ikan Air Tawar (Ikan Nila & Ikan Lele)** (*Tilapia & Catfish*): Core regional aquaculture protein in Java and Sumatra. Standardized by **SNI 2729:2021**. Evaluated at 4 kHz / 100 kHz (`[LITERATURE: P04, P13]`).
4. **Ikan Laut Pelagis (Ikan Tongkol / Cakalang)** (*Mackerel / Skipjack Tuna*): Core marine protein in coastal regions. Standardized by **SNI 2729:2021 Annex B**. Includes early warning penalty for Scombroid histamine accumulation under temperature abuse (>10°C).
5. **Susu Sapi Segar** (*Fresh Cow's Milk*): Mandatory weekly nutritional beverage. Standardized by **SNI 3141.1:2011**. Evaluated via the **same AD5933 four-electrode circuit** operating in liquid immersion mode (`[MARGO: P01, P16, P17]`), measuring bulk electrolytic conductivity $\sigma$ at 10 kHz coupled with direct pH sensing to detect bacterial lactic acidification.
6. **Tempe Kedelai** (*Fermented Soybean Cake*): National staple plant-based protein across >90% of Indonesian school lunch cycles. Standardized by **SNI 3142:2018**. Evaluated via 4-electrode penetrating probe at 4 kHz / 100 kHz coupled with headspace $\text{NH}_3$ gas sensing. The dual-sensor configuration tracks intact fungal mycelial capacitance (*Rhizopus oligosporus*) and discriminates peak fermentation from over-fermented/proteolytic spoilage (*tempe busuk*, $\text{NH}_3 > 20\ \text{ppm}$).
7. **Tahu Putih** (*Fresh Soybean Curd / Tofu Block*): Daily plant-based protein with high water activity ($a_w \approx 0.98$, moisture 80–85%). Standardized by **SNI 01-3144-1998**. Evaluated via 4-electrode probe at 2 kHz / 100 kHz (`[LITERATURE: Li et al. 2011]`) and headspace $\text{NH}_3$. Tracks protein gel matrix degradation, syneresis, and bacterial liquefaction/slime formation.

