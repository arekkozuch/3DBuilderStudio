// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 3.6: Verify measurement geometry is intact after Phase 2 sidebar stripping.
// GLGizmoMeasure is UI-only and cannot be unit-tested headlessly; these tests
// confirm the underlying geometry (bounding-box face distances) is correct on
// the cube primitive that the gizmo measures against.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"

using Catch::Matchers::WithinAbs;

TEST_CASE("Measurement: cube edge length is 10mm", "[measure]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto bb = mesh.bounding_box();
    REQUIRE_THAT(bb.size().x(), WithinAbs(10.0, 1e-6));
    REQUIRE_THAT(bb.size().y(), WithinAbs(10.0, 1e-6));
    REQUIRE_THAT(bb.size().z(), WithinAbs(10.0, 1e-6));
}

TEST_CASE("Measurement: face-to-face distance equals edge length", "[measure]") {
    auto mesh = Slic3r::make_cube(10.0, 10.0, 10.0);
    auto bb = mesh.bounding_box();
    REQUIRE_THAT(bb.max.x() - bb.min.x(), WithinAbs(10.0, 1e-6));
    REQUIRE_THAT(bb.max.y() - bb.min.y(), WithinAbs(10.0, 1e-6));
    REQUIRE_THAT(bb.max.z() - bb.min.z(), WithinAbs(10.0, 1e-6));
}

TEST_CASE("Measurement: bounding box diagonal of unit cube is sqrt(3)", "[measure]") {
    auto mesh = Slic3r::make_cube(1.0, 1.0, 1.0);
    auto bb = mesh.bounding_box();
    auto sz = bb.size();
    double diagonal = std::sqrt(sz.x() * sz.x() + sz.y() * sz.y() + sz.z() * sz.z());
    REQUIRE_THAT(diagonal, WithinAbs(std::sqrt(3.0), 1e-6));
}
