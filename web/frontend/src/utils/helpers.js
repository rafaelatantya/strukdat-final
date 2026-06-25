/**
 * Helper functions for priority/status labeling, formatting, and display.
 */

/** Priority level → human-readable label */
export function getPriorityLabel(priority) {
  switch (priority) {
    case 1: return 'Darurat';
    case 2: return 'Mendesak';
    case 3: return 'Rentan';
    case 4: return 'Reguler';
    default: return 'Lainnya';
  }
}

/** Priority level → CSS class modifier for badge */
export function getPriorityBadgeClass(priority) {
  return `badge-priority badge-priority--${priority}`;
}

/** Status enum → human-readable label */
export function getStatusLabel(status) {
  switch (status) {
    case 0: return 'Menunggu';
    case 1: return 'Dipanggil';
    case 2: return 'Selesai';
    case 3: return 'Batal';
    case 4: return 'Terjadwal';
    default: return 'Tidak Diketahui';
  }
}

/** Status enum → CSS class modifier for badge */
export function getStatusBadgeClass(status) {
  const map = {
    0: 'waiting',
    1: 'called',
    2: 'completed',
    3: 'cancelled',
    4: 'scheduled',
  };
  return `badge-status badge-status--${map[status] || 'waiting'}`;
}

/** Generate a random patient ID (P + 3 digits) */
export function generatePatientId() {
  const num = Math.floor(100 + Math.random() * 900);
  return `P${num}`;
}

/** Get current time as HH:MM string */
export function getCurrentTime() {
  const now = new Date();
  const hrs = String(now.getHours()).padStart(2, '0');
  const mins = String(now.getMinutes()).padStart(2, '0');
  return `${hrs}:${mins}`;
}

/** Get current time as HH:MM:SS string */
export function getCurrentTimeFull() {
  const now = new Date();
  const hrs = String(now.getHours()).padStart(2, '0');
  const mins = String(now.getMinutes()).padStart(2, '0');
  const secs = String(now.getSeconds()).padStart(2, '0');
  return `${hrs}:${mins}:${secs}`;
}

/** Get today's date as YYYY-MM-DD */
export function getTodayDate() {
  const now = new Date();
  const year = now.getFullYear();
  const month = String(now.getMonth() + 1).padStart(2, '0');
  const day = String(now.getDate()).padStart(2, '0');
  return `${year}-${month}-${day}`;
}
