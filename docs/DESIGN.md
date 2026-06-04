# Design System — Hospital Queue Dashboard

A deliberately crafted design system for the Sistem Antrian RS administrative interface.  
Inspired by editorial clarity, warmth, and restraint — not generated noise.

---

## 1. Design Philosophy

The interface should feel like a **well-typeset medical journal** — authoritative, calm, and instantly legible under pressure. Hospital staff use this under stress. Every pixel must reduce cognitive load, not add decoration.

### Core Principles

| Principle | Meaning |
|---|---|
| **Clarity over cleverness** | No gratuitous animations. Every motion must communicate state change. |
| **Warm neutrality** | Neither cold-clinical nor playful. The palette evokes trust and competence. |
| **Typographic hierarchy** | Information architecture is expressed through type, not color-coding alone. |
| **Deliberate density** | Show enough data at a glance without scrolling. Respect the operator's time. |
| **Accessible contrast** | All text meets WCAG AA minimum. Priority indicators use shape + color, never color alone. |

---

## 2. Typography

The typographic system uses **two families** in a classic editorial pairing:

### Font Stack

| Role | Family | Weight | Usage |
|---|---|---|---|
| **Display / Headings** | `Playfair Display` | 700, 600 | Page titles, section headings, the large queue number callout |
| **Body / UI** | `Inter` | 400, 500, 600 | All body text, labels, table content, buttons, form inputs |
| **Monospace** | `JetBrains Mono` | 400 | Patient IDs, timestamps, benchmark metrics |

### Type Scale (rem-based, 1rem = 16px)

```
--text-xs:    0.75rem   / 12px   — Captions, helper text, timestamps
--text-sm:    0.8125rem / 13px   — Table cell text, secondary labels
--text-base:  0.9375rem / 15px   — Body text, form inputs, buttons
--text-md:    1.0625rem / 17px   — Card titles, emphasized labels
--text-lg:    1.25rem   / 20px   — Section headings
--text-xl:    1.5rem    / 24px   — Page section titles
--text-2xl:   2rem      / 32px   — Page heading (Playfair)
--text-hero:  3.5rem    / 56px   — Queue number display (Playfair)
```

### Line Height

- Headings: `1.2`
- Body: `1.6`
- UI (buttons, labels): `1.4`

### Letter Spacing

- Playfair headings: `-0.01em` (optical tightening at large sizes)
- Inter body: `0`
- Uppercase labels: `0.06em`
- Mono (IDs, times): `0.02em`

---

## 3. Color System

A warm, muted palette. No saturated primaries. Colors are defined in HSL for predictable manipulation.

### Base Palette

```css
/* Surface hierarchy (light base, warm undertone) */
--surface-base:       hsl(40, 33%, 98%);    /* #FDFBF7 — Page background */
--surface-raised:     hsl(0, 0%, 100%);     /* #FFFFFF — Cards, panels */
--surface-sunken:     hsl(40, 20%, 95%);    /* #F5F2ED — Input fields, wells */
--surface-overlay:    hsl(0, 0%, 100%);     /* Modal/dropdown backgrounds */

/* Border hierarchy */
--border-default:     hsl(30, 10%, 88%);    /* #E2DDD6 — Card borders, dividers */
--border-subtle:      hsl(30, 8%, 93%);     /* #EDEBE7 — Table row separators */
--border-focus:       hsl(25, 50%, 48%);    /* #B77830 — Focus rings */

/* Text hierarchy */
--text-primary:       hsl(20, 10%, 12%);    /* #211E1B — Headings, primary content */
--text-secondary:     hsl(20, 5%, 40%);     /* #6B6460 — Descriptions, secondary info */
--text-tertiary:      hsl(20, 4%, 56%);     /* #928D88 — Placeholders, disabled */
--text-inverse:       hsl(0, 0%, 100%);     /* #FFFFFF — On dark/colored backgrounds */
```

### Accent Colors (Functional, not decorative)

```css
/* Brand accent — warm bronze, used sparingly for primary actions */
--accent-primary:       hsl(25, 50%, 48%);    /* #B77830 */
--accent-primary-hover: hsl(25, 50%, 42%);    /* #A06828 */
--accent-primary-light: hsl(25, 60%, 96%);    /* #FDF5ED */

/* Interactive — for links, active states */
--accent-interactive:       hsl(220, 55%, 50%);   /* #3B6EC2 */
--accent-interactive-hover: hsl(220, 55%, 44%);   /* #3360AE */
```

### Priority Colors (Medical Triage)

These are the **only saturated colors** in the system. They are used for priority badges and left-border accents on patient cards.

```css
/* Priority 1: DARURAT — Warm red, demands immediate attention */
--priority-emergency:       hsl(4, 70%, 55%);     /* #D44A3C */
--priority-emergency-bg:    hsl(4, 80%, 97%);     /* #FDF2F0 */
--priority-emergency-border:hsl(4, 70%, 80%);     /* #EEAAA2 */

/* Priority 2: MENDESAK — Deep amber, urgency without panic */
--priority-urgent:          hsl(30, 80%, 50%);     /* #E68A17 */
--priority-urgent-bg:       hsl(35, 80%, 96%);     /* #FDF4E6 */
--priority-urgent-border:   hsl(30, 70%, 78%);     /* #EEC48A */

/* Priority 3: RENTAN — Warm gold, notable but calm */
--priority-vulnerable:      hsl(45, 75%, 48%);     /* #D6A817 */
--priority-vulnerable-bg:   hsl(48, 70%, 96%);     /* #FDF8E8 */
--priority-vulnerable-border:hsl(45, 60%, 80%);    /* #E8D88E */

/* Priority 4: REGULER — Sage green, steady and grounded */
--priority-regular:         hsl(160, 40%, 42%);    /* #409672 */
--priority-regular-bg:      hsl(155, 40%, 96%);    /* #EEF8F3 */
--priority-regular-border:  hsl(160, 30%, 78%);    /* #A8D4BE */
```

### Status Colors

```css
--status-waiting:     hsl(220, 55%, 50%);   /* Blue — in queue */
--status-called:      hsl(270, 50%, 55%);   /* Purple — currently being served */
--status-completed:   hsl(155, 50%, 42%);   /* Green — done */
--status-cancelled:   hsl(20, 5%, 56%);     /* Muted gray — cancelled */
--status-scheduled:   hsl(340, 55%, 58%);   /* Rose — booked, not arrived */
```

---

## 4. Spacing & Layout

### Spacing Scale (4px base)

```
--space-1:   0.25rem    /  4px
--space-2:   0.5rem     /  8px
--space-3:   0.75rem    / 12px
--space-4:   1rem       / 16px
--space-5:   1.25rem    / 20px
--space-6:   1.5rem     / 24px
--space-8:   2rem       / 32px
--space-10:  2.5rem     / 40px
--space-12:  3rem       / 48px
--space-16:  4rem       / 64px
```

### Layout Grid

- **Max container width**: `1280px`
- **Page horizontal padding**: `--space-8` (32px) on desktop, `--space-4` (16px) on mobile
- **Card internal padding**: `--space-6` (24px)
- **Section gap**: `--space-8` (32px)
- **Column gap (grid layouts)**: `--space-6` (24px)

### Border Radius

```
--radius-sm:   4px    — Badges, small chips
--radius-md:   8px    — Buttons, inputs, table cells
--radius-lg:  12px    — Cards, panels
--radius-xl:  16px    — Modals, featured sections
```

---

## 5. Elevation & Depth

No glassmorphism. No blur-heavy overlays. Depth is communicated through **subtle shadow and border alone**.

```css
/* Card resting state */
--shadow-card: 0 1px 3px hsla(20, 10%, 10%, 0.04),
               0 1px 2px hsla(20, 10%, 10%, 0.06);

/* Card hover / interactive */
--shadow-card-hover: 0 4px 12px hsla(20, 10%, 10%, 0.08),
                     0 2px 4px hsla(20, 10%, 10%, 0.04);

/* Modal overlay */
--shadow-modal: 0 16px 48px hsla(20, 10%, 10%, 0.16),
                0 4px 16px hsla(20, 10%, 10%, 0.08);

/* Focus ring (replaces glow effects) */
--shadow-focus: 0 0 0 3px hsla(25, 50%, 48%, 0.25);
```

---

## 6. Component Specifications

### 6.1 Buttons

Three tiers of visual weight. All `Inter`, 500 weight, `--text-base` size.

| Variant | Background | Border | Text Color | Use Case |
|---|---|---|---|---|
| **Primary** | `--accent-primary` solid | none | `--text-inverse` | Main action: "Tambahkan Pasien", "Panggil Berikutnya" |
| **Secondary** | transparent | `1px solid --border-default` | `--text-primary` | Supporting: "Refresh", "Load Dummy" |
| **Danger** | `--priority-emergency` solid | none | `--text-inverse` | Destructive: "Batalkan Antrian" |
| **Ghost** | transparent | none | `--text-secondary` | Tertiary: inline table actions |

**Interaction states:**
- Hover: darken bg 6%, lift `translateY(-1px)` with `--shadow-card-hover`
- Active: darken bg 10%, `translateY(0)`
- Focus: `--shadow-focus` ring (keyboard navigation)
- Disabled: `opacity: 0.5`, `cursor: not-allowed`

**Dimensions:**
- Padding: `--space-3 --space-5` (12px 20px)
- Border-radius: `--radius-md`
- Min-height: `40px`
- Compact (table actions): `--space-2 --space-3` (8px 12px), `--text-sm`

### 6.2 Cards / Panels

The fundamental container. White background, thin border, minimal shadow.

```
Background:   --surface-raised
Border:       1px solid --border-default
Radius:       --radius-lg (12px)
Padding:      --space-6 (24px)
Shadow:       --shadow-card
Hover shadow: --shadow-card-hover (only on interactive cards)
```

**Priority-accented variant** (for patient rows in calling deck):  
Left border: `3px solid var(--priority-{level})`, left padding increases to `--space-8`.

### 6.3 Form Inputs

```
Background:   --surface-sunken
Border:       1px solid --border-default
Radius:       --radius-md (8px)
Padding:      --space-3 --space-4 (12px 16px)
Font:         Inter 400, --text-base
Color:        --text-primary
Placeholder:  --text-tertiary
Focus:        border-color → --border-focus, shadow → --shadow-focus
```

### 6.4 Badges

Two families: **Priority badges** and **Status badges**.

**Priority Badge:**
- Shape: pill (`border-radius: 9999px`)
- Height: `24px`, padding `--space-1 --space-3`
- Font: Inter 600, `--text-xs`, uppercase, `letter-spacing: 0.06em`
- Background: `--priority-{level}-bg`
- Border: `1px solid --priority-{level}-border`
- Text: `--priority-{level}`

**Status Badge:**
- Shape: rounded rect (`--radius-sm`)
- Same sizing as priority badge
- Follows `--status-{state}` color tokens with 10% opacity background

### 6.5 Data Tables

Clean, readable, zero decoration.

```
Header row:     --surface-sunken bg, --text-secondary, Inter 600, --text-xs, uppercase
Body rows:      --surface-raised bg, --text-primary, Inter 400, --text-sm
Row separator:  1px solid --border-subtle
Row hover:      background → hsla(25, 40%, 50%, 0.03)
Cell padding:   --space-3 --space-4 (12px 16px)
```

No zebra striping. The subtle hover is sufficient.

### 6.6 Navigation / Tabs

Horizontal tab bar. Underline-style active indicator (not filled-box style).

```
Tab text:       Inter 500, --text-base, --text-secondary
Active tab:     --text-primary, --accent-primary underline (3px bottom-border)
Hover:          --text-primary
Container:      bottom-border 1px solid --border-default
Tab spacing:    --space-8 gap between tabs
Tab padding:    --space-3 bottom (for underline clearance)
```

### 6.7 Toast Notifications

Slide in from top-right. Auto-dismiss after 4 seconds.

```
Background:   --surface-raised
Border:       1px solid --border-default
Border-left:  3px solid (success: --status-completed, error: --priority-emergency)
Shadow:       --shadow-modal
Padding:      --space-4 --space-5
Radius:       --radius-md
Font:         Inter 500, --text-sm
Max-width:    380px
```

### 6.8 Modals

Centered overlay with backdrop.

```
Backdrop:     hsla(20, 10%, 10%, 0.4)
Panel:        --surface-raised, --radius-xl, --shadow-modal
Max-width:    480px
Padding:      --space-8
Title font:   Playfair Display 600, --text-xl
```

---

## 7. Iconography

**No emoji in production UI.** Use a consistent icon set.

- **Recommended**: [Lucide Icons](https://lucide.dev) — clean, 24px stroke icons, 1.5px stroke width
- Icons should be `--text-secondary` by default, `--text-primary` on hover/active
- Icon size: `20px` inline with text, `24px` in buttons, `32px` in stat cards

---

## 8. Motion & Animation

Restrained. Functional. Never decorative.

| Property | Duration | Easing | Usage |
|---|---|---|---|
| Color/opacity transitions | `150ms` | `ease` | Hover states, focus rings |
| Layout shifts | `200ms` | `ease-out` | Tab changes, expanding sections |
| Entrance (toast, modal) | `250ms` | `cubic-bezier(0.16, 1, 0.3, 1)` | Slide-in, fade-in |
| Exit (toast, modal) | `150ms` | `ease-in` | Slide-out, fade-out |

**Prohibited:**
- Continuous pulsing animations on non-critical elements
- Parallax or scroll-linked animations
- Loading spinners with >1 second idle display (use skeleton screens instead)

**Exception:** The calling deck (active patient being served) may use a **single gentle pulse** on the priority border to draw attention — `2s infinite, opacity 0.6 → 1.0` on the left border only.

---

## 9. Responsive Breakpoints

```
--bp-mobile:   < 640px    — Single column, stacked cards
--bp-tablet:   640–1024px — Two columns, sidebar collapses to tab
--bp-desktop:  > 1024px   — Full layout, sidebar visible
```

### Mobile Adaptations

- Navbar collapses to bottom tab bar
- Stats grid → 2×2
- Data table → card-based list view (each patient as a stacked card)
- Registration form → full-width overlay slide-up panel
- Queue number display → `--text-2xl` instead of `--text-hero`

---

## 10. CSS Custom Properties Summary

All tokens are defined on `:root` for global access. This is the single source of truth.

```css
:root {
  /* Typography */
  --font-display: 'Playfair Display', Georgia, 'Times New Roman', serif;
  --font-body: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;
  --font-mono: 'JetBrains Mono', 'Fira Code', 'Cascadia Code', monospace;

  /* Type scale */
  --text-xs: 0.75rem;
  --text-sm: 0.8125rem;
  --text-base: 0.9375rem;
  --text-md: 1.0625rem;
  --text-lg: 1.25rem;
  --text-xl: 1.5rem;
  --text-2xl: 2rem;
  --text-hero: 3.5rem;

  /* Surfaces */
  --surface-base: hsl(40, 33%, 98%);
  --surface-raised: hsl(0, 0%, 100%);
  --surface-sunken: hsl(40, 20%, 95%);

  /* Borders */
  --border-default: hsl(30, 10%, 88%);
  --border-subtle: hsl(30, 8%, 93%);
  --border-focus: hsl(25, 50%, 48%);

  /* Text */
  --text-primary: hsl(20, 10%, 12%);
  --text-secondary: hsl(20, 5%, 40%);
  --text-tertiary: hsl(20, 4%, 56%);
  --text-inverse: hsl(0, 0%, 100%);

  /* Accent */
  --accent-primary: hsl(25, 50%, 48%);
  --accent-primary-hover: hsl(25, 50%, 42%);
  --accent-interactive: hsl(220, 55%, 50%);

  /* Priority */
  --priority-emergency: hsl(4, 70%, 55%);
  --priority-urgent: hsl(30, 80%, 50%);
  --priority-vulnerable: hsl(45, 75%, 48%);
  --priority-regular: hsl(160, 40%, 42%);

  /* Status */
  --status-waiting: hsl(220, 55%, 50%);
  --status-called: hsl(270, 50%, 55%);
  --status-completed: hsl(155, 50%, 42%);
  --status-cancelled: hsl(20, 5%, 56%);
  --status-scheduled: hsl(340, 55%, 58%);

  /* Spacing */
  --space-1: 0.25rem;
  --space-2: 0.5rem;
  --space-3: 0.75rem;
  --space-4: 1rem;
  --space-5: 1.25rem;
  --space-6: 1.5rem;
  --space-8: 2rem;
  --space-10: 2.5rem;
  --space-12: 3rem;
  --space-16: 4rem;

  /* Radius */
  --radius-sm: 4px;
  --radius-md: 8px;
  --radius-lg: 12px;
  --radius-xl: 16px;

  /* Shadows */
  --shadow-card: 0 1px 3px hsla(20, 10%, 10%, 0.04),
                 0 1px 2px hsla(20, 10%, 10%, 0.06);
  --shadow-card-hover: 0 4px 12px hsla(20, 10%, 10%, 0.08),
                       0 2px 4px hsla(20, 10%, 10%, 0.04);
  --shadow-modal: 0 16px 48px hsla(20, 10%, 10%, 0.16),
                  0 4px 16px hsla(20, 10%, 10%, 0.08);
  --shadow-focus: 0 0 0 3px hsla(25, 50%, 48%, 0.25);

  /* Motion */
  --ease-default: ease;
  --ease-out: ease-out;
  --ease-spring: cubic-bezier(0.16, 1, 0.3, 1);
  --duration-fast: 150ms;
  --duration-normal: 200ms;
  --duration-enter: 250ms;
}
```

---

## 11. Google Fonts Import

```html
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600&family=JetBrains+Mono:wght@400&family=Playfair+Display:wght@600;700&display=swap" rel="stylesheet">
```

---

## 12. Anti-Patterns (What NOT To Do)

| Don't | Do Instead |
|---|---|
| Glassmorphism / backdrop-filter blur | Clean solid backgrounds with subtle borders |
| Neon glow effects on text or borders | Warm, muted shadow and natural contrast |
| Emoji as icons (🏥 ⏳ 📅) | Lucide icon set, monochrome, consistent stroke |
| Gradient text fills | Solid `--text-primary` for headings |
| Dark mode as default | Light, warm base — medical environments are well-lit |
| Color alone for priority/status | Shape (pill vs rect) + label text + color together |
| Oversized border-radius (16px+ on cards) | `12px` max for cards, `8px` for inputs |
| Inline styles in JSX | CSS classes referencing design tokens |
