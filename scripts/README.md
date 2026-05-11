# scripts/

Build, verify, and release scripts for MeshForge.

---

## setup_deps_mac.sh — macOS dependency build

Builds all vendored C++ dependencies from `deps/` on macOS.

### Prerequisites

| Requirement | Minimum version | Notes |
|---|---|---|
| macOS | 11.3 (Big Sur) | Deployment target matches OrcaSlicer upstream |
| Xcode | 13.0 | Full Xcode installation required (not just Command Line Tools) |
| Homebrew | Any recent | https://brew.sh |
| CMake | 3.21 | Installed by the script via Homebrew if absent |
| Ninja | Any | Installed by the script |

### Usage

```bash
# Build for the current machine's architecture (default):
./scripts/setup_deps_mac.sh

# Build for a specific architecture:
./scripts/setup_deps_mac.sh arm64
./scripts/setup_deps_mac.sh x86_64
```

The deps build system builds one architecture at a time. Universal binaries
require two separate builds and `lipo` — this is intentional (see `deps/CMakeLists.txt`).

### Expected build time and disk usage

| Condition | Time | Disk |
|---|---|---|
| First run, no ccache | 60–90 min | ~10 GB |
| Warm ccache | ~5 min | ~10 GB (cache adds ~2 GB in `~/.ccache`) |

### Output

Artifacts are placed in `deps/build_<arch>/destdir/usr/local/`.
Pass this path to the main CMake configure step as `-DDEPS_DIR=`.

---

## setup_deps_win.bat — Windows dependency build

See [setup_deps_win.bat](setup_deps_win.bat) and the Windows section below.

### Prerequisites

| Requirement | Notes |
|---|---|
| Visual Studio 2022 Community | Desktop development with C++ workload + Windows 10 SDK 10.0.22621 |
| Strawberry Perl | Required for OpenSSL build; add to PATH |
| Git for Windows | |
| CMake ≥ 3.21 | Must be on PATH |

### Usage

```bat
scripts\setup_deps_win.bat
```

### Expected build time and disk usage

| Condition | Time | Disk |
|---|---|---|
| First run | 90–120 min | ~15 GB |

---

## verify.sh — Local CI gate

Runs all checks required before opening a PR. See `CONTRIBUTING.md` for the
full specification. Must print `ALL CHECKS PASSED` on exit 0.

```bash
./scripts/verify.sh
```

---

## run_fuzz.sh — Fuzz target runner

Runs libFuzzer harnesses from `tests/fuzz/`. Not part of the PR gate (too slow).

```bash
# Run the STL fuzzer for 5 minutes:
./scripts/run_fuzz.sh fuzz_stl -max_total_time=300
```

---

## sign_mac.sh / notarize_mac.sh — macOS code signing (Phase 6)

See Phase 6 of `DEVELOPMENT_PLAN.md`. These scripts require an Apple Developer ID
certificate and are run as part of the release process, not the build process.
