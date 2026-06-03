#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include "QueueSystem.h"

using namespace std;

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
    cout << "Status         : " << statusToString(p.status) << endl;
}

bool SistemAntrianRS::insertPatient(string id, string nama, string layanan, int prioritas, string waktuDatang, string& errorMsg) {
    if (id.empty() || nama.empty() || layanan.empty() || waktuDatang.empty()) {
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
    p.waktuDatang = waktuDatang;
    p.status = MENUNGGU;

    antrian.push(p);
    dataPasien[id] = p;
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
    else {
        errorMsg = "Perubahan status melanggar aturan:\n"
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
    if (dataPasien.empty()) {
        cout << "Belum ada data pasien.\n\n";
        return;
    }
    for (auto const& [key, p] : dataPasien) {
        tampilkanSatuPasien(p);
        cout << "-----------------------------\n";
    }
    cout << endl;
}

void SistemAntrianRS::tampilkanAntrianAktif() {
    cout << "\n=== Pasien Menunggu ===\n";
    bool ada = false;
    for (auto const& [key, p] : dataPasien) {
        if (p.status == MENUNGGU) {
            tampilkanSatuPasien(p);
            cout << "-----------------------------\n";
            ada = true;
        }
    }
    if (!ada) {
        cout << "Tidak ada pasien yang sedang menunggu.\n";
    }
    cout << endl;
}

void SistemAntrianRS::dummyData() {
    string errorMsg;
    if (dataPasien.find("P001") == dataPasien.end())
        insertPatient("P001", "Andi", "Poli Umum", REGULER, "08:00", errorMsg);
    if (dataPasien.find("P002") == dataPasien.end())
        insertPatient("P002", "Budi", "Laboratorium", REGULER, "08:05", errorMsg);
    if (dataPasien.find("P003") == dataPasien.end())
        insertPatient("P003", "Citra", "UGD", DARURAT, "08:07", errorMsg);
    if (dataPasien.find("P004") == dataPasien.end())
        insertPatient("P004", "Dewi", "Poli Geriatri", PRIORITAS_RENTAN, "08:10", errorMsg);
    if (dataPasien.find("P005") == dataPasien.end())
        insertPatient("P005", "Eko", "Poli Penyakit Dalam", MENDESAK, "08:12", errorMsg);
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

        int prioritas = prioStr.empty() ? 4 : stoi(prioStr);
        string errorMsg;

        if (insertPatient(id, nama, layanan, prioritas, waktu, errorMsg)) {
            out << "{\"status\":\"success\",\"message\":\"Pasien berhasil ditambahkan ke antrian.\",\"data\":{";
            out << "\"id\":\"" << id << "\",";
            out << "\"nama\":\"" << nama << "\",";
            out << "\"layanan\":\"" << layanan << "\",";
            out << "\"prioritas\":" << prioritas << ",";
            out << "\"prioritasLabel\":\"" << prioritasToString(prioritas) << "\",";
            out << "\"nomorAntrian\":" << (nomorBerikutnya - 1) << ",";
            out << "\"waktuDatang\":\"" << waktu << "\",";
            out << "\"status\":\"" << statusToString(MENUNGGU) << "\"";
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
    else if (action == "list_all" || action == "list_waiting") {
        out << "{\"status\":\"success\",\"data\":[";
        bool first = true;
        for (auto const& [key, p] : dataPasien) {
            if (action == "list_waiting" && p.status != MENUNGGU) {
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
