// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "BuilderToolbar.hpp"

#include <wx/button.h>
#include <wx/sizer.h>

#include "libslic3r/TriangleMesh.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/GUI_ObjectList.hpp"
#include "slic3r/GUI/I18N.hpp"

namespace Slic3r {
namespace GUI {

BuilderToolbar::BuilderToolbar(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    SetBackgroundColour(wxColour(248, 248, 248));

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->AddSpacer(FromDIP(6));

    auto* btn_cube = new wxButton(this, wxID_ANY, _L("+ Cube"),
                                  wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    btn_cube->SetToolTip(_L("Insert a 10×10×10 mm cube into the scene"));
    btn_cube->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        const TriangleMesh mesh = make_cube(10.0, 10.0, 10.0);
        wxGetApp().obj_list()->load_mesh_object(mesh, _L("Cube"));
    });

    sizer->Add(btn_cube, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(3));

    // PR 3.3: Sphere button
    auto* btn_sphere = new wxButton(this, wxID_ANY, _L("+ Sphere"),
                                    wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    btn_sphere->SetToolTip(_L("Insert a sphere with radius 5 mm into the scene"));
    btn_sphere->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        const TriangleMesh mesh = make_sphere(5.0, 2.0 * M_PI / 60.0);
        wxGetApp().obj_list()->load_mesh_object(mesh, _L("Sphere"));
    });
    sizer->Add(btn_sphere, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(3));

    // PR 3.4: Cylinder button
    auto* btn_cyl = new wxButton(this, wxID_ANY, _L("+ Cylinder"),
                                 wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    btn_cyl->SetToolTip(_L("Insert a cylinder r=5mm h=10mm into the scene"));
    btn_cyl->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        const TriangleMesh mesh = make_cylinder(5.0, 10.0);
        wxGetApp().obj_list()->load_mesh_object(mesh, _L("Cylinder"));
    });
    sizer->Add(btn_cyl, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(3));

    // PR 3.5: Cone button
    auto* btn_cone = new wxButton(this, wxID_ANY, _L("+ Cone"),
                                  wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    btn_cone->SetToolTip(_L("Insert a cone r=5mm h=10mm into the scene"));
    btn_cone->Bind(wxEVT_BUTTON, [](wxCommandEvent&) {
        const TriangleMesh mesh = make_cone(5.0, 10.0);
        wxGetApp().obj_list()->load_mesh_object(mesh, _L("Cone"));
    });
    sizer->Add(btn_cone, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(3));

    sizer->AddStretchSpacer();

    SetSizer(sizer);
    Layout();
}

} // namespace GUI
} // namespace Slic3r
