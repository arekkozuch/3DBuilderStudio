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
    sizer->AddStretchSpacer();

    SetSizer(sizer);
    Layout();
}

} // namespace GUI
} // namespace Slic3r
