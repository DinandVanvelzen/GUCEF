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

#include "gucefCORE_CDataTableIndexDef.h"

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

CDataTableIndexDef::CDataTableIndexDef( void )
    : CIDataNodeSerializable()
    , name()
    , columns()
    , unique( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableIndexDef::CDataTableIndexDef( const CString& indexName, const CString::StringVector& indexColumns, bool isUnique )
    : CIDataNodeSerializable()
    , name( indexName )
    , columns( indexColumns )
    , unique( isUnique )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableIndexDef::CDataTableIndexDef( const CDataTableIndexDef& src )
    : CIDataNodeSerializable( src )
    , name( src.name )
    , columns( src.columns )
    , unique( src.unique )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableIndexDef::~CDataTableIndexDef()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableIndexDef&
CDataTableIndexDef::operator=( const CDataTableIndexDef& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIDataNodeSerializable::operator=( src );
        name = src.name;
        columns = src.columns;
        unique = src.unique;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableIndexDef::operator==( const CDataTableIndexDef& other ) const
{GUCEF_TRACE;

    return name == other.name && columns == other.columns && unique == other.unique;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableIndexDef::operator!=( const CDataTableIndexDef& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CDataTableIndexDef::IsValidIndexName( const CString& indexName )
{GUCEF_TRACE;

    if ( indexName.IsNULLOrEmpty() )
        return false;
    const CUtf8String utf8 = ToUtf8String( indexName );
    const char* bytes = utf8.C_String();
    const UInt32 length = utf8.ByteSize() - 1;
    if ( 0 == length || length > MaxNameLength )
        return false;
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = bytes[ i ];
        const bool letter = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || '_' == c;
        const bool digit = c >= '0' && c <= '9';
        if ( !letter && !( digit && i > 0 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableIndexDef::Validate( const CDataTableSchema& schema, CString& error ) const
{GUCEF_TRACE;

    if ( !IsValidIndexName( name ) )
    {
        error = "Index name \"" + name + "\" is not usable: [A-Za-z0-9_] only, starting with a letter or '_', at most " + ToString( MaxNameLength ) + " characters";
        return false;
    }
    if ( columns.empty() )
    {
        error = "Index \"" + name + "\" has no columns";
        return false;
    }
    for ( UInt32 i=0; i<(UInt32) columns.size(); ++i )
    {
        if ( schema.FindColumn( columns[ i ] ) < 0 )
        {
            error = "Index \"" + name + "\" names column \"" + columns[ i ] + "\", which the schema does not have";
            return false;
        }
        for ( UInt32 n=0; n<i; ++n )
        {
            if ( columns[ n ] == columns[ i ] )
            {
                error = "Index \"" + name + "\" names column \"" + columns[ i ] + "\" twice";
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableIndexDef::Serialize( CDataNode& domRootNode                        ,
                               const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "name", name );
    success = domRootNode.SetValuesOfChildByName( "columns", columns ) && success;
    if ( unique )
        success = domRootNode.SetAttribute( "unique", true ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableIndexDef::Deserialize( const CDataNode& domRootNode                  ,
                                 const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CDataTableIndexDef index;
    index.name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( CString::Empty, true );
    if ( !domRootNode.GetValuesOfChildByName( "columns", index.columns ) || index.columns.empty() )
        return false;
    index.unique = domRootNode.GetAttributeValueOrChildValueByName( "unique" ).AsBool( false, true );
    if ( !IsValidIndexName( index.name ) )
        return false;
    *this = index;
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableIndexDef::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableIndexDef( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableIndexDef::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTableIndexDef";
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
