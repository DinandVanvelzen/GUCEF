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

#include "gucefCOMCORE_CRpcTypeDescriptor.h"

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

CRpcTypeDescriptor::CRpcTypeDescriptor( void )
    : m_name()
    , m_fullName()
    , m_doc()
    , m_fields()
    , m_denseSlots()
    , m_sparseSlots()
    , m_nameSlots()
    , m_reservedNumbers()
    , m_reservedNames()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcTypeDescriptor::~CRpcTypeDescriptor()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_fields.size(); ++i )
        GUCEF_DELETE m_fields[ i ];
    m_fields.clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcTypeDescriptor::BuildLookups( void )
{GUCEF_TRACE;

    m_denseSlots.clear();
    m_sparseSlots.clear();
    m_nameSlots.clear();

    UInt32 maxNumber = 0;
    for ( UInt32 i=0; i<m_fields.size(); ++i )
    {
        if ( m_fields[ i ]->GetNumber() > maxNumber )
            maxNumber = m_fields[ i ]->GetNumber();
        m_nameSlots[ m_fields[ i ]->GetName() ] = i;
    }

    // A dense table when it costs at most a few entries per field, a map otherwise
    if ( maxNumber <= 4 * (UInt32) m_fields.size() + 64 )
    {
        m_denseSlots.resize( maxNumber + 1, -1 );
        for ( UInt32 i=0; i<m_fields.size(); ++i )
            m_denseSlots[ m_fields[ i ]->GetNumber() ] = (Int32) i;
    }
    else
    {
        for ( UInt32 i=0; i<m_fields.size(); ++i )
            m_sparseSlots[ m_fields[ i ]->GetNumber() ] = i;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcTypeDescriptor::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcTypeDescriptor::GetFullName( void ) const
{GUCEF_TRACE;

    return m_fullName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcTypeDescriptor::GetDoc( void ) const
{GUCEF_TRACE;

    return m_doc;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcTypeDescriptor::GetFieldCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_fields.size();
}

/*-------------------------------------------------------------------------*/

const CRpcFieldDescriptor&
CRpcTypeDescriptor::GetFieldAt( UInt32 slotIndex ) const
{GUCEF_TRACE;

    return *m_fields[ slotIndex ];
}

/*-------------------------------------------------------------------------*/

const CRpcFieldDescriptor*
CRpcTypeDescriptor::FindFieldByNumber( UInt32 number ) const
{GUCEF_TRACE;

    if ( !m_denseSlots.empty() || m_sparseSlots.empty() )
    {
        if ( number < m_denseSlots.size() )
        {
            Int32 slot = m_denseSlots[ number ];
            if ( slot >= 0 )
                return m_fields[ (UInt32) slot ];
        }
        return GUCEF_NULL;
    }
    TNumberSlotMap::const_iterator i = m_sparseSlots.find( number );
    if ( i != m_sparseSlots.end() )
        return m_fields[ (*i).second ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcFieldDescriptor*
CRpcTypeDescriptor::FindFieldByName( const CORE::CString& name ) const
{GUCEF_TRACE;

    TNameSlotMap::const_iterator i = m_nameSlots.find( name );
    if ( i != m_nameSlots.end() )
        return m_fields[ (*i).second ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CRpcTypeDescriptor::TUInt32Set&
CRpcTypeDescriptor::GetReservedNumbers( void ) const
{GUCEF_TRACE;

    return m_reservedNumbers;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CRpcTypeDescriptor::GetReservedNames( void ) const
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
