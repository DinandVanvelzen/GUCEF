/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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

#include "gucefCORE_CPlatformNativeConsoleWindow.h"

#if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

#ifndef GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H
#include "gucefCORE_CMSWin32ConsoleWindow.h"
#define GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H
#endif /* GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H ? */

#elif GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX

// @TODO make an implementation

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPlatformNativeConsoleWindow::CPlatformNativeConsoleWindow( void )
    : m_nativeConsoleWindow( GUCEF_NULL )
{
    #if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
    m_nativeConsoleWindow = GUCEF_NEW CMsWin32ConsoleWindow();
    #elif GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX
    // @TODO make an implementation
    #endif
}

/*-------------------------------------------------------------------------*/

CPlatformNativeConsoleWindow::~CPlatformNativeConsoleWindow()
{
    #if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
    GUCEF_DELETE static_cast<CMsWin32ConsoleWindow*>( m_nativeConsoleWindow );
    #elif GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX
    // @TODO make an implementation
    #endif
    m_nativeConsoleWindow = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CPlatformNativeConsoleWindow::CreateConsole( void )
{
    #if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
    return static_cast<CMsWin32ConsoleWindow*>( m_nativeConsoleWindow )->ConsoleWindowCreate();
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
