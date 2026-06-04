import React from 'react';
import { Hospital, Clock, RefreshCw, Database } from 'lucide-react';

export default function Navbar({ currentTime, onRefresh, onLoadDummy, isLoading }) {
  return (
    <nav className="navbar" role="navigation" aria-label="Navigasi utama">
      <div className="navbar-inner">
        <div className="navbar-brand">
          <h1 className="navbar-title">
            <Hospital size={24} />
            Sistem Antrian RS
          </h1>
          <span className="navbar-subtitle">Administrative Dashboard</span>
        </div>
        <div className="navbar-actions">
          <div className="clock-pill" aria-label="Waktu saat ini">
            <Clock size={14} />
            <span>{currentTime}</span>
          </div>
          <button
            id="btn-load-dummy"
            className="btn btn-secondary btn-compact"
            onClick={onLoadDummy}
            disabled={isLoading}
          >
            <Database size={14} />
            Load Dummy
          </button>
          <button
            id="btn-refresh"
            className="btn btn-secondary btn-compact"
            onClick={onRefresh}
            disabled={isLoading}
          >
            <RefreshCw size={14} />
            Refresh
          </button>
        </div>
      </div>
    </nav>
  );
}
