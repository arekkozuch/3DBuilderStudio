// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <catch2/catch.hpp>
#include "slic3r/Utils/NetworkAgent.hpp"

TEST_CASE("NetworkAgent::is_enabled returns false when SLIC3R_NETWORK=OFF", "[network][stub]") {
#ifdef SLIC3R_NETWORK_ENABLED
    SUCCEED("SLIC3R_NETWORK is ON — skip stub test");
#else
    REQUIRE_FALSE(Slic3r::NetworkAgent::is_enabled());
#endif
}

TEST_CASE("NetworkAgent::is_network_module_loaded returns false without initialization", "[network][stub]") {
    // The module is never initialized when SLIC3R_NETWORK=OFF.
    // When ON, this still passes if no plugin was loaded.
    REQUIRE_FALSE(Slic3r::NetworkAgent::is_network_module_loaded());
}
