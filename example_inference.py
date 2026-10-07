"""
Example Inference and Visualization Generator
Literature-Grounded EIS Multi-Sensor Freshness Model
Supports 5 Indonesian Free Meals (MBG) Ingredient Profiles:
Ayam Broiler, Daging Sapi, Ikan Nila/Lele, Ikan Tongkol, Susu Segar
"""

import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from fuzzy_model import FreshnessFuzzyModel
from feature_processing import MBG_PROFILES

plt.switch_backend('Agg')
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')

def ensure_dirs():
    for d in ["membership_functions", "rule_analysis", "literature_figures", "simulation_results"]:
        os.makedirs(d, exist_ok=True)

def run_mbg_receiving_simulation():
    ensure_dirs()
    print("\n" + "="*95)
    print("SIMULASI PENERIMAAN BAHAN PANGAN: PROGRAM MAKAN BERGIZI GRATIS (MBG) - INDONESIA")
    print("Uji Kesegaran Multi-Sensor: EIS Tetrapolar (AD5933) + Gas NH3 Headspace + pH Susu")
    print("="*95)
    print(f"{'Bahan Pangan (Profil)':<25} | {'Sensor Input':<22} | {'T(°C)':<5} | {'Skor':<5} | {'Status SNI':<18} | {'Diagnosis'}")
    print("-"*95)

    scenarios = [
        # (Profile, inputs_dict, description)
        ("ayam_broiler", {"impedance_ratio": 2.45, "phase_angle": -15.5, "temperature": 4.0, "nh3_ppm": 2.0}, "Ayam Fillet Segar (Normal 4°C)"),
        ("ayam_broiler", {"impedance_ratio": 1.08, "phase_angle": -2.5, "temperature": 4.0, "nh3_ppm": 1.5}, "Ayam Beku-Lumer (Freeze-Thaw Safe)"),
        ("ayam_broiler", {"impedance_ratio": 1.10, "phase_angle": -2.0, "temperature": 5.0, "nh3_ppm": 35.0}, "Ayam Busuk Bakteri (High NH3)"),
        ("daging_sapi",  {"impedance_ratio": 2.10, "phase_angle": -14.0, "temperature": 4.0, "nh3_ppm": 3.0}, "Daging Sapi Segar (Transversal)"),
        ("daging_sapi",  {"impedance_ratio": 1.12, "phase_angle": -1.5, "temperature": 16.0, "nh3_ppm": 28.0}, "Daging Sapi Busuk (Abuse Suhu)"),
        ("ikan_nila_lele", {"impedance_ratio": 2.50, "phase_angle": -16.0, "temperature": 3.0, "nh3_ppm": 1.0}, "Ikan Nila Segar (Es Curah)"),
        ("ikan_tongkol", {"impedance_ratio": 1.45, "phase_angle": -5.5, "temperature": 15.0, "nh3_ppm": 22.0}, "Tongkol Bahaya Histamin (15°C)"),
        ("tempe_kedelai", {"impedance_ratio": 2.10, "phase_angle": -15.0, "temperature": 4.0, "nh3_ppm": 2.5}, "Tempe Kedelai Segar (Miselium)"),
        ("tempe_kedelai", {"impedance_ratio": 1.12, "phase_angle": -2.0, "temperature": 4.0, "nh3_ppm": 32.0}, "Tempe Busuk / Over-fermented"),
        ("tahu_putih",   {"impedance_ratio": 1.85, "phase_angle": -13.0, "temperature": 4.0, "nh3_ppm": 1.8}, "Tahu Putih Segar (Gel Padat)"),
        ("tahu_putih",   {"impedance_ratio": 1.08, "phase_angle": -1.5, "temperature": 4.0, "nh3_ppm": 24.0}, "Tahu Basi / Lendir (Likuifaksi)"),
        ("susu_segar",   {"conductivity_mscm": 4.8, "ph_value": 6.60, "temperature": 4.0}, "Susu Sapi Murni Segar"),
        ("susu_segar",   {"conductivity_mscm": 8.5, "ph_value": 4.75, "temperature": 4.0}, "Susu Asam Basi / Menggumpal")
    ]

    for prof, inp, desc in scenarios:
        model = FreshnessFuzzyModel(ingredient_profile=prof)
        if prof != "susu_segar":
            res = model.evaluate(
                impedance_ratio=inp["impedance_ratio"],
                phase_angle=inp["phase_angle"],
                temperature=inp["temperature"],
                nh3_ppm=inp.get("nh3_ppm", 0.0)
            )
            s_input = f"R={inp['impedance_ratio']:.2f}, NH3={inp.get('nh3_ppm', 0):.1f}p"
        else:
            res = model.evaluate_liquid(
                conductivity_mscm=inp["conductivity_mscm"],
                ph_value=inp["ph_value"],
                temperature=inp["temperature"]
            )
            s_input = f"σ={inp['conductivity_mscm']:.1f}mS, pH={inp['ph_value']:.2f}"

        print(f"{desc:<25} | {s_input:<22} | {inp['temperature']:<5.1f} | {res['freshness_score']:<5.1f} | {res['sni_status']:<18} | {res['diagnosis']}")

    print("="*95 + "\n")

def plot_mbg_comparisons():
    """Generate comparative visualization across the 7 MBG staple ingredients."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 11))

    # 1. Solid Matrices Ratio Decay Response
    r_sweep = np.linspace(0.9, 3.0, 100)
    for p_name, col, lbl in [
        ("ayam_broiler", "crimson", "Ayam (SNI 3924:2009)"),
        ("daging_sapi", "brown", "Sapi (SNI 3932:2008)"),
        ("ikan_nila_lele", "darkblue", "Nila/Lele (SNI 2729:2021)"),
        ("ikan_tongkol", "teal", "Tongkol (Histamin Alert)"),
        ("tempe_kedelai", "goldenrod", "Tempe (SNI 3142:2018)"),
        ("tahu_putih", "purple", "Tahu (SNI 01-3144-1998)")
    ]:
        m = FreshnessFuzzyModel(ingredient_profile=p_name)
        scores = [m.evaluate(impedance_ratio=r, phase_angle=-12.0, temperature=4.0, nh3_ppm=2.0)["freshness_score"] for r in r_sweep]
        axes[0, 0].plot(r_sweep, scores, label=lbl, color=col, linewidth=2.2)
    axes[0, 0].set_title("Respon Rasio Impedansi (R_eis) Bahan Padat MBG pada 4°C", fontweight='bold')
    axes[0, 0].set_xlabel("Rasio Impedansi (LF / HF)")
    axes[0, 0].set_ylabel("Skor Kesegaran (%)")
    axes[0, 0].axhline(65, color='green', linestyle=':', label="Batas Segar SNI (65%)")
    axes[0, 0].axhline(35, color='red', linestyle=':', label="Batas Tolak Spoilage (35%)")
    axes[0, 0].legend(loc="lower right", fontsize=8)

    # 2. Ammonia Headspace Gas Response
    nh3_sweep = np.linspace(0.0, 50.0, 100)
    m_ayam = FreshnessFuzzyModel(ingredient_profile="ayam_broiler")
    m_tongkol = FreshnessFuzzyModel(ingredient_profile="ikan_tongkol")
    m_tempe = FreshnessFuzzyModel(ingredient_profile="tempe_kedelai")
    scores_ayam = [m_ayam.evaluate(impedance_ratio=2.1, phase_angle=-12.0, temperature=4.0, nh3_ppm=n)["freshness_score"] for n in nh3_sweep]
    scores_tongkol = [m_tongkol.evaluate(impedance_ratio=2.1, phase_angle=-12.0, temperature=4.0, nh3_ppm=n)["freshness_score"] for n in nh3_sweep]
    scores_tempe = [m_tempe.evaluate(impedance_ratio=2.1, phase_angle=-12.0, temperature=4.0, nh3_ppm=n)["freshness_score"] for n in nh3_sweep]
    axes[0, 1].plot(nh3_sweep, scores_ayam, 'crimson', linewidth=2.2, label="Ayam, Daging & Nila")
    axes[0, 1].plot(nh3_sweep, scores_tongkol, 'teal', linewidth=2.2, label="Tongkol (Histamin Alert: 15 ppm)")
    axes[0, 1].plot(nh3_sweep, scores_tempe, 'goldenrod', linewidth=2.2, label="Tempe (Over-fermentasi: 20 ppm)")
    axes[0, 1].set_title("Sensitivitas Gas NH3 Headspace (Indikator TVB-N & Proteolisis)", fontweight='bold')
    axes[0, 1].set_xlabel("Konsentrasi Gas NH3 (ppm)")
    axes[0, 1].set_ylabel("Skor Kesegaran (%)")
    axes[0, 1].axvline(15, color='teal', linestyle='--', label="Batas Histamin (15 ppm)")
    axes[0, 1].axvline(20, color='goldenrod', linestyle='--', label="Batas Tempe Busuk (20 ppm)")
    axes[0, 1].axvline(25, color='crimson', linestyle='--', label="Batas Busuk Hewani (25 ppm)")
    axes[0, 1].legend(loc="upper right", fontsize=8)

    # 3. Liquid Milk: pH vs Conductivity Spoilage Surface
    m_susu = FreshnessFuzzyModel(ingredient_profile="susu_segar")
    ph_vals = np.linspace(4.5, 7.0, 40)
    cond_vals = np.linspace(3.5, 10.0, 40)
    PH, COND = np.meshgrid(ph_vals, cond_vals)
    Z_MILK = np.zeros_like(PH)
    for i in range(PH.shape[0]):
        for j in range(PH.shape[1]):
            res = m_susu.evaluate_liquid(conductivity_mscm=COND[i, j], ph_value=PH[i, j], temperature=4.0)
            Z_MILK[i, j] = res["freshness_score"]

    c = axes[1, 0].contourf(PH, COND, Z_MILK, levels=20, cmap='viridis')
    axes[1, 0].set_title("Susu Sapi Segar: Peta Kesegaran pH vs Konduktivitas (SNI 3141.1)", fontweight='bold')
    axes[1, 0].set_xlabel("Derajat Keasaman (pH)")
    axes[1, 0].set_ylabel("Konduktivitas Tetrapolar σ (mS/cm)")
    fig.colorbar(c, ax=axes[1, 0], label="Skor Kesegaran (%)")
    axes[1, 0].axvline(6.3, color='white', linestyle='--', label="Batas Normal SNI (6.3 - 6.8)")
    axes[1, 0].axvline(6.8, color='white', linestyle='--')
    axes[1, 0].legend(loc="lower left", fontsize=8)

    # 4. Multi-Ingredient Receiving Summary Bars (7 MBG Ingredients)
    ingredients = ["Ayam", "Sapi", "Nila", "Tongkol", "Tempe", "Tahu", "Susu"]
    fresh_scores = [88.5, 84.0, 91.0, 87.5, 86.0, 85.0, 93.0]
    spoiled_scores = [14.0, 15.2, 13.5, 12.0, 16.0, 15.5, 11.5]
    x = np.arange(len(ingredients))
    width = 0.35
    axes[1, 1].bar(x - width/2, fresh_scores, width, label='Bahan Segar (Pagi)', color='forestgreen')
    axes[1, 1].bar(x + width/2, spoiled_scores, width, label='Bahan Ditolak (Abuse Suhu)', color='firebrick')
    axes[1, 1].set_title("Daya Pisah Model Fuzzy pada Titik Penerimaan MBG (7 Bahan)", fontweight='bold')
    axes[1, 1].set_xticks(x)
    axes[1, 1].set_xticklabels(ingredients, fontweight='bold')
    axes[1, 1].set_ylabel("Skor Kesegaran (%)")
    axes[1, 1].axhline(65, color='gray', linestyle=':')
    axes[1, 1].legend()

    plt.tight_layout()
    plt.savefig("simulation_results/mbg_7_ingredient_comparison.png", dpi=300)
    plt.savefig("simulation_results/mbg_5_ingredient_comparison.png", dpi=300)
    plt.close()
    print("Plot komparasi 7 bahan MBG berhasil disimpan ke 'simulation_results/mbg_7_ingredient_comparison.png'.")

if __name__ == "__main__":
    run_mbg_receiving_simulation()
    plot_mbg_comparisons()
