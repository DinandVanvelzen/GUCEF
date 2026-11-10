/*
 *  guidriverAndroidGLES: module implementing GLES based window management for Android
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVERANDROIDGLES_CANDROIDGLESWINDOWCONTEXT_H
#include "guidriverAndroidGLES_CAndroidGLESWindowContext.h"
#define GUIDRIVERANDROIDGLES_CANDROIDGLESWINDOWCONTEXT_H
#endif /* GUIDRIVERANDROIDGLES_CANDROIDGLESWINDOWCONTEXT_H ? */

#include "guidriverAndroidGLES_CAndroidWindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERANDROIDGLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CAndroidWindowManagerImp::CAndroidWindowManagerImp( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidWindowManagerImp::~CAndroidWindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CAndroidWindowManagerImp::CreateWindowContext( const GUI::CString& title ,
                                               const GUI::UInt32 width   ,
                                               const GUI::UInt32 height  ,
                                               const bool fullscreen     )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 24 );

    return CreateWindowContext( title         ,
                                videoSettings ,
                                NULL          );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CAndroidWindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                               const GUI::CVideoSettings& settings ,
                                               const CORE::CValueList* params      )
{GUCEF_TRACE;

    TAndroidGLESWindowContextPtr windowContext( new CAndroidGLESWindowContext() );
    if ( windowContext->Initialize( title, settings, params ) )
    {
        return windowContext;
    }
    return GUI::TWindowContextPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CAndroidWindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
                                               const GUI::UInt32 width        ,
                                               const GUI::UInt32 height       ,
                                               const bool fullscreen          ,
                                               const CORE::CValueList& params )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 24 );

    return CreateWindowContext( title         ,
                                videoSettings ,
                                &params       );
}

/*-------------------------------------------------------------------------*/

void
CAndroidWindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    CAndroidGLESWindowContext* glContext = static_cast<CAndroidGLESWindowContext*>( windowContext.GetPointerAlways() );
    if ( NULL != glContext )
    {
        glContext->Shutdown();
    }
}

/*-------------------------------------------------------------------------*/

bool
CAndroidWindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                              const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    CAndroidGLESWindowContext* glesContext = static_cast< CAndroidGLESWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL == glesContext )
        return false;
    return glesContext->ApplyVideoSettings( settings );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERANDROIDGLES */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
