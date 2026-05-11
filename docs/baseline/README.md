# docs/baseline/

Baseline screenshots committed after the first successful build and launch of MeshForge (PR 1.4).

## Expected files

| File | Description |
|---|---|
| `macos_baseline.png` | macOS: app window open, no model loaded |
| `macos_baseline_stl.png` | macOS: app window with cube_10mm.stl loaded |
| `windows_baseline.png` | Windows: app window open, no model loaded |
| `windows_baseline_stl.png` | Windows: app window with cube_10mm.stl loaded |

## How to update

After a successful build:
1. Launch MeshForge.
2. Screenshot the empty viewport → save as `macos_baseline.png`.
3. Load `tests/fixtures/cube_10mm.stl` → screenshot → save as `macos_baseline_stl.png`.
4. Commit both images in the same PR that updates the build.
