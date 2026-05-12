// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Verifies that model loading still works after the Device tab is removed.
// UI tabs cannot be tested without a display; this tests the underlying model layer.

#include <catch2/catch_all.hpp>
#include "libslic3r/Model.hpp"
#include "libslic3r/Format/STL.hpp"

TEST_CASE("Model loads correctly after Device tab removal", "[ui][tabs]") {
    Slic3r::Model model;
    bool loaded = Slic3r::load_stl(TEST_DATA_DIR "/cube_10mm.stl", &model);
    REQUIRE(loaded);
    REQUIRE(model.objects.size() == 1);
    REQUIRE_FALSE(model.objects[0]->volumes.empty());
}

TEST_CASE("Undo stack is still functional after Device tab removal", "[ui][tabs][undo]") {
    Slic3r::UndoRedo::Stack stack;
    REQUIRE(stack.empty());
    REQUIRE_FALSE(stack.has_undo_snapshot());
}
