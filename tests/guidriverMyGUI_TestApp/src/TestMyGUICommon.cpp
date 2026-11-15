/*
 *  guidriverMyGUI_TestApp: Application for testing the MyGUI GUI drivers
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#include "guidriverMyGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
LoadTestPlugin( const char* moduleName, const char* pluginGroup )
{
    CORE::CString release = CORE::CString( "$MODULEDIR$/" ) + moduleName;
    CORE::CString debug = release + "_d";
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString& first = debug;
    const CORE::CString& second = release;
    #else
    const CORE::CString& first = release;
    const CORE::CString& second = debug;
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, pluginGroup, true ) || pluginControl.AddPluginFromDir( second, pluginGroup, true );
}

/*-------------------------------------------------------------------------*/

bool
LoadImageCodecs( void )
{
    static bool loaded = false;
    if ( loaded )
        return true;
    try
    {
        CORE::CPluginMetaData pluginMetaData;
        pluginMetaData.SetPluginType( "GucefImageCodecPlugin" );
        pluginMetaData.SetFullModulePath( "$MODULEDIR$" );
        pluginMetaData.SetModuleFilename( "imgpluginFreeImage" );
        pluginMetaData.SetAltModuleFilename( "imgpluginFreeImage_d" );
        pluginMetaData.SetAllLinkBackModules( "gucefCORE,gucefIMAGE" );
        IMAGE::CImageGlobal::Instance();
        loaded = CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginMetaData( pluginMetaData, "mygui_testapp_imagecodecs", true, true );
    }
    catch ( ... )
    {
    }
    return loaded;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateMyGUIContextInWindow( UInt32 windowWidth, UInt32 windowHeight )
{
    GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
    GUI::CWindowManager& windows = GUI::CGuiGlobal::Instance()->GetWindowManager();

    const char* backendName = "EGL";
    if ( windows.GetBackend( backendName ).IsNULL() )
        LoadTestPlugin( "guidriverEGL", "guidriver" );
    if ( GUCEF_NULL == manager.GetGuiDriver( "MyGUIOpenGL" ) )
    {
        LoadTestPlugin( "guidriverMyGUI", "guidriver" );
        LoadTestPlugin( "guidriverMyGUIOpenGL", "guidriver" );
    }
    LoadImageCodecs();

    GUI::TWindowManagerBackendPtr backend = windows.GetBackend( backendName );
    if ( backend.IsNULL() || GUCEF_NULL == manager.GetGuiDriver( "MyGUIOpenGL" ) )
        return GUI::TGuiContextPtr();
    GUI::TWindowContextPtr window = backend->CreateWindowContext( "Test", windowWidth, windowHeight, false );
    if ( window.IsNULL() )
        return GUI::TGuiContextPtr();
    GUI::TGuiContextPtr context = manager.CreateGUIContext( "MyGUIOpenGL", window );
    GUI::CGUIContext* guiContext = ( context.IsNULL() ? GUCEF_NULL : context->AsGUIContext() );
    if ( GUCEF_NULL != guiContext )
        guiContext->SetLogicalResolution( 640, 480 );
    return context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateMyGUIContext( void )
{
    return CreateMyGUIContextInWindow( 640, 480 );
}

/*-------------------------------------------------------------------------*/
