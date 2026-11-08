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

#include "gucefCOMCORE_CRpcMethodDescriptor.h"

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

CRpcMethodDescriptor::CRpcMethodDescriptor( void )
    : m_number( 0 )
    , m_name()
    , m_path()
    , m_kind( RPC_METHODKIND_UNARY )
    , m_requestTypeName()
    , m_responseTypeName()
    , m_requestType( GUCEF_NULL )
    , m_responseType( GUCEF_NULL )
    , m_flags( 0 )
    , m_defaultDeadlineMs( 0 )
    , m_protection( RPC_PROTECTION_CONFIDENTIAL )
    , m_requestProtection( RPC_PROTECTION_CONFIDENTIAL )
    , m_initialCredit( 0 )
    , m_since( 0 )
    , m_doc()
    , m_service( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcMethodDescriptor::~CRpcMethodDescriptor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMethodDescriptor::GetNumber( void ) const
{GUCEF_TRACE;

    return m_number;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcMethodDescriptor::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcMethodDescriptor::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

TRpcMethodKind
CRpcMethodDescriptor::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcMethodDescriptor::GetRequestTypeName( void ) const
{GUCEF_TRACE;

    return m_requestTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcMethodDescriptor::GetResponseTypeName( void ) const
{GUCEF_TRACE;

    return m_responseTypeName;
}

/*-------------------------------------------------------------------------*/

const CRpcTypeDescriptor*
CRpcMethodDescriptor::GetRequestType( void ) const
{GUCEF_TRACE;

    return m_requestType;
}

/*-------------------------------------------------------------------------*/

const CRpcTypeDescriptor*
CRpcMethodDescriptor::GetResponseType( void ) const
{GUCEF_TRACE;

    return m_responseType;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMethodDescriptor::GetFlags( void ) const
{GUCEF_TRACE;

    return m_flags;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMethodDescriptor::GetDefaultDeadlineMs( void ) const
{GUCEF_TRACE;

    return m_defaultDeadlineMs;
}

/*-------------------------------------------------------------------------*/

TRpcProtectionClass
CRpcMethodDescriptor::GetProtection( void ) const
{GUCEF_TRACE;

    return m_protection;
}

/*-------------------------------------------------------------------------*/

TRpcProtectionClass
CRpcMethodDescriptor::GetRequestProtection( void ) const
{GUCEF_TRACE;

    return m_requestProtection;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMethodDescriptor::GetInitialCredit( void ) const
{GUCEF_TRACE;

    return m_initialCredit;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMethodDescriptor::GetSince( void ) const
{GUCEF_TRACE;

    return m_since;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcMethodDescriptor::GetDoc( void ) const
{GUCEF_TRACE;

    return m_doc;
}

/*-------------------------------------------------------------------------*/

const CRpcServiceDescriptor*
CRpcMethodDescriptor::GetService( void ) const
{GUCEF_TRACE;

    return m_service;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMethodDescriptor::IsIdempotent( void ) const
{GUCEF_TRACE;

    return 0 != ( m_flags & GUCEF_RPC_METHODFLAG_IDEMPOTENT );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMethodDescriptor::MayAnswerAsync( void ) const
{GUCEF_TRACE;

    return 0 != ( m_flags & GUCEF_RPC_METHODFLAG_MAYANSWERASYNC );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMethodDescriptor::IsResumableStream( void ) const
{GUCEF_TRACE;

    return 0 != ( m_flags & GUCEF_RPC_METHODFLAG_RESUMABLESTREAM );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMethodDescriptor::IsSensitive( void ) const
{GUCEF_TRACE;

    return 0 != ( m_flags & GUCEF_RPC_METHODFLAG_SENSITIVE );
}

/*-------------------------------------------------------------------------*/

bool
CRpcMethodDescriptor::IsStreaming( void ) const
{GUCEF_TRACE;

    return RPC_METHODKIND_UNARY != m_kind;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMethodDescriptor::HasClientStream( void ) const
{GUCEF_TRACE;

    return RPC_METHODKIND_CLIENT_STREAM == m_kind || RPC_METHODKIND_BIDI_STREAM == m_kind;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMethodDescriptor::HasServerStream( void ) const
{GUCEF_TRACE;

    return RPC_METHODKIND_SERVER_STREAM == m_kind || RPC_METHODKIND_BIDI_STREAM == m_kind;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
