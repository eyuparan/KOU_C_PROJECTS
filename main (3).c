#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GOREV 2000

// Görev Durumları
#define DURUM_GELMEDI       0
#define DURUM_BEKLIYOR      1
#define DURUM_YURUTULUYOR   2
#define DURUM_TAMAMLANDI    3

// Görev Yapısı
typedef struct {
    int id;
    int gelis_zamani;
    int sure;
    int kalan_sure;
    int oncelik;
    char gorev_turu[40];
    char konum[20];

    int durum;
    int atanan_teknisyen_id;
    int ilk_baslama_zamani;
    int bitis_zamani;
} Gorev;

// Teknisyen Yapısı (4 Teknisyen: T1, T2, T3, T4)
typedef struct {
    int id;
    int mesgul_mu;
    int suanki_gorev_id;
    char son_konum[20];
    int geciste_mi;
    int toplam_calisma_suresi;
    int rr_quantum_sayaci;
} Teknisyen;

// 1. Shortest Job Algoritması (SJF)
int shortest_job(int guncel_zaman, Gorev gorevler[], int sayac) {
    int en_kisa_sure = 1000000;
    int secilen_index = -1;
    for (int i = 0; i < sayac; i++) {
        if (gorevler[i].gelis_zamani > guncel_zaman || gorevler[i].durum != DURUM_BEKLIYOR) {
            continue;
        }
        if (gorevler[i].sure < en_kisa_sure) {
            en_kisa_sure = gorevler[i].sure;
            secilen_index = i;
        } else if (gorevler[i].sure == en_kisa_sure) {
            if (secilen_index != -1 && gorevler[i].id < gorevler[secilen_index].id) {
                secilen_index = i;
            }
        }
    }
    return secilen_index;
}

// 2. Priority Algoritması
int priority(int guncel_zaman, Gorev gorevler[], int sayac) {
    int en_kucuk_oncelik = 6;
    int secilen_index = -1;
    for (int i = 0; i < sayac; i++) {
        if (gorevler[i].gelis_zamani > guncel_zaman || gorevler[i].durum != DURUM_BEKLIYOR) {
            continue;
        }
        if (gorevler[i].oncelik < en_kucuk_oncelik) {
            en_kucuk_oncelik = gorevler[i].oncelik;
            secilen_index = i;
        } else if (gorevler[i].oncelik == en_kucuk_oncelik) {
            if (secilen_index != -1 && gorevler[i].id < gorevler[secilen_index].id) {
                secilen_index = i;
            }
        }
    }
    return secilen_index;
}

// 3. Kendi Özgün Algoritmam (Konum ve Öncelik Optimizasyonlu)
int kendi_algoritmam(int guncel_zaman, Gorev gorevler[], int sayac, const char *teknisyen_konum) {
    int secilen_index = -1;
    int ayni_konumda_bulundu = 0;

    if (strlen(teknisyen_konum) > 0) {
        for (int i = 0; i < sayac; i++) {
            if (gorevler[i].gelis_zamani <= guncel_zaman && gorevler[i].durum == DURUM_BEKLIYOR) {
                if (strcmp(gorevler[i].konum, teknisyen_konum) == 0) {
                    if (!ayni_konumda_bulundu || gorevler[i].oncelik < gorevler[secilen_index].oncelik) {
                        secilen_index = i;
                        ayni_konumda_bulundu = 1;
                    }
                }
            }
        }
    }

    if (!ayni_konumda_bulundu) {
        secilen_index = priority(guncel_zaman, gorevler, sayac);
    }

    return secilen_index;
}

// Dosya Okuma ve Validasyon Fonksiyonu
int dosya_okuma(const char *dosya_yolu, Gorev gorevler[]) {
    FILE *dosya = fopen(dosya_yolu, "r");
    if (dosya == NULL) {
        printf("Hata: '%s' dosyasi acilamadi!\n", dosya_yolu);
        return -1;
    }

    char satir_oku[256];
    int sayac = 0;
    int satir_no = 0;

    while (fgets(satir_oku, sizeof(satir_oku), dosya) != NULL) {
        satir_no++;
        char *ptr = satir_oku;
        while (*ptr == ' ' || *ptr == '\t') ptr++;

        if (*ptr == '\n' || *ptr == '\r' || *ptr == '#' || *ptr == '\0') {
            continue;
        }

        Gorev g;
        int okunan_alan = sscanf(ptr, "%d %d %d %d %39s %19s", &g.id, &g.gelis_zamani, &g.sure, &g.oncelik, g.gorev_turu, g.konum);

        if (okunan_alan != 6) {
            printf("Satir %d reddedildi: Eksik veya hatali alan.\n", satir_no);
            continue;
        }

        g.kalan_sure = g.sure;
        g.durum = DURUM_GELMEDI;
        g.atanan_teknisyen_id = 0;
        g.ilk_baslama_zamani = -1;
        g.bitis_zamani = -1;

        gorevler[sayac++] = g;
    }

    fclose(dosya);
    return sayac;
}

int main() {
    char dosya_adi[100];
    printf("Gorev dosyasinin adini/yolunu giriniz: ");
    scanf("%99s", dosya_adi);

    Gorev gorevler[MAX_GOREV];
    int sayac = dosya_okuma(dosya_adi, gorevler);

    if (sayac <= 0) {
        printf("Gecerli gorev okunamadi veya dosya bos.\n");
        return 1;
    }

    printf("\nBasariyla %d gecerli gorev okundu!\n", sayac);

    Teknisyen teknisyenler[4];
    for (int i = 0; i < 4; i++) {
        teknisyenler[i].id = i + 1;
        teknisyenler[i].mesgul_mu = 0;
        teknisyenler[i].suanki_gorev_id = -1;
        strcpy(teknisyenler[i].son_konum, "");
        teknisyenler[i].geciste_mi = 0;
        teknisyenler[i].toplam_calisma_suresi = 0;
        teknisyenler[i].rr_quantum_sayaci = 0;
    }

    int guncel_zaman = 0;
    int secim;
    int secili_algoritma = 1;
    int quantum = 2; // Varsayılan quantum

    while (1) {
        printf("\n================ PROLAB SiMULASYON MENUSU ================\n");
        printf("Guncel Zaman: %d | Secili Algoritma: %d | Quantum: %d\n", guncel_zaman, secili_algoritma, quantum);
        printf("-----------------------------------------------------------\n");
        printf("1 - Algoritma Sec veya Degistir (1:SJF, 2:Priority, 3:RR, 4:Kendi)\n");
        printf("2 - Tum Gorevleri Listele\n");
        printf("3 - Gorevleri Durumuna Gore Listele\n");
        printf("4 - Gorev id ile Sorgula\n");
        printf("5 - Teknisyenlerin Durumunu Goster\n");
        printf("6 - Zamani 1 Birim ilerlet\n");
        printf("7 - Zamani N Birim ilerlet\n");
        printf("8 - Belirli Bir Zamana Git\n");
        printf("9 - istatistikleri Goster\n");
        printf("10 - Simulasyonu Sifirla\n");
        printf("11 - cikis\n");
        printf("Seciminiz: ");
        scanf("%d", &secim);

        if (secim == 11) {
            printf("Programdan cikiliyor...\n");
            break;
        }

        if (secim == 1) {
            printf("Algoritma seciniz (1: SJF, 2: Priority, 3: Round Robin, 4: Kendi Algoritmam): ");
            scanf("%d", &secili_algoritma);
            if (secili_algoritma == 3) {
                printf("Round Robin icin Quantum degerini giriniz: ");
                scanf("%d", &quantum);
            }
            // Algoritma değiştiğinde simülasyonu başa sar
            guncel_zaman = 0;
            for (int i = 0; i < sayac; i++) {
                gorevler[i].kalan_sure = gorevler[i].sure;
                gorevler[i].durum = DURUM_GELMEDI;
                gorevler[i].atanan_teknisyen_id = 0;
                gorevler[i].ilk_baslama_zamani = -1;
                gorevler[i].bitis_zamani = -1;
            }
            for (int i = 0; i < 4; i++) {
                teknisyenler[i].mesgul_mu = 0;
                teknisyenler[i].suanki_gorev_id = -1;
                strcpy(teknisyenler[i].son_konum, "");
                teknisyenler[i].geciste_mi = 0;
                teknisyenler[i].toplam_calisma_suresi = 0;
                teknisyenler[i].rr_quantum_sayaci = 0;
            }
            printf("Algoritma degistirildi ve simulasyon zaman 0'a sifirlandi.\n");
            continue;
        }

        // Zaman İlerletme Fonksiyonu (Yardımcı Blok)
        if (secim == 6 || secim == 7 || secim == 8) {
            int hedef_zaman = guncel_zaman;
            if (secim == 6) hedef_zaman = guncel_zaman + 1;
            else if (secim == 7) {
                int n;
                printf("İlerletilecek birim sayisini (N) giriniz: ");
                scanf("%d", &n);
                hedef_zaman = guncel_zaman + n;
            } else if (secim == 8) {
                printf("Gitmek istediginiz hedef zamani giriniz: ");
                scanf("%d", &hedef_zaman);
                if (hedef_zaman < guncel_zaman) {
                    printf("Gecmise donus yapiliyor, simulasyon bastan hesaplaniyor...\n");
                    guncel_zaman = 0;
                    for (int i = 0; i < sayac; i++) {
                        gorevler[i].kalan_sure = gorevler[i].sure;
                        gorevler[i].durum = DURUM_GELMEDI;
                        gorevler[i].atanan_teknisyen_id = 0;
                        gorevler[i].ilk_baslama_zamani = -1;
                        gorevler[i].bitis_zamani = -1;
                    }
                    for (int i = 0; i < 4; i++) {
                        teknisyenler[i].mesgul_mu = 0;
                        teknisyenler[i].suanki_gorev_id = -1;
                        strcpy(teknisyenler[i].son_konum, "");
                        teknisyenler[i].geciste_mi = 0;
                        teknisyenler[i].toplam_calisma_suresi = 0;
                        teknisyenler[i].rr_quantum_sayaci = 0;
                    }
                }
            }

            // Adım adım simülasyonu hedefe kadar çalıştır
            while (guncel_zaman < hedef_zaman) {
                // 1. Yeni gelen görevler
                for (int i = 0; i < sayac; i++) {
                    if (gorevler[i].durum == DURUM_GELMEDI && gorevler[i].gelis_zamani <= guncel_zaman) {
                        gorevler[i].durum = DURUM_BEKLIYOR;
                    }
                }

                // 2. Teknisyen ilerletme
                for (int t = 0; t < 4; t++) {
                    if (teknisyenler[t].mesgul_mu) {
                        int g_idx = -1;
                        for (int i = 0; i < sayac; i++) {
                            if (gorevler[i].id == teknisyenler[t].suanki_gorev_id) {
                                g_idx = i;
                                break;
                            }
                        }

                        if (g_idx != -1) {
                            if (teknisyenler[t].geciste_mi) {
                                teknisyenler[t].geciste_mi = 0;
                            } else {
                                gorevler[g_idx].kalan_sure--;
                                teknisyenler[t].toplam_calisma_suresi++;
                                teknisyenler[t].rr_quantum_sayaci++;

                                if (gorevler[g_idx].kalan_sure == 0) {
                                    gorevler[g_idx].durum = DURUM_TAMAMLANDI;
                                    gorevler[g_idx].bitis_zamani = guncel_zaman + 1;
                                    teknisyenler[t].mesgul_mu = 0;
                                    teknisyenler[t].suanki_gorev_id = -1;
                                    teknisyenler[t].rr_quantum_sayaci = 0;
                                } else if (secili_algoritma == 3 && teknisyenler[t].rr_quantum_sayaci >= quantum) {
                                    gorevler[g_idx].durum = DURUM_BEKLIYOR;
                                    teknisyenler[t].mesgul_mu = 0;
                                    teknisyenler[t].suanki_gorev_id = -1;
                                    teknisyenler[t].rr_quantum_sayaci = 0;
                                }
                            }
                        }
                    }
                }

                // 3. Atama yapma (T1 -> T4 sırasıyla)
                for (int t = 0; t < 4; t++) {
                    if (!teknisyenler[t].mesgul_mu) {
                        int secilen_idx = -1;

                        if (secili_algoritma == 1) {
                            secilen_idx = shortest_job(guncel_zaman, gorevler, sayac);
                        } else if (secili_algoritma == 2) {
                            secilen_idx = priority(guncel_zaman, gorevler, sayac);
                        } else if (secili_algoritma == 3) {
                            for (int i = 0; i < sayac; i++) {
                                if (gorevler[i].gelis_zamani <= guncel_zaman && gorevler[i].durum == DURUM_BEKLIYOR) {
                                    secilen_idx = i;
                                    break;
                                }
                            }
                        } else if (secili_algoritma == 4) {
                            secilen_idx = kendi_algoritmam(guncel_zaman, gorevler, sayac, teknisyenler[t].son_konum);
                        }

                        if (secilen_idx != -1) {
                            gorevler[secilen_idx].durum = DURUM_YURUTULUYOR;
                            gorevler[secilen_idx].atanan_teknisyen_id = teknisyenler[t].id;

                            if (gorevler[secilen_idx].ilk_baslama_zamani == -1) {
                                gorevler[secilen_idx].ilk_baslama_zamani = guncel_zaman;
                            }

                            teknisyenler[t].mesgul_mu = 1;
                            teknisyenler[t].suanki_gorev_id = gorevler[secilen_idx].id;
                            teknisyenler[t].rr_quantum_sayaci = 0;

                            if (strlen(teknisyenler[t].son_konum) > 0 && strcmp(teknisyenler[t].son_konum, gorevler[secilen_idx].konum) != 0) {
                                teknisyenler[t].geciste_mi = 1; // 1 birimlik konum geçiş cezası
                            }
                            strcpy(teknisyenler[t].son_konum, gorevler[secilen_idx].konum);
                        }
                    }
                }
                guncel_zaman++;
            }
            printf("Zaman basariyla ilerletildi. Güncel Zaman: %d\n", guncel_zaman);
            continue;
        }

        // Tüm Görevleri Listeleme
        if (secim == 2) {
            printf("\n--- TUM GOREVLER ---\n");
            printf("id\tGelis\tSure\tOncelik\tDurum\tTeknisyen\tKalan\n");
            for (int i = 0; i < sayac; i++) {
                printf("%d\t%d\t%d\t%d\t%d\tT%d\t\t%d\n",
                    gorevler[i].id, gorevler[i].gelis_zamani, gorevler[i].sure,
                    gorevler[i].oncelik, gorevler[i].durum, gorevler[i].atanan_teknisyen_id, gorevler[i].kalan_sure);
            }
            continue;
        }

        // Duruma Göre Listeleme
        if (secim == 3) {
            int d_secim;
            printf("Listelenecek durum (0: Gelmedi, 1: Bekliyor, 2: Yurutuluyor, 3: Tamamlandi): ");
            scanf("%d", &d_secim);
            printf("\n--- DURUMU %d OLAN GOREVLER ---\n", d_secim);
            int sayim = 0;
            for (int i = 0; i < sayac; i++) {
                if (gorevler[i].durum == d_secim) {
                    printf("id: %d | Tur: %s | Konum: %s | Sure: %d\n", gorevler[i].id, gorevler[i].gorev_turu, gorevler[i].konum, gorevler[i].sure);
                    sayim++;
                }
            }
            printf("Toplam %d gorev listelendi.\n", sayim);
            continue;
        }

        // ID ile Sorgulama
        if (secim == 4) {
            int aranan_id;
            printf("Sorgulanacak Gorev id giriniz: ");
            scanf("%d", &aranan_id);
            int bulundu = 0;
            for (int i = 0; i < sayac; i++) {
                if (gorevler[i].id == aranan_id) {
                    printf("\n[GOREV BULUNDU]\n");
                    printf("id: %d\nTur: %s\nKonum: %s\nGelis Zamani: %d\nToplam Sure: %d\nOncelik: %d\nilk Baslama: %d\nBitis Zamani: %d\n",
                        gorevler[i].id, gorevler[i].gorev_turu, gorevler[i].konum, gorevler[i].gelis_zamani,
                        gorevler[i].sure, gorevler[i].oncelik, gorevler[i].ilk_baslama_zamani, gorevler[i].bitis_zamani);
                    bulundu = 1;
                    break;
                }
            }
            if (!bulundu) printf("Bu id'ye sahip gorev bulunamadi.\n");
            continue;
        }

        // Teknisyen Durumları
        if (secim == 5) {
            printf("\n--- TEKNiSYEN DURUMLARi ---\n");
            for (int i = 0; i < 4; i++) {
                printf("Teknisyen T%d -> Durum: %s | Gorev iD: %d | Son Konum: %s | calisilan Birim: %d\n",
                    teknisyenler[i].id,
                    teknisyenler[i].mesgul_mu ? (teknisyenler[i].geciste_mi ? "Geciste (Yolda)" : "Mesgul") : "Bos",
                    teknisyenler[i].suanki_gorev_id,
                    strlen(teknisyenler[i].son_konum) > 0 ? teknisyenler[i].son_konum : "Yok",
                    teknisyenler[i].toplam_calisma_suresi);
            }
            continue;
        }

        // İstatistikleri Göster (3 Ondalık Basamaklı Rapor)
        if (secim == 9) {
            int tamamlanan_sayisi = 0;
            double toplam_bekleme = 0;
            double toplam_sistemde_kalma = 0;

            for (int i = 0; i < sayac; i++) {
                if (gorevler[i].durum == DURUM_TAMAMLANDI) {
                    tamamlanan_sayisi++;
                    int bekleme = (gorevler[i].bitis_zamani - gorevler[i].gelis_zamani) - gorevler[i].sure;
                    int sistemde_kalma = gorevler[i].bitis_zamani - gorevler[i].gelis_zamani;
                    toplam_bekleme += bekleme;
                    toplam_sistemde_kalma += sistemde_kalma;
                }
            }

            printf("\n================ SiMuLASYON iSTATiSTiKLERi ================\n");
            printf("Son Bitis Zamani (Makespan): %d birim\n", guncel_zaman);

            if (tamamlanan_sayisi > 0) {
                printf("Ortalama Bekleme Suresi    : %.3f\n", toplam_bekleme / tamamlanan_sayisi);
                printf("Ortalama Sistemde Kalma    : %.3f\n", toplam_sistemde_kalma / tamamlanan_sayisi);
            } else {
                printf("Ortalama Bekleme Suresi    : Hesaplanamaz (Tamamlanan gorev yok)\n");
            }

            // Teknisyen Kullanım Oranları (Düzeltilen alan: % işareti ve float değeri doğru yazdırılıyor)
            int toplam_calisma = 0;
            for (int i = 0; i < 4; i++) {
                toplam_calisma += teknisyenler[i].toplam_calisma_suresi;
                double teknisyen_oran = (guncel_zaman > 0) ? ((double)teknisyenler[i].toplam_calisma_suresi / guncel_zaman) * 100.0 : 0.0;
                printf("Teknisyen T%d Kullanim Orani: %%%.3f\n", teknisyenler[i].id, teknisyen_oran);
            }

            double genel_oran = (guncel_zaman > 0) ? ((double)toplam_calisma / (guncel_zaman * 4.0)) * 100.0 : 0.0;
            printf("Genel Kullanim Orani       : %%%.3f\n", genel_oran);
            printf("===========================================================\n");
            continue;
        }

        // Simülasyonu Sıfırla
        if (secim == 10) {
            guncel_zaman = 0;
            for (int i = 0; i < sayac; i++) {
                gorevler[i].kalan_sure = gorevler[i].sure;
                gorevler[i].durum = DURUM_GELMEDI;
                gorevler[i].atanan_teknisyen_id = 0;
                gorevler[i].ilk_baslama_zamani = -1;
                gorevler[i].bitis_zamani = -1;
            }
            for (int i = 0; i < 4; i++) {
                teknisyenler[i].mesgul_mu = 0;
                teknisyenler[i].suanki_gorev_id = -1;
                strcpy(teknisyenler[i].son_konum, "");
                teknisyenler[i].geciste_mi = 0;
                teknisyenler[i].toplam_calisma_suresi = 0;
                teknisyenler[i].rr_quantum_sayaci = 0;
            }
            printf("Simulasyon sifirlandi (Zaman = 0).\n");
            continue;
        }
    }

    return 0;
}
