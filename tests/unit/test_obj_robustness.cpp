// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 6.1: OBJ import robustness tests.
// OBJ loader is inherited from OrcaSlicer. These tests guard against
// crashes on malformed and edge-case inputs.

#include <catch2/catch_all.hpp>
#include "libslic3r/Model.hpp"
#include "libslic3r/Format/OBJ.hpp"

TEST_CASE("OBJ import: valid cube loads with one object", "[import][obj]") {
    Slic3r::Model model;
    Slic3r::ObjInfo info;
    std::string msg;
    bool ok = Slic3r::load_obj(FIXTURES_DIR "/cube_10mm.obj", &model, info, msg);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
    REQUIRE_FALSE(model.objects[0]->volumes.empty());
}

TEST_CASE("OBJ import: empty file does not crash", "[import][obj][robustness]") {
    Slic3r::Model model;
    Slic3r::ObjInfo info;
    std::string msg;
    REQUIRE_NOTHROW(Slic3r::load_obj(FIXTURES_DIR "/empty.obj", &model, info, msg));
}

TEST_CASE("OBJ import: file with missing material does not crash", "[import][obj][robustness]") {
    Slic3r::Model model;
    Slic3r::ObjInfo info;
    std::string msg;
    REQUIRE_NOTHROW(Slic3r::load_obj(FIXTURES_DIR "/no_material.obj", &model, info, msg));
}

TEST_CASE("OBJ import: non-existent file does not crash", "[import][obj][robustness]") {
    Slic3r::Model model;
    Slic3r::ObjInfo info;
    std::string msg;
    REQUIRE_NOTHROW(Slic3r::load_obj(FIXTURES_DIR "/does_not_exist.obj", &model, info, msg));
}
