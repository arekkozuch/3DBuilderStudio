# BOUNDARIES.md — Mandatory Rules for AI Coding Agents

These rules apply to every agent session, every PR, without exception.  
Violating any rule below is grounds for automatic PR rejection.

---

## 1. PR Scope: One Feature Per PR

- One PR = one logical feature or one logical removal.
- Do not bundle multiple features, multiple removals, or a feature + a removal in a single PR.
- If a task requires scaffolding (e.g., a new class + a UI button that calls it), that is one feature. If it requires two separate subsystems to both be independently useful, split it.
- There is no arbitrary line-count limit. A PR can be 20 lines or 800 lines, as long as it does exactly one thing.

**Examples of valid single-PR scope:**
- Add `make_sphere()` to `TriangleMesh.cpp` + Catch2 tests for it.
- Remove the "Device" tab from `MainFrame.cpp`.
- Stub `NetworkAgent` with `SLIC3R_NETWORK=OFF` build flag.

**Examples of invalid scope (must split):**
- Add sphere primitive AND add cylinder primitive. → Two PRs.
- Remove Device tab AND remove Calibration tab. → Two PRs.

---

## 2. Tests Are Mandatory

Every PR that adds or modifies logic must include tests.

| Code type | Required test |
|---|---|
| New geometry function (primitive, boolean, cut) | Catch2 unit test verifying output mesh properties |
| Modified geometry function | Updated or new Catch2 test covering the change |
| UI wiring (button → backend call) | Catch2 test on the backend function; UI wiring is not tested directly |
| Parser change (STL, OBJ, 3MF) | Catch2 test with known-good file + libFuzzer harness update |
| Removal-only PR | No test required unless removal changes observable behavior |

Test files live in `tests/unit/` (Catch2) and `tests/fuzz/` (libFuzzer).  
Naming: `test_<subsystem>_<feature>.cpp`. Example: `test_mesh_sphere.cpp`.

**Minimum test coverage per geometry feature:**
- Happy path: valid inputs produce expected output.
- Edge case: degenerate input (zero-size, coincident vertices).
- Postcondition: result is a valid closed manifold (use `admesh` validator or equivalent).

---

## 3. No New External Libraries

Before adding any new dependency:
1. Search `src/libslic3r/` — specifically `Geometry/`, `TriangleMesh.*`, `MeshBoolean.*`.
2. Search `deps/` for already-vendored libraries.
3. If the math or algorithm is not present, open a discussion issue before writing code.

Rationale: the full CGAL + OpenVDB + admesh + Eigen stack is already available. 99% of mesh operations are already implemented.

---

## 4. Networking: Stub, Never Delete

Do not delete networking code. Guard it with the `SLIC3R_NETWORK` CMake flag.

```cpp
#ifdef SLIC3R_NETWORK
    // original networking code
#else
    // stub returning failure / no-op
#endif
```

`SLIC3R_NETWORK` defaults to `OFF` for this fork.  
Rationale: deletion creates dangling symbols across the codebase. Stubs compile cleanly and can be re-enabled.

---

## 5. No Cloud CI/CD

Do not create or modify:
- `.github/workflows/`
- `.circleci/`
- `azure-pipelines.yml`
- Any file that triggers remote build/test/deploy infrastructure.

All build and test validation is local via `scripts/verify.sh`.

---

## 6. Brand and Identifier Scrub

The following strings must not appear in any user-visible context (UI labels, window titles, about dialogs, file associations, registry keys, bundle identifiers, binary names, installer text):

- `Orca`, `OrcaSlicer`, `Orca Slicer`
- `Bambu`, `BambuStudio`, `Bambu Studio`, `Bambu Lab`
- `SoftFever`
- `PrusaSlicer`, `Prusa`

**Allowed:** AGPL-required copyright comments in source files. Example:
```cpp
// Based on PrusaSlicer by Prusa Research (AGPL-3.0)
```
These are legally required. Do not remove them.

**Verification command** (run before every PR):
```bash
grep -riP "\b(bambu|orca|softfever|prusaslicer)\b" \
  --include="*.cpp" --include="*.h" --include="*.mm" \
  --exclude-dir=deps src/ \
  | grep -iv "copyright\|license\|based on\|adapted from\|originally"
```
Result must be empty.

---

## 7. Asset Policy

Do not include in any PR:
- Orca Slicer, BambuStudio, or Prusa Research logos, icons, or splash screens.
- Any binary asset (`.png`, `.svg`, `.ico`, `.icns`) copied from upstream without verifying it is AGPL-licensed or public domain.

Replacement assets must be original or sourced from CC0 / public domain repositories (e.g., Material Icons, Feather Icons under MIT).  
Document the source of every new asset in `LICENSING.md`.

---

## 8. 3MF Parser: Namespace-Aware Stripping Only

When modifying 3MF import (`src/libslic3r/Format/3mf.cpp`):
- **Do not remove the OPC/3MF core parser.**
- Slicer-specific extensions live in distinct XML namespaces (e.g., `http://schemas.bambulab.com/...`, `http://schemas.prusa3d.com/...`). Ignore unknown namespaces; do not fail on them.
- Required behavior: a valid ISO/ASTM 3MF file must import correctly regardless of which slicer created it.

---

## 9. Undo/Redo Canary

`tests/unit/test_undo_redo_baseline.cpp` must pass at every phase.  
If a PR causes this test to fail, it must not be merged regardless of other functionality.

---

## 10. macOS Code Signing

Every macOS binary delivered to users must be:
1. Signed with a Developer ID Application certificate.
2. Notarized via `xcrun notarytool`.
3. Stapled: `xcrun stapler staple`.

Do not ship unsigned macOS binaries. Gatekeeper will quarantine them.  
The signing step is documented in `scripts/sign_mac.sh` (Phase 6 deliverable).

---

## 11. Security Rules for Parsers

Any change to STL, OBJ, or 3MF parsers must:
- Bounds-check all `std::vector` indexing before access.
- Not assume the input geometry is manifold.
- Handle `std::bad_alloc` from large/malicious files without crashing.
- Have a corresponding libFuzzer harness in `tests/fuzz/`.

libFuzzer harnesses use `LLVMFuzzerTestOneInput` as their entry point.  
They are compiled as separate targets with `-fsanitize=fuzzer,address`.  
They are **not** Catch2 tests and are not run by `scripts/verify.sh` by default (too slow). Run them explicitly with `scripts/run_fuzz.sh`.

---

## 12. Commit Message Format

```
<phase>(<scope>): <imperative description>

Optional body. What changed and why, not how.
```

Examples:
```
feat(primitives): add make_sphere() to TriangleMesh
fix(viewport): disable Z-snap constraint for free translation
test(booleans): add Catch2 test for cube subtraction
chore(branding): scrub Orca/Bambu user-visible strings
```