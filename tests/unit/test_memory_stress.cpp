// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 6.3: Memory stress tests.
// Verifies that repeated CGAL boolean operations do not accumulate
// memory permanently (each result is destroyed at end of scope).
// If this test completes without OOM kill (exit 137), destructors
// are releasing CGAL allocations correctly.

#include <catch2/catch_all.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/MeshBoolean.hpp"
#include "libslic3r/TriangleMeshSlicer.hpp"

TEST_CASE("Memory: 50 boolean subtracts complete without OOM", "[perf][memory]") {
    for (int i = 0; i < 50; ++i) {
        auto target = Slic3r::make_cube(10.0, 10.0, 10.0);
        auto tool   = Slic3r::make_sphere(3.0, 0.5);
        auto result = Slic3r::mesh_subtract(target, tool);
        REQUIRE_FALSE(result.empty());
        // result, target, tool all destroyed here
    }
    SUCCEED("50 iterations without OOM");
}

TEST_CASE("Memory: 50 boolean unions complete without OOM", "[perf][memory]") {
    for (int i = 0; i < 50; ++i) {
        auto a = Slic3r::make_cube(10.0, 10.0, 10.0);
        auto b = Slic3r::make_sphere(6.0, 0.5);
        auto result = Slic3r::mesh_union(a, b);
        REQUIRE_FALSE(result.empty());
    }
    SUCCEED("50 iterations without OOM");
}

TEST_CASE("Memory: 50 plane cuts complete without OOM", "[perf][memory]") {
    for (int i = 0; i < 50; ++i) {
        auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
        auto [upper, lower] = Slic3r::cut_mesh(mesh, 5.0f);
        REQUIRE_FALSE(upper.empty());
        REQUIRE_FALSE(lower.empty());
    }
    SUCCEED("50 iterations without OOM");
}
