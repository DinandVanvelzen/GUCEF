/*
 *  guidriverMyGUIOpenGL: GUI backend using MyGUI with OpenGL, OpenGL 3 or OpenGL ES 3 / WebGL 2
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

#if !defined( __ANDROID__ ) && !defined( __EMSCRIPTEN__ )
#include "MyGUI_OpenGL3RenderManager.h"
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_guidriverMyGUI_CMYGUIIMAGECODEC_H
#include "guidriverMyGUI_CMyGUIImageCodec.h"
#define GUCEF_guidriverMyGUI_CMYGUIIMAGECODEC_H
#endif /* GUCEF_guidriverMyGUI_CMYGUIIMAGECODEC_H ? */

#include "guidriverMyGUIOpenGL_IGLBackend.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUIOpenGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#if !defined( __ANDROID__ ) && !defined( __EMSCRIPTEN__ )

namespace {

/** The images of MyGUI: gucefIMAGE decodes what the VFS has */
class CImageLoader : public MyGUI::OpenGL3ImageLoader
{
    public:

    virtual void* loadImage( int& width, int& height, MyGUI::PixelFormat& format, const std::string& filename ) override
    {GUCEF_TRACE;

        return guidriverMyGUI::CMyGUIImageCodec::LoadImage( filename.c_str(), width, height, format, true );
    }

    virtual void saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const std::filesystem::path& filename ) override
    {GUCEF_TRACE;

        guidriverMyGUI::CMyGUIImageCodec::SaveImage( width, height, format, texture, filename.generic_string().c_str(), true );
    }
};

/*-------------------------------------------------------------------------*/

class CBackend : public IGLBackend
{
    public:

    CBackend( void ) : m_renderManager( GUCEF_NULL ), m_loader(), m_running( false ) {}

    virtual ~CBackend()
    {GUCEF_TRACE;

        Shutdown();
        delete m_renderManager;
        m_renderManager = GUCEF_NULL;
    }

    virtual bool Initialise( void ) override
    {GUCEF_TRACE;

        if ( m_running )
            return true;
        try
        {
            if ( GUCEF_NULL == m_renderManager )
                m_renderManager = new MyGUI::OpenGL3RenderManager();
            m_renderManager->initialise( &m_loader );
            m_running = true;
        }
        catch ( const std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGL: The OpenGL3 render manager of MyGUI could not start: " << e.what() );
            return false;
        }
        return true;
    }

    virtual void Shutdown( void ) override
    {GUCEF_TRACE;

        if ( m_running && GUCEF_NULL != m_renderManager )
            m_renderManager->shutdown();
        m_running = false;
    }

    virtual void DrawOneFrame( void ) override
    {GUCEF_TRACE;

        if ( m_running )
            m_renderManager->drawOneFrame();
    }

    virtual bool IsBgraMemoryOrder( void ) const override
    {
        return true;
    }

    virtual bool SupportsMsdfFonts( void ) const override
    {
        return true;
    }

    virtual bool RegisterMsdfShader( const char* shaderName ) override
    {GUCEF_TRACE;

        if ( !m_running || GUCEF_NULL == m_renderManager )
            return false;
        try
        {
            m_renderManager->registerShader( shaderName, "MyGUI_OpenGL3_VP.glsl", "Msdf_OpenGL3_FP.glsl" );
        }
        catch ( const std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOpenGL: The MSDF font shader of the OpenGL3 platform could not be made: " << e.what() );
            return false;
        }
        return true;
    }

    virtual const char* GetName( void ) const override
    {
        return "OpenGL3";
    }

    private:

    MyGUI::OpenGL3RenderManager* m_renderManager;
    CImageLoader m_loader;
    bool m_running;
};

}

/*-------------------------------------------------------------------------*/

IGLBackend*
CreateGL3Backend( void )
{GUCEF_TRACE;

    return GUCEF_NEW CBackend();
}

#else

/*-------------------------------------------------------------------------*/

IGLBackend*
CreateGL3Backend( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOpenGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
