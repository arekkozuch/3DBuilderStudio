// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 4.4: Tests for mesh_intersect().
// make_cube(n,n,n) places the cube with one corner at the origin.
// make_sphere(r, step) centres the sphere at the origin by default.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/MeshBoolean.hpp"

using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

TEST_CASE("mesh_intersect: cube fully inside sphere — result approximates cube", "[boolean][intersect]") {
    // Cube 10×10×10 with corner at origin (spans 0..10 on all axes).
    // Large sphere radius 20 centred at (5,5,5) encloses the cube entirely.
    auto cube   = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto sphere = Slic3r::make_sphere(20.0, 0.05);
    sphere.translate(5.f, 5.f, 5.f);
    auto result = Slic3r::mesh_intersect(cube, sphere);
    REQUIRE_FALSE(result.empty());
    const double cv = cube.volume();
    REQUIRE_THAT(result.volume(), WithinRel(cv, 0.05));
}

TEST_CASE("mesh_intersect: non-overlapping meshes — result is empty or near-zero", "[boolean][intersect]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_cube(10.0, 10.0, 10.0);
    b.translate(100.f, 0.f, 0.f);
    auto result = Slic3r::mesh_intersect(a, b);
    REQUIRE_THAT(result.volume(), WithinAbs(0.0, 1.0));
}

TEST_CASE("mesh_intersect: two identical cubes — result is the cube", "[boolean][intersect]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto result = Slic3r::mesh_intersect(a, b);
    const double av = a.volume();
    REQUIRE_THAT(result.volume(), WithinRel(av, 0.02));
}

TEST_CASE("mesh_intersect: result is closed (no open edges)", "[boolean][intersect]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_sphere(8.0, 0.05);
    b.translate(5.f, 5.f, 5.f);
    auto result = Slic3r::mesh_intersect(a, b);
    REQUIRE(result.stats().open_edges == 0);
}

TEST_CASE("mesh_intersect: empty input returns empty", "[boolean][intersect][edge]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    Slic3r::TriangleMesh empty;
    auto result = Slic3r::mesh_intersect(a, empty);
    REQUIRE(result.empty());
}
