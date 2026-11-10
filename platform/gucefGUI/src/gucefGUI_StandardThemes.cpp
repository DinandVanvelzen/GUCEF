/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefGUI_StandardThemes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  Adds a Style node; 'type', 'state' and 'styleClass' may be empty. Of the styles that select a widget
 *  equally well the last one wins, so the general ones come first.
 */
CORE::CDataNode*
AddStyle( CORE::CDataNode& theme, const char* type, const char* state = "", const char* styleClass = "" )
{
    CORE::CDataNode* style = theme.AddChild( "Style" );
    if ( '\0' != type[ 0 ] )
        style->SetAttribute( "type", type );
    if ( '\0' != state[ 0 ] )
        style->SetAttribute( "state", state );
    if ( '\0' != styleClass[ 0 ] )
        style->SetAttribute( "class", styleClass );
    return style;
}

/*-------------------------------------------------------------------------*/

void
SetStyleProperty( CORE::CDataNode& style, const char* name, const CString& value )
{
    CORE::CDataNode* property = style.AddChild( "Property" );
    property->SetAttribute( "name", name );
    property->SetAttribute( "value", value );
}

/*-------------------------------------------------------------------------*/

const char* const TextColour         = "226,229,236,255";
const char* const DimTextColour      = "150,156,170,255";
const char* const DisabledTextColour = "104,110,122,255";
const char* const WindowColour       = "24,26,32,250";
const char* const SurfaceColour      = "34,37,45,255";
const char* const ControlColour      = "48,53,64,255";
const char* const HoverColour        = "62,69,84,255";
const char* const PressedColour      = "40,44,54,255";
const char* const BorderColour       = "70,76,90,255";
const char* const AccentColour       = "74,136,232,255";
const char* const SelectionColour    = "52,92,160,255";

}

/*-------------------------------------------------------------------------*/

void
BuildDarkTheme( CORE::CDataNode& theme, const CString& fontName )
{GUCEF_TRACE;

    theme.Clear();
    theme.SetName( "GUITheme" );

    CORE::CDataNode* style = AddStyle( theme, "Widget" );
    if ( !fontName.IsNULLOrEmpty() )
        SetStyleProperty( *style, "Font", fontName );
    SetStyleProperty( *style, "TextColour", TextColour );
    SetStyleProperty( *style, "DisabledTextColour", DisabledTextColour );
    SetStyleProperty( *style, "SelectionColour", SelectionColour );
    style = AddStyle( theme, "Widget", "disabled" );
    SetStyleProperty( *style, "TextColour", DisabledTextColour );

    style = AddStyle( theme, "Window" );
    SetStyleProperty( *style, "BackgroundColour", WindowColour );
    SetStyleProperty( *style, "BorderColour", BorderColour );
    SetStyleProperty( *style, "BorderWidth", "1" );
    SetStyleProperty( *style, "TitleHeight", "40" );
    SetStyleProperty( *style, "TitleBarColour", "32,35,44,255" );
    SetStyleProperty( *style, "TitleColour", "240,242,247,255" );

    style = AddStyle( theme, "Label" );
    SetStyleProperty( *style, "TextColour", TextColour );
    style = AddStyle( theme, "Label", "", "Dim" );
    SetStyleProperty( *style, "TextColour", DimTextColour );

    style = AddStyle( theme, "Button" );
    SetStyleProperty( *style, "BackgroundColour", ControlColour );
    SetStyleProperty( *style, "BorderColour", BorderColour );
    SetStyleProperty( *style, "BorderWidth", "1" );
    style = AddStyle( theme, "Button", "hover" );
    SetStyleProperty( *style, "BackgroundColour", HoverColour );
    style = AddStyle( theme, "Button", "pressed" );
    SetStyleProperty( *style, "BackgroundColour", PressedColour );
    style = AddStyle( theme, "Button", "focused" );
    SetStyleProperty( *style, "BorderColour", AccentColour );
    style = AddStyle( theme, "Button", "disabled" );
    SetStyleProperty( *style, "BackgroundColour", "40,43,51,255" );
    SetStyleProperty( *style, "TextColour", DisabledTextColour );
    style = AddStyle( theme, "Button", "", "Primary" );
    SetStyleProperty( *style, "BackgroundColour", "50,96,170,255" );
    SetStyleProperty( *style, "BorderColour", AccentColour );
    style = AddStyle( theme, "Button", "hover", "Primary" );
    SetStyleProperty( *style, "BackgroundColour", "62,112,192,255" );

    style = AddStyle( theme, "Checkbox" );
    SetStyleProperty( *style, "BackgroundColour", "0,0,0,0" );
    SetStyleProperty( *style, "BorderColour", "0,0,0,0" );
    style = AddStyle( theme, "Checkbox", "hover" );
    SetStyleProperty( *style, "BackgroundColour", "255,255,255,12" );
    style = AddStyle( theme, "Checkbox", "focused" );
    SetStyleProperty( *style, "BorderColour", AccentColour );

    style = AddStyle( theme, "Editbox" );
    SetStyleProperty( *style, "BackgroundColour", SurfaceColour );
    SetStyleProperty( *style, "BorderColour", BorderColour );
    style = AddStyle( theme, "Editbox", "focused" );
    SetStyleProperty( *style, "BorderColour", AccentColour );

    style = AddStyle( theme, "Combobox" );
    SetStyleProperty( *style, "BackgroundColour", ControlColour );
    SetStyleProperty( *style, "BorderColour", BorderColour );
    style = AddStyle( theme, "Combobox", "hover" );
    SetStyleProperty( *style, "BackgroundColour", HoverColour );
    style = AddStyle( theme, "Combobox", "focused" );
    SetStyleProperty( *style, "BorderColour", AccentColour );

    style = AddStyle( theme, "Listbox" );
    SetStyleProperty( *style, "BackgroundColour", SurfaceColour );
    SetStyleProperty( *style, "BorderColour", BorderColour );
    SetStyleProperty( *style, "HoverColour", AccentColour );

    style = AddStyle( theme, "Textbox" );
    SetStyleProperty( *style, "BackgroundColour", "16,18,22,255" );
    SetStyleProperty( *style, "BorderColour", BorderColour );

    style = AddStyle( theme, "Slider" );
    SetStyleProperty( *style, "TrackColour", ControlColour );
    SetStyleProperty( *style, "ThumbColour", AccentColour );
    SetStyleProperty( *style, "ThumbSize", "14" );
    style = AddStyle( theme, "Slider", "disabled" );
    SetStyleProperty( *style, "ThumbColour", "80,86,100,255" );

    style = AddStyle( theme, "ProgressBar" );
    SetStyleProperty( *style, "BackgroundColour", SurfaceColour );
    SetStyleProperty( *style, "BorderColour", BorderColour );
    SetStyleProperty( *style, "BarColour", AccentColour );

    style = AddStyle( theme, "TabControl" );
    SetStyleProperty( *style, "TabHeight", "36" );
    SetStyleProperty( *style, "TabColour", "30,33,41,255" );
    SetStyleProperty( *style, "ActiveTabColour", ControlColour );
    SetStyleProperty( *style, "BackgroundColour", "28,31,38,255" );
    SetStyleProperty( *style, "BorderColour", BorderColour );

    style = AddStyle( theme, "TabContentPane" );
    SetStyleProperty( *style, "BackgroundColour", "28,31,38,255" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
