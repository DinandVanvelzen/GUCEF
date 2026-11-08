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

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

#include "gucefDRN_CDRNPublishedTableSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNPublishedTableSettings::ClassTypeName = "GUCEF::DRN::CDRNPublishedTableSettings";

/*-------------------------------------------------------------------------*/

CDRNPublishedTableSettings::CDRNPublishedTableSettings( void )
    : CORE::CIConfigurable()
    , scanChunkKeyCount( 4096 )
    , snapshotCacheMs( 0 )
    , decodeCacheIdleMs( 60000 )
    , deferUntilConsistent( true )
    , maxDeferMs( 2000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNPublishedTableSettings::CDRNPublishedTableSettings( const CDRNPublishedTableSettings& src )
    : CORE::CIConfigurable( src )
    , scanChunkKeyCount( src.scanChunkKeyCount )
    , snapshotCacheMs( src.snapshotCacheMs )
    , decodeCacheIdleMs( src.decodeCacheIdleMs )
    , deferUntilConsistent( src.deferUntilConsistent )
    , maxDeferMs( src.maxDeferMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNPublishedTableSettings::~CDRNPublishedTableSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNPublishedTableSettings&
CDRNPublishedTableSettings::operator=( const CDRNPublishedTableSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        scanChunkKeyCount = src.scanChunkKeyCount;
        snapshotCacheMs = src.snapshotCacheMs;
        decodeCacheIdleMs = src.decodeCacheIdleMs;
        deferUntilConsistent = src.deferUntilConsistent;
        maxDeferMs = src.maxDeferMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTableSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "scanChunkKeyCount", scanChunkKeyCount ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "snapshotCacheMs", snapshotCacheMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "decodeCacheIdleMs", decodeCacheIdleMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "deferUntilConsistent", deferUntilConsistent ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxDeferMs", maxDeferMs ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTableSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    scanChunkKeyCount = cfg.GetAttributeValueOrChildValueByName( "scanChunkKeyCount" ).AsUInt32( scanChunkKeyCount, true );
    snapshotCacheMs = cfg.GetAttributeValueOrChildValueByName( "snapshotCacheMs" ).AsUInt32( snapshotCacheMs, true );
    decodeCacheIdleMs = cfg.GetAttributeValueOrChildValueByName( "decodeCacheIdleMs" ).AsUInt32( decodeCacheIdleMs, true );
    deferUntilConsistent = cfg.GetAttributeValueOrChildValueByName( "deferUntilConsistent" ).AsBool( deferUntilConsistent, true );
    maxDeferMs = cfg.GetAttributeValueOrChildValueByName( "maxDeferMs" ).AsUInt32( maxDeferMs, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNPublishedTableSettings::GetClassTypeName( void ) const
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
