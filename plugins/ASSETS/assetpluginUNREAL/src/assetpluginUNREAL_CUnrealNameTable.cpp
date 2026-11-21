/*
 *  assetpluginUNREAL: The name table of a package
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#include "assetpluginUNREAL_CUnrealNameTable.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealNameTable::CUnrealNameTable( void )
    : m_names()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealNameTable::Clear( void )
{GUCEF_TRACE;

    m_names.clear();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealNameTable::Read( CUnrealArchiveReader& reader, UInt32 count, const CUnrealReaderLimits& limits )
{GUCEF_TRACE;

    m_names.clear();
    if ( count > limits.maxTableEntries )
        return reader.SetError( "Name count " + CORE::UInt32ToString( count ) + " exceeds the limit" );
    if ( !reader.CheckCount( count, 8, "names" ) )
        return false;
    m_names.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString name;
        if ( !reader.ReadString( name, limits.maxNameLength ) )
            return false;
        reader.ReadUInt32(); // the two hashes of the name, not needed
        if ( reader.HasError() )
            return false;
        m_names.push_back( name );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealNameTable::Add( const CORE::CString& name )
{GUCEF_TRACE;

    m_names.push_back( name );
    return (UInt32) ( m_names.size() - 1 );
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealNameTable::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_names.size();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealNameTable::Get( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_names.size() ? m_names[ index ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealNameTable::Find( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_names.size(); ++i )
        if ( m_names[ i ] == name )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealNameTable::ReadNameRef( CUnrealArchiveReader& reader, UInt32& index, Int32& number ) const
{GUCEF_TRACE;

    index = reader.ReadUInt32();
    number = reader.ReadInt32();
    if ( reader.HasError() )
        return false;
    if ( index >= m_names.size() )
        return reader.SetError( "Name index " + CORE::UInt32ToString( index ) + " is outside the name table of " + CORE::UInt64ToString( m_names.size() ) + " names" );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealNameTable::Format( UInt32 index, Int32 number ) const
{GUCEF_TRACE;

    if ( number > 0 )
        return Get( index ) + "_" + CORE::Int32ToString( number - 1 );
    return Get( index );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealNameTable::ReadName( CUnrealArchiveReader& reader, CORE::CString& name ) const
{GUCEF_TRACE;

    UInt32 index = 0;
    Int32 number = 0;
    if ( !ReadNameRef( reader, index, number ) )
    {
        name.Clear();
        return false;
    }
    name = Format( index, number );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
