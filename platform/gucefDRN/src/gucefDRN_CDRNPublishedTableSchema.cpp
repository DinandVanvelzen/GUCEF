/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

#include "gucefDRN_CDRNPublishedTableSchema.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CDRNPublishedTableSchema::MaxColumnCount;

CDRNPublishedTableSchema::CDRNPublishedTableSchema( void )
    : columns()
    , keyColumn( 0 )
    , defaultSortColumn( -1 )
    , defaultSortAscending( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTableSchema::IsValid( CORE::CString& error ) const
{GUCEF_TRACE;

    if ( columns.empty() || keyColumn >= columns.size() )
    {
        error = "a published table needs columns and a key column among them";
        return false;
    }
    if ( columns.size() > MaxColumnCount )
    {
        error = "a published table has at most " + CORE::ToString( MaxColumnCount ) + " columns";
        return false;
    }
    if ( defaultSortColumn >= (Int32) columns.size() )
    {
        error = "the default sort column does not exist";
        return false;
    }
    for ( UInt32 i=0; i<columns.size(); ++i )
    {
        if ( !columns[ i ].def.IsValid( error ) )
            return false;
        if ( columns[ i ].id.IsNULLOrEmpty() )
        {
            error = "column " + columns[ i ].def.name + " has no id";
            return false;
        }
        for ( UInt32 n=i+1; n<columns.size(); ++n )
        {
            if ( columns[ n ].id == columns[ i ].id || columns[ n ].def.name == columns[ i ].def.name )
            {
                error = "the column " + columns[ i ].def.name + " (" + columns[ i ].id + ") is there twice";
                return false;
            }
        }
    }
    if ( columns[ keyColumn ].def.nullable )
    {
        error = "the key column cannot be nullable";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CDRNPublishedTableSchema::FindColumnById( const CORE::CString& columnId ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<columns.size(); ++i )
        if ( columns[ i ].id == columnId )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTableSchema::Encode( CORE::CVariantStream& stream ) const
{GUCEF_TRACE;

    stream << EncodingVersion << (UInt32) columns.size() << keyColumn << defaultSortColumn << defaultSortAscending;
    for ( UInt32 i=0; i<columns.size(); ++i )
        columns[ i ].Encode( stream );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTableSchema::Decode( CORE::CVariantStream& stream, bool& unsupported )
{GUCEF_TRACE;

    unsupported = false;
    UInt8 version = 0;
    UInt32 columnCount = 0;
    stream >> version;
    if ( !stream.IsValid() )
        return false;
    if ( version > EncodingVersion )
    {
        unsupported = true;
        return false;
    }
    stream >> columnCount >> keyColumn >> defaultSortColumn >> defaultSortAscending;
    if ( !stream.IsValid() || 0 == columnCount || columnCount > MaxColumnCount || keyColumn >= columnCount )
        return false;
    columns.clear();
    columns.resize( columnCount );
    for ( UInt32 i=0; i<columnCount; ++i )
        if ( !columns[ i ].Decode( stream ) )
            return false;

    // what arrives is held to the same rules as what is published
    CORE::CString error;
    return IsValid( error );    // values a later version appends are left unread
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTableSchema::ToDataTableSchema( CORE::CDataTableSchema& schema ) const
{GUCEF_TRACE;

    schema = CORE::CDataTableSchema();
    if ( keyColumn >= columns.size() || !schema.AddColumn( columns[ keyColumn ].def ) )
        return false;
    for ( UInt32 i=0; i<columns.size(); ++i )
        if ( i != keyColumn && !schema.AddColumn( columns[ i ].def ) )
            return false;
    CORE::CString::StringVector primaryKey;
    primaryKey.push_back( columns[ keyColumn ].def.name );
    return schema.SetPrimaryKey( primaryKey );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPublishedTableSchema::ToDataTableColumnIndex( UInt32 publishedIndex ) const
{GUCEF_TRACE;

    if ( publishedIndex == keyColumn )
        return 0;
    return publishedIndex < keyColumn ? publishedIndex + 1 : publishedIndex;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTableSchema::operator==( const CDRNPublishedTableSchema& other ) const
{GUCEF_TRACE;

    if ( keyColumn != other.keyColumn || defaultSortColumn != other.defaultSortColumn || defaultSortAscending != other.defaultSortAscending ||
         columns.size() != other.columns.size() )
        return false;
    for ( UInt32 i=0; i<columns.size(); ++i )
        if ( columns[ i ] != other.columns[ i ] )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTableSchema::operator!=( const CDRNPublishedTableSchema& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
