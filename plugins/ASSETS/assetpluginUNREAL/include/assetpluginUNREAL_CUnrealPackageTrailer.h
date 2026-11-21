/*
 *  assetpluginUNREAL: The package trailer: the table of the payloads that sit behind the exports
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H
#include "assetpluginUNREAL_CUnrealPayloadTocEntry.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#include "assetpluginUNREAL_CUnrealReaderLimits.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H ? */

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
 *  Header, payload table and footer of the trailer at the end of a package. The payloads follow the header; the offset of
 *  an entry is relative to the end of the header. Versions 0 to 2 of the trailer are read: version 1 adds the access
 *  mode, version 2 the flags of a payload.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPackageTrailer
{
    public:

    static const UInt64 HeaderTag = 0xD1C43B2E80A5F697ULL;
    static const UInt64 FooterTag = 0x29BFCA045138DE76ULL;

    typedef GUCEF::vector< CUnrealPayloadTocEntry > TEntryVector;

    CUnrealPackageTrailer( void );

    void Clear( void );

    /**
     *  Reads the trailer that starts at trailerOffset and runs to the end of the package of the given size. The reader
     *  is positioned at the header; the footer is read with the reader that is given for it.
     */
    bool Read( CUnrealArchiveReader& header, CUnrealArchiveReader& footer, UInt64 trailerOffset, UInt64 packageSize, const CUnrealReaderLimits& limits, CORE::CString& problem );

    UInt32 GetVersion( void ) const;

    UInt64 GetTrailerOffset( void ) const;

    UInt32 GetHeaderLength( void ) const;

    UInt64 GetPayloadsDataLength( void ) const;

    const TEntryVector& GetEntries( void ) const;

    /** The entry with the given hash, or NULL */
    const CUnrealPayloadTocEntry* FindByIdentifier( const UInt8* hash ) const;

    /** Where the bytes of an entry start in the package */
    UInt64 GetPayloadOffset( const CUnrealPayloadTocEntry& entry ) const;

    private:

    UInt32 m_version;
    UInt64 m_trailerOffset;
    UInt32 m_headerLength;
    UInt64 m_payloadsDataLength;
    TEntryVector m_entries;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H ? */
