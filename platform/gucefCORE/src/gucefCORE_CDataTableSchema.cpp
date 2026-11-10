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

#include "gucefCORE_CDataTableSchema.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableSchema::CDataTableSchema( void )
    : CIDataNodeSerializable()
    , m_columns()
    , m_primaryKey()
    , m_sortKey()
    , m_schemaVersion( 1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableSchema::CDataTableSchema( const CDataTableSchema& src )
    : CIDataNodeSerializable( src )
    , m_columns( src.m_columns )
    , m_primaryKey( src.m_primaryKey )
    , m_sortKey( src.m_sortKey )
    , m_schemaVersion( src.m_schemaVersion )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableSchema::~CDataTableSchema()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableSchema&
CDataTableSchema::operator=( const CDataTableSchema& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIDataNodeSerializable::operator=( src );
        m_columns = src.m_columns;
        m_primaryKey = src.m_primaryKey;
        m_sortKey = src.m_sortKey;
        m_schemaVersion = src.m_schemaVersion;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::operator==( const CDataTableSchema& other ) const
{GUCEF_TRACE;

    return m_columns == other.m_columns       &&
           m_primaryKey == other.m_primaryKey &&
           m_sortKey == other.m_sortKey       &&
           m_schemaVersion == other.m_schemaVersion;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::operator!=( const CDataTableSchema& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::AddColumn( const CDataTableColumnDef& column )
{GUCEF_TRACE;

    // the file schema of a codec lists the columns it cannot decode as UNSUPPORTED (they are never projected);
    // Validate() still refuses them, so a table definition never holds one
    CString error;
    if ( DATATABLE_TYPE_UNSUPPORTED == column.type ? !CDataTableColumnDef::IsValidName( column.name ) : !column.IsValid( error ) )
        return false;
    if ( FindColumn( column.name ) >= 0 )
        return false;
    m_columns.push_back( column );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableSchema::GetColumnCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_columns.size();
}

/*-------------------------------------------------------------------------*/

const CDataTableColumnDef&
CDataTableSchema::GetColumn( UInt32 index ) const
{GUCEF_TRACE;

    return m_columns[ index ];
}

/*-------------------------------------------------------------------------*/

const CDataTableSchema::TColumnDefVector&
CDataTableSchema::GetColumns( void ) const
{GUCEF_TRACE;

    return m_columns;
}

/*-------------------------------------------------------------------------*/

Int32
CDataTableSchema::FindColumn( const CString& name ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_columns.size(); ++i )
    {
        if ( m_columns[ i ].name == name )
            return (Int32) i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::ResolveColumnNames( const CString::StringVector& columnNames, TColumnIndexVector& indices ) const
{GUCEF_TRACE;

    TColumnIndexVector result;
    result.reserve( columnNames.size() );
    for ( UInt32 i=0; i<(UInt32) columnNames.size(); ++i )
    {
        Int32 index = FindColumn( columnNames[ i ] );
        if ( index < 0 )
            return false;
        for ( UInt32 n=0; n<(UInt32) result.size(); ++n )
        {
            if ( result[ n ] == (UInt32) index )
                return false;
        }
        result.push_back( (UInt32) index );
    }
    indices = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::SetPrimaryKey( const CString::StringVector& columnNames )
{GUCEF_TRACE;

    return ResolveColumnNames( columnNames, m_primaryKey );
}

/*-------------------------------------------------------------------------*/

const CDataTableSchema::TColumnIndexVector&
CDataTableSchema::GetPrimaryKey( void ) const
{GUCEF_TRACE;

    return m_primaryKey;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::SetSortKey( const CString::StringVector& columnNames )
{GUCEF_TRACE;

    return ResolveColumnNames( columnNames, m_sortKey );
}

/*-------------------------------------------------------------------------*/

const CDataTableSchema::TColumnIndexVector&
CDataTableSchema::GetSortKey( void ) const
{GUCEF_TRACE;

    return m_sortKey;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableSchema::GetSchemaVersion( void ) const
{GUCEF_TRACE;

    return m_schemaVersion;
}

/*-------------------------------------------------------------------------*/

void
CDataTableSchema::SetSchemaVersion( UInt32 version )
{GUCEF_TRACE;

    m_schemaVersion = version;
}

/*-------------------------------------------------------------------------*/

void
CDataTableSchema::Clear( void )
{GUCEF_TRACE;

    m_columns.clear();
    m_primaryKey.clear();
    m_sortKey.clear();
    m_schemaVersion = 1;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::Validate( CString& error ) const
{GUCEF_TRACE;

    if ( m_columns.empty() )
    {
        error = "The schema has no columns";
        return false;
    }
    for ( UInt32 i=0; i<(UInt32) m_columns.size(); ++i )
    {
        if ( !m_columns[ i ].IsValid( error ) )
            return false;
        for ( UInt32 n=0; n<i; ++n )
        {
            if ( m_columns[ n ].name == m_columns[ i ].name )
            {
                error = "Column \"" + m_columns[ i ].name + "\" appears more than once";
                return false;
            }
        }
    }

    const TColumnIndexVector* keys[ 2 ] = { &m_primaryKey, &m_sortKey };
    for ( UInt32 k=0; k<2; ++k )
    {
        const TColumnIndexVector& key = *keys[ k ];
        for ( UInt32 i=0; i<(UInt32) key.size(); ++i )
        {
            if ( key[ i ] >= (UInt32) m_columns.size() )
            {
                error = 0 == k ? "The primary key refers to a column that does not exist" : "The sort key refers to a column that does not exist";
                return false;
            }
            for ( UInt32 n=0; n<i; ++n )
            {
                if ( key[ n ] == key[ i ] )
                {
                    error = "Column \"" + m_columns[ key[ i ] ].name + "\" appears more than once in a key";
                    return false;
                }
            }
        }
    }

    for ( UInt32 i=0; i<(UInt32) m_primaryKey.size(); ++i )
    {
        if ( m_columns[ m_primaryKey[ i ] ].nullable )
        {
            error = "Primary key column \"" + m_columns[ m_primaryKey[ i ] ].name + "\" is nullable";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::IsReadableAs( const CDataTableSchema& fileSchema, CString& error ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_columns.size(); ++i )
    {
        const CDataTableColumnDef& tableColumn = m_columns[ i ];
        Int32 fileIndex = fileSchema.FindColumn( tableColumn.name );
        if ( fileIndex < 0 )
        {
            if ( !tableColumn.nullable )
            {
                error = "The file lacks column \"" + tableColumn.name + "\" which is not nullable in the table";
                return false;
            }
            continue;
        }

        const CDataTableColumnDef& fileColumn = fileSchema.GetColumn( (UInt32) fileIndex );
        if ( fileColumn.nullable && !tableColumn.nullable )
        {
            error = "Column \"" + tableColumn.name + "\" is nullable in the file but not in the table";
            return false;
        }
        if ( tableColumn.IsSameType( fileColumn ) )
            continue;

        bool widening = ( DATATABLE_TYPE_INT64 == tableColumn.type && DATATABLE_TYPE_INT32 == fileColumn.type ) ||
                        ( DATATABLE_TYPE_FLOAT64 == tableColumn.type && DATATABLE_TYPE_FLOAT32 == fileColumn.type );
        if ( !widening && DATATABLE_TYPE_DECIMAL == tableColumn.type && DATATABLE_TYPE_DECIMAL == fileColumn.type )
        {
            // a lower scale is rescaled exactly, provided the integer digits of the file fit the table column
            widening = fileColumn.decimalScale < tableColumn.decimalScale &&
                       ( fileColumn.decimalPrecision - fileColumn.decimalScale ) <= ( tableColumn.decimalPrecision - tableColumn.decimalScale );
        }
        if ( !widening )
        {
            error = "Column \"" + tableColumn.name + "\" is " + fileColumn.GetTypeText() + " in the file and " + tableColumn.GetTypeText() + " in the table";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::Serialize( CDataNode& domRootNode                        ,
                             const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "schemaVersion", m_schemaVersion );

    CDataNode* columnsNode = domRootNode.AddChild( "columns", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == columnsNode )
        return false;
    for ( UInt32 i=0; i<(UInt32) m_columns.size(); ++i )
    {
        CDataNode* columnNode = columnsNode->AddChild( "column", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == columnNode )
            return false;
        success = m_columns[ i ].Serialize( *columnNode, settings ) && success;
    }

    const TColumnIndexVector* keys[ 2 ] = { &m_primaryKey, &m_sortKey };
    const char* keyNames[ 2 ] = { "primaryKey", "sortKey" };
    for ( UInt32 k=0; k<2; ++k )
    {
        if ( keys[ k ]->empty() )
            continue;
        CString::StringVector names;
        for ( UInt32 i=0; i<(UInt32) keys[ k ]->size(); ++i )
            names.push_back( m_columns[ (*keys[ k ])[ i ] ].name );
        success = domRootNode.SetValuesOfChildByName( keyNames[ k ], names ) && success;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableSchema::Deserialize( const CDataNode& domRootNode                  ,
                               const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CDataTableSchema schema;
    schema.m_schemaVersion = domRootNode.GetAttributeValueOrChildValueByName( "schemaVersion" ).AsUInt32( 1, true );

    const CDataNode* columnsNode = domRootNode.FindChild( "columns" );
    if ( GUCEF_NULL == columnsNode )
        return false;
    CDataNode::const_iterator i = columnsNode->ConstBegin();
    while ( i != columnsNode->ConstEnd() )
    {
        CDataTableColumnDef column;
        if ( !column.Deserialize( *(*i), settings ) || !schema.AddColumn( column ) )
            return false;
        ++i;
    }

    CString::StringVector names;
    if ( domRootNode.GetValuesOfChildByName( "primaryKey", names ) && !schema.SetPrimaryKey( names ) )
        return false;
    names.clear();
    if ( domRootNode.GetValuesOfChildByName( "sortKey", names ) && !schema.SetSortKey( names ) )
        return false;

    *this = schema;
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableSchema::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableSchema( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableSchema::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTableSchema";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CDataTableSchemaPtr
CreateDataTableSchemaPtr( const CDataTableSchema& schema )
{GUCEF_TRACE;

    // made as a pointer to a mutable schema, the destructor of that type does the release, then shared as immutable
    CTSharedPtr< CDataTableSchema, MT::CMutex > copy( GUCEF_NEW CDataTableSchema( schema ) );
    return CDataTableSchemaPtr( copy );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
