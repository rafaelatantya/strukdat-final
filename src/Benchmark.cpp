#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <sys/resource.h>
#include "Benchmark.h"
#include "Patient.h"

using namespace std;
using namespace std::chrono;

void runFullBenchmark(int scale, const string& outputFilePath) {
    auto startTotal = high_resolution_clock::now();

    if (scale <= 0) {
        cerr << "Skala benchmark harus lebih dari 0.\n";
        return;
    }

    cout << "Menjalankan benchmark dengan skala data: " << scale << "...\n";

    // 1. bikin data dummy buat tes
    vector<Patient> dataSimulasi;
    dataSimulasi.reserve(scale);
    for (int i = 1; i <= scale; i++) {
        Patient p;
        p.id = "SIM" + to_string(i);
        p.nama = "Pasien " + to_string(i);
        p.layanan = "Poli Umum";
        p.prioritas = (i % 4) + 1; // prioritas 1-4
        p.nomorAntrian = to_string(i) + "N";
        p.waktuDatang = "08:00";
        p.status = MENUNGGU;
        dataSimulasi.push_back(p);
    }

    // --- TES PRIORITY QUEUE (HEAP) ---
    priority_queue<Patient, vector<Patient>, ComparePatient> pq;
    unordered_map<string, Patient> pqMap;

    auto pqInsertStart = high_resolution_clock::now();
    for (int i = 0; i < scale; i++) {
        pq.push(dataSimulasi[i]);
        pqMap[dataSimulasi[i].id] = dataSimulasi[i];
    }
    auto pqInsertEnd = high_resolution_clock::now();
    long long pqInsertUs = duration_cast<microseconds>(pqInsertEnd - pqInsertStart).count();

    // tes cari data pake map O(1)
    string searchTarget = "SIM" + to_string(scale / 2);
    auto hashSearchStart = high_resolution_clock::now();
    auto itMap = pqMap.find(searchTarget);
    volatile bool foundHash = (itMap != pqMap.end()); (void)foundHash;
    auto hashSearchEnd = high_resolution_clock::now();
    long long hashSearchUs = duration_cast<microseconds>(hashSearchEnd - hashSearchStart).count();

    // tes manggil/pop ampe abis di pq
    auto pqCallStart = high_resolution_clock::now();
    while (!pq.empty()) {
        Patient p = pq.top(); (void)p;
        pq.pop();
    }
    auto pqCallEnd = high_resolution_clock::now();
    long long pqCallUs = duration_cast<microseconds>(pqCallEnd - pqCallStart).count();


    // --- TES QUEUE BIASA (FIFO) ---
    queue<Patient> q;
    vector<Patient> qList; // pake vector buat linear search
    qList.reserve(scale);

    auto qInsertStart = high_resolution_clock::now();
    for (int i = 0; i < scale; i++) {
        q.push(dataSimulasi[i]);
        qList.push_back(dataSimulasi[i]);
    }
    auto qInsertEnd = high_resolution_clock::now();
    long long qInsertUs = duration_cast<microseconds>(qInsertEnd - qInsertStart).count();

    // tes cari data secara linear O(N)
    auto linearSearchStart = high_resolution_clock::now();
    bool foundLinear = false;
    for (int i = 0; i < scale; i++) {
        if (qList[i].id == searchTarget) {
            foundLinear = true;
            break;
        }
    }
    (void)foundLinear;
    auto linearSearchEnd = high_resolution_clock::now();
    long long linearSearchUs = duration_cast<microseconds>(linearSearchEnd - linearSearchStart).count();

    // tes manggil/pop ampe abis di queue biasa
    auto qCallStart = high_resolution_clock::now();
    while (!q.empty()) {
        Patient p = q.front(); (void)p;
        q.pop();
    }
    auto qCallEnd = high_resolution_clock::now();
    long long qCallUs = duration_cast<microseconds>(qCallEnd - qCallStart).count();

    // cetak hasil ke terminal
    cout << "\n=== Hasil Benchmark (Skala " << scale << ") ===\n";
    cout << "1. Operasi Insert (Batch):\n";
    cout << "   - Priority Queue (Heap) : " << pqInsertUs << " us\n";
    cout << "   - Standard Queue (FIFO) : " << qInsertUs << " us\n";
    cout << "2. Operasi Pemanggilan (Batch Pop):\n";
    cout << "   - Priority Queue (Heap) : " << pqCallUs << " us\n";
    cout << "   - Standard Queue (FIFO) : " << qCallUs << " us\n";
    cout << "3. Operasi Pencarian (Single Target):\n";
    cout << "   - Hash Table Search O(1): " << hashSearchUs << " us\n";
    cout << "   - Linear Search O(N)    : " << linearSearchUs << " us\n\n";

    auto endTotal = high_resolution_clock::now();
    long long totalTimeMs = duration_cast<milliseconds>(endTotal - startTotal).count();

    double memoryUsageMB = 0.0;
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        memoryUsageMB = usage.ru_maxrss / 1024.0;
    }

    // simpan hasilnya ke file json
    ofstream file(outputFilePath.c_str());
    if (file.is_open()) {
        file << "{\n";
        file << "  \"scale\": " << scale << ",\n";
        file << "  \"totalTimeMs\": " << totalTimeMs << ",\n";
        file << "  \"memoryUsageMB\": " << memoryUsageMB << ",\n";
        file << "  \"priority_queue\": {\n";
        file << "    \"insert_time_us\": " << pqInsertUs << ",\n";
        file << "    \"call_next_time_us\": " << pqCallUs << ",\n";
        file << "    \"search_time_us\": " << hashSearchUs << "\n";
        file << "  },\n";
        file << "  \"standard_queue\": {\n";
        file << "    \"insert_time_us\": " << qInsertUs << ",\n";
        file << "    \"call_next_time_us\": " << qCallUs << ",\n";
        file << "    \"search_time_us\": " << linearSearchUs << "\n";
        file << "  }\n";
        file << "}\n";
        file.close();
        cout << "Hasil benchmark berhasil disimpan ke: " << outputFilePath << "\n\n";
    } else {
        cerr << "Gagal menyimpan hasil benchmark ke: " << outputFilePath << "\n\n";
    }
}
