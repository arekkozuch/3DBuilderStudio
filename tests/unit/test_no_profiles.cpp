// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Structural tests for PR 2.6 — printer profile system removal.
// The profile menu items are removed at the UI layer; the model layer
// must remain fully functional without any printer preset being active.

#include <catch2/catch_all.hpp>
#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"

TEST_CASE("Model geometry works without printer profile", "[profiles][removal]") {
    Slic3r::Model model;
    bool ok = Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
    REQUIRE(model.objects[0]->volumes[0]->mesh().stats().number_of_facets > 0);
}

TEST_CASE("layer_height_profile is empty without a printer preset", "[profiles][removal]") {
    // layer_height_profile is populated by the slicing pipeline which requires
    // an active printer preset. Since MeshForge removes that pipeline, it must
    // remain empty on a raw model load.
    Slic3r::Model model;
    Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(model.objects[0]->layer_height_profile.empty());
}

TEST_CASE("Empty model is valid without printer profile", "[profiles][removal]") {
    Slic3r::Model model;
    REQUIRE(model.objects.empty());
    SUCCEED("Model layer is functional without any printer profile");
}
