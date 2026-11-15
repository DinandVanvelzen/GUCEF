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

#include "gucefASSETSTORE_CAssetToolUris.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetToolUris::Scheme = "assettools";
const CORE::CString CAssetToolUris::Prefix = "assettools://";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsUnreserved( char c )
{GUCEF_TRACE;

    return ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '.' == c || '_' == c || '~' == c;
}

Int32
HexValue( char c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return 10 + c - 'a';
    if ( c >= 'A' && c <= 'F' )
        return 10 + c - 'A';
    return -1;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::Encode( const CORE::CString& text, bool keepSlashes )
{GUCEF_TRACE;

    static const char hex[] = "0123456789ABCDEF";
    CORE::CString encoded;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        const char c = text[ i ];
        if ( IsUnreserved( c ) || ( keepSlashes && '/' == c ) )
        {
            char single[ 2 ] = { c, 0 };
            encoded += single;
        }
        else
        {
            const unsigned char byte = (unsigned char) c;
            char escape[ 4 ] = { '%', hex[ byte >> 4 ], hex[ byte & 0x0F ], 0 };
            encoded += escape;
        }
    }
    return encoded;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolUris::Decode( const CORE::CString& text, CORE::CString& decoded )
{GUCEF_TRACE;

    decoded.Clear();
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        char c = text[ i ];
        if ( '%' == c )
        {
            if ( i + 2 >= text.Length() )
                return false;
            const Int32 high = HexValue( text[ i+1 ] );
            const Int32 low = HexValue( text[ i+2 ] );
            if ( high < 0 || low < 0 )
                return false;
            c = (char) ( ( high << 4 ) | low );
            i += 2;
        }
        char single[ 2 ] = { c, 0 };
        decoded += single;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::ForPack( const CORE::CString& packId, const CORE::CString& member )
{GUCEF_TRACE;

    return Prefix + "packs/" + Encode( packId, false ) + "/" + member;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::ForAsset( const CORE::CString& packId, const CORE::CString& assetPath, const CORE::CString& member )
{GUCEF_TRACE;

    return Prefix + "packs/" + Encode( packId, false ) + "/assets/" + Encode( assetPath, true ) + "/" + member;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::ForCookProfile( const CORE::CString& packId, const CORE::CString& profile, const CORE::CString& member )
{GUCEF_TRACE;

    return Prefix + "packs/" + Encode( packId, false ) + "/cook/" + Encode( profile, false ) + "/" + member;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::ForDoc( const CORE::CString& name )
{GUCEF_TRACE;

    return Prefix + "docs/" + Encode( name, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::ForImportSettings( const CORE::CString& importer )
{GUCEF_TRACE;

    return Prefix + "docs/import-settings/" + Encode( importer, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::ForJobResult( const CORE::CString& jobId )
{GUCEF_TRACE;

    return Prefix + "jobs/" + Encode( jobId, false ) + "/result";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::ForStoreRecord( const CORE::CString& kind, const CORE::CString& id )
{GUCEF_TRACE;

    return Prefix + "store/" + kind + "/" + Encode( id, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolUris::ForStoreLicenceText( const CORE::CString& licenceId )
{GUCEF_TRACE;

    return Prefix + "store/licences/" + Encode( licenceId, false ) + "/text";
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolUris::Split( const CORE::CString& uri, CORE::CString::StringVector& segments )
{GUCEF_TRACE;

    segments.clear();
    if ( 0 != uri.HasSubstr( Prefix, true ) )
        return false;
    const CORE::CString rest = uri.CutChars( Prefix.Length(), true );
    if ( rest.IsNULLOrEmpty() || rest.HasChar( '?' ) >= 0 || rest.HasChar( '#' ) >= 0 )
        return false;
    const CORE::CString::StringVector raw = rest.ParseElements( '/', true );
    for ( size_t i=0; i<raw.size(); ++i )
    {
        CORE::CString decoded;
        if ( raw[ i ].IsNULLOrEmpty() || !Decode( raw[ i ], decoded ) || decoded.IsNULLOrEmpty() )
        {
            segments.clear();
            return false;
        }
        segments.push_back( decoded );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
