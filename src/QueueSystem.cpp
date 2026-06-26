#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <regex>
#include "QueueSystem.h"

using namespace std;

// helper buat dapet jam sekarang format HH:MM
string getCurrentTimeStr() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
    return string(buffer);
}

// helper buat dapet tanggal sekarang format YYYY-MM-DD
string getCurrentDateStr() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return string(buffer);
}

// helper buat dapet tanggal besok format YYYY-MM-DD
string getTomorrowDateStr() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time(&rawtime);
    rawtime += 24 * 3600; // tambah 1 hari
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return string(buffer);
}

// helper untuk membandingkan pasien untuk tampilan tabel/list
bool comparePatientsForDisplay(const Patient& a, const Patient& b) {
    // 1. Urutkan berdasarkan kelompok status: DIPANGGIL -> MENUNGGU -> TERJADWAL -> SELESAI -> BATAL
    auto getStatusRank = [](StatusLayanan s) {
        if (s == DIPANGGIL) return 0;
        if (s == MENUNGGU) return 1;
        if (s == TERJADWAL) return 2;
        if (s == SELESAI) return 3;
        return 4; // BATAL
    };
    
    int rankA = getStatusRank(a.status);
    int rankB = getStatusRank(b.status);
    if (rankA != rankB) {
        return rankA < rankB;
    }
    
    // 2. Jika status sama:
    if (a.status == MENUNGGU || a.status == DIPANGGIL) {
        // Urutkan berdasarkan prioritas (1 = Darurat s.d. 4 = Reguler)
        if (a.prioritas != b.prioritas) {
            return a.prioritas < b.prioritas;
        }
        // Jika prioritas sama, urutkan berdasarkan nomor antrian (FIFO)
        return std::stoi(a.nomorAntrian) < std::stoi(b.nomorAntrian);
    }
    else if (a.status == TERJADWAL) {
        // Urutkan berdasarkan tanggal booking
        if (a.tanggal != b.tanggal) {
            return a.tanggal < b.tanggal;
        }
        // Jika tanggal sama, urutkan berdasarkan ID
        return a.id < b.id;
    }
    else {
        // SELESAI atau BATAL: urutkan berdasarkan tanggal, lalu nomor antrian
        if (a.tanggal != b.tanggal) {
            return a.tanggal < b.tanggal;
        }
        return std::stoi(a.nomorAntrian) < std::stoi(b.nomorAntrian);
    }
}

// potong spasi/kutip kiri kanan
string SistemAntrianRS::trim(const string& str) {
    if (str.empty()) return str;
    size_t first = str.find_first_not_of(" \t\r\n\"");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n\"");
    return str.substr(first, (last - first + 1));
}

// ambil value dari key json sederhana
string SistemAntrianRS::extractJsonValue(const string& json, const string& key) {
    size_t keyPos = json.find("\"" + key + "\"");
    if (keyPos == string::npos) {
        keyPos = json.find(key);
        if (keyPos == string::npos) return "";
    }
    
    size_t colonPos = json.find(":", keyPos);
    if (colonPos == string::npos) return "";
    
    size_t valStart = colonPos + 1;
    while (valStart < json.length() && (json[valStart] == ' ' || json[valStart] == '\t' || json[valStart] == '\r' || json[valStart] == '\n')) {
        valStart++;
    }
    
    if (valStart >= json.length()) return "";
    
    if (json[valStart] == '"') {
        size_t valEnd = json.find("\"", valStart + 1);
        if (valEnd == string::npos) return "";
        return json.substr(valStart + 1, valEnd - valStart - 1);
    } else {
        size_t valEnd = valStart;
        while (valEnd < json.length() && json[valEnd] != ',' && json[valEnd] != '}' && json[valEnd] != ']' && json[valEnd] != ' ' && json[valEnd] != '\n' && json[valEnd] != '\r') {
            valEnd++;
        }
        return json.substr(valStart, valEnd - valStart);
    }
}

bool SistemAntrianRS::isValidDate(const string& date) {
    regex pattern("^\\d{4}-\\d{2}-\\d{2}$");
    return regex_match(date, pattern);
}

bool SistemAntrianRS::isValidTime(const string& timeStr) {
    if (timeStr == "-") return true;
    regex pattern("^([01]\\d|2[0-3]):([0-5]\\d)$");
    return regex_match(timeStr, pattern);
}

bool SistemAntrianRS::isPolyBusy(const string& layanan) {
    for (const auto& [id, p] : dataPasien) {
        if (p.layanan == layanan && p.status == DIPANGGIL) return true;
    }
    return false;
}

bool SistemAntrianRS::isHighestPriorityInPoly(const string& id, Patient& outHighest) {
    if (dataPasien.find(id) == dataPasien.end()) return false;
    Patient target = dataPasien[id];
    Patient highest = target;
    ComparePatient comp;

    for (const auto& [key, p] : dataPasien) {
        if (p.status == MENUNGGU && p.id != target.id) {
            if (comp(highest, p)) {
                highest = p;
            }
        }
    }
    outHighest = highest;
    return highest.id == target.id;
}

void SistemAntrianRS::rebuildHeap() {
    while (!antrian.empty()) {
        antrian.pop();
    }
    for (const auto& [key, p] : dataPasien) {
        if (p.status == MENUNGGU) {
            antrian.push(p);
        }
    }
}

SistemAntrianRS::SistemAntrianRS(string fileData, string fileBenchmark) {
    nomorBerikutnya = 1;
    namaFileData = fileData;
    namaFileBenchmark = fileBenchmark;
    isInteractiveMode = true;
    loadFromFile();
}

void SistemAntrianRS::setInteractiveMode(bool mode) {
    isInteractiveMode = mode;
}

bool SistemAntrianRS::prioritasValid(int p) {
    return p >= DARURAT && p <= REGULER;
}

bool SistemAntrianRS::statusValid(int s) {
    return s >= MENUNGGU && s <= BATAL;
}

string SistemAntrianRS::prioritasToString(int p) {
    switch (p) {
        case DARURAT: return "Darurat";
        case MENDESAK: return "Mendesak";
        case PRIORITAS_RENTAN: return "Prioritas Rentan";
        case REGULER: return "Reguler";
        default: return "Tidak diketahui";
    }
}

string SistemAntrianRS::statusToString(StatusLayanan s) {
    switch (s) {
        case MENUNGGU: return "Menunggu";
        case DIPANGGIL: return "Dipanggil";
        case SELESAI: return "Selesai";
        case BATAL: return "Batal";
        case TERJADWAL: return "Terjadwal";
        default: return "Tidak diketahui";
    }
}

void SistemAntrianRS::tampilkanSatuPasien(const Patient& p) {
    cout << "ID Pasien      : " << p.id << endl;
    cout << "Nama Pasien    : " << p.nama << endl;
    cout << "Jenis Layanan  : " << p.layanan << endl;
    cout << "Prioritas      : " << prioritasToString(p.prioritas) << endl;
    cout << "Nomor Antrian  : " << p.nomorAntrian << endl;
    cout << "Waktu Datang   : " << p.waktuDatang << endl;
    cout << "Waktu Dipanggil: " << p.waktuDipanggil << endl;
    cout << "Tanggal        : " << p.tanggal << endl;
    cout << "Status         : " << statusToString(p.status) << endl;
}

void SistemAntrianRS::tampilkanTabelPasien(const vector<Patient>& listPasien) {
    if (listPasien.empty()) {
        cout << "Tidak ada data pasien yang sesuai.\n";
        return;
    }

    // copy vector biar bisa disort
    vector<Patient> sortedList = listPasien;
    sort(sortedList.begin(), sortedList.end(), comparePatientsForDisplay);
    
    cout << "+------+----------------------+----------------------+------------------+------------------+--------+--------------+\n";
    cout << "| " << left << setw(4) << "ID"
         << " | " << setw(20) << "Nama Pasien"
         << " | " << setw(20) << "Layanan"
         << " | " << setw(16) << "Prioritas"
         << " | " << setw(16) << "Status"
         << " | " << setw(6) << "Datang"
         << " | " << setw(12) << "Tanggal" << " |\n";
    cout << "+------+----------------------+----------------------+------------------+------------------+--------+--------------+\n";

    for (const auto& p : sortedList) {
        cout << "| " << left << setw(4) << p.id
             << " | " << setw(20) << p.nama.substr(0, 20)
             << " | " << setw(20) << p.layanan.substr(0, 20)
             << " | " << setw(16) << prioritasToString(p.prioritas).substr(0, 16)
             << " | " << setw(16) << statusToString(p.status).substr(0, 16)
             << " | " << setw(6) << p.waktuDatang
             << " | " << setw(12) << p.tanggal.substr(0, 12) << " |\n";
    }
    cout << "+------+----------------------+----------------------+------------------+------------------+--------+--------------+\n";
}

bool SistemAntrianRS::insertPatient(string id, string nama, string layanan, int prioritas, string waktuDatang, string tanggal, string& errorMsg) {
    if (id.empty()) {
        id = "P" + to_string(nomorBerikutnya);
    }

    if (id.empty() || nama.empty() || layanan.empty() || tanggal.empty()) {
        errorMsg = "Data pasien tidak boleh ada yang kosong.";
        return false;
    }

    if (!isValidDate(tanggal)) {
        errorMsg = "Format tanggal tidak valid (harus YYYY-MM-DD).";
        return false;
    }

    if (!waktuDatang.empty() && waktuDatang != "-" && !isValidTime(waktuDatang)) {
        errorMsg = "Format waktu datang tidak valid (harus HH:MM atau -).";
        return false;
    }

    if (!waktuDatang.empty() && waktuDatang != "-") {
        if (tanggal != getCurrentDateStr()) {
            errorMsg = "Tanggal walk-in harus hari ini (" + getCurrentDateStr() + ").";
            return false;
        }
    }

    if (!prioritasValid(prioritas)) {
        errorMsg = "Prioritas tidak valid (harus 1-4).";
        return false;
    }

    if (dataPasien.find(id) != dataPasien.end()) {
        errorMsg = "ID pasien sudah ada di database.";
        return false;
    }

    Patient p;
    p.id = id;
    p.nama = nama;
    p.layanan = layanan;
    p.prioritas = prioritas;
    p.nomorAntrian = to_string(nomorBerikutnya++) + "N";
    p.tanggal = tanggal;
    p.waktuDipanggil = "-";

    if (waktuDatang == "-" || waktuDatang.empty()) {
        p.status = TERJADWAL;
        p.waktuDatang = "-";
    } else {
        p.status = MENUNGGU;
        p.waktuDatang = waktuDatang;
        antrian.push(p);
    }

    dataPasien[id] = p;
    saveToFile();

    if (isInteractiveMode) {
        cout << "\n>>> [REGISTRASI] Pasien baru berhasil didaftarkan:\n";
        cout << "    ID: " << p.id << " | Nama: " << p.nama << " | Layanan: " << p.layanan << "\n";
        if (p.status == TERJADWAL) {
            cout << "    Status: TERJADWAL (Booking untuk tanggal: " << p.tanggal << ")\n";
        } else {
            cout << "    Status: MENUNGGU (Masuk antrian aktif dengan Prioritas: " << prioritasToString(p.prioritas) << ", No. Antrian: #" << p.nomorAntrian << ")\n";
        }
    }

    return true;
}

bool SistemAntrianRS::checkInPatient(string id, string waktuDatang, string& errorMsg) {
    auto it = dataPasien.find(id);
    if (it == dataPasien.end()) {
        errorMsg = "Pasien tidak ditemukan.";
        return false;
    }

    if (!waktuDatang.empty() && waktuDatang != "-" && !isValidTime(waktuDatang)) {
        errorMsg = "Format waktu datang tidak valid (harus HH:MM atau -).";
        return false;
    }

    if (it->second.status != TERJADWAL) {
        errorMsg = "Hanya pasien dengan status TERJADWAL yang bisa check-in.";
        return false;
    }

    it->second.status = MENUNGGU;
    if (waktuDatang == "-" || waktuDatang.empty()) {
        it->second.waktuDatang = getCurrentTimeStr();
    } else {
        it->second.waktuDatang = waktuDatang;
    }

    antrian.push(it->second);
    saveToFile();

    if (isInteractiveMode) {
        cout << "\n>>> [CHECK-IN] Pasien check-in fisik berhasil:\n";
        cout << "    ID: " << it->second.id << " | Nama: " << it->second.nama << "\n";
        cout << "    Status berubah: TERJADWAL -> MENUNGGU (Masuk Antrian Aktif)\n";
        cout << "    Prioritas: " << prioritasToString(it->second.prioritas) << " | No. Antrian: #" << it->second.nomorAntrian << "\n";
    }

    return true;
}

bool SistemAntrianRS::panggilAntrianBerikutnya(Patient& outPatient, string& errorMsg) {
    vector<Patient> skipped;
    bool found = false;

    while (!antrian.empty()) {
        Patient p = antrian.top();
        antrian.pop();

        if (dataPasien[p.id].status == MENUNGGU) {
            if (isPolyBusy(dataPasien[p.id].layanan)) {
                skipped.push_back(p);
            } else {
                dataPasien[p.id].status = DIPANGGIL;
                dataPasien[p.id].waktuDipanggil = getCurrentTimeStr();
                saveToFile();
                outPatient = dataPasien[p.id];
                found = true;
                break;
            }
        }
    }

    for (const Patient& p : skipped) {
        antrian.push(p);
    }

    if (found) {
        if (isInteractiveMode) {
            cout << "\n>>> [PANGGIL ANTRIAN] Memanggil pasien berikutnya:\n";
            cout << "    ID: " << outPatient.id << " | Nama: " << outPatient.nama << "\n";
            cout << "    Status berubah: MENUNGGU -> DIPANGGIL\n";
            cout << "    Prioritas: " << prioritasToString(outPatient.prioritas) << " | No. Antrian: #" << outPatient.nomorAntrian << "\n";
        }
        return true;
    }

    errorMsg = "Antrian kosong atau poli untuk pasien teratas sedang melayani pasien lain.";
    return false;
}

bool SistemAntrianRS::searchPatient(string id, Patient& outPatient, string& errorMsg) {
    auto it = dataPasien.find(id);
    if (it != dataPasien.end()) {
        outPatient = it->second;
        return true;
    }
    errorMsg = "Pasien dengan ID " + id + " tidak ditemukan.";
    return false;
}

bool SistemAntrianRS::updateStatus(string id, int statusBaru, bool force, string& errorMsg) {
    if (dataPasien.find(id) == dataPasien.end()) {
        errorMsg = "Pasien tidak ditemukan.";
        return false;
    }
    if (!statusValid(statusBaru)) {
        errorMsg = "Status tidak valid.";
        return false;
    }

    Patient& p = dataPasien[id];
    StatusLayanan statusSekarang = p.status;
    StatusLayanan statusTujuan = static_cast<StatusLayanan>(statusBaru);

    if (statusSekarang == statusTujuan) {
        errorMsg = "Status pasien sudah " + statusToString(statusTujuan) + ".";
        return false;
    }

    if (statusSekarang == MENUNGGU && statusTujuan == DIPANGGIL) {
        if (isPolyBusy(p.layanan)) {
            errorMsg = "Poli " + p.layanan + " sedang melayani pasien lain.";
            return false;
        }
        if (!force) {
            Patient highest;
            if (!isHighestPriorityInPoly(id, highest)) {
                errorMsg = "WARNING_PRIORITY:" + highest.nama + " (Antrian #" + highest.nomorAntrian + ")";
                return false;
            }
        }
        
        p.status = DIPANGGIL;
        p.waktuDipanggil = getCurrentTimeStr();
        saveToFile();
        rebuildHeap();
        return true;
    }
    else if (statusSekarang == DIPANGGIL && statusTujuan == SELESAI) {
        p.status = SELESAI;
        saveToFile();
        rebuildHeap();
        if (isInteractiveMode) {
            cout << "\n>>> [UPDATE STATUS] Pelayanan pasien selesai:\n";
            cout << "    ID: " << id << " | Nama: " << p.nama << "\n";
            cout << "    Status berubah: DIPANGGIL -> SELESAI\n";
        }
        return true;
    }
    else if (statusSekarang == DIPANGGIL && statusTujuan == BATAL) {
        p.status = BATAL;
        saveToFile();
        rebuildHeap();
        if (isInteractiveMode) {
            cout << "\n>>> [BATAL ANTRIAN] Pelayanan pasien dibatalkan:\n";
            cout << "    ID: " << id << " | Nama: " << p.nama << "\n";
            cout << "    Status berubah: DIPANGGIL -> BATAL\n";
        }
        return true;
    }
    else if (statusSekarang == MENUNGGU && statusTujuan == BATAL) {
        p.status = BATAL;
        saveToFile();
        rebuildHeap();
        if (isInteractiveMode) {
            cout << "\n>>> [BATAL ANTRIAN] Antrian pasien dibatalkan:\n";
            cout << "    ID: " << id << " | Nama: " << p.nama << "\n";
            cout << "    Status berubah: MENUNGGU -> BATAL\n";
        }
        return true;
    }
    else if (statusSekarang == TERJADWAL && statusTujuan == MENUNGGU) {
        p.status = MENUNGGU;
        p.waktuDatang = getCurrentTimeStr();
        saveToFile();
        rebuildHeap();
        if (isInteractiveMode) {
            cout << "\n>>> [CHECK-IN] Pasien check-in fisik berhasil:\n";
            cout << "    ID: " << id << " | Nama: " << p.nama << "\n";
            cout << "    Status berubah: TERJADWAL -> MENUNGGU (Masuk Antrian Aktif)\n";
            cout << "    Prioritas: " << prioritasToString(p.prioritas) << " | No. Antrian: #" << p.nomorAntrian << "\n";
        }
        return true;
    }
    else if (statusSekarang == TERJADWAL && statusTujuan == BATAL) {
        p.status = BATAL;
        saveToFile();
        rebuildHeap();
        if (isInteractiveMode) {
            cout << "\n>>> [BATAL ANTRIAN] Booking pasien dibatalkan:\n";
            cout << "    ID: " << id << " | Nama: " << p.nama << "\n";
            cout << "    Status berubah: TERJADWAL -> BATAL\n";
        }
        return true;
    }
    else {
        errorMsg = "Perubahan status melanggar aturan:\n"
                   "- TERJADWAL -> MENUNGGU (Check-in)\n"
                   "- TERJADWAL -> BATAL (Batal Booking)\n"
                   "- MENUNGGU -> DIPANGGIL\n"
                   "- DIPANGGIL -> SELESAI\n"
                   "- DIPANGGIL -> BATAL\n"
                   "- MENUNGGU -> BATAL";
        return false;
    }
}

bool SistemAntrianRS::deleteAntrian(string id, string& errorMsg) {
    auto it = dataPasien.find(id);
    if (it == dataPasien.end()) {
        errorMsg = "Pasien tidak ditemukan.";
        return false;
    }

    if (it->second.status != MENUNGGU && it->second.status != TERJADWAL) {
        errorMsg = "Hanya pasien dengan status MENUNGGU atau TERJADWAL yang dapat dibatalkan.";
        return false;
    }

    StatusLayanan statusSebelumnya = it->second.status;
    it->second.status = BATAL;
    saveToFile();
    rebuildHeap();

    if (isInteractiveMode) {
        cout << "\n>>> [BATAL ANTRIAN] Antrian pasien dibatalkan:\n";
        cout << "    ID: " << id << " | Nama: " << it->second.nama << "\n";
        cout << "    Status berubah: " << statusToString(statusSebelumnya) << " -> BATAL\n";
    }

    return true;
}

void SistemAntrianRS::tampilkanSemuaData() {
    cout << "\n=== Seluruh Data Pasien ===\n";
    vector<Patient> listPasien;
    for (auto const& [key, p] : dataPasien) {
        listPasien.push_back(p);
    }
    tampilkanTabelPasien(listPasien);
    cout << endl;
}

void SistemAntrianRS::tampilkanAntrianAktif() {
    cout << "\n=== Pasien Menunggu ===\n";
    vector<Patient> listPasien;
    for (auto const& [key, p] : dataPasien) {
        if (p.status == MENUNGGU) listPasien.push_back(p);
    }
    tampilkanTabelPasien(listPasien);
    cout << endl;
}

void SistemAntrianRS::tampilkanPasienSelesai() {
    cout << "\n=== Pasien Selesai Layanan ===\n";
    vector<Patient> listPasien;
    for (auto const& [key, p] : dataPasien) {
        if (p.status == SELESAI) listPasien.push_back(p);
    }
    tampilkanTabelPasien(listPasien);
    cout << endl;
}

void SistemAntrianRS::tampilkanPasienBatal() {
    cout << "\n=== Pasien Batal Layanan ===\n";
    vector<Patient> listPasien;
    for (auto const& [key, p] : dataPasien) {
        if (p.status == BATAL) listPasien.push_back(p);
    }
    tampilkanTabelPasien(listPasien);
    cout << endl;
}

void SistemAntrianRS::tampilkanPasienTerjadwal() {
    cout << "\n=== Pasien Terjadwal (Belum Check-in) ===\n";
    vector<Patient> listPasien;
    for (auto const& [key, p] : dataPasien) {
        if (p.status == TERJADWAL) listPasien.push_back(p);
    }
    tampilkanTabelPasien(listPasien);
    cout << endl;
}

void SistemAntrianRS::dummyData() {
    dataPasien.clear();
    while (!antrian.empty()) {
        antrian.pop();
    }
    
    string today = getCurrentDateStr();
    string tomorrow = getTomorrowDateStr();
    
    // Inisialisasi data pasien dengan kombinasi komprehensif tanpa label aneh di nama
    Patient p1 = {"P001", "Andi", "UGD", DARURAT, "1N", "08:00", "08:15", today, SELESAI};
    Patient p2 = {"P002", "Budi", "Poli Umum", REGULER, "2N", "08:05", "08:20", today, DIPANGGIL};
    Patient p3 = {"P003", "Citra", "Poli Gigi", MENDESAK, "3N", "08:10", "08:25", today, DIPANGGIL};
    Patient p4 = {"P004", "Dani", "UGD", DARURAT, "4N", "08:15", "-", today, MENUNGGU};
    Patient p5 = {"P005", "Eka", "Poli Umum", PRIORITAS_RENTAN, "5N", "08:20", "-", today, MENUNGGU};
    Patient p6 = {"P006", "Fajar", "Poli Gigi", REGULER, "6N", "08:25", "-", today, MENUNGGU};
    Patient p7 = {"P007", "Giska", "Poli Mata", REGULER, "7N", "08:30", "-", today, MENUNGGU};
    Patient p8 = {"P008", "Hadi", "Poli Anak", MENDESAK, "8N", "08:35", "-", today, MENUNGGU};
    Patient p9 = {"P009", "Irma", "Laboratorium", REGULER, "9N", "08:40", "-", today, MENUNGGU};
    Patient p10 = {"P010", "Joko", "Poli Penyakit Dalam", PRIORITAS_RENTAN, "10N", "08:45", "-", today, MENUNGGU};
    Patient p11 = {"P011", "Kiki", "Poli Geriatri", PRIORITAS_RENTAN, "11N", "08:50", "-", today, MENUNGGU};
    Patient p12 = {"P012", "Lia", "Poli Umum", REGULER, "12N", "07:00", "07:30", today, SELESAI};
    Patient p13 = {"P013", "Mira", "Poli Anak", REGULER, "13N", "07:15", "-", today, BATAL};
    Patient p14 = {"P014", "Nina", "Poli Mata", REGULER, "14N", "-", "-", tomorrow, TERJADWAL};
    Patient p15 = {"P015", "Opan", "Laboratorium", PRIORITAS_RENTAN, "15N", "-", "-", tomorrow, TERJADWAL};
    Patient p16 = {"P016", "Putri", "Poli Gigi", MENDESAK, "16N", "-", "-", tomorrow, TERJADWAL};

    dataPasien["P001"] = p1; dataPasien["P002"] = p2; dataPasien["P003"] = p3;
    dataPasien["P004"] = p4; dataPasien["P005"] = p5; dataPasien["P006"] = p6;
    dataPasien["P007"] = p7; dataPasien["P008"] = p8; dataPasien["P009"] = p9;
    dataPasien["P010"] = p10; dataPasien["P011"] = p11; dataPasien["P012"] = p12;
    dataPasien["P013"] = p13; dataPasien["P014"] = p14; dataPasien["P015"] = p15;
    dataPasien["P016"] = p16;

    // Masukkan pasien MENUNGGU ke antrian aktif (priority queue)
    antrian.push(p4); antrian.push(p5); antrian.push(p6);
    antrian.push(p7); antrian.push(p8); antrian.push(p9);
    antrian.push(p10); antrian.push(p11);

    nomorBerikutnya = 17;
    saveToFile();
}

void SistemAntrianRS::saveToFile() {
    ofstream file(namaFileData.c_str());
    if (!file.is_open()) {
        cerr << "Gagal membuka file penyimpanan data.\n";
        return;
    }

    file << nomorBerikutnya << endl;
    for (auto const& [key, p] : dataPasien) {
        file << p.id << "|"
             << p.nama << "|"
             << p.layanan << "|"
             << p.prioritas << "|"
             << p.nomorAntrian << "|"
             << p.waktuDatang << "|"
             << p.waktuDipanggil << "|"
             << p.tanggal << "|"
             << (int)p.status << endl;
    }
    file.close();
}

void SistemAntrianRS::loadFromFile() {
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

        if (data.size() == 9) {
            Patient p;
            p.id = data[0];
            p.nama = data[1];
            p.layanan = data[2];
            p.prioritas = stoi(data[3]);
            p.nomorAntrian = data[4];
            p.waktuDatang = data[5];
            p.waktuDipanggil = data[6];
            p.tanggal = data[7];
            p.status = (StatusLayanan)stoi(data[8]);

            dataPasien[p.id] = p;

            if (p.status == MENUNGGU) {
                antrian.push(p);
            }
        }
        else if (data.size() == 8) {
            Patient p;
            p.id = data[0];
            p.nama = data[1];
            p.layanan = data[2];
            p.prioritas = stoi(data[3]);
            p.nomorAntrian = data[4];
            p.waktuDatang = data[5];
            p.waktuDipanggil = "-";
            p.tanggal = data[6];
            p.status = (StatusLayanan)stoi(data[7]);

            dataPasien[p.id] = p;

            if (p.status == MENUNGGU) {
                antrian.push(p);
            }
        }
        else if (data.size() == 7) {
            Patient p;
            p.id = data[0];
            p.nama = data[1];
            p.layanan = data[2];
            p.prioritas = stoi(data[3]);
            p.nomorAntrian = data[4];
            p.waktuDatang = data[5];
            p.waktuDipanggil = "-";
            p.tanggal = "2026-06-03";
            p.status = (StatusLayanan)stoi(data[6]);

            dataPasien[p.id] = p;

            if (p.status == MENUNGGU) {
                antrian.push(p);
            }
        }
    }
    file.close();
}

// helper buat escape string JSON
string escapeJsonString(const string& input) {
    stringstream ss;
    for (char c : input) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\n') ss << "\\n";
        else if (c == '\r') ss << "\\r";
        else if (c == '\t') ss << "\\t";
        else if (c == '\b') ss << "\\b";
        else if (c == '\f') ss << "\\f";
        else ss << c;
    }
    return ss.str();
}

// eksekutor request JSON dari Node.js
string SistemAntrianRS::executeJsonCommand(const string& jsonInput) {
    string action = extractJsonValue(jsonInput, "action");
    stringstream out;

    if (action == "insert") {
        string id = extractJsonValue(jsonInput, "id");
        string nama = extractJsonValue(jsonInput, "nama");
        string layanan = extractJsonValue(jsonInput, "layanan");
        string prioStr = extractJsonValue(jsonInput, "prioritas");
        string waktu = extractJsonValue(jsonInput, "waktuDatang");
        string tanggal = extractJsonValue(jsonInput, "tanggal");

        if (tanggal.empty()) tanggal = "2026-06-03";

        int prioritas = prioStr.empty() ? 4 : stoi(prioStr);
        string errorMsg;

        if (insertPatient(id, nama, layanan, prioritas, waktu, tanggal, errorMsg)) {
            Patient p = dataPasien[id];
            out << "{\"status\":\"success\",\"message\":\"Pasien berhasil ditambahkan ke antrian.\",\"data\":{";
            out << "\"id\":\"" << p.id << "\",";
            out << "\"nama\":\"" << p.nama << "\",";
            out << "\"layanan\":\"" << p.layanan << "\",";
            out << "\"prioritas\":" << p.prioritas << ",";
            out << "\"prioritasLabel\":\"" << prioritasToString(p.prioritas) << "\",";
            out << "\"nomorAntrian\":\"" << p.nomorAntrian << "\",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << escapeJsonString(errorMsg) << "\"}";
        }
    }
    else if (action == "check_in") {
        string id = extractJsonValue(jsonInput, "id");
        string waktu = extractJsonValue(jsonInput, "waktuDatang");
        string errorMsg;

        if (checkInPatient(id, waktu, errorMsg)) {
            Patient p = dataPasien[id];
            out << "{\"status\":\"success\",\"message\":\"Pasien berhasil check-in ke antrian.\",\"data\":{";
            out << "\"id\":\"" << p.id << "\",";
            out << "\"nama\":\"" << p.nama << "\",";
            out << "\"layanan\":\"" << p.layanan << "\",";
            out << "\"prioritas\":" << p.prioritas << ",";
            out << "\"prioritasLabel\":\"" << prioritasToString(p.prioritas) << "\",";
            out << "\"nomorAntrian\":\"" << p.nomorAntrian << "\",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << escapeJsonString(errorMsg) << "\"}";
        }
    }
    else if (action == "call_next") {
        Patient p;
        string errorMsg;
        if (panggilAntrianBerikutnya(p, errorMsg)) {
            out << "{\"status\":\"success\",\"message\":\"Pasien dipanggil.\",\"data\":{";
            out << "\"id\":\"" << p.id << "\",";
            out << "\"nama\":\"" << p.nama << "\",";
            out << "\"layanan\":\"" << p.layanan << "\",";
            out << "\"prioritas\":" << p.prioritas << ",";
            out << "\"prioritasLabel\":\"" << prioritasToString(p.prioritas) << "\",";
            out << "\"nomorAntrian\":\"" << p.nomorAntrian << "\",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << escapeJsonString(errorMsg) << "\"}";
        }
    }
    else if (action == "search") {
        string id = extractJsonValue(jsonInput, "id");
        Patient p;
        string errorMsg;
        if (searchPatient(id, p, errorMsg)) {
            out << "{\"status\":\"success\",\"message\":\"Pasien ditemukan.\",\"data\":{";
            out << "\"id\":\"" << p.id << "\",";
            out << "\"nama\":\"" << p.nama << "\",";
            out << "\"layanan\":\"" << p.layanan << "\",";
            out << "\"prioritas\":" << p.prioritas << ",";
            out << "\"prioritasLabel\":\"" << prioritasToString(p.prioritas) << "\",";
            out << "\"nomorAntrian\":\"" << p.nomorAntrian << "\",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << escapeJsonString(errorMsg) << "\"}";
        }
    }
    else if (action == "update_status") {
        string id = extractJsonValue(jsonInput, "id");
        string statusStr = extractJsonValue(jsonInput, "status");
        string forceStr = extractJsonValue(jsonInput, "force");
        int statusBaru = statusStr.empty() ? -1 : stoi(statusStr);
        bool force = (forceStr == "true" || forceStr == "1");
        string errorMsg;

        if (updateStatus(id, statusBaru, force, errorMsg)) {
            out << "{\"status\":\"success\",\"message\":\"Status pasien berhasil diupdate.\"}";
        } else {
            if (errorMsg.rfind("WARNING_PRIORITY:", 0) == 0) {
                out << "{\"status\":\"warning\",\"message\":\"" << escapeJsonString(errorMsg) << "\"}";
            } else {
                out << "{\"status\":\"error\",\"message\":\"" << escapeJsonString(errorMsg) << "\"}";
            }
        }
    }
    else if (action == "delete") {
        string id = extractJsonValue(jsonInput, "id");
        string errorMsg;

        if (deleteAntrian(id, errorMsg)) {
            out << "{\"status\":\"success\",\"message\":\"Pasien dibatalkan dari antrian.\"}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << escapeJsonString(errorMsg) << "\"}";
        }
    }
    else if (action == "get_heap") {
        struct ExposedQueue : public priority_queue<Patient, vector<Patient>, ComparePatient> {
            const vector<Patient>& getContainer() const { return c; }
        };
        
        const vector<Patient>& heapArr = static_cast<const ExposedQueue&>(antrian).getContainer();
        
        out << "{\"status\":\"success\",\"data\":[";
        bool first = true;
        for (const auto& p : heapArr) {
            if (!first) out << ",";
            first = false;
            out << "{";
            out << "\"id\":\"" << p.id << "\",";
            out << "\"nama\":\"" << p.nama << "\",";
            out << "\"nomorAntrian\":\"" << p.nomorAntrian << "\",";
            out << "\"layanan\":\"" << p.layanan << "\",";
            out << "\"prioritas\":" << p.prioritas << ",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}";
        }
        out << "]}";
    }
    else if (action == "list_all" || action == "list_waiting" || action == "list_scheduled") {
        vector<Patient> listPasien;
        for (auto const& [key, p] : dataPasien) {
            if (action == "list_waiting" && p.status != MENUNGGU && p.status != DIPANGGIL) {
                continue;
            }
            if (action == "list_scheduled" && p.status != TERJADWAL) {
                continue;
            }
            listPasien.push_back(p);
        }

        // Urutkan list sesuai aturan prioritas
        sort(listPasien.begin(), listPasien.end(), comparePatientsForDisplay);

        out << "{\"status\":\"success\",\"data\":[";
        bool first = true;
        for (const auto& p : listPasien) {
            if (!first) out << ",";
            first = false;
            out << "{";
            out << "\"id\":\"" << p.id << "\",";
            out << "\"nama\":\"" << p.nama << "\",";
            out << "\"layanan\":\"" << p.layanan << "\",";
            out << "\"prioritas\":" << p.prioritas << ",";
            out << "\"prioritasLabel\":\"" << prioritasToString(p.prioritas) << "\",";
            out << "\"nomorAntrian\":\"" << p.nomorAntrian << "\",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":" << (int)p.status << ",";
            out << "\"statusLabel\":\"" << statusToString(p.status) << "\"";
            out << "}";
        }
        out << "]}";
    }
    else if (action == "dummy_data") {
        dummyData();
        out << "{\"status\":\"success\",\"message\":\"Data simulasi berhasil dimasukkan.\"}";
    }
    else {
        out << "{\"status\":\"error\",\"message\":\"Action '" << action << "' tidak dikenal.\"}";
    }

    return out.str();
}
