/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#include <algorithm>
#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#include "gucefWEB_CWebBodyShape.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 MaxArrayElementsExamined = 5;
static const UInt32 MaxSampleLength = 120;

/** The built-in rule of secret names, for callers that give no redactor of their own */
static const CORE::CRedactor g_builtInSecretNames( CORE::REDACT_NAME_WORDS );

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct CWebBodyShape::SWalkContext
{
    UInt32 maxDepth;
    UInt32 maxLines;
    bool withSampleValues;
    const CORE::CRedactor* secretNames;
};

/*-------------------------------------------------------------------------*/

static bool
LooksLikeUrl( const CORE::CString& text )
{
    const char* t = text.C_String();
    const UInt32 len = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    if ( len >= 7 && 0 == strncmp( t, "http://", 7 ) )
        return true;
    if ( len >= 8 && 0 == strncmp( t, "https://", 8 ) )
        return true;
    return len > 1 && t[ 0 ] == '/' && t[ 1 ] != '/' && GUCEF_NULL == memchr( t, ' ', len );
}

/*-------------------------------------------------------------------------*/

/** What a URL value looks like as an endpoint: scheme://host/path-template, or the path template of a relative one */
static CORE::CString
UrlTemplateOf( const CORE::CString& url )
{
    CUrlPathTemplate parsed;
    if ( CUrlPathTemplate::Parse( url, parsed ) )
        return parsed.scheme + "://" + parsed.host + parsed.pathTemplate;
    CORE::CString path = url;
    Int32 query = path.HasChar( '?' );
    if ( query >= 0 )
        path = path.SubstrToIndex( (UInt32) query, true );
    return CUrlPathTemplate::TemplatePath( path );
}

/*-------------------------------------------------------------------------*/

static void
AddUnique( CORE::CString::StringVector& list, const CORE::CString& entry )
{
    if ( std::find( list.begin(), list.end(), entry ) == list.end() )
        list.push_back( entry );
}

static bool
IsNullType( const CORE::CString& type )
{
    return type == "null";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebBodyShape::CWebBodyShape( void )
    : lines()
    , paginationHints()
    , urlFields()
    , truncated( false )
    , mergedCount( 0 )
    , m_keyCounts()
    , m_keyOrder()
    , m_keyTexts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::Clear( void )
{GUCEF_TRACE;

    lines.clear();
    paginationHints.clear();
    urlFields.clear();
    truncated = false;
    mergedCount = 0;
    m_keyCounts.clear();
    m_keyOrder.clear();
    m_keyTexts.clear();
}

/*-------------------------------------------------------------------------*/

bool
CWebBodyShape::IsEmpty( void ) const
{GUCEF_TRACE;

    return lines.empty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebBodyShape::GetTypeName( const CORE::CVariant& value )
{GUCEF_TRACE;

    const UInt8 id = value.GetTypeId();
    if ( id == GUCEF_DATATYPE_ARRAY ) return "array";
    if ( id == GUCEF_DATATYPE_OBJECT ) return "object";
    if ( id == GUCEF_DATATYPE_NIL || id == GUCEF_DATATYPE_NULL ) return "null";
    if ( id == GUCEF_DATATYPE_BOOLEAN || ( id >= GUCEF_DATATYPE_BOOLEAN_INT32 && id <= GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING ) ) return "boolean";
    if ( id >= GUCEF_DATATYPE_UINT8 && id <= GUCEF_DATATYPE_BE_INT64 ) return "integer";
    if ( id >= GUCEF_DATATYPE_LE_FLOAT32 && id <= GUCEF_DATATYPE_BE_FLOAT64 ) return "number";
    if ( id == GUCEF_DATATYPE_BINARY_BLOB || id == GUCEF_DATATYPE_BINARY_BSOB ) return "binary";
    return "string";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebBodyShape::GuessTextType( const CORE::CString& text )
{GUCEF_TRACE;

    const char* t = text.C_String();
    const UInt32 len = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    if ( 0 == len )
        return "string";
    if ( text == "true" || text == "false" )
        return "boolean";
    UInt32 digits = 0, dots = 0;
    for ( UInt32 i=0; i<len; ++i )
    {
        if ( t[ i ] >= '0' && t[ i ] <= '9' )
            ++digits;
        else
        if ( t[ i ] == '.' )
            ++dots;
        else
        if ( !( i == 0 && t[ i ] == '-' ) )
            return "string";
    }
    if ( 0 == digits )
        return "string";
    return 0 == dots ? "integer" : ( 1 == dots ? "number" : "string" );
}

/*-------------------------------------------------------------------------*/

bool
CWebBodyShape::IsPaginationKey( const CORE::CString& key )
{GUCEF_TRACE;

    static const char* const hints[] = { "page", "pages", "pagenumber", "pageindex", "pagesize", "perpage", "per_page", "offset", "limit", "start", "skip",
                                         "count", "total", "totalcount", "totalresults", "totalpages", "next", "nextpage", "nexturl", "nextlink",
                                         "previous", "prev", "cursor", "nextcursor", "nexttoken", "continuation", "continuationtoken", "hasmore",
                                         "haspages", "lastpage", "size", "pagetoken", "more", "after", "before", GUCEF_NULL };
    const CORE::CString lower = key.Lowercase();
    for ( UInt32 i=0; hints[ i ] != GUCEF_NULL; ++i )
        if ( lower == hints[ i ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::AddLine( const CORE::CString& line, UInt32 maxLines, CWebBodyShape& shape )
{GUCEF_TRACE;

    if ( shape.lines.size() >= maxLines )
    {
        shape.truncated = true;
        return;
    }
    if ( std::find( shape.lines.begin(), shape.lines.end(), line ) == shape.lines.end() )
        shape.lines.push_back( line );
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::AddField( const CORE::CString& fieldPath  ,
                         const CORE::CString& name       ,
                         const CORE::CString& typeName   ,
                         const CORE::CString& value      ,
                         const SWalkContext& context     ,
                         CWebBodyShape& shape            ,
                         const CORE::CString& suffix     )
{GUCEF_TRACE;

    CORE::CString line = fieldPath + ": " + typeName;
    const bool secret = context.secretNames->IsSensitiveName( name );
    if ( !secret )
    {
        CORE::CString sample = value;
        if ( sample.Length() > MaxSampleLength )
            sample = sample.SubstrToIndex( MaxSampleLength, true ) + "...";
        const bool isUrl = typeName == "string" && LooksLikeUrl( sample );
        const bool isPaging = IsPaginationKey( name );
        if ( context.withSampleValues )
        {
            if ( isPaging || isUrl )
                line += " = " + sample;
            if ( isUrl )
                AddUnique( shape.urlFields, fieldPath + " = " + sample );
            if ( isPaging )
                AddUnique( shape.paginationHints, fieldPath + " = " + sample );
        }
        else
        {
            if ( isUrl )
                AddUnique( shape.urlFields, fieldPath + " -> " + UrlTemplateOf( value ) );
            if ( isPaging )
                AddUnique( shape.paginationHints, fieldPath );
        }
    }
    AddLine( line + suffix, context.maxLines, shape );
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::Walk( const CORE::CDataNode& node, const CORE::CString& path, UInt32 depth, const SWalkContext& context, CWebBodyShape& shape )
{GUCEF_TRACE;

    // fields that are scalars
    for ( UInt32 i=0; i<node.GetAttCount(); ++i )
    {
        const CORE::CDataNode::TKeyValuePair* att = node.GetAttribute( i );
        if ( GUCEF_NULL == att )
            continue;
        const CORE::CString& name = att->first;
        const CORE::CString fieldPath = path.IsNULLOrEmpty() ? name : path + "." + name;
        AddField( fieldPath, name, GetTypeName( att->second ), att->second.AsString(), context, shape );
    }

    // fields that are objects or arrays
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
    {
        const CORE::CDataNode& child = *(*i);
        const CORE::CString childName = child.GetName();
        const CORE::CString childPath = path.IsNULLOrEmpty() ? childName : ( childName.IsNULLOrEmpty() ? path : path + "." + childName );
        if ( child.GetNodeType() == GUCEF_DATATYPE_ARRAY )
        {
            UInt32 count = 0;
            for ( CORE::CDataNode::const_iterator n = child.ConstBegin(); n != child.ConstEnd(); ++n )
                ++count;
            AddLine( childPath + ": array[" + CORE::UInt32ToString( count ) + "]", context.maxLines, shape );
            if ( depth + 1 < context.maxDepth )
            {
                UInt32 examined = 0;
                for ( CORE::CDataNode::const_iterator n = child.ConstBegin(); n != child.ConstEnd() && examined < MaxArrayElementsExamined; ++n, ++examined )
                {
                    const CORE::CDataNode& element = *(*n);
                    const CORE::CString elementPath = childPath + "[]";
                    if ( element.GetAttCount() == 0 && !element.HasChildren() )
                        AddLine( elementPath + ": " + GetTypeName( element.GetValue() ), context.maxLines, shape );
                    else
                        Walk( element, elementPath, depth + 1, context, shape );
                }
            }
        }
        else
        if ( child.GetNodeType() == GUCEF_DATATYPE_OBJECT || child.HasChildren() || child.GetAttCount() > 0 )
        {
            AddLine( childPath + ": object", context.maxLines, shape );
            if ( depth + 1 < context.maxDepth )
                Walk( child, childPath, depth + 1, context, shape );
        }
        else
        {
            AddField( childPath, childName, GetTypeName( child.GetValue() ), child.GetValue().AsString(), context, shape );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::Describe( const CORE::CDataNode& root            ,
                         CWebBodyShape& shape                   ,
                         UInt32 maxDepth                        ,
                         UInt32 maxLines                        ,
                         bool withSampleValues                  ,
                         const CORE::CRedactor* secretNames     )
{GUCEF_TRACE;

    shape.Clear();
    SWalkContext context;
    context.maxDepth = maxDepth;
    context.maxLines = maxLines;
    context.withSampleValues = withSampleValues;
    context.secretNames = GUCEF_NULL != secretNames ? secretNames : &g_builtInSecretNames;

    if ( root.GetNodeType() == GUCEF_DATATYPE_ARRAY )
    {
        UInt32 count = 0;
        for ( CORE::CDataNode::const_iterator n = root.ConstBegin(); n != root.ConstEnd(); ++n )
            ++count;
        AddLine( "(root): array[" + CORE::UInt32ToString( count ) + "]", maxLines, shape );
        UInt32 examined = 0;
        for ( CORE::CDataNode::const_iterator n = root.ConstBegin(); n != root.ConstEnd() && examined < MaxArrayElementsExamined; ++n, ++examined )
            Walk( *(*n), "[]", 1, context, shape );
    }
    else
        Walk( root, CORE::CString::Empty, 0, context, shape );
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::DescribeFormUrlEncoded( const CORE::CString& body          ,
                                       CWebBodyShape& shape               ,
                                       UInt32 maxLines                    ,
                                       bool withSampleValues              ,
                                       const CORE::CRedactor* secretNames )
{GUCEF_TRACE;

    shape.Clear();
    SWalkContext context;
    context.maxDepth = 1;
    context.maxLines = maxLines;
    context.withSampleValues = withSampleValues;
    context.secretNames = GUCEF_NULL != secretNames ? secretNames : &g_builtInSecretNames;

    CORE::CString::StringVector pairs = body.ParseElements( '&', false );
    for ( size_t i=0; i<pairs.size(); ++i )
    {
        const CORE::CString& pair = pairs[ i ];
        Int32 eq = pair.HasChar( '=' );
        const CORE::CString name = CUrlPathTemplate::PercentDecode( eq >= 0 ? pair.SubstrToIndex( (UInt32) eq, true ) : pair );
        const CORE::CString value = eq >= 0 ? CUrlPathTemplate::PercentDecode( pair.CutChars( (UInt32) eq + 1, true ) ) : CORE::CString();
        if ( name.IsNULLOrEmpty() )
            continue;
        if ( IsCsrfLikeName( name ) )
            AddLine( name + ": csrf-like, value withheld", maxLines, shape );
        else
        if ( context.secretNames->IsSensitiveName( name ) )
            AddField( name, name, GuessTextType( value ), value, context, shape, ", value withheld" );
        else
            AddField( name, name, GuessTextType( value ), value, context, shape, ", " + CORE::UInt32ToString( value.IsNULLOrEmpty() ? 0 : value.ByteSize() - 1 ) + " bytes" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CWebBodyShape::IsCsrfLikeName( const CORE::CString& name )
{GUCEF_TRACE;

    static const char* const parts[] = { "csrf", "xsrf", "authenticity", "nonce", "requestverification", GUCEF_NULL };
    const CORE::CString lower = name.Lowercase();
    for ( UInt32 i=0; parts[ i ] != GUCEF_NULL; ++i )
        if ( GUCEF_NULL != strstr( lower.C_String(), parts[ i ] ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** The position of the first occurrence of a needle in a block of bytes, or len when there is none */
static UInt32
FindBytes( const char* block, UInt32 len, UInt32 from, const char* needle, UInt32 needleLen )
{
    if ( 0 == needleLen || len < needleLen )
        return len;
    for ( UInt32 i=from; i+needleLen<=len; ++i )
        if ( block[ i ] == needle[ 0 ] && 0 == memcmp( block + i, needle, needleLen ) )
            return i;
    return len;
}

/*-------------------------------------------------------------------------*/

/** The value of a parameter such as name="..." in a header line, empty when there is none */
static CORE::CString
HeaderParameter( const CORE::CString& headerLine, const char* parameter )
{
    const CORE::CString lower = headerLine.Lowercase();
    const CORE::CString needle = CORE::CString( parameter ) + "=";
    const char* base = lower.C_String();
    const char* found = base;
    while ( GUCEF_NULL != ( found = strstr( found, needle.C_String() ) ) )
    {
        // "name" must not be found inside "filename"
        if ( found > base && ( found[ -1 ] == ' ' || found[ -1 ] == ';' ) )
            break;
        found += needle.Length();
    }
    if ( GUCEF_NULL == found )
        return CORE::CString::Empty;
    const char* value = headerLine.C_String() + ( found - base ) + needle.Length();
    if ( *value == '"' )
    {
        ++value;
        const char* end = strchr( value, '"' );
        return end != GUCEF_NULL ? CORE::CString( value, (UInt32) ( end - value ) ) : CORE::CString( value );
    }
    const char* end = strchr( value, ';' );
    return end != GUCEF_NULL ? CORE::CString( value, (UInt32) ( end - value ) ) : CORE::CString( value );
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::DescribeMultipart( const CORE::CString& body             ,
                                  CWebBodyShape& shape                  ,
                                  UInt32 maxLines                       ,
                                  const CORE::CRedactor* secretNames    )
{GUCEF_TRACE;

    shape.Clear();
    if ( body.IsNULLOrEmpty() )
        return;
    const CORE::CRedactor& names = GUCEF_NULL != secretNames ? *secretNames : g_builtInSecretNames;
    const char* b = body.C_String();
    const UInt32 len = body.ByteSize() - 1;

    // the first line is the delimiter: two dashes and the boundary
    UInt32 firstEnd = 0;
    while ( firstEnd < len && b[ firstEnd ] != '\r' && b[ firstEnd ] != '\n' )
        ++firstEnd;
    if ( firstEnd < 3 || b[ 0 ] != '-' || b[ 1 ] != '-' )
        return;
    const CORE::CString delimiter( b, firstEnd );

    UInt32 partCount = 0;
    UInt32 pos = firstEnd;
    while ( pos < len )
    {
        // from the end of the delimiter line to the next delimiter
        while ( pos < len && ( b[ pos ] == '\r' || b[ pos ] == '\n' ) )
            ++pos;
        if ( pos >= len )
            break;
        UInt32 next = FindBytes( b, len, pos, delimiter.C_String(), delimiter.Length() );
        const UInt32 partEnd = next;
        UInt32 headerEnd = FindBytes( b, partEnd, pos, "\r\n\r\n", 4 );
        UInt32 contentStart = headerEnd < partEnd ? headerEnd + 4 : partEnd;
        if ( headerEnd >= partEnd )
        {
            headerEnd = FindBytes( b, partEnd, pos, "\n\n", 2 );
            contentStart = headerEnd < partEnd ? headerEnd + 2 : partEnd;
        }
        const CORE::CString headers( b + pos, headerEnd - pos );
        // the line end before the delimiter belongs to the delimiter
        UInt32 contentEnd = partEnd;
        if ( contentEnd > contentStart && b[ contentEnd - 1 ] == '\n' )
            --contentEnd;
        if ( contentEnd > contentStart && b[ contentEnd - 1 ] == '\r' )
            --contentEnd;
        const UInt32 size = contentEnd > contentStart ? contentEnd - contentStart : 0;

        CORE::CString name;
        CORE::CString fileName;
        CORE::CString partType;
        CORE::CString::StringVector headerLines = headers.ParseElements( '\n', false );
        for ( size_t h=0; h<headerLines.size(); ++h )
        {
            CORE::CString line = headerLines[ h ].Trim( true ).Trim( false );
            const CORE::CString lower = line.Lowercase();
            if ( 0 == strncmp( lower.C_String(), "content-disposition:", 20 ) )
            {
                name = HeaderParameter( line, "name" );
                fileName = HeaderParameter( line, "filename" );
            }
            else
            if ( 0 == strncmp( lower.C_String(), "content-type:", 13 ) )
                partType = line.CutChars( 13, true ).Trim( true ).Trim( false ).Lowercase();
        }

        if ( !name.IsNULLOrEmpty() )
        {
            ++partCount;
            const CORE::CString sizeText = CORE::UInt32ToString( size ) + " bytes";
            if ( !fileName.IsNULLOrEmpty() )
                AddLine( name + ": file" + ( partType.IsNULLOrEmpty() ? CORE::CString::Empty : ", " + partType ) +
                         ( names.IsSensitiveName( name ) ? CORE::CString( ", value withheld" ) : ", " + sizeText ), maxLines, shape );
            else
            if ( IsCsrfLikeName( name ) )
                AddLine( name + ": csrf-like, value withheld", maxLines, shape );
            else
            if ( names.IsSensitiveName( name ) )
                AddLine( name + ": text, value withheld", maxLines, shape );
            else
                AddLine( name + ": text, " + sizeText, maxLines, shape );
        }

        if ( next >= len )
            break;
        pos = next + delimiter.Length();
        // the closing delimiter has two more dashes
        if ( pos + 1 < len && b[ pos ] == '-' && b[ pos + 1 ] == '-' )
            break;
    }
    ( void ) partCount;
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::ToKeys( const CWebBodyShape& shape, CORE::CString::StringVector& keys, TKeyTextMap& texts )
{GUCEF_TRACE;

    for ( size_t i=0; i<shape.lines.size(); ++i )
    {
        const CORE::CString& line = shape.lines[ i ];
        const char* text = line.C_String();
        const char* split = strstr( text, ": " );
        if ( GUCEF_NULL == split )
            continue;
        const CORE::CString path( text, (UInt32) ( split - text ) );
        const char* rest = split + 2;
        UInt32 typeLen = 0;
        while ( rest[ typeLen ] != '\0' && rest[ typeLen ] != ' ' && rest[ typeLen ] != '[' && rest[ typeLen ] != ',' )
            ++typeLen;
        const CORE::CString type( rest, typeLen );
        CORE::CString after( rest + typeLen );
        bool nonEmptyArray = false;
        if ( type == "array" && !after.IsNULLOrEmpty() && after.C_String()[ 0 ] == '[' )
        {
            const char* close = strchr( after.C_String(), ']' );
            if ( GUCEF_NULL != close )
            {
                const UInt32 count = (UInt32) strtoul( after.C_String() + 1, GUCEF_NULL, 10 );
                nonEmptyArray = count > 0;
                after = CORE::CString( close + 1 );
            }
        }
        const CORE::CString key = path + ": " + type;
        if ( texts.find( key ) == texts.end() )
        {
            keys.push_back( key );
            texts[ key ] = type + after;
        }
        if ( nonEmptyArray )
        {
            const CORE::CString plusKey = path + ": array[+]";
            if ( texts.find( plusKey ) == texts.end() )
            {
                keys.push_back( plusKey );
                texts[ plusKey ] = CORE::CString::Empty;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

static bool
IsNonEmptyMarker( const CORE::CString& key )
{
    return key.Length() > 10 && 0 == strcmp( key.C_String() + key.Length() - 10, ": array[+]" );
}

/*-------------------------------------------------------------------------*/

bool
CWebBodyShape::AddsToMerged( const CWebBodyShape& merged, UInt32 maxMergedLines ) const
{GUCEF_TRACE;

    if ( 0 == merged.mergedCount )
        return !lines.empty();
    CORE::CString::StringVector keys;
    TKeyTextMap texts;
    ToKeys( *this, keys, texts );
    UInt32 fieldKeys = 0;
    for ( TKeyCountMap::const_iterator i = merged.m_keyCounts.begin(); i != merged.m_keyCounts.end(); ++i )
        if ( !IsNonEmptyMarker( i->first ) )
            ++fieldKeys;
    for ( size_t i=0; i<keys.size(); ++i )
    {
        if ( merged.m_keyCounts.find( keys[ i ] ) != merged.m_keyCounts.end() )
            continue;
        if ( !IsNonEmptyMarker( keys[ i ] ) && 0 != maxMergedLines && fieldKeys >= maxMergedLines )
            continue;       // it would not fit
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::Merge( const CWebBodyShape& single, UInt32 maxMergedLines )
{GUCEF_TRACE;

    CORE::CString::StringVector keys;
    TKeyTextMap texts;
    ToKeys( single, keys, texts );

    UInt32 fieldKeys = 0;
    for ( TKeyCountMap::const_iterator i = m_keyCounts.begin(); i != m_keyCounts.end(); ++i )
        if ( !IsNonEmptyMarker( i->first ) )
            ++fieldKeys;

    for ( size_t i=0; i<keys.size(); ++i )
    {
        TKeyCountMap::iterator found = m_keyCounts.find( keys[ i ] );
        if ( found == m_keyCounts.end() )
        {
            const bool marker = IsNonEmptyMarker( keys[ i ] );
            if ( !marker && 0 != maxMergedLines && fieldKeys >= maxMergedLines )
            {
                truncated = true;
                continue;
            }
            if ( !marker )
                ++fieldKeys;
            m_keyCounts[ keys[ i ] ] = 1;
            m_keyOrder.push_back( keys[ i ] );
            m_keyTexts[ keys[ i ] ] = texts[ keys[ i ] ];
        }
        else
            ++found->second;
    }
    ++mergedCount;

    for ( size_t i=0; i<single.paginationHints.size(); ++i )
        AddUnique( paginationHints, single.paginationHints[ i ] );
    for ( size_t i=0; i<single.urlFields.size(); ++i )
        AddUnique( urlFields, single.urlFields[ i ] );
    if ( single.truncated )
        truncated = true;
    RenderMergedLines( lines );
}

/*-------------------------------------------------------------------------*/

/** The key that counts the responses in which the container of this path was there, empty for a path at the top */
static bool
FindContainerKey( const CORE::CString& path, CORE::CString& containerKey, bool& isTop )
{
    isTop = false;
    containerKey.Clear();
    const UInt32 len = path.Length();
    if ( len >= 2 && 0 == strcmp( path.C_String() + len - 2, "[]" ) )
        return false;       // an element of an array of plain values exists whenever the array has elements
    const char* dot = strrchr( path.C_String(), '.' );
    if ( GUCEF_NULL == dot )
    {
        isTop = true;
        return true;
    }
    const CORE::CString parent( path.C_String(), (UInt32) ( dot - path.C_String() ) );
    if ( parent == "[]" )
        containerKey = "(root): array[+]";
    else
    if ( parent.Length() >= 2 && 0 == strcmp( parent.C_String() + parent.Length() - 2, "[]" ) )
        containerKey = CORE::CString( parent.C_String(), parent.Length() - 2 ) + ": array[+]";
    else
        containerKey = parent + ": object";
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebBodyShape::RenderMergedLines( CORE::CString::StringVector& out ) const
{GUCEF_TRACE;

    out.clear();
    typedef GUCEF::map< CORE::CString, CORE::CString::StringVector > TPathTypeMap;
    TPathTypeMap typesOfPath;
    CORE::CString::StringVector pathOrder;
    for ( size_t i=0; i<m_keyOrder.size(); ++i )
    {
        if ( IsNonEmptyMarker( m_keyOrder[ i ] ) )
            continue;
        const char* split = strstr( m_keyOrder[ i ].C_String(), ": " );
        if ( GUCEF_NULL == split )
            continue;
        // the last ": " separates the path from the type, a type never holds one
        const char* lastSplit = strrchr( m_keyOrder[ i ].C_String(), ':' );
        const CORE::CString path( m_keyOrder[ i ].C_String(), (UInt32) ( lastSplit - m_keyOrder[ i ].C_String() ) );
        const CORE::CString type( lastSplit + 2 );
        CORE::CString::StringVector& types = typesOfPath[ path ];
        if ( types.empty() )
            pathOrder.push_back( path );
        types.push_back( type );
    }

    for ( size_t p=0; p<pathOrder.size(); ++p )
    {
        const CORE::CString& path = pathOrder[ p ];
        CORE::CString::StringVector types = typesOfPath[ path ];
        UInt32 seen = 0;
        for ( size_t t=0; t<types.size(); ++t )
            seen += m_keyCounts.find( path + ": " + types[ t ] )->second;

        CORE::CString text;
        bool hasArray = false;
        if ( types.size() == 1 )
        {
            text = m_keyTexts.find( path + ": " + types[ 0 ] )->second;
            hasArray = types[ 0 ] == "array";
        }
        else
        {
            std::sort( types.begin(), types.end() );
            std::stable_partition( types.begin(), types.end(), IsNullType );
            for ( size_t t=0; t<types.size(); ++t )
            {
                text += ( t == 0 ? "" : " | " ) + types[ t ];
                hasArray = hasArray || types[ t ] == "array";
            }
        }
        if ( hasArray && m_keyCounts.find( path + ": array[+]" ) == m_keyCounts.end() )
            text += " (always empty so far)";

        CORE::CString containerKey;
        bool isTop = false;
        if ( FindContainerKey( path, containerKey, isTop ) )
        {
            UInt32 possible = 0;
            if ( isTop )
                possible = mergedCount;
            else
            {
                TKeyCountMap::const_iterator c = m_keyCounts.find( containerKey );
                if ( c != m_keyCounts.end() )
                    possible = c->second;
            }
            if ( 0 != possible && seen < possible )
                text += " (optional)";
        }
        out.push_back( path + ": " + text );
    }
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
