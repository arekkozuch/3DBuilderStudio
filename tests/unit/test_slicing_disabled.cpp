// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Verifies that model loading does not trigger slicing side effects.
// Slicing is disabled in MeshForge (PR 2.4).

#include <catch2/catch_all.hpp>
#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"

TEST_CASE("No slicing occurs on model load", "[slicing][disabled]") {
    Slic3r::Model model;
    bool ok = Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
    // layer_height_profile is populated only after slicing — must be empty on raw load.
    REQUIRE(model.objects[0]->layer_height_profile.empty());
}

TEST_CASE("Model volume is accessible without slicing", "[slicing][disabled]") {
    Slic3r::Model model;
    Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE_FALSE(model.objects[0]->volumes.empty());
    // Mesh is present; slice results are not.
    REQUIRE(model.objects[0]->volumes[0]->mesh().stats().number_of_facets > 0);
}
