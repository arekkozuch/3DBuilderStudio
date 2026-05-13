// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 6.2: libFuzzer harness for the OBJ parser.
// Build with: -fsanitize=fuzzer,address
// Run:  scripts/run_fuzz.sh fuzz_obj -max_total_time=300

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "libslic3r/Model.hpp"
#include "libslic3r/Format/OBJ.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    char path[] = "/tmp/fuzz_obj_XXXXXX.obj";
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
        Slic3r::ObjInfo info;
        std::string msg;
        Slic3r::load_obj(path, &model, info, msg);
    } catch (const std::bad_alloc &) {
    } catch (const std::exception &) {
    } catch (...) {
    }

    unlink(path);
    return 0;
}
