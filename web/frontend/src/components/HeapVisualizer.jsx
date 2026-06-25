import React from 'react';
import './HeapVisualizer.css'; // Opsional jika butuh styling khusus

function HeapNode({ node, index }) {
  if (!node) return null;

  return (
    <div className="heap-node">
      <div className="heap-node-content">
        <div className="heap-index">#{index}</div>
        <div className="heap-id">{node.nomorAntrian}</div>
        <div className="heap-details">
          <span>{node.nama.split(' ')[0]}</span>
          <span className={`heap-priority-badge heap-priority-${node.prioritas}`}>
            P{node.prioritas}
          </span>
        </div>
      </div>
    </div>
  );
}

// Rekursif menggambar pohon
function renderTree(heap, index) {
  if (index >= heap.length) return null;
  const node = heap[index];
  const leftChildIdx = 2 * index + 1;
  const rightChildIdx = 2 * index + 2;

  const hasLeft = leftChildIdx < heap.length;
  const hasRight = rightChildIdx < heap.length;

  return (
    <div className="tree-node-wrapper" key={index}>
      <HeapNode node={node} index={index} />
      {(hasLeft || hasRight) && (
        <div className="tree-children">
          {hasLeft ? renderTree(heap, leftChildIdx) : <div className="tree-node-wrapper empty"></div>}
          {hasRight ? renderTree(heap, rightChildIdx) : <div className="tree-node-wrapper empty"></div>}
        </div>
      )}
    </div>
  );
}

import { RefreshCw } from 'lucide-react';

export default function HeapVisualizer({ heapData, onRefresh }) {
  if (!heapData || heapData.length === 0) {
    return (
      <div className="heap-visualizer-empty">
        <div className="empty-state">
          <i className="bi bi-diagram-3" style={{ fontSize: '3rem', color: '#6c757d' }}></i>
          <p className="mt-3">Antrian prioritas kosong. Pohon Max-Heap tidak memiliki node.</p>
        </div>
      </div>
    );
  }

  return (
    <div className="heap-visualizer-container">
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', marginBottom: '1rem' }}>
        <div style={{ flex: 1 }}>
          <h3 className="mb-2 text-center">Visualisasi Antrian (Max-Heap)</h3>
          <p className="text-center text-muted mb-0">
            Ini adalah representasi array dalam bentuk struktur pohon biner lengkap yang digunakan oleh `std::priority_queue`.
          </p>
        </div>
        <button
          className="btn btn-secondary"
          onClick={onRefresh}
          title="Refresh Data Heap"
          style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}
        >
          <RefreshCw size={16} />
          <span>Refresh</span>
        </button>
      </div>
      
      <div className="tree-root">
        {renderTree(heapData, 0)}
      </div>

      <div className="heap-array-view mt-5">
        <h5 className="mb-3">Representasi Array</h5>
        <div className="array-blocks">
          {heapData.map((node, i) => (
            <div key={i} className="array-block">
              <span className="idx">{i}</span>
              <span className="val">{node.nomorAntrian}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
