/*
 *  guidriverWin32GL: module implementing GL based window management for Win32
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

#ifndef GUIDRIVERWIN32GL_CWIN32GLWINDOWCONTEXT_H
#include "guidriverWin32GL_CWin32GLWindowContext.h"
#define GUIDRIVERWIN32GL_CWIN32GLWINDOWCONTEXT_H
#endif /* GUIDRIVERWIN32GL_CWIN32GLWINDOWCONTEXT_H ? */

#include "guidriverWin32GL_CWin32GLWindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32GL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWin32GLWindowManagerImp::CWin32GLWindowManagerImp( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWin32GLWindowManagerImp::~CWin32GLWindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWin32GLWindowManagerImp::CreateWindowContext( const GUI::CString& title ,
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
CWin32GLWindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                               const GUI::CVideoSettings& settings ,
                                               const CORE::CValueList* params      )
{GUCEF_TRACE;

    TWin32GLWindowContextPtr windowContext( new CWin32GLWindowContext() );
    if ( windowContext->Initialize( title    ,
                                    settings ) )
    {
        return windowContext;
    }
    return GUI::TWindowContextPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWin32GLWindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
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
CWin32GLWindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    CWin32GLWindowContext* glContext = static_cast<CWin32GLWindowContext*>( windowContext.GetPointerAlways() );
    if ( NULL != glContext )
    {
        glContext->Shutdown();
    }
}

/*-------------------------------------------------------------------------*/

bool
CWin32GLWindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                              const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    CWin32GLWindowContext* glContext = static_cast< CWin32GLWindowContext* >( windowContext.GetPointerAlways() );
    if ( NULL == glContext )
        return false;
    return glContext->ApplyVideoSettings( settings );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32GL */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
