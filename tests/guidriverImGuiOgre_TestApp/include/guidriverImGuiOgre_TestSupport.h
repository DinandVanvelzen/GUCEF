/*
 *  guidriverImGuiOgre_TestApp: tests of the gucefGUI driver ImGuiOgre
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

#ifndef GUIDRIVERIMGUIOGRE_TESTSUPPORT_H
#define GUIDRIVERIMGUIOGRE_TESTSUPPORT_H

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

#include <OgreTexture.h>

#ifndef GUIDRIVERIMGUIOGRE_H
#include "guidriverImGuiOgre.h"
#define GUIDRIVERIMGUIOGRE_H
#endif /* GUIDRIVERIMGUIOGRE_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIOGRETEST {

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

/** A window that is not a window of the window backend Ogre: what the driver ImGuiOgre has to refuse */
class CNonOgreWindow : public GUI::CWindowContext
{
    public:

    CNonOgreWindow( void );

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

/** Where the Ogre windows of the tests are made */
enum ETestWindowMode
{
    TESTWINDOWMODE_NONE = 0 ,   /**< no windows: GUCEF_TEST_RENDER_GPU=0, or a display server the tests may not use */
    TESTWINDOWMODE_X11      ,   /**< on XWinGL windows of the X server of DISPLAY, when GUCEF_TEST_X11_WINDOWS=1 (Xvfb) */
    TESTWINDOWMODE_OFFSCREEN    /**< Ogre's headless EGL surface, without any display server */
};

ETestWindowMode GetTestWindowMode( void );

/** Why there are no windows, for the skip notice */
CString GetNoWindowReason( void );

/** Loads the support library, the render system, the VFS archive and image codec plugins of Ogre, the window drivers and the GUI drivers ImGuiOgre, RmlUiOgre and Null */
bool SetUpOgreForTests( void );

/** Unloads what SetUpOgreForTests() loaded, the Root goes with the support library */
void TearDownOgreForTests( void );

/** An Ogre window of the backend Ogre in the mode of the tests; NULL on failure */
GUI::TWindowContextPtr MakeTestOgreWindow( UInt32 width, UInt32 height );

/** The first window, kept for all tests: the GL context Ogre's resources live in belongs to it */
GUI::TWindowContextPtr GetMainTestWindow( void );
void ReleaseMainTestWindow( void );

GUIDRIVEROGRE::COgreWindowContext* AsOgreWindow( GUI::TWindowContextPtr window );

/** A context of the driver ImGuiOgre in a window (the main test window by default) with the logical resolution; NULL on failure */
GUI::TGuiContextPtr CreateImGuiOgreContext( GUI::TWindowContextPtr window = GUI::TWindowContextPtr() );

/** The Ogre textures that the driver made (named "ImGuiOgre/Texture/..."), still there */
UInt32 CountImGuiOgreTextures( void );

/** A texture of one colour, made by hand in the internal resource group */
Ogre::TexturePtr MakeSolidTexture( const char* name, UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a );

/** The test suites */
void PerformImGuiOgreContextTests( void );
void PerformImGuiOgreRenderTests( void );
void PerformImGuiOgreTearDownTests( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIOGRETEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIOGRE_TESTSUPPORT_H ? */
