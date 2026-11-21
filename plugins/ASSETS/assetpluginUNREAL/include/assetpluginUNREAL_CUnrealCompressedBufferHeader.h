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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The 64 byte header (big endian) of a payload, followed by the compressed size of each block. Method 0 means the
 *  blocks are stored without compression; other methods name the codec that is needed to read the blocks.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealCompressedBufferHeader
{
    public:

    static const UInt32 MAGIC = 0xB7756362;

    enum EMethod
    {
        METHOD_NONE = 0,
        METHOD_OODLE = 3,
        METHOD_LZ4 = 4
    };

    UInt32 crc32;
    UInt8 method;
    UInt8 compressor;
    UInt8 compressionLevel;
    UInt8 blockSizeLog2;
    UInt32 blockCount;
    UInt64 rawSize;
    UInt64 compressedSize;
    GUCEF::vector< UInt32 > blockSizes;     /**< compressed bytes of each block */

    CUnrealCompressedBufferHeader( void );

    /**
     *  Reads the header and, for the compressed methods, the block sizes; refuses a header whose sizes do not add up. A stored
     *  buffer (method 0) has one block and no table: the raw bytes follow the 64 byte header.
     */
    bool Read( CUnrealArchiveReader& reader, UInt64 storedSize );

    bool IsStored( void ) const;

    /** Bytes of the header with the block size table: where the first block starts */
    UInt64 GetHeaderBytes( void ) const;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H ? */
