# Plan: Wi-Fi Off SVG Icon Path Fix

## Must-Haves
- Replace the broken/disjointed `wifi-off` SVG in both `main/static/index.html` and `main/static/app.js` with the standard Feather icons `wifi-off` SVG path.

## Execution Plan

<plan>
<task>
<id>1</id>
<title>Update index.html</title>
<description>Replace the disjointed SVG paths inside `#conn-icon` in `main/static/index.html` with standard Feather icons wifi-off paths.</description>
<file>main/static/index.html</file>
</task>
<task>
<id>2</id>
<title>Update app.js</title>
<description>Replace the disjointed SVG paths inside `updateConnectionStatus()` in `main/static/app.js` with standard Feather icons wifi-off paths.</description>
<file>main/static/app.js</file>
</task>
</plan>
