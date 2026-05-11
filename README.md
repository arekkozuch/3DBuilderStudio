# MeshForge

> **Working name only.** Final name must be confirmed against USPTO TESS, GitHub, and domain registries before any public release. Do not use "3D Builder" — Microsoft trademark.

A free, open-source, native desktop 3D mesh-editing tool targeting the audience of the discontinued Microsoft 3D Builder (deprecated July 2024).

Forked from **OrcaSlicer** (`OrcaSlicer/OrcaSlicer`, AGPL-3.0). All 3D-printing, slicing, G-code, calibration, and printer-networking code is removed. The retained core — OpenGL viewport, object manipulation, libslic3r geometry backend (CGAL + OpenVDB + admesh) — is extended with mesh-editing features (primitives, booleans, plane cut, settle/orient).

---

## Goal

Replicate the core 3D Builder experience:
- Import STL / OBJ / 3MF
- Insert primitives (cube, sphere, cylinder, cone)
- Boolean subtract, union, intersect
- Plane cut / split
- Settle (align face to ground)
- Measure distances
- Export clean STL / 3MF (geometry only, no slicer metadata)

Non-goals: slicing, G-code, printer connection, cloud sync, accounts, telemetry.

---

## Tech Stack

| Layer | Component |
|---|---|
| Language | C++17 |
| UI Framework | wxWidgets (bundled fork) |
| 3D Rendering | OpenGL 3.3+ |
| Geometry Engine | `libslic3r` → CGAL, OpenVDB, admesh |
| Build System | CMake ≥ 3.21, Ninja (macOS), MSBuild (Windows) |
| Unit Tests | Catch2 v3 |
| Fuzz Tests | libFuzzer (separate harnesses, not Catch2) |

**macOS note:** OpenGL is deprecated since macOS 10.14 but functional. No Metal port is planned in this version; noted as future work.

---

## Platform Support

| Platform | Architecture | Status |
|---|---|---|
| macOS 12+ | arm64 + x86_64 (universal2) | Primary |
| Windows 10 22H2 / 11 | x64 | Primary |
| Linux | x64 (AppImage) | Best-effort |

---

## License

**AGPL-3.0-or-later.** See `LICENSING.md` for full obligations.  
Any distribution (binary or source) requires publishing full corresponding source under AGPL-3.0.  
All upstream copyright headers (`Copyright (c) ...`) are legally required and must not be removed.

---

## Documentation Index

| File | Purpose |
|---|---|
| `DEVELOPMENT_PLAN.md` | Phased task list, one feature per PR, with tests |
| `BOUNDARIES.md` | Hard rules for AI coding agents |
| `CONTRIBUTING.md` | PR workflow, branch model, local verify script |
| `LICENSING.md` | AGPL obligations, asset attributions |

---

## Quick-Start Build (after deps are built)

**macOS**
```bash
scripts/setup_deps_mac.sh          # first time only, ~60–90 min
cmake -B build -G Ninja \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
  -DSLIC3R_NETWORK=OFF
ninja -C build -j$(sysctl -n hw.ncpu)
```

**Windows** (Developer PowerShell for VS 2022)
```bat
scripts\setup_deps_win.bat         # first time only, ~90–120 min
cmake -B build -G "Visual Studio 17 2022" -A x64 -DSLIC3R_NETWORK=OFF
cmake --build build --config Release
```

**Run tests**
```bash
scripts/verify.sh                  # required before every PR
```