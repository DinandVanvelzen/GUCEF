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

#include "gucefASSETSTORE_CAssetStoreSafeNames.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

namespace {

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CAssetStoreSafeNames::IsReservedDeviceName( const CORE::CString& part )
{GUCEF_TRACE;

    const CORE::CString base = part.SubstrToChar( '.', true ).Uppercase();
    if ( base == "CON" || base == "PRN" || base == "AUX" || base == "NUL" )
        return true;
    if ( 4 == base.Length() && ( base.SubstrToIndex( 3, true ) == "COM" || base.SubstrToIndex( 3, true ) == "LPT" ) )
    {
        const char last = base.C_String()[ 3 ];
        return last >= '0' && last <= '9';
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreSafeNames::NormalizeSeparators( const CORE::CString& name )
{GUCEF_TRACE;

    return name.ReplaceChar( '\\', '/' );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreSafeNames::IsSafeRelativeName( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString reason;
    return IsSafeRelativeName( name, reason );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreSafeNames::IsSafeRelativeName( const CORE::CString& name, CORE::CString& reason )
{GUCEF_TRACE;

    const UInt32 length = ByteLengthOf( name );
    if ( 0 == length )
    {
        reason = "the name is empty";
        return false;
    }
    if ( length > MaxNameLength )
    {
        reason = "the name is longer than " + CORE::ToString( MaxNameLength ) + " bytes";
        return false;
    }
    const CORE::CString normalized = NormalizeSeparators( name );
    const char* text = normalized.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        const unsigned char c = (unsigned char) text[ i ];
        if ( c < 0x20 || c == 0x7F )
        {
            reason = "the name holds a control character";
            return false;
        }
        if ( c == ':' )
        {
            reason = "the name holds a drive letter or a colon";
            return false;
        }
        if ( c == '<' || c == '>' || c == '"' || c == '|' || c == '?' || c == '*' )
        {
            reason = "the name holds a character that is not allowed in file names";
            return false;
        }
    }
    if ( '/' == text[ 0 ] )
    {
        reason = "the name is an absolute path";
        return false;
    }

    const CORE::CString::StringVector parts = normalized.ParseElements( '/', true );
    for ( size_t p=0; p<parts.size(); ++p )
    {
        const CORE::CString& part = parts[ p ];
        const UInt32 partLength = ByteLengthOf( part );
        if ( 0 == partLength )
        {
            reason = "the name has an empty part";
            return false;
        }
        if ( part == "." || part == ".." )
        {
            reason = "the name refers to a parent or the current folder";
            return false;
        }
        if ( partLength > MaxPartLength )
        {
            reason = "a part of the name is longer than " + CORE::ToString( MaxPartLength ) + " bytes";
            return false;
        }
        const char last = part.C_String()[ partLength-1 ];
        if ( '.' == last || ' ' == last )
        {
            reason = "a part of the name ends in a dot or a space";
            return false;
        }
        if ( IsReservedDeviceName( part ) )
        {
            reason = "a part of the name is a reserved device name";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreSafeNames::MakeSafePart( const CORE::CString& text, UInt32 maxLength )
{GUCEF_TRACE;

    GUCEF::vector< char > bytes;
    const char* chars = text.C_String();
    const UInt32 length = ByteLengthOf( text );
    bool lastWasBlank = true;
    for ( UInt32 i=0; i<length; ++i )
    {
        unsigned char c = (unsigned char) chars[ i ];
        if ( c < 0x20 || c == 0x7F || c == '/' || c == '\\' || c == ':' || c == '<' || c == '>' || c == '"' || c == '|' || c == '?' || c == '*' )
            c = '_';
        if ( ' ' == c )
        {
            if ( lastWasBlank )
                continue;
            lastWasBlank = true;
        }
        else
            lastWasBlank = false;
        bytes.push_back( (char) c );
    }

    // at most maxLength bytes, not in the middle of a character
    if ( bytes.size() > maxLength )
    {
        size_t cut = maxLength;
        while ( cut > 0 && 0x80 == ( (unsigned char) bytes[ cut ] & 0xC0 ) )
            --cut;
        bytes.resize( cut );
    }
    while ( !bytes.empty() && ( '.' == bytes.back() || ' ' == bytes.back() ) )
        bytes.pop_back();

    CORE::CString result = bytes.empty() ? CORE::CString() : CORE::CString( &bytes[ 0 ], (UInt32) bytes.size() );
    if ( result.IsNULLOrEmpty() )
        result = "_";
    if ( IsReservedDeviceName( result ) )
        result = "_" + result;
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
