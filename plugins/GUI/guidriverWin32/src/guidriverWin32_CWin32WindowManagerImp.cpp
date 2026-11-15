/*
 *  guidriverWin32: module implementing window management for Win32
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

#ifndef GUIDRIVERWIN32_CWIN32WINDOWCONTEXT_H
#include "GUIDRIVERWIN32_CWin32WindowContext.h"
#define GUIDRIVERWIN32_CWIN32WINDOWCONTEXT_H
#endif /* GUIDRIVERWIN32_CWIN32WINDOWCONTEXT_H ? */

#include "GUIDRIVERWIN32_CWin32WindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWin32WindowManagerImp::CWin32WindowManagerImp( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWin32WindowManagerImp::~CWin32WindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWin32WindowManagerImp::CreateWindowContext( const GUI::CString& title ,
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
CWin32WindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                             const GUI::CVideoSettings& settings ,
                                             const CORE::CValueList* params      )
{GUCEF_TRACE;

    if ( !IsRequestedGraphicsApiSupported( "Win32", params ) )
        return GUI::TWindowContextPtr();

    TWin32WindowContextPtr windowContext = TWin32WindowContextPtr( new CWin32WindowContext() );
    if ( windowContext->Initialize( title    ,
                                    settings ) )
    {
        return windowContext;
    }
    return GUI::TWindowContextPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWin32WindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
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
CWin32WindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    CWin32WindowContext* context = static_cast<CWin32WindowContext*>( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL != context )
    {
        context->Shutdown();
    }
}

/*-------------------------------------------------------------------------*/

bool
CWin32WindowManagerImp::IsGraphicsApiSupported( const GUI::CString& graphicsApi ) const
{GUCEF_TRACE;

    return GUI::CWindowManagerBackend::GraphicsApiNone == graphicsApi;
}

/*-------------------------------------------------------------------------*/

bool
CWin32WindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                            const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CWin32WindowManagerImp::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVERWIN32::CWin32WindowManagerImp";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32 */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
