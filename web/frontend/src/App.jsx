import React, { useState, useEffect, useCallback, useRef } from 'react';
import Navbar from './components/Navbar';
import StatsBar from './components/StatsBar';
import TabBar from './components/TabBar';
import CallingDeck from './components/CallingDeck';
import PatientTable from './components/PatientTable';
import RegistrationForm from './components/RegistrationForm';
import CheckInModal from './components/CheckInModal';
import ConfirmDialog from './components/ConfirmDialog';
import BenchmarkView from './components/BenchmarkView';
import Toast from './components/Toast';
import HeapVisualizer from './components/HeapVisualizer';
import { Search, X } from 'lucide-react';
import {
  fetchPatients as apiFetchPatients,
  registerPatient,
  checkInPatient,
  callNextPatient,
  updatePatientStatus,
  deletePatient,
  loadDummyData,
  runBenchmark,
  getHeapData,
} from './utils/api';
import { getCurrentTimeFull, getTodayDate } from './utils/helpers';

let toastIdCounter = 0;

function App() {
  // Core state
  const [patients, setPatients] = useState([]);
  const [heapData, setHeapData] = useState([]);
  const [activeTab, setActiveTab] = useState('active');
  const [searchQuery, setSearchQuery] = useState('');
  const [isGlobalSearch, setIsGlobalSearch] = useState(false);
  const [isLoading, setIsLoading] = useState(false);
  const [currentTime, setCurrentTime] = useState(getCurrentTimeFull());

  // Toast state
  const [toasts, setToasts] = useState([]);

  // Modal state
  const [checkInPatientData, setCheckInPatientData] = useState(null);
  const [confirmDialog, setConfirmDialog] = useState(null);

  // Benchmark state
  const [benchmarkScale, setBenchmarkScale] = useState(10000);
  const [benchmarkResults, setBenchmarkResults] = useState(null);
  const [isBenchmarking, setIsBenchmarking] = useState(false);

  // Toast timeout refs
  const toastTimers = useRef({});

  // Clock
  useEffect(() => {
    const interval = setInterval(() => {
      setCurrentTime(getCurrentTimeFull());
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  // Initial data load
  useEffect(() => {
    handleFetchPatients();
  }, []);

  // Cleanup toast timers on unmount
  useEffect(() => {
    return () => {
      Object.values(toastTimers.current).forEach(clearTimeout);
    };
  }, []);

  // --- Toast helpers ---
  const addToast = useCallback((message, type = 'success') => {
    const id = ++toastIdCounter;
    setToasts((prev) => [...prev, { id, message, type }]);
    toastTimers.current[id] = setTimeout(() => {
      dismissToast(id);
    }, 4000);
    return id;
  }, []);

  const dismissToast = useCallback((id) => {
    setToasts((prev) => prev.filter((t) => t.id !== id));
    if (toastTimers.current[id]) {
      clearTimeout(toastTimers.current[id]);
      delete toastTimers.current[id];
    }
  }, []);

  // --- API handlers ---
  const handleFetchPatients = async () => {
    setIsLoading(true);
    try {
      const data = await apiFetchPatients();
      if (data.status === 'success') {
        setPatients(data.data || []);
      } else {
        addToast(data.message || 'Gagal memuat data pasien.', 'error');
      }

      const heapResponse = await getHeapData();
      if (heapResponse.status === 'success') {
        setHeapData(heapResponse.data || []);
      }
    } catch {
      addToast('Gagal terhubung ke API backend.', 'error');
    } finally {
      setIsLoading(false);
    }
  };

  const handleRegister = async (formData) => {
    if (!formData.nama || !formData.layanan || !formData.tanggal) {
      addToast('Mohon lengkapi semua field registrasi.', 'error');
      return;
    }

    // Validasi booking/tanggal tidak boleh di masa lalu
    const today = getTodayDate();
    if (formData.tanggal < today) {
      addToast('Kocak! Tanggal booking tidak boleh di masa lalu.', 'error');
      return;
    }

    try {
      const data = await registerPatient(formData);
      if (data.status === 'success') {
        addToast(`Pasien ${formData.nama} berhasil didaftarkan.`);
        handleFetchPatients();
      } else {
        addToast(data.message || 'Gagal meregistrasikan pasien.', 'error');
      }
    } catch {
      addToast('Kesalahan jaringan saat registrasi.', 'error');
    }
  };

  const handleCheckInClick = (patient) => {
    setCheckInPatientData(patient);
  };

  const executeCheckIn = async (id, waktuDatang) => {
    try {
      const data = await checkInPatient(id, waktuDatang);
      if (data.status === 'success') {
        addToast(`Pasien berhasil check-in fisik.`);
        setCheckInPatientData(null);
        handleFetchPatients();
      } else {
        addToast(data.message || 'Gagal melakukan check-in.', 'error');
      }
    } catch {
      addToast('Kesalahan jaringan saat check-in.', 'error');
    }
  };

  const handleCallNext = async () => {
    try {
      const data = await callNextPatient();
      if (data.status === 'success') {
        addToast(`Memanggil pasien: ${data.data.nama} (Antrian #${data.data.nomorAntrian})`);
        handleFetchPatients();
      } else {
        addToast(data.message || 'Antrian tunggu sedang kosong.', 'error');
      }
    } catch {
      addToast('Kesalahan jaringan saat memanggil pasien.', 'error');
    }
  };

  const handleStatusUpdate = async (id, statusVal, force = false) => {
    try {
      const data = await updatePatientStatus(id, statusVal, force);
      if (data.status === 'warning' && data.message.startsWith('WARNING_PRIORITY:')) {
        const topPatientInfo = data.message.split(':')[1].trim();
        setConfirmDialog({
          title: 'Peringatan Prioritas',
          message: `Pasien yang Anda panggil BUKAN pasien dengan prioritas tertinggi. Pasien teratas adalah: ${topPatientInfo}. Apakah Anda yakin ingin memanggil pasien ini terlebih dahulu?`,
          confirmLabel: 'Ya, Tetap Panggil',
          confirmVariant: 'warning',
          onConfirm: () => {
            setConfirmDialog(null);
            handleStatusUpdate(id, statusVal, true);
          }
        });
        return;
      }
      
      if (data.status === 'success') {
        const label = statusVal === 1 ? 'Dipanggil' : statusVal === 2 ? 'Selesai' : 'Batal';
        addToast(`Status pasien berhasil diubah menjadi ${label}.`);
        handleFetchPatients();
      } else {
        addToast(data.message || 'Gagal memperbarui status.', 'error');
      }
    } catch {
      addToast('Kesalahan jaringan saat update status.', 'error');
    }
  };

  const handleCallSpecific = (id) => {
    handleStatusUpdate(id, 1);
  };

  const handleDeletePatient = (id) => {
    const patient = patients.find((p) => p.id === id);
    setConfirmDialog({
      title: 'Konfirmasi Pembatalan',
      message: `Apakah Anda yakin ingin membatalkan antrian pasien ${patient?.nama || id}? Tindakan ini tidak dapat dibatalkan.`,
      confirmLabel: 'Ya, Batalkan',
      confirmVariant: 'danger',
      onConfirm: async () => {
        setConfirmDialog(null);
        try {
          const data = await deletePatient(id);
          if (data.status === 'success') {
            addToast('Antrian pasien berhasil dibatalkan.');
            handleFetchPatients();
          } else {
            addToast(data.message || 'Gagal membatalkan antrian.', 'error');
          }
        } catch {
          addToast('Kesalahan jaringan saat membatalkan antrian.', 'error');
        }
      },
    });
  };

  const handleLoadDummy = () => {
    setConfirmDialog({
      title: 'Muat Data Dummy',
      message: 'Muat data dummy simulasi? Data saat ini tidak akan dihapus tetapi akan digabungkan.',
      confirmLabel: 'Muat Data',
      confirmVariant: 'primary',
      onConfirm: async () => {
        setConfirmDialog(null);
        setIsLoading(true);
        try {
          const data = await loadDummyData();
          if (data.status === 'success') {
            addToast('Data dummy berhasil di-load.');
            handleFetchPatients();
          } else {
            addToast(data.message || 'Gagal memuat data dummy.', 'error');
          }
        } catch {
          addToast('Kesalahan jaringan saat memuat dummy.', 'error');
        } finally {
          setIsLoading(false);
        }
      },
    });
  };

  const handleRunBenchmark = async () => {
    setIsBenchmarking(true);
    setBenchmarkResults(null);
    try {
      const data = await runBenchmark(benchmarkScale);
      if (data.scale) {
        setBenchmarkResults(data);
        addToast('Benchmark performa berhasil diselesaikan.');
      } else {
        addToast(data.message || 'Gagal menjalankan benchmark.', 'error');
      }
    } catch {
      addToast('Kesalahan jaringan saat benchmark.', 'error');
    } finally {
      setIsBenchmarking(false);
    }
  };

  // --- Computed values ---
  const totalWaiting = patients.filter((p) => p.status === 0).length;
  const totalCalled = patients.filter((p) => p.status === 1).length;
  const totalScheduled = patients.filter((p) => p.status === 4).length;
  const totalCompleted = patients.filter((p) => p.status === 2).length;
  const totalCancelled = patients.filter((p) => p.status === 3).length;

  const allServing = patients.filter((p) => p.status === 1);
  const busyPolys = allServing.map((p) => p.layanan);
  const canCallNext = patients.some((p) => p.status === 0 && !busyPolys.includes(p.layanan));

  const getFilteredPatients = () => {
    switch (activeTab) {
      case 'active':
        return patients.filter((p) => p.status === 0 || p.status === 1);
      case 'scheduled':
        return patients.filter((p) => p.status === 4);
      case 'history':
        return patients.filter((p) => p.status === 2 || p.status === 3);
      default:
        return [];
    }
  };

  const getSearchedPatients = () => {
    const query = searchQuery.toLowerCase().trim();
    if (!query) {
      return getFilteredPatients();
    }

    const matches = (p) => {
      const namaMatch = p.nama ? p.nama.toLowerCase().includes(query) : false;
      const idMatch = p.id ? p.id.toLowerCase().includes(query) : false;
      const antrianMatch = p.nomorAntrian ? p.nomorAntrian.toLowerCase().includes(query) : false;
      return namaMatch || idMatch || antrianMatch;
    };

    if (isGlobalSearch) {
      return patients.filter(matches);
    } else {
      return getFilteredPatients().filter(matches);
    }
  };

  const filteredPatients = getFilteredPatients();
  const searchedPatients = getSearchedPatients();
  const currentTimeShort = currentTime.slice(0, 5); // HH:MM for forms

  const TABS = [
    { id: 'active', label: 'Antrian Aktif' },
    { id: 'scheduled', label: 'Janji Temu' },
    { id: 'history', label: 'Riwayat' },
    { id: 'visualizer', label: 'Visualizer Heap' },
    { id: 'benchmark', label: 'Benchmark' },
  ];

  const tabCounts = {
    active: totalWaiting + totalCalled,
    scheduled: totalScheduled,
    history: totalCompleted + totalCancelled,
  };

  return (
    <>
      {/* Navbar */}
      <Navbar
        currentTime={currentTime}
        onRefresh={handleFetchPatients}
        onLoadDummy={handleLoadDummy}
        isLoading={isLoading}
      />

      {/* Main Content */}
      <main className="app-layout">
        {/* Stats */}
        <StatsBar
          totalWaiting={totalWaiting}
          totalScheduled={totalScheduled}
          totalCalled={totalCalled}
          totalCompleted={totalCompleted}
        />

        {/* Tabs */}
        <TabBar
          activeTab={activeTab}
          onTabChange={setActiveTab}
          counts={tabCounts}
          tabs={TABS}
        />

        {/* Tab Content */}
        {activeTab === 'benchmark' ? (
          <BenchmarkView
            benchmarkScale={benchmarkScale}
            onScaleChange={setBenchmarkScale}
            benchmarkResults={benchmarkResults}
            isBenchmarking={isBenchmarking}
            onRun={handleRunBenchmark}
          />
        ) : activeTab === 'visualizer' ? (
          <div className="card" style={{ overflow: 'hidden' }}>
            <HeapVisualizer heapData={heapData} onRefresh={handleFetchPatients} />
          </div>
        ) : (
          <>
            {/* Calling Deck (only on active tab) */}
            {activeTab === 'active' && (
              <CallingDeck
                allServing={allServing}
                totalWaiting={totalWaiting}
                canCallNext={canCallNext}
                onCallNext={handleCallNext}
                onComplete={(id) => handleStatusUpdate(id, 2)}
                onCancel={(id) => handleStatusUpdate(id, 3)}
              />
            )}

            {/* Content Grid */}
            <div className={`content-grid ${activeTab === 'history' ? 'content-grid--full' : ''}`}>
              {/* Table */}
              <div className="card" style={{ overflow: 'hidden', padding: 0 }}>
                <div style={{ padding: 'var(--space-6)', borderBottom: '1px solid var(--border-color)', display: 'flex', flexDirection: 'column', gap: 'var(--space-4)' }}>
                  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', flexWrap: 'wrap', gap: 'var(--space-4)' }}>
                    <h3 className="card-title" style={{ margin: 0 }}>
                      {isGlobalSearch && searchQuery.trim() !== ''
                        ? 'Hasil Pencarian Global (Semua Status)'
                        : (activeTab === 'active' && 'Daftar Pasien Menunggu & Dilayani') ||
                          (activeTab === 'scheduled' && 'Daftar Pasien Booking Janji Temu') ||
                          (activeTab === 'history' && 'Riwayat Pasien Selesai & Batal')
                      }
                    </h3>
                    <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-3)', width: '100%', maxWidth: '400px' }}>
                      <div style={{ position: 'relative', flex: 1 }}>
                        <input
                          type="text"
                          placeholder="Cari nama atau ID..."
                          value={searchQuery}
                          onChange={(e) => setSearchQuery(e.target.value)}
                          className="form-control"
                          style={{ paddingLeft: '2.5rem', paddingRight: searchQuery ? '2.5rem' : '0.5rem', width: '100%', height: '38px' }}
                        />
                        <Search size={16} style={{ position: 'absolute', left: '1rem', top: '50%', transform: 'translateY(-50%)', color: 'var(--text-muted)' }} />
                        {searchQuery && (
                          <button
                            onClick={() => setSearchQuery('')}
                            style={{
                              position: 'absolute',
                              right: '0.75rem',
                              top: '50%',
                              transform: 'translateY(-50%)',
                              background: 'none',
                              border: 'none',
                              color: 'var(--text-muted)',
                              cursor: 'pointer',
                              display: 'flex',
                              alignItems: 'center',
                              justifyContent: 'center',
                              padding: 0
                            }}
                            title="Bersihkan pencarian"
                          >
                            <X size={16} />
                          </button>
                        )}
                      </div>
                      <label style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-2)', fontSize: 'var(--text-sm)', color: 'var(--text-secondary)', cursor: 'pointer', userSelect: 'none', whiteSpace: 'nowrap' }}>
                        <input
                          type="checkbox"
                          checked={isGlobalSearch}
                          onChange={(e) => setIsGlobalSearch(e.target.checked)}
                          style={{ width: '16px', height: '16px', cursor: 'pointer' }}
                        />
                        Cari Global
                      </label>
                    </div>
                  </div>
                </div>
                <PatientTable
                  patients={searchedPatients}
                  mode={activeTab}
                  isLoading={isLoading}
                  busyPolys={busyPolys}
                  onCheckIn={handleCheckInClick}
                  onCallSpecific={handleCallSpecific}
                  onComplete={(id) => handleStatusUpdate(id, 2)}
                  onCancelPatient={(id) => handleStatusUpdate(id, 3)}
                  onDeletePatient={handleDeletePatient}
                  isGlobalSearch={isGlobalSearch && searchQuery.trim() !== ''}
                />
              </div>

              {/* Sidebar Form (only on active & scheduled tabs) */}
              {activeTab !== 'history' && (
                <RegistrationForm
                  currentTime={currentTimeShort}
                  onSubmit={handleRegister}
                />
              )}
            </div>
          </>
        )}
      </main>

      {/* Global Overlays */}
      <Toast toasts={toasts} onDismiss={dismissToast} />

      {checkInPatientData && (
        <CheckInModal
          patient={checkInPatientData}
          currentTime={currentTimeShort}
          onConfirm={executeCheckIn}
          onCancel={() => setCheckInPatientData(null)}
        />
      )}

      {confirmDialog && (
        <ConfirmDialog
          title={confirmDialog.title}
          message={confirmDialog.message}
          confirmLabel={confirmDialog.confirmLabel}
          confirmVariant={confirmDialog.confirmVariant}
          onConfirm={confirmDialog.onConfirm}
          onCancel={() => setConfirmDialog(null)}
        />
      )}
    </>
  );
}

export default App;
