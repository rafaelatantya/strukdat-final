#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "QueueSystem.h"
#include "Benchmark.h"

using namespace std;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseLanjut() {
    cout << "\nTekan Enter untuk kembali ke menu utama...";
    string dummy;
    getline(cin, dummy);
}

string extractBenchmarkValue(const string& json, const string& key) {
    size_t keyPos = json.find("\"" + key + "\"");
    if (keyPos == string::npos) return "";
    size_t colonPos = json.find(":", keyPos);
    if (colonPos == string::npos) return "";
    size_t valStart = colonPos + 1;
    while (valStart < json.length() && (json[valStart] == ' ' || json[valStart] == '	')) valStart++;
    if (json[valStart] == '"') {
        size_t valEnd = json.find("\"", valStart + 1);
        if (valEnd == string::npos) return "";
        return json.substr(valStart + 1, valEnd - valStart - 1);
    } else {
        size_t valEnd = valStart;
        while (valEnd < json.length() && json[valEnd] != ',' && json[valEnd] != '}') valEnd++;
        return json.substr(valStart, valEnd - valStart);
    }
}


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
            { pauseLanjut(); continue; }
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
            sistem.setInteractiveMode(false);
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
        clearScreen();
        cout << "+---------------------------------------------------+\n";
        cout << "|        SISTEM ANTRIAN RUMAH SAKIT (MODULAR)       |\n";
        cout << "+---------------------------------------------------+\n";
        cout << "| 1. Registrasi Pasien Baru (Walk-in / Booking)     |\n";
        cout << "| 2. Check-in Pasien Terjadwal                      |\n";
        cout << "| 3. Panggil Antrian Berikutnya                     |\n";
        cout << "| 4. Cari Data Pasien                               |\n";
        cout << "| 5. Update Status Pasien secara Manual             |\n";
        cout << "| 6. Batalkan Antrian Pasien                        |\n";
        cout << "| 7. Tampilkan Semua Data Pasien                    |\n";
        cout << "| 8. Tampilkan Pasien Mengantri (Menunggu)          |\n";
        cout << "| 9. Tampilkan Pasien Selesai Pelayanan             |\n";
        cout << "| 10. Tampilkan Pasien Batal Pelayanan              |\n";
        cout << "| 11. Tampilkan Pasien Terjadwal (Belum Check-in)   |\n";
        cout << "| 12. Isi Dummy Data Awal                           |\n";
        cout << "| 13. Jalankan Benchmark Performa                   |\n";
        cout << "| 0. Keluar                                         |\n";
        cout << "+---------------------------------------------------+\n";
        cout << "Pilih menu: ";
        pilihan = inputAngka();

        if (pilihan == 1) {
            string id, nama, layanan, waktu, tanggal;
            int prioritas;

            if (!inputStringDenganBatal("ID Pasien (0 untuk batal)                : ", id)) { pauseLanjut(); continue; }
            if (!inputStringDenganBatal("Nama Pasien (0 untuk batal)              : ", nama)) { pauseLanjut(); continue; }
            if (!inputStringDenganBatal("Jenis Layanan (0 untuk batal)            : ", layanan)) { pauseLanjut(); continue; }

            prioritas = inputAngkaDenganBatal("Prioritas (1=Darurat, 2=Mendesak, 3=Prioritas Rentan, 4=Reguler, -1 untuk batal): ");
            if (prioritas == -1) {
                cout << "Input dibatalkan. Kembali ke menu utama.\n\n";
                { pauseLanjut(); continue; }
            }

            if (!inputStringDenganBatal("Waktu Datang (0=batal, - jika booking)   : ", waktu)) { pauseLanjut(); continue; }
            if (!inputStringDenganBatal("Tanggal Appointment (0 untuk batal)     : ", tanggal)) { pauseLanjut(); continue; }

            string errorMsg;
            if (sistem.insertPatient(id, nama, layanan, prioritas, waktu, tanggal, errorMsg)) {
                cout << "Pasien berhasil diregistrasi.\n\n";
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 2) {
            string id, waktu;
            if (!inputStringDenganBatal("Masukkan ID pasien (0 untuk batal): ", id)) { pauseLanjut(); continue; }
            if (!inputStringDenganBatal("Waktu Check-in (0=batal, - jika otomatis jam sekarang): ", waktu)) { pauseLanjut(); continue; }

            string errorMsg;
            if (sistem.checkInPatient(id, waktu, errorMsg)) {
                cout << "Pasien berhasil check-in dan masuk ke antrian aktif.\n\n";
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 3) {
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
        else if (pilihan == 4) {
            string id;
            if (!inputStringDenganBatal("Masukkan ID pasien (0 untuk batal): ", id)) { pauseLanjut(); continue; }
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
        else if (pilihan == 5) {
            string id;
            if (!inputStringDenganBatal("Masukkan ID pasien (0 untuk batal): ", id)) { pauseLanjut(); continue; }

            int statusBaru = inputAngkaDenganBatal(
                "Status baru (0=Menunggu, 1=Dipanggil, 2=Selesai, 3=Batal, 4=Terjadwal, -1 untuk batal): "
            );

            if (statusBaru == -1) {
                cout << "Input dibatalkan. Kembali ke menu utama.\n\n";
                { pauseLanjut(); continue; }
            }

            string errorMsg;
            if (sistem.updateStatus(id, statusBaru, errorMsg)) {
                cout << "Status pasien berhasil diupdate.\n\n";
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 6) {
            string id;
            if (!inputStringDenganBatal("Masukkan ID pasien yang dibatalkan (0 untuk batal): ", id)) { pauseLanjut(); continue; }
            string errorMsg;
            if (sistem.deleteAntrian(id, errorMsg)) {
                cout << "Pasien dibatalkan dari antrian.\n\n";
            } else {
                cout << "Gagal: " << errorMsg << "\n\n";
            }
        }
        else if (pilihan == 7) {
            sistem.tampilkanSemuaData();
        }
        else if (pilihan == 8) {
            sistem.tampilkanAntrianAktif();
        }
        else if (pilihan == 9) {
            sistem.tampilkanPasienSelesai();
        }
        else if (pilihan == 10) {
            sistem.tampilkanPasienBatal();
        }
        else if (pilihan == 11) {
            sistem.tampilkanPasienTerjadwal();
        }
        else if (pilihan == 12) {
            sistem.dummyData();
            cout << "Dummy data berhasil diproses.\n\n";
        }
        else if (pilihan == 13) {
            int jumlah = inputAngkaDenganBatal("Masukkan jumlah data simulasi (-1 untuk batal): ");
            if (jumlah == -1) {
                cout << "Input dibatalkan.\n";
                { pauseLanjut(); continue; }
            }
            runFullBenchmark(jumlah, "benchmark_results.json");
            
            ifstream file("benchmark_results.json");
            if (file.is_open()) {
                stringstream buffer;
                buffer << file.rdbuf();
                string jsonStr = buffer.str();
                file.close();

                cout << "\n=== Hasil Benchmark ===\n";
                cout << "Skala (Jumlah Data) : " << extractBenchmarkValue(jsonStr, "scale") << "\n";
                cout << "Waktu Eksekusi      : " << extractBenchmarkValue(jsonStr, "totalTimeMs") << " ms\n";
                cout << "Penggunaan Memori   : " << extractBenchmarkValue(jsonStr, "memoryUsageMB") << " MB\n";
                cout << "=======================\n";
            }
        }
        else if (pilihan == 0) {
            cout << "Program selesai.\n";
        }
        else {
            cout << "Pilihan tidak valid.\n\n";
        }
        if (pilihan != 0) {
            pauseLanjut();
        }
    } while (pilihan != 0);

    return 0;
}
