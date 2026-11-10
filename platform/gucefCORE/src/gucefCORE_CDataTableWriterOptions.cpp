/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include "gucefCORE_CDataTableWriterOptions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Same keys with the same values, in the same order */
bool
ValueListsAreEqual( const CValueList& a, const CValueList& b )
{
    if ( a.GetKeyCount() != b.GetKeyCount() )
        return false;
    CValueList::TValueMap::const_iterator i = a.GetDataBeginIterator();
    CValueList::TValueMap::const_iterator n = b.GetDataBeginIterator();
    while ( i != a.GetDataEndIterator() && n != b.GetDataEndIterator() )
    {
        if ( (*i).first != (*n).first || (*i).second.size() != (*n).second.size() )
            return false;
        for ( UInt32 v=0; v<(UInt32) (*i).second.size(); ++v )
        {
            if ( (*i).second[ v ] != (*n).second[ v ] )
                return false;
        }
        ++i;
        ++n;
    }
    return true;
}

/** Stores a value list as an object child, one attribute per key with its first value, when it holds anything */
bool
SaveValueList( CDataNode& parent, const char* name, const CValueList& values )
{
    if ( 0 == values.GetKeyCount() )
        return true;
    CDataNode* node = parent.AddChild( name, GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == node )
        return false;
    bool success = true;
    CValueList::TValueMap::const_iterator i = values.GetDataBeginIterator();
    while ( i != values.GetDataEndIterator() )
    {
        if ( !(*i).second.empty() )
            success = node->SetAttribute( (*i).first.AsString(), (*i).second.front() ) && success;
        ++i;
    }
    return success;
}

/** Loads a value list from the attributes and the valued children of an object child, when there is one */
bool
LoadValueList( const CDataNode& parent, const char* name, CValueList& values )
{
    values.Clear();
    const CDataNode* node = parent.FindChild( name );
    if ( GUCEF_NULL == node )
        return true;
    CDataNode::TAttributeMap::const_iterator a = node->AttributeBegin();
    while ( a != node->AttributeEnd() )
    {
        values.Delete( (*a).first );
        values.Set( (*a).first, (*a).second );
        ++a;
    }
    CDataNode::const_iterator n = node->ConstBegin();
    while ( n != node->ConstEnd() )
    {
        if ( (*n)->HasValue() )
        {
            values.Delete( (*n)->GetName() );
            values.Set( (*n)->GetName(), (*n)->GetValue() );
        }
        ++n;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableWriterOptions::CDataTableWriterOptions( void )
    : CIDataNodeSerializable()
    , compression( "none" )
    , compressionLevel( DefaultCompressionLevel )
    , rowGroupRowCount( 1048576 )
    , rowGroupByteLimit( 512ULL * 1024 * 1024 )
    , pageRowCount( 16384 )
    , pageByteLimit( 1024 * 1024 )
    , dictionary( DATATABLE_DICTIONARY_AUTO )
    , dictionaryByteLimit( 1024 * 1024 )
    , writeStatistics( true )
    , writePageStatistics( true )
    , statisticsMaxValueBytes( 64 )
    , keyValueMetaData()
    , codecOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableWriterOptions::CDataTableWriterOptions( const CDataTableWriterOptions& src )
    : CIDataNodeSerializable( src )
    , compression( src.compression )
    , compressionLevel( src.compressionLevel )
    , rowGroupRowCount( src.rowGroupRowCount )
    , rowGroupByteLimit( src.rowGroupByteLimit )
    , pageRowCount( src.pageRowCount )
    , pageByteLimit( src.pageByteLimit )
    , dictionary( src.dictionary )
    , dictionaryByteLimit( src.dictionaryByteLimit )
    , writeStatistics( src.writeStatistics )
    , writePageStatistics( src.writePageStatistics )
    , statisticsMaxValueBytes( src.statisticsMaxValueBytes )
    , keyValueMetaData( src.keyValueMetaData )
    , codecOptions( src.codecOptions )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableWriterOptions::~CDataTableWriterOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableWriterOptions&
CDataTableWriterOptions::operator=( const CDataTableWriterOptions& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIDataNodeSerializable::operator=( src );
        compression = src.compression;
        compressionLevel = src.compressionLevel;
        rowGroupRowCount = src.rowGroupRowCount;
        rowGroupByteLimit = src.rowGroupByteLimit;
        pageRowCount = src.pageRowCount;
        pageByteLimit = src.pageByteLimit;
        dictionary = src.dictionary;
        dictionaryByteLimit = src.dictionaryByteLimit;
        writeStatistics = src.writeStatistics;
        writePageStatistics = src.writePageStatistics;
        statisticsMaxValueBytes = src.statisticsMaxValueBytes;
        keyValueMetaData = src.keyValueMetaData;
        codecOptions = src.codecOptions;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableWriterOptions::operator==( const CDataTableWriterOptions& other ) const
{GUCEF_TRACE;

    return compression == other.compression                             &&
           compressionLevel == other.compressionLevel                   &&
           rowGroupRowCount == other.rowGroupRowCount                   &&
           rowGroupByteLimit == other.rowGroupByteLimit                 &&
           pageRowCount == other.pageRowCount                           &&
           pageByteLimit == other.pageByteLimit                         &&
           dictionary == other.dictionary                               &&
           dictionaryByteLimit == other.dictionaryByteLimit             &&
           writeStatistics == other.writeStatistics                     &&
           writePageStatistics == other.writePageStatistics             &&
           statisticsMaxValueBytes == other.statisticsMaxValueBytes     &&
           ValueListsAreEqual( keyValueMetaData, other.keyValueMetaData ) &&
           ValueListsAreEqual( codecOptions, other.codecOptions );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableWriterOptions::operator!=( const CDataTableWriterOptions& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableWriterOptions::Validate( CString& error ) const
{GUCEF_TRACE;

    if ( compression.IsNULLOrEmpty() )
    {
        error = "No compression is named, use \"none\" for none";
        return false;
    }
    if ( 0 == rowGroupRowCount || 0 == rowGroupByteLimit || 0 == pageRowCount || 0 == pageByteLimit )
    {
        error = "Row group and page sizes must be above zero";
        return false;
    }
    if ( pageRowCount > rowGroupRowCount )
    {
        error = "A page cannot hold more rows than its row group";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableWriterOptions::Serialize( CDataNode& domRootNode                        ,
                                    const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "compression", compression ) && success;
    if ( DefaultCompressionLevel != compressionLevel )
        success = domRootNode.SetAttribute( "compressionLevel", compressionLevel ) && success;
    success = domRootNode.SetAttribute( "rowGroupRowCount", rowGroupRowCount ) && success;
    success = domRootNode.SetAttribute( "rowGroupByteLimit", rowGroupByteLimit ) && success;
    success = domRootNode.SetAttribute( "pageRowCount", pageRowCount ) && success;
    success = domRootNode.SetAttribute( "pageByteLimit", pageByteLimit ) && success;
    success = domRootNode.SetAttribute( "dictionary", CString( DataTableDictionaryModeToString( dictionary ) ) ) && success;
    success = domRootNode.SetAttribute( "dictionaryByteLimit", dictionaryByteLimit ) && success;
    success = domRootNode.SetAttribute( "writeStatistics", writeStatistics ) && success;
    success = domRootNode.SetAttribute( "writePageStatistics", writePageStatistics ) && success;
    success = domRootNode.SetAttribute( "statisticsMaxValueBytes", statisticsMaxValueBytes ) && success;
    success = SaveValueList( domRootNode, "keyValueMetaData", keyValueMetaData ) && success;
    success = SaveValueList( domRootNode, "codecOptions", codecOptions ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableWriterOptions::Deserialize( const CDataNode& domRootNode                  ,
                                      const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CDataTableWriterOptions options;
    options.compression = domRootNode.GetAttributeValueOrChildValueByName( "compression" ).AsString( options.compression, true );
    options.compressionLevel = domRootNode.GetAttributeValueOrChildValueByName( "compressionLevel" ).AsInt32( options.compressionLevel, true );
    options.rowGroupRowCount = domRootNode.GetAttributeValueOrChildValueByName( "rowGroupRowCount" ).AsUInt32( options.rowGroupRowCount, true );
    options.rowGroupByteLimit = domRootNode.GetAttributeValueOrChildValueByName( "rowGroupByteLimit" ).AsUInt64( options.rowGroupByteLimit, true );
    options.pageRowCount = domRootNode.GetAttributeValueOrChildValueByName( "pageRowCount" ).AsUInt32( options.pageRowCount, true );
    options.pageByteLimit = domRootNode.GetAttributeValueOrChildValueByName( "pageByteLimit" ).AsUInt32( options.pageByteLimit, true );
    CString dictionaryText = domRootNode.GetAttributeValueOrChildValueByName( "dictionary" ).AsString( CString::Empty, true );
    if ( !dictionaryText.IsNULLOrEmpty() && !DataTableDictionaryModeFromString( dictionaryText, options.dictionary ) )
        return false;
    options.dictionaryByteLimit = domRootNode.GetAttributeValueOrChildValueByName( "dictionaryByteLimit" ).AsUInt32( options.dictionaryByteLimit, true );
    options.writeStatistics = domRootNode.GetAttributeValueOrChildValueByName( "writeStatistics" ).AsBool( options.writeStatistics, true );
    options.writePageStatistics = domRootNode.GetAttributeValueOrChildValueByName( "writePageStatistics" ).AsBool( options.writePageStatistics, true );
    options.statisticsMaxValueBytes = domRootNode.GetAttributeValueOrChildValueByName( "statisticsMaxValueBytes" ).AsUInt32( options.statisticsMaxValueBytes, true );
    if ( !LoadValueList( domRootNode, "keyValueMetaData", options.keyValueMetaData ) ||
         !LoadValueList( domRootNode, "codecOptions", options.codecOptions ) )
        return false;

    *this = options;
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableWriterOptions::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableWriterOptions( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableWriterOptions::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTableWriterOptions";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
