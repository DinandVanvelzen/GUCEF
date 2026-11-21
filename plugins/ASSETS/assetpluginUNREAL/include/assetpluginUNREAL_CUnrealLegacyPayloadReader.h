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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#include "assetpluginUNREAL_CUnrealPayloadReference.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H ? */

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

class CUnrealPackage;

/*-------------------------------------------------------------------------*/

/**
 *  The bytes of a payload that a legacy bulk data header (CUnrealLegacyBulkData) points at. Without the zlib flag (0x2) they are
 *  the payload. With it the payload is a chunked zlib stream, as the engines up to UE 4.27 write texture sources and mesh
 *  descriptions: a header of 64-bit numbers (the package tag 0x9E2A83C1, the chunk size, the compressed and the raw size of the
 *  whole), the compressed and raw size of every chunk, then the chunks, each a zlib stream of at most one chunk size of raw bytes.
 *  The header is checked against itself and against the bulk data header (the sums of the chunk sizes, the raw size of the
 *  bulk data, the stored size) before anything is inflated.
 *
 *  Inflating uses the codec "zlib" of the codec registry of gucefCORE (the plugin codecspluginZLIB provides it); when no such
 *  codec is loaded the read fails with 'codecMissing' set, so that the caller can report the payload as skipped for that reason.
 *  The budgets of the reader limits of the package apply: the raw size of a payload (maxPayloadRawBytes) and of a chunk
 *  (maxBlockBytes).
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealLegacyPayloadReader
{
    public:

    /** The name of the compression codec that inflates the chunks */
    static const char* const ZlibCodecName;

    /** Whether a codec that inflates the chunks is loaded */
    static bool IsZlibCodecAvailable( void );

    /**
     *  Reads the payload of the reference (of kind KIND_INLINE, with the flags of the legacy bulk data) into 'out'. False with the
     *  reason in 'error' when it cannot be read; 'codecMissing' tells that the payload is compressed and no codec inflates it.
     */
    static bool Read( CUnrealPackage& package                   ,
                      const CUnrealPayloadReference& reference  ,
                      CORE::CDynamicBuffer& out                 ,
                      bool& codecMissing                        ,
                      CORE::CString& error                      );

    /**
     *  Inflates a chunked zlib stream held in memory: 'stored' bytes at 'bytes', 'rawSize' the size the bulk data header gives (0 when
     *  not known). The limits are those of the reader limits. False with the reason in 'error'.
     */
    static bool InflateChunks( const UInt8* bytes                   ,
                               UInt64 stored                        ,
                               UInt64 rawSize                       ,
                               UInt64 maxRawBytes                   ,
                               UInt64 maxChunkBytes                 ,
                               CORE::CDynamicBuffer& out            ,
                               bool& codecMissing                   ,
                               CORE::CString& error                 );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H ? */
