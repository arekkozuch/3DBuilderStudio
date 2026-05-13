// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 4.3: Tests for mesh_union().

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/MeshBoolean.hpp"

using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

TEST_CASE("mesh_union: result is non-empty", "[boolean][union]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_cube(10.0, 10.0, 10.0);
    b.translate(10.f, 0.f, 0.f);
    auto result = Slic3r::mesh_union(a, b);
    REQUIRE_FALSE(result.empty());
}

TEST_CASE("mesh_union: two non-overlapping cubes — volume equals sum", "[boolean][union]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_cube(10.0, 10.0, 10.0);
    b.translate(10.f, 0.f, 0.f); // adjacent, not overlapping
    auto result = Slic3r::mesh_union(a, b);
    const double expected = a.volume() + b.volume();
    REQUIRE_THAT(result.volume(), WithinRel(expected, 0.02));
}

TEST_CASE("mesh_union: two identical cubes — volume equals one cube", "[boolean][union]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_cube(10.0, 10.0, 10.0); // same position
    auto result = Slic3r::mesh_union(a, b);
    const double av = a.volume();
    REQUIRE_THAT(result.volume(), WithinRel(av, 0.02));
}

TEST_CASE("mesh_union: result is closed (no open edges)", "[boolean][union]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto b = Slic3r::make_sphere(6.0, 0.05);
    auto result = Slic3r::mesh_union(a, b);
    REQUIRE(result.stats().open_edges == 0);
}

TEST_CASE("mesh_union: empty second mesh returns first unchanged", "[boolean][union][edge]") {
    auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
    Slic3r::TriangleMesh empty;
    auto result = Slic3r::mesh_union(a, empty);
    const double av = a.volume();
    REQUIRE_THAT(result.volume(), WithinRel(av, 1e-6));
}
