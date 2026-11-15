/*
 *  guidriverOgre_TestApp: Test application for the window backend Ogre (guidriverOgre)
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

#ifndef GUIDRIVEROGRE_TESTAPP_TESTOGRESUPPORT_H
#define GUIDRIVEROGRE_TESTAPP_TESTOGRESUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreRenderOperation.h>
#include <OgreTexture.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

#ifndef GUIDRIVEROGRE_COGREGUIMATERIAL_H
#include "guidriverOgre_COgreGuiMaterial.h"
#define GUIDRIVEROGRE_COGREGUIMATERIAL_H
#endif /* GUIDRIVEROGRE_COGREGUIMATERIAL_H ? */

namespace Ogre { class VertexData; class IndexData; }

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
    TESTWINDOWMODE_WAYLAND      /**< on Wayland windows of the compositor of WAYLAND_DISPLAY with the Wayland flavour of the render system, when GUCEF_TEST_WAYLAND_WINDOWS=1 (a headless compositor) */
};

ETestWindowMode GetTestWindowMode( void );

/** Why there are no windows, for the skip notice */
GUCEF::CORE::CString GetNoWindowReason( void );

/**
 *  The renderer the test windows ask for in their video settings: GUCEF_TEST_OGRE_RENDERER ("OpenGL", "Vulkan", ...),
 *  "OpenGL" without it. The window suite runs on the renderer the Root takes for it (GetExpectedTestRenderer())
 */
GUCEF::CORE::CString GetTestRenderer( void );

/** The renderer the Root draws with for the test windows: GetTestRenderer() when it is available, else the first available one */
GUCEF::CORE::CString GetExpectedTestRenderer( void );

/**
 *  Loads the support library, the render systems (OpenGL 3+ and Vulkan with the glslang plugin, in that configured
 *  order), the VFS archive and image codec plugins of Ogre and the window drivers
 */
bool SetUpOgreForTests( void );

/** Unloads what SetUpOgreForTests() loaded, the Root goes with the support library */
void TearDownOgreForTests( void );

/** The parameters of an Ogre window for the mode of the tests */
GUCEF::CORE::CValueList GetTestWindowParams( void );

/** An Ogre window of the backend Ogre in the mode of the tests; NULL on failure */
GUCEF::GUI::TWindowContextPtr MakeTestOgreWindow( GUCEF::UInt32 width, GUCEF::UInt32 height );

/** The first window, kept for all tests: the GL context Ogre's resources live in belongs to it */
GUCEF::GUI::TWindowContextPtr GetMainTestWindow( void );
void ReleaseMainTestWindow( void );

GUCEF::GUIDRIVEROGRE::COgreWindowContext* AsOgreWindow( GUCEF::GUI::TWindowContextPtr window );

/** The threads of the process */
GUCEF::UInt32 CountProcessThreads( void );

/** The pixel of an RGBA pixel map */
void GetPixel( const GUCEF::IMAGE::TPixelMapPtr& pixels, GUCEF::Int32 x, GUCEF::Int32 y, GUCEF::UInt8 rgba[ 4 ] );

/** Whether the pixel has the colour, give or take 'tolerance' per channel (RGB, alpha ignored) */
bool PixelIs( const GUCEF::IMAGE::TPixelMapPtr& pixels, GUCEF::Int32 x, GUCEF::Int32 y, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::Int32 tolerance = 2 );

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

/**
 *  Quads in pixels of a render target, drawn with the GUI material of guidriverOgre: the vertex layout of RmlUi
 *  (position float2, colour four bytes, texture coordinate float2) and 32-bit indices
 */
class CTestQuadBatch
{
    public:

    CTestQuadBatch( void );
    ~CTestQuadBatch();

    void AddQuad( float x, float y, float width, float height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a );

    /** Makes the hardware buffers; the quads cannot change afterwards */
    bool Build( void );

    const Ogre::RenderOperation& GetOperation( void ) const;

    private:

    struct SVertex
    {
        float x, y;
        GUCEF::UInt8 colour[ 4 ];
        float u, v;
    };

    GUCEF::vector< SVertex > m_vertices;
    GUCEF::vector< GUCEF::UInt32 > m_indices;
    Ogre::VertexData* m_vertexData;
    Ogre::IndexData* m_indexData;
    Ogre::RenderOperation m_operation;
};

/** A texture of one colour, made by hand in the internal resource group */
Ogre::TexturePtr MakeSolidTexture( const char* name, GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a );

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVEROGRE_TESTAPP_TESTOGRESUPPORT_H ? */
