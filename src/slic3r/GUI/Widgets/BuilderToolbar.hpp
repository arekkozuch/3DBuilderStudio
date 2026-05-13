// Copyright (c) 2026 MeshForge Project
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// BuilderToolbar — horizontal strip of buttons for inserting primitive shapes
// into the 3D scene. PR 3.2: Cube button.

#pragma once

#include <wx/panel.h>

namespace Slic3r {
namespace GUI {

class BuilderToolbar : public wxPanel
{
public:
    explicit BuilderToolbar(wxWindow* parent);
};

} // namespace GUI
} // namespace Slic3r
