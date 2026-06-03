#include <iostream>
#include <string>
#include <limits>
#include "QueueSystem.h"
#include "Benchmark.h"

using namespace std;

int inputAngka() {
    int x;
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Input harus angka. Coba lagi: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return x;
}

bool inputStringDenganBatal(const string& prompt, string& hasil) {
    cout << prompt;
    getline(cin, hasil);

    if (hasil == "0") {
        cout << "Input dibatalkan. Kembali ke menu utama.\n\n";
        return false;
    }

    return true;
}

int inputAngkaDenganBatal(const string& prompt) {
    int x;
    while (true) {
        cout << prompt;
        if (!(cin >> x)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Input harus angka.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return x;
    }
}

int main(int argc, char* argv[]) {
    SistemAntrianRS sistem;

    // cek argumen CLI
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--json" && i + 1 < argc) {
            string jsonPayload = argv[i + 1];
            string response = sistem.executeJsonCommand(jsonPayload);
            cout << response << endl;
            return 0;
        }
        else if (arg == "--benchmark") {
            int scale = 10000;
            if (i + 1 < argc) {
                try {
                    scale = stoi(argv[i + 1]);
                } catch (...) {
                    // klo gagal, pake default
                }
            }
            runFullBenchmark(scale, "benchmark_results.json");
            return 0;
        }
    }

    // jalanin menu interaktif di terminal
    int pilihan;
    do {
        cout << "=====================================\n";
        cout << " SISTEM ANTRIAN RUMAH SAKIT (MODULAR)\n";
        cout << "=====================================\n";
        cout << "1. Tambah pasien ke antrian\n";
        cout << "2. Panggil antrian berikutnya\n";
        cout << "3. Cari data pasien\n";
        cout << "4. Update status pasien\n";
        cout << "5. Batalkan antrian\n";
        cout << "6. Tampilkan semua data pasien\n";
        cout << "7. Isi dummy data\n";
        cout << "8. Tampilkan pasien menunggu\n";
        cout << "9. Jalankan benchmark performa\n";
        cout << "0. Keluar\n";
        cout << "Pilih menu: ";
        pilihan = inputAngka();

        if (pilihan == 1) {
            string id, nama, layanan, waktu;
            int prioritas;

            if (!inputStringDenganBatal("ID Pasien (0 untuk batal)        : ", id)) continue;
            if (!inputStringDenganBatal("Nama Pasien (0 untuk batal)      : ", nama)) continue;
            if (!inputStringDenganBatal("Jenis Layanan (0 untuk batal)    : ", layanan)) continue;

            prioritas = inputAngkaDenganBatal("Prioritas (1=Darurat, 2=Mendesak, 3=Prioritas Rentan, 4=Reguler, -1 untuk batal): ");
            if (prioritas == -1) {
                cout << "Input dibatalkan. Kembali ke menu utama.\n\n";
                continue;
            }

            if (!inputStringDenganBatal("Waktu Kedatangan (0 untuk batal) : ", waktu)) continue;

            string errorMsg;
            if (sistem.insertPatient(id, nama, layanan, prioritas, waktu, errorMsg)) {
                cout << "Pasien berhasil ditambahkan ke antrian.\n\n";
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 2) {
            Patient p;
            string errorMsg;
            if (sistem.panggilAntrianBerikutnya(p, errorMsg)) {
                cout << "\n=== Pasien Dipanggil ===\n";
                sistem.tampilkanSatuPasien(p);
                cout << endl;
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 3) {
            string id;
            if (!inputStringDenganBatal("Masukkan ID pasien (0 untuk batal): ", id)) continue;
            Patient p;
            string errorMsg;
            if (sistem.searchPatient(id, p, errorMsg)) {
                cout << "\n=== Data Pasien Ditemukan ===\n";
                sistem.tampilkanSatuPasien(p);
                cout << endl;
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 4) {
            string id;
            if (!inputStringDenganBatal("Masukkan ID pasien (0 untuk batal): ", id)) continue;

            int statusBaru = inputAngkaDenganBatal(
                "Status baru (1=Dipanggil, 2=Selesai, 3=Batal, -1 untuk batal): "
            );

            if (statusBaru == -1) {
                cout << "Input dibatalkan. Kembali ke menu utama.\n\n";
                continue;
            }

            string errorMsg;
            if (sistem.updateStatus(id, statusBaru, errorMsg)) {
                cout << "Status pasien berhasil diupdate.\n\n";
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 5) {
            string id;
            if (!inputStringDenganBatal("Masukkan ID pasien yang dibatalkan (0 untuk batal): ", id)) continue;
            string errorMsg;
            if (sistem.deleteAntrian(id, errorMsg)) {
                cout << "Pasien dibatalkan dari antrian.\n\n";
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 6) {
            sistem.tampilkanSemuaData();
        }
        else if (pilihan == 7) {
            sistem.dummyData();
            cout << "Dummy data berhasil diproses.\n\n";
        }
        else if (pilihan == 8) {
            sistem.tampilkanAntrianAktif();
        }
        else if (pilihan == 9) {
            int jumlah = inputAngkaDenganBatal("Masukkan jumlah data simulasi (-1 untuk batal): ");
            if (jumlah == -1) {
                cout << "Input dibatalkan. Kembali ke menu utama.\n\n";
                continue;
            }
            runFullBenchmark(jumlah, "benchmark_results.json");
        }
        else if (pilihan == 0) {
            cout << "Program selesai.\n";
        }
        else {
            cout << "Pilihan tidak valid.\n\n";
        }

    } while (pilihan != 0);

    return 0;
}
