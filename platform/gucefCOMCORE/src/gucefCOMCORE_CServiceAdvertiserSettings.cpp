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

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H
#include "gucefCOMCORE_CDiscoveryClientList.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H ? */

#ifndef GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H
#include "gucefCOMCORE_CServiceAnnouncement.h"
#define GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H
#endif /* GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H ? */

#include "gucefCOMCORE_CServiceAdvertiserSettings.h"

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

CServiceAdvertiserSettings::CServiceAdvertiserSettings( void )
    : CORE::CIConfigurable()
    , serviceType()
    , instanceId()
    , displayName()
    , ttlInSecs( CServiceAnnouncement::DefaultTtlInSecs )
    , clientConfig()
    , defaultClientTypes()
{GUCEF_TRACE;

    CDiscoveryClientList::GetDefaultServiceClientTypes( defaultClientTypes );
}

/*-------------------------------------------------------------------------*/

CServiceAdvertiserSettings::CServiceAdvertiserSettings( const CServiceAdvertiserSettings& src )
    : CORE::CIConfigurable( src )
    , serviceType( src.serviceType )
    , instanceId( src.instanceId )
    , displayName( src.displayName )
    , ttlInSecs( src.ttlInSecs )
    , clientConfig( src.clientConfig )
    , defaultClientTypes( src.defaultClientTypes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceAdvertiserSettings::~CServiceAdvertiserSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceAdvertiserSettings&
CServiceAdvertiserSettings::operator=( const CServiceAdvertiserSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        serviceType = src.serviceType;
        instanceId = src.instanceId;
        displayName = src.displayName;
        ttlInSecs = src.ttlInSecs;
        clientConfig = src.clientConfig;
        defaultClientTypes = src.defaultClientTypes;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CServiceAdvertiserSettings::SetClientTypes( const CORE::CString& typeList )
{GUCEF_TRACE;

    clientConfig.Clear();
    clientConfig.SetAttribute( CDiscoveryClientList::ClientTypesKey, typeList );
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiserSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "serviceType", serviceType );
    config.SetAttribute( "instanceId", instanceId );
    config.SetAttribute( "displayName", displayName );
    config.SetAttribute( "ttlInSecs", ttlInSecs );
    CDiscoveryClientList::CopyClientConfig( clientConfig, config );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiserSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    serviceType = config.GetAttributeValueOrChildValueByName( "serviceType" ).AsString( serviceType, true );
    instanceId = config.GetAttributeValueOrChildValueByName( "instanceId" ).AsString( instanceId, true );
    displayName = config.GetAttributeValueOrChildValueByName( "displayName" ).AsString( displayName, true );
    ttlInSecs = config.GetAttributeValueOrChildValueByName( "ttlInSecs" ).AsUInt32( ttlInSecs, true );
    if ( CDiscoveryClientList::NamesClients( config ) )
    {
        clientConfig.Clear();
        CDiscoveryClientList::CopyClientConfig( config, clientConfig );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceAdvertiserSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CServiceAdvertiserSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
