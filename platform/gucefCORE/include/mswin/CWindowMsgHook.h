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

#ifndef GUCEF_CORE_CWINDOWMSGHOOK_H
#define GUCEF_CORE_CWINDOWMSGHOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_macros.h"  /* module config macros */

#ifdef GUCEF_MSWIN_BUILD

#include <map>
#include "gucefMT_CMutex.h"    /* needed for threadsafety */

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
 *  Class that should be used when performing instance subclassing of 
 *  windows. Using an uniform aproach minimizes the risks.
 */
class GUCEF_CORE_PUBLIC_CPP CWindowMsgHook
{
    public:
    
    CWindowMsgHook();

    virtual ~CWindowMsgHook();

    /**
     *  Subscribes the hook object to messages sent to the window
     *  with the given handle
     *
     *  @param hWnd handle to the window you wish to instance subclass
     *  @param userData optional pointer that will be passed when WindowProc is called
     */
    void Hook( const HWND hWnd       ,
               void* userData = NULL );

    /**
     *  Unsubscribes the hook object from messages sent to the window
     *  with the given handle
     */
    void Unhook( const HWND hWnd );

    protected:
    
    //This is the function you need to override in your derived
    //version of CWindowMsgHook so that you can handle the window messages
    virtual LRESULT WindowProc( const HWND hWnd     ,
                                const UINT nMsg     ,
                                const WPARAM wParam ,
                                const LPARAM lParam ,
                                void* userData      ,
                                bool& consumeMsg    );

    private:

    //The callback function through which all window messages
    //will be handled when they are hooked by CHookWnd 
    static LRESULT CALLBACK HookProc( HWND hWnd     ,
                                      UINT nMsg     ,
                                      WPARAM wParam ,
                                      LPARAM lParam );

    private:
    typedef GUCEF::map< CWindowMsgHook*, void* > TWindowMsgHookList;
    struct SWindowData
    {
        TWindowMsgHookList hookList;
        WNDPROC orgWinProc;
    };
    typedef struct SWindowData TWindowData;
    typedef GUCEF::map< HWND, TWindowData > TMappedWindowHookList;

    static MT::CMutex s_globalDataLock;
    static TMappedWindowHookList s_list;

    private:

    CWindowMsgHook( const CWindowMsgHook& src );       /**< cannot be copied */
    CWindowMsgHook& operator=( const CWindowMsgHook ); /**< cannot be copied */
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

#endif /* GUCEF_CORE_CWINDOWMSGHOOK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 22-04-2003 :
        - Initial implementation
          
---------------------------------------------------------------------------*/
