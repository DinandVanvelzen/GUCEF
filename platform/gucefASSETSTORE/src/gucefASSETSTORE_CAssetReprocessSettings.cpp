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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

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

namespace {

/** The keys of the settings with the largest value each may have; the booleans have 1 */
struct SSettingKey
{
    const char* name;
    UInt32 maximum;
    UInt32 minimum;
    bool boolean;
};

const SSettingKey SettingKeys[] =
{
    { "enabled", 1, 0, true },
    { "maxConcurrentJobs", 256, 0, false },
    { "threadsPerJob", 1024, 0, false },
    { "reservedCores", 1024, 0, false },
    { "maxQueuedJobs", 100000, 0, false },
    { "maxJobsPerPackPerHour", 1000000, 0, false },
    { "maxMachineJobsPerHour", 1000000, 0, false },
    { "maxMachineRedownloadsPerDay", 1000000, 0, false },
    { "limitOwner", 1, 0, true },
    { "downloadTimeoutMinutes", 525600, 0, false },
    { "memoryBudgetMB", 1073741824, 0, false },
    { "memoryPerJobMB", 1073741824, 0, false },
    { "memoryLargestAssetFactor", 64, 0, false },
    { "maxConcurrentWritesPerLocation", 256, 0, false }
};
const UInt32 SettingKeyCount = sizeof( SettingKeys ) / sizeof( SettingKeys[ 0 ] );

const SSettingKey*
FindSettingKey( const CORE::CString& name )
{GUCEF_TRACE;

    for ( UInt32 k=0; k<SettingKeyCount; ++k )
        if ( name == SettingKeys[ k ].name )
            return &SettingKeys[ k ];
    return GUCEF_NULL;
}

/** The value of a key as a number or a boolean, or the reason it is not valid */
bool
ParseSettingValue( const SSettingKey& key, const CORE::CVariant& value, UInt32& parsed, CORE::CString& problem )
{GUCEF_TRACE;

    const CORE::CString text = value.AsString( CORE::CString::Empty, true ).Trim( true ).Trim( false );
    if ( key.boolean )
    {
        const CORE::CString lower = text.Lowercase();
        if ( "true" == lower || "1" == lower )
            parsed = 1;
        else
        if ( "false" == lower || "0" == lower )
            parsed = 0;
        else
        {
            problem = "\"" + text + "\" is not true or false";
            return false;
        }
        return true;
    }
    const char* chars = text.C_String();
    const UInt32 length = text.Length();
    if ( 0 == length || length > 10 )
    {
        problem = "\"" + text + "\" is not a whole number from " + CORE::ToString( key.minimum ) + " to " + CORE::ToString( key.maximum );
        return false;
    }
    UInt64 number = 0;
    for ( UInt32 c=0; c<length; ++c )
    {
        if ( chars[ c ] < '0' || chars[ c ] > '9' )
        {
            problem = "\"" + text + "\" is not a whole number from " + CORE::ToString( key.minimum ) + " to " + CORE::ToString( key.maximum );
            return false;
        }
        number = number * 10 + (UInt64) ( chars[ c ] - '0' );
    }
    if ( number < key.minimum || number > key.maximum )
    {
        problem = CORE::ToString( number ) + " is outside of " + CORE::ToString( key.minimum ) + " to " + CORE::ToString( key.maximum );
        return false;
    }
    parsed = (UInt32) number;
    return true;
}

}

/*-------------------------------------------------------------------------*/

CAssetReprocessSettings::CAssetReprocessSettings( void )
    : CORE::CIDataNodeSerializable()
    , enabled( true )
    , maxConcurrentJobs( 0 )
    , threadsPerJob( 0 )
    , reservedCores( 2 )
    , maxQueuedJobs( 64 )
    , maxJobsPerPackPerHour( 4 )
    , maxMachineJobsPerHour( 20 )
    , maxMachineRedownloadsPerDay( 6 )
    , limitOwner( false )
    , downloadTimeoutMinutes( 360 )
    , memoryBudgetMB( 0 )
    , memoryPerJobMB( 2048 )
    , memoryLargestAssetFactor( 4 )
    , maxConcurrentWritesPerLocation( 0 )
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
    success = domRootNode.SetAttribute( "threadsPerJob", threadsPerJob ) && success;
    success = domRootNode.SetAttribute( "reservedCores", reservedCores ) && success;
    success = domRootNode.SetAttribute( "maxQueuedJobs", maxQueuedJobs ) && success;
    success = domRootNode.SetAttribute( "maxJobsPerPackPerHour", maxJobsPerPackPerHour ) && success;
    success = domRootNode.SetAttribute( "maxMachineJobsPerHour", maxMachineJobsPerHour ) && success;
    success = domRootNode.SetAttribute( "maxMachineRedownloadsPerDay", maxMachineRedownloadsPerDay ) && success;
    success = domRootNode.SetAttribute( "limitOwner", limitOwner ) && success;
    success = domRootNode.SetAttribute( "downloadTimeoutMinutes", downloadTimeoutMinutes ) && success;
    success = domRootNode.SetAttribute( "memoryBudgetMB", memoryBudgetMB ) && success;
    success = domRootNode.SetAttribute( "memoryPerJobMB", memoryPerJobMB ) && success;
    success = domRootNode.SetAttribute( "memoryLargestAssetFactor", memoryLargestAssetFactor ) && success;
    success = domRootNode.SetAttribute( "maxConcurrentWritesPerLocation", maxConcurrentWritesPerLocation ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessSettings::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    enabled = domRootNode.GetAttributeValueOrChildValueByName( "enabled" ).AsBool( enabled, true );
    maxConcurrentJobs = domRootNode.GetAttributeValueOrChildValueByName( "maxConcurrentJobs" ).AsUInt32( maxConcurrentJobs, true );
    threadsPerJob = domRootNode.GetAttributeValueOrChildValueByName( "threadsPerJob" ).AsUInt32( threadsPerJob, true );
    reservedCores = domRootNode.GetAttributeValueOrChildValueByName( "reservedCores" ).AsUInt32( reservedCores, true );
    maxQueuedJobs = domRootNode.GetAttributeValueOrChildValueByName( "maxQueuedJobs" ).AsUInt32( maxQueuedJobs, true );
    maxJobsPerPackPerHour = domRootNode.GetAttributeValueOrChildValueByName( "maxJobsPerPackPerHour" ).AsUInt32( maxJobsPerPackPerHour, true );
    maxMachineJobsPerHour = domRootNode.GetAttributeValueOrChildValueByName( "maxMachineJobsPerHour" ).AsUInt32( maxMachineJobsPerHour, true );
    maxMachineRedownloadsPerDay = domRootNode.GetAttributeValueOrChildValueByName( "maxMachineRedownloadsPerDay" ).AsUInt32( maxMachineRedownloadsPerDay, true );
    limitOwner = domRootNode.GetAttributeValueOrChildValueByName( "limitOwner" ).AsBool( limitOwner, true );
    downloadTimeoutMinutes = domRootNode.GetAttributeValueOrChildValueByName( "downloadTimeoutMinutes" ).AsUInt32( downloadTimeoutMinutes, true );
    memoryBudgetMB = domRootNode.GetAttributeValueOrChildValueByName( "memoryBudgetMB" ).AsUInt32( memoryBudgetMB, true );
    memoryPerJobMB = domRootNode.GetAttributeValueOrChildValueByName( "memoryPerJobMB" ).AsUInt32( memoryPerJobMB, true );
    memoryLargestAssetFactor = domRootNode.GetAttributeValueOrChildValueByName( "memoryLargestAssetFactor" ).AsUInt32( memoryLargestAssetFactor, true );
    maxConcurrentWritesPerLocation = domRootNode.GetAttributeValueOrChildValueByName( "maxConcurrentWritesPerLocation" ).AsUInt32( maxConcurrentWritesPerLocation, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessSettings::ApplyChanges( const CORE::CDataNode& changes, CORE::CString::StringMap& errors )
{GUCEF_TRACE;

    errors.clear();
    CORE::CDataNode values( "reprocess", GUCEF_DATATYPE_OBJECT );
    UInt32 count = 0;

    // the keys as attributes and as child values: every one is checked before anything changes
    GUCEF::map< CORE::CString, CORE::CVariant > given;
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = changes.AttributeBegin(); a != changes.AttributeEnd(); ++a )
        given[ (*a).first ] = (*a).second;
    for ( CORE::CDataNode::const_iterator c = changes.ConstBegin(); c != changes.ConstEnd(); ++c )
        given[ (*c)->GetName() ] = (*c)->GetValue();
    for ( GUCEF::map< CORE::CString, CORE::CVariant >::const_iterator g = given.begin(); g != given.end(); ++g )
    {
        const SSettingKey* key = FindSettingKey( (*g).first );
        if ( GUCEF_NULL == key )
        {
            errors[ (*g).first ] = "not a setting of the reprocessing";
            continue;
        }
        UInt32 parsed = 0;
        CORE::CString problem;
        if ( !ParseSettingValue( *key, (*g).second, parsed, problem ) )
        {
            errors[ (*g).first ] = problem;
            continue;
        }
        if ( key->boolean )
            values.SetAttribute( key->name, 0 != parsed );
        else
            values.SetAttribute( key->name, parsed );
        ++count;
    }
    if ( !errors.empty() )
        return false;
    if ( count > 0 )
    {
        CORE::CDataNodeSerializableSettings serializerSettings;
        Deserialize( values, serializerSettings );
    }
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
