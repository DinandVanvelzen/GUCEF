/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREECODECTYPES_H
#endif /* GUCEF_CORE_COREECODECTYPES_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHBINARY_H
#include "gucefMULTIPLAY_CMatchBinary.h"
#define GUCEF_MULTIPLAY_CMATCHBINARY_H
#endif /* GUCEF_MULTIPLAY_CMATCHBINARY_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#include "gucefMULTIPLAY_CMatchContainer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt8 ContainerMagic[ 4 ] = { 'G', 'M', 'T', 'C' };
static const UInt8 ContainerVersion = 1;
static const UInt8 ContainerFlagGzip = 0x01;

/** The input of gzip data is given to the codec in steps this large: a step expands to at most about a thousand times that */
static const UInt32 GzipInputStepSize = 2048;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CCodecRegistry::TICodecPtr
GetGzipCodec( void )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip" );
}

/*-------------------------------------------------------------------------*/

static bool
GzipEncode( const CORE::CDynamicBuffer& raw, CORE::CDynamicBuffer& compressed )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtr codec = GetGzipCodec();
    CORE::CICodecInstancePtr instance;
    if ( codec.IsNULL() || !codec->CreateInstance( CORE::CValueList(), instance ) || instance.IsNULL() )
        return false;
    compressed.Clear();
    return instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, raw.GetConstBufferPtr(), raw.GetDataSize(), GUCEF_NULL, compressed );
}

/*-------------------------------------------------------------------------*/

/** Expands gzip data in small steps and gives up as soon as the output is more than maxSize */
static TMatchContainerResult
GzipDecodeBounded( const UInt8* data, UInt32 size, UInt32 maxSize, CORE::CDynamicBuffer& raw )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtr codec = GetGzipCodec();
    if ( codec.IsNULL() )
        return MATCHCONTAINER_CODEC_UNAVAILABLE;
    CORE::CICodecInstancePtr instance;
    if ( !codec->CreateInstance( CORE::CValueList(), instance ) || instance.IsNULL() )
        return MATCHCONTAINER_CODEC_UNAVAILABLE;

    raw.Clear();
    raw.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    UInt32 offset = 0;
    UInt32 flags = GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_DECODE;
    do
    {
        UInt32 step = size - offset;
        if ( step > GzipInputStepSize )
            step = GzipInputStepSize;
        if ( offset + step == size )
            flags |= GUCEF_CODEC_PROCESS_FINAL;
        if ( !instance->Process( flags, data + offset, step, GUCEF_NULL, raw ) )
            return MATCHCONTAINER_BAD_COMPRESSION;
        if ( raw.GetDataSize() > maxSize )
            return MATCHCONTAINER_BAD_COMPRESSION;
        offset += step;
        flags = GUCEF_CODEC_PROCESS_UPDATE | GUCEF_CODEC_PROCESS_DECODE;
    }
    while ( offset < size );
    return MATCHCONTAINER_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
MatchContainerResultToString( TMatchContainerResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case MATCHCONTAINER_OK:                 return "ok";
        case MATCHCONTAINER_TOO_LARGE:          return "The data is larger than a turn may be";
        case MATCHCONTAINER_TRUNCATED:          return "The data ends before the end of the turn";
        case MATCHCONTAINER_BAD_MAGIC:          return "The data is not a turn";
        case MATCHCONTAINER_BAD_VERSION:        return "The turn is of a version that is not known";
        case MATCHCONTAINER_BAD_HEADER:         return "The header of the turn is not valid";
        case MATCHCONTAINER_BAD_PREVIEW:        return "The preview in the turn is not valid";
        case MATCHCONTAINER_BAD_PAYLOAD:        return "The content of the turn is damaged";
        case MATCHCONTAINER_BAD_COMPRESSION:    return "The compressed content of the turn is damaged";
        case MATCHCONTAINER_CODEC_UNAVAILABLE:  return "The turn is compressed and there is no codec to read it";
        case MATCHCONTAINER_INCONSISTENT:       return "The parts of the turn do not belong together";
        default:                                return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

CMatchContainer::CMatchContainer( void )
    : preview()
    , package()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchContainer::CMatchContainer( const CMatchContainer& src )
    : preview( src.preview )
    , package( src.package )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchContainer::~CMatchContainer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchContainer&
CMatchContainer::operator=( const CMatchContainer& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        preview = src.preview;
        package = src.package;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchContainer::IsCompressionAvailable( void )
{GUCEF_TRACE;

    return !GetGzipCodec().IsNULL();
}

/*-------------------------------------------------------------------------*/

TMatchContainerResult
CMatchContainer::Encode( const CMatchPreview& preview   ,
                         const CMatchPackage& package   ,
                         bool compress                  ,
                         CORE::CDynamicBuffer& bytes    )
{GUCEF_TRACE;

    bytes.Clear();
    if ( package.payload.GetDataSize() > CMatchPackage::MaxPayloadSize )
        return MATCHCONTAINER_TOO_LARGE;
    if ( !package.IsConsistent() || preview.packageSha256 != package.sha256 || preview.revision != package.revision ||
         preview.baseSha256 != package.baseSha256 || 0 == package.revision || package.baseRevision != package.revision - 1 )
        return MATCHCONTAINER_INCONSISTENT;

    CORE::CDynamicBuffer previewBytes;
    if ( !preview.ToBinary( previewBytes ) )
        return MATCHCONTAINER_BAD_PREVIEW;

    UInt8 baseSha[ 32 ];
    memset( baseSha, 0, sizeof( baseSha ) );
    if ( !package.baseSha256.IsNULLOrEmpty() )
    {
        if ( 0 == package.baseRevision || !DRN::CDRNCrypto::FromHex( package.baseSha256, baseSha, 32 ) )
            return MATCHCONTAINER_INCONSISTENT;
    }
    else
    if ( 0 != package.baseRevision )
        return MATCHCONTAINER_INCONSISTENT;
    UInt8 payloadSha[ 32 ];
    if ( !DRN::CDRNCrypto::FromHex( package.sha256, payloadSha, 32 ) )
        return MATCHCONTAINER_INCONSISTENT;

    CORE::CDynamicBuffer compressed;
    const CORE::CDynamicBuffer* stored = &package.payload;
    UInt8 flags = 0;
    if ( compress && package.payload.GetDataSize() > 0 && GzipEncode( package.payload, compressed ) &&
         compressed.GetDataSize() < package.payload.GetDataSize() )
    {
        stored = &compressed;
        flags |= ContainerFlagGzip;
    }

    if ( (UInt64) HeaderSize + previewBytes.GetDataSize() + stored->GetDataSize() > MaxContainerSize )
        return MATCHCONTAINER_TOO_LARGE;

    CMatchBinaryWriter writer( bytes );
    writer.WriteRaw( ContainerMagic, 4 );
    writer.WriteUInt8( ContainerVersion );
    writer.WriteUInt8( flags );
    writer.WriteUInt16( (UInt16) previewBytes.GetDataSize() );
    writer.WriteUInt32( stored->GetDataSize() );
    writer.WriteUInt32( package.payload.GetDataSize() );
    writer.WriteUInt32( package.revision );
    writer.WriteUInt32( package.baseRevision );
    writer.WriteRaw( baseSha, 32 );
    writer.WriteRaw( payloadSha, 32 );
    writer.WriteRaw( previewBytes.GetConstBufferPtr(), previewBytes.GetDataSize() );
    writer.WriteRaw( stored->GetConstBufferPtr(), stored->GetDataSize() );
    return MATCHCONTAINER_OK;
}

/*-------------------------------------------------------------------------*/

TMatchContainerResult
CMatchContainer::Decode( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( size > MaxContainerSize )
        return MATCHCONTAINER_TOO_LARGE;
    if ( GUCEF_NULL == data || size < 4 )
        return size > 0 && GUCEF_NULL != data && 0 != memcmp( data, ContainerMagic, size ) ? MATCHCONTAINER_BAD_MAGIC : MATCHCONTAINER_TRUNCATED;

    CMatchBinaryReader reader( data, size );
    UInt8 magic[ 4 ];
    reader.ReadRaw( magic, 4 );
    if ( 0 != memcmp( magic, ContainerMagic, 4 ) )
        return MATCHCONTAINER_BAD_MAGIC;
    if ( size < HeaderSize )
        return MATCHCONTAINER_TRUNCATED;

    UInt8 version = reader.ReadUInt8();
    UInt8 flags = reader.ReadUInt8();
    UInt32 previewSize = reader.ReadUInt16();
    UInt32 storedSize = reader.ReadUInt32();
    UInt32 rawSize = reader.ReadUInt32();
    UInt32 revision = reader.ReadUInt32();
    UInt32 baseRevision = reader.ReadUInt32();
    UInt8 baseSha[ 32 ];
    UInt8 payloadSha[ 32 ];
    reader.ReadRaw( baseSha, 32 );
    reader.ReadRaw( payloadSha, 32 );
    if ( !reader.IsOk() )
        return MATCHCONTAINER_TRUNCATED;

    if ( version != ContainerVersion )
        return MATCHCONTAINER_BAD_VERSION;
    if ( 0 != ( flags & ~ContainerFlagGzip ) || 0 == previewSize || previewSize > CMatchPreview::MaxBinarySize ||
         storedSize > MaxContainerSize || rawSize > CMatchPackage::MaxPayloadSize )
        return MATCHCONTAINER_BAD_HEADER;
    if ( 0 == ( flags & ContainerFlagGzip ) && storedSize != rawSize )
        return MATCHCONTAINER_BAD_HEADER;
    if ( 0 != ( flags & ContainerFlagGzip ) && 0 == rawSize )
        return MATCHCONTAINER_BAD_HEADER;

    // the sizes decide what the whole has to be, exactly
    UInt64 expectedSize = (UInt64) HeaderSize + previewSize + storedSize;
    if ( size < expectedSize )
        return MATCHCONTAINER_TRUNCATED;
    if ( size > expectedSize )
        return MATCHCONTAINER_BAD_HEADER;

    // a first package has no base
    static const UInt8 zeroSha[ 32 ] = { 0 };
    bool hasBase = 0 != memcmp( baseSha, zeroSha, 32 );
    if ( ( 0 == baseRevision ) == hasBase )
        return MATCHCONTAINER_BAD_HEADER;

    const UInt8* previewBytes = reader.ReadRawInPlace( previewSize );
    const UInt8* storedBytes = reader.ReadRawInPlace( storedSize );
    if ( GUCEF_NULL == previewBytes || ( storedSize > 0 && GUCEF_NULL == storedBytes ) || !reader.IsAtEndAndOk() )
        return MATCHCONTAINER_TRUNCATED;

    CMatchPreview newPreview;
    if ( !newPreview.FromBinary( previewBytes, previewSize ) )
        return MATCHCONTAINER_BAD_PREVIEW;

    CMatchPackage newPackage;
    CORE::CDynamicBuffer raw;
    if ( 0 != ( flags & ContainerFlagGzip ) )
    {
        TMatchContainerResult expanded = GzipDecodeBounded( storedBytes, storedSize, rawSize, raw );
        if ( MATCHCONTAINER_OK != expanded )
            return expanded;
        if ( raw.GetDataSize() != rawSize )
            return MATCHCONTAINER_BAD_COMPRESSION;
    }
    else
    if ( rawSize > 0 )
        raw.Append( storedBytes, rawSize );

    UInt8 actualSha[ 32 ];
    DRN::CDRNCrypto::Sha256( raw.GetConstBufferPtr(), raw.GetDataSize(), actualSha );
    if ( !CORE::CSha256::ConstantTimeEquals( actualSha, payloadSha, 32 ) )
        return MATCHCONTAINER_BAD_PAYLOAD;

    newPackage.payload.Swap( raw );
    newPackage.sha256 = DRN::CDRNCrypto::ToHex( payloadSha, 32 );
    newPackage.revision = revision;
    newPackage.baseRevision = baseRevision;
    newPackage.baseSha256 = hasBase ? DRN::CDRNCrypto::ToHex( baseSha, 32 ) : CORE::CString::Empty;

    if ( newPreview.revision != revision || newPreview.packageSha256 != newPackage.sha256 || newPreview.baseSha256 != newPackage.baseSha256 ||
         0 == revision || baseRevision != revision - 1 )
        return MATCHCONTAINER_INCONSISTENT;

    preview = newPreview;
    package.payload.Swap( newPackage.payload );
    package.sha256 = newPackage.sha256;
    package.revision = newPackage.revision;
    package.baseRevision = newPackage.baseRevision;
    package.baseSha256 = newPackage.baseSha256;
    return MATCHCONTAINER_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
