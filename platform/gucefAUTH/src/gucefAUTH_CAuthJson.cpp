/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CAuthJson.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CAuthJson::DefaultMaxDepth;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CAuthJson::IsAvailable( void )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

namespace {

/** The scanner state of one open object or array */
struct SContainer
{
    bool isObject;
    bool expectKey;
    GUCEF::set< CORE::CString > keys;

    SContainer( bool object ) : isObject( object ), expectKey( object ), keys() {}
};

}

bool
CAuthJson::CheckStructure( const void* data, UInt32 size, UInt32 maxSize, UInt32 maxDepth, bool strictMembers )
{GUCEF_TRACE;

    if ( GUCEF_NULL == data || 0 == size || size > maxSize )
        return false;

    const char* text = static_cast< const char* >( data );
    GUCEF::vector< SContainer > stack;
    stack.reserve( maxDepth + 1 );

    UInt32 i = 0;
    while ( i < size )
    {
        char c = text[ i ];
        if ( '\0' == c )
            return false;

        if ( '"' == c )
        {
            // A string: find its end, a backslash escapes the next byte
            UInt32 start = i + 1;
            bool escaped = false;
            ++i;
            while ( i < size && '"' != text[ i ] )
            {
                if ( '\0' == text[ i ] )
                    return false;
                if ( '\\' == text[ i ] )
                {
                    escaped = true;
                    ++i;
                    if ( i >= size )
                        return false;
                }
                ++i;
            }
            if ( i >= size )
                return false;

            if ( strictMembers && !stack.empty() && stack.back().isObject && stack.back().expectKey )
            {
                if ( escaped )
                    return false;
                CORE::CString key = MakeString( text + start, i - start );
                if ( !stack.back().keys.insert( key ).second )
                    return false;
                stack.back().expectKey = false;
            }
            else
            if ( !stack.empty() && stack.back().isObject && stack.back().expectKey )
                stack.back().expectKey = false;
            ++i;
            continue;
        }

        if ( '{' == c || '[' == c )
        {
            if ( stack.size() >= maxDepth )
                return false;
            stack.push_back( SContainer( '{' == c ) );
        }
        else
        if ( '}' == c || ']' == c )
        {
            if ( stack.empty() || stack.back().isObject != ( '}' == c ) )
                return false;
            stack.pop_back();
        }
        else
        if ( ',' == c && !stack.empty() && stack.back().isObject )
            stack.back().expectKey = true;

        ++i;
    }
    return stack.empty();
}

/*-------------------------------------------------------------------------*/

bool
CAuthJson::Parse( const void* data           ,
                  UInt32 size                ,
                  UInt32 maxSize             ,
                  UInt32 maxDepth            ,
                  bool strictMembers         ,
                  CORE::CDataNode& document  )
{GUCEF_TRACE;

    document.Clear();
    if ( !CheckStructure( data, size, maxSize, maxDepth, strictMembers ) )
        return false;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) || codec.IsNULL() )
        return false;

    CORE::CDynamicBuffer linked;
    linked.LinkTo( data, size );
    CORE::CDynamicBufferAccess access( linked );
    if ( !codec->BuildDataTree( &document, &access ) )
    {
        document.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthJson::Parse( const CORE::CString& text    ,
                  UInt32 maxSize               ,
                  UInt32 maxDepth              ,
                  bool strictMembers           ,
                  CORE::CDataNode& document    )
{GUCEF_TRACE;

    UInt32 size = text.ByteSize();
    return Parse( text.C_String(), size > 0 ? size - 1 : 0, maxSize, maxDepth, strictMembers, document );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CAuthJson::GetDocument( const CORE::CDataNode& root )
{GUCEF_TRACE;

    return root;
}

/*-------------------------------------------------------------------------*/

bool
CAuthJson::GetString( const CORE::CDataNode& object, const CORE::CString& name, CORE::CString& value )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL == attribute || attribute->second.IsNULL() || !attribute->second.IsString() )
        return false;
    value = attribute->second.AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthJson::GetInt64( const CORE::CDataNode& object, const CORE::CString& name, Int64& value )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL == attribute )
        return false;
    const CORE::CVariant& v = attribute->second;
    if ( v.IsInteger() )
    {
        value = v.AsInt64();
        return true;
    }
    if ( v.IsFloat() )
    {
        Float64 f = v.AsFloat64();
        if ( f != f || f < -9.0e18 || f > 9.0e18 )
            return false;
        value = (Int64) f;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAuthJson::GetBool( const CORE::CDataNode& object, const CORE::CString& name, bool& value )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL == attribute )
        return false;
    UInt8 type = attribute->second.GetTypeId();
    if ( GUCEF_DATATYPE_BOOLEAN_INT32 != type && GUCEF_DATATYPE_BOOLEAN_STRING != type )
        return false;
    value = attribute->second.AsBool();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthJson::GetStringList( const CORE::CDataNode& object, const CORE::CString& name, CORE::CString::StringVector& values )
{GUCEF_TRACE;

    values.clear();
    const CORE::CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL != attribute )
    {
        if ( attribute->second.IsNULL() || !attribute->second.IsString() )
            return false;
        values.push_back( attribute->second.AsString() );
        return true;
    }

    const CORE::CDataNode* child = object.FindChild( name );
    if ( GUCEF_NULL == child || GUCEF_DATATYPE_ARRAY != child->GetNodeType() )
        return false;
    CORE::CDataNode::TVariantVector entries = child->GetChildrenValues();
    values.reserve( entries.size() );
    for ( size_t i=0; i<entries.size(); ++i )
    {
        if ( entries[ i ].IsNULL() || !entries[ i ].IsString() )
        {
            values.clear();
            return false;
        }
        values.push_back( entries[ i ].AsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAuthJson::FlattenInto( const CORE::CDataNode& object, const CORE::CString& prefix, UInt32 depth, CORE::CString::StringMap& values, UInt32 maxEntries )
{GUCEF_TRACE;

    UInt32 attributeCount = object.GetAttCount();
    for ( UInt32 i=0; i<attributeCount && values.size() < maxEntries; ++i )
    {
        const CORE::CDataNode::TKeyValuePair* attribute = object.GetAttribute( i );
        if ( GUCEF_NULL == attribute || attribute->second.IsNULL() )
            continue;
        CORE::CString key = prefix.IsNULLOrEmpty() ? attribute->first : prefix + "." + attribute->first;
        if ( attribute->second.IsString() || attribute->second.IsInteger() || attribute->second.IsFloat() ||
             GUCEF_DATATYPE_BOOLEAN_INT32 == attribute->second.GetTypeId() || GUCEF_DATATYPE_BOOLEAN_STRING == attribute->second.GetTypeId() )
            values[ key ] = attribute->second.AsString();
    }

    if ( depth >= CAuthJson::DefaultMaxDepth )
        return;
    CORE::CDataNode::const_iterator c = object.ConstBegin();
    while ( c != object.ConstEnd() && values.size() < maxEntries )
    {
        const CORE::CDataNode* child = *c;
        CORE::CString key = prefix.IsNULLOrEmpty() ? child->GetName() : prefix + "." + child->GetName();
        if ( GUCEF_DATATYPE_ARRAY == child->GetNodeType() )
        {
            // Arrays of scalars are one entry, the values joined by a space
            CORE::CString joined;
            CORE::CDataNode::TVariantVector entries = child->GetChildrenValues();
            for ( size_t e=0; e<entries.size(); ++e )
            {
                if ( entries[ e ].IsNULL() )
                    continue;
                if ( !joined.IsNULLOrEmpty() )
                    joined += " ";
                joined += entries[ e ].AsString();
            }
            if ( !joined.IsNULLOrEmpty() )
                values[ key ] = joined;
        }
        else
            FlattenInto( *child, key, depth + 1, values, maxEntries );
        ++c;
    }
}

/*-------------------------------------------------------------------------*/

void
CAuthJson::Flatten( const CORE::CDataNode& object, CORE::CString::StringMap& values, UInt32 maxEntries )
{GUCEF_TRACE;

    values.clear();
    FlattenInto( object, CORE::CString::Empty, 0, values, maxEntries );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
