// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <catch2/catch_all.hpp>
#include "libslic3r/libslic3r.h"

TEST_CASE("libslic3r version string is present", "[sanity]") {
    REQUIRE_FALSE(std::string(SLIC3R_VERSION).empty());
}

TEST_CASE("libslic3r version does not contain upstream brand names", "[sanity][branding]") {
    std::string version = SLIC3R_VERSION;
    REQUIRE(version.find("Orca") == std::string::npos);
    REQUIRE(version.find("Bambu") == std::string::npos);
    REQUIRE(version.find("Prusa") == std::string::npos);
}
