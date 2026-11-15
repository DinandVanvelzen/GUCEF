/*
 *  guidriverMyGUIOgre: GUI backend using MyGUI with Ogre
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

#include <Ogre.h>

#include "MyGUI_OgreRenderManager.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverMyGUIOgre_COgreContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUIOgre {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreContext::COgreContext( GUI::CGUIDriver* driver              ,
                            GUI::TWindowContextPtr windowContext ,
                            Ogre::RenderWindow* renderWindow     ,
                            Ogre::SceneManager* sceneManager     )
    : guidriverMyGUI::CMyGUIContext( driver, windowContext )
    , m_renderWindow( renderWindow )
    , m_renderManager( GUCEF_NULL )
    , m_isValid( false )
{GUCEF_TRACE;

    if ( GUCEF_NULL == renderWindow || GUCEF_NULL == sceneManager )
        return;

    try
    {
        m_renderManager = new MyGUI::OgreRenderManager();
        m_renderManager->initialise( renderWindow, sceneManager );
        m_isValid = StartGui();
        if ( !m_isValid )
            m_renderManager->shutdown();
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOgreContext: MyGUI could not start on Ogre: " << e.what() );
        m_isValid = false;
    }
}

/*-------------------------------------------------------------------------*/

COgreContext::~COgreContext()
{GUCEF_TRACE;

    // MyGUI lets go of its textures and buffers through the render manager, which goes after it
    ReleaseGui();
    if ( GUCEF_NULL != m_renderManager )
    {
        try
        {
            if ( m_isValid )
                m_renderManager->shutdown();
        }
        catch ( const std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOgreContext: The render manager of MyGUI did not shut down cleanly: " << e.what() );
        }
        delete m_renderManager;
        m_renderManager = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
COgreContext::IsValid( void ) const
{GUCEF_TRACE;

    return m_isValid;
}

/*-------------------------------------------------------------------------*/

bool
COgreContext::IsBgraTextureMemory( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
COgreContext::SupportsMsdfFonts( void ) const
{GUCEF_TRACE;

    return m_isValid && GUCEF_NULL != m_renderManager;
}

/*-------------------------------------------------------------------------*/

bool
COgreContext::RegisterMsdfShader( void )
{GUCEF_TRACE;

    if ( !m_isValid || GUCEF_NULL == m_renderManager )
        return false;
    try
    {
        // the shader files are in the data of MyGUI.Engine, which the Ogre resource group of MyGUI has as an
        // archive of the VFS: no resource of its own is needed. The vertex program is the one of the default shader.
        const std::string extension = m_renderManager->getShaderExtension( true );
        m_renderManager->registerShader( MsdfShaderName, "MyGUI_Ogre_VP." + extension, "Msdf_Ogre_FP." + extension );
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIOgreContext: The MSDF font shader could not be made: " << e.what() );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
COgreContext::OnRender( void )
{GUCEF_TRACE;

    // Ogre draws the widgets while it renders the window: the render queue listener of MyGUI
    if ( m_isValid )
        PrepareFrame();
}

/*-------------------------------------------------------------------------*/

bool
COgreContext::CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( !m_isValid || GUCEF_NULL == m_renderWindow )
        return false;

    const UInt32 width = m_renderWindow->getWidth();
    const UInt32 height = m_renderWindow->getHeight();
    if ( 0 == width || 0 == height )
        return false;

    if ( pixels.IsNULL() || pixels->GetWidthInPixels() != width || pixels->GetHeightInPixels() != height ||
         IMAGE::PSF_RGBA != pixels->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() )
    {
        pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    }

    Ogre::PixelBox box( width, height, 1, Ogre::PF_BYTE_RGBA, pixels->GetDataPtr() );
    m_renderWindow->copyContentsToMemory( box, box );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COgreContext::CaptureFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    // the widgets are drawn inside the window: the frame is what the window shows
    return CapturePresentedFrame( pixels );
}

/*-------------------------------------------------------------------------*/

const CString&
COgreContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::guidriverMyGUIOgre::COgreContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOgre */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
