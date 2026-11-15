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

#include "gucefASSETSTORE_CAssetStrictConfigReader.h"

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

CAssetStrictConfigReader::CAssetStrictConfigReader( const CORE::CDataNode& node, const CORE::CString& context )
    : m_node( node )
    , m_context( context )
    , m_named()
    , m_problem()
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_OBJECT != node.GetNodeType() )
        m_problem = m_context + " is not an object";
}

/*-------------------------------------------------------------------------*/

void
CAssetStrictConfigReader::Allow( const CORE::CString& name )
{GUCEF_TRACE;

    m_named.insert( name );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStrictConfigReader::Has( const CORE::CString& name, const CORE::CVariant*& attribute, const CORE::CDataNode*& child )
{GUCEF_TRACE;

    m_named.insert( name );
    attribute = GUCEF_NULL;
    child = GUCEF_NULL;
    if ( !m_problem.IsNULLOrEmpty() )
        return false;
    const CORE::CDataNode::TKeyValuePair* pair = m_node.GetAttribute( name );
    if ( GUCEF_NULL != pair )
    {
        attribute = &(*pair).second;
        return true;
    }
    child = m_node.FindChild( name );
    return GUCEF_NULL != child;
}

/*-------------------------------------------------------------------------*/

void
CAssetStrictConfigReader::ReadString( const CORE::CString& name, CORE::CString& value, UInt32 maxLength )
{GUCEF_TRACE;

    const CORE::CVariant* attribute = GUCEF_NULL;
    const CORE::CDataNode* child = GUCEF_NULL;
    if ( !Has( name, attribute, child ) )
        return;
    if ( GUCEF_NULL == attribute || !attribute->IsString() )
    {
        m_problem = m_context + ": \"" + name + "\" must be a text";
        return;
    }
    const CORE::CString text = attribute->AsString();
    if ( maxLength > 0 && text.Length() > maxLength )
    {
        m_problem = m_context + ": \"" + name + "\" is longer than " + CORE::ToString( maxLength ) + " characters";
        return;
    }
    value = text;
}

/*-------------------------------------------------------------------------*/

void
CAssetStrictConfigReader::ReadChoice( const CORE::CString& name, const CORE::CString& choices, CORE::CString& value )
{GUCEF_TRACE;

    CORE::CString text = value;
    ReadString( name, text );
    if ( !m_problem.IsNULLOrEmpty() )
        return;
    const CORE::CString::StringVector list = choices.ParseElements( '|', false );
    for ( size_t i=0; i<list.size(); ++i )
    {
        if ( list[ i ] == text )
        {
            value = text;
            return;
        }
    }
    m_problem = m_context + ": \"" + name + "\" must be one of " + choices.ReplaceChar( '|', ',' );
}

/*-------------------------------------------------------------------------*/

void
CAssetStrictConfigReader::ReadBool( const CORE::CString& name, bool& value )
{GUCEF_TRACE;

    const CORE::CVariant* attribute = GUCEF_NULL;
    const CORE::CDataNode* child = GUCEF_NULL;
    if ( !Has( name, attribute, child ) )
        return;
    if ( GUCEF_NULL == attribute || !attribute->IsBoolean() )
    {
        m_problem = m_context + ": \"" + name + "\" must be true or false";
        return;
    }
    value = attribute->AsBool( value );
}

/*-------------------------------------------------------------------------*/

void
CAssetStrictConfigReader::ReadUInt64( const CORE::CString& name, UInt64 minimum, UInt64 maximum, UInt64& value )
{GUCEF_TRACE;

    const CORE::CVariant* attribute = GUCEF_NULL;
    const CORE::CDataNode* child = GUCEF_NULL;
    if ( !Has( name, attribute, child ) )
        return;
    if ( GUCEF_NULL == attribute || !attribute->IsInteger() || ( attribute->IsSignedInteger() && attribute->AsInt64( -1 ) < 0 ) )
    {
        m_problem = m_context + ": \"" + name + "\" must be a whole number of 0 or more";
        return;
    }
    const UInt64 number = attribute->AsUInt64( 0 );
    if ( number < minimum || number > maximum )
    {
        m_problem = m_context + ": \"" + name + "\" must lie in " + CORE::ToString( minimum ) + " to " + CORE::ToString( maximum );
        return;
    }
    value = number;
}

/*-------------------------------------------------------------------------*/

void
CAssetStrictConfigReader::ReadUInt32( const CORE::CString& name, UInt32 minimum, UInt32 maximum, UInt32& value )
{GUCEF_TRACE;

    UInt64 number = value;
    ReadUInt64( name, minimum, maximum, number );
    if ( m_problem.IsNULLOrEmpty() )
        value = (UInt32) number;
}

/*-------------------------------------------------------------------------*/

void
CAssetStrictConfigReader::ReadStringList( const CORE::CString& name, CORE::CString::StringVector& values, UInt32 maxItems )
{GUCEF_TRACE;

    const CORE::CVariant* attribute = GUCEF_NULL;
    const CORE::CDataNode* child = GUCEF_NULL;
    if ( !Has( name, attribute, child ) )
        return;
    if ( GUCEF_NULL == child || GUCEF_DATATYPE_ARRAY != child->GetNodeType() )
    {
        m_problem = m_context + ": \"" + name + "\" must be a list of texts";
        return;
    }
    CORE::CString::StringVector list;
    CORE::CDataNode::const_iterator i = child->ConstBegin();
    while ( i != child->ConstEnd() )
    {
        const CORE::CVariant& item = (*i)->GetValue();
        if ( !item.IsString() || (*i)->HasChildren() )
        {
            m_problem = m_context + ": \"" + name + "\" must be a list of texts";
            return;
        }
        list.push_back( item.AsString() );
        ++i;
    }
    if ( maxItems > 0 && list.size() > maxItems )
    {
        m_problem = m_context + ": \"" + name + "\" has more than " + CORE::ToString( maxItems ) + " entries";
        return;
    }
    values = list;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CAssetStrictConfigReader::ReadObject( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CVariant* attribute = GUCEF_NULL;
    const CORE::CDataNode* child = GUCEF_NULL;
    if ( !Has( name, attribute, child ) )
        return GUCEF_NULL;
    if ( GUCEF_NULL == child || GUCEF_DATATYPE_OBJECT != child->GetNodeType() )
    {
        m_problem = m_context + ": \"" + name + "\" must be an object";
        return GUCEF_NULL;
    }
    return child;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CAssetStrictConfigReader::ReadArray( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CVariant* attribute = GUCEF_NULL;
    const CORE::CDataNode* child = GUCEF_NULL;
    if ( !Has( name, attribute, child ) )
        return GUCEF_NULL;
    if ( GUCEF_NULL == child || GUCEF_DATATYPE_ARRAY != child->GetNodeType() )
    {
        m_problem = m_context + ": \"" + name + "\" must be a list";
        return GUCEF_NULL;
    }
    return child;
}

/*-------------------------------------------------------------------------*/

void
CAssetStrictConfigReader::Fail( const CORE::CString& problem )
{GUCEF_TRACE;

    if ( m_problem.IsNULLOrEmpty() )
        m_problem = m_context + ": " + problem;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStrictConfigReader::HasProblem( void ) const
{GUCEF_TRACE;

    return !m_problem.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStrictConfigReader::Finish( CORE::CString& problem )
{GUCEF_TRACE;

    if ( m_problem.IsNULLOrEmpty() )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = m_node.AttributeBegin();
        while ( a != m_node.AttributeEnd() && m_problem.IsNULLOrEmpty() )
        {
            if ( m_named.end() == m_named.find( (*a).first ) )
                m_problem = m_context + ": the member \"" + (*a).first + "\" is not known";
            ++a;
        }
        CORE::CDataNode::const_iterator c = m_node.ConstBegin();
        while ( c != m_node.ConstEnd() && m_problem.IsNULLOrEmpty() )
        {
            if ( m_named.end() == m_named.find( (*c)->GetName() ) )
                m_problem = m_context + ": the member \"" + (*c)->GetName() + "\" is not known";
            ++c;
        }
    }
    problem = m_problem;
    return m_problem.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
