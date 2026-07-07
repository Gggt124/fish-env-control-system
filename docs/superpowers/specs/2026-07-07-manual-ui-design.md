# Firmware Manual UI Modernization (docs/manual.html)

## 1. Overview
The current firmware manual (`docs/manual.html`) has readability issues. The content is dense, lacks visual structural elements (cards, icons, clear grouping), and sections are poorly divided, making it difficult for operators to scan for information quickly. This design document outlines a UI modernization effort following a "Modern Tech Docs" approach (inspired by Vercel/Stripe docs) while adhering to the project's existing "product" design register.

## 2. Core Principles
- **Scannability**: Break dense text walls into digestible chunks.
- **Visual Anchors**: Use iconography and typography to guide the eye.
- **Progressive Disclosure**: Hide deep technical details (like error codes) behind interactive elements until needed.
- **Flat-by-Default (Impeccable Product Register)**: Rely on tonal contrast (Card vs Canvas) rather than heavy drop shadows.

## 3. Structural Layout & Chunking

### 3.1 Content Layout Strategy
- **Canvas vs Cards**: The main wrapper will rely heavily on `var(--bg-body)` for the background canvas, while distinct topics will be grouped inside `var(--bg-card)` containers with a `1px` solid border (`var(--border-color)`).
- **Responsive Grid**: Sections with multiple discrete topics (e.g., Section 6 Dashboard Usage) will utilize CSS Grid (`grid-template-columns: repeat(auto-fit, minmax(300px, 1fr))`) to place related control consoles side-by-side on desktop.

### 3.2 Specific Section Restructuring
- **Section 1 & 2 (Overview & Specs)**: Add feature highlight badges. Convert the table container to a cleaner card style.
- **Section 3 (Wiring)**: Wrap the critical safety alerts and schematic SVGs in visually distinct "Step" blocks. Emphasize "Step 1", "Step 2" using numbered badges.
- **Section 6 (Dashboard Usage)**: Currently a wall of text with flat `<h3>` tags. This will be transformed into a grid of **Feature Cards** (e.g., "Pump Control Console", "Cooling Control Console", "Hardware Config"). Each card will have an SVG icon header.
- **Section 7 (Troubleshooting)**: Transform the troubleshooting tables and deep diagnostic info into **Accordion (`<details>` / `<summary>`) elements**. This will allow operators to quickly scan high-level symptoms and click to reveal detailed fixes.

## 4. New UI Components

### 4.1 SVG Iconography System
Integrate inline `<svg>` icons (following the SVG & Text-Reduction Directive) to visually anchor sections without relying on external fonts:
- 🛜 `icon-wifi` for networking sections.
- ⚡ `icon-power` for wiring/flashing.
- 💧 `icon-pump` for relay control.
- 🌡️ `icon-temp` for cooling control.
- ⚠️ `icon-warning` for troubleshooting.

### 4.2 Accordion (Details/Summary)
```css
/* Conceptual styling */
details.trouble-accordion {
    background: var(--bg-card);
    border: 1px solid var(--border-color);
    border-radius: var(--radius-md);
    margin-bottom: var(--space-sm);
}
summary {
    padding: var(--space-md);
    cursor: pointer;
    font-weight: 600;
    /* Custom caret styling */
}
```

### 4.3 Step Badges
A circular number indicator to denote strict sequential steps.
```css
.step-badge {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    width: 24px;
    height: 24px;
    border-radius: var(--radius-full);
    background-color: var(--primary-container);
    color: var(--primary);
    font-size: 0.75rem;
    font-weight: 700;
}
```

## 5. Typography & Polish
- Ensure body text line height and margins provide adequate breathing room (`line-height: 1.6`, generous `margin-bottom`).
- Ensure no aphoristic marketing copy is introduced; keep language strict, instructional, and concise.
- Refine existing Alert boxes (`.alert-warning`, `.alert-danger`) to ensure they don't use banned design tropes (like thick left-stripe borders). They should be full boxes with tinted backgrounds (which they currently are, but spacing and text hierarchy inside them will be improved).

## 6. Implementation Plan Next Steps
Once this design is approved, the next step is to run `writing-plans` to generate the exact file modification chunks for `docs/manual.html`.
