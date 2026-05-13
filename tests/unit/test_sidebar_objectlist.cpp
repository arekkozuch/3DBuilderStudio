// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Verifies that the Model layer (backing store for ObjectList) works correctly
// after PR 2.5 strips the sidebar down to ObjectList only.

#include <catch2/catch_all.hpp>
#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"

TEST_CASE("ObjectList backing: model objects accessible after sidebar strip", "[sidebar][objectlist]") {
    Slic3r::Model model;
    bool ok = Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
    REQUIRE_FALSE(model.objects[0]->name.empty());
}

TEST_CASE("ObjectList backing: volumes present for display", "[sidebar][objectlist]") {
    Slic3r::Model model;
    Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE_FALSE(model.objects[0]->volumes.empty());
    REQUIRE(model.objects[0]->volumes[0]->mesh().stats().number_of_facets > 0);
}

TEST_CASE("ObjectList backing: multiple loads do not cross-contaminate", "[sidebar][objectlist]") {
    Slic3r::Model model_a;
    Slic3r::Model model_b;
    Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model_a);
    Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model_b);
    REQUIRE(model_a.objects.size() == 1);
    REQUIRE(model_b.objects.size() == 1);
    REQUIRE(model_a.objects[0] != model_b.objects[0]);
}
