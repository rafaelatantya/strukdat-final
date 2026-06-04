import React from 'react';

/**
 * Pure SVG bar chart comparing two values.
 * Used in the benchmark view to compare Priority Queue vs Standard Queue.
 */
export default function BarChart({ valA, valB, labelA, labelB, colorA, colorB }) {
  const maxVal = Math.max(valA, valB, 1);
  const maxHeight = 140;
  const heightA = Math.max(Math.round((valA / maxVal) * maxHeight), 4);
  const heightB = Math.max(Math.round((valB / maxVal) * maxHeight), 4);

  // Colors from design tokens (passed as CSS variable strings or hex)
  const barColorA = colorA || 'hsl(220, 55%, 50%)';
  const barColorB = colorB || 'hsl(20, 4%, 56%)';

  return (
    <svg
      width="280"
      height="220"
      viewBox="0 0 280 220"
      role="img"
      aria-label={`Bar chart: ${labelA} = ${valA}μs, ${labelB} = ${valB}μs`}
      style={{
        background: 'hsl(40, 20%, 95%)',
        borderRadius: '8px',
      }}
    >
      {/* Grid lines */}
      <line x1="40" y1="40" x2="260" y2="40" stroke="hsl(30, 10%, 88%)" strokeDasharray="4" />
      <line x1="40" y1="110" x2="260" y2="110" stroke="hsl(30, 10%, 88%)" strokeDasharray="4" />
      <line x1="40" y1="180" x2="260" y2="180" stroke="hsl(30, 10%, 82%)" />

      {/* Bar A */}
      <rect
        x="70"
        y={180 - heightA}
        width="48"
        height={heightA}
        fill={barColorA}
        rx="4"
      />
      <text
        x="94"
        y={175 - heightA}
        fill="hsl(20, 10%, 12%)"
        fontSize="11"
        textAnchor="middle"
        fontWeight="600"
        fontFamily="'JetBrains Mono', monospace"
      >
        {valA.toLocaleString()} μs
      </text>

      {/* Bar B */}
      <rect
        x="162"
        y={180 - heightB}
        width="48"
        height={heightB}
        fill={barColorB}
        rx="4"
      />
      <text
        x="186"
        y={175 - heightB}
        fill="hsl(20, 10%, 12%)"
        fontSize="11"
        textAnchor="middle"
        fontWeight="600"
        fontFamily="'JetBrains Mono', monospace"
      >
        {valB.toLocaleString()} μs
      </text>

      {/* X-axis labels */}
      <text
        x="94"
        y="198"
        fill="hsl(20, 5%, 40%)"
        fontSize="10"
        textAnchor="middle"
        fontWeight="500"
        fontFamily="'Inter', sans-serif"
      >
        {labelA || 'PQ (Heap)'}
      </text>
      <text
        x="186"
        y="198"
        fill="hsl(20, 5%, 40%)"
        fontSize="10"
        textAnchor="middle"
        fontWeight="500"
        fontFamily="'Inter', sans-serif"
      >
        {labelB || 'FIFO'}
      </text>

      {/* Footer */}
      <text
        x="140"
        y="215"
        fill="hsl(20, 4%, 56%)"
        fontSize="9"
        textAnchor="middle"
        fontFamily="'Inter', sans-serif"
      >
        *Lebih pendek = lebih cepat (μs = microseconds)
      </text>
    </svg>
  );
}
