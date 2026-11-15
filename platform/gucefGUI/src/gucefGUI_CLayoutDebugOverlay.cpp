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

#include <stdlib.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CLayoutDebugOverlay.h"

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

const CColour ErrorColour( 230, 40, 40, 255 );
const CColour WarningColour( 240, 170, 30, 255 );
const CColour ErrorBand( 230, 40, 40, 90 );
const CColour LabelBackground( 0, 0, 0, 200 );

void
AddFill( CDrawSurface::TCommandVector& commands, const CRect& rect, const CColour& colour )
{
    if ( rect.IsEmpty() )
        return;
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_FILLRECT;
    command.rect = rect;
    command.colour = colour;
    commands.push_back( command );
}

void
AddStroke( CDrawSurface::TCommandVector& commands, const CRect& rect, const CColour& colour, Int32 thickness )
{
    if ( rect.IsEmpty() )
        return;
    CDrawSurface::TCommand command;
    command.type = CDrawSurface::COMMAND_STROKERECT;
    command.rect = rect;
    command.colour = colour;
    command.thickness = thickness;
    commands.push_back( command );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLayoutDebugOverlay::CLayoutDebugOverlay( void )
    : m_mode( MODE_OFF )
    , m_lint()
    , m_options()
    , m_findings()
    , m_dirty( true )
    , m_lintedRevision( 0 )
    , m_lintedTime( 0 )
    , m_loggedReport()
{GUCEF_TRACE;

    m_options.layoutFirst = false;
}

/*-------------------------------------------------------------------------*/

void
CLayoutDebugOverlay::SetMode( UInt32 mode )
{GUCEF_TRACE;

    m_mode = mode & ( MODE_OVERLAY | MODE_LOG );
    m_dirty = true;
    if ( MODE_OFF == m_mode )
    {
        m_findings.clear();
        m_loggedReport.Clear();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutDebugOverlay::GetMode( void ) const
{GUCEF_TRACE;

    return m_mode;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutDebugOverlay::IsOn( void ) const
{GUCEF_TRACE;

    return MODE_OFF != m_mode;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutDebugOverlay::ParseMode( const CString& text, UInt32& mode )
{GUCEF_TRACE;

    UInt32 parsed = MODE_OFF;
    CString::StringVector words = text.Lowercase().ReplaceChar( ' ', ',' ).ReplaceChar( '+', ',' ).ParseElements( ',', false );
    if ( words.empty() )
        return false;
    for ( UInt32 i=0; i<(UInt32) words.size(); ++i )
    {
        const CString& word = words[ i ];
        if ( "off" == word || "0" == word || "false" == word )
            parsed = MODE_OFF;
        else
        if ( "on" == word || "1" == word || "true" == word || "overlay" == word )
            parsed |= MODE_OVERLAY;
        else
        if ( "log" == word )
            parsed |= MODE_LOG;
        else
        if ( "all" == word || "both" == word )
            parsed |= MODE_OVERLAY | MODE_LOG;
        else
            return false;
    }
    mode = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutDebugOverlay::ApplyEnvironment( void )
{GUCEF_TRACE;

    const char* value = getenv( "GUCEF_GUI_LAYOUT_LINT" );
    if ( GUCEF_NULL == value || '\0' == *value )
        return false;
    UInt32 mode = MODE_OFF;
    if ( !ParseMode( value, mode ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CLayoutDebugOverlay: GUCEF_GUI_LAYOUT_LINT=\"" << value << "\" is not a mode (overlay, log, overlay,log, off)" );
        return false;
    }
    SetMode( mode );
    return true;
}

/*-------------------------------------------------------------------------*/

CLayoutLintOptions&
CLayoutDebugOverlay::GetOptions( void )
{GUCEF_TRACE;

    m_dirty = true;
    return m_options;
}

/*-------------------------------------------------------------------------*/

const TLayoutLintFindingVector&
CLayoutDebugOverlay::GetFindings( void ) const
{GUCEF_TRACE;

    return m_findings;
}

/*-------------------------------------------------------------------------*/

void
CLayoutDebugOverlay::Invalidate( void )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CLayoutDebugOverlay::Relint( CGUIContext& context )
{GUCEF_TRACE;

    m_lint.Run( context, m_options, m_findings );
    m_dirty = false;
    m_lintedRevision = context.GetLayoutRevision();
    m_lintedTime = context.GetTimeInMs();

    if ( 0 != ( m_mode & MODE_LOG ) )
    {
        // once per change of what was found, never per frame
        const CString report = CLayoutLint::Report( m_findings, 50 );
        if ( report != m_loggedReport )
        {
            m_loggedReport = report;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Layout lint: " << report );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLayoutDebugOverlay::Paint( CGUIContext& context, CDrawSurface::TCommandVector& commands )
{GUCEF_TRACE;

    if ( MODE_OFF == m_mode )
        return;
    if ( m_dirty || m_lintedRevision != context.GetLayoutRevision() || context.GetTimeInMs() >= m_lintedTime + (UInt64) RelintIntervalInMs )
        Relint( context );
    if ( 0 == ( m_mode & MODE_OVERLAY ) )
        return;

    for ( UInt32 i=0; i<(UInt32) m_findings.size(); ++i )
    {
        const CLayoutLintFinding& finding = m_findings[ i ];
        if ( LAYOUTLINTSEVERITY_INFO == finding.severity )
            continue;
        const CColour& colour = LAYOUTLINTSEVERITY_ERROR == finding.severity ? ErrorColour : WarningColour;
        AddStroke( commands, finding.rect, colour, 2 );

        // the part of a cut text outside its room, as a band
        if ( ( LAYOUTLINTRULE_TEXTCLIPPED == finding.rule || LAYOUTLINTRULE_NUMBERELIDED == finding.rule ) && !finding.otherRect.IsEmpty() )
        {
            const CRect& run = finding.rect;
            const CRect& room = finding.otherRect;
            if ( run.Right() > room.Right() )
                AddFill( commands, CRect( room.Right(), run.top, run.Right() - room.Right(), run.height ), ErrorBand );
            if ( run.left < room.left )
                AddFill( commands, CRect( run.left, run.top, room.left - run.left, run.height ), ErrorBand );
            if ( run.Bottom() > room.Bottom() )
                AddFill( commands, CRect( run.left, room.Bottom(), run.width, run.Bottom() - room.Bottom() ), ErrorBand );
            if ( run.top < room.top )
                AddFill( commands, CRect( run.left, run.top, run.width, room.top - run.top ), ErrorBand );
        }

        // the rule id at the top left of the outline
        const CString& id = LayoutLintRuleToId( finding.rule );
        Int32 width = 0, height = 0;
        if ( context.MeasureText( finding.font, id, 0, width, height ) && width > 0 )
        {
            const CRect label( finding.rect.left, finding.rect.top, width + 4, context.GetLineHeight( finding.font ) );
            AddFill( commands, label, LabelBackground );
            CDrawSurface::TCommand text;
            text.type = CDrawSurface::COMMAND_TEXT;
            text.font = finding.font;
            text.text = id;
            text.x0 = label.left + 2;
            text.y0 = label.top;
            text.colour = colour;
            commands.push_back( text );
        }
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
