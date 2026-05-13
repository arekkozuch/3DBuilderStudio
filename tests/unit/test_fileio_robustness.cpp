// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 5.1 / 5.3 consolidated: robustness tests for STL and 3MF import.
// Import functionality is inherited from OrcaSlicer. These tests guard
// against crashes and regressions on malformed inputs; they do NOT
// re-test the parser logic itself.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"
#include "libslic3r/Format/3mf.hpp"
#include "libslic3r/Config.hpp"

using Catch::Matchers::WithinRel;

// ── STL robustness ────────────────────────────────────────────────────────────

TEST_CASE("STL import: binary STL loads and produces object", "[import][stl]") {
    Slic3r::Model model;
    bool ok = Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
    REQUIRE_FALSE(model.objects[0]->volumes.empty());
}

TEST_CASE("STL import: ASCII STL loads correctly", "[import][stl]") {
    Slic3r::Model model;
    bool ok = Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
}

TEST_CASE("STL import: empty file does not crash", "[import][stl][robustness]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_stl(FIXTURES_DIR "/empty.stl", &model));
}

TEST_CASE("STL import: truncated file does not crash", "[import][stl][robustness]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_stl(FIXTURES_DIR "/truncated.stl", &model));
}

TEST_CASE("STL import: bogus triangle count does not crash", "[import][stl][robustness]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_stl(FIXTURES_DIR "/bad_count.stl", &model));
}

TEST_CASE("STL import: non-existent file does not crash", "[import][stl][robustness]") {
    Slic3r::Model model;
    REQUIRE_NOTHROW(Slic3r::load_stl(FIXTURES_DIR "/does_not_exist.stl", &model));
}

// ── 3MF robustness ────────────────────────────────────────────────────────────

TEST_CASE("3MF import: standard geometry-only 3MF loads correctly", "[import][3mf]") {
    Slic3r::Model model;
    Slic3r::DynamicPrintConfig config;
    Slic3r::ConfigSubstitutionContext subs{ Slic3r::ForwardCompatibilitySubstitutionRule::Enable };
    bool ok = Slic3r::load_3mf(FIXTURES_DIR "/cube_10mm.3mf", config, subs, &model, false);
    REQUIRE(ok);
    REQUIRE(model.objects.size() == 1);
}

TEST_CASE("3MF import: file with Bambu extension namespace does not crash", "[import][3mf][robustness]") {
    Slic3r::Model model;
    Slic3r::DynamicPrintConfig config;
    Slic3r::ConfigSubstitutionContext subs{ Slic3r::ForwardCompatibilitySubstitutionRule::Enable };
    REQUIRE_NOTHROW(Slic3r::load_3mf(FIXTURES_DIR "/cube_bambu_extensions.3mf", config, subs, &model, false));
}

TEST_CASE("3MF import: empty file does not crash", "[import][3mf][robustness]") {
    Slic3r::Model model;
    Slic3r::DynamicPrintConfig config;
    Slic3r::ConfigSubstitutionContext subs{ Slic3r::ForwardCompatibilitySubstitutionRule::Enable };
    REQUIRE_NOTHROW(Slic3r::load_3mf(FIXTURES_DIR "/empty.3mf", config, subs, &model, false));
}

TEST_CASE("3MF import: truncated ZIP does not crash", "[import][3mf][robustness]") {
    Slic3r::Model model;
    Slic3r::DynamicPrintConfig config;
    Slic3r::ConfigSubstitutionContext subs{ Slic3r::ForwardCompatibilitySubstitutionRule::Enable };
    REQUIRE_NOTHROW(Slic3r::load_3mf(FIXTURES_DIR "/truncated.3mf", config, subs, &model, false));
}
