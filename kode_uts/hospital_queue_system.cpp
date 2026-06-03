#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <limits>

using namespace std;

enum PriorityLevel {
    DARURAT = 1,
    MENDESAK = 2,
    PRIORITAS_RENTAN = 3,
    REGULER = 4
};

enum StatusLayanan {
    MENUNGGU = 0,
    DIPANGGIL = 1,
    SELESAI = 2,
    BATAL = 3
};

struct Patient {
    string id;
    string nama;
    string layanan;
    int prioritas;
    int nomorAntrian;
    string waktuDatang;
    StatusLayanan status;
};

struct ComparePatient {
    bool operator()(const Patient& a, const Patient& b) const {
        if (a.prioritas == b.prioritas) {
            return a.nomorAntrian > b.nomorAntrian;
        }
        return a.prioritas > b.prioritas;
    }
};

class SistemAntrianRS {
private:
    priority_queue<Patient, vector<Patient>, ComparePatient> antrian;
    unordered_map<string, Patient> dataPasien;
    int nomorBerikutnya;
    string namaFileData;
    string namaFileBenchmark;

public:
    SistemAntrianRS(
        string fileData = "data_pasien_rs.txt",
        string fileBenchmark = "benchmark_minggu7.txt"
    ) {
        nomorBerikutnya = 1;
        namaFileData = fileData;
        namaFileBenchmark = fileBenchmark;
        loadFromFile();
    }

    bool prioritasValid(int p) {
        return p >= DARURAT && p <= REGULER;
    }

    bool statusValid(int s) {
        return s >= MENUNGGU && s <= BATAL;
    }

    string prioritasToString(int p) {
        switch (p) {
            case DARURAT: return "Darurat";
            case MENDESAK: return "Mendesak";
            case PRIORITAS_RENTAN: return "Prioritas Rentan";
            case REGULER: return "Reguler";
            default: return "Tidak diketahui";
        }
    }

    string statusToString(StatusLayanan s) {
        switch (s) {
            case MENUNGGU: return "Menunggu";
            case DIPANGGIL: return "Dipanggil";
            case SELESAI: return "Selesai";
            case BATAL: return "Batal";
            default: return "Tidak diketahui";
        }
    }

    void tampilkanSatuPasien(const Patient& p) {
        cout << "ID Pasien      : " << p.id << endl;
        cout << "Nama Pasien    : " << p.nama << endl;
        cout << "Jenis Layanan  : " << p.layanan << endl;
        cout << "Prioritas      : " << prioritasToString(p.prioritas) << endl;
        cout << "Nomor Antrian  : " << p.nomorAntrian << endl;
        cout << "Waktu Datang   : " << p.waktuDatang << endl;
        cout << "Status         : " << statusToString(p.status) << endl;
    }

    void insertPatient(string id, string nama, string layanan, int prioritas, string waktuDatang) {
        if (id.empty() || nama.empty() || layanan.empty() || waktuDatang.empty()) {
            cout << "Data pasien tidak boleh kosong.\n\n";
            return;
        }

        if (!prioritasValid(prioritas)) {
            cout << "Prioritas tidak valid.\n\n";
            return;
        }

        if (dataPasien.find(id) != dataPasien.end()) {
            cout << "ID pasien sudah ada.\n\n";
            return;
        }

        Patient p;
        p.id = id;
        p.nama = nama;
        p.layanan = layanan;
        p.prioritas = prioritas;
        p.nomorAntrian = nomorBerikutnya++;
        p.waktuDatang = waktuDatang;
        p.status = MENUNGGU;

        antrian.push(p);
        dataPasien[id] = p;
        saveToFile();

        cout << "Pasien berhasil ditambahkan ke antrian.\n";
        cout << "Nomor antrian: " << p.nomorAntrian << "\n\n";
    }

    void panggilAntrianBerikutnya() {
        while (!antrian.empty()) {
            Patient p = antrian.top();
            antrian.pop();

            if (dataPasien[p.id].status == MENUNGGU) {
                dataPasien[p.id].status = DIPANGGIL;
                saveToFile();

                cout << "\n=== Pasien Dipanggil ===\n";
                tampilkanSatuPasien(dataPasien[p.id]);
                cout << endl;
                return;
            }
        }

        cout << "Antrian kosong.\n\n";
    }

    void searchPatient(string id) {
        if (dataPasien.find(id) != dataPasien.end()) {
            cout << "\n=== Data Pasien Ditemukan ===\n";
            tampilkanSatuPasien(dataPasien[id]);
            cout << endl;
        } else {
            cout << "Pasien dengan ID " << id << " tidak ditemukan.\n\n";
        }
    }

    void updateStatus(string id, int statusBaru) {
        if (!statusValid(statusBaru)) {
            cout << "Status tidak valid.\n\n";
            return;
        }

        if (dataPasien.find(id) == dataPasien.end()) {
            cout << "Pasien tidak ditemukan.\n\n";
            return;
        }

        StatusLayanan statusSekarang = dataPasien[id].status;
        StatusLayanan statusTujuan = (StatusLayanan)statusBaru;

        if (statusSekarang == MENUNGGU && statusTujuan == DIPANGGIL) {
            dataPasien[id].status = DIPANGGIL;
            saveToFile();
            cout << "Status pasien berhasil diupdate.\n\n";
        }
        else if (statusSekarang == DIPANGGIL && statusTujuan == SELESAI) {
            dataPasien[id].status = SELESAI;
            saveToFile();
            cout << "Status pasien berhasil diupdate.\n\n";
        }
        else if (statusSekarang == MENUNGGU && statusTujuan == BATAL) {
            dataPasien[id].status = BATAL;
            saveToFile();
            cout << "Status pasien berhasil diupdate.\n\n";
        }
        else {
            cout << "Perubahan status tidak diperbolehkan.\n";
            cout << "Aturan status:\n";
            cout << "- MENUNGGU -> DIPANGGIL\n";
            cout << "- DIPANGGIL -> SELESAI\n";
            cout << "- MENUNGGU -> BATAL\n\n";
        }
    }

    void deleteAntrian(string id) {
        if (dataPasien.find(id) == dataPasien.end()) {
            cout << "Pasien tidak ditemukan.\n\n";
            return;
        }

        if (dataPasien[id].status != MENUNGGU) {
            cout << "Hanya pasien dengan status MENUNGGU yang bisa dibatalkan.\n\n";
            return;
        }

        dataPasien[id].status = BATAL;
        saveToFile();
        cout << "Pasien dibatalkan dari antrian.\n\n";
    }

    void tampilkanSemuaData() {
        cout << "\n=== Seluruh Data Pasien ===\n";

        if (dataPasien.empty()) {
            cout << "Belum ada data pasien.\n\n";
            return;
        }

        for (auto item : dataPasien) {
            tampilkanSatuPasien(item.second);
            cout << "-----------------------------\n";
        }
        cout << endl;
    }

    void tampilkanAntrianAktif() {
        cout << "\n=== Pasien Menunggu ===\n";

        bool ada = false;
        for (auto item : dataPasien) {
            if (item.second.status == MENUNGGU) {
                tampilkanSatuPasien(item.second);
                cout << "-----------------------------\n";
                ada = true;
            }
        }

        if (!ada) {
            cout << "Tidak ada pasien yang sedang menunggu.\n";
        }

        cout << endl;
    }

    void dummyData() {
        if (dataPasien.find("P001") == dataPasien.end())
            insertPatient("P001", "Andi", "Poli Umum", REGULER, "08:00");
        if (dataPasien.find("P002") == dataPasien.end())
            insertPatient("P002", "Budi", "Laboratorium", REGULER, "08:05");
        if (dataPasien.find("P003") == dataPasien.end())
            insertPatient("P003", "Citra", "UGD", DARURAT, "08:07");
        if (dataPasien.find("P004") == dataPasien.end())
            insertPatient("P004", "Dewi", "Poli Geriatri", PRIORITAS_RENTAN, "08:10");
        if (dataPasien.find("P005") == dataPasien.end())
            insertPatient("P005", "Eko", "Poli Penyakit Dalam", MENDESAK, "08:12");
    }

    void saveToFile() {
        ofstream file(namaFileData.c_str());

        if (!file.is_open()) {
            cout << "Gagal membuka file penyimpanan.\n";
            return;
        }

        file << nomorBerikutnya << endl;

        for (auto item : dataPasien) {
            Patient p = item.second;
            file << p.id << "|"
                 << p.nama << "|"
                 << p.layanan << "|"
                 << p.prioritas << "|"
                 << p.nomorAntrian << "|"
                 << p.waktuDatang << "|"
                 << (int)p.status << endl;
        }

        file.close();
    }

    void loadFromFile() {
        ifstream file(namaFileData.c_str());

        if (!file.is_open()) {
            return;
        }

        dataPasien.clear();
        while (!antrian.empty()) {
            antrian.pop();
        }

        string line;
        if (getline(file, line)) {
            stringstream ss(line);
            ss >> nomorBerikutnya;
        }

        while (getline(file, line)) {
            stringstream ss(line);
            string token;
            vector<string> data;

            while (getline(ss, token, '|')) {
                data.push_back(token);
            }

            if (data.size() == 7) {
                Patient p;
                p.id = data[0];
                p.nama = data[1];
                p.layanan = data[2];
                p.prioritas = stoi(data[3]);
                p.nomorAntrian = stoi(data[4]);
                p.waktuDatang = data[5];
                p.status = (StatusLayanan)stoi(data[6]);

                dataPasien[p.id] = p;

                if (p.status == MENUNGGU) {
                    antrian.push(p);
                }
            }
        }

        file.close();
    }

    void eksperimenAwal(int jumlahData) {
        if (jumlahData <= 0) {
            cout << "Jumlah data harus lebih dari 0.\n\n";
            return;
        }

        priority_queue<Patient, vector<Patient>, ComparePatient> antrianUji;
        unordered_map<string, Patient> dataUji;

        auto startInsert = chrono::high_resolution_clock::now();

        for (int i = 1; i <= jumlahData; i++) {
            Patient p;
            p.id = "SIM" + to_string(i);
            p.nama = "Pasien" + to_string(i);
            p.layanan = "Poli Umum";
            p.prioritas = (i % 4) + 1;
            p.nomorAntrian = i;
            p.waktuDatang = "08:00";
            p.status = MENUNGGU;

            antrianUji.push(p);
            dataUji[p.id] = p;
        }

        auto endInsert = chrono::high_resolution_clock::now();

        auto startSearch = chrono::high_resolution_clock::now();
        string target = "SIM" + to_string(jumlahData / 2);
        bool ditemukan = (dataUji.find(target) != dataUji.end());
        auto endSearch = chrono::high_resolution_clock::now();

        auto startCall = chrono::high_resolution_clock::now();
        if (!antrianUji.empty()) {
            Patient p = antrianUji.top();
            antrianUji.pop();
        }
        auto endCall = chrono::high_resolution_clock::now();

        long long insertTime = chrono::duration_cast<chrono::microseconds>(endInsert - startInsert).count();
        long long searchTime = chrono::duration_cast<chrono::microseconds>(endSearch - startSearch).count();
        long long callTime   = chrono::duration_cast<chrono::microseconds>(endCall - startCall).count();

        cout << "\n=== Eksperimen Awal Minggu 7 ===\n";
        cout << "Jumlah data pasien : " << jumlahData << endl;
        cout << "Waktu insert       : " << insertTime << " microseconds" << endl;
        cout << "Waktu search       : " << searchTime << " microseconds" << endl;
        cout << "Waktu call next    : " << callTime << " microseconds" << endl;
        cout << "Target search      : " << target << " -> "
             << (ditemukan ? "Ditemukan" : "Tidak ditemukan") << endl << endl;

        simpanBenchmark(jumlahData, insertTime, searchTime, callTime);
    }

    void simpanBenchmark(int jumlahData, long long insertTime, long long searchTime, long long callTime) {
        ofstream file(namaFileBenchmark.c_str(), ios::app);

        if (!file.is_open()) {
            cout << "Gagal membuka file benchmark.\n";
            return;
        }

        file << "Jumlah data: " << jumlahData
             << ", Insert: " << insertTime
             << " us, Search: " << searchTime
             << " us, Call: " << callTime << " us\n";

        file.close();
    }

    friend int main();
};

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

int main() {
    SistemAntrianRS sistem;
    int pilihan;

    do {
        cout << "=====================================\n";
        cout << " SISTEM ANTRIAN RUMAH SAKIT (MINGGU 7)\n";
        cout << "=====================================\n";
        cout << "1. Tambah pasien ke antrian\n";
        cout << "2. Panggil antrian berikutnya\n";
        cout << "3. Cari data pasien\n";
        cout << "4. Update status pasien\n";
        cout << "5. Batalkan antrian\n";
        cout << "6. Tampilkan semua data pasien\n";
        cout << "7. Isi dummy data\n";
        cout << "8. Tampilkan pasien menunggu\n";
        cout << "9. Jalankan eksperimen awal\n";
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

            sistem.insertPatient(id, nama, layanan, prioritas, waktu);
        }
        else if (pilihan == 2) {
            sistem.panggilAntrianBerikutnya();
        }
        else if (pilihan == 3) {
            string id;
            if (!inputStringDenganBatal("Masukkan ID pasien (0 untuk batal): ", id)) continue;
            sistem.searchPatient(id);
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

            sistem.updateStatus(id, statusBaru);
        }
        else if (pilihan == 5) {
            string id;
            if (!inputStringDenganBatal("Masukkan ID pasien yang dibatalkan (0 untuk batal): ", id)) continue;
            sistem.deleteAntrian(id);
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
            sistem.eksperimenAwal(jumlah);
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