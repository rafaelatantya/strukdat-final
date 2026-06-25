#ifndef QUEUE_SYSTEM_H
#define QUEUE_SYSTEM_H

#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
#include "Patient.h"

class SistemAntrianRS {
private:
    std::priority_queue<Patient, std::vector<Patient>, ComparePatient> antrian;
    std::unordered_map<std::string, Patient> dataPasien;
    int nomorBerikutnya;
    std::string namaFileData;
    std::string namaFileBenchmark;
    bool isInteractiveMode;

    // helper parsing json tipis-tipis
    std::string trim(const std::string& str);
    std::string extractJsonValue(const std::string& json, const std::string& key);

    // helper validasi format tanggal & waktu
    bool isValidDate(const std::string& date);
    bool isValidTime(const std::string& time);

public:
    SistemAntrianRS(
        std::string fileData = "data_pasien_rs.txt",
        std::string fileBenchmark = "benchmark_minggu7.txt"
    );

    void setInteractiveMode(bool mode);

    bool prioritasValid(int p);
    bool statusValid(int s);

    std::string prioritasToString(int p);
    std::string statusToString(StatusLayanan s);

    void tampilkanSatuPasien(const Patient& p);
    void tampilkanTabelPasien(const std::vector<Patient>& listPasien);

    // method utama sistem antrian
    bool insertPatient(std::string id, std::string nama, std::string layanan, int prioritas, std::string waktuDatang, std::string tanggal, std::string& errorMsg);
    bool checkInPatient(std::string id, std::string waktuDatang, std::string& errorMsg);
    bool panggilAntrianBerikutnya(Patient& outPatient, std::string& errorMsg);
    bool searchPatient(std::string id, Patient& outPatient, std::string& errorMsg);
    bool updateStatus(std::string id, int statusBaru, std::string& errorMsg);
    bool deleteAntrian(std::string id, std::string& errorMsg);

    void tampilkanSemuaData();
    void tampilkanAntrianAktif();
    void tampilkanPasienSelesai();
    void tampilkanPasienBatal();
    void tampilkanPasienTerjadwal();
    void dummyData();

    void saveToFile();
    void loadFromFile();

    // eksekusi command lewat json
    std::string executeJsonCommand(const std::string& jsonInput);
};

#endif // QUEUE_SYSTEM_H
