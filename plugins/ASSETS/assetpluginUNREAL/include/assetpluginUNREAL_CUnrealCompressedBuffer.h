/*
 *  assetpluginUNREAL: Reader of the payloads that are compressed buffers (stored or Oodle blocks)
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#include "assetpluginUNREAL_CUnrealCompressedBufferHeader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#include "assetpluginUNREAL_CIUnrealOodleProvider.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H ? */

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
 *  The decompressed bytes of a payload. Open() reads the header and the block table of the compressed buffer; ReadAll() and
 *  ReadRange() give the raw bytes. A stored buffer (method 0) is read directly; the blocks of an Oodle buffer (method 3) are
 *  decompressed one by one through the provider, and only the blocks a range touches are decompressed. A block whose compressed
 *  size equals its raw size is stored as it is (the encoder does that when compression does not help).
 *
 *  Checks: the magic number, the sizes of the header against each other and against the stored size of the payload, the number of
 *  blocks against the raw size, the sum of the block sizes against the compressed size, the checksum of the header, the size of
 *  every decompressed block. The budget is CUnrealReaderLimits::maxPayloadRawBytes (what ReadAll() holds at once) and maxBlockBytes.
 *  The 32 byte hash of the raw bytes in the header is not verified (it is not a hash that this module implements).
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealCompressedBuffer
{
    public:

    /** @param provider the Oodle provider; NULL when none exists (Oodle buffers then fail with the reason) */
    CUnrealCompressedBuffer( CUnrealPackage& package, CIUnrealOodleProvider* provider );

    ~CUnrealCompressedBuffer();

    /** The report that is polled for the request to cancel (between blocks); NULL for none */
    void SetCancellationSource( const ASSETCONVERTER::CConversionReport* report );

    bool Open( const CUnrealPayloadReference& reference );

    bool IsOpen( void ) const;

    const CUnrealCompressedBufferHeader& GetHeader( void ) const;

    UInt64 GetRawSize( void ) const;

    /** Whether the blocks need the Oodle provider (method 3) */
    bool NeedsOodle( void ) const;

    /** Whether the last failure was the missing provider (tool not found), not damaged data */
    bool IsProviderMissing( void ) const;

    /** Whether the last failure was a request to cancel */
    bool IsCancelled( void ) const;

    /** The exit code or answer code of the tool for the block that failed, 0 when not the tool's failure */
    Int32 GetToolCode( void ) const;

    const CORE::CString& GetError( void ) const;

    /** All raw bytes, within the budget */
    bool ReadAll( CORE::CDynamicBuffer& out );

    /** Raw bytes [offset,offset+size) into dest; only the blocks that overlap the range are decompressed */
    bool ReadRange( UInt64 offset, UInt64 size, void* dest );

    /** The checksum of the header: CRC-32 (the zip and PNG one) over the bytes after the checksum field up to the end of the block table */
    static UInt32 HeaderChecksum( const UInt8* bytes, UInt32 size );

    private:

    CUnrealCompressedBuffer( const CUnrealCompressedBuffer& src );              /**< not implemented */
    CUnrealCompressedBuffer& operator=( const CUnrealCompressedBuffer& src );   /**< not implemented */

    bool Fail( const CORE::CString& reason );
    UInt32 RawBlockSize( UInt32 block ) const;
    bool ReadBlock( CUnrealArchiveReader& reader, UInt32 block, void* dest );

    private:

    typedef GUCEF::vector< UInt64 > TOffsetVector;

    CUnrealPackage& m_package;
    CIUnrealOodleProvider* m_provider;
    const ASSETCONVERTER::CConversionReport* m_cancellation;
    CUnrealPayloadReference m_reference;
    CUnrealCompressedBufferHeader m_header;
    TOffsetVector m_blockOffsets;           /**< of the compressed bytes of each block, relative to the start of the payload */
    GUCEF::vector< UInt8 > m_compressed;
    bool m_open;
    bool m_providerMissing;
    bool m_cancelled;
    Int32 m_toolCode;
    CORE::CString m_error;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H ? */
