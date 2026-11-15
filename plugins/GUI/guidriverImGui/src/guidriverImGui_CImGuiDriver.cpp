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

#include "imgui.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#include "gucefGUI_CFontRasterizerRegistry.h"
#define GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#endif /* GUCEF_GUI_CFONTRASTERIZERREGISTRY_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#include "guidriverImGui_CImGuiGlobal.h"
#define GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#endif /* GUIDRIVERIMGUI_CIMGUIGLOBAL_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXT_H
#include "guidriverImGui_CImGuiContext.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXT_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXT_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H
#include "guidriverImGui_CImGuiWidgetHandler.h"
#define GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H
#endif /* GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H ? */

#include "guidriverImGui_CImGuiDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CImGuiDriver::DriverName = "ImGui";
const char* CImGuiDriver::ConfigSectionName = "ImGui";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiDriver::SSettings::SSettings( void )
    : fontPath( GUI::CGuiGlobal::DefaultUiFontPath )
    , fontSize( 16.0f )
    , uiScale( 1.0f )
    , style( "Dark" )
    , keyboardNavigation( true )
    , softwareCursor( false )
    #ifdef GUCEF_DEBUG_MODE
    , configErrorRecoveryAsserts( true )
    #else
    , configErrorRecoveryAsserts( false )
    #endif
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiDriver::CImGuiDriver( void )
    : GUI::CGUIDriver()
    , m_handlers()
    , m_contexts()
    , m_fonts()
    , m_settings()
{GUCEF_TRACE;

    CImGuiContext::RegisterEvents();
    CImGuiGlobal::Instance()->RegisterDriver();
}

/*-------------------------------------------------------------------------*/

CImGuiDriver::~CImGuiDriver()
{GUCEF_TRACE;

    // the contexts must go before the driver: their owners should have let go of them by now
    if ( GetContextCount() > 0 )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ImGui: " << GetContextCount() << " context(s) still exist while the driver goes" );
    m_contexts.clear();
    for ( THandlerMap::iterator i=m_handlers.begin(); i!=m_handlers.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_handlers.clear();
    CImGuiGlobal::Instance()->UnregisterDriver();
}

/*-------------------------------------------------------------------------*/

const CImGuiDriver::TSettings&
CImGuiDriver::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CImGuiDriver::SetSettings( const TSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiDriver::CanCreateContext( void ) const
{GUCEF_TRACE;

    if ( !CImGuiGlobal::Instance()->CheckImGuiThread( "an ImGui context" ) )
        return false;
    if ( !GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry().HasRasterizers() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ImGui: No font rasterizer is registered, an ImGui context needs one: load the plugin fontpluginFREETYPE" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CImGuiDriver::AddContext( CImGuiContext* context )
{GUCEF_TRACE;

    if ( GUCEF_NULL == context )
        return GUI::TGuiContextPtr();
    if ( !context->IsReady() )
    {
        GUCEF_DELETE context;
        return GUI::TGuiContextPtr();
    }
    GUI::TGuiContextPtr contextPtr( context );
    m_contexts.insert( TGuiContextWeakPtr( contextPtr ) );
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

CImGuiDriver::TGUIContextSet
CImGuiDriver::GetContextList( void )
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
CImGuiDriver::GetContextCount( void )
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TWeakContextSet::const_iterator i=m_contexts.begin(); i!=m_contexts.end(); ++i )
        if ( !(*i).IsExpired() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CString
CImGuiDriver::GetDriverName( void )
{GUCEF_TRACE;

    return DriverName;
}

/*-------------------------------------------------------------------------*/

CImGuiDriver::TStringSet
CImGuiDriver::GetAvailableFormTypes( void )
{GUCEF_TRACE;

    TStringSet types;
    types.insert( "Form" );
    types.insert( "FormEx" );
    return types;
}

/*-------------------------------------------------------------------------*/

CImGuiDriver::TStringSet
CImGuiDriver::GetAvailableWidgetTypes( void )
{GUCEF_TRACE;

    return GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetTypeNames();
}

/*-------------------------------------------------------------------------*/

CImGuiDriver::TStringSet
CImGuiDriver::GetAvailableFonts( void )
{GUCEF_TRACE;

    return m_fonts;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiDriver::LoadFontFromAsset( const CString& assetPath )
{GUCEF_TRACE;

    if ( assetPath.IsNULLOrEmpty() )
        return false;
    m_fonts.insert( assetPath );
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CImGuiDriver::GetDriverProperty( const CString& propertyName ) const
{GUCEF_TRACE;

    if ( "ImGuiVersion" == propertyName )
        return IMGUI_VERSION;
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

GUI::CWidgetHandler*
CImGuiDriver::GetWidgetHandler( const CString& widgetType )
{GUCEF_TRACE;

    if ( !GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().IsRegistered( widgetType ) )
        return GUCEF_NULL;
    THandlerMap::iterator i = m_handlers.find( widgetType );
    if ( i != m_handlers.end() )
        return (*i).second;
    CImGuiWidgetHandler* handler = GUCEF_NEW CImGuiWidgetHandler( widgetType );
    m_handlers[ widgetType ] = handler;
    return handler;
}

/*-------------------------------------------------------------------------*/

CImGuiDriver::TStringSet
CImGuiDriver::GetCapabilities( void )
{GUCEF_TRACE;

    TStringSet capabilities;
    capabilities.insert( "Render" );
    capabilities.insert( "CaptureFrame" );
    capabilities.insert( "BitmapFonts" );
    capabilities.insert( "VectorFonts" );
    capabilities.insert( "DrawSurface" );
    capabilities.insert( "DynamicImages" );
    capabilities.insert( "PresentScaling" );
    capabilities.insert( "ImmediateMode" );
    // the grid features the own painter of gucefGUI draws
    GUI::CGridView::GetCapabilities( capabilities );
    return capabilities;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiDriver::GetInterfaceVersion( void ) const
{GUCEF_TRACE;

    return GUI::CGUIDriver::InterfaceVersion;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiDriver::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;
    section->SetAttribute( "fontPath", m_settings.fontPath );
    section->SetAttribute( "fontSize", CORE::FloatToString( m_settings.fontSize ) );
    section->SetAttribute( "uiScale", CORE::FloatToString( m_settings.uiScale ) );
    section->SetAttribute( "style", m_settings.style );
    section->SetAttribute( "keyboardNavigation", CORE::BoolToString( m_settings.keyboardNavigation ) );
    section->SetAttribute( "softwareCursor", CORE::BoolToString( m_settings.softwareCursor ) );
    section->SetAttribute( "configErrorRecoveryAsserts", CORE::BoolToString( m_settings.configErrorRecoveryAsserts ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiDriver::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return true;

    m_settings.fontPath = section->GetAttributeValueOrChildValueByName( "fontPath" ).AsString( m_settings.fontPath, true );
    m_settings.fontSize = section->GetAttributeValueOrChildValueByName( "fontSize" ).AsFloat32( m_settings.fontSize, true );
    m_settings.uiScale = section->GetAttributeValueOrChildValueByName( "uiScale" ).AsFloat32( m_settings.uiScale, true );
    m_settings.style = section->GetAttributeValueOrChildValueByName( "style" ).AsString( m_settings.style, true );
    m_settings.keyboardNavigation = section->GetAttributeValueOrChildValueByName( "keyboardNavigation" ).AsBool( m_settings.keyboardNavigation, true );
    m_settings.softwareCursor = section->GetAttributeValueOrChildValueByName( "softwareCursor" ).AsBool( m_settings.softwareCursor, true );
    m_settings.configErrorRecoveryAsserts = section->GetAttributeValueOrChildValueByName( "configErrorRecoveryAsserts" ).AsBool( m_settings.configErrorRecoveryAsserts, true );
    if ( m_settings.fontSize <= 0.0f )
        m_settings.fontSize = 16.0f;
    if ( m_settings.uiScale <= 0.0f )
        m_settings.uiScale = 1.0f;
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CImGuiDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUIDRIVERIMGUI::CImGuiDriver";
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
