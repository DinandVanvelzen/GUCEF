/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOMCORE_CDiscoveryEntry.h"

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

CDiscoveryEntry::CDiscoveryEntry( void )
    : CIDiscoveryEntry()
    , m_applicationId()
    , m_applicationInstanceId()
    , m_hostInstanceId()
    , m_host()
    , m_capabilities()
    , m_serviceType()
    , m_instanceName()
    , m_attributes()
    , m_endpoints()
    , m_ttlInSecs( 120 )
    , m_lastSeen()
    , m_sourceClientName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryEntry::CDiscoveryEntry( const CDiscoveryEntry& src )
    : CIDiscoveryEntry( src )
    , m_applicationId( src.m_applicationId )
    , m_applicationInstanceId( src.m_applicationInstanceId )
    , m_hostInstanceId( src.m_hostInstanceId )
    , m_host( src.m_host )
    , m_capabilities( src.m_capabilities )
    , m_serviceType( src.m_serviceType )
    , m_instanceName( src.m_instanceName )
    , m_attributes( src.m_attributes )
    , m_endpoints( src.m_endpoints )
    , m_ttlInSecs( src.m_ttlInSecs )
    , m_lastSeen( src.m_lastSeen )
    , m_sourceClientName( src.m_sourceClientName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryEntry::CDiscoveryEntry( const CIDiscoveryEntry& src )
    : CIDiscoveryEntry()
    , m_applicationId()
    , m_applicationInstanceId()
    , m_hostInstanceId()
    , m_host()
    , m_capabilities()
    , m_serviceType()
    , m_instanceName()
    , m_attributes()
    , m_endpoints()
    , m_ttlInSecs( 120 )
    , m_lastSeen()
    , m_sourceClientName()
{GUCEF_TRACE;

    Assign( src );
}

/*-------------------------------------------------------------------------*/

CDiscoveryEntry::~CDiscoveryEntry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryEntry&
CDiscoveryEntry::operator=( const CDiscoveryEntry& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIDiscoveryEntry::operator=( src );
        m_applicationId = src.m_applicationId;
        m_applicationInstanceId = src.m_applicationInstanceId;
        m_hostInstanceId = src.m_hostInstanceId;
        m_host = src.m_host;
        m_capabilities = src.m_capabilities;
        m_serviceType = src.m_serviceType;
        m_instanceName = src.m_instanceName;
        m_attributes = src.m_attributes;
        m_endpoints = src.m_endpoints;
        m_ttlInSecs = src.m_ttlInSecs;
        m_lastSeen = src.m_lastSeen;
        m_sourceClientName = src.m_sourceClientName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryEntry::Assign( const CIDiscoveryEntry& src )
{GUCEF_TRACE;

    if ( &src == this )
        return;

    src.GetApplicationId( m_applicationId );
    src.GetApplicationInstanceId( m_applicationInstanceId );
    src.GetHostInstanceId( m_hostInstanceId );
    src.GetHost( m_host );
    src.GetCapabilities( m_capabilities );
    m_serviceType = src.GetServiceType();
    m_instanceName = src.GetInstanceName();
    m_attributes = src.GetAttributes();
    m_endpoints = src.GetEndpoints();
    m_ttlInSecs = src.GetTtlInSecs();
    m_lastSeen = src.GetLastSeen();
    m_sourceClientName = src.GetSourceClientName();
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::AreAttributesEqual( const CORE::CValueList& a, const CORE::CValueList& b )
{GUCEF_TRACE;

    UInt32 keyCount = a.GetKeyCount();
    if ( keyCount != b.GetKeyCount() )
        return false;

    for ( UInt32 i=0; i<keyCount; ++i )
    {
        const CORE::CVariant& key = a.GetKey( i );
        CORE::CValueList::TVariantVector otherValues;
        if ( !b.TryGetValueVector( key, otherValues ) )
            return false;

        const CORE::CValueList::TVariantVector& values = a.GetValueVector( i );
        if ( values.size() != otherValues.size() )
            return false;
        for ( size_t n=0; n<values.size(); ++n )
        {
            if ( values[ n ].AsString() != otherValues[ n ].AsString() )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::HasSameContent( const CIDiscoveryEntry& other ) const
{GUCEF_TRACE;

    if ( &other == this )
        return true;

    if ( m_serviceType != other.GetServiceType()     ||
         m_instanceName != other.GetInstanceName()   ||
         m_ttlInSecs != other.GetTtlInSecs()         ||
         m_endpoints != other.GetEndpoints()          )
        return false;

    CORE::CString str;
    if ( !other.GetApplicationId( str ) || str != m_applicationId )
        return false;
    if ( !other.GetApplicationInstanceId( str ) || str != m_applicationInstanceId )
        return false;
    if ( !other.GetHostInstanceId( str ) || str != m_hostInstanceId )
        return false;
    if ( !other.GetCapabilities( str ) || str != m_capabilities )
        return false;

    CHostAddress host;
    if ( !other.GetHost( host ) || host != m_host )
        return false;

    return AreAttributesEqual( m_attributes, other.GetAttributes() );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetApplicationId( const CORE::CString& appId )
{GUCEF_TRACE;

    m_applicationId = appId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::GetApplicationId( CORE::CString& appId ) const
{GUCEF_TRACE;

    appId = m_applicationId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetApplicationInstanceId( const CORE::CString& appInstanceId )
{GUCEF_TRACE;

    m_applicationInstanceId = appInstanceId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::GetApplicationInstanceId( CORE::CString& appInstanceId ) const
{GUCEF_TRACE;

    appInstanceId = m_applicationInstanceId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetHostInstanceId( const CORE::CString& hostInstanceId )
{GUCEF_TRACE;

    m_hostInstanceId = hostInstanceId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::GetHostInstanceId( CORE::CString& hostInstanceId ) const
{GUCEF_TRACE;

    hostInstanceId = m_hostInstanceId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetHost( const COMCORE::CHostAddress& host )
{GUCEF_TRACE;

    m_host = host;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::GetHost( COMCORE::CHostAddress& host ) const
{GUCEF_TRACE;

    host = m_host;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetCapabilities( const CORE::CString& capabilities )
{GUCEF_TRACE;

    m_capabilities = capabilities;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::GetCapabilities( CORE::CString& capabilities ) const
{GUCEF_TRACE;

    capabilities = m_capabilities;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetServiceType( const CORE::CString& serviceType )
{GUCEF_TRACE;

    m_serviceType = serviceType;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDiscoveryEntry::GetServiceType( void ) const
{GUCEF_TRACE;

    return m_serviceType;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetInstanceName( const CORE::CString& instanceName )
{GUCEF_TRACE;

    m_instanceName = instanceName;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDiscoveryEntry::GetInstanceName( void ) const
{GUCEF_TRACE;

    return m_instanceName;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetAttributes( const CORE::CValueList& attributes )
{GUCEF_TRACE;

    m_attributes = attributes;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
CDiscoveryEntry::GetAttributes( void ) const
{GUCEF_TRACE;

    return m_attributes;
}

/*-------------------------------------------------------------------------*/

CORE::CValueList&
CDiscoveryEntry::GetAttributes( void )
{GUCEF_TRACE;

    return m_attributes;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetEndpoints( const CORE::CString::StringVector& endpoints )
{GUCEF_TRACE;

    m_endpoints = endpoints;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CDiscoveryEntry::GetEndpoints( void ) const
{GUCEF_TRACE;

    return m_endpoints;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector&
CDiscoveryEntry::GetEndpoints( void )
{GUCEF_TRACE;

    return m_endpoints;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetTtlInSecs( UInt32 ttlInSecs )
{GUCEF_TRACE;

    m_ttlInSecs = ttlInSecs;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryEntry::GetTtlInSecs( void ) const
{GUCEF_TRACE;

    return m_ttlInSecs;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetLastSeen( const CORE::CDateTime& lastSeen )
{GUCEF_TRACE;

    m_lastSeen = lastSeen;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CDateTime&
CDiscoveryEntry::GetLastSeen( void ) const
{GUCEF_TRACE;

    return m_lastSeen;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryEntry::SetSourceClientName( const CORE::CString& clientName )
{GUCEF_TRACE;

    m_sourceClientName = clientName;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDiscoveryEntry::GetSourceClientName( void ) const
{GUCEF_TRACE;

    return m_sourceClientName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
