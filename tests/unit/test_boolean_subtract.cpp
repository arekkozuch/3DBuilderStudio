// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 4.1: Tests for mesh_subtract() — the value-returning wrapper around
// MeshBoolean::cgal::minus(). The tool cube always shares the origin corner
// with the target cube so it is fully contained; no explicit translation needed.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/MeshBoolean.hpp"

using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

TEST_CASE("mesh_subtract: small cube from large cube — result non-empty", "[boolean][subtract]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0,  5.0,  5.0);
    auto result = Slic3r::mesh_subtract(target, tool);
    REQUIRE_FALSE(result.empty());
}

TEST_CASE("mesh_subtract: volume is reduced by tool volume", "[boolean][subtract]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0,  5.0,  5.0);
    auto result = Slic3r::mesh_subtract(target, tool);
    REQUIRE(result.volume() < target.volume());
    const double expected = target.volume() - tool.volume();
    REQUIRE_THAT(result.volume(), WithinRel(expected, 0.02));
}

TEST_CASE("mesh_subtract: result is closed (no open edges)", "[boolean][subtract]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0,  5.0,  5.0);
    auto result = Slic3r::mesh_subtract(target, tool);
    REQUIRE(result.stats().open_edges == 0);
}

TEST_CASE("mesh_subtract: non-overlapping tool — target volume preserved", "[boolean][subtract]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(5.0,  5.0,  5.0);
    // Move tool far away so it does not intersect target.
    tool.translate(100.f, 100.f, 100.f);
    auto result = Slic3r::mesh_subtract(target, tool);
    const double tv = target.volume();
    REQUIRE_THAT(result.volume(), WithinRel(tv, 0.01));
}

TEST_CASE("mesh_subtract: subtracting identical mesh — near-zero volume", "[boolean][subtract][edge]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool   = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto result = Slic3r::mesh_subtract(target, tool);
    REQUIRE_THAT(result.volume(), WithinAbs(0.0, 1.0));
}

TEST_CASE("mesh_subtract: empty tool returns target unchanged", "[boolean][subtract][edge]") {
    auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
    Slic3r::TriangleMesh empty_tool;
    auto result = Slic3r::mesh_subtract(target, empty_tool);
    const double tv = target.volume();
    REQUIRE_THAT(result.volume(), WithinRel(tv, 1e-6));
}
