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

#ifndef GUCEF_INPUT_CKEYBOARD_H
#define GUCEF_INPUT_CKEYBOARD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_INPUT_CABSTRACTINPUTDEVICE_H
#include "gucefINPUT_CAbstractInputDevice.h"
#define GUCEF_INPUT_CABSTRACTINPUTDEVICE_H
#endif /* GUCEF_INPUT_CABSTRACTINPUTDEVICE_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#include "gucefINPUT_CKeyStateChangedEventData.h"
#define GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H
#include "gucefINPUT_CKeyModStateChangedEventData.h"
#define GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H ? */

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

class CInputController;

/*-------------------------------------------------------------------------*/

/**
 *  In-software representation of a keyboard hardware input device
 */
class GUCEF_INPUT_PUBLIC_CPP CKeyboard : public CAbstractInputDevice
{
    public:
    
    static const CORE::CEvent KeyStateChangedEvent;      /**< event signaling that the state of a key changed, eventdata = CKeyStateChangedEventData */
    static const CORE::CEvent KeyModStateChangedEvent;   /**< event signaling that the state of a key modifier changed, eventdata = CKeyModStateChangedEventData */
    static const CORE::CEvent TextInputEvent;            /**< text was typed (after keyboard layout and input method), eventdata = CKeyboardTextEventData */
    
    static void RegisterEvents( void );

    static const char* DeviceType;     /** string representing the type of device */
    
    public:
    
    typedef std::map< KeyCode, bool > TKeyStates;
    
    bool IsKeyDown( const KeyCode key ) const;
    
    bool IsKeyModifierActive( const KeyModifier keyMod ) const;
    
    const TKeyStates& GetKeyStates( void ) const;
    
    UInt32 GetKeyModStates( void ) const;
    
    /**
     *  Attempts to convert the given KeyCode into a unicode 
     *  compatible value. Note that the conversion method is
     *  determined by the input driver used.
     *
     *  @param keyCode the keyCode to translate into unicode
     *  @param keyModifiers placeholder for the key modifier flags
     *  @param unicode the output variable for the unicode translation
     *  @return whether the KeyCode could be converted 
     */
    bool GetUnicodeForKeyCode( const KeyCode keyCode     ,
                               const UInt32 keyModifiers ,
                               UInt32& unicode           ) const;

    virtual const char* GetDeviceType( void ) const;

    virtual bool IsDeviceType( const char* deviceType ) const;

    virtual CInputDriver* GetInputDriver( void );
    
    virtual const CString& GetClassTypeName( void ) const;
    
    private:
    friend class CInputController;
    
    CKeyboard( CInputDriver& inputDriver ,
               const UInt32 deviceID     );
    
    virtual ~CKeyboard();
    
    void SetKeyState( const Int32 contextId   ,
                      const KeyCode key       , 
                      const bool pressedState );

    void SetKeyModState( const KeyModifier keyMod , 
                         const bool pressedState  );

    void SetText( const Int32 contextId ,
                  const CString& text   );
                         
    void ResetKeyboardState( void );
       
    private:
    
    CKeyboard( const CKeyboard& src );
    CKeyboard& operator=( const CKeyboard& src );
    
    private:
    
    Int32 m_contextId;
    UInt32 m_deviceID;
    TKeyStates m_keyState;
    UInt32 m_keyModStates;
    CInputDriver* m_inputDriver;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_INPUT_CKEYBOARD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 28-09-2007 :
        - Initial implementation

-----------------------------------------------------------------------------*/
