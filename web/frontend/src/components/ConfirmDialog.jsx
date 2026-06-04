import React from 'react';

/**
 * Reusable confirmation dialog modal.
 * Replaces native window.confirm() per secure coding guidelines.
 */
export default function ConfirmDialog({ title, message, confirmLabel, confirmVariant, onConfirm, onCancel }) {
  return (
    <div className="modal-backdrop" onClick={onCancel} role="dialog" aria-modal="true" aria-labelledby="confirm-dialog-title">
      <div className="modal-panel" onClick={(e) => e.stopPropagation()}>
        <h3 id="confirm-dialog-title" className="modal-title">{title}</h3>
        <div className="modal-body">
          <p>{message}</p>
        </div>
        <div className="modal-footer">
          <button className="btn btn-secondary" onClick={onCancel}>
            Batal
          </button>
          <button
            className={`btn ${confirmVariant === 'danger' ? 'btn-danger' : 'btn-primary'}`}
            onClick={onConfirm}
          >
            {confirmLabel || 'Konfirmasi'}
          </button>
        </div>
      </div>
    </div>
  );
}
