/*
 *  assetpluginUNREAL: The type of a property as a tree of names
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H
#include "assetpluginUNREAL_CUnrealPropertyTypeName.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H ? */

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

CUnrealPropertyTypeName::CUnrealPropertyTypeName( void )
    : m_nodes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealPropertyTypeName::Clear( void )
{GUCEF_TRACE;

    m_nodes.clear();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyTypeName::Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, UInt32 maxNodes )
{GUCEF_TRACE;

    m_nodes.clear();
    Int64 remaining = 1;
    do
    {
        SNode node;
        if ( !names.ReadName( reader, node.name ) )
            return false;
        const Int32 inner = reader.ReadInt32();
        if ( reader.HasError() )
            return false;
        if ( inner < 0 || (UInt32) inner > maxNodes || m_nodes.size() >= maxNodes )
            return reader.SetError( "A property type name has a parameter count or a size that is not valid" );
        node.innerCount = (UInt32) inner;
        m_nodes.push_back( node );
        remaining += (Int64) inner - 1;
    }
    while ( remaining > 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUnrealPropertyTypeName::SetSimple( const CORE::CString& name )
{GUCEF_TRACE;

    m_nodes.clear();
    AddNode( name, 0 );
}

/*-------------------------------------------------------------------------*/

void
CUnrealPropertyTypeName::AddNode( const CORE::CString& name, UInt32 innerCount )
{GUCEF_TRACE;

    SNode node;
    node.name = name;
    node.innerCount = innerCount;
    m_nodes.push_back( node );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyTypeName::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_nodes.empty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealPropertyTypeName::GetName( void ) const
{GUCEF_TRACE;

    return m_nodes.empty() ? CORE::CString::Empty : m_nodes[ 0 ].name;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealPropertyTypeName::GetParameterCount( void ) const
{GUCEF_TRACE;

    return m_nodes.empty() ? 0 : m_nodes[ 0 ].innerCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealPropertyTypeName::FindParameterStart( UInt32 i ) const
{GUCEF_TRACE;

    // the parameters follow the first node in order; skipping one means skipping its whole subtree
    UInt32 position = 1;
    for ( UInt32 skip=i; skip>0; --skip )
    {
        Int64 remaining = 1;
        while ( remaining > 0 && position < m_nodes.size() )
        {
            remaining += (Int64) m_nodes[ position ].innerCount - 1;
            ++position;
        }
    }
    return position;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealPropertyTypeName::GetParameterName( UInt32 i ) const
{GUCEF_TRACE;

    if ( i >= GetParameterCount() )
        return CORE::CString::Empty;
    const UInt32 position = FindParameterStart( i );
    return position < m_nodes.size() ? m_nodes[ position ].name : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CUnrealPropertyTypeName
CUnrealPropertyTypeName::GetParameter( UInt32 i ) const
{GUCEF_TRACE;

    CUnrealPropertyTypeName result;
    if ( i >= GetParameterCount() )
        return result;
    UInt32 position = FindParameterStart( i );
    Int64 remaining = 1;
    while ( remaining > 0 && position < m_nodes.size() )
    {
        result.m_nodes.push_back( m_nodes[ position ] );
        remaining += (Int64) m_nodes[ position ].innerCount - 1;
        ++position;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealPropertyTypeName::ToString( void ) const
{GUCEF_TRACE;

    if ( m_nodes.empty() )
        return CORE::CString();
    CORE::CString text = m_nodes[ 0 ].name;
    const UInt32 count = m_nodes[ 0 ].innerCount;
    if ( count > 0 )
    {
        text += "(";
        for ( UInt32 i=0; i<count; ++i )
        {
            if ( i > 0 )
                text += ",";
            text += GetParameter( i ).ToString();
        }
        text += ")";
    }
    return text;
}

/*-------------------------------------------------------------------------*/

const CUnrealPropertyTypeName::TNodeVector&
CUnrealPropertyTypeName::GetNodes( void ) const
{GUCEF_TRACE;

    return m_nodes;
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
