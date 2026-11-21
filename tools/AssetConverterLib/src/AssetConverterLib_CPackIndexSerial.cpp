/*
 *  AssetConverterLib: the read and write helpers the pack index classes share (absent members are unknown, never zero)
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

#include <algorithm>

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#include "AssetConverterLib_CContentHash.h"
#define ASSETCONVERTERLIB_CCONTENTHASH_H
#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */

#include "AssetConverterLib_CPackIndexSerial.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPackIndexSerial::SetText( CORE::CDataNode& node, const char* name, const CORE::CString& value )
{GUCEF_TRACE;

    if ( !value.IsNULLOrEmpty() )
        node.SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

void
CPackIndexSerial::SetTextAlways( CORE::CDataNode& node, const char* name, const CORE::CString& value )
{GUCEF_TRACE;

    node.SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

void
CPackIndexSerial::SetCount( CORE::CDataNode& node, const char* name, UInt64 value )
{GUCEF_TRACE;

    if ( 0 != value )
        node.SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

void
CPackIndexSerial::SetFlag( CORE::CDataNode& node, const char* name, bool value )
{GUCEF_TRACE;

    if ( value )
        node.SetAttribute( name, true );
}

/*-------------------------------------------------------------------------*/

void
CPackIndexSerial::SetList( CORE::CDataNode& node, const char* name, const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    if ( !values.empty() )
        node.SetValuesOfChildByName( name, values );
}

/*-------------------------------------------------------------------------*/

void
CPackIndexSerial::SetSortedList( CORE::CDataNode& node, const char* name, const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    CORE::CString::StringVector sorted = values;
    std::sort( sorted.begin(), sorted.end() );
    sorted.erase( std::unique( sorted.begin(), sorted.end() ), sorted.end() );
    SetList( node, name, sorted );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexSerial::GetText( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    return node.GetAttributeValueOrChildValueByName( name ).AsString();
}

/*-------------------------------------------------------------------------*/

UInt64
CPackIndexSerial::GetCount( const CORE::CDataNode& node, const char* name, UInt64 defaultValue )
{GUCEF_TRACE;

    return node.GetAttributeValueOrChildValueByName( name ).AsUInt64( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

Int64
CPackIndexSerial::GetInt( const CORE::CDataNode& node, const char* name, Int64 defaultValue )
{GUCEF_TRACE;

    return node.GetAttributeValueOrChildValueByName( name ).AsInt64( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexSerial::GetFlag( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    return node.GetAttributeValueOrChildValueByName( name ).AsBool( false, true );
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexSerial::HasMember( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    return node.IsAttribute( name ) || GUCEF_NULL != node.FindChild( name );
}

/*-------------------------------------------------------------------------*/

void
CPackIndexSerial::GetList( const CORE::CDataNode& node, const char* name, CORE::CString::StringVector& values )
{GUCEF_TRACE;

    values.clear();
    node.GetValuesOfChildByName( name, values );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexSerial::HashParts( const CORE::CString::StringVector& parts )
{GUCEF_TRACE;

    // a line feed ends each part: ( "ab", "c" ) and ( "a", "bc" ) never give the same key
    CORE::CString text;
    for ( size_t i=0; i<parts.size(); ++i )
    {
        text += parts[ i ];
        text += '\n';
    }
    return CContentHash::HashText( text );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
