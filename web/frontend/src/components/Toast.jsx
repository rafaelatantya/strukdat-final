import React from 'react';
import { X, CheckCircle, AlertTriangle } from 'lucide-react';

export default function Toast({ toasts, onDismiss }) {
  if (!toasts || toasts.length === 0) return null;

  return (
    <div className="toast-container" role="status" aria-live="polite">
      {toasts.map((toast) => (
        <div
          key={toast.id}
          className={`toast toast--${toast.type}`}
        >
          <span className={`toast-icon toast-icon--${toast.type}`}>
            {toast.type === 'success'
              ? <CheckCircle size={18} />
              : <AlertTriangle size={18} />
            }
          </span>
          <div className="toast-content">
            <p className="toast-message">{toast.message}</p>
          </div>
          <button
            className="toast-dismiss"
            onClick={() => onDismiss(toast.id)}
            aria-label="Tutup notifikasi"
          >
            <X size={16} />
          </button>
        </div>
      ))}
    </div>
  );
}
