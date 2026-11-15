/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#include "gucefCORE_CDataTableDefinition.h"
#define GUCEF_CORE_CDATATABLEDEFINITION_H
#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDataTableIndexHints.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CString::StringVector
IndexHintNames( const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL )
{
    CString::StringVector names;
    if ( GUCEF_NULL != a ) names.push_back( a );
    if ( GUCEF_NULL != b ) names.push_back( b );
    if ( GUCEF_NULL != c ) names.push_back( c );
    return names;
}

/*-------------------------------------------------------------------------*/

/** A catalogue like table: an asset per row, keyed by id, found by pack and by digest */
CDataTableDefinition
MakeAssetsDefinition( void )
{
    CDataTableDefinition definition;
    definition.name = "assets";
    definition.schema.AddColumn( CDataTableColumnDef( "asset_id", DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "pack_id", DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "kind", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "name", DATATABLE_TYPE_UTF8 ) );
    definition.schema.AddColumn( CDataTableColumnDef( "sha1", DATATABLE_TYPE_UTF8 ) );
    definition.schema.SetPrimaryKey( IndexHintNames( "asset_id" ) );
    definition.backendOptions.Set( "note", CVariant( "kept" ) );
    return definition;
}

/*-------------------------------------------------------------------------*/

/** The names of the children of a node, in order */
CString::StringVector
ChildNames( const CDataNode& node )
{
    CString::StringVector names;
    CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        names.push_back( (*i)->GetName() );
        ++i;
    }
    return names;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDataTableIndexHintTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING DATA TABLE INDEX HINT TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "DataTableIndexHints" );

    GUCEF_TESTFW_TESTCASE( "Test: a definition without index hints has none and serializes the members it always had, no \"indexes\"" )
        try
        {
            CDataTableDefinition definition = MakeAssetsDefinition();
            ASSERT_TRUE( definition.indexes.empty() && GUCEF_NULL == definition.FindIndex( "by_pack" ) );
            CString error;
            ASSERT_TRUE( definition.Validate( error ) );

            CDataNode node;
            ASSERT_TRUE( definition.Serialize( node, CDataNodeSerializableSettings() ) );
            const CString::StringVector children = ChildNames( node );
            ASSERT_TRUE( 4 == children.size() && children[ 0 ] == "columns" && children[ 1 ] == "primaryKey" && children[ 2 ] == "storage" && children[ 3 ] == "backendOptions" );
            ASSERT_TRUE( GUCEF_NULL == node.FindChild( "indexes" ) && !node.IsAttribute( "indexes" ) );

            CDataTableDefinition back;
            back.indexes.push_back( CDataTableIndexDef( "stale", IndexHintNames( "kind" ) ) );
            ASSERT_TRUE( back.Deserialize( node, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( back.indexes.empty() && back == definition );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: index hints round trip through a DataNode, copies and clones, in order with their columns and unique flags" )
        try
        {
            CDataTableDefinition definition = MakeAssetsDefinition();
            definition.indexes.push_back( CDataTableIndexDef( "by_pack", IndexHintNames( "pack_id", "kind" ) ) );
            definition.indexes.push_back( CDataTableIndexDef( "by_digest", IndexHintNames( "sha1" ), true ) );
            CString error;
            ASSERT_TRUE( definition.Validate( error ) );

            CDataNode node;
            ASSERT_TRUE( definition.Serialize( node, CDataNodeSerializableSettings() ) );
            const CDataNode* indexesNode = node.FindChild( "indexes" );
            ASSERT_TRUE( GUCEF_NULL != indexesNode && 2 == ChildNames( *indexesNode ).size() );
            const CDataNode* first = indexesNode->FindChild( "index" );
            ASSERT_TRUE( GUCEF_NULL != first && first->GetAttributeValue( "name" ).AsString() == "by_pack" );
            ASSERT_TRUE( !first->IsAttribute( "unique" ) );

            CDataTableDefinition back;
            ASSERT_TRUE( back.Deserialize( node, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( back == definition && back.Validate( error ) );
            ASSERT_TRUE( 2 == back.indexes.size() );
            ASSERT_TRUE( back.indexes[ 0 ].name == "by_pack" && back.indexes[ 0 ].columns == IndexHintNames( "pack_id", "kind" ) && !back.indexes[ 0 ].unique );
            ASSERT_TRUE( back.indexes[ 1 ].name == "by_digest" && back.indexes[ 1 ].columns == IndexHintNames( "sha1" ) && back.indexes[ 1 ].unique );
            ASSERT_TRUE( GUCEF_NULL != back.FindIndex( "by_digest" ) && back.FindIndex( "by_digest" )->unique );
            ASSERT_TRUE( GUCEF_NULL == back.FindIndex( "BY_DIGEST" ) );

            CDataTableDefinition copy( definition );
            ASSERT_TRUE( copy == definition );
            CDataTableDefinition assigned;
            assigned = definition;
            ASSERT_TRUE( assigned == definition );
            CICloneable* clone = definition.Clone();
            ASSERT_TRUE( GUCEF_NULL != clone && *static_cast< CDataTableDefinition* >( clone ) == definition );
            GUCEF_DELETE clone;

            // the hints are part of equality: order, columns and the unique flag
            CDataTableDefinition other( definition );
            other.indexes[ 1 ].unique = false;
            ASSERT_TRUE( other != definition );
            other = definition;
            other.indexes[ 0 ].columns = IndexHintNames( "kind", "pack_id" );
            ASSERT_TRUE( other != definition );
            other = definition;
            other.indexes.pop_back();
            ASSERT_TRUE( other != definition );
            other = definition;
            CDataTableIndexDef swap = other.indexes[ 0 ];
            other.indexes[ 0 ] = other.indexes[ 1 ];
            other.indexes[ 1 ] = swap;
            ASSERT_TRUE( other != definition );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: index hint validation refuses unknown and repeated columns, no columns, unusable names and names used twice" )
        try
        {
            CString error;
            const CDataTableDefinition base = MakeAssetsDefinition();

            CDataTableDefinition definition = base;
            definition.indexes.push_back( CDataTableIndexDef( "by_size", IndexHintNames( "size" ) ) );
            ASSERT_FALSE( definition.Validate( error ) );
            ASSERT_TRUE( error.HasSubstr( "size" ) >= 0 );

            definition = base;
            definition.indexes.push_back( CDataTableIndexDef( "twice", IndexHintNames( "kind", "kind" ) ) );
            ASSERT_FALSE( definition.Validate( error ) );

            definition = base;
            definition.indexes.push_back( CDataTableIndexDef( "empty", CString::StringVector() ) );
            ASSERT_FALSE( definition.Validate( error ) );

            definition = base;
            definition.indexes.push_back( CDataTableIndexDef( "by_kind", IndexHintNames( "kind" ) ) );
            definition.indexes.push_back( CDataTableIndexDef( "BY_KIND", IndexHintNames( "name" ) ) );
            ASSERT_FALSE( definition.Validate( error ) );

            // column names are case sensitive, as in the schema
            definition = base;
            definition.indexes.push_back( CDataTableIndexDef( "by_kind", IndexHintNames( "Kind" ) ) );
            ASSERT_FALSE( definition.Validate( error ) );

            const char* badNames[] = { "", "1st", "by-kind", "by kind", "by.kind", "\xC3\xA9t\xC3\xA9", "a\x01" };
            for ( UInt32 i=0; i<sizeof( badNames ) / sizeof( badNames[ 0 ] ); ++i )
            {
                ASSERT_FALSE( CDataTableIndexDef::IsValidIndexName( badNames[ i ] ) );
                definition = base;
                definition.indexes.push_back( CDataTableIndexDef( badNames[ i ], IndexHintNames( "kind" ) ) );
                ASSERT_FALSE( definition.Validate( error ) );
            }
            const CString longest( "a23456789012345678901234567890123456789012345678901234567890123" );
            ASSERT_TRUE( CDataTableIndexDef::MaxNameLength == longest.Length() && CDataTableIndexDef::IsValidIndexName( longest ) );
            ASSERT_FALSE( CDataTableIndexDef::IsValidIndexName( longest + "4" ) );
            ASSERT_TRUE( CDataTableIndexDef::IsValidIndexName( "_x" ) && CDataTableIndexDef::IsValidIndexName( "by_pack_2" ) && CDataTableIndexDef::IsValidIndexName( "Z" ) );

            // a valid set: the same columns in two indexes and an index on the primary key column are allowed
            definition = base;
            definition.indexes.push_back( CDataTableIndexDef( "a", IndexHintNames( "pack_id" ) ) );
            definition.indexes.push_back( CDataTableIndexDef( "b", IndexHintNames( "pack_id", "name" ), true ) );
            definition.indexes.push_back( CDataTableIndexDef( "c", IndexHintNames( "asset_id" ), true ) );
            ASSERT_TRUE( definition.Validate( error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a definition node with a malformed index hint is refused and leaves the definition as it was" )
        try
        {
            CDataTableDefinition definition = MakeAssetsDefinition();
            definition.indexes.push_back( CDataTableIndexDef( "by_pack", IndexHintNames( "pack_id" ) ) );
            CDataNode node;
            ASSERT_TRUE( definition.Serialize( node, CDataNodeSerializableSettings() ) );

            CDataNode noColumns( node );
            CDataNode* index = noColumns.FindChild( "indexes" )->FindChild( "index" );
            index->DelSubTree();
            index->SetAttribute( "name", "by_pack" );
            CDataTableDefinition target = MakeAssetsDefinition();
            ASSERT_FALSE( target.Deserialize( noColumns, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( target == MakeAssetsDefinition() );

            CDataNode badName( node );
            badName.FindChild( "indexes" )->FindChild( "index" )->SetAttribute( "name", "by pack" );
            ASSERT_FALSE( target.Deserialize( badName, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( target == MakeAssetsDefinition() );

            // an index on a column the schema lacks deserializes (the node is read as it is) and fails validation
            CDataNode unknownColumn( node );
            CDataNode* unknownIndex = unknownColumn.FindChild( "indexes" )->FindChild( "index" );
            CString::StringVector columns;
            columns.push_back( "missing" );
            unknownIndex->SetValuesOfChildByName( "columns", columns );
            ASSERT_TRUE( target.Deserialize( unknownColumn, CDataNodeSerializableSettings() ) );
            CString error;
            ASSERT_FALSE( target.Validate( error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
