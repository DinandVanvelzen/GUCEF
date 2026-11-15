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

#ifndef GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H
#define GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* module config macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifdef GUCEF_MSWIN_BUILD

#ifndef GUCEF_CORE_CMSWIN32WINDOW_H
#include "gucefCORE_CMsWin32Window.h"
#define GUCEF_CORE_CMSWIN32WINDOW_H
#endif /* GUCEF_CORE_CMSWIN32WINDOW_H ? */

#ifndef GUCEF_CORE_CMSWIN32EDITBOX_H
#include "gucefCORE_CMsWin32Editbox.h"
#define GUCEF_CORE_CMSWIN32EDITBOX_H
#endif /* GUCEF_CORE_CMSWIN32EDITBOX_H ? */

#ifndef GUCEF_CORE_CSYSCONSOLECLIENT_H
#include "CSysConsoleClient.h"
#define GUCEF_CORE_CSYSCONSOLECLIENT_H
#endif /* GUCEF_CORE_CSYSCONSOLECLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class which creates a Win32 API generated console window for communication
 *  with the system console API.
 */
class GUCEF_CORE_PUBLIC_CPP CMsWin32ConsoleWindow : public CMsWin32Window ,
                                                    public CSysConsoleClient
{
    public:
    
    CMsWin32ConsoleWindow();

    virtual ~CMsWin32ConsoleWindow();
    
    bool ConsoleWindowCreate( void );
    
    virtual const CString& GetClassTypeName( void ) const;

    protected:
    
    virtual LRESULT WindowProc( const HWND hWnd     ,
                                const UINT nMsg     ,
                                const WPARAM wParam ,
                                const LPARAM lParam );

    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = GUCEF_NULL );

    private:

    CMsWin32ConsoleWindow( const CMsWin32ConsoleWindow& src );       /**< cannot be copied */
    CMsWin32ConsoleWindow& operator=( const CMsWin32ConsoleWindow ); /**< cannot be copied */

    void AppendLines( const TStringVector& lines );
    
    void AppendLine( const CString& line );
        
    private:
    
    CMsWin32Editbox m_inputbox;
    CMsWin32Editbox m_outputbox;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MSWIN_BUILD ? */

#endif /* GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 22-04-2003 :
        - Initial implementation
          
---------------------------------------------------------------------------*/
