/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#include "gucefCOM_CMcpInputRequired.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpInputRequired::SInputRequest::SInputRequest( void )
    : method()
    , params( GUCEF_DATATYPE_OBJECT )
    , hasParams( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpInputRequired::CMcpInputRequired( void )
    : CORE::CICloneable()
    , m_inputRequests()
    , m_state()
    , m_hasState( false )
    , m_wantsStateNonce( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpInputRequired::CMcpInputRequired( const CMcpInputRequired& src )
    : CORE::CICloneable( src )
    , m_inputRequests( src.m_inputRequests )
    , m_state( src.m_state )
    , m_hasState( src.m_hasState )
    , m_wantsStateNonce( src.m_wantsStateNonce )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpInputRequired::~CMcpInputRequired()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpInputRequired&
CMcpInputRequired::operator=( const CMcpInputRequired& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_inputRequests = src.m_inputRequests;
        m_state = src.m_state;
        m_hasState = src.m_hasState;
        m_wantsStateNonce = src.m_wantsStateNonce;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpInputRequired&
CMcpInputRequired::AddFormElicitation( const CORE::CString& key, const CORE::CString& message, const CORE::CDataNode& requestedSchema )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "mode", CORE::CVariant( "form" ) );
    params.SetAttribute( "message", CORE::CVariant( message ) );
    CJsonRpcEndpoint::AddJsonMember( params, "requestedSchema", requestedSchema );
    return AddInputRequest( key, CMcpMethods::ElicitationCreate, params );
}

/*-------------------------------------------------------------------------*/

CMcpInputRequired&
CMcpInputRequired::AddInputRequest( const CORE::CString& key, const CORE::CString& method, const CORE::CDataNode& params )
{GUCEF_TRACE;

    SInputRequest& request = m_inputRequests[ key ];
    request.method = method;
    request.params = params;
    request.params.SetName( CORE::CString::Empty );
    request.hasParams = true;
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpInputRequired&
CMcpInputRequired::SetState( const CORE::CDataNode& handlerState )
{GUCEF_TRACE;

    m_state = handlerState;
    m_state.SetName( CORE::CString::Empty );
    m_hasState = true;
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpInputRequired&
CMcpInputRequired::SetStateNonce( void )
{GUCEF_TRACE;

    m_wantsStateNonce = true;
    if ( !m_hasState )
    {
        m_state = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
        m_hasState = true;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMcpInputRequired::Clear( void )
{GUCEF_TRACE;

    m_inputRequests.clear();
    m_state = CORE::CDataNode();
    m_hasState = false;
    m_wantsStateNonce = false;
}

/*-------------------------------------------------------------------------*/

const CMcpInputRequired::TInputRequestMap&
CMcpInputRequired::GetInputRequests( void ) const
{GUCEF_TRACE;

    return m_inputRequests;
}

/*-------------------------------------------------------------------------*/

bool
CMcpInputRequired::HasInputRequests( void ) const
{GUCEF_TRACE;

    return !m_inputRequests.empty();
}

/*-------------------------------------------------------------------------*/

bool
CMcpInputRequired::HasState( void ) const
{GUCEF_TRACE;

    return m_hasState;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CMcpInputRequired::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CMcpInputRequired::GetWantsStateNonce( void ) const
{GUCEF_TRACE;

    return m_wantsStateNonce;
}

/*-------------------------------------------------------------------------*/

void
CMcpInputRequired::GetRequiredCapabilities( CORE::CDataNode& capabilities ) const
{GUCEF_TRACE;

    bool needsForm = false;
    bool needsUrl = false;
    bool needsSampling = false;
    bool needsRoots = false;
    for ( TInputRequestMap::const_iterator i = m_inputRequests.begin(); i != m_inputRequests.end(); ++i )
    {
        const SInputRequest& request = (*i).second;
        if ( request.method == CMcpMethods::ElicitationCreate )
        {
            // a request without a mode is form mode
            const CORE::CDataNode::TKeyValuePair* mode = request.params.GetAttribute( "mode" );
            if ( GUCEF_NULL != mode && (*mode).second.AsString() == "url" )
                needsUrl = true;
            else
                needsForm = true;
        }
        else
        if ( request.method == CMcpMethods::SamplingCreateMessage )
            needsSampling = true;
        else
        if ( request.method == CMcpMethods::RootsList )
            needsRoots = true;
    }

    capabilities.Clear();
    capabilities.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( needsForm || needsUrl )
    {
        // an empty elicitation object means form mode only, so form is named once url is named
        CORE::CDataNode* elicitation = capabilities.AddChild( "elicitation", GUCEF_DATATYPE_OBJECT );
        if ( needsUrl )
        {
            if ( needsForm )
                elicitation->AddChild( "form", GUCEF_DATATYPE_OBJECT );
            elicitation->AddChild( "url", GUCEF_DATATYPE_OBJECT );
        }
    }
    if ( needsRoots )
        capabilities.AddChild( "roots", GUCEF_DATATYPE_OBJECT );
    if ( needsSampling )
        capabilities.AddChild( "sampling", GUCEF_DATATYPE_OBJECT );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMcpInputRequired::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMcpInputRequired( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMcpInputRequired::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMcpInputRequired::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CMcpInputRequired";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
