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

#include "gucefCOMCORE_CRpcServiceDescriptor.h"

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

CRpcServiceDescriptor::CRpcServiceDescriptor( void )
    : m_number( 0 )
    , m_name()
    , m_fullName()
    , m_since( 0 )
    , m_doc()
    , m_methods()
    , m_reservedNumbers()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcServiceDescriptor::~CRpcServiceDescriptor()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_methods.size(); ++i )
        GUCEF_DELETE m_methods[ i ];
    m_methods.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcServiceDescriptor::GetNumber( void ) const
{GUCEF_TRACE;

    return m_number;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcServiceDescriptor::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcServiceDescriptor::GetFullName( void ) const
{GUCEF_TRACE;

    return m_fullName;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcServiceDescriptor::GetSince( void ) const
{GUCEF_TRACE;

    return m_since;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcServiceDescriptor::GetDoc( void ) const
{GUCEF_TRACE;

    return m_doc;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcServiceDescriptor::GetMethodCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_methods.size();
}

/*-------------------------------------------------------------------------*/

const CRpcMethodDescriptor&
CRpcServiceDescriptor::GetMethodAt( UInt32 index ) const
{GUCEF_TRACE;

    return *m_methods[ index ];
}

/*-------------------------------------------------------------------------*/

const CRpcMethodDescriptor*
CRpcServiceDescriptor::FindMethodByNumber( UInt32 number ) const
{GUCEF_TRACE;

    // sorted by number: binary search
    UInt32 low = 0;
    UInt32 high = (UInt32) m_methods.size();
    while ( low < high )
    {
        UInt32 mid = ( low + high ) / 2;
        UInt32 n = m_methods[ mid ]->GetNumber();
        if ( n == number )
            return m_methods[ mid ];
        if ( n < number )
            low = mid + 1;
        else
            high = mid;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcMethodDescriptor*
CRpcServiceDescriptor::FindMethodByName( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_methods.size(); ++i )
        if ( m_methods[ i ]->GetName() == name )
            return m_methods[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcServiceDescriptor::TUInt32Set&
CRpcServiceDescriptor::GetReservedNumbers( void ) const
{GUCEF_TRACE;

    return m_reservedNumbers;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
