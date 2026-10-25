/*
 *  inputdriverXWINMSG: GUCEF input driver module for X11
 *  Copyright (C) 2002 - 2012.  Dinand Vanvelzen
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

#ifndef GUCEF_INPUT_XWINMSG_CEVENTLISTNER_H
#define GUCEF_INPUT_XWINMSG_CEVENTLISTNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"  /* module config macros */
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

#ifdef GUCEF_LINUX_BUILD

#ifndef GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H
#include "inputdriverpluginstructs.h"   /* plugin API structures */
#define GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H
#endif /* GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H ? */

#ifndef GUCEF_CORE_CX11EVENTDISPATCHER_H
#include "gucefCORE_CX11EventDispatcher.h"
#define GUCEF_CORE_CX11EVENTDISPATCHER_H
#endif /* GUCEF_CORE_CX11EVENTDISPATCHER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {
namespace XWINMSG {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *
 */
class GUCEF_INPUT_PLUGIN_PRIVATE_CPP CEventListner : public CORE::CObserver
{
    public:

    CEventListner( ::Window window                  ,
                   const TInputCallbacks* callbacks );

    virtual ~CEventListner();

    ::Window GetWindow( void ) const;

    const TInputCallbacks* GetCallbacks( void ) const;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL );

    private:

    void OnX11Event( ::XEvent& eventData );

    void SendText( ::XKeyEvent& keyEvent );

    INPUT::KeyCode MapKeyCode( int x11KeyCode );

    private:

    ::Window m_window;
    TInputCallbacks m_callbacks;
    Int32 m_mouseX;
    Int32 m_mouseY;
    Int32 m_wheel;
    Int32 m_horizontalWheel;
    UInt32 m_movesSinceButtonChange;
    ::XIM m_inputMethod;
    ::XIC m_inputContext;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XWINMSG */
}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LINUX_BUILD ? */

#endif /* GUCEF_INPUT_XWINMSG_CEVENTLISTNER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 22-04-2003 :
        - Initial implementation

---------------------------------------------------------------------------*/
