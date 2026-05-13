// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 3.5: Tests for make_cone(). The BuilderToolbar "Cone" button calls
// make_cone(5.0, 10.0) and inserts via ObjectList::load_mesh_object.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("make_cone: mesh is non-empty", "[primitives][cone]") {
    auto mesh = Slic3r::make_cone(5.0, 10.0);
    REQUIRE_FALSE(mesh.empty());
}

TEST_CASE("make_cone: mesh is closed (no open edges)", "[primitives][cone]") {
    auto mesh = Slic3r::make_cone(5.0, 10.0);
    REQUIRE(mesh.stats().open_edges == 0);
}

TEST_CASE("make_cone: height matches bounding box Z", "[primitives][cone]") {
    auto mesh = Slic3r::make_cone(5.0, 10.0);
    auto bb = mesh.bounding_box();
    REQUIRE_THAT(bb.size().z(), WithinAbs(10.0, 1e-4));
}

TEST_CASE("make_cone: volume approximates (1/3)*pi*r^2*h", "[primitives][cone]") {
    const double r = 5.0, h = 10.0;
    const double expected = (1.0 / 3.0) * PI * r * r * h;
    auto mesh = Slic3r::make_cone(r, h, 0.01);
    REQUIRE_THAT(mesh.volume(), WithinRel(expected, 0.03));
}

TEST_CASE("make_cone: zero radius does not crash", "[primitives][cone][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_cone(0.0, 10.0));
}
