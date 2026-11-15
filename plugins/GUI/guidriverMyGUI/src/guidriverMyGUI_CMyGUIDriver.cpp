/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#include "MyGUI_LogManager.h"
#include "MyGUI_LogSource.h"

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

#ifndef GUCEF_guidriverMyGUI_CMYGUIDATAMANAGER_H
#include "guidriverMyGUI_CMyGUIDataManager.h"
#define GUCEF_guidriverMyGUI_CMYGUIDATAMANAGER_H
#endif /* GUCEF_guidriverMyGUI_CMYGUIDATAMANAGER_H ? */

#ifndef GUCEF_guidriverMyGUI_CMYGUILOGLISTENER_H
#include "guidriverMyGUI_CMyGUILogListener.h"
#define GUCEF_guidriverMyGUI_CMYGUILOGLISTENER_H
#endif /* GUCEF_guidriverMyGUI_CMYGUILOGLISTENER_H ? */

#ifndef GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#include "guidriverMyGUI_CMyGUIContext.h"
#define GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#endif /* GUCEF_guidriverMyGUI_CMYGUICONTEXT_H ? */

#ifndef GUCEF_guidriverMyGUI_CMYGUIWIDGETHANDLER_H
#include "guidriverMyGUI_CMyGUIWidgetHandler.h"
#define GUCEF_guidriverMyGUI_CMYGUIWIDGETHANDLER_H
#endif /* GUCEF_guidriverMyGUI_CMYGUIWIDGETHANDLER_H ? */

#include "guidriverMyGUI_CMyGUIDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CMyGUIDriver::EngineDataName = "MyGUI.Engine";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMyGUIDriver::CMyGUIDriver( bool ownDataManager )
    : GUI::CGUIDriver()
    , m_logManager( GUCEF_NULL )
    , m_logSource( GUCEF_NULL )
    , m_logListener( GUCEF_NULL )
    , m_dataManager( GUCEF_NULL )
    , m_handlers()
    , m_contexts()
    , m_fonts()
    , m_fontPaths()
{GUCEF_TRACE;

    // the messages of MyGUI go to the GUCEF logging
    m_logManager = new MyGUI::LogManager();
    m_logManager->setSTDOutputEnabled( false );
    m_logListener = GUCEF_NEW CMyGUILogListener();
    m_logSource = new MyGUI::LogSource();
    m_logSource->addLogListener( m_logListener );
    m_logManager->addLogSource( m_logSource );

    // the data of MyGUI (skins, fonts, shaders) is in the VFS like everything else
    if ( !GUI::CGuiGlobal::MountModuleData( EngineDataName ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIDriver: The data of MyGUI.Engine (the standard skins and fonts) cannot be mounted, MyGUI will not start" );
    if ( ownDataManager )
    {
        m_dataManager = GUCEF_NEW CMyGUIDataManager();
        m_dataManager->AddSearchPath( EngineDataName );
    }
}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::~CMyGUIDriver()
{GUCEF_TRACE;

    if ( GetContextCount() > 0 )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "MyGUIDriver: " << GetContextCount() << " context(s) still exist while the driver goes" );
    m_contexts.clear();
    for ( THandlerMap::iterator i=m_handlers.begin(); i!=m_handlers.end(); ++i )
        delete (*i).second;
    m_handlers.clear();

    // the managers of MyGUI are singletons that remove themselves
    delete m_dataManager;
    m_dataManager = GUCEF_NULL;
    delete m_logManager;
    m_logManager = GUCEF_NULL;
    delete m_logSource;
    m_logSource = GUCEF_NULL;
    delete m_logListener;
    m_logListener = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CMyGUIDataManager*
CMyGUIDriver::GetDataManager( void )
{GUCEF_TRACE;

    return m_dataManager;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIDriver::AddContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_contexts.insert( TGuiContextWeakPtr( context ) );
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIDriver::HasContext( void )
{GUCEF_TRACE;

    return GetContextCount() > 0;
}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::TGUIContextSet
CMyGUIDriver::GetContextList( void )
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

UInt32
CMyGUIDriver::GetContextCount( void )
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TWeakContextSet::const_iterator i=m_contexts.begin(); i!=m_contexts.end(); ++i )
        if ( !(*i).IsExpired() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::TStringSet
CMyGUIDriver::GetAvailableFormTypes( void )
{GUCEF_TRACE;

    TStringSet types;
    types.insert( "Form" );
    types.insert( "FormEx" );
    return types;
}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::TStringSet
CMyGUIDriver::GetAvailableWidgetTypes( void )
{GUCEF_TRACE;

    return GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetTypeNames();
}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::TStringSet
CMyGUIDriver::GetAvailableFonts( void )
{GUCEF_TRACE;

    return m_fonts;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIDriver::LoadFontFromAsset( const CString& assetPath )
{GUCEF_TRACE;

    CString name = CORE::StripFileExtention( CORE::ExtractFilename( assetPath ) );

    m_fontPaths[ name ] = assetPath;

    // the font is made when MyGUI runs: now when it does, else when it starts
    TGUIContextSet contexts = GetContextList();
    for ( TGUIContextSet::iterator i=contexts.begin(); i!=contexts.end(); ++i )
    {
        GUI::TGuiContextPtr contextPtr = (*i);
        GUI::CGUIContext* guiContext = contextPtr->AsGUIContext();
        CMyGUIContext* context = static_cast< CMyGUIContext* >( guiContext );
        if ( GUCEF_NULL != context && context->IsGuiStarted() )
        {
            if ( !context->LoadFont( name, assetPath, 12 ) )
            {
                m_fontPaths.erase( name );
                return false;
            }
        }
    }
    m_fonts.insert( name );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIDriver::LoadPendingFonts( CMyGUIContext& context )
{GUCEF_TRACE;

    for ( TFontMap::iterator i=m_fontPaths.begin(); i!=m_fontPaths.end(); ++i )
        if ( !context.HasMyGUIFont( (*i).first ) )
            context.LoadFont( (*i).first, (*i).second, 12 );
}

/*-------------------------------------------------------------------------*/

CString
CMyGUIDriver::GetDriverProperty( const CString& propertyName ) const
{GUCEF_TRACE;

    if ( "MyGUIVersion" == propertyName )
        return "3.5.4";
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

GUI::CWidgetHandler*
CMyGUIDriver::GetWidgetHandler( const CString& widgetType )
{GUCEF_TRACE;

    CMyGUIWidgetHandler::EKind kind = CMyGUIWidgetHandler::KIND_WIDGET;
    if ( !CMyGUIWidgetHandler::GetKindOfType( widgetType, kind ) )
        return GUCEF_NULL;

    THandlerMap::iterator i = m_handlers.find( widgetType );
    if ( i != m_handlers.end() )
        return (*i).second;
    CMyGUIWidgetHandler* handler = GUCEF_NEW CMyGUIWidgetHandler( kind );
    m_handlers[ widgetType ] = handler;
    return handler;
}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::TStringSet
CMyGUIDriver::GetCapabilities( void )
{GUCEF_TRACE;

    TStringSet capabilities;
    capabilities.insert( "NativeWidgets" );
    capabilities.insert( "VectorFonts" );
    capabilities.insert( "DynamicImages" );
    return capabilities;
}

/*-------------------------------------------------------------------------*/

UInt32
CMyGUIDriver::GetInterfaceVersion( void ) const
{GUCEF_TRACE;

    return GUI::CGUIDriver::InterfaceVersion;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIDriver::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIDriver::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CMyGUIDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::guidriverMyGUI::CMyGUIDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
