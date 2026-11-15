/*
 *  guidriverImGuiOpenGL_TestApp: tests of the gucefGUI driver ImGui (OpenGL)
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

#ifndef GUIDRIVERIMGUIGL_TESTSUPPORT_H
#define GUIDRIVERIMGUIGL_TESTSUPPORT_H

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

#ifndef GUIDRIVERIMGUI_H
#include "guidriverImGui.h"
#define GUIDRIVERIMGUI_H
#endif /* GUIDRIVERIMGUI_H ? */

#ifndef GUIDRIVERIMGUIGL_H
#include "guidriverImGuiOpenGL.h"
#define GUIDRIVERIMGUIGL_H
#endif /* GUIDRIVERIMGUIGL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIGLTEST {

typedef CORE::CString CString;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A Dear ImGui driver without a GPU renderer: its contexts serve textures in memory and capture with the software rasterizer; the reference of the GPU captures */
class CTestSoftwareDriver : public GUIDRIVERIMGUI::CImGuiDriver
{
    public:

    CTestSoftwareDriver( void );

    virtual ~CTestSoftwareDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverName( void ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** A window without any GL property: what an OpenGL driver has to refuse (a window of another renderer, a window made without a GL context) */
class CNoGlWindow : public GUI::CWindowContext
{
    public:

    CNoGlWindow( void );

    virtual UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetProperty( const CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** A frame builder that calls a function of a test in every frame */
class CTestFrameBuilder : public GUIDRIVERIMGUI::CImGuiFrameBuilder
{
    public:

    typedef void ( *TBuildFunction )( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs, void* userData );

    CTestFrameBuilder( TBuildFunction function, void* userData );

    void SetFunction( TBuildFunction function, void* userData );

    protected:

    virtual void OnBuildFrame( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    TBuildFunction m_function;
    void* m_userData;
};

/*-------------------------------------------------------------------------*/

/** Counts the log messages that contain a text, or any message of a kind of error */
class CLogTextCounter : public CORE::CILogger
{
    public:

    CORE::CString m_text;
    UInt32 m_count;
    MT::CMutex m_lock;

    CLogTextCounter( const CORE::CString& text );

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetMinimalLogLevel( const Int32 minimalLogLevel ) GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The software driver of the tests, made at the first use; destroyed at the end */
CTestSoftwareDriver& GetSoftwareDriver( void );
void DestroySoftwareDriver( void );

/** The OpenGL driver of this plugin made directly (not through the plugin entry points), made at the first use; destroyed at the end */
GUIDRIVERIMGUIGL::CImGuiOpenGLDriver& GetOpenGLDriver( void );
void DestroyOpenGLDriver( void );

/** Loads a plugin from the directory of the test application */
bool LoadTestPlugin( const char* moduleName, const char* pluginGroup );

/** The FreeType font rasterizer, which every ImGui context needs */
bool LoadFreeTypeFontPlugin( void );

/** A software-only ImGui context of 640 x 480 logical pixels (the reference) */
GUI::TGuiContextPtr CreateSoftwareContext( void );

/** The ImGui context of a context pointer */
GUIDRIVERIMGUI::CImGuiContext* AsImGui( const GUI::TGuiContextPtr& context );

IMAGE::TPixelMapPtr MakeSolidImage( UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a );

/** A white atlas with one cell per glyph for the printable ASCII code points; every glyph is a full block */
GUI::TBitmapFontPtr MakeTestFont( Int32 width, Int32 height );

GUI::CColour PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y );

/** Same colour, give or take a little */
bool Near( const GUI::CColour& a, const GUI::CColour& b, Int32 tolerance );

/** The test suites */
void PerformGLCoreTests( void );
void PerformGLRenderTests( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIGLTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_TESTSUPPORT_H ? */
