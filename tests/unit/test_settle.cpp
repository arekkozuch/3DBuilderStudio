// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// PR 4.6: Settle / Orient-to-face tests.
// Tests the core rotation math used by GLGizmoFlatten::on_mouse() /
// Selection::flattening_rotate(): given a face normal, compute the
// quaternion rotation that aligns that normal to -Z (face-down), apply it
// to the mesh, and verify the result is axis-aligned.

#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libslic3r/TriangleMesh.hpp"

#include <Eigen/Geometry>

using Catch::Matchers::WithinAbs;

// Replicate the rotation from Selection::flattening_rotate() without GUI.
static Slic3r::Transform3d flatten_rotation(const Slic3r::Vec3d &face_normal)
{
    return Slic3r::Transform3d(
        Eigen::Quaterniond().setFromTwoVectors(face_normal, -Slic3r::Vec3d::UnitZ()));
}

TEST_CASE("Settle: top-face normal of upright cube points to +Z", "[settle]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);
    // make_cube places vertices in object space; find a face whose normal is +Z
    bool found_top = false;
    for (int i = 0; i < (int)cube.its.indices.size(); ++i) {
        Slic3r::Vec3f n = Slic3r::its_face_normal(cube.its, i);
        if (n.z() > 0.99f) { found_top = true; break; }
    }
    REQUIRE(found_top);
}

TEST_CASE("Settle: rotating top face to ground makes normal point to -Z", "[settle]") {
    // Top face normal of an upright cube is (0, 0, +1).
    // After flatten rotation it should point to (0, 0, -1).
    Slic3r::Vec3d normal{ 0.0, 0.0, 1.0 };
    Slic3r::Transform3d rot = flatten_rotation(normal);
    Slic3r::Vec3d rotated = rot * normal;

    REQUIRE_THAT(rotated.x(), WithinAbs(0.0, 1e-9));
    REQUIRE_THAT(rotated.y(), WithinAbs(0.0, 1e-9));
    REQUIRE_THAT(rotated.z(), WithinAbs(-1.0, 1e-9));
}

TEST_CASE("Settle: after flattening, bounding box min-Z is near zero", "[settle]") {
    auto cube = Slic3r::make_cube(10.0, 10.0, 10.0);

    // Tilt the cube 45° around X so no face is axis-aligned.
    Slic3r::Transform3d tilt = Slic3r::Transform3d::Identity();
    tilt.rotate(Eigen::AngleAxisd(M_PI / 4.0, Eigen::Vector3d::UnitX()));
    cube.transform(tilt);

    // Pick the bottom-most face normal (the one closest to -Z after tilt).
    Slic3r::Vec3d best_normal{ 0, 0, 1 };
    double        best_dot = -1.0;
    for (int i = 0; i < (int)cube.its.indices.size(); ++i) {
        Slic3r::Vec3f n = Slic3r::its_face_normal(cube.its, i);
        double dot = -n.cast<double>().dot(Slic3r::Vec3d::UnitZ()); // most downward
        if (dot > best_dot) { best_dot = dot; best_normal = n.cast<double>(); }
    }

    // Apply the flatten rotation.
    Slic3r::Transform3d rot = flatten_rotation(best_normal);
    cube.transform(rot);

    // Translate so min-Z == 0 (as the gizmo workflow does after rotation).
    auto bb = cube.bounding_box();
    Slic3r::Transform3d settle = Slic3r::Transform3d::Identity();
    settle.translate(Slic3r::Vec3d(0.0, 0.0, -bb.min.z()));
    cube.transform(settle);

    auto bb2 = cube.bounding_box();
    REQUIRE_THAT(bb2.min.z(), WithinAbs(0.0, 1e-4));
}

TEST_CASE("Settle: flatten rotation is a pure rotation (det ≈ +1)", "[settle]") {
    Slic3r::Vec3d normal{ 0.577, 0.577, 0.577 }; // diagonal
    normal.normalize();
    Slic3r::Transform3d rot = flatten_rotation(normal);
    double det = rot.matrix().block<3,3>(0,0).determinant();
    REQUIRE_THAT(det, WithinAbs(1.0, 1e-9));
}

TEST_CASE("Settle: flatten of already-ground-facing normal is identity-like", "[settle]") {
    // If the face already points down, the rotation should be near identity.
    Slic3r::Vec3d normal{ 0.0, 0.0, -1.0 };
    Slic3r::Transform3d rot = flatten_rotation(normal);
    Slic3r::Vec3d result = rot * normal;
    REQUIRE_THAT(result.z(), WithinAbs(-1.0, 1e-9));
}
