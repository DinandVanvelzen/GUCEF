/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDataNode.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDataNodeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.DataNode" );

    GUCEF_TESTFW_TESTCASE( "DataNode: A mutable iterator walks every child once and then equals End()" )
    try
    {
        CORE::CDataNode root( "Root" );
        root.AddChild( "A" );
        root.AddChild( "B" );
        root.AddChild( "C" );

        CORE::CString seen;
        UInt32 steps = 0;
        CORE::CDataNode::iterator i = root.Begin();
        while ( i != root.End() && steps < 10 )
        {
            seen += (*i)->GetName();
            ++i;
            ++steps;
        }
        ASSERT_TRUE( "ABC" == seen && 3 == steps );
        ASSERT_TRUE( i == root.End() );

        // the const iterator agrees
        const CORE::CDataNode& constRoot = root;
        steps = 0;
        CORE::CDataNode::const_iterator c = constRoot.Begin();
        while ( c != constRoot.End() && steps < 10 )
        {
            ++c;
            ++steps;
        }
        ASSERT_TRUE( 3 == steps );

        // a node without children: Begin() is End() at once
        CORE::CDataNode empty( "Empty" );
        ASSERT_TRUE( empty.Begin() == empty.End() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DataNode: Children can be changed and deleted while walking with a mutable iterator" )
    try
    {
        CORE::CDataNode root( "Root" );
        for ( UInt32 n=0; n<5; ++n )
            root.AddChild( n % 2 == 0 ? "Keep" : "Drop" )->SetAttribute( "index", n );

        // step past a node before deleting it; a deleted node leaves its parent
        CORE::CDataNode::iterator i = root.Begin();
        while ( i != root.End() )
        {
            CORE::CDataNode* child = (*i);
            ++i;
            if ( child->GetName() == "Drop" )
                GUCEF_DELETE child;
            else
                child->SetAttribute( "seen", "yes" );
        }
        ASSERT_TRUE( 3 == root.GetNrOfDirectChildNodes() );
        UInt32 kept = 0;
        for ( CORE::CDataNode::iterator k = root.Begin(); k != root.End(); ++k )
        {
            ASSERT_TRUE( "Keep" == (*k)->GetName() && "yes" == (*k)->GetAttributeValue( "seen" ).AsString() );
            ++kept;
        }
        ASSERT_TRUE( 3 == kept );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DataNode: A copy of a child is a node of its own, not in the tree of the original, and outlives that tree" )
    try
    {
        CORE::CDataNode* tree = GUCEF_NEW CORE::CDataNode( "root" );
        CORE::CDataNode* list = tree->AddChild( "list" );
        list->AddChild( "first" )->SetAttribute( "a", "1" );
        CORE::CDataNode* second = list->AddChild( "second" );
        second->SetAttribute( "b", "2" );
        second->AddChild( "inner" );
        list->AddChild( "third" );

        CORE::CDataNode copy( *second );
        ASSERT_TRUE( GUCEF_NULL == copy.GetParent() && "second" == copy.GetName() && "2" == copy.GetAttributeValue( "b" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL != copy.FindChild( "inner" ) && copy.FindChild( "inner" )->GetParent() == &copy );
        ASSERT_TRUE( second == list->FindChild( "second" ) && 4 == list->GetNrOfChildNodes() );   // the original stays in its tree

        // the copies kept by value, as a list of settings keeps them, live on after the tree is gone
        std::vector< CORE::CDataNode > kept;
        kept.push_back( *second );
        kept.push_back( *list->FindChild( "third" ) );
        GUCEF_DELETE tree;
        ASSERT_TRUE( 2 == kept.size() && "second" == kept[ 0 ].GetName() && GUCEF_NULL == kept[ 1 ].GetParent() );

        #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
        // a moved node takes its children along: they have it as their parent
        CORE::CDataNode* source = GUCEF_NEW CORE::CDataNode( "source" );
        source->AddChild( "child" );
        CORE::CDataNode moved( GUCEF_MOVE( *source ) );
        GUCEF_DELETE source;
        ASSERT_TRUE( GUCEF_NULL != moved.FindChild( "child" ) && &moved == moved.FindChild( "child" )->GetParent() );
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DataNode: GetChildAtIndex gives the child at the index, and nothing past the last child" )
    try
    {
        CORE::CDataNode parent( "parent" );
        ASSERT_TRUE( GUCEF_NULL == parent.GetChildAtIndex( 0 ) );
        parent.AddChild( "first" );
        parent.AddChild( "second" );
        parent.AddChild( "third" );
        ASSERT_TRUE( parent.FindChild( "first" ) == parent.GetChildAtIndex( 0 ) );
        ASSERT_TRUE( parent.FindChild( "second" ) == parent.GetChildAtIndex( 1 ) );
        ASSERT_TRUE( parent.FindChild( "third" ) == parent.GetChildAtIndex( 2 ) );
        ASSERT_TRUE( GUCEF_NULL == parent.GetChildAtIndex( 3 ) );
        ASSERT_TRUE( GUCEF_NULL == parent.GetChildAtIndex( 1000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DataNode: A list of strings stored with SetValuesOfChildByName comes back in the same order with GetValuesOfChildByName" )
    try
    {
        CORE::CDataNode root( "root" );
        CORE::CString::StringVector scopes;
        scopes.push_back( "openid" );
        scopes.push_back( "profile" );
        scopes.push_back( "email" );
        scopes.push_back( "" );
        scopes.push_back( "openid" );
        ASSERT_TRUE( root.SetValuesOfChildByName( "scopes", scopes ) );

        const CORE::CDataNode* node = root.FindChild( "scopes" );
        ASSERT_TRUE( GUCEF_NULL != node && GUCEF_DATATYPE_ARRAY == node->GetNodeType() && 5 == node->GetNrOfChildNodes() );

        CORE::CString::StringVector read;
        read.push_back( "stale" );
        ASSERT_TRUE( root.GetValuesOfChildByName( "scopes", read ) );
        ASSERT_TRUE( scopes.size() == read.size() );
        for ( size_t i=0; i<scopes.size(); ++i )
            ASSERT_TRUE( scopes[ i ] == read[ i ] );

        // an empty list is a node that is there with nothing in it
        CORE::CString::StringVector none;
        ASSERT_TRUE( root.SetValuesOfChildByName( "nothing", none ) && root.GetValuesOfChildByName( "nothing", read ) && read.empty() );

        // a name that is not there: false, and the result is cleared and not left as it was
        read.push_back( "stale" );
        ASSERT_FALSE( root.GetValuesOfChildByName( "missing", read ) );
        ASSERT_TRUE( read.empty() );

        // every codec keeps the order of an array: the values of the children are not sorted or made unique
        CORE::CString::StringVector unordered;
        unordered.push_back( "zulu" );
        unordered.push_back( "alpha" );
        unordered.push_back( "zulu" );
        CORE::CDataNode other( "other" );
        other.SetValuesOfChildByName( "list", unordered );
        other.GetValuesOfChildByName( "list", read );
        ASSERT_TRUE( 3 == read.size() && "zulu" == read[ 0 ] && "alpha" == read[ 1 ] && "zulu" == read[ 2 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DataNode: GetStringListByName reads an array of children, or a text with separators from an attribute or a child value, and trims it" )
    try
    {
        CORE::CString::StringVector list;

        // the array form
        CORE::CDataNode arrayForm( "root" );
        CORE::CDataNode* array = arrayForm.AddChild( "audiences", GUCEF_DATATYPE_ARRAY );
        array->AddValueAsChild( CORE::CString( "  one " ) );
        array->AddValueAsChild( CORE::CString( "" ) );
        array->AddValueAsChild( CORE::CString( "two" ) );
        ASSERT_TRUE( arrayForm.GetStringListByName( "audiences", list ) );
        ASSERT_TRUE( 2 == list.size() && "one" == list[ 0 ] && "two" == list[ 1 ] );

        // an attribute with a list: blanks around the elements and empty elements are left out
        CORE::CDataNode attributeForm( "root" );
        attributeForm.SetAttribute( "audiences", CORE::CString( " a , b,, c ,  " ) );
        ASSERT_TRUE( attributeForm.GetStringListByName( "audiences", list ) );
        ASSERT_TRUE( 3 == list.size() && "a" == list[ 0 ] && "b" == list[ 1 ] && "c" == list[ 2 ] );

        // a child with a value
        CORE::CDataNode childForm( "root" );
        childForm.AddChildWithValue( "audiences", CORE::CString( "x,y" ) );
        ASSERT_TRUE( childForm.GetStringListByName( "audiences", list ) );
        ASSERT_TRUE( 2 == list.size() && "x" == list[ 0 ] && "y" == list[ 1 ] );

        // another separator
        CORE::CDataNode separated( "root" );
        separated.SetAttribute( "ids", CORE::CString( "p;q; r" ) );
        ASSERT_TRUE( separated.GetStringListByName( "ids", list, ';' ) );
        ASSERT_TRUE( 3 == list.size() && "p" == list[ 0 ] && "q" == list[ 1 ] && "r" == list[ 2 ] );

        // a single element
        CORE::CDataNode single( "root" );
        single.SetAttribute( "ids", CORE::CString( "only" ) );
        ASSERT_TRUE( single.GetStringListByName( "ids", list ) && 1 == list.size() && "only" == list[ 0 ] );

        // mentioned and empty is not the same as not mentioned
        CORE::CDataNode empty( "root" );
        empty.SetAttribute( "ids", CORE::CString( "" ) );
        list.push_back( "stale" );
        ASSERT_TRUE( empty.GetStringListByName( "ids", list ) && list.empty() );
        list.push_back( "stale" );
        ASSERT_FALSE( empty.GetStringListByName( "other", list ) );
        ASSERT_TRUE( list.empty() );

        // what SetValuesOfChildByName stores is read back, with the blanks and the empty element taken away
        CORE::CString::StringVector stored;
        stored.push_back( "first" );
        stored.push_back( " " );
        stored.push_back( "second" );
        CORE::CDataNode roundTrip( "root" );
        roundTrip.SetValuesOfChildByName( "names", stored );
        ASSERT_TRUE( roundTrip.GetStringListByName( "names", list ) );
        ASSERT_TRUE( 2 == list.size() && "first" == list[ 0 ] && "second" == list[ 1 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DataNode: A value list stored with SetAttributesOfChildByName is the attributes of one child and comes back with GetAttributesOfChildByName" )
    try
    {
        CORE::CValueList pairs;
        pairs.Set( "mode", "capture" );
        pairs.Set( "slots", "8" );
        pairs.Set( "empty", "" );

        CORE::CDataNode root( "root" );
        ASSERT_TRUE( root.SetAttributesOfChildByName( "settings", pairs ) );

        // one object node, the pairs are its attributes and it has no children
        const CORE::CDataNode* node = root.FindChild( "settings" );
        ASSERT_TRUE( GUCEF_NULL != node && GUCEF_DATATYPE_OBJECT == node->GetNodeType() && 0 == node->GetNrOfChildNodes() );
        ASSERT_TRUE( "capture" == node->GetAttributeValue( "mode" ).AsString() && "8" == node->GetAttributeValue( "slots" ).AsString() && node->IsAttribute( "empty" ) );

        CORE::CValueList read;
        read.Set( "stale", "x" );
        ASSERT_TRUE( root.GetAttributesOfChildByName( "settings", read ) );
        ASSERT_TRUE( 3 == read.GetKeyCount() && !read.HasKey( "stale" ) );
        ASSERT_TRUE( "capture" == read.GetValue( "mode" ).AsString() && "8" == read.GetValue( "slots" ).AsString() && read.HasKey( "empty" ) );

        // a key with several values keeps the first one
        CORE::CValueList several;
        several.SetAllowMultipleValues( true );
        several.Set( "key", "first" );
        several.Set( "key", "second" );
        CORE::CDataNode other( "other" );
        other.SetAttributesOfChildByName( "list", several );
        ASSERT_TRUE( "first" == other.FindChild( "list" )->GetAttributeValue( "key" ).AsString() );

        // no such child: false and the list is emptied
        read.Set( "stale", "x" );
        ASSERT_FALSE( root.GetAttributesOfChildByName( "missing", read ) );
        ASSERT_TRUE( 0 == read.GetKeyCount() );

        // an empty list is an object node without attributes
        CORE::CValueList none;
        ASSERT_TRUE( root.SetAttributesOfChildByName( "nothing", none ) && root.GetAttributesOfChildByName( "nothing", read ) && 0 == read.GetKeyCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
