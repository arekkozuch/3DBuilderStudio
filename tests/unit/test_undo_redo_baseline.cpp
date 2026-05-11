// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Canary test for the undo/redo stack. Must pass after every Phase 2 PR.
// If this test fails, the PR must not be merged (see BOUNDARIES.md §9).

#include <catch2/catch_all.hpp>
#include "slic3r/Utils/UndoRedo.hpp"

TEST_CASE("Undo stack: default-constructed stack is empty", "[undo][canary]") {
    Slic3r::UndoRedo::Stack stack;
    REQUIRE(stack.empty());
}

TEST_CASE("Undo stack: fresh stack has no undo snapshot", "[undo][canary]") {
    Slic3r::UndoRedo::Stack stack;
    REQUIRE_FALSE(stack.has_undo_snapshot());
}

TEST_CASE("Undo stack: fresh stack has no redo snapshot", "[undo][canary]") {
    Slic3r::UndoRedo::Stack stack;
    REQUIRE_FALSE(stack.has_redo_snapshot());
}

TEST_CASE("Undo stack: snapshots vector is accessible on empty stack", "[undo][canary][edge]") {
    Slic3r::UndoRedo::Stack stack;
    REQUIRE_NOTHROW(stack.snapshots());
}

TEST_CASE("Undo stack: clear on empty stack does not crash", "[undo][canary][edge]") {
    Slic3r::UndoRedo::Stack stack;
    REQUIRE_NOTHROW(stack.clear());
    REQUIRE(stack.empty());
}

TEST_CASE("Undo stack: memory limit can be set and retrieved", "[undo][canary]") {
    Slic3r::UndoRedo::Stack stack;
    constexpr size_t limit = 256 * 1024 * 1024; // 256 MB
    stack.set_memory_limit(limit);
    REQUIRE(stack.get_memory_limit() == limit);
}
