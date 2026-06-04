import React from 'react';
import { Play, BarChart3, Lightbulb } from 'lucide-react';
import BarChart from './BarChart';
import EmptyState from './EmptyState';

/**
 * Full benchmark page view with controls, charts, and explanation.
 */
export default function BenchmarkView({
  benchmarkScale,
  onScaleChange,
  benchmarkResults,
  isBenchmarking,
  onRun,
}) {
  return (
    <div className="card">
      {/* Header / Controls */}
      <div className="benchmark-header">
        <div>
          <h3 className="card-title">Analisis Performa: Heap vs Standard Queue</h3>
          <p className="card-subtitle">
            Bandingkan performa C++ antara Priority Queue (Min-Heap) dan Standard Queue (FIFO) secara realtime.
          </p>
        </div>
        <div className="benchmark-controls">
          <label htmlFor="benchmark-scale">Skala Data:</label>
          <select
            id="benchmark-scale"
            className="form-control"
            value={benchmarkScale}
            onChange={(e) => onScaleChange(parseInt(e.target.value, 10))}
            disabled={isBenchmarking}
          >
            <option value="100">100 Data</option>
            <option value="1000">1.000 Data</option>
            <option value="10000">10.000 Data</option>
            <option value="50000">50.000 Data</option>
          </select>
          <button
            id="btn-run-benchmark"
            className="btn btn-primary"
            onClick={onRun}
            disabled={isBenchmarking}
          >
            <Play size={16} />
            {isBenchmarking ? 'Running...' : 'Mulai Test'}
          </button>
        </div>
      </div>

      {/* Loading: Skeleton */}
      {isBenchmarking && (
        <div>
          <div className="skeleton-grid">
            {[1, 2, 3].map((i) => (
              <div key={i} className="skeleton-card">
                <div className="skeleton-bar skeleton-bar--title" />
                <div className="skeleton-bar skeleton-bar--chart" />
              </div>
            ))}
          </div>
          <p style={{
            textAlign: 'center',
            marginTop: 'var(--space-6)',
            fontSize: 'var(--text-sm)',
            color: 'var(--text-tertiary)',
          }}>
            Menjalankan benchmark engine di C++...
          </p>
        </div>
      )}

      {/* Empty State */}
      {!isBenchmarking && !benchmarkResults && (
        <EmptyState
          icon={<BarChart3 size={48} />}
          title="Hasil benchmark belum tersedia."
          description={'Tentukan skala data dan tekan "Mulai Test".'}
        />
      )}

      {/* Results */}
      {!isBenchmarking && benchmarkResults && (
        <div>
          <div className="benchmark-charts">
            {/* 1. Insert */}
            <div className="chart-card">
              <h4 className="chart-card-title">
                Batch Insertion ({benchmarkResults.scale.toLocaleString()} Data)
              </h4>
              <BarChart
                valA={benchmarkResults.priority_queue.insert_time_us}
                valB={benchmarkResults.standard_queue.insert_time_us}
                labelA="PQ (Heap)"
                labelB="FIFO"
                colorA="hsl(220, 55%, 50%)"
                colorB="hsl(20, 4%, 56%)"
              />
              <p className="chart-card-explanation">
                Heap insertion membutuhkan O(log N) untuk mengurutkan prioritas, sedangkan Standard Queue bernilai O(1) karena hanya di-push ke belakang array.
              </p>
            </div>

            {/* 2. Call Next / Pop */}
            <div className="chart-card">
              <h4 className="chart-card-title">
                Batch Pop / Call Next ({benchmarkResults.scale.toLocaleString()} Data)
              </h4>
              <BarChart
                valA={benchmarkResults.priority_queue.call_next_time_us}
                valB={benchmarkResults.standard_queue.call_next_time_us}
                labelA="PQ (Heap)"
                labelB="FIFO"
                colorA="hsl(270, 50%, 55%)"
                colorB="hsl(20, 4%, 56%)"
              />
              <p className="chart-card-explanation">
                Heap pop membutuhkan O(log N) untuk menyeimbangkan ulang min-heap setelah mengeluarkan elemen teratas.
              </p>
            </div>

            {/* 3. Search */}
            <div className="chart-card">
              <h4 className="chart-card-title">
                Single Target Search (Hash Map vs Linear)
              </h4>
              <BarChart
                valA={benchmarkResults.priority_queue.search_time_us}
                valB={benchmarkResults.standard_queue.search_time_us}
                labelA="Hash O(1)"
                labelB="Linear O(N)"
                colorA="hsl(155, 50%, 42%)"
                colorB="hsl(20, 4%, 56%)"
              />
              <p className="chart-card-explanation">
                Hash Table beroperasi pada O(1) konstan, sedangkan antrian standar membutuhkan pencarian sekuensial linear O(N).
              </p>
            </div>
          </div>

          {/* Explanation Panel */}
          <div className="benchmark-explanation">
            <h4 className="benchmark-explanation-title">
              <Lightbulb size={20} />
              Mengapa Priority Queue untuk Rumah Sakit?
            </h4>
            <p className="benchmark-explanation-body">
              Dari data grafik di atas, <strong>Standard Queue (FIFO)</strong> memiliki
              kecepatan insertion yang lebih cepat karena tidak melakukan pengurutan.
              Namun, pada pelayanan kesehatan nyata, pasien gawat darurat (prioritas 1)
              <strong> tidak boleh</strong> menunggu di belakang antrian pasien reguler
              (prioritas 4). Dengan <strong>Min-Heap (Priority Queue)</strong>, C++ engine
              secara dinamis memastikan pasien kritis selalu di puncak antrian — dengan
              overhead O(log N) yang sangat efisien. Dipadukan dengan <strong>Hash Table</strong>,
              kita mendapat pencarian dan update status instan O(1).
            </p>
          </div>
        </div>
      )}
    </div>
  );
}
