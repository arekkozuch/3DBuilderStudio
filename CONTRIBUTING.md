# CONTRIBUTING.md

---

## Branch Model

| Branch | Purpose |
|---|---|
| `main` | Stable, always compiles, all tests pass |
| `feature/<short-name>` | One feature per branch, one PR per branch |
| `fix/<short-name>` | Bug fix branches |
| `chore/<short-name>` | Non-functional changes (branding, docs, scripts) |

Branch off `main`. PR targets `main`. No long-lived feature branches.

---

## One Feature Per PR — Definition

A PR must do exactly one of the following:
- Add one new capability (a function, a UI element, a file format, a test suite).
- Remove one subsystem or UI element.
- Fix one bug.
- Update one configuration or build concern.

If completing a task requires multiple independent capabilities, open multiple PRs in dependency order. Mark them in the PR description: `Depends on #<number>`.

There is no line-count limit. The constraint is logical scope, not size.

---

## PR Checklist

Every PR description must include the following, filled in:

```
## Feature / Change
<!-- One sentence. -->

## Files Changed
<!-- List changed files and what was changed in each. -->

## Tests Added or Modified
<!-- List test files. For each: what it tests, what the pass condition is. -->

## Brand Scrub Verified
<!-- Paste output of: -->
<!-- grep -ri "bambu\|orca\|softfever\|prusaslicer" \ -->
<!--   --include="*.cpp" --include="*.h" --include="*.mm" \ -->
<!--   --exclude-dir=deps src/ | grep -iv "copyright\|license\|based on\|adapted from\|originally" -->
<!-- Must be empty. -->

## verify.sh Output
<!-- Paste last 20 lines of scripts/verify.sh output. Must end: "ALL CHECKS PASSED". -->

## Notes for Reviewer
<!-- Anything not obvious from code. -->
```

---

## verify.sh — Specification

`scripts/verify.sh` is the local CI gate. Every PR must pass it before opening.  
It runs in order; stops on first failure.

### Steps (in order)

1. **Brand scrub check**
   ```bash
   grep -ri "bambu\|orca\|softfever\|prusaslicer" \
     --include="*.cpp" --include="*.h" --include="*.mm" \
     --exclude-dir=deps src/ \
     | grep -iv "copyright\|license\|based on\|adapted from\|originally"
   ```
   Pass: empty output.

2. **CMake configure** (both Release and Debug)
   ```bash
   cmake -B build_verify -G Ninja \
     -DCMAKE_BUILD_TYPE=Debug \
     -DSLIC3R_NETWORK=OFF \
     -DCMAKE_OSX_ARCHITECTURES=arm64   # adjust per host
   ```

3. **Full build**
   ```bash
   ninja -C build_verify -j$(sysctl -n hw.ncpu)
   ```

4. **Catch2 unit tests**
   ```bash
   ctest --test-dir build_verify -L unit --output-on-failure
   ```
   Pass: 0 failures.

5. **Undo/redo canary** (separate explicit step)
   ```bash
   ctest --test-dir build_verify -R test_undo_redo_baseline --output-on-failure
   ```
   Pass: must pass even if other tests fail.

6. **Summary line**
   ```
   ALL CHECKS PASSED
   ```
   or
   ```
   FAILED: <step name>
   ```

### What verify.sh does NOT run
- libFuzzer harnesses (use `scripts/run_fuzz.sh` explicitly, too slow for PR gate).
- Windows-specific steps (run separately on Windows; macOS-first development).

---

## Test File Conventions

### Catch2 unit tests (`tests/unit/`)

Filename: `test_<subsystem>_<feature>.cpp`

```cpp
#include <catch2/catch_test_macros.hpp>
#include "libslic3r/TriangleMesh.hpp"   // adjust include per subsystem

TEST_CASE("Sphere has expected vertex count", "[primitives][sphere]") {
    auto mesh = Slic3r::make_sphere(1.0, 0.05);   // radius, detail
    REQUIRE(mesh.its.vertices.size() > 0);
    REQUIRE(mesh.its.indices.size() > 0);
    // Euler characteristic for a manifold sphere: V - E + F = 2
    // Not checking full Euler here; check that mesh is non-empty and reports closed.
    REQUIRE_FALSE(mesh.empty());
}

TEST_CASE("Sphere with zero radius is degenerate — does not crash", "[primitives][sphere][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_sphere(0.0, 0.05));
}
```

Each test file must have:
- At least one happy-path test.
- At least one edge/degenerate-input test.
- Catch2 tags in square brackets: `[subsystem][feature]` and optionally `[edge]`, `[regression]`.

### libFuzzer harnesses (`tests/fuzz/`)

Filename: `fuzz_<parser>.cpp`

```cpp
#include <cstdint>
#include <cstddef>
#include "libslic3r/Format/STL.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Write data to a temp file or wrap in a stream, call the parser.
    // Must not crash. Exceptions are acceptable.
    try {
        Slic3r::Model model;
        Slic3r::load_stl_from_buffer(data, size, &model);   // hypothetical API
    } catch (...) {
        // Handled: parser threw on bad input. Correct behavior.
    }
    return 0;
}
```

Fuzz targets are compiled with:
```cmake
target_compile_options(fuzz_stl PRIVATE -fsanitize=fuzzer,address)
target_link_options(fuzz_stl PRIVATE -fsanitize=fuzzer,address)
```

Separate CMake target, not linked into the main binary.

---

## Code Style

Follow the existing OrcaSlicer/libslic3r style:
- Indent: 4 spaces.
- Braces: K&R for functions, Allman for classes (match surrounding code).
- Naming: `snake_case` for variables and functions, `PascalCase` for types.
- Minimize comments. Code should be self-explanatory. Comment only non-obvious decisions.
- No trailing whitespace.

Do not run clang-format on files you didn't touch. Do not reformat large existing blocks.

---

## What Agents Must Not Do

See `BOUNDARIES.md` for the full list. Summary:
- Do not add cloud CI.
- Do not delete networking code (stub it).
- Do not add new external libraries without discussion.
- Do not open PRs without passing `scripts/verify.sh`.
- Do not leave upstream brand strings in user-visible code.