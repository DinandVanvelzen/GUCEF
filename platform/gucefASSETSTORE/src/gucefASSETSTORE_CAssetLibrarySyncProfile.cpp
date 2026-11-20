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

#ifndef GUCEF_CORE_CCRONSCHEDULE_H
#include "gucefCORE_CCronSchedule.h"
#define GUCEF_CORE_CCRONSCHEDULE_H
#endif /* GUCEF_CORE_CCRONSCHEDULE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */

#include "gucefASSETSTORE_CAssetLibrarySyncProfile.h"

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

const CORE::CString CAssetLibrarySyncProfile::OnStaleKeep = "keep";
const CORE::CString CAssetLibrarySyncProfile::OnStaleReport = "report";
const CORE::CString CAssetLibrarySyncProfile::OrderNewest = "newest";
const CORE::CString CAssetLibrarySyncProfile::OrderOldest = "oldest";
const CORE::CString CAssetLibrarySyncProfile::OrderSmallest = "smallest";
const CORE::CString CAssetLibrarySyncProfile::OrderStore = "store";

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncProfile::CAssetLibrarySyncProfile( void )
    : CORE::CIDataNodeSerializable()
    , enabled( false )
    , schedule()
    , formats()
    , listingTypes()
    , licenceClasses()
    , maxItemBytes( 0 )
    , minItemBytes( 0 )
    , skipUnknownSize( false )
    , onStale( OnStaleKeep )
    , order( OrderNewest )
    , adoptExisting( true )
    , maxSyncAttempts( 5 )
    , retryBackoffMinutes( 15 )
    , maxInFlightPerStore( 0 )
    , reservePercent( 5 )
    , reserveBytes( 20ULL * 1024 * 1024 * 1024 )
    , diskFactor( 2.0 )
    , unknownSizeEstimateBytes( 1024ULL * 1024 * 1024 )
    , maxItemsPerHour( 0 )
    , maxItemsPerDay( 0 )
    , maxBytesPerDay( 0 )
    , maxStagedBytes( 50ULL * 1024 * 1024 * 1024 )
    , maxPendingIngests( 8 )
    , enrichInFlight( 1 )
    , indexAfter( true )
    , planMaxAgeMinutes( 1440 )
    , detailsMaxAgeDays( 14 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncProfile::~CAssetLibrarySyncProfile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncProfile::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    reason.Clear();
    CORE::CCronSchedule cron;
    if ( !schedule.IsNULLOrEmpty() && !cron.FromCronString( schedule ) )
        reason = "schedule \"" + schedule + "\" is not a cron expression (minute hour day-of-month month day-of-week, UTC)";
    else
    if ( OnStaleKeep != onStale && OnStaleReport != onStale )
        reason = "onStale \"" + onStale + "\" is not known: keep or report (replace is not built in this version)";
    else
    if ( OrderNewest != order && OrderOldest != order && OrderSmallest != order && OrderStore != order )
        reason = "order \"" + order + "\" is not known: newest, oldest, smallest or store";
    else
    if ( diskFactor < 1.0 )
        reason = "diskFactor is below 1";
    else
    if ( reservePercent > 100 )
        reason = "reservePercent is above 100";
    else
    if ( 0 == maxSyncAttempts )
        reason = "maxSyncAttempts is 0";
    return reason.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncProfile::GetPlanFingerprint( void ) const
{GUCEF_TRACE;

    // the members that shape a plan, in a fixed order; the sets are ordered already
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode formatsNode( "formats", GUCEF_DATATYPE_OBJECT );
    formats.Serialize( formatsNode, settings );
    CORE::CString text;
    CAssetStoreJson::ToText( formatsNode, text );
    text += "\n";
    for ( CORE::CString::StringSet::const_iterator i = listingTypes.begin(); i != listingTypes.end(); ++i )
        text += (*i) + ",";
    text += "\n";
    for ( CORE::CString::StringSet::const_iterator i = licenceClasses.begin(); i != licenceClasses.end(); ++i )
        text += (*i) + ",";
    text += "\n" + CORE::ToString( maxItemBytes ) + "\n" + CORE::ToString( minItemBytes ) + "\n" + CORE::ToString( skipUnknownSize ) +
            "\n" + onStale + "\n" + CORE::ToString( adoptExisting );
    CAssetDigest digest( CAssetDigest::Sha256 );
    const UInt32 size = text.ByteSize();
    digest.Update( text.C_String(), size > 0 ? size - 1 : 0 );
    return digest.FinalHex();
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncProfile::Serialize( CORE::CDataNode& domRootNode                        ,
                                     const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "enabled", enabled );
    if ( !schedule.IsNULLOrEmpty() )
        success = domRootNode.SetAttribute( "schedule", schedule ) && success;
    CORE::CDataNode* formatsNode = domRootNode.AddChild( "formats", GUCEF_DATATYPE_OBJECT );
    success = GUCEF_NULL != formatsNode && formats.Serialize( *formatsNode, settings ) && success;
    success = domRootNode.SetValuesOfChildByName( "listingTypes", listingTypes ) && success;
    success = domRootNode.SetValuesOfChildByName( "licenceClasses", licenceClasses ) && success;
    success = domRootNode.SetAttribute( "maxItemBytes", maxItemBytes ) && success;
    success = domRootNode.SetAttribute( "minItemBytes", minItemBytes ) && success;
    success = domRootNode.SetAttribute( "skipUnknownSize", skipUnknownSize ) && success;
    success = domRootNode.SetAttribute( "onStale", onStale ) && success;
    success = domRootNode.SetAttribute( "order", order ) && success;
    success = domRootNode.SetAttribute( "adoptExisting", adoptExisting ) && success;
    success = domRootNode.SetAttribute( "maxSyncAttempts", maxSyncAttempts ) && success;
    success = domRootNode.SetAttribute( "retryBackoffMinutes", retryBackoffMinutes ) && success;
    success = domRootNode.SetAttribute( "maxInFlightPerStore", maxInFlightPerStore ) && success;
    success = domRootNode.SetAttribute( "reservePercent", reservePercent ) && success;
    success = domRootNode.SetAttribute( "reserveBytes", reserveBytes ) && success;
    success = domRootNode.SetAttribute( "diskFactor", CORE::CVariant( diskFactor ) ) && success;
    success = domRootNode.SetAttribute( "unknownSizeEstimateBytes", unknownSizeEstimateBytes ) && success;
    success = domRootNode.SetAttribute( "maxItemsPerHour", maxItemsPerHour ) && success;
    success = domRootNode.SetAttribute( "maxItemsPerDay", maxItemsPerDay ) && success;
    success = domRootNode.SetAttribute( "maxBytesPerDay", maxBytesPerDay ) && success;
    success = domRootNode.SetAttribute( "maxStagedBytes", maxStagedBytes ) && success;
    success = domRootNode.SetAttribute( "maxPendingIngests", maxPendingIngests ) && success;
    success = domRootNode.SetAttribute( "enrichInFlight", enrichInFlight ) && success;
    success = domRootNode.SetAttribute( "indexAfter", indexAfter ) && success;
    success = domRootNode.SetAttribute( "planMaxAgeMinutes", planMaxAgeMinutes ) && success;
    success = domRootNode.SetAttribute( "detailsMaxAgeDays", detailsMaxAgeDays ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncProfile::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                       const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    enabled = domRootNode.GetAttributeValueOrChildValueByName( "enabled" ).AsBool( enabled, true );
    schedule = domRootNode.GetAttributeValueOrChildValueByName( "schedule" ).AsString( schedule, true );
    const CORE::CDataNode* formatsNode = domRootNode.FindChild( "formats" );
    if ( GUCEF_NULL != formatsNode )
        formats.Deserialize( *formatsNode, settings );
    if ( GUCEF_NULL != domRootNode.FindChild( "listingTypes" ) )
    {
        listingTypes.clear();
        domRootNode.GetValuesOfChildByName( "listingTypes", listingTypes );
    }
    if ( GUCEF_NULL != domRootNode.FindChild( "licenceClasses" ) )
    {
        licenceClasses.clear();
        domRootNode.GetValuesOfChildByName( "licenceClasses", licenceClasses );
    }
    maxItemBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxItemBytes" ).AsUInt64( maxItemBytes, true );
    minItemBytes = domRootNode.GetAttributeValueOrChildValueByName( "minItemBytes" ).AsUInt64( minItemBytes, true );
    skipUnknownSize = domRootNode.GetAttributeValueOrChildValueByName( "skipUnknownSize" ).AsBool( skipUnknownSize, true );
    onStale = domRootNode.GetAttributeValueOrChildValueByName( "onStale" ).AsString( onStale, true );
    order = domRootNode.GetAttributeValueOrChildValueByName( "order" ).AsString( order, true );
    adoptExisting = domRootNode.GetAttributeValueOrChildValueByName( "adoptExisting" ).AsBool( adoptExisting, true );
    maxSyncAttempts = domRootNode.GetAttributeValueOrChildValueByName( "maxSyncAttempts" ).AsUInt32( maxSyncAttempts, true );
    retryBackoffMinutes = domRootNode.GetAttributeValueOrChildValueByName( "retryBackoffMinutes" ).AsUInt32( retryBackoffMinutes, true );
    maxInFlightPerStore = domRootNode.GetAttributeValueOrChildValueByName( "maxInFlightPerStore" ).AsUInt32( maxInFlightPerStore, true );
    reservePercent = domRootNode.GetAttributeValueOrChildValueByName( "reservePercent" ).AsUInt32( reservePercent, true );
    reserveBytes = domRootNode.GetAttributeValueOrChildValueByName( "reserveBytes" ).AsUInt64( reserveBytes, true );
    diskFactor = domRootNode.GetAttributeValueOrChildValueByName( "diskFactor" ).AsFloat64( diskFactor, true );
    unknownSizeEstimateBytes = domRootNode.GetAttributeValueOrChildValueByName( "unknownSizeEstimateBytes" ).AsUInt64( unknownSizeEstimateBytes, true );
    maxItemsPerHour = domRootNode.GetAttributeValueOrChildValueByName( "maxItemsPerHour" ).AsUInt32( maxItemsPerHour, true );
    maxItemsPerDay = domRootNode.GetAttributeValueOrChildValueByName( "maxItemsPerDay" ).AsUInt32( maxItemsPerDay, true );
    maxBytesPerDay = domRootNode.GetAttributeValueOrChildValueByName( "maxBytesPerDay" ).AsUInt64( maxBytesPerDay, true );
    maxStagedBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxStagedBytes" ).AsUInt64( maxStagedBytes, true );
    maxPendingIngests = domRootNode.GetAttributeValueOrChildValueByName( "maxPendingIngests" ).AsUInt32( maxPendingIngests, true );
    enrichInFlight = domRootNode.GetAttributeValueOrChildValueByName( "enrichInFlight" ).AsUInt32( enrichInFlight, true );
    indexAfter = domRootNode.GetAttributeValueOrChildValueByName( "indexAfter" ).AsBool( indexAfter, true );
    planMaxAgeMinutes = domRootNode.GetAttributeValueOrChildValueByName( "planMaxAgeMinutes" ).AsUInt32( planMaxAgeMinutes, true );
    detailsMaxAgeDays = domRootNode.GetAttributeValueOrChildValueByName( "detailsMaxAgeDays" ).AsUInt32( detailsMaxAgeDays, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLibrarySyncProfile::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLibrarySyncProfile( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLibrarySyncProfile::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLibrarySyncProfile::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetLibrarySyncProfile";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncProfile::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLibrarySyncProfile";
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
