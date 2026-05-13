// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 3.4: Tests for make_cylinder(). The BuilderToolbar "Cylinder" button
// calls make_cylinder(5.0, 10.0) and inserts via ObjectList::load_mesh_object.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("make_cylinder: mesh is non-empty", "[primitives][cylinder]") {
    auto mesh = Slic3r::make_cylinder(5.0, 10.0);
    REQUIRE_FALSE(mesh.empty());
}

TEST_CASE("make_cylinder: mesh is closed (no open edges)", "[primitives][cylinder]") {
    auto mesh = Slic3r::make_cylinder(5.0, 10.0);
    REQUIRE(mesh.stats().open_edges == 0);
}

TEST_CASE("make_cylinder: height matches bounding box Z", "[primitives][cylinder]") {
    auto mesh = Slic3r::make_cylinder(5.0, 10.0);
    auto bb = mesh.bounding_box();
    REQUIRE_THAT(bb.size().z(), WithinAbs(10.0, 1e-4));
}

TEST_CASE("make_cylinder: volume approximates pi*r^2*h", "[primitives][cylinder]") {
    const double r = 5.0, h = 10.0;
    const double expected = PI * r * r * h;
    auto mesh = Slic3r::make_cylinder(r, h, 0.01);
    REQUIRE_THAT(mesh.volume(), WithinRel(expected, 0.02));
}

TEST_CASE("make_cylinder: zero radius does not crash", "[primitives][cylinder][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_cylinder(0.0, 10.0));
}

TEST_CASE("make_cylinder: zero height does not crash", "[primitives][cylinder][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_cylinder(5.0, 0.0));
}
