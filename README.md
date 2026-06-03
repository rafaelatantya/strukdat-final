# 🏥 Sistem Antrian Rumah Sakit Terintegrasi (Hospital Queue System)

Proyek ini adalah prototipe **Sistem Antrian Rumah Sakit** yang menggabungkan kecepatan pemrosesan data struktur data tingkat rendah menggunakan **Modular C++ Engine** dengan kemudahan pengelolaan operasional staf medis melalui **Web Admin Dashboard** berbasis **Node.js (Express)** dan **React (Vite)**.

---

## 3. Desain Arsitektur Program

Desain arsitektur program menggunakan arsitektur 5-Layer terintegrasi yang memisahkan antara Presentation (Web Client), API Gateway, Core Logic, In-Memory Data Structures, dan File Persistence Storage:

![Desain Arsitektur Program](docs/architecture.png)

---

## ⚡ Fitur Utama

1. **Sistem Antrian Skala Prioritas Medis (Priority Queue)**:
   Pasien diurutkan berdasarkan prioritas kondisi kesehatannya:
   * **Prioritas 1 (Darurat/Emergency)**: Korban kecelakaan, UGD, serangan jantung.
   * **Prioritas 2 (Mendesak/Urgent)**: Nyeri hebat, demam sangat tinggi.
   * **Prioritas 3 (Pasien Rentan)**: Lansia, ibu hamil, penyandang disabilitas.
   * **Prioritas 4 (Reguler)**: Pemeriksaan umum, rawat jalan biasa.

2. **Logika Check-in Terjadwal (Stateless Booking)**:
   Pasien yang melakukan booking janji temu secara online akan mendapat status `TERJADWAL` (Scheduled) dan tidak akan dimasukkan ke antrian aktif (heap) memori sebelum datang secara fisik untuk konfirmasi kedatangan (check-in) di loket loket.

3. **Pencatatan Waktu Panggil Otomatis (Called Time Timestamping)**:
   Ketika dokter menekan tombol panggil pasien berikutnya (`call_next`), program C++ secara dinamis mencatat waktu panggil aktual menggunakan jam sistem lokal (system clock) dalam format `HH:MM`.

4. **Persistensi Database File Lokal**:
   Semua modifikasi data (`insert`, `check_in`, `call_next`, `update_status`, `delete`) secara reaktif disinkronisasikan langsung ke dalam file flat-file database lokal `data_pasien_rs.txt`.

5. **Pengujian Analisis Performa (Benchmark Engine)**:
   Membandingkan kecepatan eksekusi algoritma antrian prioritas berbasis **Min-Heap** dengan antrian standar berbasis **FIFO (First-In First-Out)** pada berbagai skala pengujian (100 hingga 50.000 records).

---

## 🧬 Struktur Data yang Digunakan

* **Struct (`struct Patient`)**:
  Digunakan untuk menampung record data pasien beserta seluruh atributnya di berkas [Patient.h](include/Patient.h).
* **Priority Queue (`std::priority_queue`)**:
  Struktur data berbasis **Min-Heap** di memori C++ yang mengurutkan pasien secara instan berdasarkan tingkat prioritas terkecil (1 tertinggi) dan nomor antrian terkecil jika prioritasnya sama.
* **Hash Table (`std::unordered_map`)**:
  Digunakan untuk melacak dan mencari data pasien di memori secara cepat $O(1)$ untuk keperluan update status atau pembatalan antrian secara acak menggunakan ID Pasien.
* **Queue (`std::queue`)**:
  Digunakan di dalam modul benchmark [Benchmark.cpp](src/Benchmark.cpp) sebagai struktur data antrian standar pembanding performa.

---

## 🚀 Panduan Menjalankan Program

### 1. Kompilasi Kode C++
Kompilasi program C++ menggunakan `Makefile` di root folder proyek:
```bash
make clean && make
```

### 2. Jalankan Mode Terminal CLI (Menu Interaktif)
Untuk menjalankan program menu interaktif berbasis teks langsung di terminal:
```bash
./hospital_queue
```

### 3. Jalankan Mode Web UI Dashboard
Aplikasi web dijalankan dalam arsitektur terpisah (Backend + Frontend):

#### A. Jalankan API Backend (Express.js)
Buka terminal baru:
```bash
cd web/backend
npm install
node server.js
```
*(Backend akan aktif di `http://localhost:5000`)*

#### B. Jalankan Frontend (React + Vite)
Buka terminal baru lagi:
```bash
cd web/frontend
npm install
npm run dev
```
*(Frontend akan aktif di `http://localhost:5173/`)*

Buka browser Anda lalu kunjungi: **`http://localhost:5173/`**
