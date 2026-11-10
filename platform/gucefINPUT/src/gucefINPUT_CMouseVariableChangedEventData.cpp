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

#include "gucefINPUT_CMouseVariableChangedEventData.h"

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

CMouseVariableChangedEventData::CMouseVariableChangedEventData( const Int32 contextId, const UInt32 deviceID, const UInt32 variableIndex, const Int32 value, const Int32 valueDelta )
    : m_contextId( contextId )
    , m_deviceID( deviceID )
    , m_variableIndex( variableIndex )
    , m_value( value )
    , m_valueDelta( valueDelta )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMouseVariableChangedEventData::~CMouseVariableChangedEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Int32
CMouseVariableChangedEventData::GetContextId( void ) const
{GUCEF_TRACE;

    return m_contextId;
}

/*-------------------------------------------------------------------------*/

UInt32
CMouseVariableChangedEventData::GetDeviceID( void ) const
{GUCEF_TRACE;

    return m_deviceID;
}

/*-------------------------------------------------------------------------*/

UInt32
CMouseVariableChangedEventData::GetVariableIndex( void ) const
{GUCEF_TRACE;

    return m_variableIndex;
}

/*-------------------------------------------------------------------------*/

Int32
CMouseVariableChangedEventData::GetValue( void ) const
{GUCEF_TRACE;

    return m_value;
}

/*-------------------------------------------------------------------------*/

Int32
CMouseVariableChangedEventData::GetValueDelta( void ) const
{GUCEF_TRACE;

    return m_valueDelta;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMouseVariableChangedEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMouseVariableChangedEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
