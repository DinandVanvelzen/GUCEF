/*
 *  gucefINPUT: GUCEF module providing input device interaction
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

#ifndef GUCEF_INPUT_CINPUTACTIONMAP_H
#define GUCEF_INPUT_CINPUTACTIONMAP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF{ namespace CORE { class CDataNode; } }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class for easy mapping of various input events to generic action ID's
 *  If you build your clients to be triggered by actions rather then specific
 *  input device events you will be able to customize the mappings with ease.
 */
class GUCEF_INPUT_PUBLIC_CPP CInputActionMap : public CORE::CObservingNotifier
{
    public:
    
    static const CORE::CEvent ActionEvent;
    
    static void RegisterEvents( void );
    
    public:
    
    typedef std::map< UInt32, UInt32 >                      TKeyStateModMapActionMap;
    typedef std::map< KeyCode, TKeyStateModMapActionMap >   TKeyStateActionMap;
    typedef std::map< UInt32, UInt32 >                      TStateActionMap;
    typedef std::map< UInt32, TStateActionMap >             TStateActionDeviceMap;
    
    public:
    
    CInputActionMap( const CORE::CString& name = "" );

    CInputActionMap( const CInputActionMap& src );
    
    virtual ~CInputActionMap();
    
    CInputActionMap& operator=( const CInputActionMap& src );
    
    TKeyStateActionMap& GetKeyboardKeyDownMap( void );

    TKeyStateActionMap& GetKeyboardKeyUpMap( void );
    
    TStateActionMap& GetMouseButtonDownMap( const UInt32 mouseIndex );

    TStateActionMap& GetMouseButtonUpMap( const UInt32 mouseIndex );
    
    TStateActionMap& GetDeviceBoolOnMap( const UInt32 deviceIndex );
    
    TStateActionMap& GetDeviceBoolOffMap( const UInt32 deviceIndex );
    
    TStateActionMap& GetDeviceVarChanged( const UInt32 deviceIndex );
                                        
    bool SaveConfig( CORE::CDataNode& tree ) const;
                                                           
    bool LoadConfig( const CORE::CDataNode& treeroot );
    
    void Clear( void );
    
    const CORE::CString& GetName( void ) const;
    
    protected:

    /**
     *  Event callback member function.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = GUCEF_NULL );

    private:
    
    TKeyStateActionMap m_keyUpMap;
    TKeyStateActionMap m_keyDownMap;
    TStateActionDeviceMap m_mouseButtonUpMap;
    TStateActionDeviceMap m_mouseButtonDownMap;    
    TStateActionDeviceMap m_deviceBoolOnMap; 
    TStateActionDeviceMap m_deviceBoolOffMap;
    TStateActionDeviceMap m_deviceVarChangeMap;    
    CORE::CString m_name;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_INPUT_CINPUTACTIONMAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 06-10-2005 :
        - Added utils for string aliases for the actions
- 16-09-2005 :
        - Initial implementation

-----------------------------------------------------------------------------*/