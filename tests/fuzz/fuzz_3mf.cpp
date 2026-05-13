// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 5.4: libFuzzer harness for the 3MF parser.
// Build with: -fsanitize=fuzzer,address
// Run:  scripts/run_fuzz.sh fuzz_3mf -max_total_time=300

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "libslic3r/Model.hpp"
#include "libslic3r/Format/3mf.hpp"
#include "libslic3r/Config.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    char path[] = "/tmp/fuzz_3mf_XXXXXX.3mf";
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
        Slic3r::DynamicPrintConfig config;
        Slic3r::ConfigSubstitutionContext subs{
            Slic3r::ForwardCompatibilitySubstitutionRule::Enable };
        Slic3r::load_3mf(path, config, subs, &model, false);
    } catch (const std::bad_alloc &) {
    } catch (const std::exception &) {
    } catch (...) {
    }

    unlink(path);
    return 0;
}
