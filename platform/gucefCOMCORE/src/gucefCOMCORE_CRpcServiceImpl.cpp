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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#include "gucefCOMCORE_CRpcServiceImpl.h"

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

CRpcServiceImpl::CRpcServiceImpl( void )
    : CIRpcServiceImpl()
    , m_lock()
    , m_handlers()
    , m_modes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcServiceImpl::~CRpcServiceImpl()
{GUCEF_TRACE;

    THandlerMap::iterator i = m_handlers.begin();
    for ( ; i != m_handlers.end(); ++i )
        if ( (*i).second.owned )
            GUCEF_DELETE (*i).second.handler;
    m_handlers.clear();
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceImpl::BindMethod( const CORE::CString& methodName, CIRpcMethodHandler* handler, bool takeOwnership )
{GUCEF_TRACE;

    if ( GUCEF_NULL == handler )
        return false;
    MT::CScopeMutex lock( m_lock );
    THandlerMap::iterator i = m_handlers.find( methodName );
    if ( i != m_handlers.end() && (*i).second.owned && (*i).second.handler != handler )
        GUCEF_DELETE (*i).second.handler;
    SHandler entry;
    entry.handler = handler;
    entry.owned = takeOwnership;
    m_handlers[ methodName ] = entry;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcServiceImpl::UnbindMethod( const CORE::CString& methodName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    THandlerMap::iterator i = m_handlers.find( methodName );
    if ( i == m_handlers.end() )
        return;
    if ( (*i).second.owned )
        GUCEF_DELETE (*i).second.handler;
    m_handlers.erase( i );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceImpl::GetExecutionMode( const CORE::CString& methodName, TRpcExecutionMode& mode ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TModeMap::const_iterator i = m_modes.find( methodName );
    if ( i == m_modes.end() )
        return false;
    mode = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcServiceImpl::SetExecutionMode( const CORE::CString& methodName, TRpcExecutionMode mode )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_modes[ methodName ] = mode;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceImpl::GetMethodExecutionMode( const CRpcMethodDescriptor& method, TRpcExecutionMode& mode ) const
{GUCEF_TRACE;

    return GetExecutionMode( method.GetName(), mode );
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceImpl::HasHandler( const CRpcMethodDescriptor& method ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_handlers.find( method.GetName() ) != m_handlers.end();
}

/*-------------------------------------------------------------------------*/

TRpcHandlerResult
CRpcServiceImpl::OnCall( CRpcServerCall& call )
{GUCEF_TRACE;

    CIRpcMethodHandler* handler = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        THandlerMap::iterator i = m_handlers.find( call.GetMethod().GetName() );
        if ( i != m_handlers.end() )
            handler = (*i).second.handler;
    }
    if ( GUCEF_NULL == handler )
    {
        call.Fail( CRpcStatus( RPC_STATUS_UNIMPLEMENTED, "no handler for " + call.GetMethod().GetPath() ) );
        return RPC_HANDLER_DONE;
    }
    return handler->OnCall( call );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
