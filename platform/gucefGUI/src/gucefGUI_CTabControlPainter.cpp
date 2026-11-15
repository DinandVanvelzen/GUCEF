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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#include "gucefGUI_CTabControlPainter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CColour CTabControlPainter::BadgeColour( 74, 136, 232, 255 );
const CColour CTabControlPainter::BadgeTextColour( 255, 255, 255, 255 );

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CWidgetPainter::TCommandVector TCommands;

/* the look of the standard theme, as CWidgetPainter has it */
const CColour BoxColour( 48, 48, 52, 255 );
const CColour BoxBorderColour( 120, 120, 128, 255 );
const CColour DisabledTextColour( 128, 128, 128, 255 );

CColour
ColourOr( const CWidget& widget, EWidgetProperty property, const CColour& fallback )
{
    CColour colour;
    if ( !widget.TryGetColourProperty( property, colour ) )
        return fallback;
    return colour;
}

/** The text colour, grey when the widget is disabled */
CColour
TextColourOf( const CWidget& widget )
{
    if ( !widget.IsEffectivelyEnabled() )
        return ColourOr( widget, EWidgetProperty_DisabledTextColour, DisabledTextColour );
    return ColourOr( widget, EWidgetProperty_TextColour, CColour( 255, 255, 255, 255 ) );
}

void
Fill( TCommands& commands, const CRect& rect, const CColour& colour )
{
    if ( rect.IsEmpty() || 0 == colour.a )
        return;
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_FILLRECT;
    command.rect = rect;
    command.colour = colour;
    commands.push_back( command );
}

void
Stroke( TCommands& commands, const CRect& rect, const CColour& colour, Int32 thickness )
{
    if ( rect.IsEmpty() || 0 == colour.a || thickness <= 0 )
        return;
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_STROKERECT;
    command.rect = rect;
    command.colour = colour;
    command.thickness = thickness;
    commands.push_back( command );
}

void
Line( TCommands& commands, Int32 x0, Int32 y0, Int32 x1, Int32 y1, const CColour& colour )
{
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_LINE;
    command.x0 = x0;
    command.y0 = y0;
    command.x1 = x1;
    command.y1 = y1;
    command.colour = colour;
    commands.push_back( command );
}

/** Three short bars, the sign of a menu, in the middle of a box */
void
MenuSign( TCommands& commands, const CRect& box, const CColour& colour )
{
    const Int32 cx = box.left + box.width / 2;
    const Int32 cy = box.top + box.height / 2;
    for ( Int32 i=-1; i<=1; ++i )
        Line( commands, cx - 5, cy + i * 4, cx + 5, cy + i * 4, colour );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTabControlPainter::Paint( const CTabControl& tabs, CGUIContext& context, const CRect& area, TCommandVector& commands )
{GUCEF_TRACE;

    Fill( commands, area, ColourOr( tabs, EWidgetProperty_BackgroundColour, CColour( 0, 0, 0, 0 ) ) );
    const CTabControl::TStringVector titles = tabs.GetStringListProperty( EWidgetProperty_Tabs );
    const CString& font = tabs.GetPropertyRef( EWidgetProperty_Font );
    const bool ellipsis = "Ellipsis" == tabs.GetPropertyRef( EWidgetProperty_TextOverflow );
    const CColour textColour = TextColourOf( tabs );
    for ( UInt32 i=0; i<(UInt32)titles.size(); ++i )
    {
        const CRect header = tabs.GetTabRect( i );
        if ( header.IsEmpty() )
            continue;           // not shown: the tabs overflow
        const bool active = (Int32) i == tabs.GetActiveTab();
        Fill( commands, header, ColourOr( tabs, active ? EWidgetProperty_ActiveTabColour : EWidgetProperty_TabColour, BoxColour ) );
        Stroke( commands, header, BoxBorderColour, 1 );

        // the title in the room left of the badge; a title wider than its tab: cut with "..." within the padding of the tab (the tooltip has the whole title)
        const Int32 badgeRoom = tabs.GetTabBadgeRoom( i );
        bool cut = false;
        const CString shown = context.GetDisplayText( titles[ i ] );
        const CString title = ellipsis ? CWidgetPainter::FitText( context, font, shown, tabs.GetTabTextRoom( i ), cut ) : shown;
        const CRect textArea( header.left, header.top, header.width - badgeRoom > 0 ? header.width - badgeRoom : 0, header.height );
        CWidgetPainter::PaintText( context, font, title, badgeRoom > 0 ? textArea : header, "Center", "Center", false, textColour, commands,
                                   TEXTOVERFLOW_CLIP, cut ? CDrawSurface::TEXTFLAG_ELIDED : CDrawSurface::TEXTFLAG_NONE, false );

        // the badge: a dot, or a pill with its text
        if ( badgeRoom > 0 )
        {
            const CRect badge = tabs.GetTabBadgeRect( i );
            Fill( commands, badge, BadgeColour );
            const CString badgeText = tabs.GetTabBadge( i );
            if ( !( badgeText == "*" ) )
                CWidgetPainter::PaintText( context, font, badgeText, badge, "Center", "Center", false, BadgeTextColour, commands );
        }
    }

    // the overflow button: the menu of every tab
    const CRect button = tabs.GetOverflowButtonRect();
    if ( !button.IsEmpty() )
    {
        Fill( commands, button, ColourOr( tabs, EWidgetProperty_TabColour, BoxColour ) );
        Stroke( commands, button, BoxBorderColour, 1 );
        MenuSign( commands, button, textColour );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
