import React from 'react';
import { Clock, CalendarCheck, Megaphone, CheckCircle } from 'lucide-react';

export default function StatsBar({ totalWaiting, totalScheduled, totalCalled, totalCompleted }) {
  return (
    <section className="stats-grid" aria-label="Ringkasan statistik">
      <div className="stat-card">
        <div className="stat-icon stat-icon--waiting">
          <Clock />
        </div>
        <div>
          <p className="stat-label">Menunggu</p>
          <p className="stat-value">{totalWaiting}</p>
        </div>
      </div>

      <div className="stat-card">
        <div className="stat-icon stat-icon--scheduled">
          <CalendarCheck />
        </div>
        <div>
          <p className="stat-label">Terjadwal</p>
          <p className="stat-value">{totalScheduled}</p>
        </div>
      </div>

      <div className="stat-card">
        <div className="stat-icon stat-icon--called">
          <Megaphone />
        </div>
        <div>
          <p className="stat-label">Dipanggil</p>
          <p className="stat-value">{totalCalled}</p>
        </div>
      </div>

      <div className="stat-card">
        <div className="stat-icon stat-icon--completed">
          <CheckCircle />
        </div>
        <div>
          <p className="stat-label">Selesai</p>
          <p className="stat-value">{totalCompleted}</p>
        </div>
      </div>
    </section>
  );
}
