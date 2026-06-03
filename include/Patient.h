#ifndef PATIENT_H
#define PATIENT_H

#include <string>

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
    std::string id;
    std::string nama;
    std::string layanan;
    int prioritas;
    int nomorAntrian;
    std::string waktuDatang;
    std::string tanggal;
    StatusLayanan status;
};

// bandingin pasien: prioritas dulu, klo sama baru nomor antrian
struct ComparePatient {
    bool operator()(const Patient& a, const Patient& b) const {
        if (a.prioritas == b.prioritas) {
            return a.nomorAntrian > b.nomorAntrian;
        }
        return a.prioritas > b.prioritas;
    }
};

#endif // PATIENT_H
