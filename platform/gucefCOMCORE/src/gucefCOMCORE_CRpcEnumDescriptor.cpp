/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcEnumDescriptor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcEnumDescriptor::SValue::SValue( void )
    : number( 0 )
    , name()
    , since( 0 )
    , doc()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcEnumDescriptor::CRpcEnumDescriptor( void )
    : m_name()
    , m_fullName()
    , m_doc()
    , m_values()
    , m_reservedNumbers()
    , m_reservedNames()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcEnumDescriptor::~CRpcEnumDescriptor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcEnumDescriptor::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcEnumDescriptor::GetFullName( void ) const
{GUCEF_TRACE;

    return m_fullName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcEnumDescriptor::GetDoc( void ) const
{GUCEF_TRACE;

    return m_doc;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcEnumDescriptor::GetValueCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_values.size();
}

/*-------------------------------------------------------------------------*/

const CRpcEnumDescriptor::SValue&
CRpcEnumDescriptor::GetValueAt( UInt32 index ) const
{GUCEF_TRACE;

    return m_values[ index ];
}

/*-------------------------------------------------------------------------*/

const CRpcEnumDescriptor::SValue*
CRpcEnumDescriptor::FindValueByNumber( Int32 number ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_values.size(); ++i )
        if ( m_values[ i ].number == number )
            return &m_values[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcEnumDescriptor::SValue*
CRpcEnumDescriptor::FindValueByName( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_values.size(); ++i )
        if ( m_values[ i ].name == name )
            return &m_values[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcEnumDescriptor::TInt32Set&
CRpcEnumDescriptor::GetReservedNumbers( void ) const
{GUCEF_TRACE;

    return m_reservedNumbers;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CRpcEnumDescriptor::GetReservedNames( void ) const
{GUCEF_TRACE;

    return m_reservedNames;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
