import React from 'react';
import { getPriorityLabel, getPriorityBadgeClass } from '../utils/helpers';

/**
 * Calling Deck — prominent card showing the patient currently being served,
 * or an empty state with "Call Next" CTA.
 */
export default function CallingDeck({
  currentServing,
  totalWaiting,
  onCallNext,
  onComplete,
  onCancel,
}) {
  if (currentServing) {
    return (
      <section className="calling-deck" aria-label="Pasien sedang dilayani">
        <div className={`card card--elevated priority-${currentServing.prioritas} is-active`}>
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
      </section>
    );
  }

  return (
    <section className="calling-deck" aria-label="Panggilan pasien">
      <div className="card">
        <div className="calling-deck-empty">
          <div>
            <h3 className="calling-deck-empty-title">Tidak Ada Panggilan Aktif</h3>
            <p className="calling-deck-empty-desc">
              Tekan tombol untuk memproses pasien berikutnya dari priority heap.
            </p>
          </div>
          <button
            id="btn-call-next"
            className="btn btn-primary btn-lg"
            onClick={onCallNext}
            disabled={totalWaiting === 0}
          >
            Panggil Berikutnya ({totalWaiting} mengantri)
          </button>
        </div>
      </div>
    </section>
  );
}
