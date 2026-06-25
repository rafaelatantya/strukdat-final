import React from 'react';

const TABS = [
  { id: 'active', label: 'Antrian Aktif' },
  { id: 'scheduled', label: 'Terjadwal' },
  { id: 'history', label: 'Riwayat' },
  { id: 'benchmark', label: 'Analisis Performa' },
];

export default function TabBar({ activeTab, onTabChange, counts, tabs }) {
  const currentTabs = tabs || TABS;
  return (
    <div className="tab-bar" role="tablist" aria-label="Navigasi tab halaman">
      {currentTabs.map((tab) => (
        <button
          key={tab.id}
          role="tab"
          aria-selected={activeTab === tab.id}
          aria-controls={`panel-${tab.id}`}
          className={`tab-item ${activeTab === tab.id ? 'active' : ''}`}
          onClick={() => onTabChange(tab.id)}
        >
          {tab.label}
          {counts[tab.id] !== undefined && (
            <span className="tab-count"> ({counts[tab.id]})</span>
          )}
        </button>
      ))}
    </div>
  );
}
