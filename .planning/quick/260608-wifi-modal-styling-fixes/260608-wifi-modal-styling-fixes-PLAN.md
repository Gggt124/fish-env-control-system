# Plan: Wi-Fi Card and Icon Style Refinements

## Must-Haves
- Update `.connection-status` card in `main/static/style.css` to use standard white card styling (background `var(--surface)`, border `1px solid var(--outline-variant)`, shadow `var(--shadow-sm)`).
- Style `.connection-status .status-icon` to be greyish (`color: var(--on-surface-variant)`) by default.

## Execution Plan

<plan>
<task>
<id>1</id>
<title>Update CSS style file</title>
<description>Update `.connection-status` and `.status-icon` in `main/static/style.css` to restore the default white card container style and make the disconnected icon color greyish.</description>
<file>main/static/style.css</file>
</task>
</plan>
