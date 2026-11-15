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

#include "gucefCOMCORE_CServiceFinderSettings.h"

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

CServiceFinderSettings::CServiceFinderSettings( void )
    : CORE::CIConfigurable()
    , serviceType()
    , filter()
    , findTimeoutMs( 3000 )
    , findMinCandidates( 1 )
    , threadPoolName()
    , clientConfig()
    , defaultClientTypes()
{GUCEF_TRACE;

    CDiscoveryClientList::GetDefaultServiceClientTypes( defaultClientTypes );
}

/*-------------------------------------------------------------------------*/

CServiceFinderSettings::CServiceFinderSettings( const CServiceFinderSettings& src )
    : CORE::CIConfigurable( src )
    , serviceType( src.serviceType )
    , filter( src.filter )
    , findTimeoutMs( src.findTimeoutMs )
    , findMinCandidates( src.findMinCandidates )
    , threadPoolName( src.threadPoolName )
    , clientConfig( src.clientConfig )
    , defaultClientTypes( src.defaultClientTypes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceFinderSettings::~CServiceFinderSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceFinderSettings&
CServiceFinderSettings::operator=( const CServiceFinderSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        serviceType = src.serviceType;
        filter = src.filter;
        findTimeoutMs = src.findTimeoutMs;
        findMinCandidates = src.findMinCandidates;
        threadPoolName = src.threadPoolName;
        clientConfig = src.clientConfig;
        defaultClientTypes = src.defaultClientTypes;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CServiceFinderSettings::SetClientTypes( const CORE::CString& typeList )
{GUCEF_TRACE;

    clientConfig.Clear();
    clientConfig.SetAttribute( CDiscoveryClientList::ClientTypesKey, typeList );
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinderSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "serviceType", serviceType );
    config.SetAttribute( "findTimeoutMs", findTimeoutMs );
    config.SetAttribute( "findMinCandidates", findMinCandidates );
    config.SetAttribute( "threadPoolName", threadPoolName );
    CORE::CDataNode* filterNode = config.AddChild( "Filter", GUCEF_DATATYPE_OBJECT );
    filter.SaveConfig( *filterNode );
    CDiscoveryClientList::CopyClientConfig( clientConfig, config );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinderSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    serviceType = config.GetAttributeValueOrChildValueByName( "serviceType" ).AsString( serviceType, true );
    findTimeoutMs = config.GetAttributeValueOrChildValueByName( "findTimeoutMs" ).AsUInt32( findTimeoutMs, true );
    findMinCandidates = config.GetAttributeValueOrChildValueByName( "findMinCandidates" ).AsUInt32( findMinCandidates, true );
    threadPoolName = config.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( threadPoolName, true );
    const CORE::CDataNode* filterNode = config.FindChild( "Filter" );
    if ( GUCEF_NULL != filterNode && !filter.LoadConfig( *filterNode ) )
        return false;
    if ( CDiscoveryClientList::NamesClients( config ) )
    {
        clientConfig.Clear();
        CDiscoveryClientList::CopyClientConfig( config, clientConfig );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceFinderSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CServiceFinderSettings";
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
