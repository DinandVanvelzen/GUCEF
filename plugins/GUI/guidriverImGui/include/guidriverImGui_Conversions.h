/*
 *  guidriverImGui: the gucefGUI driver library of Dear ImGui, shared by its renderer drivers
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUIDRIVERIMGUI_CONVERSIONS_H
#define GUIDRIVERIMGUI_CONVERSIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#include "imgui.h"

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Conversions between the types of gucefGUI and those of Dear ImGui. They are free functions rather than the
 *  IM_VEC2_CLASS_EXTRA hooks of imconfig.h, so that the imgui module does not need the GUCEF headers.
 */

/** A point in logical pixels */
inline ImVec2
ToImVec2( Int32 x, Int32 y )
{
    return ImVec2( (float) x, (float) y );
}

/** The top left corner of a rectangle */
inline ImVec2
ToImVec2( const GUI::CRect& rect )
{
    return ImVec2( (float) rect.left, (float) rect.top );
}

/** A rectangle as Dear ImGui writes clip rectangles: left, top, right, bottom */
inline ImVec4
ToImVec4( const GUI::CRect& rect )
{
    return ImVec4( (float) rect.left, (float) rect.top, (float) rect.Right(), (float) rect.Bottom() );
}

/** A colour with channels from 0 to 1, not premultiplied */
inline ImVec4
ToImVec4( const GUI::CColour& colour )
{
    return ImVec4( colour.r / 255.0f, colour.g / 255.0f, colour.b / 255.0f, colour.a / 255.0f );
}

/** A colour packed as Dear ImGui keeps it in vertices (IM_COL32) */
inline ImU32
ToImU32( const GUI::CColour& colour )
{
    return IM_COL32( colour.r, colour.g, colour.b, colour.a );
}

/** A packed colour of Dear ImGui */
inline GUI::CColour
ToColour( ImU32 colour )
{
    return GUI::CColour( (UInt8) ( ( colour >> IM_COL32_R_SHIFT ) & 0xFF ), (UInt8) ( ( colour >> IM_COL32_G_SHIFT ) & 0xFF ),
                         (UInt8) ( ( colour >> IM_COL32_B_SHIFT ) & 0xFF ), (UInt8) ( ( colour >> IM_COL32_A_SHIFT ) & 0xFF ) );
}

/** A colour with channels from 0 to 1, rounded to 8 bits */
inline GUI::CColour
ToColour( const ImVec4& colour )
{
    return ToColour( ImGui::ColorConvertFloat4ToU32( colour ) );
}

/** A rectangle of Dear ImGui (left, top, right, bottom) in whole pixels: the edges are rounded down */
inline GUI::CRect
ToRect( const ImVec4& rect )
{
    Int32 left = (Int32) floorf( rect.x );
    Int32 top = (Int32) floorf( rect.y );
    return GUI::CRect( left, top, (Int32) floorf( rect.z ) - left, (Int32) floorf( rect.w ) - top );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CONVERSIONS_H ? */
