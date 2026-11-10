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

#include "gucefINPUT_CKeyboardTextEventData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKeyboardTextEventData::CKeyboardTextEventData( const Int32 contextId, const UInt32 deviceID, const CString& text )
    : m_contextId( contextId )
    , m_deviceID( deviceID )
    , m_text( text )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyboardTextEventData::~CKeyboardTextEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Int32
CKeyboardTextEventData::GetContextId( void ) const
{GUCEF_TRACE;

    return m_contextId;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyboardTextEventData::GetDeviceID( void ) const
{GUCEF_TRACE;

    return m_deviceID;
}

/*-------------------------------------------------------------------------*/

const CString&
CKeyboardTextEventData::GetText( void ) const
{GUCEF_TRACE;

    return m_text;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CKeyboardTextEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKeyboardTextEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
