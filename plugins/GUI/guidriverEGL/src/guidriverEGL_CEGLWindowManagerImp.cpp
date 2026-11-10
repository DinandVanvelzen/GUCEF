/*
 *  guidriverEGL: GUCEF window driver giving offscreen OpenGL windows through EGL, for tests, tools and headless runs
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

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_GUIDRIVEREGL_CEGLWINDOWCONTEXT_H
#include "guidriverEGL_CEGLWindowContext.h"
#define GUCEF_GUIDRIVEREGL_CEGLWINDOWCONTEXT_H
#endif /* GUCEF_GUIDRIVEREGL_CEGLWINDOWCONTEXT_H ? */

#include "guidriverEGL_CEGLWindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEREGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEGLWindowManagerImp::CEGLWindowManagerImp( void )
    : GUI::CWindowManagerBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEGLWindowManagerImp::~CEGLWindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CEGLWindowManagerImp::CreateWindowContext( const GUI::CString& title ,
                                           const GUI::UInt32 width   ,
                                           const GUI::UInt32 height  ,
                                           const bool fullscreen     )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 32 );
    videoSettings.SetFrequency( 0 );
    return CreateWindowContext( title, videoSettings, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CEGLWindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                           const GUI::CVideoSettings& settings ,
                                           const CORE::CValueList* params      )
{GUCEF_TRACE;

    TEGLWindowContextPtr windowContext( GUCEF_NEW CEGLWindowContext() );
    if ( windowContext->Initialize( title, settings ) )
        return windowContext;
    return GUI::TWindowContextPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CEGLWindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
                                           const GUI::UInt32 width        ,
                                           const GUI::UInt32 height       ,
                                           const bool fullscreen          ,
                                           const CORE::CValueList& params )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 32 );
    videoSettings.SetFrequency( 0 );
    return CreateWindowContext( title, videoSettings, &params );
}

/*-------------------------------------------------------------------------*/

void
CEGLWindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    CEGLWindowContext* eglContext = static_cast< CEGLWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL != eglContext )
        eglContext->Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CEGLWindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                          const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    CEGLWindowContext* eglContext = static_cast< CEGLWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL == eglContext )
        return false;
    return eglContext->Resize( settings.GetResolutionWidthInPixels(), settings.GetResolutionHeightInPixels() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEGLWindowManagerImp::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVEREGL::CEGLWindowManagerImp";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEREGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
