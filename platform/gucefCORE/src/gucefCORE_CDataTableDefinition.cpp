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

#include "gucefCORE_CDataTableDefinition.h"

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

inline char
UpperAscii( char c )
{
    return ( c >= 'a' && c <= 'z' ) ? (char) ( c - 'a' + 'A' ) : c;
}

/**
 *  True when the part of the name before its first '.' is, ignoring case, a device name of Windows: CON, PRN, AUX, NUL,
 *  COM0 .. COM9, LPT0 .. LPT9. Windows opens the device for such a file name, with any extension.
 */
bool
IsWindowsDeviceName( const char* bytes, UInt32 length )
{
    UInt32 stem = 0;
    while ( stem < length && '.' != bytes[ stem ] )
        ++stem;
    static const char* const names[] = { "CON", "PRN", "AUX", "NUL" };
    if ( 3 == stem )
    {
        for ( UInt32 i=0; i<4; ++i )
        {
            if ( UpperAscii( bytes[ 0 ] ) == names[ i ][ 0 ] && UpperAscii( bytes[ 1 ] ) == names[ i ][ 1 ] && UpperAscii( bytes[ 2 ] ) == names[ i ][ 2 ] )
                return true;
        }
        return false;
    }
    if ( 4 == stem && bytes[ 3 ] >= '0' && bytes[ 3 ] <= '9' )
    {
        const char a = UpperAscii( bytes[ 0 ] ), b = UpperAscii( bytes[ 1 ] ), c = UpperAscii( bytes[ 2 ] );
        return ( 'C' == a && 'O' == b && 'M' == c ) || ( 'L' == a && 'P' == b && 'T' == c );
    }
    return false;
}

bool
BackendOptionsAreEqual( const CValueList& a, const CValueList& b )
{
    if ( a.GetKeyCount() != b.GetKeyCount() )
        return false;
    CValueList::TValueMap::const_iterator i = a.GetDataBeginIterator();
    CValueList::TValueMap::const_iterator n = b.GetDataBeginIterator();
    while ( i != a.GetDataEndIterator() && n != b.GetDataEndIterator() )
    {
        if ( (*i).first != (*n).first || (*i).second != (*n).second )
            return false;
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

CDataTableDefinition::CDataTableDefinition( void )
    : CIDataNodeSerializable()
    , name()
    , schema()
    , partitioning()
    , codecType( "parquet" )
    , storage()
    , backendOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableDefinition::CDataTableDefinition( const CDataTableDefinition& src )
    : CIDataNodeSerializable( src )
    , name( src.name )
    , schema( src.schema )
    , partitioning( src.partitioning )
    , codecType( src.codecType )
    , storage( src.storage )
    , backendOptions( src.backendOptions )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableDefinition::~CDataTableDefinition()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableDefinition&
CDataTableDefinition::operator=( const CDataTableDefinition& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIDataNodeSerializable::operator=( src );
        name = src.name;
        schema = src.schema;
        partitioning = src.partitioning;
        codecType = src.codecType;
        storage = src.storage;
        backendOptions = src.backendOptions;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableDefinition::operator==( const CDataTableDefinition& other ) const
{GUCEF_TRACE;

    return name == other.name                   &&
           schema == other.schema               &&
           partitioning == other.partitioning   &&
           codecType == other.codecType         &&
           storage == other.storage             &&
           BackendOptionsAreEqual( backendOptions, other.backendOptions );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableDefinition::operator!=( const CDataTableDefinition& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableDefinition::IsValidTableName( const CString& tableName )
{GUCEF_TRACE;

    if ( tableName.IsNULLOrEmpty() )
        return false;
    CUtf8String utf8 = ToUtf8String( tableName );
    const char* bytes = utf8.C_String();
    const UInt32 length = utf8.ByteSize() - 1;
    // a trailing '.' is dropped by Windows ("x." and "x" are one file there), a device name opens the device
    if ( '.' == bytes[ 0 ] || '.' == bytes[ length - 1 ] || IsWindowsDeviceName( bytes, length ) )
        return false;
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = bytes[ i ];
        bool allowed = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '_' == c || '.' == c || '-' == c;
        if ( !allowed )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableDefinition::Validate( CString& error ) const
{GUCEF_TRACE;

    if ( !IsValidTableName( name ) )
    {
        error = "Table name \"" + name + "\" is not usable: [A-Za-z0-9_.-] only, not starting or ending with '.', not a device name of Windows (CON, PRN, AUX, NUL, COM0-9, LPT0-9, with or without an extension)";
        return false;
    }
    if ( codecType.IsNULLOrEmpty() )
    {
        error = "Table \"" + name + "\" names no codec";
        return false;
    }
    if ( !schema.Validate( error ) || !partitioning.Validate( schema, error ) || !storage.Validate( error ) )
    {
        error = "Table \"" + name + "\": " + error;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CDataTableSchemaPtr
CDataTableDefinition::CreateSchemaPtr( void ) const
{GUCEF_TRACE;

    return CreateDataTableSchemaPtr( schema );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableDefinition::Serialize( CDataNode& domRootNode                        ,
                                 const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    domRootNode.SetName( "DataTableDefinition" );
    domRootNode.SetNodeType( GUCEF_DATATYPE_OBJECT );
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "codec", codecType ) && success;
    success = schema.Serialize( domRootNode, settings ) && success;

    if ( partitioning.IsPartitioned() )
    {
        CDataNode* partitioningNode = domRootNode.AddChild( "partitioning", GUCEF_DATATYPE_OBJECT );
        success = GUCEF_NULL != partitioningNode && partitioning.Serialize( *partitioningNode, settings ) && success;
    }

    CDataNode* storageNode = domRootNode.AddChild( "storage", GUCEF_DATATYPE_OBJECT );
    success = GUCEF_NULL != storageNode && storage.Serialize( *storageNode, settings ) && success;

    success = SaveValueList( domRootNode, "backendOptions", backendOptions ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableDefinition::Deserialize( const CDataNode& domRootNode                  ,
                                   const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CDataTableDefinition definition;
    definition.name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( CString::Empty, true );
    definition.codecType = domRootNode.GetAttributeValueOrChildValueByName( "codec" ).AsString( definition.codecType, true );
    if ( !definition.schema.Deserialize( domRootNode, settings ) )
        return false;

    const CDataNode* partitioningNode = domRootNode.FindChild( "partitioning" );
    if ( GUCEF_NULL != partitioningNode && !definition.partitioning.Deserialize( *partitioningNode, settings ) )
        return false;

    const CDataNode* storageNode = domRootNode.FindChild( "storage" );
    if ( GUCEF_NULL != storageNode && !definition.storage.Deserialize( *storageNode, settings ) )
        return false;

    if ( !LoadValueList( domRootNode, "backendOptions", definition.backendOptions ) )
        return false;

    *this = definition;
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableDefinition::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableDefinition( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableDefinition::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTableDefinition";
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
