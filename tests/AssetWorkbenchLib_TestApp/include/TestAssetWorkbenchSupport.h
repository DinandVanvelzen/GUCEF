/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#ifndef ASSETWORKBENCHLIB_TESTAPP_TESTSUPPORT_H
#define ASSETWORKBENCHLIB_TESTAPP_TESTSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreTexture.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Where the Ogre windows of the tests are made */
enum ETestWindowMode
{
    TESTWINDOWMODE_NONE = 0 ,   /**< no windows: GUCEF_TEST_RENDER_GPU=0, or a display server the tests may not use */
    TESTWINDOWMODE_X11      ,   /**< on XWinGL windows of the X server of DISPLAY, when GUCEF_TEST_X11_WINDOWS=1 (Xvfb) */
    TESTWINDOWMODE_OFFSCREEN,   /**< Ogre's headless EGL surface, without any display server */
    TESTWINDOWMODE_WIN32        /**< on Win32 windows of the desktop of the session (Windows has no headless surface) */
};

ETestWindowMode GetTestWindowMode( void );

/** Why there are no windows, for the skip notice */
GUCEF::CORE::CString GetNoWindowReason( void );

/** The renderer the tests ask for: GUCEF_TEST_OGRE_RENDERER ("OpenGL", "Vulkan"), "OpenGL" without it and always with the headless surface */
GUCEF::CORE::CString GetTestRenderer( void );

/**
 *  Loads the Ogre support library, the Ogre plugins the workbench needs (render systems, the VFS archive, the image codec, the Assimp
 *  codec and DotScene), the window and GUI drivers, the font plugin and the zip archives of the VFS
 */
bool SetUpWorkbenchTests( void );

/** Unloads what SetUpWorkbenchTests() loaded; the Root goes with the support library */
void TearDownWorkbenchTests( void );

/** The first window, kept for all tests: the GL context that Ogre's resources live in belongs to it; NULL when none can be made */
GUCEF::GUI::TWindowContextPtr GetMainTestWindow( void );
void ReleaseMainTestWindow( void );
GUCEF::GUIDRIVEROGRE::COgreWindowContext* AsOgreWindow( GUCEF::GUI::TWindowContextPtr window );

/** An ImGuiOgre context in the main window with the font and the theme of the workbench, at the real resolution of the window */
GUCEF::GUI::TGuiContextPtr CreateWorkbenchContext( void );

/** A folder of the tests that is empty, below the work folder of the test application */
GUCEF::CORE::CString MakeCleanTestDir( const GUCEF::CORE::CString& name );

bool WriteTestFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& content );

/** The pixel of an RGBA pixel map */
void GetPixel( const GUCEF::IMAGE::TPixelMapPtr& pixels, GUCEF::Int32 x, GUCEF::Int32 y, GUCEF::UInt8 rgba[ 4 ] );

/** Whether the pixel has the colour, give or take 'tolerance' per channel */
bool PixelIs( const GUCEF::IMAGE::TPixelMapPtr& pixels, GUCEF::Int32 x, GUCEF::Int32 y, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::Int32 tolerance = 6 );

/** Whether the pixel differs from the other by more than 'tolerance' in some channel */
bool PixelsDiffer( const GUCEF::IMAGE::TPixelMapPtr& pixels, GUCEF::Int32 x1, GUCEF::Int32 y1, GUCEF::Int32 x2, GUCEF::Int32 y2, GUCEF::Int32 tolerance = 12 );

/** Writes a picture as PNG when GUCEF_TEST_DUMP_DIR names a folder, for the eyes of the person who runs the tests */
void DumpPicture( const GUCEF::IMAGE::TPixelMapPtr& pixels, const GUCEF::CORE::CString& name );

/** Counts the thumbnails a provider announces and keeps what the last one said */
class CThumbnailRecorder : public GUCEF::CORE::CObserver
{
    public:

    GUCEF::UInt32 count;
    GUCEF::CORE::CString lastPath;
    GUCEF::UInt32 lastSize;
    bool lastHadPixels;

    CThumbnailRecorder( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& id );
    virtual ~CThumbnailRecorder();

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& id, GUCEF::CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::CORE::CEvent m_eventId;
};

/** Pulses the application until the condition object answers true, about two seconds at most */
template < typename TCondition >
bool PulseUntil( TCondition condition )
{
    GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    for ( GUCEF::UInt32 i=0; i<200; ++i )
    {
        pulseGenerator->ForceDirectPulse();
        if ( condition() )
            return true;
        GUCEF::MT::PrecisionDelay( 10 );
    }
    return condition();
}

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_TESTAPP_TESTSUPPORT_H ? */
