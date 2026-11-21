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

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#include "gucefGUI_CTooltipService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTooltipService::CTooltipService( void )
    : m_enabled( true )
    , m_showDelayInMs( (UInt32) DefaultShowDelayInMs )
    , m_maxWidth( (Int32) DefaultMaxWidth )
    , m_background( 250, 250, 210, 255 )
    , m_border( 60, 60, 60, 255 )
    , m_text( 0, 0, 0, 255 )
    , m_helpSource()
    , m_moreHint()
    , m_textProvider( GUCEF_NULL )
    , m_restStartInMs( 0 )
    , m_suppressed( false )
    , m_showing( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTooltipService::SetEnabled( bool enabled )
{GUCEF_TRACE;

    m_enabled = enabled;
    if ( !enabled )
        m_showing = false;
}

/*-------------------------------------------------------------------------*/

bool
CTooltipService::IsEnabled( void ) const
{GUCEF_TRACE;

    return m_enabled;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::SetShowDelayInMs( UInt32 delayInMs )
{GUCEF_TRACE;

    m_showDelayInMs = delayInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CTooltipService::GetShowDelayInMs( void ) const
{GUCEF_TRACE;

    return m_showDelayInMs;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::SetMaxWidth( Int32 maxWidth )
{GUCEF_TRACE;

    m_maxWidth = maxWidth > 2 * (Int32) DefaultPaddingX ? maxWidth : (Int32) DefaultMaxWidth;
}

/*-------------------------------------------------------------------------*/

Int32
CTooltipService::GetMaxWidth( void ) const
{GUCEF_TRACE;

    return m_maxWidth;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::SetColours( const CColour& background, const CColour& border, const CColour& text )
{GUCEF_TRACE;

    m_background = background;
    m_border = border;
    m_text = text;
}

/*-------------------------------------------------------------------------*/

const CColour&
CTooltipService::GetBackgroundColour( void ) const
{GUCEF_TRACE;

    return m_background;
}

/*-------------------------------------------------------------------------*/

const CColour&
CTooltipService::GetBorderColour( void ) const
{GUCEF_TRACE;

    return m_border;
}

/*-------------------------------------------------------------------------*/

const CColour&
CTooltipService::GetTextColour( void ) const
{GUCEF_TRACE;

    return m_text;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::SetHelpSource( const THelpSourcePtr& source )
{GUCEF_TRACE;

    m_helpSource = source;
}

/*-------------------------------------------------------------------------*/

const THelpSourcePtr&
CTooltipService::GetHelpSource( void ) const
{GUCEF_TRACE;

    return m_helpSource;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::SetMoreHint( const CString& hint )
{GUCEF_TRACE;

    m_moreHint = hint;
}

/*-------------------------------------------------------------------------*/

const CString&
CTooltipService::GetMoreHint( void ) const
{GUCEF_TRACE;

    return m_moreHint;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::OnPointerMoved( UInt64 nowInMs )
{GUCEF_TRACE;

    if ( !m_showing )
        m_restStartInMs = nowInMs;
    m_suppressed = false;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::OnHoverChanged( UInt64 nowInMs )
{GUCEF_TRACE;

    m_restStartInMs = nowInMs;
    m_showing = false;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::OnInputPressed( void )
{GUCEF_TRACE;

    m_suppressed = true;
    m_showing = false;
}

/*-------------------------------------------------------------------------*/

bool
CTooltipService::IsDue( UInt64 nowInMs ) const
{GUCEF_TRACE;

    if ( !m_enabled || m_suppressed || nowInMs < m_restStartInMs )
        return false;
    return nowInMs - m_restStartInMs >= (UInt64) m_showDelayInMs;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::SetShowing( bool showing )
{GUCEF_TRACE;

    m_showing = showing;
}

/*-------------------------------------------------------------------------*/

bool
CTooltipService::IsShowing( void ) const
{GUCEF_TRACE;

    return m_showing;
}

/*-------------------------------------------------------------------------*/

UInt64
CTooltipService::GetRestStartInMs( void ) const
{GUCEF_TRACE;

    return m_restStartInMs;
}

/*-------------------------------------------------------------------------*/

CString
CTooltipService::FindHelpTopicAt( const CWidget& widget, Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    const CString topic = widget.GetHelpTopicAt( localX, localY );
    if ( !topic.IsNULLOrEmpty() )
        return topic;
    for ( const CWidget* ancestor = widget.GetParentWidget(); GUCEF_NULL != ancestor; ancestor = ancestor->GetParentWidget() )
    {
        const CString& property = ancestor->GetPropertyRef( EWidgetProperty_HelpTopic );
        if ( !property.IsNULLOrEmpty() )
            return property;
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CTooltipService::GetTooltipText( const CWidget& widget, Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    CString text = widget.GetTooltipAt( localX, localY );
    if ( text.IsNULLOrEmpty() && GUCEF_NULL != m_textProvider )
        text = m_textProvider->GetTooltipFor( widget, localX, localY );

    // a text the widget shows shortened ("...") is shown whole: as the tooltip, or above the tooltip the widget has
    CGUIContext* context = widget.GetContext();
    CString fullText;
    if ( GUCEF_NULL != context && CWidgetPainter::IsOwnTextElided( widget, *context, fullText ) && text.HasSubstr( fullText ) < 0 )
        text = text.IsNULLOrEmpty() ? fullText : fullText + "\n" + text;

    if ( m_helpSource.IsNULL() )
        return text;
    const CString topic = FindHelpTopicAt( widget, localX, localY );
    if ( topic.IsNULLOrEmpty() )
        return text;
    CString brief, moreTopic;
    if ( !m_helpSource->GetBrief( topic, brief, moreTopic ) )
        return text;
    if ( text.IsNULLOrEmpty() )
        text = brief;
    if ( !text.IsNULLOrEmpty() && !m_moreHint.IsNULLOrEmpty() && !moreTopic.IsNULLOrEmpty() && m_helpSource->HasTopic( moreTopic ) )
        text += "\n" + m_moreHint;
    return text;
}

/*-------------------------------------------------------------------------*/

void
CTooltipService::SetTextProvider( const CITooltipTextProvider* provider )
{GUCEF_TRACE;

    m_textProvider = provider;
}

/*-------------------------------------------------------------------------*/

const CITooltipTextProvider*
CTooltipService::GetTextProvider( void ) const
{GUCEF_TRACE;

    return m_textProvider;
}

/*-------------------------------------------------------------------------*/

CString
CTooltipService::ResolveTopic( const CString& helpId ) const
{GUCEF_TRACE;

    if ( m_helpSource.IsNULL() || helpId.IsNULLOrEmpty() )
        return CString::Empty;
    if ( m_helpSource->HasTopic( helpId ) )
        return helpId;
    CString brief, moreTopic;
    if ( m_helpSource->GetBrief( helpId, brief, moreTopic ) && !moreTopic.IsNULLOrEmpty() && m_helpSource->HasTopic( moreTopic ) )
        return moreTopic;
    return CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
