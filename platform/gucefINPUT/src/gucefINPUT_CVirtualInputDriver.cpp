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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#include "gucefINPUT_CVirtualInputDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const Int32 CVirtualInputDriver::FirstVirtualDeviceId = 0x10000000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVirtualInputDriver::CVirtualInputDriver( void )
    : CInputDriver()
    , m_keyboards()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVirtualInputDriver::~CVirtualInputDriver()
{GUCEF_TRACE;

    // a copy: every removal changes the list
    TDeviceIdVector keyboards = m_keyboards;
    for ( TDeviceIdVector::iterator i=keyboards.begin(); i!=keyboards.end(); ++i )
        DestroyKeyboard( *i );
}

/*-------------------------------------------------------------------------*/

const CORE::TVersion*
CVirtualInputDriver::GetVersion( void )
{GUCEF_TRACE;

    static CORE::TVersion version = { 1, 0, 0, 0 };
    return &version;
}

/*-------------------------------------------------------------------------*/

CString
CVirtualInputDriver::GetName( void ) const
{GUCEF_TRACE;

    return "Virtual";
}

/*-------------------------------------------------------------------------*/

Int32
CVirtualInputDriver::CreateKeyboard( void )
{GUCEF_TRACE;

    // the first id from the base that no device of the controller has, whichever driver made it
    CInputController& controller = CInputGlobal::Instance()->GetInputController();
    const CInputController::TKeyboardMap& keyboards = controller.GetKeyboardMap();
    Int32 deviceId = FirstVirtualDeviceId;
    while ( keyboards.find( deviceId ) != keyboards.end() )
        ++deviceId;

    AddKeyboard( deviceId );
    if ( keyboards.find( deviceId ) == keyboards.end() )
        return -1;
    m_keyboards.push_back( deviceId );
    return deviceId;
}

/*-------------------------------------------------------------------------*/

bool
CVirtualInputDriver::DestroyKeyboard( Int32 deviceId )
{GUCEF_TRACE;

    TDeviceIdVector::iterator i = std::find( m_keyboards.begin(), m_keyboards.end(), deviceId );
    if ( i == m_keyboards.end() )
        return false;
    m_keyboards.erase( i );
    RemoveKeyboard( deviceId );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVirtualInputDriver::GetKeyboardCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_keyboards.size();
}

/*-------------------------------------------------------------------------*/

bool
CVirtualInputDriver::IsKeyboardOfThisDriver( Int32 deviceId ) const
{GUCEF_TRACE;

    return std::find( m_keyboards.begin(), m_keyboards.end(), deviceId ) != m_keyboards.end();
}

/*-------------------------------------------------------------------------*/

bool
CVirtualInputDriver::SendKey( Int32 inputContextId, Int32 deviceId, KeyCode keyCode, bool pressed )
{GUCEF_TRACE;

    if ( !IsKeyboardOfThisDriver( deviceId ) )
        return false;
    InjectKeyboardKeyChange( inputContextId, deviceId, keyCode, pressed );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVirtualInputDriver::SendText( Int32 inputContextId, Int32 deviceId, const CString& utf8Text )
{GUCEF_TRACE;

    if ( !IsKeyboardOfThisDriver( deviceId ) )
        return false;
    InjectKeyboardText( inputContextId, deviceId, utf8Text );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVirtualInputDriver::OnUpdate( const UInt64 tickcount               ,
                               const Float64 updateDeltaInMilliSecs ,
                               CInputContext* context               )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

CInputContext*
CVirtualInputDriver::CreateContext( const CORE::CValueList& params )
{GUCEF_TRACE;

    // no window: the devices send for the input context of the sender
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CVirtualInputDriver::DeleteContext( CInputContext* context )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
