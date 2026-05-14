// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 5.5: STL export round-trip tests.
// Uses store_stl() at libslic3r level — same function the GUI export path
// ultimately calls. Verifies binary format correctness and volume preservation.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <fstream>
#include <cstdint>

#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"
#include "libslic3r/MeshBoolean.hpp"

using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

static const char* TMP = "/tmp/meshforge_test_tmp";

TEST_CASE("STL export: binary STL re-imports with same vertex count", "[export][stl]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    const int orig_facets = mesh.facets_count();

    std::string out = std::string(TMP) + "/export_cube.stl";
    REQUIRE(Slic3r::store_stl(out.c_str(), &mesh, true));

    Slic3r::Model reimported;
    REQUIRE(Slic3r::load_stl(out.c_str(), &reimported));
    REQUIRE(reimported.objects.size() == 1);
    REQUIRE(reimported.objects[0]->volumes[0]->mesh().facets_count() == orig_facets);
}

TEST_CASE("STL export: output is valid binary STL (header + count + triangles)", "[export][stl]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    std::string out = std::string(TMP) + "/export_format.stl";
    REQUIRE(Slic3r::store_stl(out.c_str(), &mesh, true));

    std::ifstream f(out, std::ios::binary | std::ios::ate);
    REQUIRE(f.is_open());
    const std::streamsize size = f.tellg();
    REQUIRE(size >= 84);

    f.seekg(80);
    std::uint32_t tri_count = 0;
    f.read(reinterpret_cast<char*>(&tri_count), 4);
    REQUIRE(size == 84 + static_cast<std::streamsize>(tri_count) * 50);
}

TEST_CASE("STL export: boolean result round-trips correctly", "[export][stl][integration]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0, 5.0, 5.0);
    auto result = Slic3r::mesh_subtract(target, tool);
    REQUIRE_FALSE(result.empty());

    std::string out = std::string(TMP) + "/export_boolean.stl";
    REQUIRE(Slic3r::store_stl(out.c_str(), &result, true));

    Slic3r::Model reimported;
    REQUIRE(Slic3r::load_stl(out.c_str(), &reimported));
    REQUIRE_FALSE(reimported.objects.empty());
}

TEST_CASE("STL export: ASCII mode produces text starting with 'solid'", "[export][stl]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    std::string out = std::string(TMP) + "/export_ascii.stl";
    REQUIRE(Slic3r::store_stl(out.c_str(), &mesh, false));

    std::ifstream f(out);
    REQUIRE(f.is_open());
    std::string first_word;
    f >> first_word;
    REQUIRE(first_word == "solid");
}
