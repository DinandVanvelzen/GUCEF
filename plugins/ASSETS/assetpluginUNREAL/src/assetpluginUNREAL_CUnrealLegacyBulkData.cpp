/*
 *  assetpluginUNREAL: The header of a legacy bulk data payload (packages of UE4 and early UE5)
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#include "assetpluginUNREAL_CUnrealLegacyBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H ? */

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

CUnrealLegacyBulkData::CUnrealLegacyBulkData( void )
    : flags( 0 )
    , elementCount( 0 )
    , sizeOnDisk( 0 )
    , offsetInFile( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealLegacyBulkData::Read( CUnrealArchiveReader& reader, const CUnrealVersionContext& )
{GUCEF_TRACE;

    flags = reader.ReadUInt32();
    if ( flags & FLAG_SIZE_64BIT )
    {
        elementCount = reader.ReadInt64();
        sizeOnDisk = reader.ReadInt64();
    }
    else
    {
        elementCount = reader.ReadInt32();
        sizeOnDisk = reader.ReadInt32();
    }
    offsetInFile = reader.ReadInt64();
    if ( flags & FLAG_BAD_DATA_VERSION )
        reader.ReadUInt16();
    if ( !reader.HasError() && ( elementCount < 0 || sizeOnDisk < 0 ) )
        return reader.SetError( "Legacy bulk data with a negative size" );
    return !reader.HasError();
}

/*-------------------------------------------------------------------------*/

void
CUnrealLegacyBulkData::ToReference( Int64 bulkDataStartOffset, UInt64 headerEnd, UInt64 packageSize, CUnrealPayloadReference& reference ) const
{GUCEF_TRACE;

    reference = CUnrealPayloadReference();
    reference.flags = flags;
    reference.storedSize = (UInt64) sizeOnDisk;
    reference.rawSize = (UInt64) elementCount;
    if ( 0 == sizeOnDisk || ( flags & FLAG_UNUSED ) )
    {
        reference.kind = CUnrealPayloadReference::KIND_EMPTY;
        return;
    }
    reference.kind = CUnrealPayloadReference::KIND_UNAVAILABLE;
    if ( flags & FLAG_PAYLOAD_IN_SEPERATE_FILE )
    {
        reference.reason = "Payload is in a separate file, which is not supported";
        return;
    }
    UInt64 offset = 0;
    if ( flags & FLAG_FORCE_INLINE_PAYLOAD )
        offset = headerEnd;
    else
    {
        Int64 absolute = offsetInFile;
        if ( 0 == ( flags & FLAG_NO_OFFSET_FIXUP ) )
            absolute += bulkDataStartOffset;
        if ( absolute < 0 )
        {
            reference.reason = "Payload offset is negative";
            return;
        }
        offset = (UInt64) absolute;
    }
    if ( offset > packageSize || reference.storedSize > packageSize - offset )
    {
        reference.reason = "Payload lies outside the package";
        return;
    }
    reference.kind = CUnrealPayloadReference::KIND_INLINE;
    reference.offset = offset;
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
