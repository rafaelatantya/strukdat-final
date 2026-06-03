import React, { useState, useEffect } from 'react';
import './App.css';

const API_BASE = 'http://localhost:5000/api';

function App() {
  // State
  const [patients, setPatients] = useState([]);
  const [activeTab, setActiveTab] = useState('active'); // active, scheduled, history, benchmark
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState(null);
  const [success, setSuccess] = useState(null);
  const [currentTime, setCurrentTime] = useState('');

  // Form State
  const [formData, setFormData] = useState({
    id: '',
    nama: '',
    layanan: '',
    prioritas: '4',
    tanggal: '',
    waktuDatang: '',
    isWalkIn: true
  });

  // Modal Check-In State
  const [checkInPatientData, setCheckInPatientData] = useState(null);
  const [checkInTimeStr, setCheckInTimeStr] = useState('');

  // Benchmark State
  const [benchmarkScale, setBenchmarkScale] = useState(10000);
  const [benchmarkResults, setBenchmarkResults] = useState(null);
  const [isBenchmarking, setIsBenchmarking] = useState(false);

  // Auto-clock effect
  useEffect(() => {
    const updateClock = () => {
      const now = new Date();
      const hrs = String(now.getHours()).padStart(2, '0');
      const mins = String(now.getMinutes()).padStart(2, '0');
      setCurrentTime(`${hrs}:${mins}`);
    };
    updateClock();
    const interval = setInterval(updateClock, 1000);
    return () => clearInterval(interval);
  }, []);

  // Fetch patients on load
  useEffect(() => {
    fetchPatients();
  }, []);

  // Auto-generate Patient ID and Date when form type or visibility changes
  useEffect(() => {
    if (!formData.id) {
      generateNewId();
    }
    const today = new Date().toISOString().split('T')[0];
    setFormData(prev => ({
      ...prev,
      tanggal: prev.tanggal || today,
      waktuDatang: prev.isWalkIn ? currentTime : '-'
    }));
  }, [formData.isWalkIn, currentTime]);

  const generateNewId = () => {
    const randNum = Math.floor(100 + Math.random() * 900);
    setFormData(prev => ({ ...prev, id: `P${randNum}` }));
  };

  // Toast Alerts helper
  const showToast = (message, isError = false) => {
    if (isError) {
      setError(message);
      setTimeout(() => setError(null), 4000);
    } else {
      setSuccess(message);
      setTimeout(() => setSuccess(null), 4000);
    }
  };

  // API calls
  const fetchPatients = async () => {
    setIsLoading(true);
    try {
      const res = await fetch(`${API_BASE}/patients`);
      const data = await res.json();
      if (data.status === 'success') {
        setPatients(data.data || []);
      } else {
        showToast(data.message || 'Gagal memuat data pasien.', true);
      }
    } catch (err) {
      showToast('Gagal terhubung ke API backend.', true);
    } finally {
      setIsLoading(false);
    }
  };

  const handleRegister = async (e) => {
    e.preventDefault();
    if (!formData.id || !formData.nama || !formData.layanan || !formData.tanggal) {
      showToast('Mohon lengkapi semua field registrasi.', true);
      return;
    }

    try {
      const res = await fetch(`${API_BASE}/patients`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          ...formData,
          waktuDatang: formData.isWalkIn ? currentTime : '-'
        })
      });
      const data = await res.json();
      if (data.status === 'success') {
        showToast(`Pasien ${formData.nama} berhasil didaftarkan.`);
        fetchPatients();
        // Reset form
        setFormData({
          id: '',
          nama: '',
          layanan: '',
          prioritas: '4',
          tanggal: new Date().toISOString().split('T')[0],
          waktuDatang: '',
          isWalkIn: true
        });
        generateNewId();
      } else {
        showToast(data.message || 'Gagal meregistrasikan pasien.', true);
      }
    } catch (err) {
      showToast('Kesalahan jaringan saat registrasi.', true);
    }
  };

  const handleCheckInClick = (patient) => {
    setCheckInPatientData(patient);
    setCheckInTimeStr(currentTime);
  };

  const executeCheckIn = async () => {
    if (!checkInPatientData || !checkInTimeStr) return;
    try {
      const res = await fetch(`${API_BASE}/checkin`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          id: checkInPatientData.id,
          waktuDatang: checkInTimeStr
        })
      });
      const data = await res.json();
      if (data.status === 'success') {
        showToast(`Pasien ${checkInPatientData.nama} berhasil check-in fisik.`);
        setCheckInPatientData(null);
        fetchPatients();
      } else {
        showToast(data.message || 'Gagal melakukan check-in.', true);
      }
    } catch (err) {
      showToast('Kesalahan jaringan saat check-in.', true);
    }
  };

  const handleCallNext = async () => {
    try {
      const res = await fetch(`${API_BASE}/call-next`, { method: 'POST' });
      const data = await res.json();
      if (data.status === 'success') {
        showToast(`Memanggil pasien: ${data.data.nama} (Antrian #${data.data.nomorAntrian})`);
        fetchPatients();
      } else {
        showToast(data.message || 'Antrian tunggu sedang kosong.', true);
      }
    } catch (err) {
      showToast('Kesalahan jaringan saat memanggil pasien.', true);
    }
  };

  const handleStatusUpdate = async (id, statusVal, labelName) => {
    try {
      const res = await fetch(`${API_BASE}/update-status`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ id, status: statusVal })
      });
      const data = await res.json();
      if (data.status === 'success') {
        showToast(`Status pasien berhasil diubah menjadi ${labelName}.`);
        fetchPatients();
      } else {
        showToast(data.message || 'Gagal memperbarui status.', true);
      }
    } catch (err) {
      showToast('Kesalahan jaringan saat update status.', true);
    }
  };

  const handleCancelPatient = async (id) => {
    if (!window.confirm('Apakah Anda yakin ingin membatalkan/menghapus antrian pasien ini?')) return;
    try {
      const res = await fetch(`${API_BASE}/delete`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ id })
      });
      const data = await res.json();
      if (data.status === 'success') {
        showToast('Antrian pasien berhasil dibatalkan.');
        fetchPatients();
      } else {
        showToast(data.message || 'Gagal membatalkan antrian.', true);
      }
    } catch (err) {
      showToast('Kesalahan jaringan saat membatalkan antrian.', true);
    }
  };

  const handleLoadDummy = async () => {
    if (!window.confirm('Muat data dummy awal? Data saat ini tidak akan dihapus tetapi akan digabungkan.')) return;
    setIsLoading(true);
    try {
      const res = await fetch(`${API_BASE}/dummy`, { method: 'POST' });
      const data = await res.json();
      if (data.status === 'success') {
        showToast('Data dummy berhasil di-load.');
        fetchPatients();
      } else {
        showToast(data.message || 'Gagal memuat data dummy.', true);
      }
    } catch (err) {
      showToast('Kesalahan jaringan saat memuat dummy.', true);
    } finally {
      setIsLoading(false);
    }
  };

  const handleRunBenchmark = async () => {
    setIsBenchmarking(true);
    setBenchmarkResults(null);
    try {
      const res = await fetch(`${API_BASE}/benchmark?scale=${benchmarkScale}`);
      const data = await res.json();
      if (data.scale) {
        setBenchmarkResults(data);
        showToast('Benchmark performa berhasil diselesaikan.');
      } else {
        showToast(data.message || 'Gagal menjalankan benchmark.', true);
      }
    } catch (err) {
      showToast('Kesalahan jaringan saat benchmark.', true);
    } finally {
      setIsBenchmarking(false);
    }
  };

  // Get statistics counts
  const totalWaiting = patients.filter(p => p.status === 0).length;
  const totalCalled = patients.filter(p => p.status === 1).length;
  const totalScheduled = patients.filter(p => p.status === 4).length;
  const totalCompleted = patients.filter(p => p.status === 2).length;
  const totalCancelled = patients.filter(p => p.status === 3).length;

  // Find currently serving patient (if any)
  const currentServing = patients.find(p => p.status === 1);

  // Filter patient arrays based on current tab
  const getFilteredPatients = () => {
    if (activeTab === 'active') {
      // Return waiting & called patients
      return patients.filter(p => p.status === 0 || p.status === 1);
    } else if (activeTab === 'scheduled') {
      // Return terjadwal patients
      return patients.filter(p => p.status === 4);
    } else if (activeTab === 'history') {
      // Return selesai & batal patients
      return patients.filter(p => p.status === 2 || p.status === 3);
    }
    return [];
  };

  const filteredPatients = getFilteredPatients();

  // Helper to format priority label
  const getPriorityBadgeClass = (prio) => {
    return `badge-priority badge-priority-${prio}`;
  };

  // Helper to format status label
  const getStatusBadge = (status) => {
    switch (status) {
      case 0: return <span className="badge-status badge-status-waiting">Menunggu</span>;
      case 1: return <span className="badge-status badge-status-called">Dipanggil</span>;
      case 2: return <span className="badge-status badge-status-completed">Selesai</span>;
      case 3: return <span className="badge-status badge-status-cancelled">Batal</span>;
      case 4: return <span className="badge-status badge-status-scheduled">Terjadwal</span>;
      default: return null;
    }
  };

  const getPriorityLabel = (prio) => {
    switch (prio) {
      case 1: return 'Darurat';
      case 2: return 'Mendesak';
      case 3: return 'Prioritas Rentan';
      case 4: return 'Reguler';
      default: return 'Lainnya';
    }
  };

  return (
    <div className="dashboard-layout">
      {/* Toast Alert Notifications */}
      <div className="toast-container">
        {error && (
          <div className="toast toast-error">
            <span style={{ fontSize: '1.25rem' }}>⚠️</span>
            <div><strong>Error:</strong> {error}</div>
          </div>
        )}
        {success && (
          <div className="toast">
            <span style={{ fontSize: '1.25rem' }}>✨</span>
            <div>{success}</div>
          </div>
        )}
      </div>

      {/* Header */}
      <header className="dashboard-header">
        <div>
          <h1 className="dashboard-title glow-text-primary">
            🏥 Sistem Antrian RS Sentosa
          </h1>
          <p style={{ color: 'var(--text-secondary)', marginTop: '0.25rem' }}>
            Modular Engine (C++) & Modern Administrative Web Terminal
          </p>
        </div>
        <div className="dashboard-actions">
          <div className="glass-panel" style={{ padding: '0.5rem 1rem', display: 'flex', alignItems: 'center', gap: '0.75rem', borderRadius: '8px' }}>
            <span style={{ color: 'var(--color-primary)', fontWeight: 'bold' }}>🕒 Clock:</span>
            <span style={{ fontSize: '1.1rem', fontWeight: '600', letterSpacing: '0.05em' }}>{currentTime}</span>
          </div>
          <button className="btn btn-secondary" onClick={handleLoadDummy} disabled={isLoading}>
            🧪 Load Dummy
          </button>
          <button className="btn btn-primary" onClick={fetchPatients} disabled={isLoading}>
            🔄 Refresh
          </button>
        </div>
      </header>

      {/* Quick Statistics Bar */}
      <section className="stats-grid">
        <div className="glass-panel stat-card">
          <div className="stat-icon" style={{ background: 'rgba(56, 189, 248, 0.15)', color: 'var(--color-primary)' }}>⏳</div>
          <div className="stat-info">
            <h4>Menunggu</h4>
            <p>{totalWaiting}</p>
          </div>
        </div>
        <div className="glass-panel stat-card">
          <div className="stat-icon" style={{ background: 'rgba(251, 113, 133, 0.15)', color: 'var(--status-terjadwal)' }}>📅</div>
          <div className="stat-info">
            <h4>Terjadwal (Booking)</h4>
            <p>{totalScheduled}</p>
          </div>
        </div>
        <div className="glass-panel stat-card">
          <div className="stat-icon" style={{ background: 'rgba(168, 85, 247, 0.15)', color: 'var(--color-secondary)' }}>🩺</div>
          <div className="stat-info">
            <h4>Dipanggil</h4>
            <p>{totalCalled}</p>
          </div>
        </div>
        <div className="glass-panel stat-card">
          <div className="stat-icon" style={{ background: 'rgba(16, 185, 129, 0.15)', color: 'var(--status-selesai)' }}>✅</div>
          <div className="stat-info">
            <h4>Selesai Dilayani</h4>
            <p>{totalCompleted}</p>
          </div>
        </div>
      </section>

      {/* Main Calling Deck - Current Serving Patient */}
      <section className="mb-md">
        {currentServing ? (
          <div className="glass-panel pulse-emergency" style={{ borderLeft: `6px solid var(--priority-${currentServing.prioritas})`, padding: '2rem' }}>
            <div className="flex-between">
              <div>
                <span className={getPriorityBadgeClass(currentServing.prioritas)} style={{ marginBottom: '0.75rem' }}>
                  {getPriorityLabel(currentServing.prioritas)}
                </span>
                <h2 style={{ fontSize: '2.25rem', fontWeight: '700', marginBottom: '0.25rem' }}>
                  {currentServing.nama}
                </h2>
                <p style={{ color: 'var(--text-secondary)', fontSize: '1.1rem' }}>
                  ID: <strong>{currentServing.id}</strong> | Poli/Layanan: <strong style={{ color: 'var(--color-primary)' }}>{currentServing.layanan}</strong>
                </p>
                <div style={{ display: 'flex', gap: '1.5rem', marginTop: '1rem', fontSize: '0.95rem', color: 'var(--text-muted)' }}>
                  <span>📥 Waktu Datang: <strong>{currentServing.waktuDatang}</strong></span>
                  <span>📢 Dipanggil: <strong style={{ color: 'var(--color-secondary)' }}>{currentServing.waktuDipanggil}</strong></span>
                  <span>📅 Tanggal: <strong>{currentServing.tanggal}</strong></span>
                </div>
              </div>
              <div style={{ textAlign: 'right' }}>
                <div style={{ fontSize: '0.85rem', textTransform: 'uppercase', color: 'var(--text-secondary)', letterSpacing: '0.05em' }}>Nomor Antrian</div>
                <div style={{ fontSize: '4.5rem', fontWeight: '800', color: 'var(--color-primary)', lineHeight: '1', margin: '0.25rem 0' }}>
                  #{currentServing.nomorAntrian}
                </div>
                <div style={{ display: 'flex', gap: '0.75rem', marginTop: '1rem' }}>
                  <button className="btn btn-success" onClick={() => handleStatusUpdate(currentServing.id, 2, 'Selesai Pelayanan')}>
                    Selesai
                  </button>
                  <button className="btn btn-danger" onClick={() => handleStatusUpdate(currentServing.id, 3, 'Batal Pelayanan')}>
                    Batal
                  </button>
                </div>
              </div>
            </div>
          </div>
        ) : (
          <div className="glass-panel flex-between" style={{ padding: '2rem', borderLeft: '6px solid var(--text-muted)' }}>
            <div>
              <h3 style={{ fontSize: '1.5rem', fontWeight: '600' }}>📢 Tidak Ada Panggilan Aktif</h3>
              <p style={{ color: 'var(--text-secondary)', marginTop: '0.25rem' }}>
                Priority queue saat ini siap. Tekan tombol panggil untuk memproses pasien berikutnya berdasarkan sistem prioritas (Min-Heap).
              </p>
            </div>
            <div>
              <button 
                className="btn btn-primary" 
                style={{ padding: '1rem 2rem', fontSize: '1.1rem' }} 
                onClick={handleCallNext}
                disabled={totalWaiting === 0}
              >
                Panggil Antrian Berikutnya ({totalWaiting} mengantri)
              </button>
            </div>
          </div>
        )}
      </section>

      {/* Navigation tabs */}
      <div className="tab-container">
        <button className={`tab-button ${activeTab === 'active' ? 'active' : ''}`} onClick={() => setActiveTab('active')}>
          ⏳ Antrian Aktif ({totalWaiting + totalCalled})
        </button>
        <button className={`tab-button ${activeTab === 'scheduled' ? 'active' : ''}`} onClick={() => setActiveTab('scheduled')}>
          📅 Terjadwal (Booking) ({totalScheduled})
        </button>
        <button className={`tab-button ${activeTab === 'history' ? 'active' : ''}`} onClick={() => setActiveTab('history')}>
          📜 Riwayat Pelayanan ({totalCompleted + totalCancelled})
        </button>
        <button className={`tab-button ${activeTab === 'benchmark' ? 'active' : ''}`} onClick={() => setActiveTab('benchmark')}>
          ⚡ Analisis Performa (Benchmark)
        </button>
      </div>

      {/* Main Grid Content */}
      {activeTab !== 'benchmark' ? (
        <div className="dashboard-grid">
          {/* Left Side: Tables */}
          <div className="glass-panel" style={{ overflow: 'hidden' }}>
            <h3 style={{ marginBottom: '1.25rem', fontSize: '1.25rem', fontWeight: '600' }}>
              {activeTab === 'active' && 'Daftar Pasien Menunggu & Dilayani'}
              {activeTab === 'scheduled' && 'Daftar Pasien Booking Janji Temu'}
              {activeTab === 'history' && 'Riwayat Pasien Selesai & Batal'}
            </h3>

            {isLoading ? (
              <div className="flex-center" style={{ height: '300px', flexDirection: 'column', gap: '1rem' }}>
                <div style={{ width: '40px', height: '40px', border: '3px solid rgba(56,189,248,0.2)', borderTopColor: 'var(--color-primary)', borderRadius: '50%', animation: 'spin 1s linear infinite' }} />
                <p style={{ color: 'var(--text-secondary)' }}>Memuat data pasien...</p>
              </div>
            ) : filteredPatients.length === 0 ? (
              <div className="flex-center" style={{ height: '300px', color: 'var(--text-secondary)', flexDirection: 'column' }}>
                <span style={{ fontSize: '3rem', marginBottom: '1rem' }}>📭</span>
                <p>Tidak ada data pasien yang sesuai.</p>
              </div>
            ) : (
              <div style={{ overflowX: 'auto' }}>
                <table className="queue-table">
                  <thead>
                    <tr>
                      <th>ID</th>
                      <th>Nama Pasien</th>
                      <th>Poli/Layanan</th>
                      <th>Prioritas</th>
                      <th>No. Antrian</th>
                      <th>Datang</th>
                      {activeTab !== 'scheduled' && <th>Dipanggil</th>}
                      <th>Tanggal</th>
                      <th>Status</th>
                      <th>Aksi</th>
                    </tr>
                  </thead>
                  <tbody>
                    {filteredPatients.map((patient) => (
                      <tr key={patient.id}>
                        <td style={{ fontWeight: '600', color: 'var(--color-primary)' }}>{patient.id}</td>
                        <td style={{ fontWeight: '500' }}>{patient.nama}</td>
                        <td>{patient.layanan}</td>
                        <td>
                          <span className={getPriorityBadgeClass(patient.prioritas)}>
                            {patient.prioritas} - {getPriorityLabel(patient.prioritas)}
                          </span>
                        </td>
                        <td style={{ fontWeight: 'bold', textAlign: 'center' }}>
                          {patient.status === 4 ? '-' : `#${patient.nomorAntrian}`}
                        </td>
                        <td>{patient.waktuDatang}</td>
                        {activeTab !== 'scheduled' && (
                          <td style={{ color: patient.waktuDipanggil !== '-' ? 'var(--color-secondary)' : 'inherit' }}>
                            {patient.waktuDipanggil}
                          </td>
                        )}
                        <td style={{ fontSize: '0.85rem' }}>{patient.tanggal}</td>
                        <td>{getStatusBadge(patient.status)}</td>
                        <td>
                          <div style={{ display: 'flex', gap: '0.5rem' }}>
                            {patient.status === 4 && (
                              <button className="btn btn-primary" style={{ padding: '0.35rem 0.75rem', fontSize: '0.8rem' }} onClick={() => handleCheckInClick(patient)}>
                                Check-In
                              </button>
                            )}
                            {patient.status === 0 && (
                              <span style={{ color: 'var(--text-muted)', fontSize: '0.85rem', fontStyle: 'italic' }}>Mengantri...</span>
                            )}
                            {patient.status === 1 && (
                              <>
                                <button className="btn btn-success" style={{ padding: '0.35rem 0.75rem', fontSize: '0.8rem' }} onClick={() => handleStatusUpdate(patient.id, 2, 'Selesai Pelayanan')}>
                                  Selesai
                                </button>
                                <button className="btn btn-danger" style={{ padding: '0.35rem 0.75rem', fontSize: '0.8rem' }} onClick={() => handleStatusUpdate(patient.id, 3, 'Batal Pelayanan')}>
                                  Batal
                                </button>
                              </>
                            )}
                            {(patient.status === 0 || patient.status === 4) && (
                              <button className="btn btn-secondary" style={{ padding: '0.35rem 0.75rem', fontSize: '0.8rem', color: 'var(--priority-1)' }} onClick={() => handleCancelPatient(patient.id)}>
                                Batal
                              </button>
                            )}
                            {(patient.status === 2 || patient.status === 3) && (
                              <span style={{ color: 'var(--text-muted)', fontSize: '0.85rem' }}>Selesai</span>
                            )}
                          </div>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            )}
          </div>

          {/* Right Side: Registration Form */}
          <div className="glass-panel" style={{ alignSelf: 'start' }}>
            <h3 style={{ marginBottom: '1.25rem', fontSize: '1.25rem', fontWeight: '600' }}>
              📝 Registrasi Pasien Baru
            </h3>
            <form onSubmit={handleRegister}>
              <div className="form-group">
                <label className="form-label">Tipe Registrasi</label>
                <div style={{ display: 'flex', gap: '1rem' }}>
                  <label style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', cursor: 'pointer' }}>
                    <input 
                      type="radio" 
                      name="regType" 
                      checked={formData.isWalkIn} 
                      onChange={() => setFormData(prev => ({ ...prev, isWalkIn: true }))}
                    />
                    <span>Walk-In (Datang Hari Ini)</span>
                  </label>
                  <label style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', cursor: 'pointer' }}>
                    <input 
                      type="radio" 
                      name="regType" 
                      checked={!formData.isWalkIn} 
                      onChange={() => setFormData(prev => ({ ...prev, isWalkIn: false }))}
                    />
                    <span>Booking (Terjadwal)</span>
                  </label>
                </div>
              </div>

              <div className="form-group">
                <label className="form-label">ID Pasien (Generated)</label>
                <div style={{ display: 'flex', gap: '0.5rem' }}>
                  <input 
                    type="text" 
                    className="form-control" 
                    value={formData.id} 
                    onChange={(e) => setFormData(prev => ({ ...prev, id: e.target.value.toUpperCase() }))}
                  />
                  <button type="button" className="btn btn-secondary" onClick={generateNewId} style={{ padding: '0.5rem 0.75rem' }}>
                    🎲
                  </button>
                </div>
              </div>

              <div className="form-group">
                <label className="form-label">Nama Pasien</label>
                <input 
                  type="text" 
                  className="form-control" 
                  placeholder="Masukkan nama lengkap pasien"
                  value={formData.nama} 
                  onChange={(e) => setFormData(prev => ({ ...prev, nama: e.target.value }))}
                />
              </div>

              <div className="form-group">
                <label className="form-label">Poli / Jenis Layanan</label>
                <select 
                  className="form-control"
                  value={formData.layanan}
                  onChange={(e) => setFormData(prev => ({ ...prev, layanan: e.target.value }))}
                >
                  <option value="">-- Pilih Poli --</option>
                  <option value="Poli Umum">Poli Umum</option>
                  <option value="UGD">UGD (Gawat Darurat)</option>
                  <option value="Poli Penyakit Dalam">Poli Penyakit Dalam</option>
                  <option value="Poli Anak">Poli Anak</option>
                  <option value="Poli Gigi">Poli Gigi</option>
                  <option value="Poli Mata">Poli Mata</option>
                  <option value="Laboratorium">Laboratorium</option>
                  <option value="Poli Geriatri">Poli Geriatri (Rentan)</option>
                </select>
              </div>

              <div className="form-group">
                <label className="form-label">Tingkat Prioritas</label>
                <select 
                  className="form-control"
                  value={formData.prioritas}
                  onChange={(e) => setFormData(prev => ({ ...prev, prioritas: e.target.value }))}
                >
                  <option value="1">1 - Darurat (Critical condition)</option>
                  <option value="2">2 - Mendesak (Urgent illness)</option>
                  <option value="3">3 - Prioritas Rentan (Lansia / Bumil / Disabilitas)</option>
                  <option value="4">4 - Reguler (Pemeriksaan Umum)</option>
                </select>
              </div>

              <div className="form-group">
                <label className="form-label">Tanggal Kunjungan</label>
                <input 
                  type="date" 
                  className="form-control" 
                  value={formData.tanggal} 
                  onChange={(e) => setFormData(prev => ({ ...prev, tanggal: e.target.value }))}
                />
              </div>

              <div className="form-group">
                <label className="form-label">Waktu Kedatangan Fisik</label>
                <input 
                  type="text" 
                  className="form-control" 
                  disabled
                  value={formData.isWalkIn ? currentTime : '-'} 
                />
                <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)', marginTop: '0.25rem', display: 'block' }}>
                  {formData.isWalkIn 
                    ? 'Walk-in didaftarkan dengan status MENUNGGU dan langsung mengantri.' 
                    : 'Booking online mendapat status TERJADWAL dan belum masuk priority heap.'}
                </span>
              </div>

              <button type="submit" className="btn btn-primary" style={{ width: '100%', marginTop: '1rem' }}>
                Tambahkan Pasien
              </button>
            </form>
          </div>
        </div>
      ) : (
        /* Performance Benchmark View */
        <div className="glass-panel" style={{ minHeight: '500px' }}>
          <div className="flex-between" style={{ borderBottom: '1px solid rgba(255, 255, 255, 0.05)', paddingBottom: '1.25rem', marginBottom: '1.5rem' }}>
            <div>
              <h3 style={{ fontSize: '1.5rem', fontWeight: '600' }}>⚡ Analisis Performa: Heap vs Standard Queue</h3>
              <p style={{ color: 'var(--text-secondary)', marginTop: '0.25rem' }}>
                Uji coba performa C++ antara **Priority Queue (Min-Heap)** dan **Standard Queue (FIFO)** secara realtime.
              </p>
            </div>
            <div style={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
              <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                <span style={{ fontSize: '0.9rem', color: 'var(--text-secondary)' }}>Skala Data:</span>
                <select 
                  className="form-control" 
                  style={{ width: '120px', padding: '0.5rem' }} 
                  value={benchmarkScale}
                  onChange={(e) => setBenchmarkScale(parseInt(e.target.value, 10))}
                  disabled={isBenchmarking}
                >
                  <option value="100">100 Data</option>
                  <option value="1000">1,000 Data</option>
                  <option value="10000">10,000 Data</option>
                  <option value="50000">50,000 Data</option>
                </select>
              </div>
              <button className="btn btn-primary" onClick={handleRunBenchmark} disabled={isBenchmarking}>
                {isBenchmarking ? 'Running...' : 'Mulai Test Performa'}
              </button>
            </div>
          </div>

          {isBenchmarking && (
            <div className="flex-center" style={{ height: '350px', flexDirection: 'column', gap: '1.5rem' }}>
              <div style={{ width: '50px', height: '50px', border: '3px solid rgba(56,189,248,0.1)', borderTopColor: 'var(--color-primary)', borderRadius: '50%', animation: 'spin 1s linear infinite' }} />
              <div style={{ textAlign: 'center' }}>
                <h4 style={{ fontWeight: '500' }}>Menjalankan Benchmark Engine di C++...</h4>
                <p style={{ color: 'var(--text-secondary)', fontSize: '0.85rem', marginTop: '0.25rem' }}>
                  Memproses {benchmarkScale.toLocaleString()} input data secara batch di memory.
                </p>
              </div>
            </div>
          )}

          {!isBenchmarking && !benchmarkResults && (
            <div className="flex-center" style={{ height: '350px', color: 'var(--text-secondary)', flexDirection: 'column' }}>
              <span style={{ fontSize: '4rem', marginBottom: '1rem' }}>📈</span>
              <p style={{ fontWeight: '500' }}>Hasil test performa belum tersedia.</p>
              <p style={{ fontSize: '0.85rem', marginTop: '0.25rem' }}>
                Silakan tentukan skala data di atas dan tekan tombol "Mulai Test Performa".
              </p>
            </div>
          )}

          {!isBenchmarking && benchmarkResults && (
            <div>
              {/* Benchmark Charts Section */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(320px, 1fr))', gap: '2rem', marginBottom: '2.5rem' }}>
                
                {/* 1. Insert Operation Chart */}
                <div className="glass-panel" style={{ background: 'rgba(7, 10, 19, 0.4)', border: '1px solid rgba(255, 255, 255, 0.03)' }}>
                  <h4 style={{ fontWeight: '600', marginBottom: '1.5rem', textAlign: 'center', fontSize: '1.1rem' }}>
                    📥 Batch Insertion ({benchmarkResults.scale} Data)
                  </h4>
                  <div className="flex-center">
                    <RenderBarChart 
                      valA={benchmarkResults.priority_queue.insert_time_us} 
                      valB={benchmarkResults.standard_queue.insert_time_us}
                      labelA="Priority Queue (Heap)"
                      labelB="Standard Queue (FIFO)"
                      colorA="var(--color-primary)"
                      colorB="var(--text-muted)"
                    />
                  </div>
                  <p style={{ fontSize: '0.8rem', color: 'var(--text-secondary)', marginTop: '1rem', textAlign: 'center', fontStyle: 'italic' }}>
                    Heap insertion membutuhkan O(log N) untuk mengurutkan prioritas, sedangkan Standard Queue bernilai O(1) karena hanya di-push ke belakang array.
                  </p>
                </div>

                {/* 2. Call Next Operation Chart */}
                <div className="glass-panel" style={{ background: 'rgba(7, 10, 19, 0.4)', border: '1px solid rgba(255, 255, 255, 0.03)' }}>
                  <h4 style={{ fontWeight: '600', marginBottom: '1.5rem', textAlign: 'center', fontSize: '1.1rem' }}>
                    📢 Batch Pop / Call Next ({benchmarkResults.scale} Data)
                  </h4>
                  <div className="flex-center">
                    <RenderBarChart 
                      valA={benchmarkResults.priority_queue.call_next_time_us} 
                      valB={benchmarkResults.standard_queue.call_next_time_us}
                      labelA="Priority Queue (Heap)"
                      labelB="Standard Queue (FIFO)"
                      colorA="var(--color-secondary)"
                      colorB="var(--text-muted)"
                    />
                  </div>
                  <p style={{ fontSize: '0.8rem', color: 'var(--text-secondary)', marginTop: '1rem', textAlign: 'center', fontStyle: 'italic' }}>
                    Heap pop membutuhkan O(log N) untuk menyeimbangkan ulang min-heap setelah mengeluarkan element teratas.
                  </p>
                </div>

                {/* 3. Search Operation Chart */}
                <div className="glass-panel" style={{ background: 'rgba(7, 10, 19, 0.4)', border: '1px solid rgba(255, 255, 255, 0.03)' }}>
                  <h4 style={{ fontWeight: '600', marginBottom: '1.5rem', textAlign: 'center', fontSize: '1.1rem' }}>
                    🔍 Single Target Search (Hash Map vs Linear)
                  </h4>
                  <div className="flex-center">
                    <RenderBarChart 
                      valA={benchmarkResults.priority_queue.search_time_us} 
                      valB={benchmarkResults.standard_queue.search_time_us}
                      labelA="Hash Table Search O(1)"
                      labelB="Standard Linear Search O(N)"
                      colorA="#10b981"
                      colorB="var(--text-muted)"
                    />
                  </div>
                  <p style={{ fontSize: '0.8rem', color: 'var(--text-secondary)', marginTop: '1rem', textAlign: 'center', fontStyle: 'italic' }}>
                    Pencarian ID menggunakan Hash Table beroperasi pada O(1) konstan, sedangkan antrian standar membutuhkan pencarian sekuensial linear O(N) melintasi seluruh data.
                  </p>
                </div>

              </div>

              {/* Data Explanation Panel */}
              <div className="glass-panel" style={{ background: 'rgba(56, 189, 248, 0.03)', border: '1px solid rgba(56, 189, 248, 0.1)' }}>
                <h4 style={{ color: 'var(--color-primary)', fontWeight: '600', marginBottom: '0.5rem' }}>
                  💡 Mengapa Menggunakan Priority Queue untuk Rumah Sakit?
                </h4>
                <p style={{ fontSize: '0.9rem', color: 'var(--text-secondary)', lineHeight: '1.6' }}>
                  Dari data grafik di atas, <strong>Standard Queue (FIFO)</strong> memiliki kecepatan insertion yang lebih cepat karena ia tidak melakukan pengurutan sama sekali dan langsung memasukkan pasien ke baris paling belakang.
                  Namun, pada operasional pelayanan kesehatan nyata, kita <strong>tidak boleh</strong> memperlakukan pasien gawat darurat (prioritas 1) secara FIFO jika ada antrian pasien reguler (prioritas 4) di depannya.
                  <br /><br />
                  Dengan menggunakan struktur data <strong>Min-Heap (Priority Queue)</strong>, C++ engine secara dinamis memastikan pasien dengan kondisi kritis akan selalu berada di puncak antrian agar dipanggil pertama kali ($O(1)$ untuk mendapatkan data tertinggi), dengan overhead reorganisasi internal heap yang sangat efisien ($O(\log N)$).
                  Dipadukan dengan **Hash Table (Unordered Map)**, kita memperoleh keuntungan mutlak untuk pencarian data instan ($O(1)$) untuk keperluan update status atau pembatalan antrian secara real-time.
                </p>
              </div>
            </div>
          )}
        </div>
      )}

      {/* Modal Check-In */}
      {checkInPatientData && (
        <div style={{ position: 'fixed', top: 0, left: 0, right: 0, bottom: 0, background: 'rgba(0,0,0,0.8)', display: 'flex', alignItems: 'center', justifySelf: 'center', justifyContent: 'center', zIndex: 1100 }}>
          <div className="glass-panel" style={{ width: '400px', border: '1px solid var(--color-primary)' }}>
            <h3 style={{ marginBottom: '1rem', color: 'var(--color-primary)' }}>📅 Check-In Fisik Pasien</h3>
            <p style={{ fontSize: '0.95rem', marginBottom: '1.25rem', color: 'var(--text-secondary)' }}>
              Pasien <strong>{checkInPatientData.nama}</strong> ({checkInPatientData.id}) telah tiba secara fisik di Rumah Sakit.
            </p>
            <div className="form-group">
              <label className="form-label">Jam Kedatangan Aktual</label>
              <input 
                type="text" 
                className="form-control" 
                placeholder="HH:MM"
                value={checkInTimeStr} 
                onChange={(e) => setCheckInTimeStr(e.target.value)}
              />
            </div>
            <p style={{ fontSize: '0.8rem', color: 'var(--text-muted)', marginBottom: '1.5rem' }}>
              Check-in akan mengubah status pasien dari TERJADWAL menjadi MENUNGGU, dan memasukkan pasien ke dalam Heap antrian aktif RS.
            </p>
            <div style={{ display: 'flex', gap: '0.75rem', justifyContent: 'flex-end' }}>
              <button className="btn btn-secondary" onClick={() => setCheckInPatientData(null)}>
                Batal
              </button>
              <button className="btn btn-primary" onClick={executeCheckIn}>
                Konfirmasi Check-In
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Spinner animation definition */}
      <style>{`
        @keyframes spin {
          to { transform: rotate(360deg); }
        }
      `}</style>
    </div>
  );
}

// Custom Pure SVG Bar Chart Component
function RenderBarChart({ valA, valB, labelA, labelB, colorA, colorB }) {
  const maxVal = Math.max(valA, valB, 1);
  // Scale height relative to 140px max height
  const heightA = Math.round((valA / maxVal) * 140) || 5;
  const heightB = Math.round((valB / maxVal) * 140) || 5;

  return (
    <svg width="280" height="220" viewBox="0 0 280 220" style={{ background: 'rgba(0,0,0,0.2)', borderRadius: '10px', padding: '10px' }}>
      {/* Grid Lines */}
      <line x1="40" y1="40" x2="260" y2="40" stroke="rgba(255,255,255,0.05)" strokeDasharray="4" />
      <line x1="40" y1="110" x2="260" y2="110" stroke="rgba(255,255,255,0.05)" strokeDasharray="4" />
      <line x1="40" y1="180" x2="260" y2="180" stroke="rgba(255,255,255,0.1)" />

      {/* Bar 1 (A) */}
      <rect 
        x="70" 
        y={180 - heightA} 
        width="45" 
        height={heightA} 
        fill={colorA} 
        rx="4" 
        style={{ transition: 'all 0.5s ease-out' }}
      />
      <text x="92.5" y={175 - heightA} fill="#fff" fontSize="11" textAnchor="middle" fontWeight="bold">
        {valA.toLocaleString()} us
      </text>

      {/* Bar 2 (B) */}
      <rect 
        x="165" 
        y={180 - heightB} 
        width="45" 
        height={heightB} 
        fill={colorB} 
        rx="4" 
        style={{ transition: 'all 0.5s ease-out' }}
      />
      <text x="187.5" y={175 - heightB} fill="#fff" fontSize="11" textAnchor="middle" fontWeight="bold">
        {valB.toLocaleString()} us
      </text>

      {/* X Labels */}
      <text x="92.5" y="198" fill="var(--text-secondary)" fontSize="10" textAnchor="middle" fontWeight="600">
        PQ (Heap)
      </text>
      <text x="187.5" y="198" fill="var(--text-secondary)" fontSize="10" textAnchor="middle" fontWeight="600">
        FIFO
      </text>

      {/* Legend / Title details */}
      <text x="140" y="215" fill="var(--text-muted)" fontSize="9" textAnchor="middle">
        *Lebih pendek bar = Lebih cepat (us = microseconds)
      </text>
    </svg>
  );
}

export default App;
