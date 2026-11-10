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

#include "gucefCOMCORE_CRpcFieldDescriptor.h"

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

CRpcFieldDescriptor::CRpcFieldDescriptor( void )
    : m_number( 0 )
    , m_name()
    , m_type( RPC_FIELDTYPE_INVALID )
    , m_cardinality( RPC_CARDINALITY_SINGLE )
    , m_mapKeyType( RPC_FIELDTYPE_INVALID )
    , m_typeRef()
    , m_messageType( GUCEF_NULL )
    , m_enumType( GUCEF_NULL )
    , m_decimalScale( -1 )
    , m_flags( 0 )
    , m_since( 0 )
    , m_slotIndex( 0 )
    , m_wireType( GUCEF_RPC_WIRETYPE_LEN )
    , m_doc()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcFieldDescriptor::~CRpcFieldDescriptor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CRpcFieldDescriptor::GetNumber( void ) const
{GUCEF_TRACE;

    return m_number;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcFieldDescriptor::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

TRpcFieldType
CRpcFieldDescriptor::GetType( void ) const
{GUCEF_TRACE;

    return m_type;
}

/*-------------------------------------------------------------------------*/

TRpcCardinality
CRpcFieldDescriptor::GetCardinality( void ) const
{GUCEF_TRACE;

    return m_cardinality;
}

/*-------------------------------------------------------------------------*/

TRpcFieldType
CRpcFieldDescriptor::GetMapKeyType( void ) const
{GUCEF_TRACE;

    return m_mapKeyType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcFieldDescriptor::GetTypeRef( void ) const
{GUCEF_TRACE;

    return m_typeRef;
}

/*-------------------------------------------------------------------------*/

const CRpcTypeDescriptor*
CRpcFieldDescriptor::GetMessageType( void ) const
{GUCEF_TRACE;

    return m_messageType;
}

/*-------------------------------------------------------------------------*/

const CRpcEnumDescriptor*
CRpcFieldDescriptor::GetEnumType( void ) const
{GUCEF_TRACE;

    return m_enumType;
}

/*-------------------------------------------------------------------------*/

Int32
CRpcFieldDescriptor::GetDecimalScale( void ) const
{GUCEF_TRACE;

    return m_decimalScale;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcFieldDescriptor::GetFlags( void ) const
{GUCEF_TRACE;

    return m_flags;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcFieldDescriptor::GetSince( void ) const
{GUCEF_TRACE;

    return m_since;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcFieldDescriptor::GetSlotIndex( void ) const
{GUCEF_TRACE;

    return m_slotIndex;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcFieldDescriptor::GetDoc( void ) const
{GUCEF_TRACE;

    return m_doc;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcFieldDescriptor::GetWireType( void ) const
{GUCEF_TRACE;

    return m_wireType;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/

bool
CRpcFieldDescriptor::IsRepeated( void ) const
{GUCEF_TRACE;

    return RPC_CARDINALITY_REPEATED == m_cardinality;
}

/*-------------------------------------------------------------------------*/

bool
CRpcFieldDescriptor::IsMap( void ) const
{GUCEF_TRACE;

    return RPC_CARDINALITY_MAP == m_cardinality;
}

/*-------------------------------------------------------------------------*/

bool
CRpcFieldDescriptor::IsSingular( void ) const
{GUCEF_TRACE;

    return RPC_CARDINALITY_SINGLE == m_cardinality || RPC_CARDINALITY_OPTIONAL == m_cardinality;
}

/*-------------------------------------------------------------------------*/

bool
CRpcFieldDescriptor::IsRequired( void ) const
{GUCEF_TRACE;

    return 0 != ( m_flags & GUCEF_RPC_FIELDFLAG_REQUIRED );
}

/*-------------------------------------------------------------------------*/

bool
CRpcFieldDescriptor::IsSensitive( void ) const
{GUCEF_TRACE;

    return 0 != ( m_flags & GUCEF_RPC_FIELDFLAG_SENSITIVE );
}

/*-------------------------------------------------------------------------*/

bool
CRpcFieldDescriptor::IsFixedScaleDecimal( void ) const
{GUCEF_TRACE;

    return RPC_FIELDTYPE_DECIMAL == m_type && m_decimalScale >= 0;
}

/*-------------------------------------------------------------------------*/

bool
CRpcFieldDescriptor::IsPacked( void ) const
{GUCEF_TRACE;

    return IsRepeated() && GUCEF_RPC_WIRETYPE_LEN != m_wireType;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
