/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#include <string>
#include <algorithm>

#include "AncestryBackup_CJsonShape.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 MaxArrayElementsExamined = 5;
static const UInt32 MaxSampleLength = 120;

/*-------------------------------------------------------------------------*/

CORE::CString
CJsonShape::GetTypeName( const CORE::CVariant& value )
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

bool
CJsonShape::IsPaginationKey( const CORE::CString& key )
{GUCEF_TRACE;

    static const char* const hints[] = { "page", "pages", "pagenumber", "pageindex", "pagesize", "perpage", "per_page", "offset", "limit", "start", "skip",
                                         "count", "total", "totalcount", "totalresults", "totalpages", "next", "nextpage", "nexturl", "nextlink",
                                         "previous", "prev", "cursor", "nextcursor", "nexttoken", "continuation", "continuationtoken", "hasmore",
                                         "haspages", "lastpage", "size", "pagetoken", "more", "after", "before", GUCEF_NULL };
    std::string lower = key.STL_String().c_str();
    for ( size_t i=0; i<lower.size(); ++i )
        lower[ i ] = (char) tolower( lower[ i ] );

    for ( UInt32 i=0; hints[ i ] != GUCEF_NULL; ++i )
        if ( lower == hints[ i ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CJsonShape::AddLine( const CORE::CString& line, UInt32 maxLines, SJsonShape& shape )
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

static bool
LooksLikeUrl( const std::string& text )
{
    return text.compare( 0, 7, "http://" ) == 0 || text.compare( 0, 8, "https://" ) == 0 ||
           ( text.size() > 1 && text[ 0 ] == '/' && text[ 1 ] != '/' && text.find( ' ' ) == std::string::npos );
}

/*-------------------------------------------------------------------------*/

void
CJsonShape::Walk( const CORE::CDataNode& node, const CORE::CString& path, UInt32 depth, UInt32 maxDepth, UInt32 maxLines, SJsonShape& shape )
{GUCEF_TRACE;

    // fields that are scalars
    for ( UInt32 i=0; i<node.GetAttCount(); ++i )
    {
        const CORE::CDataNode::TKeyValuePair* att = node.GetAttribute( i );
        if ( GUCEF_NULL == att )
            continue;

        const CORE::CString name = att->first;
        const CORE::CString fieldPath = path.IsNULLOrEmpty() ? name : path + "." + name;
        CORE::CString line = fieldPath + ": " + GetTypeName( att->second );

        const CORE::CString typeName = GetTypeName( att->second );
        std::string sample = att->second.AsString().STL_String().c_str();
        if ( sample.size() > MaxSampleLength )
            sample = sample.substr( 0, MaxSampleLength ) + "...";

        const bool isUrl = typeName == "string" && LooksLikeUrl( sample );
        if ( IsPaginationKey( name ) || isUrl )
        {
            line += " = " + CORE::CString( sample.c_str() );
            if ( isUrl )
                shape.urlFields.push_back( fieldPath + " = " + CORE::CString( sample.c_str() ) );
            if ( IsPaginationKey( name ) )
                shape.paginationHints.push_back( fieldPath + " = " + CORE::CString( sample.c_str() ) );
        }
        AddLine( line, maxLines, shape );
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
            AddLine( childPath + ": array[" + CORE::UInt32ToString( count ) + "]", maxLines, shape );

            if ( depth + 1 < maxDepth )
            {
                UInt32 examined = 0;
                for ( CORE::CDataNode::const_iterator n = child.ConstBegin(); n != child.ConstEnd() && examined < MaxArrayElementsExamined; ++n, ++examined )
                {
                    const CORE::CDataNode& element = *(*n);
                    const CORE::CString elementPath = childPath + "[]";
                    if ( element.GetAttCount() == 0 && !element.HasChildren() )
                        AddLine( elementPath + ": " + GetTypeName( element.GetValue() ), maxLines, shape );
                    else
                        Walk( element, elementPath, depth + 1, maxDepth, maxLines, shape );
                }
            }
        }
        else
        {
            AddLine( childPath + ": " + ( child.GetNodeType() == GUCEF_DATATYPE_OBJECT ? CORE::CString( "object" ) : GetTypeName( child.GetValue() ) ), maxLines, shape );
            if ( depth + 1 < maxDepth )
                Walk( child, childPath, depth + 1, maxDepth, maxLines, shape );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CJsonShape::Describe( const CORE::CDataNode& root, SJsonShape& shape, UInt32 maxDepth, UInt32 maxLines )
{GUCEF_TRACE;

    shape = SJsonShape();
    if ( root.GetNodeType() == GUCEF_DATATYPE_ARRAY )
    {
        UInt32 count = 0;
        for ( CORE::CDataNode::const_iterator n = root.ConstBegin(); n != root.ConstEnd(); ++n )
            ++count;
        AddLine( "(root): array[" + CORE::UInt32ToString( count ) + "]", maxLines, shape );
        UInt32 examined = 0;
        for ( CORE::CDataNode::const_iterator n = root.ConstBegin(); n != root.ConstEnd() && examined < MaxArrayElementsExamined; ++n, ++examined )
            Walk( *(*n), "[]", 1, maxDepth, maxLines, shape );
    }
    else
        Walk( root, CORE::CString::Empty, 0, maxDepth, maxLines, shape );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
