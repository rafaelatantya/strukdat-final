import React from 'react';
import { getPriorityLabel, getPriorityBadgeClass } from '../utils/helpers';

/**
 * Calling Deck — prominent card showing the patient currently being served,
 * or an empty state with "Call Next" CTA.
 */
export default function CallingDeck({
  allServing = [],
  totalWaiting,
  canCallNext,
  onCallNext,
  onComplete,
  onCancel,
}) {
  return (
    <section className="calling-deck" aria-label="Panggilan pasien" style={{ display: 'flex', flexDirection: 'column', gap: '1rem' }}>
      {/* 1. Kartu Pasien yang Sedang Dilayani (Bisa multiple karena beda poli) */}
      {allServing.length > 0 ? (
        <div className="ongoing-patients-list" style={{ display: 'flex', flexDirection: 'column', gap: '1rem' }}>
          {allServing.map((currentServing) => (
            <div key={currentServing.id} className={`card card--elevated priority-${currentServing.prioritas} is-active`}>
          <div className="calling-deck-content">
            <div className="calling-deck-info">
              <span className={getPriorityBadgeClass(currentServing.prioritas)}>
                {getPriorityLabel(currentServing.prioritas)}
              </span>
              <h2 className="calling-deck-name">{currentServing.nama}</h2>
              <p className="calling-deck-meta">
                ID: <span className="patient-id">{currentServing.id}</span>
                {' · '}
                <span className="service-name">{currentServing.layanan}</span>
              </p>
              <div className="calling-deck-times">
                <span>
                  Datang: <span className="time-value text-mono">{currentServing.waktuDatang}</span>
                </span>
                <span>
                  Dipanggil: <span className="time-value time-value--called text-mono">{currentServing.waktuDipanggil}</span>
                </span>
                <span>
                  Tanggal: <span className="time-value text-mono">{currentServing.tanggal}</span>
                </span>
              </div>
            </div>
            <div className="calling-deck-queue">
              <div className="calling-deck-queue-label">Nomor Antrian</div>
              <div className="calling-deck-queue-number">#{currentServing.nomorAntrian}</div>
              <div className="calling-deck-actions">
                <button
                  id="btn-complete-patient"
                  className="btn btn-success btn-compact"
                  onClick={() => onComplete(currentServing.id)}
                >
                  Selesai
                </button>
                <button
                  id="btn-cancel-serving"
                  className="btn btn-danger btn-compact"
                  onClick={() => onCancel(currentServing.id)}
                >
                  Batalkan
                </button>
              </div>
            </div>
          </div>
            </div>
          ))}
        </div>
      ) : (
        <div className="card">
          <div className="calling-deck-empty" style={{ textAlign: 'center' }}>
            <h3 className="calling-deck-empty-title">Tidak Ada Panggilan Aktif</h3>
            <p className="calling-deck-empty-desc mb-0">Belum ada pasien yang sedang dilayani di poli mana pun.</p>
          </div>
        </div>
      )}

      {/* 2. Kartu Khusus Panggil Berikutnya */}
      <div className="card">
        <div className="calling-deck-empty" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', textAlign: 'left' }}>
          <div>
            <h3 className="calling-deck-empty-title" style={{ marginBottom: '0.25rem' }}>Panggil Pasien Berikutnya</h3>
            <p className="calling-deck-empty-desc mb-0">
              {totalWaiting === 0 
                ? 'Tidak ada antrian yang menunggu.' 
                : canCallNext 
                  ? 'Tekan tombol untuk memanggil dari priority heap.'
                  : 'Semua poli untuk pasien yang mengantri sedang sibuk.'}
            </p>
          </div>
          <button
            id="btn-call-next"
            className="btn btn-primary btn-lg"
            onClick={onCallNext}
            disabled={!canCallNext || totalWaiting === 0}
            title={(!canCallNext && totalWaiting > 0) ? "Harus selesaikan pasien di poli yang sibuk terlebih dahulu" : "Panggil pasien"}
            style={{ whiteSpace: 'nowrap' }}
          >
            Panggil Berikutnya ({totalWaiting} antri)
          </button>
        </div>
      </div>
    </section>
  );
}
