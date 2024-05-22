/*  Muhamad Dzaky Maulana 2306264401
    Nabiel Harits Utomo   2306267044
    (JUDUL)*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <ctype.h>

#define MAX_NAMA_LEN 50
#define MAX_KATEGORI_LEN 50

typedef struct Produk {
    char nama[MAX_NAMA_LEN];
    char kategori[MAX_KATEGORI_LEN];
    int harga;
    int stok;
    float diskon;
    int diskon_aktif;
    struct Produk *next;
} Produk;

Produk *head = NULL;
omp_lock_t lock; 

//Function untuk membersihkan layar
void bersihkanLayar() {
    system("cls"); 
}

//Function untuk mengatur diskon dari suatu produk
void setDiskonProduk() {
    bersihkanLayar();
    char namaProduk[MAX_NAMA_LEN];
    printf("Masukkan nama produk yang ingin diberikan diskon: ");
    scanf(" %[^\n]%*c", namaProduk);

    float diskon;
    printf("Masukkan nilai diskon (dalam persentase, misal 15 untuk 15%%): ");
    scanf("%f", &diskon); getchar();

    if (diskon < 0 || diskon > 100) {
        printf("Nilai diskon tidak valid. Diskon harus antara 0%% dan 100%%.\n");
        printf("Tekan enter untuk kembali ke menu utama...");
        getchar();
        return;
    }

    Produk *ptr = head;
    int found = 0;

    #pragma omp parallel shared(found)
    {
        #pragma omp for
        for (int i = 0; ptr != NULL; i++) {
            #pragma omp critical
            {
                if (strcmp(ptr->nama, namaProduk) == 0) {
                    ptr->diskon = diskon;
                    ptr->diskon_aktif = 1;
                    printf("Diskon berhasil diterapkan. %s sekarang memiliki diskon %.2f%%.\n\n", ptr->nama, ptr->diskon);
                    found = 1;
                }
                ptr = ptr->next;
            }
        }
    }

    if (!found) {
        printf("Produk dengan nama \"%s\" tidak ditemukan.\n\n", namaProduk);
    }
    printf("Tekan enter untuk kembali ke menu utama...");
    getchar();
}

//Function untuk menambah produk kedalam sistem manajemen inventaris
void tambahProduk() {
    bersihkanLayar();
    Produk *p = (Produk *)malloc(sizeof(Produk));
    printf("Masukkan nama produk: ");
    scanf(" %[^\n]%*c", p->nama);
    printf("Masukkan kategori: ");
    scanf(" %[^\n]%*c", p->kategori);
    printf("Masukkan harga: ");
    scanf("%d", &p->harga);
    printf("Masukkan stok: ");
    scanf("%d", &p->stok); getchar();

    #pragma omp critical
    {
        p->next = head;
        head = p;
    }

    printf("Produk berhasil ditambahkan.\n\n");
    printf("Tekan enter untuk kembali ke menu utama...");
    getchar();
}

//Function untuk menampilkan produk
void tampilkanProduk() {
    bersihkanLayar();
    Produk *ptr = head;
    int count = 1;
    
    printf("Daftar Produk:\n");

    #pragma omp parallel shared(count)
    {
        #pragma omp for
        for (int i = 0; ptr != NULL; i++) {
            float hargaAkhir = ptr->diskon_aktif ? ptr->harga * (1 - (ptr->diskon / 100)) : ptr->harga;
            #pragma omp critical
            {
                printf("%d. Nama: %s\n    Kategori: %s\n    Harga Asli: %d\n    Diskon: %.2f%%\n    Harga Akhir: %.f\n    Stok: %d\n\n",
                       count, ptr->nama, ptr->kategori, ptr->harga, ptr->diskon, hargaAkhir, ptr->stok);
                ptr = ptr->next;
                count++;
            }
        }
    }
    printf("Tekan enter untuk kembali ke menu utama...");
    getchar();
}

//function untuk mengupdate stok dari produk
void updateStokProduk() {
    bersihkanLayar();
    char namaProduk[MAX_NAMA_LEN];
    printf("Masukkan nama produk untuk diperbarui stoknya: ");
    scanf(" %[^\n]%*c", namaProduk);

    Produk *ptr = head;
    int found = 0;

    #pragma omp parallel shared(found)
    {
        #pragma omp for
        for (int i = 0; ptr != NULL; i++) {
            if (strcmp(ptr->nama, namaProduk) == 0) {
                printf("Stok saat ini untuk %s: %d\n", ptr->nama, ptr->stok);
                printf("Masukkan perubahan stok (positif untuk menambah, negatif untuk mengurangi): ");
                int perubahanStok;
                scanf("%d", &perubahanStok); getchar();

                #pragma omp critical
                {
                    if (ptr->stok + perubahanStok < 0) {
                        printf("Pengurangan melebihi stok yang tersedia. Stok tidak dapat negatif.\n");
                        printf("Stok sekarang menjadi 0.\n\n");
                        ptr->stok = 0;
                    } else {
                        ptr->stok += perubahanStok;
                        printf("Stok untuk %s berhasil diperbarui menjadi %d.\n\n", ptr->nama, ptr->stok);
                    }
                    found = 1;
                }
            }
            ptr = ptr->next;
        }
    }

    if (!found) {
        printf("Produk dengan nama \"%s\" tidak ditemukan.\n\n", namaProduk);
    }
    printf("Tekan enter untuk kembali ke menu utama...");
    getchar();
}

//Function untuk mencari produk

void cariProduk() {
    bersihkanLayar();
    int pilihanPencarian;
    char inputCari[MAX_NAMA_LEN];

    printf("Cari berdasarkan:\n");
    printf("1. Nama Produk\n");
    printf("2. Kategori Produk\n");
    printf("Masukkan pilihan: ");
    scanf("%d", &pilihanPencarian); getchar();

    if (pilihanPencarian == 1) {
        printf("Masukkan nama produk: ");
    } else if (pilihanPencarian == 2) {
        printf("Masukkan kategori produk: ");
    } else {
        printf("Pilihan tidak valid.\n");
        return;
    }

    scanf(" %[^\n]%*c", inputCari);

    int found = 0;

    #pragma omp parallel shared(found)
    {
        #pragma omp for
        for (Produk *ptr = head; ptr != NULL; ptr = ptr->next) {
            #pragma omp task shared(ptr, found)
            {
                if ((pilihanPencarian == 1 && strcasecmp(ptr->nama, inputCari) != NULL) ||
                    (pilihanPencarian == 2 && strcasecmp(ptr->kategori, inputCari) != NULL)) {
                    #pragma omp critical
                    {
                        if (!found) {
                            printf("\nProduk Ditemukan:\n");
                            found = 1;
                        }
                        printf("Nama: %s\n", ptr->nama);
                        printf("Kategori: %s\n", ptr->kategori);
                        printf("Harga: %d\n", ptr->harga);
                        printf("Stok: %d\n\n", ptr->stok);
                    }
                }
            }
        }
        #pragma omp taskwait
    }

    if (!found) {
        printf("\nProduk tidak ditemukan.\n\n");
    }
    printf("Tekan enter untuk kembali ke menu utama...");
    getchar();
}

//Function menu selamat datang
void menuSelamatDatang() {
    printf("================================================================\n");
    printf(" Welcome to the Electronic Store Inventory Management System \n");
    printf("================================================================\n\n");
    printf("Press enter to continue to the Main Menu...");
    getchar();
}

//function untuk menampilkan panduan
void tampilkanPanduan() {
    printf("\n");
    printf("====================================================\n");
    printf("|                    PANDUAN                       |\n");
    printf("====================================================\n");
    printf("\n");
    printf("[*] Fitur Sistem Manajemen Inventaris Toko Elektronik:\n");
    printf("    - [1] Tambah Produk      : Untuk menambahkan produk baru ke inventaris.\n");
    printf("    - [2] Tampilkan Produk   : Menampilkan semua produk yang ada dalam inventaris.\n");
    printf("    - [3] Cari Produk        : Mencari produk berdasarkan nama atau kategori.\n");
    printf("    - [4] Perbarui Stok Produk: Mengupdate jumlah stok produk tertentu.\n");
    printf("    - [5] Set Diskon Produk  : Mengatur dikon dari suatu produk\n");
    printf("    - [6] Panduan            : Panduan dari program.\n");
    printf("    - [7] Keluar             : Keluar dari sistem.");
    printf("\n");
    printf("[*] Cara Menggunakan:\n");
    printf("    1. Pilih opsi dari menu utama.\n");
    printf("    2. Ikuti instruksi pada layar untuk setiap fitur.\n");
    printf("    3. Gunakan fitur 'Cari Produk' untuk menemukan produk dengan cepat.\n");
    printf("    4. Untuk mengupdate stok, pastikan produk sudah ditambahkan ke sistem.\n");
    printf("\n");
    printf("[*] Tips:\n");
    printf("    - Pastikan input yang dimasukkan sesuai dengan format yang diminta.\n");
    printf("    - Gunakan angka untuk navigasi menu.\n");
    printf("    - Tekan 'enter' setelah menyelesaikan input pada setiap langkah.\n");
    printf("\n");
    printf("****************************************************\n");
    printf(" Tekan enter untuk kembali ke menu utama...\n");
    getchar(); 
}

//function untuk menampilkan menu
void menu(){
    printf("\n");
    printf("========================================\n");
    printf("|   Sistem Manajemen Inventaris Toko   |\n");
    printf("|             Elektronik               |\n");
    printf("========================================\n");
    printf("\n");
    printf("[1] Tambah Produk\n");
    printf("[2] Tampilkan Produk\n");
    printf("[3] Cari Produk\n");
    printf("[4] Perbarui Stok Produk\n");
    printf("[5] Set Diskon Produk\n");
    printf("[6] Panduan\n");
    printf("[7] Keluar\n");
    printf("\n");
    printf("Silakan pilih opsi [1-7]: ");
}

int main() {
    int pilihan;
    #pragma omp_init_lock(&lock); // Initialize lock
    menuSelamatDatang();
    do {
        bersihkanLayar();
        menu();
        scanf("%d", &pilihan); getchar();
        switch (pilihan) {
            case 1:
                tambahProduk();
                break;
            case 2:
                tampilkanProduk();
                break;
            case 3:
                cariProduk();
                break;
            case 4:
                updateStokProduk();
                break;
            case 5:
                setDiskonProduk();
                break;
            case 6:
                tampilkanPanduan();
                break;
            case 7:
                printf("Keluar dari sistem.\n");
                break;
            default:
                printf("Pilihan tidak valid. Tekan enter untuk kembali ke menu utama...");
                getchar();
        }
    } while (pilihan != 7);

    #pragma omp_destroy_lock(&lock); // Destroy lock
    return 0;
}

