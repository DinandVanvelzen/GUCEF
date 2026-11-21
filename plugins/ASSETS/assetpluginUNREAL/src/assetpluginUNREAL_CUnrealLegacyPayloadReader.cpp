/*
 *  assetpluginUNREAL: Reads the bytes of a legacy bulk data payload, stored as they are or compressed with zlib in chunks
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

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#include "assetpluginUNREAL_CUnrealLegacyBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#include "assetpluginUNREAL_CUnrealLegacyPayloadReader.h"

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

const char* const CUnrealLegacyPayloadReader::ZlibCodecName = "zlib";

/*-------------------------------------------------------------------------*/

/** The codec of the highest priority that inflates zlib streams; NULL when none is loaded */
static CORE::CCodecRegistry::TICodecPtr
FindZlibCodec( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtrVector codecs;
    if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().GetCodecs( CORE::CoreCodecTypes::CompressionCodec, CUnrealLegacyPayloadReader::ZlibCodecName, codecs ) && !codecs.empty() )
        return codecs.front();
    return CORE::CCodecRegistry::TICodecPtr();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealLegacyPayloadReader::IsZlibCodecAvailable( void )
{GUCEF_TRACE;

    return !FindZlibCodec().IsNULL();
}

/*-------------------------------------------------------------------------*/

/** A little endian 64-bit number of the header of the chunks */
static Int64
Int64At( const UInt8* bytes )
{
    UInt64 value = 0;
    for ( UInt32 i=0; i<8; ++i )
        value |= ( (UInt64) bytes[ i ] ) << ( 8 * i );
    return (Int64) value;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealLegacyPayloadReader::InflateChunks( const UInt8* bytes                   ,
                                           UInt64 stored                        ,
                                           UInt64 rawSize                       ,
                                           UInt64 maxRawBytes                   ,
                                           UInt64 maxChunkBytes                 ,
                                           CORE::CDynamicBuffer& out            ,
                                           bool& codecMissing                   ,
                                           CORE::CString& error                 )
{GUCEF_TRACE;

    codecMissing = false;
    out.Clear( true );
    if ( GUCEF_NULL == bytes || stored < 32 )
    {
        error = "The compressed payload is shorter than the header of its chunks";
        return false;
    }
    const Int64 tag = Int64At( bytes );
    const Int64 chunkSize = Int64At( bytes + 8 );
    const Int64 totalCompressed = Int64At( bytes + 16 );
    const Int64 totalRaw = Int64At( bytes + 24 );
    if ( (Int64) UE_PACKAGE_FILE_TAG != tag )
    {
        error = "The compressed payload does not start with the package tag (byte swapped or 32-bit chunk headers are not read)";
        return false;
    }
    if ( chunkSize <= 0 || (UInt64) chunkSize > maxChunkBytes || totalCompressed < 0 || totalRaw < 0 )
    {
        error = "The header of the compressed payload has a chunk size or totals that are not possible (chunk size " + CORE::Int64ToString( chunkSize ) + ")";
        return false;
    }
    if ( (UInt64) totalRaw > maxRawBytes || (UInt64) totalRaw > 0xFFFFFFF0ULL )
    {
        error = "The payload has " + CORE::Int64ToString( totalRaw ) + " raw bytes, more than the budget of " + CORE::UInt64ToString( maxRawBytes );
        return false;
    }
    if ( 0 != rawSize && (UInt64) totalRaw != rawSize )
    {
        error = "The raw size of the compressed payload (" + CORE::Int64ToString( totalRaw ) + ") differs from the size of the bulk data (" + CORE::UInt64ToString( rawSize ) + ")";
        return false;
    }
    const UInt64 chunkCount = ( (UInt64) totalRaw + (UInt64) chunkSize - 1 ) / (UInt64) chunkSize;
    const UInt64 headerBytes = 32 + chunkCount * 16;
    if ( headerBytes > stored || (UInt64) totalCompressed > stored - headerBytes )
    {
        error = "The chunks of the compressed payload do not fit in its stored size";
        return false;
    }

    // the sizes of the chunks must add up to the totals before anything is inflated
    UInt64 sumCompressed = 0, sumRaw = 0;
    for ( UInt64 c=0; c<chunkCount; ++c )
    {
        const Int64 compressed = Int64At( bytes + 32 + c * 16 );
        const Int64 raw = Int64At( bytes + 32 + c * 16 + 8 );
        if ( compressed <= 0 || raw <= 0 || raw > chunkSize || (UInt64) compressed > maxChunkBytes )
        {
            error = "Chunk " + CORE::UInt64ToString( c ) + " of the compressed payload has sizes that are not possible";
            return false;
        }
        sumCompressed += (UInt64) compressed;
        sumRaw += (UInt64) raw;
    }
    if ( sumCompressed != (UInt64) totalCompressed || sumRaw != (UInt64) totalRaw )
    {
        error = "The sizes of the chunks of the compressed payload do not add up to its totals";
        return false;
    }

    CORE::CCodecRegistry::TICodecPtr codec = FindZlibCodec();
    if ( codec.IsNULL() )
    {
        codecMissing = true;
        error = "The payload is compressed with zlib and no zlib codec is loaded (the plugin codecspluginZLIB provides it)";
        return false;
    }
    if ( !out.SetBufferSize( (UInt32) totalRaw, false ) )
    {
        error = "The memory for the raw payload cannot be had";
        return false;
    }
    out.SetDataSize( 0 );
    UInt64 position = headerBytes;
    for ( UInt64 c=0; c<chunkCount; ++c )
    {
        const UInt64 compressed = (UInt64) Int64At( bytes + 32 + c * 16 );
        const UInt64 raw = (UInt64) Int64At( bytes + 32 + c * 16 + 8 );
        CORE::CDynamicBuffer source;
        source.CopyFrom( (UInt32) compressed, bytes + position );
        CORE::CDynamicBuffer chunk;
        chunk.SetBufferSize( (UInt32) raw, false );
        CORE::CDynamicBufferAccess sourceAccess( &source, false );
        CORE::CDynamicBufferAccess chunkAccess( &chunk, false );
        if ( !codec->Decode( sourceAccess, chunkAccess ) || chunk.GetDataSize() != raw )
        {
            error = "Chunk " + CORE::UInt64ToString( c ) + " of the compressed payload does not inflate to its " + CORE::UInt64ToString( raw ) + " bytes";
            out.Clear( true );
            return false;
        }
        out.CopyFrom( chunk, out.GetDataSize() );
        position += compressed;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealLegacyPayloadReader::Read( CUnrealPackage& package                   ,
                                  const CUnrealPayloadReference& reference  ,
                                  CORE::CDynamicBuffer& out                 ,
                                  bool& codecMissing                        ,
                                  CORE::CString& error                      )
{GUCEF_TRACE;

    codecMissing = false;
    out.Clear( true );
    if ( !reference.IsReadable() )
    {
        error = "The payload is not in this package: " + reference.reason;
        return false;
    }
    const UInt64 stored = reference.storedSize;
    const bool compressed = 0 != ( reference.flags & CUnrealLegacyBulkData::FLAG_COMPRESSED_ZLIB );
    const UInt64 budget = package.GetLimits().maxPayloadRawBytes;
    if ( stored > budget || stored > 0xFFFFFFF0ULL )
    {
        error = "The stored payload has " + CORE::UInt64ToString( stored ) + " bytes, more than the budget of " + CORE::UInt64ToString( budget );
        return false;
    }
    CUnrealArchiveReader reader;
    if ( !package.OpenPayload( reference, reader ) )
    {
        error = "The payload cannot be opened";
        return false;
    }
    CORE::CDynamicBuffer bytes;
    if ( !bytes.SetBufferSize( (UInt32) stored, false ) )
    {
        error = "The memory for the stored payload cannot be had";
        return false;
    }
    bytes.SetDataSize( (UInt32) stored );
    if ( stored > 0 && !reader.Read( bytes.GetBufferPtr(), stored ) )
    {
        error = "The payload cannot be read: " + reader.GetError();
        return false;
    }
    package.AccountFetched( reader );
    if ( !compressed )
    {
        out = bytes;
        return true;
    }
    return InflateChunks( static_cast< const UInt8* >( bytes.GetConstBufferPtr() ), stored, reference.rawSize, package.GetLimits().maxPayloadRawBytes,
                          package.GetLimits().maxBlockBytes, out, codecMissing, error );
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
