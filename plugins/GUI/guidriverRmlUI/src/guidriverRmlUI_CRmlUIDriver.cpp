/*
 *  guidriverRmlUI: GUI backend using RmlUi
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

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementInstancer.h>
#include <RmlUi/Core/Factory.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_guidriverRmlUI_CRmlUISYSTEMINTERFACE_H
#include "guidriverRmlUI_CRmlUISystemInterface.h"
#define GUCEF_guidriverRmlUI_CRmlUISYSTEMINTERFACE_H
#endif /* GUCEF_guidriverRmlUI_CRmlUISYSTEMINTERFACE_H ? */

#ifndef GUCEF_guidriverRmlUI_CRmlUIVFSINTERFACE_H
#include "guidriverRmlUI_CRmlUIVFSInterface.h"
#define GUCEF_guidriverRmlUI_CRmlUIVFSINTERFACE_H
#endif /* GUCEF_guidriverRmlUI_CRmlUIVFSINTERFACE_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#include "guidriverRmlUI_CRmlUIWidgetElement.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H
#include "guidriverRmlUI_CRmlUIWidgetHandler.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H ? */

#include "guidriverRmlUI_CRmlUIDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CRmlUIDriver::DriverName = "RmlUi";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** RmlUi is one per process: started by the first driver, shut down after the last */
UInt32 g_rmlUiUsers = 0;
bool g_rmlUiStarted = false;
CRmlUISystemInterface* g_systemInterface = GUCEF_NULL;
CRmlUIVFSInterface* g_fileInterface = GUCEF_NULL;
Rml::ElementInstancerGeneric< CRmlUIWidgetElement >* g_widgetElementInstancer = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

bool
StartRmlUi( void )
{
    ++g_rmlUiUsers;
    if ( g_rmlUiStarted )
        return true;

    g_systemInterface = GUCEF_NEW CRmlUISystemInterface();
    g_fileInterface = GUCEF_NEW CRmlUIVFSInterface();
    Rml::SetSystemInterface( g_systemInterface );
    Rml::SetFileInterface( g_fileInterface );
    if ( !Rml::Initialise() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RmlUIDriver: RmlUi could not be started" );
        return false;
    }
    g_widgetElementInstancer = GUCEF_NEW Rml::ElementInstancerGeneric< CRmlUIWidgetElement >();
    Rml::Factory::RegisterElementInstancer( CRmlUIWidgetElement::TagName, g_widgetElementInstancer );
    g_rmlUiStarted = true;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIDriver: Started RmlUi " << Rml::GetVersion().c_str() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
StopRmlUi( void )
{
    if ( g_rmlUiUsers > 0 )
        --g_rmlUiUsers;
    if ( g_rmlUiUsers > 0 || !g_rmlUiStarted )
        return;

    Rml::Shutdown();
    g_rmlUiStarted = false;
    delete g_widgetElementInstancer;
    g_widgetElementInstancer = GUCEF_NULL;
    delete g_fileInterface;
    g_fileInterface = GUCEF_NULL;
    delete g_systemInterface;
    g_systemInterface = GUCEF_NULL;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIDriver::CRmlUIDriver( void )
    : GUI::CGUIDriver()
    , m_handlers()
    , m_contexts()
    , m_fonts()
    , m_rmlUiReady( false )
{GUCEF_TRACE;

    m_rmlUiReady = StartRmlUi();
}

/*-------------------------------------------------------------------------*/

CRmlUIDriver::~CRmlUIDriver()
{GUCEF_TRACE;

    // the contexts must go before RmlUi does: their owners should have let go of them by now
    if ( GetContextCount() > 0 )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RmlUIDriver: " << GetContextCount() << " context(s) still exist while the driver goes" );
    m_contexts.clear();
    for ( THandlerMap::iterator i=m_handlers.begin(); i!=m_handlers.end(); ++i )
        delete (*i).second;
    m_handlers.clear();
    StopRmlUi();
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIDriver::IsRmlUiReady( void ) const
{GUCEF_TRACE;

    return m_rmlUiReady;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIDriver::AddContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_contexts.insert( TGuiContextWeakPtr( context ) );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIDriver::RemoveContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_contexts.erase( TGuiContextWeakPtr( context ) );
}

/*-------------------------------------------------------------------------*/

CRmlUIDriver::TGUIContextSet
CRmlUIDriver::GetContextList( void )
{GUCEF_TRACE;

    // the contexts that still exist, the others are forgotten
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

UInt32
CRmlUIDriver::GetContextCount( void )
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TWeakContextSet::const_iterator i=m_contexts.begin(); i!=m_contexts.end(); ++i )
        if ( !(*i).IsExpired() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CString
CRmlUIDriver::GetDriverName( void )
{GUCEF_TRACE;

    return DriverName;
}

/*-------------------------------------------------------------------------*/

CRmlUIDriver::TStringSet
CRmlUIDriver::GetAvailableFormTypes( void )
{GUCEF_TRACE;

    TStringSet types;
    types.insert( "Form" );
    types.insert( "FormEx" );
    return types;
}

/*-------------------------------------------------------------------------*/

CRmlUIDriver::TStringSet
CRmlUIDriver::GetAvailableWidgetTypes( void )
{GUCEF_TRACE;

    return GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetTypeNames();
}

/*-------------------------------------------------------------------------*/

CRmlUIDriver::TStringSet
CRmlUIDriver::GetAvailableFonts( void )
{GUCEF_TRACE;

    return m_fonts;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIDriver::LoadFontFromAsset( const CString& assetPath )
{GUCEF_TRACE;

    if ( !m_rmlUiReady || !Rml::LoadFontFace( assetPath.C_String() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIDriver: Could not load the font " << assetPath );
        return false;
    }
    m_fonts.insert( assetPath );
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CRmlUIDriver::GetDriverProperty( const CString& propertyName ) const
{GUCEF_TRACE;

    if ( "RmlUiVersion" == propertyName )
        return Rml::GetVersion().c_str();
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

GUI::CWidgetHandler*
CRmlUIDriver::GetWidgetHandler( const CString& widgetType )
{GUCEF_TRACE;

    if ( !GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().IsRegistered( widgetType ) )
        return GUCEF_NULL;
    THandlerMap::iterator i = m_handlers.find( widgetType );
    if ( i != m_handlers.end() )
        return (*i).second;
    CRmlUIWidgetHandler* handler = GUCEF_NEW CRmlUIWidgetHandler( widgetType );
    m_handlers[ widgetType ] = handler;
    return handler;
}

/*-------------------------------------------------------------------------*/

CRmlUIDriver::TStringSet
CRmlUIDriver::GetCapabilities( void )
{GUCEF_TRACE;

    TStringSet capabilities;
    capabilities.insert( "BitmapFonts" );
    capabilities.insert( "DrawSurface" );
    capabilities.insert( "DynamicImages" );
    capabilities.insert( "NativeLayouts" );
    capabilities.insert( "BackendHints" );
    // the grid features the own painter of gucefGUI draws
    GUI::CGridView::GetCapabilities( capabilities );
    return capabilities;
}

/*-------------------------------------------------------------------------*/

UInt32
CRmlUIDriver::GetInterfaceVersion( void ) const
{GUCEF_TRACE;

    return GUI::CGUIDriver::InterfaceVersion;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIDriver::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIDriver::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CRmlUIDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::guidriverRmlUI::CRmlUIDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
