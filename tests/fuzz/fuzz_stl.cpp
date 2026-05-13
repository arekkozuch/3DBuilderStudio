// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 5.2: libFuzzer harness for the STL parser.
// Build with: -fsanitize=fuzzer,address
// Run:  scripts/run_fuzz.sh fuzz_stl -max_total_time=300

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Write to a temp file — admesh requires a path-based interface.
    char path[] = "/tmp/fuzz_stl_XXXXXX.stl";
    int fd = mkstemps(path, 4);
    if (fd < 0)
        return 0;

    if (write(fd, data, size) < 0) {
        close(fd);
        unlink(path);
        return 0;
    }
    close(fd);

    try {
        Slic3r::Model model;
        Slic3r::load_stl(path, &model);
    } catch (const std::bad_alloc &) {
        // Acceptable: oversized input exhausted memory.
    } catch (const std::exception &) {
        // Acceptable: parser rejected bad input.
    } catch (...) {
        // Also acceptable.
    }

    unlink(path);
    return 0;
}
