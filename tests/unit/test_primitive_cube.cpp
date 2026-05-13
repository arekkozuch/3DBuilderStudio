// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 3.2: Tests for the make_cube() primitive. The BuilderToolbar calls
// make_cube(10, 10, 10) and passes the result to ObjectList::load_mesh_object.
// These tests verify the geometry is correct before the toolbar wires it up.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/Model.hpp"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("make_cube: geometry is correct", "[primitives][cube]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    REQUIRE(mesh.its.vertices.size() == 8);
    REQUIRE(mesh.its.indices.size() == 12);
    REQUIRE_FALSE(mesh.empty());
}

TEST_CASE("make_cube: mesh is closed (no open edges)", "[primitives][cube]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    // fill_initial_stats() computes open_edges in the constructor — no repair needed.
    REQUIRE(mesh.stats().open_edges == 0);
}

TEST_CASE("make_cube: volume is correct", "[primitives][cube]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    REQUIRE_THAT(mesh.volume(), WithinRel(1000.0, 0.01));
}

TEST_CASE("make_cube: non-uniform dimensions", "[primitives][cube]") {
    auto mesh = Slic3r::make_cube(5.0, 10.0, 20.0);
    auto bb = mesh.bounding_box();
    REQUIRE_THAT(bb.size().x(), WithinAbs(5.0,  1e-6));
    REQUIRE_THAT(bb.size().y(), WithinAbs(10.0, 1e-6));
    REQUIRE_THAT(bb.size().z(), WithinAbs(20.0, 1e-6));
}

TEST_CASE("make_cube: zero-size dimension does not crash", "[primitives][cube][edge]") {
    REQUIRE_NOTHROW(Slic3r::make_cube(0.0, 10.0, 10.0));
}

TEST_CASE("make_cube: added to model appears in object list", "[primitives][cube][integration]") {
    Slic3r::Model model;
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto* obj = model.add_object();
    obj->add_volume(mesh);
    REQUIRE(model.objects.size() == 1);
    REQUIRE(model.objects[0]->volumes.size() == 1);
}
