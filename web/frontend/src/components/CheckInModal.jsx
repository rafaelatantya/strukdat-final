import React, { useState } from 'react';

/**
 * Modal for checking in a scheduled patient.
 * Pre-fills the arrival time with current system time.
 */
export default function CheckInModal({ patient, currentTime, onConfirm, onCancel }) {
  const [timeStr, setTimeStr] = useState(currentTime);

  if (!patient) return null;

  const handleConfirm = () => {
    if (timeStr) {
      onConfirm(patient.id, timeStr);
    }
  };

  return (
    <div className="modal-backdrop" onClick={onCancel} role="dialog" aria-modal="true" aria-labelledby="checkin-modal-title">
      <div className="modal-panel" onClick={(e) => e.stopPropagation()}>
        <h3 id="checkin-modal-title" className="modal-title">Check-In Fisik Pasien</h3>
        <div className="modal-body">
          <p>
            Pasien <strong>{patient.nama}</strong> ({patient.id}) telah tiba di rumah sakit.
          </p>
        </div>

        <div className="form-group">
          <label className="form-label" htmlFor="checkin-time">Jam Kedatangan Aktual</label>
          <input
            id="checkin-time"
            type="text"
            className="form-control form-control--mono"
            placeholder="HH:MM"
            value={timeStr}
            onChange={(e) => setTimeStr(e.target.value)}
          />
          <p className="form-helper">
            Check-in mengubah status dari TERJADWAL menjadi MENUNGGU dan memasukkan pasien ke priority heap antrian aktif.
          </p>
        </div>

        <div className="modal-footer">
          <button className="btn btn-secondary" onClick={onCancel}>
            Batal
          </button>
          <button className="btn btn-primary" onClick={handleConfirm}>
            Konfirmasi Check-In
          </button>
        </div>
      </div>
    </div>
  );
}
