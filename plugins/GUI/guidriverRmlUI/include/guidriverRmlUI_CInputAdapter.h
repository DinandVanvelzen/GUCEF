/*
 *  guidriverRmlUI: GUI backend using RmlUI
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

#ifndef GUCEF_guidriverRmlUI_CINPUTADAPTER_H
#define GUCEF_guidriverRmlUI_CINPUTADAPTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <RmlUI/Core.h>

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class which routes input events to RmlUI
 */
class GUCEF_guidriverRmlUI_PUBLIC_CPP CInputAdapter : public CORE::CObserver
{
    public:

	CInputAdapter();

	virtual ~CInputAdapter();

    void SetRmlUIContext( RmlUI::Core::Context* RmlUIContext );

    RmlUI::Core::Context* GetRmlUIContext( void );

    void StartListningForInputEvents( void );

    void StopListningForInputEvents( void );

    void SetInputContext( INPUT::CInputContext* inputContext );

    INPUT::CInputContext* GetInputContext( void ) const;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL );

    private:

    void OnMouseEvent( INPUT::CMouse* mouse         ,
                       const CORE::CEvent& eventid  ,
                       CORE::CICloneable* eventdata );

    void OnKeyboardEvent( INPUT::CKeyboard* mouse      ,
                          const CORE::CEvent& eventid  ,
                          CORE::CICloneable* eventdata );

    static int MapKeyModifierState( const INPUT::UInt32 keyModifierState );

    static RmlUI::Core::Input::KeyIdentifier MapKeyIdentifier( const INPUT::KeyCode keyCode );
                           
    private:

    INPUT::CInputContext* m_inputContext;
    RmlUI::Core::Context* m_RmlUIContext;
    int m_lastKeyModifierState;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_guidriverRmlUI_CINPUTADAPTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/

