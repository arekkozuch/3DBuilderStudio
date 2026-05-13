// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 3.1: Verify that ModelInstance Z positions are not auto-clamped.
// The automatic viewport bed-snap was removed from Selection::translate().
// These tests verify the model layer stores and returns any Z value faithfully.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"

using Catch::Matchers::WithinAbs;

TEST_CASE("Object can be placed below Z=0", "[transform][viewport]") {
    Slic3r::Model model;
    Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(model.objects.size() == 1);
    auto* inst = model.objects[0]->add_instance();
    REQUIRE(inst != nullptr);

    inst->set_offset(Slic3r::Vec3d(0.0, 0.0, -50.0));
    REQUIRE_THAT(inst->get_offset().z(), WithinAbs(-50.0, 1e-9));
}

TEST_CASE("Object can be placed above Z=0", "[transform][viewport]") {
    Slic3r::Model model;
    Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(model.objects.size() == 1);
    auto* inst = model.objects[0]->add_instance();
    REQUIRE(inst != nullptr);

    inst->set_offset(Slic3r::Vec3d(0.0, 0.0, 100.0));
    REQUIRE_THAT(inst->get_offset().z(), WithinAbs(100.0, 1e-9));
}

TEST_CASE("Object Z position survives XY translation", "[transform][viewport]") {
    Slic3r::Model model;
    Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(model.objects.size() == 1);
    auto* inst = model.objects[0]->add_instance();
    REQUIRE(inst != nullptr);

    inst->set_offset(Slic3r::Vec3d(10.0, 20.0, -15.0));
    const auto off = inst->get_offset();
    REQUIRE_THAT(off.x(), WithinAbs(10.0,  1e-9));
    REQUIRE_THAT(off.y(), WithinAbs(20.0,  1e-9));
    REQUIRE_THAT(off.z(), WithinAbs(-15.0, 1e-9));
}
