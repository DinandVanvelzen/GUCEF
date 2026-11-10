/*
 *  guidriverEmscripten: GUCEF window driver for web browsers: a canvas with WebGL 2
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

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWCONTEXT_H
#include "guidriverEmscripten_CEmscriptenWindowContext.h"
#define GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWCONTEXT_H
#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_CEMSCRIPTENWINDOWCONTEXT_H ? */

#include "guidriverEmscripten_CEmscriptenWindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEREMSCRIPTEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEmscriptenWindowManagerImp::CEmscriptenWindowManagerImp( void )
    : GUI::CWindowManagerBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEmscriptenWindowManagerImp::~CEmscriptenWindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CEmscriptenWindowManagerImp::CreateWindowContext( const GUI::CString& title ,
                                                  const GUI::UInt32 width   ,
                                                  const GUI::UInt32 height  ,
                                                  const bool fullscreen     )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 32 );
    videoSettings.SetFrequency( 60 );
    return CreateWindowContext( title, videoSettings, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CEmscriptenWindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                                  const GUI::CVideoSettings& settings ,
                                                  const CORE::CValueList* params      )
{GUCEF_TRACE;

    TEmscriptenWindowContextPtr windowContext( GUCEF_NEW CEmscriptenWindowContext() );
    if ( windowContext->Initialize( title, settings, params ) )
        return windowContext;
    return GUI::TWindowContextPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CEmscriptenWindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
                                                  const GUI::UInt32 width        ,
                                                  const GUI::UInt32 height       ,
                                                  const bool fullscreen          ,
                                                  const CORE::CValueList& params )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 32 );
    videoSettings.SetFrequency( 60 );
    return CreateWindowContext( title, videoSettings, &params );
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenWindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    CEmscriptenWindowContext* canvasContext = static_cast< CEmscriptenWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL != canvasContext )
        canvasContext->Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenWindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                                 const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    CEmscriptenWindowContext* canvasContext = static_cast< CEmscriptenWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL == canvasContext )
        return false;
    if ( !canvasContext->Resize( settings.GetResolutionWidthInPixels(), settings.GetResolutionHeightInPixels() ) )
        return false;
    return canvasContext->SetFullscreen( settings.GetFullscreenState() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEmscriptenWindowManagerImp::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVEREMSCRIPTEN::CEmscriptenWindowManagerImp";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
