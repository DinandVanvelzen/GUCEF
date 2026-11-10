/*
 *  guidriverNull: GUCEF GUI driver that keeps the widgets but draws nothing, for tests and tools
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

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CSOFTWARERENDERER_H
#include "gucefGUI_CSoftwareRenderer.h"
#define GUCEF_GUI_CSOFTWARERENDERER_H
#endif /* GUCEF_GUI_CSOFTWARERENDERER_H ? */

#include "guidriverNull_CNullGUIDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERNULL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNullGUIContext::CNullGUIContext( GUI::CGUIDriver* driver, GUI::TWindowContextPtr windowContext )
    : GUI::CGUIContext( driver, windowContext )
    , m_renderCount( 0 )
    , m_rootOrderChanges( 0 )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CNullGUIContext::~CNullGUIContext()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CNullGUIContext::GetRenderCount( void ) const
{GUCEF_TRACE;

    return m_renderCount;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CNullGUIContext::GetRootOrderChangeCount( void ) const
{GUCEF_TRACE;

    return m_rootOrderChanges;
}

/*-------------------------------------------------------------------------*/

bool
CNullGUIContext::CaptureFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    return GUI::CSoftwareRenderer::RenderContext( *this, pixels );
}

/*-------------------------------------------------------------------------*/

void
CNullGUIContext::OnRender( void )
{GUCEF_TRACE;

    ++m_renderCount;
}

/*-------------------------------------------------------------------------*/

void
CNullGUIContext::OnRootOrderChanged( void )
{GUCEF_TRACE;

    ++m_rootOrderChanges;
}

/*-------------------------------------------------------------------------*/

CNullGUIDriver::CNullGUIDriver( void )
    : GUI::CGUIDriver()
    , m_handlers()
    , m_contexts()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CNullGUIDriver::~CNullGUIDriver()
{GUCEF_TRACE;

    m_contexts.clear();
    for ( THandlerMap::iterator i=m_handlers.begin(); i!=m_handlers.end(); ++i )
        delete (*i).second;
    m_handlers.clear();
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CNullGUIDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    GUI::TGuiContextPtr context( GUCEF_NEW CNullGUIContext( this, windowContext ) );
    m_contexts.insert( TGuiContextWeakPtr( context ) );
    return context;
}

/*-------------------------------------------------------------------------*/

CNullGUIDriver::TGUIContextSet
CNullGUIDriver::GetContextList( void )
{GUCEF_TRACE;

    TGUIContextSet contexts;
    TWeakContextSet::iterator i = m_contexts.begin();
    while ( i != m_contexts.end() )
    {
        GUI::TGuiContextPtr context = (*i).GetSharedPtr();
        if ( context.IsNULL() )
        {
            m_contexts.erase( i++ );
            continue;
        }
        contexts.insert( context );
        ++i;
    }
    return contexts;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CNullGUIDriver::GetContextCount( void )
{GUCEF_TRACE;

    CORE::UInt32 count = 0;
    for ( TWeakContextSet::const_iterator i=m_contexts.begin(); i!=m_contexts.end(); ++i )
        if ( !(*i).IsExpired() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNullGUIDriver::GetDriverName( void )
{GUCEF_TRACE;

    return "Null";
}

/*-------------------------------------------------------------------------*/

CNullGUIDriver::TStringSet
CNullGUIDriver::GetAvailableFormTypes( void )
{GUCEF_TRACE;

    TStringSet types;
    types.insert( "Form" );
    types.insert( "FormEx" );
    return types;
}

/*-------------------------------------------------------------------------*/

CNullGUIDriver::TStringSet
CNullGUIDriver::GetAvailableWidgetTypes( void )
{GUCEF_TRACE;

    return GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetTypeNames();
}

/*-------------------------------------------------------------------------*/

CNullGUIDriver::TStringSet
CNullGUIDriver::GetAvailableFonts( void )
{GUCEF_TRACE;

    return TStringSet();
}

/*-------------------------------------------------------------------------*/

bool
CNullGUIDriver::LoadFontFromAsset( const CORE::CString& assetPath )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNullGUIDriver::GetDriverProperty( const CORE::CString& propertyName ) const
{GUCEF_TRACE;

    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

GUI::CWidgetHandler*
CNullGUIDriver::GetWidgetHandler( const CORE::CString& widgetType )
{GUCEF_TRACE;

    if ( !GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().IsRegistered( widgetType ) )
        return GUCEF_NULL;
    THandlerMap::iterator i = m_handlers.find( widgetType );
    if ( i != m_handlers.end() )
        return (*i).second;
    CNullWidgetHandler* handler = GUCEF_NEW CNullWidgetHandler( widgetType );
    m_handlers[ widgetType ] = handler;
    return handler;
}

/*-------------------------------------------------------------------------*/

CNullGUIDriver::TStringSet
CNullGUIDriver::GetCapabilities( void )
{GUCEF_TRACE;

    // it keeps everything and shows nothing; a capture draws on the CPU
    TStringSet capabilities;
    capabilities.insert( "CaptureFrame" );
    capabilities.insert( "BitmapFonts" );
    capabilities.insert( "DrawSurface" );
    capabilities.insert( "DynamicImages" );
    // the grid features the own painter of gucefGUI draws
    GUI::CGridView::GetCapabilities( capabilities );
    return capabilities;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CNullGUIDriver::GetInterfaceVersion( void ) const
{GUCEF_TRACE;

    return GUI::CGUIDriver::InterfaceVersion;
}

/*-------------------------------------------------------------------------*/

bool
CNullGUIDriver::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNullGUIDriver::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CNullGUIDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVERNULL::CNullGUIDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERNULL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
