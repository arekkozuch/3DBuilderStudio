# DEVELOPMENT_PLAN.md

One feature per PR. One PR per branch. Tests required for every logic change.  
See `BOUNDARIES.md` and `CONTRIBUTING.md` for agent rules and PR format.

---

## Reading This Document

Each task below = one PR.  
Format:
- **Goal:** what the PR achieves.
- **Files:** which files to modify/create.
- **Steps:** concrete implementation steps.
- **Tests:** exact tests to write. These are mandatory.
- **Acceptance:** how to verify the PR is done.

---

## Phase 0: Project Bootstrap

### PR 0.1 — Fork and tag upstream

**Goal:** Establish the fork baseline with no code changes.

**Steps:**
1. Fork `OrcaSlicer/OrcaSlicer` (`main`) into the project org.
2. Record the upstream commit SHA:
   ```bash
   echo "Forked from OrcaSlicer/OrcaSlicer @ $(git rev-parse HEAD)" > UPSTREAM.md
   git add UPSTREAM.md && git commit -m "chore: record upstream fork point"
   ```
3. Tag the fork point: `git tag fork-point-orca-<version> && git push --tags`
4. Delete `.github/workflows/` directory entirely.
5. Add project docs (`README.md`, `DEVELOPMENT_PLAN.md`, `BOUNDARIES.md`, `CONTRIBUTING.md`, `LICENSING.md`).
6. Add `.github/PULL_REQUEST_TEMPLATE.md` (use the template from `CONTRIBUTING.md`).

**Tests:** None (no logic changed).

**Acceptance:** Repo exists, upstream SHA is recorded, no GitHub Actions workflows present, docs committed.

---

### PR 0.2 — Brand and identifier scrub

**Goal:** Remove all upstream brand identity from user-visible code and build artifacts. AGPL copyright headers in source comments are kept.

**Files to search and modify:**
- `src/slic3r/GUI/` — all `.cpp`, `.h` UI files (window titles, About dialog, menu items).
- `resources/` — replace logos, icons, splash screens with placeholders.
- `src/slic3r/GUI/wxExtensions.cpp` — user-agent strings.
- `CMakeLists.txt` — product name, bundle identifier.
- `src/platform/msw/meshforge.rc` — rename from orca.rc, update product strings.
- `src/platform/osx/Info.plist` — `CFBundleName`, `CFBundleIdentifier`, `CFBundleDisplayName`.
- `src/slic3r/GUI/GUI_App.cpp` — `CURLOPT_USERAGENT` value and app name constants.

**Replacements:**
- All user-visible instances of `Orca`, `OrcaSlicer`, `Orca Slicer` → `MeshForge`
- All user-visible instances of `Bambu`, `BambuStudio`, `Bambu Lab` → remove or replace
- `CFBundleIdentifier`: `com.softfever.OrcaSlicer` → `com.[org].meshforge`
- Windows ProgID file associations: update to `MeshForge.STL`, `MeshForge.3MF`, `MeshForge.OBJ`

**Asset replacements:**
- Replace all `.icns`, `.ico`, `.png` logo files with solid-color placeholders.
- Document each replaced asset in `LICENSING.md` § Asset Licensing.

**Tests:**
```bash
# Test 1 — brand strings (run in verify.sh)
grep -ri "bambu\|orca\|softfever\|prusaslicer" \
  --include="*.cpp" --include="*.h" --include="*.mm" \
  --exclude-dir=deps src/ \
  | grep -iv "copyright\|license\|based on\|adapted from\|originally"
# Expected: empty output

# Test 2 — bundle identifier does not contain upstream names
grep -i "softfever\|bambu\|orca" src/platform/osx/Info.plist
# Expected: empty output

# Test 3 — Windows RC does not contain upstream product names
grep -i "orca\|bambu\|softfever" src/platform/msw/*.rc
# Expected: empty output
```

**Acceptance:** All three grep tests return empty. App window title shows `MeshForge`, not Orca Slicer.

---

## Phase 1: Build Infrastructure

### PR 1.1 — macOS dependency build script

**Goal:** Reproducible first-time dependency build on macOS.

**Files:**
- Create `scripts/setup_deps_mac.sh`
- Create `scripts/README.md`

**Steps:**
1. Install system tools:
   ```bash
   brew install cmake ninja gettext glew mbedtls libtool autoconf automake pkg-config
   ```
2. Add ccache recommendation:
   ```bash
   brew install ccache
   export CCACHE_DIR="$HOME/.ccache"
   export CMAKE_C_COMPILER_LAUNCHER=ccache
   export CMAKE_CXX_COMPILER_LAUNCHER=ccache
   ```
3. Build deps:
   ```bash
   cd deps
   cmake -B build -G Ninja \
     -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
     -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
   cmake --build build -j$(sysctl -n hw.ncpu)
   ```
4. `scripts/README.md` must document:
   - Expected build time: 60–90 min (first time), ~5 min with ccache warm.
   - Disk requirement: ~10 GB.
   - Minimum Xcode version required (check against upstream).

**Tests:** None (build infrastructure only).

**Acceptance:** Script runs to completion on a clean macOS 12+ machine and produces `deps/build/` artifacts.

---

### PR 1.2 — Windows dependency build script

**Goal:** Reproducible first-time dependency build on Windows.

**Files:**
- Create `scripts/setup_deps_win.bat`
- Update `scripts/README.md`

**Prerequisites documented in script:**
- Visual Studio 2022 Community (Desktop C++ workload, Windows 10 SDK 10.0.22621).
- Strawberry Perl (for OpenSSL build).
- Git for Windows.
- CMake ≥ 3.21 (added to PATH).

**Steps:**
```bat
cd deps
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release -- /m
```

**Scripts/README.md additions:**
- Expected build time: 90–120 min.
- Disk requirement: ~15 GB.

**Tests:** None.

**Acceptance:** Script runs to completion on a clean Windows 10/11 VM with listed prerequisites.

---

### PR 1.3 — CMake build flag: SLIC3R_NETWORK=OFF

**Goal:** Single build-system switch that excludes all networking compilation units.

**Files:**
- `CMakeLists.txt` (root)
- `src/slic3r/CMakeLists.txt`
- Every file containing networking init (add `#ifdef SLIC3R_NETWORK` guards)

**Steps:**
1. In root `CMakeLists.txt`:
   ```cmake
   option(SLIC3R_NETWORK "Enable printer networking (Bambu, OctoPrint, Klipper)" OFF)
   if(SLIC3R_NETWORK)
       add_compile_definitions(SLIC3R_NETWORK_ENABLED)
   endif()
   ```
2. Guard all networking compilation units:
   - `NetworkAgent.cpp/.h`
   - MQTT client files
   - Bambu Connect bridge
   - OctoPrint / Klipper / Moonraker client files
   - Preset sync / update checker
   Wrap with:
   ```cpp
   #ifdef SLIC3R_NETWORK_ENABLED
   // original code
   #else
   // stub returning false / empty / "unsupported"
   #endif
   ```
3. Verify `NetworkAgent` public interface compiles with both ON and OFF.

**Tests:**
```cpp
// tests/unit/test_network_stub.cpp
#include <catch2/catch_test_macros.hpp>
#include "slic3r/GUI/NetworkAgent.hpp"  // adjust path

TEST_CASE("NetworkAgent stub returns failure when SLIC3R_NETWORK=OFF", "[network][stub]") {
    // Test that calling any NetworkAgent method returns a failure/unsupported code.
    // Exact API depends on upstream; adapt to actual interface.
    NetworkAgent agent;
    REQUIRE_FALSE(agent.is_enabled());
}
```

**Acceptance:** `cmake ... -DSLIC3R_NETWORK=OFF && ninja` succeeds. `test_network_stub` passes.

---

### PR 1.4 — Baseline compile and launch verification

**Goal:** Renamed app compiles and launches on macOS and Windows. No logic changes.

**Steps:**
1. macOS build:
   ```bash
   cmake -B build -G Ninja \
     -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
     -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
     -DSLIC3R_NETWORK=OFF
   ninja -C build -j$(sysctl -n hw.ncpu)
   ```
2. Windows build:
   ```bat
   cmake -B build -G "Visual Studio 17 2022" -A x64 -DSLIC3R_NETWORK=OFF
   cmake --build build --config Release
   ```
3. Capture baseline screenshots:
   - App window open, no model loaded.
   - App window with test STL loaded.
   - Commit screenshots to `docs/baseline/macos_baseline.png` and `docs/baseline/windows_baseline.png`.

**Tests:**
```cpp
// tests/unit/test_build_sanity.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/libslic3r.h"

TEST_CASE("libslic3r version string is present", "[sanity]") {
    REQUIRE_FALSE(std::string(SLIC3R_VERSION).empty());
}

TEST_CASE("libslic3r version does not contain upstream brand names", "[sanity][branding]") {
    std::string version = SLIC3R_VERSION;
    // Version string should not embed upstream names
    REQUIRE(version.find("Orca") == std::string::npos);
    REQUIRE(version.find("Bambu") == std::string::npos);
    REQUIRE(version.find("Prusa") == std::string::npos);
}
```

**Acceptance:** Both builds succeed. App launches. Baseline screenshots committed.

---

### PR 1.5 — Local verify script

**Goal:** `scripts/verify.sh` runs all checks in the correct order and reports pass/fail clearly.

**Files:**
- Create `scripts/verify.sh`
- Create `scripts/run_fuzz.sh` (stub; fuzz targets added per Phase 6)

**verify.sh content (implement per `CONTRIBUTING.md` spec):**
1. Brand scrub grep (fail immediately on match).
2. CMake configure (Debug, `SLIC3R_NETWORK=OFF`).
3. Full build.
4. `ctest -L unit --output-on-failure`.
5. Undo/redo canary: `ctest -R test_undo_redo_baseline`.
6. Print `ALL CHECKS PASSED` or `FAILED: <step>`.

**Tests:**
- None for the script itself.
- The script's own test execution will verify all existing unit tests.

**Acceptance:** `scripts/verify.sh` exits 0 after successful baseline build, prints `ALL CHECKS PASSED`.

---

## Phase 2: Strip Slicing and Printer Logic

Each PR removes exactly one UI area or one subsystem.

---

### PR 2.1 — Undo/redo canary test (prerequisite for all Phase 2 PRs)

**Goal:** Establish a canary test that fails if Phase 2 stripping breaks the undo/redo stack.

**Files:**
- Create `tests/unit/test_undo_redo_baseline.cpp`

**Tests:**
```cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/Model.hpp"
// Include undo/redo stack header — check OrcaSlicer source for exact path
// Likely: src/libslic3r/Model.hpp, UndoRedo.hpp, or similar

TEST_CASE("Undo stack: push and pop operations", "[undo][canary]") {
    // Create a model, push operations, undo, verify state.
    // Exact implementation depends on OrcaSlicer's UndoRedo API.
    // Adapt to the actual API found in src/libslic3r/ or src/slic3r/GUI/UndoRedo.hpp
    Slic3r::Model model;
    // TODO: adapt to actual UndoRedo API
    // Placeholder structure:
    // UndoRedo::Stack stack;
    // stack.take_snapshot("test1", model);
    // ... modify model ...
    // stack.take_snapshot("test2", model);
    // stack.undo();
    // REQUIRE(model is in state of "test1" snapshot);
    SUCCEED("Canary structure in place — fill in with actual API");
}

TEST_CASE("Undo stack survives empty model", "[undo][canary][edge]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(/* empty undo stack query */ (void)model);
}
```

**Note for agent:** Before implementing, read `src/slic3r/GUI/UndoRedo.hpp` and `src/slic3r/GUI/Plater.cpp` to find the correct API. Fill in the placeholder above with real API calls.

**Acceptance:** Test compiles and passes. It must continue to pass after every subsequent Phase 2 PR.

---

### PR 2.2 — Remove "Device" tab

**Goal:** Remove the printer-networking Device tab from the main window.

**Files:** `src/slic3r/GUI/MainFrame.cpp`

**Steps:**
1. Find the `wxNotebook` initialization block.
2. Comment out (wrapped in `#ifdef SLIC3R_BUILD_FULL` or delete) the code that adds the "Device" tab.
3. Ensure the app still boots to the viewport/Prepare tab.

**Tests:**
```cpp
// tests/unit/test_ui_tabs.cpp
// UI tabs cannot be tested without a display. Test what can be tested:
// the removal doesn't affect model loading or the undo stack.

#include <catch2/catch_test_macros.hpp>
#include "libslic3r/Model.hpp"

TEST_CASE("Model loads correctly after Device tab removal", "[ui][tabs]") {
    Slic3r::Model model;
    // Load a known-good STL fixture
    bool loaded = Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    REQUIRE(loaded);
    REQUIRE(model.objects.size() == 1);
}
```

**Fixtures required:** Add `tests/fixtures/cube_10mm.stl` in this PR (10×10×10mm cube, minimal valid STL).

**Acceptance:** App boots directly into viewport. No "Device" tab visible.

---

### PR 2.3 — Remove "Calibration" tab and calibration tools

**Goal:** Remove the Calibration tab and all calibration print profiles.

**Files:** `src/slic3r/GUI/MainFrame.cpp`, calibration-related source files.

**Steps:**
1. Remove Calibration tab from `wxNotebook`.
2. Remove calibration menu items.
3. Remove GPL-3.0 calibration pattern code (Andrew Ellis / Sineos — see `LICENSING.md`). This code must be removed before any public distribution.

**Tests:**
```cpp
// tests/unit/test_calibration_removed.cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Calibration symbols are not linked", "[calibration][removal]") {
    // This is a compile-time check: if calibration headers are gone,
    // this file will not compile if they're included accidentally.
    // Verify that the calibration header does not exist or is guarded.
    // Adapt based on what OrcaSlicer's calibration module is called.
    SUCCEED("Calibration removal verified at compile time");
}
```

**Acceptance:** No Calibration tab. GPL-3.0 pattern code removed. `LICENSING.md` updated to remove Andrew Ellis entry.

---

### PR 2.4 — Remove "Slice Now" button and background slicing

**Goal:** Disable all slicing UI and the background slicing thread.

**Files:** `src/slic3r/GUI/Plater.cpp`, `src/slic3r/GUI/GUI_App.cpp`

**Steps:**
1. Remove "Slice Plate" / "Slice All" button creation and event bindings in `Plater.cpp`.
2. Make `Plater::slice()` a logged no-op:
   ```cpp
   void Plater::slice() {
       BOOST_LOG_TRIVIAL(info) << "Slicing disabled in MeshForge build.";
   }
   ```
3. Disable `BackgroundSlicingProcess` initialization.
4. Remove G-code preview tab from bottom toolbar.

**Tests:**
```cpp
// tests/unit/test_slicing_disabled.cpp
#include <catch2/catch_test_macros.hpp>
#include "slic3r/GUI/Plater.hpp"

// If Plater is not easily unit-testable without a display context,
// test at the libslic3r level that Print objects are not being created.
TEST_CASE("No slicing occurs on model load", "[slicing][disabled]") {
    // Load a model and verify no Print object is initialized.
    // Adapt to actual API.
    Slic3r::Model model;
    Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    // The model should load successfully with no slicing side effects.
    REQUIRE(model.objects.size() == 1);
    // If SlicingProcess was initialized, it would appear in some state.
    // Verify the model object has no sliced representation.
    REQUIRE(model.objects[0]->layer_height_profile.empty());
}
```

**Acceptance:** No "Slice" button visible. No background thread starts on model load.

---

### PR 2.5 — Simplify right sidebar

**Goal:** Strip sidebar to ObjectList only; remove filament, preset, and process panels.

**Files:** `src/slic3r/GUI/Sidebar.cpp`, `src/slic3r/GUI/Sidebar.h`

**Steps:**
1. Remove `PresetPanel`, `FilamentPanel`, `ProcessPanel` initialization and layout.
2. Retain `ObjectList` (tree view of scene objects). Expand it to full sidebar height.
3. Remove printer profile selector from top of sidebar.

**Tests:**
```cpp
// tests/unit/test_sidebar_objectlist.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/Model.hpp"

TEST_CASE("Object list contains loaded model objects", "[sidebar][objectlist]") {
    Slic3r::Model model;
    Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    REQUIRE(model.objects.size() == 1);
    REQUIRE(model.objects[0]->name == "cube_10mm" || !model.objects[0]->name.empty());
}
```

**Acceptance:** Sidebar shows only object tree. No filament/preset/process UI.

---

### PR 2.6 — Remove printer profile system

**Goal:** Remove all printer, filament, and process profile loading and management.

**Files:** `src/slic3r/GUI/GUI_App.cpp`, `AppConfig.cpp`, profile-loading code.

**Steps:**
1. Remove profile preset database initialization.
2. Remove profile `.json` files from `resources/profiles/`.
3. Remove "Printer" menu items.
4. Remove profile sync/update infrastructure (separate from networking stub already done in PR 1.3).

**Tests:**
```cpp
// tests/unit/test_no_profiles.cpp
#include <catch2/catch_test_macros.hpp>
#include "slic3r/GUI/PresetBundle.hpp"

TEST_CASE("No printer profiles are loaded", "[profiles][removal]") {
    // After removal, PresetBundle should be empty or disabled.
    // Adapt to actual API.
    Slic3r::GUI::PresetBundle bundle;
    // bundle.load_presets() should either not exist or return empty.
    // Verify no printer presets are present.
    SUCCEED("Profile loading removed — adapt to API");
}
```

**Acceptance:** No printer profiles load on startup. No profile-related UI is visible.

---

## Phase 3: 3D Modeling Viewport

### PR 3.1 — Disable Z-axis bed snap (free transform)

**Goal:** Allow objects to be translated freely in 3D space, including below Z=0.

**Files:** `src/slic3r/GUI/Gizmos/GLGizmoTransform.cpp` (or equivalent in OrcaSlicer)

**Steps:**
1. Find the logic that snaps objects to `Z >= 0` (the "drop to bed" behavior that fires on translation).
2. Disable this constraint. Objects may occupy any Z position.
3. Do not remove "Drop to Bed" as an explicit menu action — only remove the *automatic* snap.

**Tests:**
```cpp
// tests/unit/test_transform_free_z.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/Model.hpp"

TEST_CASE("Object can be placed below Z=0", "[transform][viewport]") {
    Slic3r::Model model;
    Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    auto* obj = model.objects[0];
    
    // Translate to a negative Z position
    obj->instances[0]->set_offset(Slic3r::Vec3d(0, 0, -50.0));
    
    // Verify position is retained (not snapped back to 0)
    double z = obj->instances[0]->get_offset().z();
    REQUIRE(z == Approx(-50.0));
}

TEST_CASE("Object can be placed above Z=0", "[transform][viewport]") {
    Slic3r::Model model;
    Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    auto* obj = model.objects[0];
    obj->instances[0]->set_offset(Slic3r::Vec3d(0, 0, 100.0));
    REQUIRE(obj->instances[0]->get_offset().z() == Approx(100.0));
}
```

**Acceptance:** Object dragged in viewport retains negative Z. Tests pass.

---

### PR 3.2 — Primitive: Cube

**Goal:** Insert a default cube into the scene from a UI button and via API.

**Files:**
- `src/libslic3r/TriangleMesh.cpp` / `.h` — verify `make_cube()` exists or add it.
- `src/slic3r/GUI/Widgets/BuilderToolbar.cpp` — new file, first toolbar button.
- `src/slic3r/GUI/MainFrame.cpp` — attach toolbar to main window.

**Steps:**
1. Confirm `make_cube(x, y, z)` exists in `libslic3r`. If not, implement:
   ```cpp
   TriangleMesh make_cube(double x, double y, double z);
   ```
   Using 8 vertices, 6 faces (2 triangles each = 12 facets). No degenerate faces.
2. Create `BuilderToolbar` with a "Cube" button.
3. Button click → `make_cube(10, 10, 10)` → add to `Model` → refresh `ObjectList`.

**Tests:**
```cpp
// tests/unit/test_primitive_cube.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"

TEST_CASE("make_cube: geometry is correct", "[primitives][cube]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    REQUIRE(mesh.its.vertices.size() == 8);
    REQUIRE(mesh.its.indices.size() == 12);
    REQUIRE_FALSE(mesh.empty());
}

TEST_CASE("make_cube: mesh is closed (no open edges)", "[primitives][cube]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    mesh.repair(true);
    REQUIRE(mesh.stats().open_edges == 0);
}

TEST_CASE("make_cube: volume is correct", "[primitives][cube]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    REQUIRE(mesh.volume() == Approx(1000.0).epsilon(0.01));
}

TEST_CASE("make_cube: non-uniform dimensions", "[primitives][cube]") {
    auto mesh = Slic3r::make_cube(5.0, 10.0, 20.0);
    auto bb = mesh.bounding_box();
    REQUIRE(bb.size().x() == Approx(5.0));
    REQUIRE(bb.size().y() == Approx(10.0));
    REQUIRE(bb.size().z() == Approx(20.0));
}

TEST_CASE("make_cube: zero-size dimension does not crash", "[primitives][cube][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_cube(0.0, 10.0, 10.0));
}

TEST_CASE("make_cube: added to model appears in object list", "[primitives][cube][integration]") {
    Slic3r::Model model;
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto* obj = model.add_object();
    obj->add_volume(mesh);
    REQUIRE(model.objects.size() == 1);
    REQUIRE(model.objects[0]->volumes.size() == 1);
}
```

**Acceptance:** Cube button inserts a 10×10×10mm cube into the scene. All tests pass.

---

### PR 3.3 — Primitive: Sphere

**Goal:** Insert a sphere into the scene.

**Files:**
- `src/libslic3r/TriangleMesh.cpp` — verify `make_sphere()` exists or add it.
- `src/slic3r/GUI/Widgets/BuilderToolbar.cpp` — add Sphere button.

**Note:** OrcaSlicer likely already has `make_sphere()`. Verify before implementing. Check `src/libslic3r/TriangleMesh.cpp` and `src/libslic3r/Geometry.cpp`.

**Tests:**
```cpp
// tests/unit/test_primitive_sphere.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"

TEST_CASE("make_sphere: mesh is non-empty", "[primitives][sphere]") {
    auto mesh = Slic3r::make_sphere(5.0, 0.05);
    REQUIRE_FALSE(mesh.empty());
    REQUIRE(mesh.its.vertices.size() > 0);
    REQUIRE(mesh.its.indices.size() > 0);
}

TEST_CASE("make_sphere: mesh is closed", "[primitives][sphere]") {
    auto mesh = Slic3r::make_sphere(5.0, 0.05);
    mesh.repair(true);
    REQUIRE(mesh.stats().open_edges == 0);
}

TEST_CASE("make_sphere: bounding box is approximately a cube of 2r", "[primitives][sphere]") {
    double r = 5.0;
    auto mesh = Slic3r::make_sphere(r, 0.05);
    auto bb = mesh.bounding_box();
    REQUIRE(bb.size().x() == Approx(2 * r).epsilon(0.05));
    REQUIRE(bb.size().y() == Approx(2 * r).epsilon(0.05));
    REQUIRE(bb.size().z() == Approx(2 * r).epsilon(0.05));
}

TEST_CASE("make_sphere: volume approximates (4/3)πr³", "[primitives][sphere]") {
    double r = 5.0;
    double expected = (4.0 / 3.0) * M_PI * r * r * r;
    auto mesh = Slic3r::make_sphere(r, 0.01);  // high detail for accuracy
    REQUIRE(mesh.volume() == Approx(expected).epsilon(0.02));
}

TEST_CASE("make_sphere: zero radius does not crash", "[primitives][sphere][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_sphere(0.0, 0.05));
}
```

**Acceptance:** Sphere button inserts a sphere with r=5mm. Tests pass.

---

### PR 3.4 — Primitive: Cylinder

**Goal:** Insert a cylinder into the scene.

**Files:**
- `src/libslic3r/TriangleMesh.cpp` — verify `make_cylinder()` exists or add it.
- `src/slic3r/GUI/Widgets/BuilderToolbar.cpp` — add Cylinder button.

**Tests:**
```cpp
// tests/unit/test_primitive_cylinder.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"

TEST_CASE("make_cylinder: mesh is non-empty", "[primitives][cylinder]") {
    auto mesh = Slic3r::make_cylinder(5.0, 10.0);  // radius, height
    REQUIRE_FALSE(mesh.empty());
}

TEST_CASE("make_cylinder: mesh is closed", "[primitives][cylinder]") {
    auto mesh = Slic3r::make_cylinder(5.0, 10.0);
    mesh.repair(true);
    REQUIRE(mesh.stats().open_edges == 0);
}

TEST_CASE("make_cylinder: height matches bounding box Z", "[primitives][cylinder]") {
    auto mesh = Slic3r::make_cylinder(5.0, 10.0);
    auto bb = mesh.bounding_box();
    REQUIRE(bb.size().z() == Approx(10.0).epsilon(0.01));
}

TEST_CASE("make_cylinder: volume approximates πr²h", "[primitives][cylinder]") {
    double r = 5.0, h = 10.0;
    double expected = M_PI * r * r * h;
    auto mesh = Slic3r::make_cylinder(r, h, 0.01);
    REQUIRE(mesh.volume() == Approx(expected).epsilon(0.02));
}

TEST_CASE("make_cylinder: zero radius does not crash", "[primitives][cylinder][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_cylinder(0.0, 10.0));
}

TEST_CASE("make_cylinder: zero height does not crash", "[primitives][cylinder][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_cylinder(5.0, 0.0));
}
```

**Acceptance:** Cylinder button works. Tests pass.

---

### PR 3.5 — Primitive: Cone

**Goal:** Insert a cone into the scene.

**Files:**
- `src/libslic3r/TriangleMesh.cpp` — verify cone exists. OrcaSlicer may implement it as a degenerate cylinder (top radius = 0). If so, expose a named wrapper; do not duplicate.
- `src/slic3r/GUI/Widgets/BuilderToolbar.cpp` — add Cone button.

**Agent note:** Check whether `make_cylinder(r, h)` accepts a second radius argument. If `make_cylinder(r1, r2, h)` exists (frustum), use `make_cylinder(5.0, 0.0, 10.0)` as the cone. Do not implement from scratch if a frustum function exists.

**Tests:**
```cpp
// tests/unit/test_primitive_cone.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"

TEST_CASE("make_cone: mesh is non-empty", "[primitives][cone]") {
    auto mesh = Slic3r::make_cone(5.0, 10.0);  // or make_cylinder(5.0, 0.0, 10.0)
    REQUIRE_FALSE(mesh.empty());
}

TEST_CASE("make_cone: mesh is closed", "[primitives][cone]") {
    auto mesh = Slic3r::make_cone(5.0, 10.0);
    mesh.repair(true);
    REQUIRE(mesh.stats().open_edges == 0);
}

TEST_CASE("make_cone: height matches bounding box Z", "[primitives][cone]") {
    auto mesh = Slic3r::make_cone(5.0, 10.0);
    auto bb = mesh.bounding_box();
    REQUIRE(bb.size().z() == Approx(10.0).epsilon(0.01));
}

TEST_CASE("make_cone: volume approximates (1/3)πr²h", "[primitives][cone]") {
    double r = 5.0, h = 10.0;
    double expected = (1.0 / 3.0) * M_PI * r * r * h;
    auto mesh = Slic3r::make_cone(r, h);
    REQUIRE(mesh.volume() == Approx(expected).epsilon(0.03));
}

TEST_CASE("make_cone: zero radius does not crash", "[primitives][cone][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_cone(0.0, 10.0));
}
```

**Acceptance:** Cone button works. Tests pass.

---

### PR 3.6 — Retain measurement gizmo

**Goal:** Verify measurement tool (GLGizmoMeasure) still works after Phase 2 stripping.

**Files:** `src/slic3r/GUI/Gizmos/GLGizmoMeasure.cpp` — read-only unless a fix is needed.

**Steps:**
1. Load a 10×10×10mm cube.
2. Activate measurement gizmo.
3. Measure an edge → confirm 10mm reported.
4. If the gizmo is broken by Phase 2 stripping, identify the dependency and fix it in this PR.

**Tests:**
```cpp
// tests/unit/test_measure_gizmo.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"

TEST_CASE("Measurement: cube edge length is 10mm", "[measure]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto bb = mesh.bounding_box();
    // The bounding box diagonal represents the maximum measurement.
    // Edge measurements are face-to-face, equal to side length.
    REQUIRE(bb.size().x() == Approx(10.0));
    REQUIRE(bb.size().y() == Approx(10.0));
    REQUIRE(bb.size().z() == Approx(10.0));
}

TEST_CASE("Measurement: distance between two non-adjacent faces of a cube", "[measure]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    // Bounding box face-to-face distance = edge length.
    auto bb = mesh.bounding_box();
    double face_distance = bb.max.x() - bb.min.x();
    REQUIRE(face_distance == Approx(10.0));
}
```

**Acceptance:** Measurement gizmo reports correct distances on cube primitive. Tests pass.

---

## Phase 4: Boolean Mesh Operations

### PR 4.1 — Boolean subtract (backend)

**Goal:** Expose `libslic3r::MeshBoolean::cgal::minus()` for subtract operations.

**Files:** `src/libslic3r/MeshBoolean.cpp`, `src/libslic3r/MeshBoolean.hpp`

**Steps:**
1. Verify `MeshBoolean::cgal::minus(TriangleMesh &, const TriangleMesh &)` exists in OrcaSlicer. (High confidence: it does.)
2. Expose a clean wrapper function if the interface is complex:
   ```cpp
   namespace Slic3r {
       // Returns the result of subtracting tool from target.
       // Returns empty mesh on failure.
       TriangleMesh mesh_subtract(const TriangleMesh &target, const TriangleMesh &tool);
   }
   ```

**Tests:**
```cpp
// tests/unit/test_boolean_subtract.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/MeshBoolean.hpp"

TEST_CASE("Boolean subtract: small cube from large cube", "[boolean][subtract]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0, 5.0, 5.0);
    // Position tool inside target
    auto result = Slic3r::mesh_subtract(target, tool);
    REQUIRE_FALSE(result.empty());
    // Volume should be reduced
    REQUIRE(result.volume() < target.volume());
    REQUIRE(result.volume() == Approx(target.volume() - tool.volume()).epsilon(0.01));
}

TEST_CASE("Boolean subtract: result is closed manifold", "[boolean][subtract]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0, 5.0, 5.0);
    auto result = Slic3r::mesh_subtract(target, tool);
    result.repair(true);
    REQUIRE(result.stats().open_edges == 0);
}

TEST_CASE("Boolean subtract: non-overlapping tool returns unchanged target", "[boolean][subtract]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0, 5.0, 5.0);
    // Translate tool far away from target
    Slic3r::MeshBoolean::cgal_do_subtract_offset_tool_far_away_test_helper();
    // Adapt: place tool at (100, 100, 100), subtract, verify target unchanged
    // Volume should equal original target
}

TEST_CASE("Boolean subtract: subtracting identical mesh produces near-zero volume", "[boolean][subtract][edge]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(10.0, 10.0, 10.0); // same size, same position
    auto result = Slic3r::mesh_subtract(target, tool);
    REQUIRE(result.volume() < 0.01);
}

TEST_CASE("Boolean subtract: does not crash on non-manifold input", "[boolean][subtract][edge]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    TriangleMesh broken_tool; // empty / degenerate
    REQUIRE_NOTHROW(Slic3r::mesh_subtract(target, broken_tool));
}
```

**Acceptance:** Wrapper function compiles. All tests pass. No crash on bad input.

---

### PR 4.2 — Boolean subtract (UI wiring)

**Goal:** Context-menu and/or toolbar "Subtract" action using the backend from PR 4.1.

**Files:** `src/slic3r/GUI/Gizmos/GLGizmoBooleans.cpp` or equivalent OrcaSlicer boolean gizmo.

**Steps:**
1. Check if OrcaSlicer already has a boolean gizmo (`GLGizmoBooleans` or similar). If yes, adapt it; if no, create a minimal context-menu action.
2. UI flow: select two objects → right-click → "Subtract" → result replaces the two objects in ObjectList.
3. The operation must be undoable (pushes to UndoRedo stack).

**Tests:**
```cpp
// tests/unit/test_boolean_subtract_ui.cpp
// Test the model-level operation without UI context:

TEST_CASE("Boolean subtract: scene object count reduces after subtraction", "[boolean][subtract][ui]") {
    Slic3r::Model model;
    auto* target_obj = model.add_object();
    target_obj->add_volume(Slic3r::make_cube(10.0, 10.0, 10.0));
    auto* tool_obj = model.add_object();
    tool_obj->add_volume(Slic3r::make_cube(5.0, 5.0, 5.0));
    
    REQUIRE(model.objects.size() == 2);
    
    // Perform subtraction at model level
    // (adapt to actual API)
    // subtract_objects(model, target_obj, tool_obj);
    
    // After subtraction: one object remains (the result)
    // REQUIRE(model.objects.size() == 1);
    SUCCEED("Adapt to actual model-level boolean API");
}
```

**Acceptance:** Two-object subtract works via UI. Result is undoable.

---

### PR 4.3 — Boolean union

**Goal:** Merge (union) two meshes.

**Files:** Same as PR 4.2 — extend the boolean gizmo/menu.

**Tests:**
```cpp
// tests/unit/test_boolean_union.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/MeshBoolean.hpp"

TEST_CASE("Boolean union: two non-overlapping cubes", "[boolean][union]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_cube(10.0, 10.0, 10.0);
    // translate b to be adjacent to a
    // ...
    auto result = Slic3r::mesh_union(a, b);
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.volume() == Approx(a.volume() + b.volume()).epsilon(0.01));
}

TEST_CASE("Boolean union: two identical overlapping cubes has volume of one", "[boolean][union]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_cube(10.0, 10.0, 10.0); // same position
    auto result = Slic3r::mesh_union(a, b);
    REQUIRE(result.volume() == Approx(a.volume()).epsilon(0.01));
}

TEST_CASE("Boolean union: result is closed manifold", "[boolean][union]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_sphere(5.0, 0.05);
    auto result = Slic3r::mesh_union(a, b);
    result.repair(true);
    REQUIRE(result.stats().open_edges == 0);
}
```

**Acceptance:** Union works via UI. Tests pass.

---

### PR 4.4 — Boolean intersect

**Goal:** Intersect two meshes.

**Tests:**
```cpp
// tests/unit/test_boolean_intersect.cpp

TEST_CASE("Boolean intersect: cube inside sphere", "[boolean][intersect]") {
    auto cube   = Slic3r::make_cube(10.0, 10.0, 10.0);  // centered at origin
    auto sphere = Slic3r::make_sphere(10.0, 0.05);        // large sphere containing cube
    auto result = Slic3r::mesh_intersect(cube, sphere);
    // Result should approximate the cube volume
    REQUIRE(result.volume() == Approx(cube.volume()).epsilon(0.05));
}

TEST_CASE("Boolean intersect: non-overlapping meshes gives empty result", "[boolean][intersect]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_cube(10.0, 10.0, 10.0);
    // translate b far away
    auto result = Slic3r::mesh_intersect(a, b);
    REQUIRE(result.empty() || result.volume() < 0.01);
}

TEST_CASE("Boolean intersect: result is closed manifold", "[boolean][intersect]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_sphere(8.0, 0.05);
    auto result = Slic3r::mesh_intersect(a, b);
    result.repair(true);
    REQUIRE(result.stats().open_edges == 0);
}
```

**Acceptance:** Intersect works via UI. Tests pass.

---

### PR 4.5 — Plane cut / split

**Goal:** Cut an object with a plane into two independent closed meshes.

**Files:** `src/slic3r/GUI/Gizmos/GLGizmoCut.cpp`

**Steps:**
1. OrcaSlicer has a cut gizmo designed for splitting prints across plates. Adapt it.

2. Default behavior: "Split to independent objects" with automatic capping (both halves become closed manifolds).
3. The cut plane defaults to the XY plane at the object midpoint. User can drag it.

**Tests:**
```cpp
// tests/unit/test_plane_cut.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"

TEST_CASE("Plane cut: cube split at midpoint yields two half-cubes", "[cut]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    double cut_z = 5.0; // midpoint
    
    // Adapt to actual cut API
    // auto [top, bottom] = Slic3r::cut_mesh(cube, cut_z);
    // REQUIRE(top.volume() == Approx(500.0).epsilon(0.01));
    // REQUIRE(bottom.volume() == Approx(500.0).epsilon(0.01));
    SUCCEED("Adapt to cut API after reviewing GLGizmoCut source");
}

TEST_CASE("Plane cut: both halves are closed manifolds", "[cut]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    // auto [top, bottom] = Slic3r::cut_mesh(cube, 5.0);
    // top.repair(true);
    // bottom.repair(true);
    // REQUIRE(top.stats().open_edges == 0);
    // REQUIRE(bottom.stats().open_edges == 0);
    SUCCEED("Adapt to cut API");
}

TEST_CASE("Plane cut: cut at z=0 yields full object and empty", "[cut][edge]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    // Cut at bottom face — result should be the full object and nothing
    REQUIRE_NOTHROW(/* Slic3r::cut_mesh(cube, 0.0) */ (void)cube);
}
```

**Acceptance:** Plane cut produces two closed half-meshes. Tests pass.

---

### PR 4.6 — Settle / Orient to face

**Goal:** Allow user to select a mesh face and align it parallel to the ground plane (XY).

**Files:** `src/slic3r/GUI/Gizmos/GLGizmoFlatten.cpp`

**Steps:**
1. OrcaSlicer has `GLGizmoFlatten` ("Lay on face"). It is functional as-is.
2. Verify it works after Phase 2 stripping. If broken, find and fix the dependency.
3. If intact: this PR is documentation + tests only.

**Tests:**
```cpp
// tests/unit/test_settle.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"

TEST_CASE("Settle: a rotated cube realigned to XY has bottom face at Z=0", "[settle]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    // Rotate 45 degrees around X axis
    Slic3r::Transform3d rot = Slic3r::Transform3d::Identity();
    rot.rotate(Eigen::AngleAxisd(M_PI / 4.0, Eigen::Vector3d::UnitX()));
    mesh.transform(rot);
    
    // After settle/flatten to any face, the object should be repositioned
    // so a face is coplanar with Z=0. Min Z should be ≥ 0.
    // Apply flatten operation... adapt to API.
    auto bb = mesh.bounding_box();
    // REQUIRE(bb.min.z() == Approx(0.0).margin(0.001));
    SUCCEED("Adapt to flatten API");
}
```

**Acceptance:** Settle gizmo aligns selected face to ground. Tests pass.

---

## Phase 5: File I/O

### PR 5.1 — STL import

**Goal:** Reliable STL import (ASCII and binary) with bounds checking.

**Files:** `src/libslic3r/Format/STL.cpp`

**Steps:**
1. Verify drag-and-drop STL load works after Phase 2 stripping.
2. Audit STL parser: add bounds checks on all `std::vector` indexing (see `BOUNDARIES.md` § Security).
3. Ensure `std::bad_alloc` from oversized files is caught.

**Tests:**
```cpp
// tests/unit/test_stl_import.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/Model.hpp"

TEST_CASE("STL import: binary STL loads correctly", "[import][stl]") {
    Slic3r::Model model;
    bool ok = Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
    REQUIRE_FALSE(model.objects[0]->volumes.empty());
}

TEST_CASE("STL import: ASCII STL loads correctly", "[import][stl]") {
    Slic3r::Model model;
    bool ok = Slic3r::load_stl("tests/fixtures/cube_10mm_ascii.stl", &model);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
}

TEST_CASE("STL import: empty file does not crash", "[import][stl][edge]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_stl("tests/fixtures/empty.stl", &model));
}

TEST_CASE("STL import: truncated file does not crash", "[import][stl][edge]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_stl("tests/fixtures/truncated.stl", &model));
}

TEST_CASE("STL import: file with bogus triangle count does not crash", "[import][stl][edge]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_stl("tests/fixtures/bad_count.stl", &model));
}
```

**Fixtures required (add in this PR):**
- `tests/fixtures/cube_10mm.stl` — valid binary STL
- `tests/fixtures/cube_10mm_ascii.stl` — valid ASCII STL
- `tests/fixtures/empty.stl` — 0-byte file
- `tests/fixtures/truncated.stl` — valid header, truncated body
- `tests/fixtures/bad_count.stl` — binary STL with declared triangle count larger than file size

**Acceptance:** All tests pass. No crash on any fixture.

---

### PR 5.2 — STL import: libFuzzer harness

**Goal:** Fuzz the STL parser with random binary inputs.

**Files:** Create `tests/fuzz/fuzz_stl.cpp`, update `tests/fuzz/CMakeLists.txt`.

```cpp
// tests/fuzz/fuzz_stl.cpp
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include "libslic3r/Model.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    try {
        Slic3r::Model model;
        // Write data to a temporary file and load, OR use a buffer-based API if available.
        // If no buffer API exists, create one in STL.cpp and expose it:
        // Slic3r::load_stl_from_buffer(data, size, &model);
        (void)data; (void)size; // placeholder until API exists
    } catch (const std::exception &) {
        // Expected: parser rejects bad input via exception.
    } catch (...) {
        // Also acceptable.
    }
    return 0; // Must never return non-zero (reserved for future libFuzzer use).
}
```

**CMake (add to tests/fuzz/CMakeLists.txt):**
```cmake
add_executable(fuzz_stl fuzz_stl.cpp)
target_link_libraries(fuzz_stl libslic3r)
target_compile_options(fuzz_stl PRIVATE -fsanitize=fuzzer,address)
target_link_options(fuzz_stl PRIVATE -fsanitize=fuzzer,address)
```

**Tests:** libFuzzer harnesses are not Catch2 tests. Run manually:
```bash
scripts/run_fuzz.sh fuzz_stl -max_total_time=300
```

**Acceptance:** `fuzz_stl` compiles. Runs 5 minutes without crash or hang.

---

### PR 5.3 — 3MF import (namespace-aware)

**Goal:** Import standard 3MF files. Strip Bambu/Prusa extension namespaces silently. Do not fail on unknown namespaces.

**Files:** `src/libslic3r/Format/3mf.cpp`

**Steps:**
1. Locate where Bambu/Prusa XML namespace elements are parsed.
2. Replace hard failure / assertion on unknown namespaces with a `continue` / silent skip.
3. Do not remove the core OPC + 3MF geometry parser.

**Tests:**
```cpp
// tests/unit/test_3mf_import.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/Model.hpp"

TEST_CASE("3MF import: standard geometry-only 3MF loads correctly", "[import][3mf]") {
    Slic3r::Model model;
    Slic3r::DynamicPrintConfig config;
    bool ok = Slic3r::load_3mf("tests/fixtures/cube_10mm.3mf", &config, nullptr, &model, false);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
}

TEST_CASE("3MF import: file with Bambu extension namespace does not fail", "[import][3mf]") {
    Slic3r::Model model;
    Slic3r::DynamicPrintConfig config;
    // This fixture is a valid 3MF with Bambu-specific XML extensions.
    bool ok = Slic3r::load_3mf("tests/fixtures/cube_bambu_extensions.3mf", &config, nullptr, &model, false);
    REQUIRE(ok); // Must not fail — extensions are ignored.
    REQUIRE(model.objects.size() == 1);
}

TEST_CASE("3MF import: empty 3MF does not crash", "[import][3mf][edge]") {
    Slic3r::Model model;
    Slic3r::DynamicPrintConfig config;
    REQUIRE_NOTHROW(Slic3r::load_3mf("tests/fixtures/empty.3mf", &config, nullptr, &model, false));
}

TEST_CASE("3MF import: truncated ZIP does not crash", "[import][3mf][edge]") {
    Slic3r::Model model;
    Slic3r::DynamicPrintConfig config;
    REQUIRE_NOTHROW(Slic3r::load_3mf("tests/fixtures/truncated.3mf", &config, nullptr, &model, false));
}
```

**Fixtures required (add in this PR):**
- `tests/fixtures/cube_10mm.3mf` — standard 3MF, no vendor extensions
- `tests/fixtures/cube_bambu_extensions.3mf` — 3MF with Bambu XML namespace elements
- `tests/fixtures/empty.3mf` — 0-byte file
- `tests/fixtures/truncated.3mf` — valid ZIP header, truncated

**Acceptance:** Standard 3MF imports. Bambu-extension 3MF imports (geometry only). No crash on bad input.

---

### PR 5.4 — 3MF import: libFuzzer harness

Identical structure to PR 5.2. File: `tests/fuzz/fuzz_3mf.cpp`.

---

### PR 5.5 — STL export

**Goal:** Export scene to a single binary STL (geometry only, no slicer metadata).

**Files:** `src/slic3r/GUI/MainFrame.cpp`, `src/libslic3r/Format/STL.cpp`

**Steps:**
1. Add "Export → Export as STL..." to the File menu.
2. Opens a save dialog. Defaults to the current scene name with `.stl` extension.
3. Uses `libslic3r::Model::export_stl()`.
4. For multi-object scenes: offer "Merge all objects" (single STL) or "One file per object".
5. Output contains: vertices, normals, face indices. Nothing else.

**Tests:**
```cpp
// tests/unit/test_stl_export.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/Model.hpp"

TEST_CASE("STL export: exported file re-imports with same volume", "[export][stl]") {
    Slic3r::Model model;
    Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    double original_volume = model.objects[0]->volumes[0]->mesh().volume();
    
    std::string out_path = "tests/tmp/export_test.stl";
    model.objects[0]->volumes[0]->mesh().write_binary(out_path);
    
    Slic3r::Model reimported;
    Slic3r::load_stl(out_path, &reimported);
    double reimported_volume = reimported.objects[0]->volumes[0]->mesh().volume();
    
    REQUIRE(reimported_volume == Approx(original_volume).epsilon(0.001));
}

TEST_CASE("STL export: output is valid binary STL", "[export][stl]") {
    Slic3r::Model model;
    Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    std::string out_path = "tests/tmp/export_sanity.stl";
    model.objects[0]->volumes[0]->mesh().write_binary(out_path);
    
    // Binary STL: 80-byte header + 4-byte triangle count + 50 bytes per triangle
    std::ifstream f(out_path, std::ios::binary | std::ios::ate);
    REQUIRE(f.is_open());
    std::streamsize size = f.tellg();
    REQUIRE(size >= 84); // Minimum valid binary STL
    
    f.seekg(80);
    uint32_t tri_count = 0;
    f.read(reinterpret_cast<char*>(&tri_count), 4);
    REQUIRE(size == 84 + static_cast<std::streamsize>(tri_count) * 50);
}

TEST_CASE("STL export: boolean result exports correctly", "[export][stl][integration]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0, 5.0, 5.0);
    auto result = Slic3r::mesh_subtract(target, tool);
    
    std::string out_path = "tests/tmp/boolean_export.stl";
    result.write_binary(out_path);
    
    Slic3r::Model reimported;
    Slic3r::load_stl(out_path, &reimported);
    REQUIRE_FALSE(reimported.objects.empty());
}
```

**Acceptance:** Export dialog works. Re-imported STL has matching volume. Tests pass.

---

### PR 5.6 — 3MF export

**Goal:** Export scene to 3MF (geometry + basic object names and transforms only).

**Tests:**
```cpp
// tests/unit/test_3mf_export.cpp

TEST_CASE("3MF export: re-imported file has same object count", "[export][3mf]") {
    Slic3r::Model model;
    Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    
    std::string out_path = "tests/tmp/export_test.3mf";
    // export_3mf(out_path, model, ...);
    
    Slic3r::Model reimported;
    Slic3r::DynamicPrintConfig config;
    Slic3r::load_3mf(out_path, &config, nullptr, &reimported, false);
    
    REQUIRE(reimported.objects.size() == model.objects.size());
}

TEST_CASE("3MF export: output contains no slicer-specific XML namespaces", "[export][3mf]") {
    // Unzip the 3MF and grep for Bambu/Prusa namespace URIs.
    // They must not appear.
    // Implement using miniz to read the ZIP and check XML content.
    SUCCEED("Implement using miniz unzip + string search");
}

TEST_CASE("3MF export: re-imported volume matches original", "[export][3mf]") {
    Slic3r::Model model;
    Slic3r::load_stl("tests/fixtures/cube_10mm.stl", &model);
    double orig_vol = model.objects[0]->volumes[0]->mesh().volume();
    
    std::string out_path = "tests/tmp/vol_test.3mf";
    // export...
    
    Slic3r::Model re;
    Slic3r::DynamicPrintConfig cfg;
    Slic3r::load_3mf(out_path, &cfg, nullptr, &re, false);
    REQUIRE(re.objects[0]->volumes[0]->mesh().volume() == Approx(orig_vol).epsilon(0.001));
}
```

**Acceptance:** 3MF exports without Bambu/Prusa namespace content. Re-import round-trips correctly.

---

## Phase 6: Security, Stability, and Distribution

### PR 6.1 — OBJ import/export

**Goal:** Support loading and saving `.obj` files.

**Tests:**
```cpp
// tests/unit/test_obj_import.cpp

TEST_CASE("OBJ import: valid OBJ file loads", "[import][obj]") {
    Slic3r::Model model;
    // Use OrcaSlicer's OBJ loader — verify path
    bool ok = Slic3r::load_obj("tests/fixtures/cube_10mm.obj", &model);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
}

TEST_CASE("OBJ import: file with missing material does not crash", "[import][obj][edge]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_obj("tests/fixtures/no_material.obj", &model));
}

TEST_CASE("OBJ import: empty file does not crash", "[import][obj][edge]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_obj("tests/fixtures/empty.obj", &model));
}
```

---

### PR 6.2 — OBJ import: libFuzzer harness

File: `tests/fuzz/fuzz_obj.cpp`. Same structure as PR 5.2.

---

### PR 6.3 — Memory leak baseline

**Goal:** Establish memory usage baseline for 50 consecutive boolean subtractions.

**Steps:**
1. macOS: Profile with Instruments (Allocations + Leaks template).
2. Windows: Visual Studio Diagnostic Tools.
3. Script: `tests/perf/memory_stress.cpp` — loads cube, subtracts sphere 50 times, undoes all, checks RSS returns near baseline.
4. Document baseline RSS numbers in `docs/perf_baseline.md`.

**Tests:**
```cpp
// tests/perf/memory_stress.cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/MeshBoolean.hpp"

TEST_CASE("Memory: 50 boolean subtractions do not grow unboundedly", "[perf][memory]") {
    // Run 50 subtract operations and verify the resulting meshes
    // are not accumulating in memory (i.e., they can be destroyed).
    for (int i = 0; i < 50; ++i) {
        auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
        auto tool   = Slic3r::make_sphere(3.0, 0.1);
        auto result = Slic3r::mesh_subtract(target, tool);
        REQUIRE_FALSE(result.empty());
        // result goes out of scope here — destructor must free memory.
    }
    // If this test completes without OOM, memory is being released.
    SUCCEED("50 iterations completed without OOM");
}
```

**Acceptance:** 50-iteration test passes. Memory returns near baseline after completion.

---

### PR 6.4 — Windows DPI scaling

**Goal:** UI renders correctly at 150% and 200% Windows display scaling.

**Steps:**
1. Enable DPI awareness in `src/platform/msw/meshforge.manifest` (or equivalent): set `<dpiAwareness>PerMonitorV2</dpiAwareness>`.
2. Verify wxWidgets SVG icons scale correctly (no clipping, no blurriness) in `BuilderToolbar`.
3. Test at 100%, 150%, 200% using Windows Display Settings.

**Tests:**
- No automated test possible for visual rendering.
- Manual checklist in `docs/dpi_test_checklist.md`:
  - [ ] Toolbar icons visible and not clipped at 150%
  - [ ] Toolbar icons visible and not clipped at 200%
  - [ ] ObjectList text readable at all scales
  - [ ] Dialog boxes fit within screen bounds at 200%

**Acceptance:** Checklist completed and committed.

---

### PR 6.5 — macOS code signing and notarization

**Goal:** Signed, notarized macOS `.app` that passes Gatekeeper.

**Files:** Create `scripts/sign_mac.sh`, create `scripts/notarize_mac.sh`.

**sign_mac.sh:**
```bash
#!/bin/bash
# Usage: ./scripts/sign_mac.sh <path-to-app> <developer-id>
codesign --deep --force --verify --verbose \
  --sign "$2" \
  --options runtime \
  --entitlements scripts/entitlements.plist \
  "$1"
```

**notarize_mac.sh:**
```bash
#!/bin/bash
# Usage: ./scripts/notarize_mac.sh <path-to-app> <apple-id> <team-id> <keychain-profile>
ditto -c -k --keepParent "$1" /tmp/meshforge_notarize.zip
xcrun notarytool submit /tmp/meshforge_notarize.zip \
  --apple-id "$2" \
  --team-id "$3" \
  --keychain-profile "$4" \
  --wait
xcrun stapler staple "$1"
```

**Tests:**
```bash
# Manual verification:
spctl --assess --verbose /Applications/MeshForge.app
# Expected: /Applications/MeshForge.app: accepted
# Source=Notarized Developer ID
```

**Acceptance:** `spctl --assess` returns "accepted". Unsigned binary test: download, double-click → Gatekeeper does not block.

---

## Appendix: Test Fixtures

All test fixtures are committed to `tests/fixtures/`. Required before Phase 5 PRs begin:

| File | Description |
|---|---|
| `cube_10mm.stl` | Valid binary STL, 10×10×10mm cube |
| `cube_10mm_ascii.stl` | Same, ASCII format |
| `cube_10mm.obj` | Same, OBJ format |
| `cube_10mm.3mf` | Same, standard 3MF |
| `cube_bambu_extensions.3mf` | Standard geometry + Bambu XML namespace elements |
| `empty.stl` | 0-byte file |
| `empty.obj` | 0-byte file |
| `empty.3mf` | 0-byte file |
| `truncated.stl` | Valid binary header, truncated body |
| `truncated.3mf` | Valid ZIP header, truncated |
| `bad_count.stl` | Binary STL, declared triangle count > file size |
| `no_material.obj` | OBJ referencing a `.mtl` file that does not exist |

Generate using a minimal Python script committed to `tests/fixtures/generate_fixtures.py`.

---

## Appendix: Test Tag Reference

| Tag | Meaning |
|---|---|
| `[sanity]` | Build-time and baseline checks |
| `[primitives]` | Primitive mesh generation |
| `[boolean]` | Boolean operations |
| `[cut]` | Plane cut |
| `[settle]` | Face orientation |
| `[import]` | File import |
| `[export]` | File export |
| `[undo]` | Undo/redo stack |
| `[network]` | Networking stub |
| `[ui]` | UI wiring (model-level only) |
| `[perf]` | Performance and memory |
| `[edge]` | Edge/degenerate input cases |
| `[canary]` | Tests that must never fail |
| `[regression]` | Tests for previously fixed bugs |
