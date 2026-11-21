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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */

#include "gucefASSETSTORE_CPackStorageLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

CORE::CString
Normalize( const CORE::CString& path )
{
    CORE::CString normalized = path.ReplaceChar( '\\', '/' );
    while ( ByteLengthOf( normalized ) > 0 && '/' == normalized.C_String()[ 0 ] )
        normalized = normalized.CutChars( 1, true );
    while ( ByteLengthOf( normalized ) > 0 && '/' == normalized.C_String()[ ByteLengthOf( normalized )-1 ] )
        normalized = normalized.CutChars( 1, false );
    return normalized;
}

inline bool
IsAlphaNumeric( char c )
{
    return ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
}

inline bool
IsLowerHex( char c )
{
    return ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPackStorageLayout::OriginalArea = "original";
const CORE::CString CPackStorageLayout::IntermediateArea = "intermediate";
const CORE::CString CPackStorageLayout::CookedArea = "gucef-ogre3d";
const CORE::CString CPackStorageLayout::StagingArea = "staging";
const CORE::CString CPackStorageLayout::GenerationsArea = "generations";
const CORE::CString CPackStorageLayout::RootPathMarker = "layout2:";

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::Join( const CORE::CString& left, const CORE::CString& right )
{GUCEF_TRACE;

    const CORE::CString a = Normalize( left );
    const CORE::CString b = Normalize( right );
    if ( a.IsNULLOrEmpty() )
        return b;
    if ( b.IsNULLOrEmpty() )
        return a;
    return a + "/" + b;
}

/*-------------------------------------------------------------------------*/

bool
CPackStorageLayout::IsAreaName( const CORE::CString& name )
{GUCEF_TRACE;

    return name == OriginalArea || name == IntermediateArea || name == CookedArea;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::MakePackFolderName( const CORE::CString& sourceName )
{GUCEF_TRACE;

    // the last part of a path, and no leading dots or blanks
    CORE::CString name = Normalize( sourceName );
    const UInt32 nameLength = ByteLengthOf( name );
    const char* chars = name.C_String();
    UInt32 start = 0;
    for ( UInt32 i=0; i<nameLength; ++i )
        if ( '/' == chars[ i ] )
            start = i + 1;
    while ( start < nameLength && ( '.' == chars[ start ] || ' ' == chars[ start ] || (unsigned char) chars[ start ] < 0x20 ) )
        ++start;
    UInt32 end = nameLength;

    // an extension is up to 16 letters and digits after the last dot, of a name that has more than the extension
    UInt32 dot = end;
    for ( UInt32 i=end; i>start; --i )
    {
        if ( '.' == chars[ i-1 ] )
        {
            dot = i - 1;
            break;
        }
    }
    CORE::CString extension;
    if ( dot < end && dot > start && end - dot - 1 >= 1 && end - dot - 1 <= 16 )
    {
        bool plain = true;
        for ( UInt32 i=dot+1; i<end; ++i )
            plain = plain && IsAlphaNumeric( chars[ i ] );
        if ( plain )
            extension = CORE::CString( chars + dot, end - dot );
    }
    const UInt32 stemEnd = extension.IsNULLOrEmpty() ? end : dot;
    const CORE::CString stem = start < stemEnd ? CORE::CString( chars + start, stemEnd - start ) : CORE::CString::Empty;

    const UInt32 extensionBytes = ByteLengthOf( extension );
    const CORE::CString safeStem = CAssetStoreSafeNames::MakeSafePart( stem.IsNULLOrEmpty() ? CORE::CString( "pack" ) : stem, MaxFolderNameBytes - extensionBytes );
    return safeStem + extension;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::MakeShortId( const CORE::CString& packId )
{GUCEF_TRACE;

    const CORE::CString lower = packId.Lowercase();
    const char* chars = lower.C_String();
    const UInt32 length = ByteLengthOf( lower );
    CORE::CString hex;
    for ( UInt32 i=0; i<length && ByteLengthOf( hex ) < 8; ++i )
    {
        if ( IsLowerHex( chars[ i ] ) )
            hex += CORE::CString( chars + i, 1 );
    }
    if ( ByteLengthOf( hex ) < 8 )
        return CAssetStoreSafeNames::MakeSafePart( packId, 40 );
    return hex;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::ChoosePackFolderName( const CORE::CString& cleanName, const CORE::CString& packId, const CORE::CString::StringSet& takenLowercase )
{GUCEF_TRACE;

    if ( takenLowercase.end() == takenLowercase.find( cleanName.Lowercase() ) )
        return cleanName;

    // the id follows the whole name, extension included
    const CORE::CString withShortId = cleanName + " [" + MakeShortId( packId ) + "]";
    if ( takenLowercase.end() == takenLowercase.find( withShortId.Lowercase() ) )
        return withShortId;
    const CORE::CString withId = cleanName + " [" + CAssetStoreSafeNames::MakeSafePart( packId, 60 ) + "]";
    if ( takenLowercase.end() == takenLowercase.find( withId.Lowercase() ) )
        return withId;

    // every form is taken (folders on the disk the catalogue does not know): a number after the short id
    for ( UInt32 n=2; n<10000; ++n )
    {
        const CORE::CString numbered = cleanName + " [" + MakeShortId( packId ) + "-" + CORE::ToString( n ) + "]";
        if ( takenLowercase.end() == takenLowercase.find( numbered.Lowercase() ) )
            return numbered;
    }
    return withId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::MakeRootPath( const CORE::CString& packFolder )
{GUCEF_TRACE;

    return RootPathMarker + packFolder;
}

/*-------------------------------------------------------------------------*/

CPackStorageLayout::CPackStorageLayout( void )
    : m_version( LayoutFolders )
    , m_packFolder()
    , m_legacyRoot()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackStorageLayout::CPackStorageLayout( const CORE::CString& storedRootPath )
    : m_version( LayoutFolders )
    , m_packFolder()
    , m_legacyRoot()
{GUCEF_TRACE;

    if ( 0 == storedRootPath.HasSubstr( RootPathMarker, true ) && ByteLengthOf( storedRootPath ) > ByteLengthOf( RootPathMarker ) )
    {
        m_version = LayoutAreas;
        m_packFolder = storedRootPath.CutChars( ByteLengthOf( RootPathMarker ), true );
    }
    else
        m_legacyRoot = Normalize( storedRootPath );
}

/*-------------------------------------------------------------------------*/

UInt32
CPackStorageLayout::GetVersion( void ) const
{GUCEF_TRACE;

    return m_version;
}

/*-------------------------------------------------------------------------*/

bool
CPackStorageLayout::IsAreaLayout( void ) const
{GUCEF_TRACE;

    return LayoutAreas == m_version;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPackStorageLayout::GetPackFolder( void ) const
{GUCEF_TRACE;

    return m_packFolder;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPackStorageLayout::GetLegacyRoot( void ) const
{GUCEF_TRACE;

    return m_legacyRoot;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetStoredRootPath( void ) const
{GUCEF_TRACE;

    return IsAreaLayout() ? MakeRootPath( m_packFolder ) : m_legacyRoot;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::Resolve( const CORE::CString& locationRoot, const CORE::CString& logicalPath ) const
{GUCEF_TRACE;

    if ( !IsAreaLayout() )
        return Join( Join( locationRoot, m_legacyRoot ), logicalPath );

    // the first part names the area; paths of the earlier layout that were moved along ("converted/...") live in the intermediate area
    const CORE::CString path = Normalize( logicalPath );
    const char* chars = path.C_String();
    const UInt32 length = ByteLengthOf( path );
    UInt32 slash = length;
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( '/' == chars[ i ] )
        {
            slash = i;
            break;
        }
    }
    const CORE::CString first = CORE::CString( chars, slash );
    const CORE::CString rest = slash < length ? path.CutChars( slash + 1, true ) : CORE::CString::Empty;
    if ( IsAreaName( first ) )
        return Join( Join( Join( locationRoot, first ), m_packFolder ), rest );
    return Join( Join( Join( locationRoot, IntermediateArea ), m_packFolder ), path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetOriginalFolder( const CORE::CString& locationRoot ) const
{GUCEF_TRACE;

    return Resolve( locationRoot, OriginalArea );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetIntermediateFolder( const CORE::CString& locationRoot, const CORE::CString& profile ) const
{GUCEF_TRACE;

    return Resolve( locationRoot, GetIntermediatePrefix( profile ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetCookedFolder( const CORE::CString& locationRoot, const CORE::CString& profile ) const
{GUCEF_TRACE;

    return Resolve( locationRoot, GetCookedPrefix( profile ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetGenerationsFolder( const CORE::CString& locationRoot ) const
{GUCEF_TRACE;

    if ( !IsAreaLayout() )
        return CORE::CString::Empty;
    return Join( Join( locationRoot, GenerationsArea ), m_packFolder );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetOriginalPrefix( void ) const
{GUCEF_TRACE;

    return OriginalArea;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetIntermediatePrefix( const CORE::CString& profile ) const
{GUCEF_TRACE;

    return IsAreaLayout() ? IntermediateArea : Join( "converted", profile );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetCookedPrefix( const CORE::CString& profile ) const
{GUCEF_TRACE;

    return IsAreaLayout() ? CookedArea : Join( "converted", profile );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetBrowseFolder( const CORE::CString& locationRoot ) const
{GUCEF_TRACE;

    return IsAreaLayout() ? GetOriginalFolder( locationRoot ) : Join( locationRoot, m_legacyRoot );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackStorageLayout::GetPackIndexPath( const CORE::CString& locationRoot, const CORE::CString& profile ) const
{GUCEF_TRACE;

    return Join( GetIntermediateFolder( locationRoot, profile ), "pack-index.json" );
}

/*-------------------------------------------------------------------------*/

void
CPackStorageLayout::GetOwnedFolders( const CORE::CString& locationRoot, CORE::CString::StringVector& folders ) const
{GUCEF_TRACE;

    folders.clear();
    if ( IsAreaLayout() )
    {
        folders.push_back( Join( Join( locationRoot, OriginalArea ), m_packFolder ) );
        folders.push_back( Join( Join( locationRoot, IntermediateArea ), m_packFolder ) );
        folders.push_back( Join( Join( locationRoot, CookedArea ), m_packFolder ) );
        folders.push_back( GetGenerationsFolder( locationRoot ) );
    }
    else
    if ( 0 == m_legacyRoot.HasSubstr( "packs/", true ) )
        folders.push_back( Join( locationRoot, m_legacyRoot ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
