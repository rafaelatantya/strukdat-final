#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <iomanip>
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

SistemAntrianRS::SistemAntrianRS(string fileData, string fileBenchmark) {
    nomorBerikutnya = 1;
    namaFileData = fileData;
    namaFileBenchmark = fileBenchmark;
    loadFromFile();
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
    sort(sortedList.begin(), sortedList.end(), [](const Patient& a, const Patient& b) {
        return a.id < b.id; // Urutkan berdasarkan ID
    });
    
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
    if (id.empty() || nama.empty() || layanan.empty() || tanggal.empty()) {
        errorMsg = "Data pasien tidak boleh ada yang kosong.";
        return false;
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
    p.nomorAntrian = nomorBerikutnya++;
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

    return true;
}

bool SistemAntrianRS::checkInPatient(string id, string waktuDatang, string& errorMsg) {
    auto it = dataPasien.find(id);
    if (it == dataPasien.end()) {
        errorMsg = "Pasien tidak ditemukan.";
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
    return true;
}

bool SistemAntrianRS::panggilAntrianBerikutnya(Patient& outPatient, string& errorMsg) {
    while (!antrian.empty()) {
        Patient p = antrian.top();
        antrian.pop();

        // cek map, sapa tau pasiennya udah dibatalin pas masih ngantri
        if (dataPasien[p.id].status == MENUNGGU) {
            dataPasien[p.id].status = DIPANGGIL;
            dataPasien[p.id].waktuDipanggil = getCurrentTimeStr();
            saveToFile();
            outPatient = dataPasien[p.id];
            return true;
        }
    }
    errorMsg = "Antrian kosong atau semua pasien dalam antrian sudah dipanggil/dibatalkan.";
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

bool SistemAntrianRS::updateStatus(string id, int statusBaru, string& errorMsg) {
    if (!statusValid(statusBaru)) {
        errorMsg = "Status baru tidak valid.";
        return false;
    }

    auto it = dataPasien.find(id);
    if (it == dataPasien.end()) {
        errorMsg = "Pasien tidak ditemukan.";
        return false;
    }

    StatusLayanan statusSekarang = it->second.status;
    StatusLayanan statusTujuan = (StatusLayanan)statusBaru;

    if (statusSekarang == MENUNGGU && statusTujuan == DIPANGGIL) {
        it->second.status = DIPANGGIL;
        it->second.waktuDipanggil = getCurrentTimeStr();
        saveToFile();
        return true;
    }
    else if (statusSekarang == DIPANGGIL && statusTujuan == SELESAI) {
        it->second.status = SELESAI;
        saveToFile();
        return true;
    }
    else if (statusSekarang == MENUNGGU && statusTujuan == BATAL) {
        it->second.status = BATAL;
        saveToFile();
        return true;
    }
    else if (statusSekarang == TERJADWAL && statusTujuan == MENUNGGU) {
        it->second.status = MENUNGGU;
        it->second.waktuDatang = getCurrentTimeStr();
        antrian.push(it->second);
        saveToFile();
        return true;
    }
    else if (statusSekarang == TERJADWAL && statusTujuan == BATAL) {
        it->second.status = BATAL;
        saveToFile();
        return true;
    }
    else {
        errorMsg = "Perubahan status melanggar aturan:\n"
                   "- TERJADWAL -> MENUNGGU (Check-in)\n"
                   "- TERJADWAL -> BATAL (Batal Booking)\n"
                   "- MENUNGGU -> DIPANGGIL\n"
                   "- DIPANGGIL -> SELESAI\n"
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

    if (it->second.status != MENUNGGU) {
        errorMsg = "Hanya pasien dengan status MENUNGGU yang dapat dibatalkan.";
        return false;
    }

    it->second.status = BATAL;
    saveToFile();
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
    
    // Inisialisasi data pasien dengan semua kombinasi status dan prioritas secara adaptif
    Patient p1 = {"P001", "Andi", "Poli Umum", REGULER, 1, "08:00", "08:15", today, SELESAI};
    Patient p2 = {"P002", "Budi", "Laboratorium", REGULER, 2, "08:05", "-", today, MENUNGGU};
    Patient p3 = {"P003", "Citra", "UGD", DARURAT, 3, "08:07", "08:10", today, DIPANGGIL};
    Patient p4 = {"P004", "Dewi", "Poli Geriatri", PRIORITAS_RENTAN, 4, "08:10", "-", today, BATAL};
    Patient p5 = {"P005", "Eko", "Poli Penyakit Dalam", MENDESAK, 5, "08:12", "-", today, MENUNGGU};
    Patient p6 = {"P006", "Farhan", "Poli Gigi", REGULER, 6, "-", "-", today, TERJADWAL};
    Patient p7 = {"P007", "Giska", "Poli Mata", PRIORITAS_RENTAN, 7, "-", "-", tomorrow, TERJADWAL};
    Patient p8 = {"P008", "Hari", "UGD", DARURAT, 8, "08:20", "-", today, MENUNGGU};
    Patient p9 = {"P009", "Irma", "Poli Anak", MENDESAK, 9, "-", "-", today, TERJADWAL};

    dataPasien["P001"] = p1;
    dataPasien["P002"] = p2;
    dataPasien["P003"] = p3;
    dataPasien["P004"] = p4;
    dataPasien["P005"] = p5;
    dataPasien["P006"] = p6;
    dataPasien["P007"] = p7;
    dataPasien["P008"] = p8;
    dataPasien["P009"] = p9;

    // Masukkan pasien MENUNGGU ke antrian aktif (priority queue)
    antrian.push(p2);
    antrian.push(p5);
    antrian.push(p8);

    nomorBerikutnya = 10;
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
            p.nomorAntrian = stoi(data[4]);
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
            p.nomorAntrian = stoi(data[4]);
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
            p.nomorAntrian = stoi(data[4]);
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
            out << "\"nomorAntrian\":" << p.nomorAntrian << ",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << errorMsg << "\"}";
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
            out << "\"nomorAntrian\":" << p.nomorAntrian << ",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << errorMsg << "\"}";
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
            out << "\"nomorAntrian\":" << p.nomorAntrian << ",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << errorMsg << "\"}";
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
            out << "\"nomorAntrian\":" << p.nomorAntrian << ",";
            out << "\"waktuDatang\":\"" << p.waktuDatang << "\",";
            out << "\"waktuDipanggil\":\"" << p.waktuDipanggil << "\",";
            out << "\"tanggal\":\"" << p.tanggal << "\",";
            out << "\"status\":\"" << statusToString(p.status) << "\"";
            out << "}}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << errorMsg << "\"}";
        }
    }
    else if (action == "update_status") {
        string id = extractJsonValue(jsonInput, "id");
        string statusStr = extractJsonValue(jsonInput, "status");
        int statusBaru = statusStr.empty() ? -1 : stoi(statusStr);
        string errorMsg;

        if (updateStatus(id, statusBaru, errorMsg)) {
            out << "{\"status\":\"success\",\"message\":\"Status pasien berhasil diupdate.\"}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << errorMsg << "\"}";
        }
    }
    else if (action == "delete") {
        string id = extractJsonValue(jsonInput, "id");
        string errorMsg;

        if (deleteAntrian(id, errorMsg)) {
            out << "{\"status\":\"success\",\"message\":\"Pasien dibatalkan dari antrian.\"}";
        } else {
            out << "{\"status\":\"error\",\"message\":\"" << errorMsg << "\"}";
        }
    }
    else if (action == "list_all" || action == "list_waiting" || action == "list_scheduled") {
        out << "{\"status\":\"success\",\"data\":[";
        bool first = true;
        for (auto const& [key, p] : dataPasien) {
            if (action == "list_waiting" && p.status != MENUNGGU) {
                continue;
            }
            if (action == "list_scheduled" && p.status != TERJADWAL) {
                continue;
            }
            if (!first) out << ",";
            first = false;
            out << "{";
            out << "\"id\":\"" << p.id << "\",";
            out << "\"nama\":\"" << p.nama << "\",";
            out << "\"layanan\":\"" << p.layanan << "\",";
            out << "\"prioritas\":" << p.prioritas << ",";
            out << "\"prioritasLabel\":\"" << prioritasToString(p.prioritas) << "\",";
            out << "\"nomorAntrian\":" << p.nomorAntrian << ",";
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
