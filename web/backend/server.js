const express = require('express');
const cors = require('cors');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = process.env.PORT || 7331; // Port antimainstream untuk menghindari bentrok dengan Mac AirPlay

app.use(cors());
app.use(express.json());

// Middleware untuk logging setiap request
app.use((req, res, next) => {
  console.log(`[${new Date().toISOString()}] ${req.method} ${req.url}`);
  next();
});

// Path ke file eksekutabel C++ dan working directory
const BINARY_PATH = path.resolve(__dirname, '../../hospital_queue');
const WORK_DIR = path.resolve(__dirname, '../..');

// Helper untuk menjalankan command JSON pada program C++
function runCppCommand(payload) {
  return new Promise((resolve, reject) => {
    const jsonStr = JSON.stringify(payload);
    // Spawn binary dengan argumen --json
    const child = spawn(BINARY_PATH, ['--json', jsonStr], { cwd: WORK_DIR });

    let stdoutData = '';
    let stderrData = '';

    child.stdout.on('data', (data) => {
      stdoutData += data.toString();
    });

    child.stderr.on('data', (data) => {
      stderrData += data.toString();
    });

    child.on('close', (code) => {
      if (code !== 0) {
        return reject({
          code,
          message: 'Program C++ keluar dengan kode error',
          error: stderrData.trim()
        });
      }
      try {
        const parsed = JSON.parse(stdoutData.trim());
        resolve(parsed);
      } catch (err) {
        reject({
          message: 'Gagal mengurai output JSON dari C++',
          raw: stdoutData,
          error: err.message
        });
      }
    });

    child.on('error', (err) => {
      reject({
        message: 'Gagal memulai program C++',
        error: err.message
      });
    });
  });
}

// 1. Ambil semua data pasien
app.get('/api/patients', async (req, res) => {
  try {
    const result = await runCppCommand({ action: 'list_all' });
    res.json(result);
  } catch (error) {
    res.status(500).json({ status: 'error', ...error });
  }
});

// 2. Registrasi pasien baru
app.post('/api/patients', async (req, res) => {
  const { nama, layanan, prioritas, waktuDatang, tanggal } = req.body;
  if (!nama || !layanan || !prioritas || !tanggal) {
    return res.status(400).json({
      status: 'error',
      message: 'Nama, layanan, prioritas, dan tanggal wajib diisi.'
    });
  }

  // Validasi format tanggal YYYY-MM-DD
  const dateRegex = /^\d{4}-\d{2}-\d{2}$/;
  if (!dateRegex.test(tanggal)) {
    return res.status(400).json({
      status: 'error',
      message: 'Format tanggal tidak valid. Harus menggunakan format YYYY-MM-DD.'
    });
  }

  // Validasi format waktuDatang HH:MM atau -
  const wd = waktuDatang || '-';
  const timeRegex = /^([01]\d|2[0-3]):([0-5]\d)$/;
  if (wd !== '-' && !timeRegex.test(wd)) {
    return res.status(400).json({
      status: 'error',
      message: 'Format waktu tidak valid. Harus menggunakan format HH:MM (24 jam) atau -.'
    });
  }

  // Validasi tanggal walk-in harus hari ini
  if (wd !== '-') {
    const today = new Date();
    const todayStr = today.getFullYear() + '-' + String(today.getMonth() + 1).padStart(2, '0') + '-' + String(today.getDate()).padStart(2, '0');
    if (tanggal !== todayStr) {
      return res.status(400).json({
        status: 'error',
        message: `Tanggal walk-in harus hari ini (${todayStr}).`
      });
    }
  }

  try {
    const result = await runCppCommand({
      action: 'insert',
      id: '', // Kosongkan agar C++ auto-generate ID
      nama,
      layanan,
      prioritas: parseInt(prioritas, 10),
      waktuDatang: wd,
      tanggal
    });
    res.json(result);
  } catch (error) {
    res.status(500).json({ status: 'error', ...error });
  }
});

// 3. Check-in pasien terjadwal
app.post('/api/checkin', async (req, res) => {
  const { id, waktuDatang } = req.body;
  if (!id || !waktuDatang) {
    return res.status(400).json({
      status: 'error',
      message: 'ID dan waktuDatang wajib diisi.'
    });
  }

  // Validasi format waktuDatang HH:MM atau -
  const timeRegex = /^([01]\d|2[0-3]):([0-5]\d)$/;
  if (waktuDatang !== '-' && !timeRegex.test(waktuDatang)) {
    return res.status(400).json({
      status: 'error',
      message: 'Format waktu tidak valid. Harus menggunakan format HH:MM (24 jam) atau -.'
    });
  }

  try {
    const result = await runCppCommand({
      action: 'check_in',
      id,
      waktuDatang
    });
    res.json(result);
  } catch (error) {
    res.status(500).json({ status: 'error', ...error });
  }
});

// 4. Panggil pasien berikutnya
app.post('/api/call-next', async (req, res) => {
  try {
    const result = await runCppCommand({ action: 'call_next' });
    res.json(result);
  } catch (error) {
    res.status(500).json({ status: 'error', ...error });
  }
});

// 5. Update status (Selesai/Batal)
app.post('/api/update-status', async (req, res) => {
  const { id, status } = req.body;
  if (!id || status === undefined) {
    return res.status(400).json({
      status: 'error',
      message: 'ID dan status wajib diisi.'
    });
  }
  try {
    const result = await runCppCommand({
      action: 'update_status',
      id,
      status: parseInt(status, 10)
    });
    res.json(result);
  } catch (error) {
    res.status(500).json({ status: 'error', ...error });
  }
});

// 6. Hapus/Batal total pasien
app.post('/api/delete', async (req, res) => {
  const { id } = req.body;
  if (!id) {
    return res.status(400).json({
      status: 'error',
      message: 'ID wajib diisi.'
    });
  }
  try {
    const result = await runCppCommand({
      action: 'delete',
      id
    });
    res.json(result);
  } catch (error) {
    res.status(500).json({ status: 'error', ...error });
  }
});

// 7. Load data dummy
app.post('/api/dummy', async (req, res) => {
  try {
    const result = await runCppCommand({ action: 'dummy_data' });
    res.json(result);
  } catch (error) {
    res.status(500).json({ status: 'error', ...error });
  }
});

// 8. Jalankan benchmark
app.get('/api/benchmark', async (req, res) => {
  let scale = parseInt(req.query.scale, 10) || 10000;
  if (scale <= 0) scale = 10000;

  // Jalankan benchmark dengan command line --benchmark
  const child = spawn(BINARY_PATH, ['--benchmark', scale.toString()], { cwd: WORK_DIR });

  child.on('close', (code) => {
    if (code !== 0) {
      return res.status(500).json({
        status: 'error',
        message: 'Benchmark C++ keluar dengan kode error'
      });
    }

    // Baca file benchmark_results.json
    const resultsPath = path.resolve(WORK_DIR, 'benchmark_results.json');
    fs.readFile(resultsPath, 'utf8', (err, data) => {
      if (err) {
        return res.status(500).json({
          status: 'error',
          message: 'Gagal membaca file hasil benchmark',
          error: err.message
        });
      }
      try {
        res.json(JSON.parse(data));
      } catch (parseErr) {
        res.status(500).json({
          status: 'error',
          message: 'Gagal mengurai file hasil benchmark JSON',
          raw: data,
          error: parseErr.message
        });
      }
    });
  });

  child.on('error', (err) => {
    res.status(500).json({
      status: 'error',
      message: 'Gagal mengeksekusi program benchmark C++',
      error: err.message
    });
  });
});

app.listen(PORT, () => {
  console.log(`[Server] Hospital Queue API running on http://localhost:${PORT}`);
});
