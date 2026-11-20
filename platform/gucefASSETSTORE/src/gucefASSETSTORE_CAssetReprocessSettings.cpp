/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include "gucefASSETSTORE_CAssetReprocessSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetReprocessSettings::CAssetReprocessSettings( void )
    : CORE::CIDataNodeSerializable()
    , enabled( true )
    , maxConcurrentJobs( 1 )
    , maxQueuedJobs( 8 )
    , maxJobsPerPackPerHour( 4 )
    , maxMachineJobsPerHour( 20 )
    , maxMachineRedownloadsPerDay( 6 )
    , limitOwner( false )
    , downloadTimeoutMinutes( 360 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetReprocessSettings::~CAssetReprocessSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessSettings::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "enabled", enabled );
    success = domRootNode.SetAttribute( "maxConcurrentJobs", maxConcurrentJobs ) && success;
    success = domRootNode.SetAttribute( "maxQueuedJobs", maxQueuedJobs ) && success;
    success = domRootNode.SetAttribute( "maxJobsPerPackPerHour", maxJobsPerPackPerHour ) && success;
    success = domRootNode.SetAttribute( "maxMachineJobsPerHour", maxMachineJobsPerHour ) && success;
    success = domRootNode.SetAttribute( "maxMachineRedownloadsPerDay", maxMachineRedownloadsPerDay ) && success;
    success = domRootNode.SetAttribute( "limitOwner", limitOwner ) && success;
    success = domRootNode.SetAttribute( "downloadTimeoutMinutes", downloadTimeoutMinutes ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessSettings::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    enabled = domRootNode.GetAttributeValueOrChildValueByName( "enabled" ).AsBool( enabled, true );
    maxConcurrentJobs = domRootNode.GetAttributeValueOrChildValueByName( "maxConcurrentJobs" ).AsUInt32( maxConcurrentJobs, true );
    maxQueuedJobs = domRootNode.GetAttributeValueOrChildValueByName( "maxQueuedJobs" ).AsUInt32( maxQueuedJobs, true );
    maxJobsPerPackPerHour = domRootNode.GetAttributeValueOrChildValueByName( "maxJobsPerPackPerHour" ).AsUInt32( maxJobsPerPackPerHour, true );
    maxMachineJobsPerHour = domRootNode.GetAttributeValueOrChildValueByName( "maxMachineJobsPerHour" ).AsUInt32( maxMachineJobsPerHour, true );
    maxMachineRedownloadsPerDay = domRootNode.GetAttributeValueOrChildValueByName( "maxMachineRedownloadsPerDay" ).AsUInt32( maxMachineRedownloadsPerDay, true );
    limitOwner = domRootNode.GetAttributeValueOrChildValueByName( "limitOwner" ).AsBool( limitOwner, true );
    downloadTimeoutMinutes = domRootNode.GetAttributeValueOrChildValueByName( "downloadTimeoutMinutes" ).AsUInt32( downloadTimeoutMinutes, true );
    if ( 0 == maxConcurrentJobs )
        maxConcurrentJobs = 1;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetReprocessSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetReprocessSettings( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetReprocessSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetReprocessSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
