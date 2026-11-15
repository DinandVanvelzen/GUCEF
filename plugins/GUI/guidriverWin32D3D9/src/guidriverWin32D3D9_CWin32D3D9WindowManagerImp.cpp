/*
 *  guidriverWin32D3D9: module implementing D3D9 based window management for Win32
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

#ifndef GUIDRIVERWIN32D3D9_CWIN32GLWINDOWCONTEXT_H
#include "GUIDRIVERWIN32D3D9_CWin32D3D9WindowContext.h"
#define GUIDRIVERWIN32D3D9_CWIN32GLWINDOWCONTEXT_H
#endif /* GUIDRIVERWIN32D3D9_CWIN32GLWINDOWCONTEXT_H ? */

#include "GUIDRIVERWIN32D3D9_CWin32D3D9WindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32D3D9 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWin32D3D9WindowManagerImp::CWin32D3D9WindowManagerImp( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWin32D3D9WindowManagerImp::~CWin32D3D9WindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWin32D3D9WindowManagerImp::CreateWindowContext( const GUI::CString& title ,
                                               const GUI::UInt32 width   ,
                                               const GUI::UInt32 height  ,
                                               const bool fullscreen     )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 24 );

    return CreateWindowContext( title         ,
                                videoSettings ,
                                GUCEF_NULL          );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWin32D3D9WindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                               const GUI::CVideoSettings& settings ,
                                               const CORE::CValueList* params      )
{GUCEF_TRACE;

    if ( !IsRequestedGraphicsApiSupported( "Win32D3D9", params ) )
        return GUI::TWindowContextPtr();

    TWin32GLWindowContextPtr windowContext = TWin32GLWindowContextPtr( new CWin32D3D9WindowContext() );
    if ( windowContext->Initialize( title    ,
                                    settings ) )
    {
        return windowContext;
    }
    return GUI::TWindowContextPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWin32D3D9WindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
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
CWin32D3D9WindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    CWin32D3D9WindowContext* d3d9Context = static_cast<CWin32D3D9WindowContext*>( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL != d3d9Context )
    {
        d3d9Context->Shutdown();
    }
}

/*-------------------------------------------------------------------------*/

bool
CWin32D3D9WindowManagerImp::IsGraphicsApiSupported( const GUI::CString& graphicsApi ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWin32D3D9WindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                                const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32D3D9 */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
