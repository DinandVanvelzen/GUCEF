/*
 *  AssetConverterLib: the common naming conventions of asset packs as data: prefix tokens and domain tokens
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

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CPackNamingConventions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SPair
{
    const char* token;
    const char* value;
};

/** The first tokens of names: what the file is */
const char* const PrefixTokens[] = { "sm", "sk", "skm", "st", "t", "tex", "txt", "m", "mi", "mat", "ma", "a", "anim", "am", "ps", "p", "pf", "bp", "wbp", "mesh", "model", "mdl", "spr", "snd", "sfx", "mus" };

/** The domain tokens and the sub-group they give */
const SPair DomainTokens[] =
{
    { "bld", "buildings" }, { "bldg", "buildings" }, { "building", "buildings" }, { "buildings", "buildings" }, { "house", "buildings" }, { "houses", "buildings" },
    { "prop", "props" }, { "props", "props" }, { "prp", "props" },
    { "veh", "vehicles" }, { "vehicle", "vehicles" }, { "vehicles", "vehicles" }, { "car", "vehicles" }, { "truck", "vehicles" }, { "boat", "vehicles" },
    { "sign", "signs" }, { "signs", "signs" }, { "signage", "signs" },
    { "env", "environment" }, { "environment", "environment" }, { "terrain", "environment" }, { "ground", "environment" },
    { "veg", "vegetation" }, { "vegetation", "vegetation" }, { "tree", "vegetation" }, { "trees", "vegetation" }, { "plant", "vegetation" }, { "plants", "vegetation" },
    { "foliage", "vegetation" }, { "grass", "vegetation" }, { "bush", "vegetation" },
    { "rock", "rocks" }, { "rocks", "rocks" }, { "stone", "rocks" },
    { "chr", "characters" }, { "char", "characters" }, { "character", "characters" }, { "characters", "characters" }, { "npc", "characters" },
    { "fx", "effects" }, { "vfx", "effects" }, { "effect", "effects" }, { "effects", "effects" }, { "particle", "effects" }, { "particles", "effects" },
    { "wpn", "weapons" }, { "weapon", "weapons" }, { "weapons", "weapons" },
    { "furn", "furniture" }, { "furniture", "furniture" },
    { "ui", "ui" }, { "gui", "ui" }, { "hud", "ui" }, { "icon", "ui" }, { "icons", "ui" },
    { "wall", "walls" }, { "walls", "walls" }, { "floor", "floors" }, { "floors", "floors" }, { "roof", "roofs" }, { "roofs", "roofs" },
    { "door", "doors" }, { "doors", "doors" }, { "window", "windows" }, { "windows", "windows" },
    { "light", "lights" }, { "lights", "lights" }, { "lamp", "lights" }, { "food", "food" }, { "clothes", "clothing" }, { "clothing", "clothing" }
};

const char* const GenericSubGroups[] = { "misc", "miscellaneous", "other", "others", "common", "default", "general", "various", "extra", "extras", "generic", "shared", "unsorted", "temp", "old" };

template< typename T, size_t N > size_t CountOf( T (&)[ N ] ) { return N; }

bool
IsDigits( const CORE::CString& token )
{
    for ( UInt32 i=0; i<token.Length(); ++i )
        if ( token[ i ] < '0' || token[ i ] > '9' )
            return false;
    return token.Length() > 0;
}

}

/*-------------------------------------------------------------------------*/

void
CPackNamingConventions::SplitTokens( const CORE::CString& name, CORE::CString::StringVector& tokens, bool keepDigits )
{GUCEF_TRACE;

    tokens.clear();
    CORE::CString current;
    char previous = '\0';
    const UInt32 length = name.Length();
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = name[ i ];
        const bool upper = c >= 'A' && c <= 'Z';
        const bool lower = c >= 'a' && c <= 'z';
        const bool digit = c >= '0' && c <= '9';
        if ( !upper && !lower && !digit )
        {
            if ( !current.IsNULLOrEmpty() )
                tokens.push_back( current );
            current.Clear();
            continue;
        }
        if ( !current.IsNULLOrEmpty() )
        {
            const char last = previous;
            const bool lastLower = last >= 'a' && last <= 'z';
            const bool lastUpper = last >= 'A' && last <= 'Z';
            const bool lastDigit = last >= '0' && last <= '9';
            const char next = i+1 < length ? name[ i+1 ] : '\0';
            const bool nextLower = next >= 'a' && next <= 'z';
            bool split = false;
            if ( upper && lastLower )
                split = true;                           // fooBar
            else
            if ( upper && lastUpper && nextLower )
                split = true;                           // FOOBar
            else
            if ( digit != lastDigit && ( digit || lastLower || lastUpper ) )
                split = true;                           // foo12, 12foo
            if ( split )
            {
                tokens.push_back( current );
                current.Clear();
            }
        }
        current += (char) ( upper ? c + ( 'a' - 'A' ) : c );
        previous = c;
    }
    if ( !current.IsNULLOrEmpty() )
        tokens.push_back( current );

    // digit runs say nothing of what a file is: they are kept for a title only
    if ( keepDigits )
        return;
    CORE::CString::StringVector kept;
    for ( size_t t=0; t<tokens.size(); ++t )
        if ( !IsDigits( tokens[ t ] ) )
            kept.push_back( tokens[ t ] );
    tokens = kept;
}

/*-------------------------------------------------------------------------*/

bool
CPackNamingConventions::IsPrefixToken( const CORE::CString& token )
{GUCEF_TRACE;

    for ( size_t i=0; i<CountOf( PrefixTokens ); ++i )
        if ( token == PrefixTokens[ i ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackNamingConventions::GetSubGroupOfToken( const CORE::CString& token )
{GUCEF_TRACE;

    for ( size_t i=0; i<CountOf( DomainTokens ); ++i )
        if ( token == DomainTokens[ i ].token )
            return DomainTokens[ i ].value;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackNamingConventions::GetStoreKind( const CORE::CString& prefixToken, EAssetKind kind )
{GUCEF_TRACE;

    if ( ASSETKIND_MODEL == kind )
    {
        if ( "sm" == prefixToken || "mesh" == prefixToken )
            return "static-model";
        if ( "sk" == prefixToken || "skm" == prefixToken )
            return "skeletal-model";
    }
    if ( ( ASSETKIND_TEXTURE == kind || ASSETKIND_IMAGE == kind ) && ( "t" == prefixToken || "tex" == prefixToken || "txt" == prefixToken ) )
        return "texture";
    if ( ASSETKIND_MATERIAL == kind && ( "m" == prefixToken || "mi" == prefixToken || "mat" == prefixToken || "ma" == prefixToken ) )
        return "material";
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/
CORE::CString
CPackNamingConventions::GetGroupOfPrefix( const CORE::CString& prefixToken, EAssetKind kind )
{GUCEF_TRACE;

    if ( ASSETKIND_DATA != kind && ASSETKIND_PACKAGE != kind && ASSETKIND_OTHER != kind )
        return CORE::CString::Empty;
    static const SPair prefixGroups[] =
    {
        { "sm", "static-models" }, { "mesh", "static-models" }, { "sk", "characters" }, { "skm", "characters" },
        { "t", "textures" }, { "tex", "textures" }, { "txt", "textures" },
        { "m", "materials" }, { "mi", "materials" }, { "mat", "materials" }, { "ma", "materials" },
        { "a", "animations" }, { "anim", "animations" }, { "am", "animations" },
        { "ps", "visual-effects" }
    };
    for ( size_t i=0; i<CountOf( prefixGroups ); ++i )
        if ( prefixToken == prefixGroups[ i ].token )
            return prefixGroups[ i ].value;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/


bool
CPackNamingConventions::IsGenericSubGroup( const CORE::CString& subGroupId )
{GUCEF_TRACE;

    for ( size_t i=0; i<CountOf( GenericSubGroups ); ++i )
        if ( subGroupId == GenericSubGroups[ i ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackNamingConventions::MakeTitle( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString::StringVector tokens;
    SplitTokens( CVfsPaths::StemOf( name ), tokens, true );
    size_t first = 0;
    if ( tokens.size() > 1 && IsPrefixToken( tokens[ 0 ] ) )
        first = 1;
    CORE::CString title;
    for ( size_t t=first; t<tokens.size(); ++t )
    {
        // a LOD suffix (lod0, lod1) is no part of the title: the split leaves "lod" before the digits
        if ( "lod" == tokens[ t ] && t+2 == tokens.size() && IsDigits( tokens[ t+1 ] ) )
            break;
        if ( !title.IsNULLOrEmpty() )
            title += ' ';
        title += tokens[ t ];
    }
    if ( title.IsNULLOrEmpty() )
        return CVfsPaths::StemOf( name );
    CORE::CString head;
    head += (char) ( title[ 0 ] >= 'a' && title[ 0 ] <= 'z' ? title[ 0 ] - ( 'a' - 'A' ) : title[ 0 ] );
    title = head + title.SubstrFromRange( 1, title.Length() );
    return title;
}

/*-------------------------------------------------------------------------*/

UInt32
CPackNamingConventions::GetDomainTokenCount( void )
{GUCEF_TRACE;

    return (UInt32) CountOf( DomainTokens );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
