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

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#define GUCEF_INPUT_CINPUTCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <set>

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CTNUMERICIDGENERATOR_H
#include "CTNumericIDGenerator.h"
#define GUCEF_CORE_CTNUMERICIDGENERATOR_H
#endif /* GUCEF_CORE_CTNUMERICIDGENERATOR_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_INPUT_CKEYBOARD_H
#include "gucefINPUT_CKeyboard.h"
#define GUCEF_INPUT_CKEYBOARD_H
#endif /* GUCEF_INPUT_CKEYBOARD_H ? */

#ifndef GUCEF_INPUT_CMOUSE_H
#include "gucefINPUT_CMouse.h"
#define GUCEF_INPUT_CMOUSE_H
#endif /* GUCEF_INPUT_CMOUSE_H ? */

#ifndef GUCEF_INPUT_CINPUTDRIVER_H
#include "gucefINPUT_CInputDriver.h"
#define GUCEF_INPUT_CINPUTDRIVER_H
#endif /* GUCEF_INPUT_CINPUTDRIVER_H ? */

#ifndef GUCEF_INPUT_CINPUTDRIVERPLUGINMANAGER_H
#include "gucefINPUT_CInputDriverPluginManager.h"
#define GUCEF_INPUT_CINPUTDRIVERPLUGINMANAGER_H
#endif /* GUCEF_INPUT_CINPUTDRIVERPLUGINMANAGER_H ? */

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

class CInputContext;
class CInputDriver;

/*-------------------------------------------------------------------------*/

class GUCEF_INPUT_PUBLIC_CPP CInputController : public CORE::CTSGNotifier
{
    public:                

    static const CORE::CEvent InputDriverLoadedEvent;
    static const CORE::CEvent InputDriverUnloadedEvent;
    static const CORE::CEvent MouseAttachedEvent;
    static const CORE::CEvent MouseDetachedEvent;
    static const CORE::CEvent KeyboardAttachedEvent;
    static const CORE::CEvent KeyboardDetachedEvent;
    static const CORE::CEvent JoystickAttachedEvent;
    static const CORE::CEvent JoystickDetachedEvent;

    typedef CORE::TCloneableInt32 TMouseAttachedEventData; 
    typedef CORE::TCloneableInt32 TMouseDetachedEventData;
    typedef CORE::TCloneableInt32 TKeyboardAttachedEventData; 
    typedef CORE::TCloneableInt32 TKeyboardDetachedEventData;
    typedef CORE::TCloneableInt32 TJoystickAttachedEventData;
    typedef CORE::TCloneableInt32 TJoystickDetachedEventData;
    
    static void RegisterEvents( void );
    
    public:

    typedef std::map< Int32, CMouse* >      TMouseMap;  
    typedef std::map< Int32, CKeyboard* >   TKeyboardMap;
    typedef std::set< CString >             TStringSet;              
    
    /**
     *  Create the context using the specified driver
     */
    CInputContext* CreateContext( const CString& driverName      ,
                                  const CORE::CValueList& params );
    
    /**
     *  Create the context using the default driver
     */
    CInputContext* CreateContext( const CORE::CValueList& params );
    
    void DestroyContext( CInputContext* context );
  
    UInt32 GetContextCount( void ) const;

    void SetDefaultDriver( const CString& driverName );

    const CString& GetDefaultDriver( void ) const;

    TStringSet GetListOfAvailableDriversByName( void ) const;

    void RegisterDriver( TInputDriverPtr driver );

    void UnregisterDriver( const CString& driverName );
    
    bool SetDriver( TInputDriverPtr driver );
    
    TInputDriverPtr GetDriverByName( const CString& driverName );
            
    bool LoadDriverModule( const CORE::CString& filename  ,
                           const CORE::CValueList& params );

    CKeyboard& GetKeyboard( const Int32 deviceID );
    
    UInt32 GetKeyboardCount( void ) const;

    /**
     *  Helper function which allows you to easily subscribe to all
     *  currently connected keyboards which is the typically desired use-case.
     */
    void SubscribeToAllKeyboards( CORE::CObserver* keyboardObserver );
    
    const TKeyboardMap& GetKeyboardMap( void ) const;
    
    CMouse& GetMouse( const Int32 deviceID );
    
    UInt32 GetMouseCount( void ) const;
    
    const TMouseMap& GetMouseMap( void ) const;

    /**
     *  Helper function which allows you to easily subscribe to all
     *  currently connected mice which is the typically desired use-case.
     */
    void SubscribeToAllMice( CORE::CObserver* mouseObserver );

    /**
     *  Provides access to the plugin manager for input driver plugins
     */
    CInputDriverPluginManager& GetInputDriverPluginManager( void );
    
    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_INPUT_PUBLIC_CPP, EInvalidIndex );
    
    protected:
        
    /**
     *  Event callback member function.
     *  Implement this in your descending class to handle
     *  notification events.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnPumpedNotify( CORE::CNotifier* notifier           ,
                                 const CORE::CEvent& eventid         ,
                                 CORE::CICloneable* eventdata = NULL ) GUCEF_VIRTUAL_OVERRIDE;
        
    typedef CORE::CTEventHandlerFunctor< CInputController > TEventCallback;
    
    void OnPulse( CORE::CNotifier* notifier           ,
                  const CORE::CEvent& eventid         ,
                  CORE::CICloneable* eventdata = NULL );

    private:
    friend class CInputGlobal;
    
    CInputController( void );        

    virtual ~CInputController();

    private:
    friend class CInputDriver;
    
    void SetMouseButtonState( const Int32 contextId    ,
                              const Int32 deviceID     ,
                              const UInt32 buttonIndex , 
                              const bool pressedState  );

    void SetMousePos( const Int32 contextId ,
                      const Int32 deviceID  ,
                      const UInt32 xPos     ,
                      const UInt32 yPos     ,
                      const Int32 xDelta    ,
                      const Int32 yDelta    );
                         
    void ResetMouseStates( const Int32 deviceID );    

    void SetKeyboardKeyState( const Int32 contextId ,
                              const Int32 deviceID  ,
                              const KeyCode keyCode ,
                              const bool keyPressed );

    void AddMouse( CInputDriver* inputDriver, const Int32 deviceID );
    
    void RemoveMouse( const Int32 deviceID );
    
    void AddKeyboard( CInputDriver* inputDriver, const Int32 deviceID );
    
    void RemoveKeyboard( const Int32 deviceID );
    
    void AddDevice( CInputDriver* inputDriver, const Int32 deviceID );
    
    void RemoveDevice( const Int32 deviceID );
    
    private:    
    
    CInputController( const CInputController& src );            /** <- not implemented */
    CInputController& operator=( const CInputController& src ); /** <- not implemented */
    
    private:
    typedef std::set< CInputContext* > TContextSet;
    typedef std::map< CString, TInputDriverPtr >    TInputDriverMap;
    typedef CORE::CTNumericIDGenerator< CORE::Int32, MT::CMutex > TIdGenerator;
        
    static CInputController* m_instance;
    
    TIdGenerator m_idGenerator;
    TContextSet m_contextSet;
    TKeyboardMap m_keyboardMap;
    TMouseMap m_mouseMap;    
    CORE::PulseGeneratorPtr m_pulseGenerator;    
    #ifdef GUCEF_MSWIN_BUILD
    UInt64 m_hinstance;
    #endif    
    CInputDriverPluginManager m_inputDriverPluginManager;
    CString m_defaultDriver;
    TInputDriverMap m_inputdriverMap;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 13-09-2005 :
        - Initial implementation

-----------------------------------------------------------------------------*/
