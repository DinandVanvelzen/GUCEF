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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H
#include "assetpluginUNREAL_CUnrealPackageTrailer.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H ? */

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

CUnrealPackageTrailer::CUnrealPackageTrailer( void )
    : m_version( 0 )
    , m_trailerOffset( 0 )
    , m_headerLength( 0 )
    , m_payloadsDataLength( 0 )
    , m_entries()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealPackageTrailer::Clear( void )
{GUCEF_TRACE;

    m_version = 0;
    m_trailerOffset = 0;
    m_headerLength = 0;
    m_payloadsDataLength = 0;
    m_entries.clear();
}

/*-------------------------------------------------------------------------*/

UInt32 CUnrealPackageTrailer::GetVersion( void ) const { return m_version; }
UInt64 CUnrealPackageTrailer::GetTrailerOffset( void ) const { return m_trailerOffset; }
UInt32 CUnrealPackageTrailer::GetHeaderLength( void ) const { return m_headerLength; }
UInt64 CUnrealPackageTrailer::GetPayloadsDataLength( void ) const { return m_payloadsDataLength; }
const CUnrealPackageTrailer::TEntryVector& CUnrealPackageTrailer::GetEntries( void ) const { return m_entries; }

/*-------------------------------------------------------------------------*/

const CUnrealPayloadTocEntry*
CUnrealPackageTrailer::FindByIdentifier( const UInt8* hash ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].HasIdentifier( hash ) )
            return &m_entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealPackageTrailer::GetPayloadOffset( const CUnrealPayloadTocEntry& entry ) const
{GUCEF_TRACE;

    return m_trailerOffset + m_headerLength + (UInt64) entry.offsetInFile;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageTrailer::Read( CUnrealArchiveReader& header, CUnrealArchiveReader& footer, UInt64 trailerOffset, UInt64 packageSize, const CUnrealReaderLimits& limits, CORE::CString& problem )
{GUCEF_TRACE;

    Clear();
    if ( trailerOffset >= packageSize || packageSize - trailerOffset < 28 + 20 )
    {
        problem = "The package trailer offset " + CORE::UInt64ToString( trailerOffset ) + " is outside the package of " + CORE::UInt64ToString( packageSize ) + " bytes";
        return false;
    }
    m_trailerOffset = trailerOffset;

    // the footer: tag, length of the whole trailer, package tag
    if ( !footer.Seek( packageSize - 20 ) )
    {
        problem = "The package trailer footer cannot be reached";
        return false;
    }
    const UInt64 footerTag = footer.ReadUInt64();
    const UInt64 trailerLength = footer.ReadUInt64();
    const UInt32 packageTag = footer.ReadUInt32();
    if ( footer.HasError() || FooterTag != footerTag || UE_PACKAGE_FILE_TAG != packageTag )
    {
        problem = "The package trailer footer is not valid";
        return false;
    }
    if ( trailerLength != packageSize - trailerOffset )
    {
        problem = "The package trailer length " + CORE::UInt64ToString( trailerLength ) + " does not match the offset " + CORE::UInt64ToString( trailerOffset ) + " in a package of " + CORE::UInt64ToString( packageSize ) + " bytes";
        return false;
    }

    const UInt64 tag = header.ReadUInt64();
    m_version = header.ReadUInt32();
    m_headerLength = header.ReadUInt32();
    m_payloadsDataLength = header.ReadUInt64();
    const UInt32 count = header.ReadUInt32();
    if ( header.HasError() || HeaderTag != tag )
    {
        problem = "The package trailer header is not valid";
        return false;
    }
    if ( m_version > 2 )
    {
        problem = "Package trailer version " + CORE::UInt32ToString( m_version ) + " is newer than the newest supported (2)";
        return false;
    }
    UInt32 entryBytes = 44;
    if ( m_version >= 1 ) entryBytes += 1;
    if ( m_version >= 2 ) entryBytes += 4;
    if ( count > limits.maxTableEntries || !header.CheckCount( count, entryBytes, "payload entries" ) )
    {
        problem = "The payload count " + CORE::UInt32ToString( count ) + " of the package trailer is not valid";
        return false;
    }
    if ( m_headerLength != 28 + (UInt64) count * entryBytes || m_headerLength + m_payloadsDataLength + 20 != trailerLength )
    {
        problem = "The package trailer sizes do not add up (header " + CORE::UInt32ToString( m_headerLength ) + ", payloads " + CORE::UInt64ToString( m_payloadsDataLength ) + ", total " + CORE::UInt64ToString( trailerLength ) + ")";
        return false;
    }
    m_entries.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        CUnrealPayloadTocEntry entry;
        header.Read( entry.identifier, sizeof( entry.identifier ) );
        entry.offsetInFile = header.ReadInt64();
        entry.compressedSize = header.ReadUInt64();
        entry.rawSize = header.ReadUInt64();
        if ( m_version >= 2 )
            entry.flags = header.ReadUInt32();
        if ( m_version >= 1 )
            entry.accessMode = header.ReadUInt8();
        if ( header.HasError() )
        {
            problem = "The payload table of the package trailer is truncated";
            return false;
        }
        if ( 0 == entry.accessMode && ( entry.offsetInFile < 0 || (UInt64) entry.offsetInFile > m_payloadsDataLength || entry.compressedSize > m_payloadsDataLength - (UInt64) entry.offsetInFile ) )
        {
            problem = "Payload " + CORE::UInt32ToString( i ) + " of the package trailer lies outside the payload data";
            return false;
        }
        m_entries.push_back( entry );
    }
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
