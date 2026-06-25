# Aturan & Batasan Sistem

Dokumen ini mendefinisikan aturan bisnis utama, skema prioritas, spesifikasi data, dan batasan status pada Sistem Antrian Rumah Sakit terintegrasi.

---

## 1. Spesifikasi Data Domain

Setiap record data pasien harus berisi field-field berikut:

| Nama Field | Tipe Data | Batasan / Aturan | Deskripsi |
|---|---|---|---|
| **ID Pasien** | `string` | Harus unik. Tidak boleh kosong. | Kode unik pengenal pasien (misal: `P001`). |
| **Nama Pasien** | `string` | Tidak boleh kosong. | Nama lengkap pasien. |
| **Jenis Layanan** | `string` | Tidak boleh kosong. | Unit/Poli tujuan (misal: `Poli Umum`, `UGD`, `Laboratorium`). |
| **Tingkat Prioritas** | `int` | Rentang: `1` hingga `4` (1 tertinggi). | Kategori penentu urutan pelayanan. |
| **Nomor Antrian** | `string` | Format `[ANGKA]N` (contoh: `1N`). | Nomor urut yang didapatkan saat registrasi. |
| **Waktu Kedatangan** | `string` | Format: `HH:MM` (24 jam) atau `-`. | Waktu check-in fisik aktual pasien di lokasi loket. Bernilai `-` jika pasien belum check-in. |
| **Waktu Dipanggil** | `string` | Format: `HH:MM` (24 jam) atau `-`. | Waktu ketika pasien dipanggil oleh dokter. Bernilai `-` jika pasien masih mengantri atau belum datang. |
| **Tanggal Appointment** | `string` | Format: `YYYY-MM-DD`. | Tanggal kunjungan terjadwal pelayanan pasien. |
| **Status Layanan** | `enum` | Nilai: `0` (Menunggu), `1` (Dipanggil), `2` (Selesai), `3` (Batal), `4` (Terjadwal). | Status alur pelayanan pasien saat ini. |

### 1.1 Penjelasan Detil Desain Waktu & Tanggal

Untuk menjaga kebersihan data (data integrity) dan optimalisasi performa pencarian, informasi penjadwalan dipecah menjadi dua field terpisah:

1. **Tanggal Appointment (`tanggal`)**:
   * **Definisi**: Tanggal terjadwal pasien untuk berobat (misal: `2026-06-03`).
   * **Peran**: Membantu petugas memfilter database pasien berdasarkan hari operasional secara efisien tanpa perlu melakukan parsing datetime yang rumit.
2. **Arrival Time / Waktu Datang (`waktuDatang`)**:
   * **Definisi**: Jam ketika pasien datang ke rumah sakit untuk melakukan konfirmasi booking/check-in fisik (misal: `08:15`).
   * **Peran**: Digunakan sebagai penentu urutan (tie-breaker FIFO) pada antrian dengan tingkat prioritas yang sama. Pasien yang check-in lebih dulu pada tanggal tersebut akan mendapatkan nomor antrian lebih kecil dan dilayani lebih dulu.

*Catatan: Selisih antara jam check-in fisik (`waktuDatang`) dan jam ketika status diubah menjadi `DIPANGGIL` merepresentasikan durasi waktu tunggu aktual pasien (Wait Time).*

---

## 2. Hierarki Prioritas

Sistem menggunakan priority queue untuk mengurutkan pelayanan pasien berdasarkan tingkat prioritas terlebih dahulu, kemudian berdasarkan nomor kedatangan (nomor antrian) jika tingkat prioritasnya sama.

| Tingkat Prioritas | Kategori | Deskripsi / Contoh Kasus |
| :---: | :---: | :--- |
| **1** | **Darurat (Emergency)** | UGD, kecelakaan, serangan jantung, kondisi kritis mengancam nyawa. |
| **2** | **Mendesak (Urgent)** | Nyeri berat, demam sangat tinggi, kondisi butuh penanganan segera. |
| **3** | **Rentan (Vulnerable)** | Lansia, ibu hamil, penyandang disabilitas. |
| **4** | **Reguler (Regular)** | Pemeriksaan rutin, poli umum biasa. |

### Logika Pengurutan (Min-Heap pada Kode Prioritas)
Misalkan terdapat pasien $A$ dan pasien $B$ di dalam antrian:
1. Jika $A.\text{prioritas} < B.\text{prioritas}$, maka $A$ dilayani lebih dulu dari $B$.
2. Jika $A.\text{prioritas} == B.\text{prioritas}$, maka $A$ dilayani lebih dulu jika $A.\text{nomorAntrian} < B.\text{nomorAntrian}$ (menjaga aturan FIFO).

---

## 3. State Machine Status Layanan

Status pasien wajib berubah secara konsisten mengikuti diagram transisi berikut untuk mencegah inkonsistensi data:

```mermaid
stateDiagram-v2
    [*] --> TERJADWAL : Booking Pasien (tanpa Check-in)
    [*] --> MENUNGGU : Registrasi Pasien (Walk-in Langsung)
    TERJADWAL --> MENUNGGU : Check-in Fisik (Masuk Heap Antrian)
    MENUNGGU --> DIPANGGIL : Dipanggil ke Dokter (Keluar Heap)
    DIPANGGIL --> SELESAI : Pelayanan Medis Selesai
    TERJADWAL --> BATAL : Batal Booking
    MENUNGGU --> BATAL : Batal Antrian Fisik
    SELESAI --> [*]
    BATAL --> [*]
```

### Aturan Transisi
* **Transisi yang Diperbolehkan:**
  * `TERJADWAL` $\rightarrow$ `MENUNGGU` (Check-in fisik saat tiba di RS).
  * `TERJADWAL` $\rightarrow$ `BATAL` (Batal booking janji temu).
  * `MENUNGGU` $\rightarrow$ `DIPANGGIL` (Pasien dipanggil dokter masuk ruangan).
  * `DIPANGGIL` $\rightarrow$ `SELESAI` (Pemeriksaan medis selesai).
  * `MENUNGGU` $\rightarrow$ `BATAL` (Batal mengantri setelah check-in).
* **Transisi yang Dilarang:**
  * Transisi apa pun dari status `SELESAI` atau `BATAL` (status final tidak bisa diubah lagi).
  * Transisi langsung dari `TERJADWAL` ke `DIPANGGIL` (pasien wajib datang fisik dan check-in `MENUNGGU` dulu agar masuk Priority Queue heap).
  * Transisi dari `DIPANGGIL` ke `BATAL` (pelayanan sedang berjalan tidak boleh langsung dibatalkan sebelum diselesaikan).

### 3.1 Perbedaan Status Antrian Pasien

1. **TERJADWAL (Scheduled)**:
   * Pasien baru melakukan booking online atau terdaftar untuk hari kunjungan, tetapi **belum tiba secara fisik** di rumah sakit.
   * Pasien berstatus ini **tidak dimasukkan ke dalam Priority Queue aktif (heap)** di memori.
   * Nilai `waktuDatang` dan `waktuDipanggil` bernilai `"-"` (belum ada).
2. **MENUNGGU (Waiting)**:
   * Pasien sudah check-in secara fisik di rumah sakit dan **masuk ke dalam Priority Queue aktif (heap)** di memori.
   * Urutan pasien ditentukan oleh tingkat prioritas (1-4) dan nomor antrian (FIFO) jika prioritasnya sama.
   * Pasien secara fisik berada di ruang tunggu menanti giliran nomor antriannya dipanggil.
3. **DIPANGGIL (Called)**:
   * Nomor antrian pasien dipanggil oleh petugas/dokter. Data pasien **dikeluarkan (pop) dari Priority Queue aktif** agar pasien berikutnya bisa bergeser naik.
   * Status pasien diubah menjadi `DIPANGGIL` di dalam Hash Table.
   * Waktu saat tombol panggil ditekan akan secara otomatis dicatat sebagai `waktuDipanggil`.
   * Pasien saat ini secara fisik berada di dalam ruang pemeriksaan dan sedang menjalani tindakan medis bersama dokter.

### 3.2 Pemanggilan Tidak Sesuai Urutan (Bypass Priority)
Jika petugas mencoba mengubah status pasien secara manual ke `DIPANGGIL` tetapi pasien tersebut bukan prioritas tertinggi di layanan yang sama, sistem akan mengeluarkan peringatan (`WARNING_PRIORITY`). 
* Jika di-bypass secara paksa (`force = true`), maka pasien akan langsung dipanggil.
* Supaya heap / priority queue tetap konsisten, C++ backend akan secara otomatis menghancurkan struktur lamanya dan melakukan proses `rebuildHeap()` dari hash table. Hal ini berguna supaya Visualisasi Heap di Web UI juga konsisten dan membersihkan elemen kotor dari pemanggilan yang lompat antrian.

---

## 4. Aturan Arsitektural

1. **Konsistensi Struktur Ganda**: Data wajib sinkron antara `Priority Queue` (untuk alur pemanggilan urutan) dan `Hash Table` (untuk pencarian, update status, dan pembatalan instan $O(1)$).
2. **Persistensi File**: Setiap perubahan data (`insert`, `update`, `cancel`, `call`) harus otomatis disimpan ke file penyimpanan (`data_pasien_rs.txt`) untuk mencegah kehilangan data.
3. **Desain Modular**: Kode C++ wajib dipisah ke dalam header (`.h`) dan source (`.cpp`) untuk menjaga kualitas, keterbacaan, dan modularitas sistem.

### 4.1 Desain Arsitektur Program

Berikut adalah diagram alur data dan arsitektur sistem dari Frontend Web, Backend API, hingga C++ Database Engine:

```mermaid
flowchart TB
    %% Class Definitions
    classDef front fill:mistyrose,stroke:lightcoral,stroke-width:2px,color:darkred,rx:8px,ry:8px;
    classDef back fill:peachpuff,stroke:sandybrown,stroke-width:2px,color:saddlebrown,rx:8px,ry:8px;
    classDef cpp fill:lightyellow,stroke:darkkhaki,stroke-width:2px,color:darkgoldenrod,rx:8px,ry:8px;
    classDef mem fill:honeydew,stroke:lightgreen,stroke-width:2px,color:darkgreen,rx:8px,ry:8px;
    classDef store fill:lightcyan,stroke:cadetblue,stroke-width:2px,color:teal,rx:8px,ry:8px;

    subgraph Frontend ["Layer 1: Presentation (React SPA + Vite)"]
        UI["Dashboard Web UI (App.jsx)"]
        SVG["Chart Module (SVG Benchmark Rendering)"]
        Client["HTTP Client (Fetch API)"]
    end
    class UI,SVG,Client front;

    subgraph Backend ["Layer 2: API Gateway (Node.js + Express)"]
        Server["Express.js Server (server.js)"]
        Spawn["Child Process Controller (child_process.spawn)"]
    end
    class Server,Spawn back;

    subgraph CPP_Engine ["Layer 3: Core Logic (Modular C++ Engine)"]
        Main["Main Entry (main.cpp)"]
        QueueCtrl["Queue Manager (QueueSystem.cpp)"]
        Bench["Benchmark Engine (Benchmark.cpp)"]
    end
    class Main,QueueCtrl,Bench cpp;

    subgraph Memory ["Layer 4: In-Memory Data Structures (C++)"]
        Heap["Priority Queue (std::priority_queue) <br/> Min-Heap: Medically Sorted"]
        HashMap["Hash Table (std::unordered_map) <br/> O(1) Quick Lookup / Updates"]
        QueueFIFO["Standard Queue (std::queue) <br/> FIFO Comparator (Benchmark Only)"]
    end
    class Heap,HashMap,QueueFIFO mem;

    subgraph Storage ["Layer 5: Storage & Persistence"]
        DB[("Database File <br/> (data_pasien_rs.txt)")]
        BenchJSON[("Benchmark Data <br/> (benchmark_results.json)")]
    end
    class DB,BenchJSON store;

    %% Subgraph Styling (Consistent Pastel Gradients)
    style Frontend fill:mistyrose,stroke:lightcoral,stroke-width:1px,color:darkred
    style Backend fill:papayawhip,stroke:sandybrown,stroke-width:1px,color:saddlebrown
    style CPP_Engine fill:lightyellow,stroke:darkkhaki,stroke-width:1px,color:darkgoldenrod
    style Memory fill:honeydew,stroke:lightgreen,stroke-width:1px,color:darkgreen
    style Storage fill:lightcyan,stroke:cadetblue,stroke-width:1px,color:teal

    %% Component Connections
    UI --- SVG
    UI --- Client
    Server --- Spawn
    Main --> QueueCtrl
    Main --> Bench
    QueueCtrl --> Heap
    QueueCtrl --> HashMap
    Bench --> QueueFIFO

    %% Communications and Data Flows
    Client ==>|"HTTP Requests (REST / JSON)"| Server
    Server ==>|"HTTP Responses (JSON)"| Client
    
    Spawn ==>|"Execute Process <br/> ./hospital_queue --json '...'"| Main
    Spawn ==>|"Execute Benchmark <br/> ./hospital_queue --benchmark <scale>"| Main
    
    Main ==>|"Stdout JSON Result"| Spawn
    
    QueueCtrl <-->|"Read / Write Records"| DB
    Bench --->|"Generate Results"| BenchJSON
    Server <---|"Read File"| BenchJSON

    %% Link Styling (Flow Lines)
    linkStyle 8 stroke:lightcoral,stroke-width:3px,color:darkred;
    linkStyle 9 stroke:lightcoral,stroke-width:3px,color:darkred;
    linkStyle 10 stroke:sandybrown,stroke-width:3px,color:saddlebrown;
    linkStyle 11 stroke:sandybrown,stroke-width:3px,color:saddlebrown;
    linkStyle 12 stroke:sandybrown,stroke-width:3px,color:saddlebrown;
    linkStyle 13 stroke:green,stroke-width:2px,color:darkgreen;
    linkStyle 14 stroke:cadetblue,stroke-width:2px,color:teal;
    linkStyle 15 stroke:green,stroke-width:2px,color:darkgreen;
```

