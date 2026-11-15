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

#ifndef GUIDRIVERIMGUI_CIMGUIDRIVER_H
#define GUIDRIVERIMGUI_CIMGUIDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_GUI_CGUIDRIVER_H
#include "gucefGUI_CGUIDriver.h"
#define GUCEF_GUI_CGUIDRIVER_H
#endif /* GUCEF_GUI_CGUIDRIVER_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CImGuiContext;
class CImGuiWidgetHandler;

/**
 *  What every Dear ImGui GUI driver shares, whatever draws it: the contexts (kept weakly, their owners decide when they
 *  go), the widget handlers, the capabilities and the configuration section "ImGui". A renderer driver
 *  (guidriverImGuiOpenGL: "ImGui", guidriverImGuiOgre: "ImGuiOgre") derives from it and makes its contexts in
 *  CreateGUIContext(): CanCreateContext() first, then AddContext() with a new context of its own class.
 *
 *  Capabilities: Render, CaptureFrame, BitmapFonts, VectorFonts, DrawSurface, DynamicImages, PresentScaling,
 *  ImmediateMode (its contexts notify CImGuiContext::BuildFrameEvent) and the grid features of the painter of gucefGUI.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiDriver : public GUI::CGUIDriver
{
    public:

    /** The name of the driver for layouts and tools; a renderer driver registers under its own name */
    static const char* DriverName;

    /** The configuration section the driver reads */
    static const char* ConfigSectionName;

    /** The configuration of the driver, the section "ImGui": what new contexts start with */
    struct SSettings
    {
        CString fontPath;                   /**< fontPath: a VFS path, the UI font of gucefGUI by default (CGuiGlobal::DefaultUiFontPath) */
        Float32 fontSize;                   /**< fontSize: the height of a line in logical pixels, 16 by default */
        Float32 uiScale;                    /**< uiScale: CImGuiContext::SetUiScale(), 1 by default */
        CString style;                      /**< style: Dark (default), Light or Classic */
        bool keyboardNavigation;            /**< keyboardNavigation: ImGuiConfigFlags_NavEnableKeyboard, on by default */
        bool softwareCursor;                /**< softwareCursor: Dear ImGui draws the pointer (io.MouseDrawCursor), off by default */
        bool configErrorRecoveryAsserts;    /**< configErrorRecoveryAsserts: asserts on recoverable errors, on in debug builds */

        SSettings( void );
    };
    typedef struct SSettings TSettings;

    CImGuiDriver( void );

    virtual ~CImGuiDriver();

    const TSettings& GetSettings( void ) const;

    void SetSettings( const TSettings& settings );

    virtual TGUIContextSet GetContextList( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetContextCount( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverName( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableFormTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableWidgetTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The font files offered to new contexts (LoadFontFromAsset()) */
    virtual TStringSet GetAvailableFonts( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Offers a font file (a VFS path) to the contexts made from now on: each adds it at the font size of the settings */
    virtual bool LoadFontFromAsset( const CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE;

    /** "ImGuiVersion": the version of Dear ImGui */
    virtual CString GetDriverProperty( const CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CWidgetHandler* GetWidgetHandler( const CString& widgetType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetCapabilities( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetInterfaceVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  Whether a context can be made now: on the ImGui thread (CImGuiGlobal) and with a font rasterizer registered
     *  (fontpluginFREETYPE). Logs why not
     */
    bool CanCreateContext( void ) const;

    /** Keeps a new context weakly and gives it to its owner; a context that is not ready is destroyed and NULL returned */
    GUI::TGuiContextPtr AddContext( CImGuiContext* context );

    private:

    typedef GUCEF::map< CString, CImGuiWidgetHandler* > THandlerMap;
    typedef CORE::CTWeakPtr< GUI::CIGUIContext, MT::CMutex > TGuiContextWeakPtr;
    typedef GUCEF::set< TGuiContextWeakPtr > TWeakContextSet;

    CImGuiDriver( const CImGuiDriver& src );
    CImGuiDriver& operator=( const CImGuiDriver& src );

    private:

    THandlerMap m_handlers;
    TWeakContextSet m_contexts;
    TStringSet m_fonts;
    TSettings m_settings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUIDRIVER_H ? */
