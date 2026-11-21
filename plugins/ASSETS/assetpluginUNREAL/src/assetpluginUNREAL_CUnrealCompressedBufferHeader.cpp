/*
 *  assetpluginUNREAL: The header of a compressed buffer, the container of the payloads of a package trailer
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#include "assetpluginUNREAL_CUnrealCompressedBufferHeader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H ? */

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

CUnrealCompressedBufferHeader::CUnrealCompressedBufferHeader( void )
    : crc32( 0 )
    , method( 0 )
    , compressor( 0 )
    , compressionLevel( 0 )
    , blockSizeLog2( 0 )
    , blockCount( 0 )
    , rawSize( 0 )
    , compressedSize( 0 )
    , blockSizes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealCompressedBufferHeader::IsStored( void ) const
{GUCEF_TRACE;

    return METHOD_NONE == method;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealCompressedBufferHeader::GetHeaderBytes( void ) const
{GUCEF_TRACE;

    return METHOD_NONE == method ? 64 : 64 + (UInt64) blockCount * 4;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCompressedBufferHeader::Read( CUnrealArchiveReader& reader, UInt64 storedSize )
{GUCEF_TRACE;

    blockSizes.clear();
    const UInt32 magic = reader.ReadUInt32BE();
    crc32 = reader.ReadUInt32BE();
    method = reader.ReadUInt8();
    compressor = reader.ReadUInt8();
    compressionLevel = reader.ReadUInt8();
    blockSizeLog2 = reader.ReadUInt8();
    blockCount = reader.ReadUInt32BE();
    rawSize = reader.ReadUInt64BE();
    compressedSize = reader.ReadUInt64BE();
    reader.Skip( 32 ); // hash of the raw bytes
    if ( reader.HasError() )
        return false;
    if ( MAGIC != magic )
        return reader.SetError( "Compressed buffer header has a wrong magic number" );
    if ( METHOD_NONE == method )
    {
        // stored: the raw bytes follow the 64 byte header, in one block, without a table of block sizes
        if ( blockCount > 1 || ( 0 == blockCount && rawSize > 0 ) )
            return reader.SetError( "Stored compressed buffer with a block count that is not valid" );
        if ( compressedSize != 64 + rawSize || ( 0 != storedSize && compressedSize > storedSize ) )
            return reader.SetError( "Stored compressed buffer: the sizes do not add up" );
        if ( 1 == blockCount )
            blockSizes.push_back( (UInt32) rawSize );
        return true;
    }
    if ( blockSizeLog2 > 30 || ( blockCount > 0 && blockSizeLog2 < 8 ) )
        return reader.SetError( "Compressed buffer header has a block size that is not valid" );
    if ( !reader.CheckCount( blockCount, 4, "compressed blocks" ) )
        return false;
    if ( 0 == blockCount && rawSize > 0 )
        return reader.SetError( "Compressed buffer header has data but no blocks" );
    if ( blockCount > 0 && rawSize > ( (UInt64) blockCount << blockSizeLog2 ) )
        return reader.SetError( "Compressed buffer header: the raw size is larger than the blocks can hold" );
    if ( 0 != storedSize && compressedSize > storedSize )
        return reader.SetError( "Compressed buffer header: the compressed size is larger than the stored payload" );
    UInt64 total = 0;
    blockSizes.reserve( blockCount );
    for ( UInt32 i=0; i<blockCount; ++i )
    {
        const UInt32 size = reader.ReadUInt32BE();
        total += size;
        blockSizes.push_back( size );
    }
    if ( reader.HasError() )
        return false;
    return true;
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
