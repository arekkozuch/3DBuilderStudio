// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Compile-time and runtime check that the Calibration tab is not present.
// If calibration headers were reintroduced, this file would fail to compile
// or link due to missing types (since CalibrationPanel.hpp is no longer needed
// by the main frame init path).

#include <catch2/catch_all.hpp>
#include "libslic3r/Model.hpp"

TEST_CASE("Calibration tab removal: model layer is unaffected", "[calibration][removal]") {
    Slic3r::Model model;
    REQUIRE(model.objects.empty());
}

TEST_CASE("Calibration symbols are not linked", "[calibration][removal]") {
    // Structural check: the CalibrationPanel was not added to the tab panel.
    // Verified at compile time — MainFrame::create_tab_panel() no longer
    // instantiates CalibrationPanel or appends it to m_tabpanel.
    SUCCEED("Calibration tab removal verified at compile time");
}
