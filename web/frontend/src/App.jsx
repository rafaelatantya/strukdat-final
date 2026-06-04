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
import {
  fetchPatients as apiFetchPatients,
  registerPatient,
  checkInPatient,
  callNextPatient,
  updatePatientStatus,
  deletePatient,
  loadDummyData,
  runBenchmark,
} from './utils/api';
import { getCurrentTimeFull, getTodayDate } from './utils/helpers';

let toastIdCounter = 0;

function App() {
  // Core state
  const [patients, setPatients] = useState([]);
  const [activeTab, setActiveTab] = useState('active');
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
    } catch {
      addToast('Gagal terhubung ke API backend.', 'error');
    } finally {
      setIsLoading(false);
    }
  };

  const handleRegister = async (formData) => {
    if (!formData.id || !formData.nama || !formData.layanan || !formData.tanggal) {
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

  const handleStatusUpdate = async (id, statusVal) => {
    try {
      const data = await updatePatientStatus(id, statusVal);
      if (data.status === 'success') {
        const label = statusVal === 2 ? 'Selesai' : 'Batal';
        addToast(`Status pasien berhasil diubah menjadi ${label}.`);
        handleFetchPatients();
      } else {
        addToast(data.message || 'Gagal memperbarui status.', 'error');
      }
    } catch {
      addToast('Kesalahan jaringan saat update status.', 'error');
    }
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

  const currentServing = patients.find((p) => p.status === 1);

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

  const filteredPatients = getFilteredPatients();
  const currentTimeShort = currentTime.slice(0, 5); // HH:MM for forms

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
        />

        {/* Tab Content */}
        {activeTab !== 'benchmark' ? (
          <>
            {/* Calling Deck (only on active tab) */}
            {activeTab === 'active' && (
              <CallingDeck
                currentServing={currentServing}
                totalWaiting={totalWaiting}
                onCallNext={handleCallNext}
                onComplete={(id) => handleStatusUpdate(id, 2)}
                onCancel={(id) => handleStatusUpdate(id, 3)}
              />
            )}

            {/* Content Grid */}
            <div className={`content-grid ${activeTab === 'history' ? 'content-grid--full' : ''}`}>
              {/* Table */}
              <div className="card" style={{ overflow: 'hidden', padding: 0 }}>
                <div style={{ padding: 'var(--space-6)' }}>
                  <h3 className="card-title" style={{ marginBottom: 'var(--space-4)' }}>
                    {activeTab === 'active' && 'Daftar Pasien Menunggu & Dilayani'}
                    {activeTab === 'scheduled' && 'Daftar Pasien Booking Janji Temu'}
                    {activeTab === 'history' && 'Riwayat Pasien Selesai & Batal'}
                  </h3>
                </div>
                <PatientTable
                  patients={filteredPatients}
                  mode={activeTab}
                  isLoading={isLoading}
                  onCheckIn={handleCheckInClick}
                  onComplete={(id) => handleStatusUpdate(id, 2)}
                  onCancelPatient={(id) => handleStatusUpdate(id, 3)}
                  onDeletePatient={handleDeletePatient}
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
        ) : (
          /* Benchmark Tab */
          <BenchmarkView
            benchmarkScale={benchmarkScale}
            onScaleChange={setBenchmarkScale}
            benchmarkResults={benchmarkResults}
            isBenchmarking={isBenchmarking}
            onRun={handleRunBenchmark}
          />
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
