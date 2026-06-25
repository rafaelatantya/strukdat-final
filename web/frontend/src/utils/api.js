const API_BASE = 'http://localhost:7331/api';

/**
 * Wrapper for all API calls to the Express backend.
 * Centralizes error handling and response parsing.
 */

export async function fetchPatients() {
  const res = await fetch(`${API_BASE}/patients`);
  return res.json();
}

export async function registerPatient(data) {
  const res = await fetch(`${API_BASE}/patients`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data),
  });
  return res.json();
}

export async function checkInPatient(id, waktuDatang) {
  const res = await fetch(`${API_BASE}/checkin`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ id, waktuDatang }),
  });
  return res.json();
}

export async function callNextPatient() {
  const res = await fetch(`${API_BASE}/call-next`, { method: 'POST' });
  return res.json();
}

export async function updatePatientStatus(id, status, force = false) {
  const res = await fetch(`${API_BASE}/status`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ id, status, force }),
  });
  return res.json();
}

export async function deletePatient(id) {
  const res = await fetch(`${API_BASE}/delete`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ id }),
  });
  return res.json();
}

export async function loadDummyData() {
  const res = await fetch(`${API_BASE}/dummy`, { method: 'POST' });
  return res.json();
}

export async function runBenchmark(scale) {
  const res = await fetch(`${API_BASE}/benchmark?scale=${scale}`);
  return res.json();
}

export async function getHeapData() {
  const res = await fetch(`${API_BASE}/heap`);
  return res.json();
}
