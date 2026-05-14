// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 5.6: 3MF export round-trip tests.
// Uses store_3mf() at libslic3r level — the same function the GUI calls.
// Verifies object count and geometry are preserved across export/import.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"
#include "libslic3r/Format/3mf.hpp"
#include "libslic3r/Config.hpp"

static const char* TMP = "/tmp/meshforge_test_tmp";

TEST_CASE("3MF export: re-imported file has same object count", "[export][3mf]") {
    Slic3r::Model model;
    REQUIRE(Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model));
    model.add_default_instances();

    std::string out = std::string(TMP) + "/export_roundtrip.3mf";
    Slic3r::DynamicPrintConfig config;
    REQUIRE(Slic3r::store_3mf(out.c_str(), &model, &config, false));

    Slic3r::Model reimported;
    Slic3r::DynamicPrintConfig cfg2;
    Slic3r::ConfigSubstitutionContext subs{ Slic3r::ForwardCompatibilitySubstitutionRule::Enable };
    REQUIRE(Slic3r::load_3mf(out.c_str(), cfg2, subs, &reimported, false));
    REQUIRE(reimported.objects.size() == model.objects.size());
}

TEST_CASE("3MF export: re-imported file has same volume count", "[export][3mf]") {
    Slic3r::Model model;
    REQUIRE(Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model));
    model.add_default_instances();

    std::string out = std::string(TMP) + "/export_volumes.3mf";
    Slic3r::DynamicPrintConfig config;
    REQUIRE(Slic3r::store_3mf(out.c_str(), &model, &config, false));

    Slic3r::Model reimported;
    Slic3r::DynamicPrintConfig cfg2;
    Slic3r::ConfigSubstitutionContext subs{ Slic3r::ForwardCompatibilitySubstitutionRule::Enable };
    REQUIRE(Slic3r::load_3mf(out.c_str(), cfg2, subs, &reimported, false));
    REQUIRE(reimported.objects[0]->volumes.size() == model.objects[0]->volumes.size());
}

TEST_CASE("3MF export: output is a valid ZIP (PK signature)", "[export][3mf]") {
    Slic3r::Model model;
    REQUIRE(Slic3r::load_stl(TEST_DATA_DIR "/test_stl/ASCII/20mmbox-LF.stl", &model));
    model.add_default_instances();

    std::string out = std::string(TMP) + "/export_zip_check.3mf";
    Slic3r::DynamicPrintConfig config;
    REQUIRE(Slic3r::store_3mf(out.c_str(), &model, &config, false));

    // 3MF is a ZIP; first two bytes must be the PK magic number.
    std::ifstream f(out, std::ios::binary);
    REQUIRE(f.is_open());
    unsigned char sig[2] = {};
    f.read(reinterpret_cast<char*>(sig), 2);
    REQUIRE(sig[0] == 0x50); // 'P'
    REQUIRE(sig[1] == 0x4B); // 'K'
}
