/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */

#include "gucefASSETSTORE_CAssetItemRevision.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
Sha256OfText( const CORE::CString& text )
{
    CAssetDigest digest( CAssetDigest::Sha256 );
    const UInt32 size = text.ByteSize();
    digest.Update( text.C_String(), size > 0 ? size - 1 : 0 );
    return digest.FinalHex();
}

/** Sorts names in place (insertion sort: the lists are small) */
void
SortTexts( CORE::CString::StringVector& texts )
{
    for ( size_t i=1; i<texts.size(); ++i )
    {
        const CORE::CString value = texts[ i ];
        size_t j = i;
        while ( j > 0 && value < texts[ j - 1 ] )
        {
            texts[ j ] = texts[ j - 1 ];
            --j;
        }
        texts[ j ] = value;
    }
}

}

/*-------------------------------------------------------------------------*/

const CORE::CString CAssetItemRevision::TimestampBasis = "ts";
const CORE::CString CAssetItemRevision::VersionBasis = "ver";
const CORE::CString CAssetItemRevision::RevisionBasis = "rev";
const CORE::CString CAssetItemRevision::FormatBasis = "fmt";
const CORE::CString CAssetItemRevision::ListingBasis = "lst";
const CORE::CString CAssetItemRevision::UnknownBasis = "unk";

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetItemRevision::MakeTagged( const CORE::CString& basis, const CORE::CString& value )
{GUCEF_TRACE;

    return basis + ":" + value;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetItemRevision::GetBasis( const CORE::CString& tagged )
{GUCEF_TRACE;

    if ( tagged.IsNULLOrEmpty() || tagged.HasChar( ':' ) < 0 )
        return CORE::CString::Empty;
    return tagged.SubstrToChar( ':', true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetItemRevision::GetValue( const CORE::CString& tagged )
{GUCEF_TRACE;

    const Int32 colon = tagged.HasChar( ':' );
    if ( colon < 0 )
        return CORE::CString::Empty;
    return tagged.CutChars( (UInt32) colon + 1, true );
}

/*-------------------------------------------------------------------------*/

Int64
CAssetItemRevision::GetChangedMs( const CORE::CString& tagged )
{GUCEF_TRACE;

    if ( TimestampBasis != GetBasis( tagged ) )
        return 0;
    return CORE::StringToInt64( GetValue( tagged ), 0 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetItemRevision::ComputeFormatFingerprint( const CAssetStoreItem& item )
{GUCEF_TRACE;

    if ( item.GetEffectiveFormatsLevel() < 2 )
        return CORE::CString::Empty;
    CORE::CString::StringVector lines;
    for ( size_t f=0; f<item.formats.size(); ++f )
    {
        const CAssetStoreFormat& format = item.formats[ f ];
        for ( size_t i=0; i<format.files.size(); ++i )
        {
            const CAssetStoreFile& file = format.files[ i ];
            lines.push_back( format.formatId + "\t" + file.fileId + "\t" + CORE::ToString( file.size ) + "\t" + file.digest.Lowercase() );
        }
        if ( format.files.empty() )
            lines.push_back( format.formatId );
    }
    SortTexts( lines );
    CORE::CString text;
    for ( size_t l=0; l<lines.size(); ++l )
        text += lines[ l ] + "\n";
    return Sha256OfText( text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetItemRevision::ComputeListingFingerprint( const CAssetStoreItem& item )
{GUCEF_TRACE;

    CORE::CString::StringVector formatIds;
    for ( size_t f=0; f<item.formats.size(); ++f )
        formatIds.push_back( item.formats[ f ].formatId );
    SortTexts( formatIds );
    CORE::CString text = item.itemId + "\n" + item.title + "\n" + item.version + "\n" + item.listingType + "\n";
    for ( size_t f=0; f<formatIds.size(); ++f )
        text += formatIds[ f ] + "\n";
    return Sha256OfText( text );
}

const CORE::CString CAssetItemRevision::EntryChangeHintAttribute = "entryChangeHint";

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetItemRevision::ComputeEntryFingerprint( const CAssetStoreItem& item )
{GUCEF_TRACE;

    CORE::CString::StringVector formats;
    for ( size_t f=0; f<item.formats.size(); ++f )
        formats.push_back( item.formats[ f ].formatId + "\t" + item.formats[ f ].name + "\t" + CORE::ToString( (UInt32) item.formats[ f ].files.size() ) );
    SortTexts( formats );
    CORE::CString text = item.itemId + "\n" + item.title + "\n" + item.version + "\n" + item.listingType + "\n" + item.revision + "\n" +
                         CORE::ToString( item.changedAtMs ) + "\n" + CORE::ToString( item.acquiredAtMs ) + "\n" + CORE::ToString( item.downloadable ) + "\n";
    for ( size_t f=0; f<formats.size(); ++f )
        text += formats[ f ] + "\n";
    // what a store plugin knows of its entries that changes with the item (the engine versions of a build, ...)
    text += item.storeAttributes.GetAttributeValue( EntryChangeHintAttribute ).AsString() + "\n";
    return Sha256OfText( text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetItemRevision::GetEffectiveRevision( const CAssetStoreItem& item )
{GUCEF_TRACE;

    if ( item.changedAtMs > 0 )
        return MakeTagged( TimestampBasis, CORE::ToString( item.changedAtMs ) );
    if ( !item.version.IsNULLOrEmpty() )
        return MakeTagged( VersionBasis, item.version );
    if ( !item.revision.IsNULLOrEmpty() )
        return MakeTagged( RevisionBasis, item.revision );
    const CORE::CString formats = ComputeFormatFingerprint( item );
    if ( !formats.IsNULLOrEmpty() )
        return MakeTagged( FormatBasis, formats );
    if ( !item.itemId.IsNULLOrEmpty() )
        return MakeTagged( ListingBasis, ComputeListingFingerprint( item ) );
    return MakeTagged( UnknownBasis, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

CAssetItemRevision::TComparison
CAssetItemRevision::Compare( const CORE::CString& stored, const CORE::CString& listed )
{GUCEF_TRACE;

    const CORE::CString storedBasis = GetBasis( stored );
    const CORE::CString listedBasis = GetBasis( listed );
    if ( storedBasis.IsNULLOrEmpty() || listedBasis.IsNULLOrEmpty() || UnknownBasis == storedBasis || UnknownBasis == listedBasis )
        return COMPARISON_UNKNOWN;
    if ( storedBasis != listedBasis )
        return COMPARISON_OTHER_BASIS;
    if ( TimestampBasis == storedBasis )
        return GetChangedMs( listed ) > GetChangedMs( stored ) ? COMPARISON_STALE : COMPARISON_SAME;
    if ( GetValue( stored ) == GetValue( listed ) )
        return COMPARISON_SAME;
    return ListingBasis == storedBasis ? COMPARISON_CHECK_FURTHER : COMPARISON_STALE;
}

/*-------------------------------------------------------------------------*/

const char*
CAssetItemRevision::ComparisonToString( TComparison comparison )
{GUCEF_TRACE;

    switch ( comparison )
    {
        case COMPARISON_SAME:           return "same";
        case COMPARISON_STALE:          return "stale";
        case COMPARISON_CHECK_FURTHER:  return "check-further";
        case COMPARISON_OTHER_BASIS:    return "other-basis";
        default:                        return "unknown";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
