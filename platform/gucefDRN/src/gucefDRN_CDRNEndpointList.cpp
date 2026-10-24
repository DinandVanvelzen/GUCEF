/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNEndpointList.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNEndpointList::ClassTypeName = "GUCEF::DRN::CDRNEndpointList";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNEndpointList::CDRNEndpointList( void )
    : CORE::CIConfigurable()
    , m_endpoints()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNEndpointList::CDRNEndpointList( const CDRNEndpoint& endpoint )
    : CORE::CIConfigurable()
    , m_endpoints()
{GUCEF_TRACE;

    m_endpoints.push_back( endpoint );
}

/*-------------------------------------------------------------------------*/

CDRNEndpointList::CDRNEndpointList( const CDRNEndpointList& src )
    : CORE::CIConfigurable( src )
    , m_endpoints( src.m_endpoints )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNEndpointList::~CDRNEndpointList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNEndpointList&
CDRNEndpointList::operator=( const CDRNEndpointList& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        m_endpoints = src.m_endpoints;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpointList::operator==( const CDRNEndpointList& other ) const
{GUCEF_TRACE;

    if ( m_endpoints.size() != other.m_endpoints.size() )
        return false;
    for ( UInt32 i=0; i<m_endpoints.size(); ++i )
        if ( m_endpoints[ i ] != other.m_endpoints[ i ] )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNEndpointList::Add( const CDRNEndpoint& endpoint )
{GUCEF_TRACE;

    m_endpoints.push_back( endpoint );
}

/*-------------------------------------------------------------------------*/

void
CDRNEndpointList::Clear( void )
{GUCEF_TRACE;

    m_endpoints.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNEndpointList::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_endpoints.size();
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpointList::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_endpoints.empty();
}

/*-------------------------------------------------------------------------*/

const CDRNEndpoint&
CDRNEndpointList::Get( UInt32 index ) const
{GUCEF_TRACE;

    return m_endpoints[ index ];
}

/*-------------------------------------------------------------------------*/

const CDRNEndpointList::TEndpointVector&
CDRNEndpointList::GetEndpoints( void ) const
{GUCEF_TRACE;

    return m_endpoints;
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpointList::FromString( const CORE::CString& endpoints )
{GUCEF_TRACE;

    m_endpoints.clear();
    bool allValid = true;
    CORE::CString::StringVector uris = endpoints.ParseElements( ';', false );
    CORE::CString::StringVector::iterator i = uris.begin();
    while ( i != uris.end() )
    {
        CORE::CString uri = (*i).Trim( true ).Trim( false );
        if ( !uri.IsNULLOrEmpty() )
        {
            CDRNEndpoint endpoint;
            if ( endpoint.FromString( uri ) )
                m_endpoints.push_back( endpoint );
            else
                allValid = false;
        }
        ++i;
    }
    return allValid;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNEndpointList::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString result;
    for ( UInt32 i=0; i<m_endpoints.size(); ++i )
    {
        if ( i > 0 )
            result += ";";
        result += m_endpoints[ i ].ToString();
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpointList::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    for ( UInt32 i=0; i<m_endpoints.size(); ++i )
    {
        CORE::CDataNode* child = cfg.AddChildWithValue( "Endpoint", CORE::CVariant( m_endpoints[ i ].ToString() ) );
        totalSuccess = ( GUCEF_NULL != child ) && totalSuccess;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNEndpointList::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_endpoints.clear();
    bool allValid = true;
    CORE::CDataNode::const_iterator i = cfg.ConstBegin();
    while ( i != cfg.ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        if ( GUCEF_NULL != child && child->GetName() == "Endpoint" )
        {
            CDRNEndpoint endpoint;
            if ( endpoint.FromString( child->GetValue().AsString() ) )
                m_endpoints.push_back( endpoint );
            else
                allValid = false;
        }
        ++i;
    }
    return allValid;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNEndpointList::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
