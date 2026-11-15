/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CSEARCHQUERY_H
#include "gucefGUI_CSearchQuery.h"
#define GUCEF_GUI_CSEARCHQUERY_H
#endif /* GUCEF_GUI_CSEARCHQUERY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

namespace {

/** Whether two nodes have the same name, attributes and children, in the same order */
bool
NodesEqual( const CORE::CDataNode& a, const CORE::CDataNode& b )
{GUCEF_TRACE;

    if ( a.GetName() != b.GetName() )
        return false;
    CORE::CDataNode::TAttributeMap::const_iterator x = a.AttributeBegin();
    CORE::CDataNode::TAttributeMap::const_iterator y = b.AttributeBegin();
    for ( ; x != a.AttributeEnd() && y != b.AttributeEnd(); ++x, ++y )
        if ( (*x).first != (*y).first || (*x).second.AsString() != (*y).second.AsString() )
            return false;
    if ( x != a.AttributeEnd() || y != b.AttributeEnd() )
        return false;
    CORE::CDataNode::const_iterator i = a.ConstBegin();
    CORE::CDataNode::const_iterator j = b.ConstBegin();
    for ( ; i != a.ConstEnd() && j != b.ConstEnd(); ++i, ++j )
        if ( !NodesEqual( *(*i), *(*j) ) )
            return false;
    return i == a.ConstEnd() && j == b.ConstEnd();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


CSearchQuery::SRange::SRange( void )
    : hasMinimum( false )
    , hasMaximum( false )
    , minimum( 0.0 )
    , maximum( 0.0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CSearchQuery::SRange::IsSet( void ) const
{GUCEF_TRACE;

    return hasMinimum || hasMaximum;
}

/*-------------------------------------------------------------------------*/

bool
CSearchQuery::SRange::operator==( const SRange& other ) const
{GUCEF_TRACE;

    return hasMinimum == other.hasMinimum && hasMaximum == other.hasMaximum &&
           ( !hasMinimum || minimum == other.minimum ) && ( !hasMaximum || maximum == other.maximum );
}

/*-------------------------------------------------------------------------*/

CSearchQuery::CSearchQuery( void )
    : text()
    , choices()
    , ranges()
    , sort()
    , pageIndex( 0 )
    , pageSize( 0 )
    , extra()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSearchQuery::CSearchQuery( const CSearchQuery& src )
    : text( src.text )
    , choices( src.choices )
    , ranges( src.ranges )
    , sort( src.sort )
    , pageIndex( src.pageIndex )
    , pageSize( src.pageSize )
    , extra()
{GUCEF_TRACE;

    extra.Copy( src.extra );
}

/*-------------------------------------------------------------------------*/

CSearchQuery&
CSearchQuery::operator=( const CSearchQuery& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        text = src.text;
        choices = src.choices;
        ranges = src.ranges;
        sort = src.sort;
        pageIndex = src.pageIndex;
        pageSize = src.pageSize;
        extra.Clear();
        extra.Copy( src.extra );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CSearchQuery::Clear( void )
{GUCEF_TRACE;

    text.Clear();
    choices.clear();
    ranges.clear();
    sort.Clear();
    pageIndex = 0;
    pageSize = 0;
    extra.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CSearchQuery::IsEmpty( void ) const
{GUCEF_TRACE;

    return text.IsNULLOrEmpty() && choices.empty() && ranges.empty();
}

/*-------------------------------------------------------------------------*/

void
CSearchQuery::AddChoice( const CString& groupId, const CString& value )
{GUCEF_TRACE;

    choices[ groupId ].insert( value );
}

/*-------------------------------------------------------------------------*/

void
CSearchQuery::RemoveChoice( const CString& groupId, const CString& value )
{GUCEF_TRACE;

    TChoiceMap::iterator i = choices.find( groupId );
    if ( i == choices.end() )
        return;
    (*i).second.erase( value );
    if ( (*i).second.empty() )
        choices.erase( i );
}

/*-------------------------------------------------------------------------*/

bool
CSearchQuery::HasChoice( const CString& groupId, const CString& value ) const
{GUCEF_TRACE;

    TChoiceMap::const_iterator i = choices.find( groupId );
    return i != choices.end() && (*i).second.find( value ) != (*i).second.end();
}

/*-------------------------------------------------------------------------*/

void
CSearchQuery::SetRange( const CString& groupId, const TRange& range )
{GUCEF_TRACE;

    if ( range.IsSet() )
        ranges[ groupId ] = range;
    else
        ranges.erase( groupId );
}

/*-------------------------------------------------------------------------*/

CSearchQuery::TRange
CSearchQuery::GetRange( const CString& groupId ) const
{GUCEF_TRACE;

    TRangeMap::const_iterator i = ranges.find( groupId );
    return i != ranges.end() ? (*i).second : TRange();
}

/*-------------------------------------------------------------------------*/

bool
CSearchQuery::IsSameSearch( const CSearchQuery& other ) const
{GUCEF_TRACE;

    if ( text != other.text || sort != other.sort || choices != other.choices || ranges.size() != other.ranges.size() )
        return false;
    for ( TRangeMap::const_iterator i = ranges.begin(); i != ranges.end(); ++i )
    {
        TRangeMap::const_iterator o = other.ranges.find( (*i).first );
        if ( o == other.ranges.end() || !( (*i).second == (*o).second ) )
            return false;
    }
    return NodesEqual( extra, other.extra );
}

/*-------------------------------------------------------------------------*/

bool
CSearchQuery::operator==( const CSearchQuery& other ) const
{GUCEF_TRACE;

    return pageIndex == other.pageIndex && pageSize == other.pageSize && IsSameSearch( other );
}

/*-------------------------------------------------------------------------*/

bool
CSearchQuery::operator!=( const CSearchQuery& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

void
CSearchQuery::Save( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetName( "SearchQuery" );
    node.SetAttribute( "text", text );
    node.SetAttribute( "sort", sort );
    node.SetAttribute( "pageSize", pageSize );
    for ( TChoiceMap::const_iterator i = choices.begin(); i != choices.end(); ++i )
    {
        CORE::CDataNode* group = node.AddChild( "Choices" );
        group->SetAttribute( "group", (*i).first );
        for ( CString::StringSet::const_iterator v = (*i).second.begin(); v != (*i).second.end(); ++v )
            group->AddChild( "Value" )->SetAttribute( "value", *v );
    }
    for ( TRangeMap::const_iterator i = ranges.begin(); i != ranges.end(); ++i )
    {
        CORE::CDataNode* range = node.AddChild( "Range" );
        range->SetAttribute( "group", (*i).first );
        if ( (*i).second.hasMinimum )
            range->SetAttribute( "minimum", CORE::CVariant( (*i).second.minimum ) );
        if ( (*i).second.hasMaximum )
            range->SetAttribute( "maximum", CORE::CVariant( (*i).second.maximum ) );
    }
    if ( extra.HasChildren() || extra.AttributeBegin() != extra.AttributeEnd() )
    {
        CORE::CDataNode* extraNode = node.AddChild( "extra" );
        extraNode->Copy( extra );
        extraNode->SetName( "extra" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSearchQuery::Load( const CORE::CDataNode& node )
{GUCEF_TRACE;

    if ( "SearchQuery" != node.GetName() )
        return false;
    Clear();
    text = node.GetAttributeValueOrChildValueByName( "text" ).AsString( CString::Empty, true );
    sort = node.GetAttributeValueOrChildValueByName( "sort" ).AsString( CString::Empty, true );
    pageSize = node.GetAttributeValueOrChildValueByName( "pageSize" ).AsUInt32( 0, true );
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
    {
        const CORE::CDataNode* child = (*i);
        if ( "Choices" == child->GetName() )
        {
            const CString group = child->GetAttributeValueOrChildValueByName( "group" ).AsString( CString::Empty, true );
            for ( CORE::CDataNode::const_iterator v = child->ConstBegin(); v != child->ConstEnd(); ++v )
                AddChoice( group, (*v)->GetAttributeValueOrChildValueByName( "value" ).AsString( CString::Empty, true ) );
        }
        else
        if ( "Range" == child->GetName() )
        {
            TRange range;
            const CORE::CVariant minimum = child->GetAttributeValueOrChildValueByName( "minimum" );
            const CORE::CVariant maximum = child->GetAttributeValueOrChildValueByName( "maximum" );
            if ( !minimum.IsNULLOrEmpty() )
            {
                range.hasMinimum = true;
                range.minimum = minimum.AsFloat64();
            }
            if ( !maximum.IsNULLOrEmpty() )
            {
                range.hasMaximum = true;
                range.maximum = maximum.AsFloat64();
            }
            SetRange( child->GetAttributeValueOrChildValueByName( "group" ).AsString( CString::Empty, true ), range );
        }
        else
        if ( "extra" == child->GetName() )
        {
            extra.Copy( *child );
            extra.SetName( "extra" );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
