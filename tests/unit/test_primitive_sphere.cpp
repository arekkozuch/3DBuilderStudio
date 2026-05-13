// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 3.3: Tests for make_sphere(). The BuilderToolbar "Sphere" button calls
// make_sphere(5.0, 2π/60) and inserts via ObjectList::load_mesh_object.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("make_sphere: mesh is non-empty", "[primitives][sphere]") {
    auto mesh = Slic3r::make_sphere(5.0, 0.05);
    REQUIRE_FALSE(mesh.empty());
    REQUIRE(mesh.its.vertices.size() > 0);
    REQUIRE(mesh.its.indices.size() > 0);
}

TEST_CASE("make_sphere: mesh is closed (no open edges)", "[primitives][sphere]") {
    auto mesh = Slic3r::make_sphere(5.0, 0.05);
    REQUIRE(mesh.stats().open_edges == 0);
}

TEST_CASE("make_sphere: bounding box is approximately 2r in each dimension", "[primitives][sphere]") {
    const double r = 5.0;
    auto mesh = Slic3r::make_sphere(r, 0.05);
    auto bb = mesh.bounding_box();
    REQUIRE_THAT(bb.size().x(), WithinRel(2.0 * r, 0.05));
    REQUIRE_THAT(bb.size().y(), WithinRel(2.0 * r, 0.05));
    REQUIRE_THAT(bb.size().z(), WithinRel(2.0 * r, 0.05));
}

TEST_CASE("make_sphere: volume approximates (4/3)πr³", "[primitives][sphere]") {
    const double r        = 5.0;
    const double expected = (4.0 / 3.0) * PI * r * r * r;
    auto mesh = Slic3r::make_sphere(r, 0.01);
    REQUIRE_THAT(mesh.volume(), WithinRel(expected, 0.02));
}

TEST_CASE("make_sphere: zero radius does not crash", "[primitives][sphere][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_sphere(0.0, 0.05));
}
