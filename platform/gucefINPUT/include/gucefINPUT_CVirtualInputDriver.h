/*
 *  gucefINPUT: GUCEF module providing input device interaction
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_INPUT_CVIRTUALINPUTDRIVER_H
#define GUCEF_INPUT_CVIRTUALINPUTDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

#ifndef GUCEF_INPUT_CINPUTDRIVER_H
#include "gucefINPUT_CInputDriver.h"
#define GUCEF_INPUT_CINPUTDRIVER_H
#endif /* GUCEF_INPUT_CINPUTDRIVER_H ? */

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
 *  An input driver for devices that exist only in software: an on screen keyboard, a script, a replay.
 *
 *  The devices it makes are the ordinary gucefINPUT devices of the input controller (CKeyboard), so the
 *  events they send are the same events a keyboard of the hardware sends: CKeyboard::KeyStateChangedEvent
 *  with the modifier state, CKeyboard::KeyModStateChangedEvent and CKeyboard::TextInputEvent. What receives
 *  them (CGUIInputAdapter, an input action map, code that subscribes to the keyboards) cannot tell the
 *  difference except by the device id and the driver of the device (CKeyboard::GetInputDriver()), which
 *  is how code that wants to know finds out.
 *
 *  Events carry an input context id, which is the window of the input driver that made them. A virtual
 *  device has no window: the sender says which input context its input is for (the id of the context
 *  of the window the GUI draws in), or -1 when it is for whoever listens.
 *
 *  The driver is not registered with the input controller and needs no input context; each instance
 *  keeps the devices it made and removes them when it goes.
 */
class GUCEF_INPUT_PUBLIC_CPP CVirtualInputDriver : public CInputDriver
{
    public:

    /** Virtual devices get ids from here upwards; real input drivers number their devices from 0 */
    static const Int32 FirstVirtualDeviceId;

    CVirtualInputDriver( void );

    virtual ~CVirtualInputDriver();

    virtual const CORE::TVersion* GetVersion( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Adds a keyboard to the input controller; its device id, or -1 when none could be made */
    Int32 CreateKeyboard( void );

    /** Removes a keyboard this driver made; false when it is none of them */
    bool DestroyKeyboard( Int32 deviceId );

    /** The keyboards of this driver that exist now */
    UInt32 GetKeyboardCount( void ) const;

    bool IsKeyboardOfThisDriver( Int32 deviceId ) const;

    /** A key went down or up on a keyboard of this driver */
    bool SendKey( Int32 inputContextId, Int32 deviceId, KeyCode keyCode, bool pressed );

    /** UTF-8 text typed on a keyboard of this driver, as a keyboard sends it after the keys that made it */
    bool SendText( Int32 inputContextId, Int32 deviceId, const CString& utf8Text );

    protected:

    virtual bool OnUpdate( const UInt64 tickcount               ,
                           const Float64 updateDeltaInMilliSecs ,
                           CInputContext* context               ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CInputContext* CreateContext( const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DeleteContext( CInputContext* context ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVirtualInputDriver( const CVirtualInputDriver& src );            /**< not implemented */
    CVirtualInputDriver& operator=( const CVirtualInputDriver& src ); /**< not implemented */

    typedef GUCEF::vector< Int32 > TDeviceIdVector;

    TDeviceIdVector m_keyboards;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_INPUT_CVIRTUALINPUTDRIVER_H ? */
