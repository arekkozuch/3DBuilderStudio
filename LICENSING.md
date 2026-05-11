# LICENSING.md

## Project License

This project is licensed under the **GNU Affero General Public License, version 3** (AGPL-3.0-or-later).

Full license text: https://www.gnu.org/licenses/agpl-3.0.html

---

## AGPL-3.0 Obligations — What This Means for Contributors and Distributors

### If you distribute a binary (to anyone, including privately within an org):
- You must make the complete corresponding source code available under AGPL-3.0.
- Source must be the exact source used to build that binary, including all patches and modifications.
- You must include a written offer (or a direct link) to obtain the source alongside the binary.

### If you modify the code:
- Your modifications are automatically AGPL-3.0.
- You cannot relicense the project or any derivative under a more permissive license.

### Copyright headers in source files:
- **Do not remove or modify copyright headers** from upstream files.
- When you add a new file, add a header:
  ```
  // Copyright (c) [Year] [Your Name / Org]
  // SPDX-License-Identifier: AGPL-3.0-or-later
  ```

---

## Upstream Attribution Chain

This project is a fork. The AGPL requires preserving the full attribution chain:

| Project | License | Repository |
|---|---|---|
| MeshForge (this project) | AGPL-3.0 | [this repo] |
| OrcaSlicer | AGPL-3.0 | https://github.com/OrcaSlicer/OrcaSlicer |
| BambuStudio | AGPL-3.0 | https://github.com/bambulab/BambuStudio |
| PrusaSlicer | AGPL-3.0 | https://github.com/prusa3d/PrusaSlicer |
| Slic3r | AGPL-3.0 | https://github.com/slic3r/Slic3r |

The calibration pattern test code (from Andrew Ellis / Sineos) is GPL-3.0.  
**This code must be removed** during Phase 2 cleanup as it is calibration-specific and unneeded.  
Remove before any public binary distribution.

---

## Third-Party Dependencies (vendored in `deps/`)

The following libraries are vendored in the dependency tree and have their own licenses.  
These are carried over from OrcaSlicer. Full license texts are in `deps/` subdirectories.

| Library | License | Notes |
|---|---|---|
| CGAL | LGPL-3.0 / GPL-3.0 | Core geometry. CGAL headers included under LGPL exception. |
| OpenVDB | MPL-2.0 | Boolean mesh operations. |
| admesh | GPL-2.0+ | STL repair. |
| Eigen | MPL-2.0 | Linear algebra. |
| Boost | BSL-1.0 | Various utilities. |
| wxWidgets | LGPL-2.0 + exception | UI framework. |
| OpenGL | — | System library, not vendored. |
| GLEW | BSD / MIT | OpenGL extension loading. |
| nlohmann/json | MIT | JSON parsing. |
| miniz | MIT | ZIP/3MF container. |
| libigl | MPL-2.0 | Mesh geometry utilities. |

**Before adding any new dependency**, verify its license is compatible with AGPL-3.0.  
Licenses compatible with AGPL-3.0: MIT, BSD-2/3, Apache-2.0 (with patent grant), MPL-2.0, LGPL, GPL-2+, AGPL-3.  
Licenses **not** compatible: GPL-2.0-only (no "or later"), proprietary, CC-BY-NC, SSPL.

---

## Asset Licensing

Every binary asset (icon, image, font, sound) must be documented here with its source and license.

| Asset | Source | License | Notes |
|---|---|---|---|
| *(none yet — all upstream assets replaced in Phase 1.2)* | | | |

New assets must be one of: original work (AGPL), CC0, MIT, or public domain.  
Record each addition in this table in the same PR that adds the asset.

---

## What Is NOT Covered by This License

The Bambu Lab networking plugin (a closed-source, non-free binary) is **not** part of this project and must not be included. It was optional in OrcaSlicer and is not carried forward.

---

## Source Distribution for Releases

Every release tag must be accompanied by:
1. A source archive (`git archive HEAD | gzip > meshforge-<version>.tar.gz`).
2. A link to the archive in the GitHub release notes.
3. A `SOURCE_URL` file in the binary distribution pointing to the archive.

This satisfies AGPL §13.