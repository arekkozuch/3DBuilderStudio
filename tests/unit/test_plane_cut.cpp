// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 4.5: Plane-cut / split tests.
// cut_mesh(mesh, z) returns {upper, lower} where upper contains all geometry
// above z and lower contains all geometry below z, both with capped faces.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/TriangleMeshSlicer.hpp"

using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

TEST_CASE("Plane cut: cube split at midpoint yields two equal halves", "[cut]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto [upper, lower] = Slic3r::cut_mesh(cube, 5.0f);

    REQUIRE_FALSE(upper.empty());
    REQUIRE_FALSE(lower.empty());

    const double half_vol = cube.volume() / 2.0;
    REQUIRE_THAT((double)upper.volume(), WithinRel(half_vol, 0.02));
    REQUIRE_THAT((double)lower.volume(), WithinRel(half_vol, 0.02));
}

TEST_CASE("Plane cut: both halves are closed manifolds", "[cut]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto [upper, lower] = Slic3r::cut_mesh(cube, 5.0f);

    REQUIRE(upper.stats().open_edges == 0);
    REQUIRE(lower.stats().open_edges == 0);
}

TEST_CASE("Plane cut: volumes sum to original", "[cut]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    const double original_vol = cube.volume();
    auto [upper, lower] = Slic3r::cut_mesh(cube, 3.0f);

    const double sum = (double)upper.volume() + (double)lower.volume();
    REQUIRE_THAT(sum, WithinRel(original_vol, 0.02));
}

TEST_CASE("Plane cut: cut above mesh yields empty upper and full lower", "[cut][edge]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    // Cutting at z=15 is above the cube (which spans 0..10)
    auto [upper, lower] = Slic3r::cut_mesh(cube, 15.0f);

    REQUIRE(upper.empty());
    REQUIRE_THAT((double)lower.volume(), WithinRel((double)cube.volume(), 0.02));
}

TEST_CASE("Plane cut: cut below mesh yields full upper and empty lower", "[cut][edge]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    // Cutting at z=-1 is below the cube (which spans 0..10)
    auto [upper, lower] = Slic3r::cut_mesh(cube, -1.0f);

    REQUIRE_THAT((double)upper.volume(), WithinRel((double)cube.volume(), 0.02));
    REQUIRE(lower.empty());
}
