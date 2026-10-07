/**
 * @file mbg_inspect_cli.c
 * @brief Standalone Central Kitchen (SPPG) Inspection CLI Tool
 * Features interactive main menu navigation, zero blue SNI clutter, and automatic HACCP audit logging.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../firmware/src/mbg_fuzzy_core.h"
#include "../firmware/src/mbg_profiles.h"

static void clear_screen(void) {
    printf("\033[H\033[J");
}

static void log_to_csv(
    const char* timestamp,
    const MbgProfileConfig* cfg,
    float input1,
    float input2,
    float temp,
    float score,
    const char* status,
    const char* diagnosis
) {
    FILE* f = fopen("audit_logs/receiving_inspection.csv", "a");
    if (!f) {
        system("mkdir -p audit_logs");
        f = fopen("audit_logs/receiving_inspection.csv", "a");
    }
    if (f) {
        fseek(f, 0, SEEK_END);
        if (ftell(f) == 0) {
            fprintf(f, "Timestamp,Profile,Input1,Input2,Temperature_C,Freshness_Score,Status_Mutu,Diagnosis\n");
        }
        char score_csv[16] = "";
        if (isfinite(score)) snprintf(score_csv, sizeof(score_csv), "%.1f", score);
        fprintf(f, "\"%s\",\"%s\",%.2f,%.2f,%.1f,%s,\"%s\",\"%s\"\n",
                timestamp, cfg->code_name, input1, input2, temp, score_csv, status, diagnosis);
        fclose(f);
    }
}

static void print_result_card(
    const MbgProfileConfig* cfg,
    float r_eis,
    float phase,
    float temp,
    float nh3,
    float cond,
    float ph,
    MbgInferenceResult res
) {
    time_t now = time(NULL);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    printf("\n\033[1;37m========================================================================\033[0m\n");
    printf("\033[1;33m                     HASIL INSPEKSI PENERIMAAN                          \033[0m\n");
    printf("\033[1;37m========================================================================\033[0m\n");
    printf("  \033[1mWaktu Uji\033[0m         : %s\n", time_str);
    printf("  \033[1mBahan Pangan\033[0m      : \033[1;32m%s\033[0m\n", cfg->indonesian_name);
    printf("  \033[1mOrientasi Jarum\033[0m   : %s\n", cfg->orientation_notice);
    printf("  ----------------------------------------------------------------------\n");

    if (cfg->mode == MBG_MODE_SOLID_MATRIX) {
        printf("  \033[1mParameter Sensor\033[0m  : R_eis=\033[33m%.2f\033[0m, Fasa=\033[33m%.1f°\033[0m, Suhu=\033[33m%.1f°C\033[0m, NH3=\033[33m%.1f ppm\033[0m\n",
               r_eis, phase, temp, nh3);
        log_to_csv(time_str, cfg, r_eis, nh3, temp, res.freshness_score, res.sni_status, res.diagnosis);
    } else {
        printf("  \033[1mParameter Sensor\033[0m  : Konduktivitas=\033[33m%.2f mS/cm\033[0m, pH=\033[33m%.2f\033[0m, Suhu=\033[33m%.1f°C\033[0m\n",
               cond, ph, temp);
        log_to_csv(time_str, cfg, cond, ph, temp, res.freshness_score, res.sni_status, res.diagnosis);
    }

    printf("  ----------------------------------------------------------------------\n");
    if (res.classification == MBG_CLASS_INDETERMINATE) {
        printf("  \033[1mSkor Kesegaran\033[0m    : -- (ulangi pengukuran)\n");
    } else {
        printf("  \033[1mSkor Kesegaran\033[0m    : \033[1;37m%.1f %%\033[0m\n", res.freshness_score);
    }

    if (res.classification == MBG_CLASS_FRESH) {
        printf("  \033[1mStatus Kelayakan\033[0m  : \033[1;42;37m %s \033[0m\n", res.sni_status);
    } else if (res.classification == MBG_CLASS_SPOILED) {
        printf("  \033[1mStatus Kelayakan\033[0m  : \033[1;41;37m %s \033[0m\n", res.sni_status);
    } else {
        printf("  \033[1mStatus Kelayakan\033[0m  : \033[1;43;30m %s \033[0m\n", res.sni_status);
    }

    printf("  \033[1mDiagnosis Fisik\033[0m   : \033[1;35m%s\033[0m\n", res.diagnosis);

    if (res.histamine_hazard_alert) {
        printf("  \033[1;41;37m [PERINGATAN BAHAYA] Terdeteksi toksin histamin! Segera tolak/karantina! \033[0m\n");
    }
    if (res.freeze_thaw_detected) {
        printf("  \033[1;33m [INFO BEKU-LUMER] Lisis membran akibat kristal es, bukan mikroba (Aman diolah).\033[0m\n");
    }

    printf("\033[1;37m========================================================================\033[0m\n");
    printf("  \033[90mLog tersimpan di: audit_logs/receiving_inspection.csv\033[0m\n\n");
}

static void run_inspection_for_profile(MbgProfileId prof_id, int scenario_mode) {
    const MbgProfileConfig* cfg = mbg_get_profile_config(prof_id);
    float r_eis = 2.20f, phase = -14.0f, temp = 4.0f, nh3 = 2.0f;
    float cond = 4.8f, ph = 6.60f;

    if (cfg->mode == MBG_MODE_SOLID_MATRIX) {
        if (scenario_mode == 1) {
            /* Fresh sample */
            switch (prof_id) {
                case MBG_PROFILE_AYAM_BROILER: r_eis = 2.45f; phase = -15.5f; nh3 = 2.0f; break;
                case MBG_PROFILE_DAGING_SAPI:  r_eis = 2.10f; phase = -14.0f; nh3 = 3.0f; break;
                case MBG_PROFILE_IKAN_NILA_LELE: r_eis = 2.50f; phase = -16.0f; nh3 = 1.0f; break;
                case MBG_PROFILE_IKAN_TONGKOL: r_eis = 2.20f; phase = -15.0f; nh3 = 3.0f; break;
                case MBG_PROFILE_TEMPE_KEDELAI: r_eis = 2.10f; phase = -15.0f; nh3 = 2.5f; break;
                case MBG_PROFILE_TAHU_PUTIH:   r_eis = 1.85f; phase = -13.0f; nh3 = 1.8f; break;
                default: break;
            }
            temp = 4.0f;
        } else if (scenario_mode == 2) {
            /* Spoiled / abused sample */
            switch (prof_id) {
                case MBG_PROFILE_AYAM_BROILER: r_eis = 1.10f; phase = -2.0f; nh3 = 35.0f; temp = 5.0f; break;
                case MBG_PROFILE_DAGING_SAPI:  r_eis = 1.12f; phase = -1.5f; nh3 = 28.0f; temp = 16.0f; break;
                case MBG_PROFILE_IKAN_NILA_LELE: r_eis = 1.08f; phase = -2.0f; nh3 = 30.0f; temp = 6.0f; break;
                case MBG_PROFILE_IKAN_TONGKOL: r_eis = 1.45f; phase = -5.5f; nh3 = 22.0f; temp = 15.0f; break;
                case MBG_PROFILE_TEMPE_KEDELAI: r_eis = 1.12f; phase = -2.0f; nh3 = 32.0f; temp = 4.0f; break;
                case MBG_PROFILE_TAHU_PUTIH:   r_eis = 1.08f; phase = -1.5f; nh3 = 24.0f; temp = 4.0f; break;
                default: break;
            }
        } else if (scenario_mode == 3) {
            /* Freeze-thaw sample for meats */
            r_eis = 1.08f; phase = -2.5f; nh3 = 1.5f; temp = 4.0f;
        }
        MbgInferenceResult res = mbg_evaluate_solid(prof_id, r_eis, phase, temp, nh3);
        print_result_card(cfg, r_eis, phase, temp, nh3, 0.0f, 0.0f, res);
    } else {
        /* Liquid milk */
        if (scenario_mode == 1) {
            cond = 4.8f; ph = 6.60f; temp = 4.0f;
        } else {
            cond = 8.5f; ph = 4.75f; temp = 4.0f;
        }
        MbgInferenceResult res = mbg_evaluate_liquid(cond, ph, temp);
        print_result_card(cfg, 0.0f, 0.0f, temp, 0.0f, cond, ph, res);
    }
}

static void wait_for_enter_to_return(void) {
    printf("\n  \033[1;32m[Tekan ENTER untuk kembali ke Menu Utama]\033[0m ");
    fflush(stdout);
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin)) {
        /* User pressed Enter to return */
    }
}

static void run_all_benchmark_simulation(void) {
    clear_screen();
    printf("\n\033[1;37m========================================================================\033[0m\n");
    printf("\033[1;33m       SIMULASI PENERIMAAN 7 BAHAN PANGAN MBG (CCP-1 RECEIVING)         \033[0m\n");
    printf("\033[1;37m========================================================================\033[0m\n\n");

    for (int i = 0; i < MBG_PROFILE_COUNT; ++i) {
        printf("\033[1;37m>>> Uji Sampel Segar [%d/7]:\033[0m\n", i + 1);
        run_inspection_for_profile((MbgProfileId)i, 1);
    }

    wait_for_enter_to_return();
}

static void run_interactive_menu(void) {
    char input_buf[64];

    while (1) {
        clear_screen();
        printf("\n\033[1;37m========================================================================\033[0m\n");
        printf("\033[1;33m     SISTEM INSPEKSI KESEGARAN BAHAN PANGAN MBG (CCP-1 RECEIVING)       \033[0m\n");
        printf("\033[1;37m========================================================================\033[0m\n");
        printf("  Silakan pilih bahan pangan yang akan diuji:\n\n");
        printf("  \033[1m[1]\033[0m Daging Ayam Broiler\n");
        printf("  \033[1m[2]\033[0m Daging Sapi (Otot Skelet)\n");
        printf("  \033[1m[3]\033[0m Ikan Air Tawar (Ikan Nila / Lele)\n");
        printf("  \033[1m[4]\033[0m Ikan Laut Pelagis (Ikan Tongkol / Cakalang)\n");
        printf("  \033[1m[5]\033[0m Tempe Kedelai (Fermentasi Rhizopus)\n");
        printf("  \033[1m[6]\033[0m Tahu Putih (Protein Gel)\n");
        printf("  \033[1m[7]\033[0m Susu Sapi Segar (Liquid)\n");
        printf("  ----------------------------------------------------------------------\n");
        printf("  \033[1m[8]\033[0m Jalankan Simulasi Lengkap (Semua 7 Bahan Sekaligus)\n");
        printf("  \033[1m[0]\033[0m Keluar Aplikasi\n");
        printf("\033[1;37m========================================================================\033[0m\n");
        printf("  Pilihan Anda [0-8]: ");
        fflush(stdout);

        if (!fgets(input_buf, sizeof(input_buf), stdin)) {
            break;
        }

        /* Skip empty enter on menu */
        char *p = input_buf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\0') {
            continue;
        }

        int choice = atoi(p);
        if (choice == 0 && *p == '0') {
            printf("\nTerima kasih. Sistem dimatikan.\n\n");
            break;
        }

        if (choice >= 1 && choice <= 7) {
            MbgProfileId prof_id = (MbgProfileId)(choice - 1);
            const MbgProfileConfig* cfg = mbg_get_profile_config(prof_id);

            printf("\n  Bahan dipilih: \033[1;32m%s\033[0m\n", cfg->indonesian_name);
            printf("  Pilih Kondisi Sampel:\n");
            printf("    \033[1m[1]\033[0m Sampel Segar (Normal 4°C)\n");
            printf("    \033[1m[2]\033[0m Sampel Rusak / Suhu Hangat (Spoiled)\n");
            if (cfg->mode == MBG_MODE_SOLID_MATRIX && prof_id != MBG_PROFILE_TEMPE_KEDELAI && prof_id != MBG_PROFILE_TAHU_PUTIH) {
                printf("    \033[1m[3]\033[0m Sampel Beku-Lumer (Freeze-Thaw Safe)\n");
            }
            printf("  Pilih [1/2]: ");
            fflush(stdout);

            int sub_choice = 1;
            if (fgets(input_buf, sizeof(input_buf), stdin)) {
                sub_choice = atoi(input_buf);
                if (sub_choice < 1) sub_choice = 1;
            }

            clear_screen();
            run_inspection_for_profile(prof_id, sub_choice);
            wait_for_enter_to_return();

        } else if (choice == 8) {
            run_all_benchmark_simulation();
        } else {
            printf("\n  \033[31mPilihan tidak valid. Silakan coba lagi.\033[0m\n");
            wait_for_enter_to_return();
        }
    }
}

int main(int argc, char** argv) {
    /* If arguments provided, run once in direct batch mode */
    if (argc > 1) {
        const char* profile_str = "ayam_broiler";
        float r_eis = 2.45f;
        float phase = -15.0f;
        float temp = 4.0f;
        float nh3 = NAN;
        float cond = 4.8f;
        float ph = NAN;

        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "--menu") == 0) {
                run_interactive_menu();
                return 0;
            } else if (strcmp(argv[i], "--profile") == 0 && i + 1 < argc) {
                profile_str = argv[++i];
            } else if (strcmp(argv[i], "--ratio") == 0 && i + 1 < argc) {
                r_eis = (float)atof(argv[++i]);
            } else if (strcmp(argv[i], "--phase") == 0 && i + 1 < argc) {
                phase = (float)atof(argv[++i]);
            } else if (strcmp(argv[i], "--temp") == 0 && i + 1 < argc) {
                temp = (float)atof(argv[++i]);
            } else if (strcmp(argv[i], "--nh3") == 0 && i + 1 < argc) {
                nh3 = (float)atof(argv[++i]);
            } else if (strcmp(argv[i], "--cond") == 0 && i + 1 < argc) {
                cond = (float)atof(argv[++i]);
            } else if (strcmp(argv[i], "--ph") == 0 && i + 1 < argc) {
                ph = (float)atof(argv[++i]);
            }
        }

        MbgProfileId prof_id = mbg_profile_from_string(profile_str);
        const MbgProfileConfig* cfg = mbg_get_profile_config(prof_id);
        MbgInferenceResult res;

        if (cfg->mode == MBG_MODE_SOLID_MATRIX) {
            res = mbg_evaluate_solid(prof_id, r_eis, phase, temp, nh3);
            print_result_card(cfg, r_eis, phase, temp, nh3, 0.0f, 0.0f, res);
        } else {
            res = mbg_evaluate_liquid(cond, ph, temp);
            print_result_card(cfg, 0.0f, 0.0f, temp, 0.0f, cond, ph, res);
        }

        printf("  Ketik './bin/mbg_inspect' tanpa argumen untuk membuka Menu Utama.\n\n");
        return 0;
    }

    /* Otherwise, enter interactive full-screen menu loop */
    run_interactive_menu();
    return 0;
}
