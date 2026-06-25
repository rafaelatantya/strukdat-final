import React, { useState } from 'react';
import { UserPlus } from 'lucide-react';
import { getCurrentTime, getTodayDate } from '../utils/helpers';

/**
 * Patient registration form (sidebar).
 * Supports Walk-In (status MENUNGGU) and Booking (status TERJADWAL).
 */
export default function RegistrationForm({ currentTime, onSubmit }) {
  const [formData, setFormData] = useState({
    nama: '',
    layanan: '',
    prioritas: '4',
    tanggal: getTodayDate(),
    waktuDatang: '',
    isWalkIn: true,
  });

  const handleField = (field, value) => {
    if (field === 'isWalkIn' && value === true) {
      setFormData((prev) => ({ ...prev, isWalkIn: true, tanggal: getTodayDate() }));
    } else {
      setFormData((prev) => ({ ...prev, [field]: value }));
    }
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    // Pass validation to App.jsx to show toasts properly

    onSubmit({
      ...formData,
      prioritas: parseInt(formData.prioritas, 10),
      waktuDatang: formData.isWalkIn ? currentTime : '-',
    });
    // Reset form
    setFormData({
      nama: '',
      layanan: '',
      prioritas: '4',
      tanggal: getTodayDate(),
      waktuDatang: '',
      isWalkIn: true,
    });
  };

  return (
    <div className="card sidebar-form">
      <h3 className="card-title">Registrasi Pasien Baru</h3>
      <p className="card-subtitle mb-0" style={{ marginBottom: 'var(--space-6)' }}>
        Daftarkan pasien walk-in atau booking terjadwal.
      </p>

      <form onSubmit={handleSubmit}>
        {/* Registration Type */}
        <div className="form-group">
          <label className="form-label">Tipe Registrasi</label>
          <div className="radio-group">
            <label className="radio-label">
              <input
                type="radio"
                name="regType"
                checked={formData.isWalkIn}
                onChange={() => handleField('isWalkIn', true)}
              />
              <span>Walk-In (Hari Ini)</span>
            </label>
            <label className="radio-label">
              <input
                type="radio"
                name="regType"
                checked={!formData.isWalkIn}
                onChange={() => handleField('isWalkIn', false)}
              />
              <span>Booking (Terjadwal)</span>
            </label>
          </div>
        </div>

        {/* Name */}
        <div className="form-group">
          <label className="form-label" htmlFor="input-patient-name">Nama Pasien</label>
          <input
            id="input-patient-name"
            type="text"
            className="form-control"
            placeholder="Masukkan nama lengkap"
            value={formData.nama}
            onChange={(e) => handleField('nama', e.target.value)}
          />
        </div>

        {/* Service */}
        <div className="form-group">
          <label className="form-label" htmlFor="select-service">Poli / Layanan</label>
          <select
            id="select-service"
            className="form-control"
            value={formData.layanan}
            onChange={(e) => handleField('layanan', e.target.value)}
          >
            <option value="">— Pilih Poli —</option>
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

        {/* Priority */}
        <div className="form-group">
          <label className="form-label" htmlFor="select-priority">Tingkat Prioritas</label>
          <select
            id="select-priority"
            className="form-control"
            value={formData.prioritas}
            onChange={(e) => handleField('prioritas', e.target.value)}
          >
            <option value="1">1 — Darurat (Critical)</option>
            <option value="2">2 — Mendesak (Urgent)</option>
            <option value="3">3 — Rentan (Lansia / Bumil / Disabilitas)</option>
            <option value="4">4 — Reguler (Pemeriksaan Umum)</option>
          </select>
        </div>

        {/* Date */}
        <div className="form-group">
          <label className="form-label" htmlFor="input-date">Tanggal Kunjungan</label>
          <input
            id="input-date"
            type="date"
            className="form-control"
            value={formData.tanggal}
            min={getTodayDate()}
            disabled={formData.isWalkIn}
            onChange={(e) => handleField('tanggal', e.target.value)}
          />
        </div>

        {/* Arrival Time */}
        <div className="form-group">
          <label className="form-label">Waktu Kedatangan</label>
          <input
            type="text"
            className="form-control form-control--mono"
            disabled
            value={formData.isWalkIn ? currentTime : '-'}
          />
          <p className="form-helper">
            {formData.isWalkIn
              ? 'Pasien didaftarkan langsung ke antrian aktif dengan status MENUNGGU.'
              : 'Pasien mendapat status TERJADWAL dan belum masuk priority heap.'
            }
          </p>
        </div>

        <button
          id="btn-register-patient"
          type="submit"
          className="btn btn-primary btn-block mt-4"
        >
          <UserPlus size={18} />
          Tambahkan Pasien
        </button>
      </form>
    </div>
  );
}
