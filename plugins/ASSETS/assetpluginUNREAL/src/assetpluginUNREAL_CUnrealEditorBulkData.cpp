/*
 *  assetpluginUNREAL: The 48 byte record of an editor bulk data payload that refers to the package trailer
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

#include <stdio.h>
#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#include "assetpluginUNREAL_CUnrealEditorBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealEditorBulkData::CUnrealEditorBulkData( void )
    : flags( 0 )
    , bulkDataId()
    , payloadSize( 0 )
{GUCEF_TRACE;

    memset( payloadId, 0, sizeof( payloadId ) );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealEditorBulkData::Read( CUnrealArchiveReader& reader, const CUnrealVersionContext& versions )
{GUCEF_TRACE;

    // the record has the same 48 byte layout from the first file version with the payload table of the package trailer
    // (1002, verified on packages of version 1008 and 1009 to 1018)
    if ( !versions.IsAtLeast( UE5_PAYLOAD_TOC ) )
        return reader.SetError( "Editor bulk data records of UE5 file versions before 1002 are not supported" );
    flags = reader.ReadUInt32();
    reader.ReadGuid( bulkDataId );
    reader.Read( payloadId, sizeof( payloadId ) );
    payloadSize = reader.ReadInt64();
    if ( !reader.HasError() && payloadSize < 0 )
        return reader.SetError( "Editor bulk data record with a negative payload size" );
    return !reader.HasError();
}

/*-------------------------------------------------------------------------*/

void
CUnrealEditorBulkData::ToReference( const CUnrealPackageTrailer* trailer, CUnrealPayloadReference& reference ) const
{GUCEF_TRACE;

    reference = CUnrealPayloadReference();
    reference.flags = flags;
    reference.rawSize = (UInt64) payloadSize;
    memcpy( reference.identifier, payloadId, sizeof( payloadId ) );
    reference.hasIdentifier = true;
    if ( 0 == payloadSize )
    {
        reference.kind = CUnrealPayloadReference::KIND_EMPTY;
        return;
    }
    reference.kind = CUnrealPayloadReference::KIND_UNAVAILABLE;
    if ( 0 == ( flags & FLAG_STORED_IN_PACKAGE_TRAILER ) )
    {
        {
            char hex[ 16 ];
            sprintf( hex, "%X", (unsigned int) flags );
            reference.reason = CORE::CString( "Payload is not stored in the package trailer (flags 0x" ) + hex + "); it is virtualized outside the package";
        }
        return;
    }
    if ( GUCEF_NULL == trailer )
    {
        reference.reason = "The record refers to the package trailer but the package has none";
        return;
    }
    const CUnrealPayloadTocEntry* entry = trailer->FindByIdentifier( payloadId );
    if ( GUCEF_NULL == entry )
    {
        reference.reason = "The payload is not in the table of the package trailer";
        return;
    }
    if ( 0 != entry->accessMode )
    {
        reference.reason = "The payload of the package trailer is stored elsewhere (access mode " + CORE::UInt32ToString( entry->accessMode ) + ")";
        return;
    }
    reference.kind = CUnrealPayloadReference::KIND_TRAILER;
    reference.offset = trailer->GetPayloadOffset( *entry );
    reference.storedSize = entry->compressedSize;
    reference.rawSize = entry->rawSize;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
