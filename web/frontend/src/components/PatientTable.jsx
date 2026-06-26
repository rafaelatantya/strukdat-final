import React from 'react';
import { Inbox, CalendarOff, Archive, XCircle } from 'lucide-react';
import { getPriorityLabel, getPriorityBadgeClass, getStatusLabel, getStatusBadgeClass } from '../utils/helpers';
import EmptyState from './EmptyState';

/**
 * Reusable patient data table. Renders different columns and actions
 * based on the current view mode: 'active', 'scheduled', or 'history'.
 */
export default function PatientTable({
  patients,
  mode,
  isLoading,
  busyPolys = [],
  onCheckIn,
  onCallSpecific,
  onComplete,
  onCancelPatient,
  onDeletePatient,
  isGlobalSearch = false,
}) {
  if (isLoading) {
    return (
      <div className="empty-state">
        <p className="empty-state-title">Memuat data pasien...</p>
      </div>
    );
  }

  if (patients.length === 0) {
    const emptyConfig = {
      active: {
        icon: <Inbox size={48} />,
        title: 'Tidak ada pasien dalam antrian aktif.',
        desc: 'Daftarkan pasien baru melalui form di samping.',
      },
      scheduled: {
        icon: <CalendarOff size={48} />,
        title: 'Tidak ada janji temu terjadwal.',
        desc: 'Pasien booking akan muncul di sini.',
      },
      history: {
        icon: <Archive size={48} />,
        title: 'Belum ada riwayat pelayanan.',
        desc: 'Pasien yang sudah selesai atau dibatalkan akan tampil di sini.',
      },
    };
    const cfg = emptyConfig[mode] || emptyConfig.active;
    return <EmptyState icon={cfg.icon} title={cfg.title} description={cfg.desc} />;
  }

  const showCalled = mode !== 'scheduled' || isGlobalSearch;
  const showActions = mode !== 'history' || isGlobalSearch;

  return (
    <div style={{ overflowX: 'auto' }}>
      <table className="data-table">
        <thead>
          <tr>
            <th>ID</th>
            <th>Nama Pasien</th>
            <th>Poli / Layanan</th>
            <th className="cell-center">Prioritas</th>
            {(mode !== 'scheduled' || isGlobalSearch) && <th className="cell-center">No. Antrian</th>}
            {(mode !== 'scheduled' || isGlobalSearch) && <th className="cell-center">Datang</th>}
            {showCalled && <th className="cell-center">Dipanggil</th>}
            <th className="cell-center">Tanggal</th>
            <th className="cell-center">Status</th>
            {showActions && <th className="cell-actions">Aksi</th>}
          </tr>
        </thead>
        <tbody>
          {patients.map((patient) => (
            <tr
              key={patient.id}
              className={patient.status === 3 ? 'row-dimmed' : ''}
            >
              <td className="cell-id">{patient.id}</td>
              <td className="cell-name">{patient.nama}</td>
              <td>{patient.layanan}</td>
              <td className="cell-center">
                <span className={getPriorityBadgeClass(patient.prioritas)}>
                  {patient.prioritas} — {getPriorityLabel(patient.prioritas)}
                </span>
              </td>
              {(mode !== 'scheduled' || isGlobalSearch) && (
                <td className="cell-queue">
                  {patient.status === 4 ? '—' : `#${patient.nomorAntrian}`}
                </td>
              )}
              {(mode !== 'scheduled' || isGlobalSearch) && (
                <td className="cell-center cell-time">{patient.waktuDatang}</td>
              )}
              {showCalled && (
                <td className={`cell-center cell-time ${patient.waktuDipanggil !== '-' ? 'cell-time--called' : ''}`}>
                  {patient.waktuDipanggil}
                </td>
              )}
              <td className="cell-center cell-time">{patient.tanggal}</td>
              <td className="cell-center">
                <span className={getStatusBadgeClass(patient.status)}>
                  {getStatusLabel(patient.status)}
                </span>
              </td>
              {showActions && (
                <td className="cell-actions">
                  <div className="cell-actions-inner">
                    {/* Scheduled: Check-in + Cancel */}
                    {patient.status === 4 && (
                      <>
                        <button
                          className="btn btn-primary btn-compact"
                          onClick={() => onCheckIn(patient)}
                        >
                          Check-In
                        </button>
                        <button
                          className="btn btn-ghost-danger btn-compact"
                          onClick={() => onDeletePatient(patient.id)}
                          aria-label={`Batalkan antrian ${patient.nama}`}
                        >
                          <XCircle size={14} />
                        </button>
                      </>
                    )}
                    {/* Waiting: queuing text, Call + Cancel */}
                    {patient.status === 0 && (
                      <>
                        <button
                          className="btn btn-success btn-compact"
                          disabled={busyPolys.includes(patient.layanan)}
                          onClick={() => onCallSpecific(patient.id)}
                          title={busyPolys.includes(patient.layanan) ? "Poli sedang melayani pasien lain" : "Panggil pasien ini"}
                        >
                          Panggil
                        </button>
                        <button
                          className="btn btn-ghost-danger btn-compact"
                          onClick={() => onDeletePatient(patient.id)}
                          aria-label={`Batalkan antrian ${patient.nama}`}
                        >
                          <XCircle size={14} />
                        </button>
                      </>
                    )}
                    {/* Called: Complete + Cancel */}
                    {patient.status === 1 && (
                      <>
                        <button
                          className="btn btn-success btn-compact"
                          onClick={() => onComplete(patient.id)}
                        >
                          Selesai
                        </button>
                        <button
                          className="btn btn-danger btn-compact"
                          onClick={() => onCancelPatient(patient.id)}
                        >
                          Batal
                        </button>
                      </>
                    )}
                    {/* History rows: no actions */}
                    {(patient.status === 2 || patient.status === 3) && (
                      <span className="queue-text-italic">{patient.status === 2 ? 'Selesai' : 'Batal'}</span>
                    )}
                  </div>
                </td>
              )}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
