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

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H
#include "gucefCORE_CDataNodeDocumentBuilder.h"
#define GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H
#endif /* GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H ? */

#ifndef GUCEF_CORE_CDATANODESUBTREECOLLECTOR_H
#include "gucefCORE_CDataNodeSubtreeCollector.h"
#define GUCEF_CORE_CDATANODESUBTREECOLLECTOR_H
#endif /* GUCEF_CORE_CDATANODESUBTREECOLLECTOR_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDStoreStreaming.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

typedef CORE::CDStoreCodecRegistry::TDStoreCodecPtr TCodecPtr;

/*-------------------------------------------------------------------------*/

static bool
GetCodec( const char* codecType, const char* pluginName, TCodecPtr& codec )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    if ( coreGlobal->GetDStoreCodecRegistry().TryLookup( codecType, codec, false ) )
        return true;
    if ( GUCEF_NULL == pluginName )
        return false;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    CORE::CString secondPluginPath = CORE::CString( "$MODULEDIR$/" ) + pluginName;
    #else
    CORE::CString firstPluginPath = CORE::CString( "$MODULEDIR$/" ) + pluginName;
    CORE::CString secondPluginPath = CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );

    return coreGlobal->GetDStoreCodecRegistry().TryLookup( codecType, codec, false );
}

/*-------------------------------------------------------------------------*/

/** a small document resembling a genealogy database */
static void
BuildSampleTree( CORE::CDataNode& root, UInt32 personCount )
{GUCEF_TRACE;

    root.Clear();
    root.SetName( "database" );
    root.SetAttribute( "version", "1.7.1" );
    CORE::CDataNode* people = root.AddChild( "people" );
    for ( UInt32 i=0; i<personCount; ++i )
    {
        CORE::CDataNode* person = people->AddChild( "person" );
        person->SetAttribute( "handle", "_P" + CORE::UInt32ToString( i ) );
        person->SetAttribute( "id", "I" + CORE::UInt32ToString( 1000+i ) );
        CORE::CDataNode* name = person->AddChild( "name" );
        name->SetAttribute( "type", "Birth Name" );
        name->AddChildWithValue( "first", CORE::CVariant( "Jan" ), GUCEF_DATATYPE_OBJECT );
        name->AddChildWithValue( "surname", CORE::CVariant( "de Vr\xC3\xBB" ), GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* gender = person->AddChild( "gender" );
        gender->SetValue( CORE::CVariant( ( 0 == i % 2 ) ? "M" : "F" ) );
    }
    CORE::CDataNode* families = root.AddChild( "families" );
    CORE::CDataNode* family = families->AddChild( "family" );
    family->SetAttribute( "handle", "_F0" );
}

/*-------------------------------------------------------------------------*/

/** writes the given tree node by node using BeginNode() and EndNode() only */
static bool
WriteNodeByNode( CORE::CDocumentStreamWriter& writer, const CORE::CDataNode& node )
{GUCEF_TRACE;

    bool success = writer.BeginNode( node, node.HasChildren() );
    CORE::CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        success = WriteNodeByNode( writer, *(*i) ) && success;
        ++i;
    }
    return writer.EndNode() && success;
}

/*-------------------------------------------------------------------------*/

/** writes the given tree the way a large export would: the outer nodes node by node and each record in one go */
static bool
WriteRecordByRecord( CORE::CDocumentStreamWriter& writer, const CORE::CDataNode& root )
{GUCEF_TRACE;

    bool success = writer.BeginNode( root, true );
    CORE::CDataNode::const_iterator i = root.ConstBegin();
    while ( i != root.ConstEnd() )
    {
        const CORE::CDataNode& section = *(*i);
        success = writer.BeginNode( section, true ) && success;
        CORE::CDataNode::const_iterator n = section.ConstBegin();
        while ( n != section.ConstEnd() )
        {
            success = writer.WriteNode( *(*n) ) && success;
            ++n;
        }
        success = writer.EndNode() && success;
        ++i;
    }
    return writer.EndNode() && success;
}

/*-------------------------------------------------------------------------*/

static bool
BuffersEqual( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{GUCEF_TRACE;

    return a.GetDataSize() == b.GetDataSize() &&
           0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** keeps every sub-tree it is handed */
class CKeepingCollector : public CORE::CDataNodeSubtreeCollector
{
    public:

    CKeepingCollector( UInt32 depth, const CORE::CString& name = CORE::CString::Empty )
        : CORE::CDataNodeSubtreeCollector( depth, name )
        , subtrees()
        , ancestorNames()
        , ancestorVersion()
    {}

    virtual void OnSubtree( const CORE::CDataNode& subtree ) GUCEF_VIRTUAL_OVERRIDE
    {
        subtrees.push_back( subtree );
        ancestorNames.clear();
        const TDataNodeVector& ancestors = GetAncestors();
        for ( size_t i=0; i<ancestors.size(); ++i )
            ancestorNames.push_back( ancestors[ i ].GetName() );
        if ( !ancestors.empty() )
            ancestorVersion = ancestors[ 0 ].GetAttributeValue( "version" ).AsString();
    }

    GUCEF::vector< CORE::CDataNode > subtrees;
    GUCEF::vector< CORE::CString > ancestorNames;
    CORE::CString ancestorVersion;
};

/** only counts, as a reader of a huge document would */
class CCountingCollector : public CORE::CDataNodeSubtreeCollector
{
    public:

    CCountingCollector( UInt32 depth, const CORE::CString& name )
        : CORE::CDataNodeSubtreeCollector( depth, name )
        , lastHandle()
    {}

    virtual void OnSubtree( const CORE::CDataNode& subtree ) GUCEF_VIRTUAL_OVERRIDE
    {
        lastHandle = subtree.GetAttributeValue( "handle" ).AsString();
    }

    CORE::CString lastHandle;
};

/*-------------------------------------------------------------------------*/

/** Checks writing and parsing documents with the given codec */
static void
PerformStreamingTestsForCodec( const char* codecType, const char* pluginName )
{GUCEF_TRACE;

    TCodecPtr codec;
    if ( !GetCodec( codecType, pluginName, codec ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "DStore streaming tests skipped for codec type " ) + codecType + ": codec not available" );
        return;
    }

    CORE::CString caseSuffix = CORE::CString( " (" ) + codecType + ")";

    CORE::CDataNode sample;
    BuildSampleTree( sample, 5 );

    GUCEF_TESTFW_TESTCASE( ( CORE::CString( "Test: Writing node by node gives the same output as storing the whole tree" ) + caseSuffix ).C_String() )
        try
        {
            CORE::CDynamicBuffer wholeTreeOutput;
            {
                CORE::CDynamicBufferAccess access( &wholeTreeOutput, false );
                ASSERT_TRUE( codec->StoreDataTree( &sample, &access ) );
            }
            ASSERT_TRUE( wholeTreeOutput.GetDataSize() > 0 );

            CORE::CDynamicBuffer nodeByNodeOutput;
            {
                CORE::CDynamicBufferAccess access( &nodeByNodeOutput, false );
                CORE::CDocumentStreamWriterPtr writer = codec->CreateDocumentWriter( access );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() )
                {
                    ASSERT_TRUE( WriteNodeByNode( *writer, sample ) );
                    ASSERT_TRUE( 0 == writer->GetDepth() );
                    ASSERT_TRUE( writer->Close() );
                    ASSERT_FALSE( writer->IsOpen() );
                }
            }
            ASSERT_TRUE( BuffersEqual( wholeTreeOutput, nodeByNodeOutput ) );

            CORE::CDynamicBuffer recordOutput;
            {
                CORE::CDynamicBufferAccess access( &recordOutput, false );
                CORE::CDocumentStreamWriterPtr writer = codec->CreateDocumentWriter( access );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() )
                {
                    ASSERT_TRUE( WriteRecordByRecord( *writer, sample ) );
                    ASSERT_TRUE( writer->Close() );
                }
            }
            ASSERT_TRUE( BuffersEqual( wholeTreeOutput, recordOutput ) );

            CORE::CDynamicBuffer singleCallOutput;
            {
                CORE::CDynamicBufferAccess access( &singleCallOutput, false );
                CORE::CDocumentStreamWriterPtr writer = codec->CreateDocumentWriter( access );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() )
                {
                    ASSERT_TRUE( writer->WriteNode( sample ) );
                    // releasing the writer closes it
                }
            }
            ASSERT_TRUE( BuffersEqual( wholeTreeOutput, singleCallOutput ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( CORE::CString( "Test: Parsing reports the same content as building the tree" ) + caseSuffix ).C_String() )
        try
        {
            CORE::CDynamicBuffer output;
            {
                CORE::CDynamicBufferAccess access( &output, false );
                ASSERT_TRUE( codec->StoreDataTree( &sample, &access ) );
            }

            CORE::CDataNode built;
            {
                CORE::CDynamicBufferAccess access( output );
                ASSERT_TRUE( codec->BuildDataTree( &built, &access ) );
            }

            CKeepingCollector wholeDocument( 0 );
            {
                CORE::CDynamicBufferAccess access( output );
                ASSERT_TRUE( codec->ParseDocument( access, wholeDocument ) );
            }
            ASSERT_FALSE( wholeDocument.HasParseError() );
            ASSERT_TRUE( 1 == wholeDocument.subtrees.size() );
            if ( 1 == wholeDocument.subtrees.size() )
                ASSERT_TRUE( wholeDocument.subtrees[ 0 ] == built );

            CORE::CDataNodeDocumentBuilder builder;
            {
                CORE::CDynamicBufferAccess access( output );
                ASSERT_TRUE( codec->ParseDocument( access, builder ) );
            }
            ASSERT_TRUE( builder.document == built );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( CORE::CString( "Test: The subtree collector hands over each record with its ancestors" ) + caseSuffix ).C_String() )
        try
        {
            CORE::CDynamicBuffer output;
            {
                CORE::CDynamicBufferAccess access( &output, false );
                CORE::CDocumentStreamWriterPtr writer = codec->CreateDocumentWriter( access );
                ASSERT_FALSE( writer.IsNULL() );
                if ( !writer.IsNULL() )
                    ASSERT_TRUE( WriteRecordByRecord( *writer, sample ) );
            }

            CORE::CDataNode built;
            {
                CORE::CDynamicBufferAccess access( output );
                ASSERT_TRUE( codec->BuildDataTree( &built, &access ) );
            }
            CORE::CDataNode* builtPeople = built.FindChild( "people" );
            ASSERT_TRUE( GUCEF_NULL != builtPeople );

            // only persons, the family at the same depth is skipped
            CKeepingCollector persons( 2, "person" );
            {
                CORE::CDynamicBufferAccess access( output );
                ASSERT_TRUE( codec->ParseDocument( access, persons ) );
            }
            ASSERT_TRUE( 5 == persons.subtrees.size() && 5 == persons.GetSubtreeCount() );
            if ( 5 == persons.subtrees.size() && GUCEF_NULL != builtPeople )
            {
                UInt32 index = 0;
                CORE::CDataNode::const_iterator i = builtPeople->ConstBegin();
                while ( i != builtPeople->ConstEnd() && index < 5 )
                {
                    ASSERT_TRUE( persons.subtrees[ index ] == *(*i) );
                    ++i; ++index;
                }
                ASSERT_TRUE( persons.subtrees[ 3 ].GetAttributeValue( "handle" ).AsString() == "_P3" );
            }
            // the enclosing nodes as the format represents them (JSON for example has no name for the root)
            ASSERT_TRUE( 2 == persons.ancestorNames.size() );
            if ( 2 == persons.ancestorNames.size() && GUCEF_NULL != builtPeople )
                ASSERT_TRUE( persons.ancestorNames[ 0 ] == built.GetName() && persons.ancestorNames[ 1 ] == builtPeople->GetName() );
            ASSERT_TRUE( persons.ancestorVersion == "1.7.1" );

            // without a name filter every node at that depth is handed over
            UInt32 nodesAtDepth2 = 0;
            CORE::CDataNode::const_iterator section = built.ConstBegin();
            while ( section != built.ConstEnd() )
            {
                nodesAtDepth2 += (*section)->GetNrOfDirectChildNodes();
                ++section;
            }
            CKeepingCollector everything( 2 );
            {
                CORE::CDynamicBufferAccess access( output );
                ASSERT_TRUE( codec->ParseDocument( access, everything ) );
            }
            ASSERT_TRUE( nodesAtDepth2 == everything.subtrees.size() );
            ASSERT_TRUE( everything.subtrees.size() >= 5 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( CORE::CString( "Test: Misuse of the writer is refused" ) + caseSuffix ).C_String() )
        try
        {
            CORE::CDynamicBuffer output;
            CORE::CDynamicBufferAccess access( &output, false );
            CORE::CDocumentStreamWriterPtr writer = codec->CreateDocumentWriter( access );
            ASSERT_FALSE( writer.IsNULL() );
            if ( !writer.IsNULL() )
            {
                CORE::CDataNode root( GUCEF_DATATYPE_OBJECT );
                root.SetName( "root" );
                CORE::CDataNode leaf( GUCEF_DATATYPE_OBJECT );
                leaf.SetName( "leaf" );
                leaf.SetAttribute( "a", "1" );

                ASSERT_FALSE( writer->EndNode() );                  // nothing begun
                ASSERT_TRUE( writer->BeginNode( root, true ) );
                ASSERT_TRUE( writer->BeginNode( leaf, false ) );
                ASSERT_FALSE( writer->WriteNode( leaf ) );          // leaf was begun without children
                ASSERT_FALSE( writer->BeginNode( leaf, false ) );
                ASSERT_TRUE( writer->EndNode() );
                ASSERT_TRUE( writer->WriteNode( leaf ) );
                ASSERT_TRUE( 1 == writer->GetDepth() );
                ASSERT_TRUE( writer->Close() );                     // ends the root node
                ASSERT_TRUE( 0 == writer->GetDepth() );
                ASSERT_FALSE( writer->BeginNode( root, true ) );    // closed
                ASSERT_FALSE( writer->WriteNode( leaf ) );
                ASSERT_TRUE( writer->Close() );                     // harmless

                // what was written is a complete document
                CORE::CDataNode readBack;
                CORE::CDynamicBufferAccess readAccess( output );
                ASSERT_TRUE( codec->BuildDataTree( &readBack, &readAccess ) );
                ASSERT_TRUE( readBack.HasChildren() || readBack.GetAttCount() > 0 );
            }

            CORE::CDynamicBuffer output2;
            CORE::CDynamicBufferAccess access2( &output2, false );
            CORE::CDocumentStreamWriterPtr writer2 = codec->CreateDocumentWriter( access2 );
            if ( !writer2.IsNULL() )
            {
                CORE::CDataNode root( GUCEF_DATATYPE_OBJECT );
                root.SetName( "root" );
                ASSERT_TRUE( writer2->WriteNode( root ) );
                ASSERT_FALSE( writer2->WriteNode( root ) );         // a second root
                ASSERT_FALSE( writer2->BeginNode( root, false ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformDStoreStreamingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DStore streaming TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DStoreStreaming" );

    PerformStreamingTestsForCodec( "xml", "dstorepluginPARSIFALXML" );
    PerformStreamingTestsForCodec( "json", "dstorepluginJSONPARSER" );
    PerformStreamingTestsForCodec( "dnc", GUCEF_NULL );     // built-in, uses the in-memory fallback

    GUCEF_TESTFW_TESTCASE( "Test: Parse errors are reported" )
        try
        {
            TCodecPtr codec;
            ASSERT_TRUE( GetCodec( "xml", "dstorepluginPARSIFALXML", codec ) );
            if ( !codec.IsNULL() )
            {
                const char* broken = "<?xml version=\"1.0\"?><root><a></b></root>";
                CORE::CDynamicBuffer input;
                input.CopyFrom( (UInt32) strlen( broken ), broken );
                CORE::CDynamicBufferAccess access( input );
                CKeepingCollector collector( 1 );
                ASSERT_FALSE( codec->ParseDocument( access, collector ) );
                ASSERT_TRUE( collector.HasParseError() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: JSON objects with a single member keep their name and level when written back" )
        try
        {
            TCodecPtr codec;
            ASSERT_TRUE( GetCodec( "json", "dstorepluginJSONPARSER", codec ) );
            if ( !codec.IsNULL() )
            {
                // read, write, read again: the objects with one member stay objects at their level
                const char* text = "{ \"Other\": { \"keep\": \"yes\" }, \"Outer\": { \"Inner\": { \"value\": 7 } } }";
                CORE::CDynamicBuffer input;
                input.CopyFrom( (UInt32) strlen( text ), text );
                CORE::CDynamicBufferAccess inputAccess( input );
                CORE::CDataNode tree;
                ASSERT_TRUE( codec->BuildDataTree( &tree, &inputAccess ) );

                // and a node holding a value, made in code, stays a value of its parent
                CORE::CDataNode* main = tree.Structure( "Main", '/' );
                main->AddValueAsChild( CORE::CVariant( (Int32) 5 ), false, "count" );

                CORE::CDynamicBuffer output;
                {
                    CORE::CDynamicBufferAccess outputAccess( &output, false );
                    ASSERT_TRUE( codec->StoreDataTree( &tree, &outputAccess ) );
                }
                CORE::CDynamicBufferAccess rereadAccess( output );
                CORE::CDataNode reread;
                ASSERT_TRUE( codec->BuildDataTree( &reread, &rereadAccess ) );

                const CORE::CDataNode* other = reread.Search( "Other", '/', true, true );
                ASSERT_TRUE( GUCEF_NULL != other && "yes" == other->GetAttributeValueOrChildValueByName( "keep" ).AsString() );
                const CORE::CDataNode* inner = reread.Search( "Outer/Inner", '/', true, true );
                ASSERT_TRUE( GUCEF_NULL != inner && 7 == inner->GetAttributeValueOrChildValueByName( "value" ).AsInt32() );
                const CORE::CDataNode* readMain = reread.Search( "Main", '/', true, true );
                ASSERT_TRUE( GUCEF_NULL != readMain && 5 == readMain->GetAttributeValueOrChildValueByName( "count" ).AsInt32() );
                ASSERT_TRUE( GUCEF_NULL == reread.Search( "keep", '/', true, true ) && GUCEF_NULL == reread.GetAttribute( "keep" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A large document is written and read one record at a time" )
        try
        {
            TCodecPtr codec;
            ASSERT_TRUE( GetCodec( "xml", "dstorepluginPARSIFALXML", codec ) );
            if ( !codec.IsNULL() )
            {
                const UInt32 personCount = 50000;
                CORE::CDataNode person;
                BuildSampleTree( person, 1 );
                CORE::CDataNode* templatePerson = person.FindChild( "people" )->FindChild( "person" );
                CORE::CDataNode record( *templatePerson );

                CORE::CDataNode database( GUCEF_DATATYPE_OBJECT );
                database.SetName( "database" );
                database.SetAttribute( "version", "1.7.1" );
                CORE::CDataNode people( GUCEF_DATATYPE_OBJECT );
                people.SetName( "people" );

                CORE::CDynamicBuffer output;
                {
                    // written in many small pieces
                    CORE::CDynamicBufferAccess access( &output, false, CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
                    CORE::CDocumentStreamWriterPtr writer = codec->CreateDocumentWriter( access );
                    ASSERT_FALSE( writer.IsNULL() );
                    if ( !writer.IsNULL() )
                    {
                        bool success = writer->BeginNode( database, true ) && writer->BeginNode( people, true );
                        for ( UInt32 i=0; i<personCount && success; ++i )
                        {
                            // one record at a time: only this record is held in memory as a node
                            record.SetAttribute( "handle", "_P" + CORE::UInt32ToString( i ) );
                            success = writer->WriteNode( record );
                        }
                        ASSERT_TRUE( success );
                        ASSERT_TRUE( writer->Close() );
                    }
                }
                ASSERT_TRUE( output.GetDataSize() > personCount * 100 );

                CCountingCollector counter( 2, "person" );
                CORE::CDynamicBufferAccess access( output );
                ASSERT_TRUE( codec->ParseDocument( access, counter ) );
                ASSERT_TRUE( personCount == counter.GetSubtreeCount() );
                ASSERT_TRUE( counter.lastHandle == "_P" + CORE::UInt32ToString( personCount-1 ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL DStore streaming TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
