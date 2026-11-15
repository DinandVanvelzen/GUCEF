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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#include "guidriverImGui_CImGuiGlobal.h"
#define GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#endif /* GUIDRIVERIMGUI_CIMGUIGLOBAL_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXT_H
#include "guidriverImGui_CImGuiContext.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXT_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXT_H ? */

#include "guidriverImGui_CImGuiFrameBuilder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiFrameBuilder::CImGuiFrameBuilder( void )
    : CORE::CObservingNotifier()
    , m_context()
    , m_imguiContext( GUCEF_NULL )
    , m_visible( true )
    , m_builds( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiFrameBuilder::~CImGuiFrameBuilder()
{GUCEF_TRACE;

    // not called again from here on, also while the base classes go
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiFrameBuilder::Attach( GUI::TGuiContextPtr context )
{GUCEF_TRACE;

    Detach();
    if ( context.IsNULL() || !CImGuiGlobal::HasInstance() )
        return false;
    CImGuiContext* imguiContext = CImGuiGlobal::Instance()->FindContext( context.GetPointerAlways() );
    if ( GUCEF_NULL == imguiContext )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: A frame builder can only be attached to an ImGui context" );
        return false;
    }
    m_context = TGuiContextWeakPtr( context );
    m_imguiContext = imguiContext;
    TEventCallback callback( this, &CImGuiFrameBuilder::OnBuildFrameEvent );
    SubscribeTo( imguiContext, CImGuiContext::BuildFrameEvent, callback );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiFrameBuilder::Detach( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_imguiContext && !m_context.IsExpired() )
        UnsubscribeFrom( m_imguiContext );
    m_imguiContext = GUCEF_NULL;
    m_context = TGuiContextWeakPtr();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiFrameBuilder::IsAttached( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_imguiContext && !m_context.IsExpired();
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CImGuiFrameBuilder::GetContext( void ) const
{GUCEF_TRACE;

    return m_context.GetSharedPtr();
}

/*-------------------------------------------------------------------------*/

void
CImGuiFrameBuilder::SetVisible( bool visible )
{GUCEF_TRACE;

    m_visible = visible;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiFrameBuilder::IsVisible( void ) const
{GUCEF_TRACE;

    return m_visible;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiFrameBuilder::GetBuildCount( void ) const
{GUCEF_TRACE;

    return m_builds;
}

/*-------------------------------------------------------------------------*/

void
CImGuiFrameBuilder::OnBuildFrameEvent( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_visible || GUCEF_NULL == m_imguiContext || notifier != m_imguiContext )
        return;
    UInt32 delta = 0;
    CORE::TCloneableUInt32* frameTime = static_cast< CORE::TCloneableUInt32* >( eventData );
    if ( GUCEF_NULL != frameTime )
        delta = frameTime->GetData();
    ++m_builds;
    OnBuildFrame( *m_imguiContext, delta );
}

/*-------------------------------------------------------------------------*/

const CString&
CImGuiFrameBuilder::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUIDRIVERIMGUI::CImGuiFrameBuilder";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
