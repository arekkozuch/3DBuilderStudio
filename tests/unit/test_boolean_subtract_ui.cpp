// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 4.2: Model-level boolean subtract tests (no GUI context required).
// Verifies that operating on Model objects produces the expected result
// mesh, which is what the ObjectList::boolean_subtract() UI action does.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"
#include "libslic3r/MeshBoolean.hpp"
#include "libslic3r/Model.hpp"

using Catch::Matchers::WithinRel;

TEST_CASE("boolean subtract UI: result mesh is non-empty", "[boolean][subtract][ui]") {
    auto target_mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool_mesh   = Slic3r::make_cube(5.0,  5.0,  5.0);
    auto result = Slic3r::mesh_subtract(target_mesh, tool_mesh);
    REQUIRE_FALSE(result.empty());
}

TEST_CASE("boolean subtract UI: result volume is reduced", "[boolean][subtract][ui]") {
    auto target_mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto tool_mesh   = Slic3r::make_cube(5.0,  5.0,  5.0);
    auto result = Slic3r::mesh_subtract(target_mesh, tool_mesh);
    REQUIRE(result.volume() < target_mesh.volume());
}

TEST_CASE("boolean subtract UI: two Model volumes subtract correctly", "[boolean][subtract][ui]") {
    Slic3r::Model model;
    auto* obj_a = model.add_object();
    obj_a->add_volume(Slic3r::make_cube(10.0, 10.0, 10.0));

    auto* obj_b = model.add_object();
    obj_b->add_volume(Slic3r::make_cube(5.0, 5.0, 5.0));

    REQUIRE(model.objects.size() == 2);

    auto target_mesh = obj_a->volumes[0]->mesh();
    auto tool_mesh   = obj_b->volumes[0]->mesh();
    auto result = Slic3r::mesh_subtract(target_mesh, tool_mesh);

    REQUIRE_FALSE(result.empty());
    REQUIRE(result.stats().open_edges == 0);
    const double expected = target_mesh.volume() - tool_mesh.volume();
    REQUIRE_THAT(result.volume(), WithinRel(expected, 0.02));
}
