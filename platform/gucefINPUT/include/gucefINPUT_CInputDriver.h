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

#ifndef GUCEF_INPUT_CINPUTDRIVER_H
#define GUCEF_INPUT_CINPUTDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ESTRUCTS_H
#include "EStructs.h"
#define ESTRUCTS_H
#endif /* ESTRUCTS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef CINPUTCONTEXT_H
#include "CInputContext.h"
#define CINPUTCONTEXT_H
#endif /* CINPUTCONTEXT_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF{ namespace CORE { class CValueList; } }

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
class CInputController;

/*-------------------------------------------------------------------------*/

class GUCEF_INPUT_PUBLIC_CPP CInputDriver
{
    public:
    
    CInputDriver( void );

    virtual ~CInputDriver();        
    
    virtual const CORE::TVersion* GetVersion( void ) = 0;

    virtual CString GetName( void ) const = 0;

    /**
     *  Attempts to convert the given KeyCode into a unicode 
     *  compatible value.
     *
     *  @param keyCode the keyCode to translate into unicode
     *  @param keyModifiers placeholder for the key modifier flags
     *  @param unicode the output variable for the unicode translation
     *  @return whether the KeyCode could be converted 
     */
    virtual bool GetUnicodeForKeyCode( const KeyCode keyCode     ,
                                       const UInt32 keyModifiers ,
                                       UInt32& unicode           ) const;
    
    protected:
    friend class CInputController;
    
    virtual bool OnUpdate( const UInt64 tickcount               ,
                           const Float64 updateDeltaInMilliSecs ,
                           CInputContext* context               ) = 0;        
    
    virtual CInputContext* CreateContext( const CORE::CValueList& params ) = 0;
    
    virtual void DeleteContext( CInputContext* context ) = 0;

    protected:
    
    void InjectMouseButtonChange( const Int32 contextId    ,  
                                  const Int32 deviceIndex  ,
                                  const UInt32 buttonIndex ,
                                  const bool buttonPressed );

    void InjectMouseMove( const Int32 contextId   ,
                          const Int32 deviceIndex ,
                          const Int32 xPos        ,
                          const Int32 yPos        ,
                          const Int32 xDelta      ,
                          const Int32 yDelta      );

    void InjectKeyboardKeyChange( const Int32 contextId   ,
                                  const Int32 deviceIndex ,
                                  const KeyCode keyCode   ,
                                  const bool keyPressed   );

    /** A variable of a mouse changed, see CMouse::EMouseVariable (the wheel: 'valueDelta' notches, up is positive) */
    void InjectMouseVariableChange( const Int32 contextId     ,
                                    const Int32 deviceIndex   ,
                                    const UInt32 variableIndex ,
                                    const Int32 value         ,
                                    const Int32 valueDelta    );

    /** Text typed on a keyboard, UTF-8 */
    void InjectKeyboardText( const Int32 contextId   ,
                             const Int32 deviceIndex ,
                             const CString& utf8Text );

    void AddMouse( const Int32 deviceID );
    
    void RemoveMouse( const Int32 deviceID );
    
    void AddKeyboard( const Int32 deviceID );
    
    void RemoveKeyboard( const Int32 deviceID );
    
    void AddDevice( const Int32 deviceID );
    
    void RemoveDevice( const Int32 deviceID );
                          
    private:
    
    CInputController* m_controller;                              
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CInputDriver, MT::CMutex > TInputDriverPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_INPUT_CINPUTDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 15-10-2007 :
        - Reworked class into a base class that interfaces with the controller
- 13-09-2005 :
        - Initial implementation

-----------------------------------------------------------------------------*/
