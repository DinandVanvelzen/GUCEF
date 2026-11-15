/*
 *  assetpluginGODOT: a parsed Godot file
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTTEXTREADER_H
#include "assetpluginGODOT_CGodotTextReader.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTTEXTREADER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTTEXTREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H
#include "assetpluginGODOT_CGodotBinaryReader.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "assetpluginGODOT_CGodotDocument.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGodotDocument::CGodotDocument( void )
    : m_root( "godot", GUCEF_DATATYPE_OBJECT )
    , m_assetPath()
    , m_problem()
    , m_loaded( false )
    , m_extResources()
    , m_subResources()
    , m_nodes()
    , m_main( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGodotDocument::~CGodotDocument()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGodotDocument::Load( const ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& assetPath )
{GUCEF_TRACE;

    m_loaded = false;
    m_assetPath = assetPath;
    CORE::CDynamicBuffer file;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( file, context.ToVfsPath( assetPath ), "rb" ) )
    {
        m_problem = assetPath + ": the file cannot be read";
        return false;
    }
    if ( CGodotBinaryReader::IsBinaryResource( file.GetConstBufferPtr(), file.GetDataSize() ) )
    {
        CGodotBinaryReader binaryReader;
        if ( !binaryReader.Parse( file.GetConstBufferPtr(), file.GetDataSize(), m_root ) )
        {
            m_problem = assetPath + ": " + binaryReader.GetProblem();
            return false;
        }
    }
    else
    {
        CGodotTextReader reader;
        if ( !reader.ParseData( static_cast< const char* >( file.GetConstBufferPtr() ), file.GetDataSize(), m_root ) )
        {
            m_problem = assetPath + ": " + reader.GetProblem();
            return false;
        }
    }
    BuildIndex();
    m_loaded = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotDocument::LoadText( const CORE::CString& text, const CORE::CString& assetPath )
{GUCEF_TRACE;

    m_loaded = false;
    m_assetPath = assetPath;
    CGodotTextReader reader;
    if ( !reader.Parse( text, m_root ) )
    {
        m_problem = assetPath + ": " + reader.GetProblem();
        return false;
    }
    BuildIndex();
    m_loaded = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGodotDocument::BuildIndex( void )
{GUCEF_TRACE;

    m_extResources.clear();
    m_subResources.clear();
    m_nodes.clear();
    m_main = GUCEF_NULL;
    for ( const CORE::CDataNode* section = m_root.GetFirstChild(); GUCEF_NULL != section; section = section->GetNext() )
    {
        const CORE::CString tag = section->GetAttributeValue( "tag" ).AsString();
        if ( "ext_resource" == tag )
            m_extResources[ section->GetAttributeValue( "id" ).AsString() ] = section;
        else
        if ( "sub_resource" == tag )
            m_subResources[ section->GetAttributeValue( "id" ).AsString() ] = section;
        else
        if ( "node" == tag )
            m_nodes.push_back( section );
        else
        if ( "resource" == tag && GUCEF_NULL == m_main )
            m_main = section;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotDocument::IsLoaded( void ) const
{GUCEF_TRACE;

    return m_loaded;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGodotDocument::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGodotDocument::GetAssetPath( void ) const
{GUCEF_TRACE;

    return m_assetPath;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CGodotDocument::GetRoot( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotDocument::GetKind( void ) const
{GUCEF_TRACE;

    return m_root.GetAttributeValue( "kind" ).AsString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotDocument::GetResourceType( void ) const
{GUCEF_TRACE;

    const CORE::CDataNode* header = FindSection( "gd_resource" );
    return GUCEF_NULL != header ? header->GetAttributeValue( "type" ).AsString() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotDocument::GetUid( void ) const
{GUCEF_TRACE;

    const CORE::CDataNode* header = m_root.GetFirstChild();
    return GUCEF_NULL != header ? header->GetAttributeValue( "uid" ).AsString() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

Int32
CGodotDocument::GetFormat( void ) const
{GUCEF_TRACE;

    const CORE::CDataNode* header = m_root.GetFirstChild();
    return GUCEF_NULL != header ? header->GetAttributeValue( "format" ).AsInt32() : 0;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotDocument::FindSection( const CORE::CString& tag ) const
{GUCEF_TRACE;

    for ( const CORE::CDataNode* section = m_root.GetFirstChild(); GUCEF_NULL != section; section = section->GetNext() )
        if ( tag == section->GetAttributeValue( "tag" ).AsString() )
            return section;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotDocument::FindExtResource( const CORE::CString& id ) const
{GUCEF_TRACE;

    TSectionMap::const_iterator found = m_extResources.find( id );
    return found != m_extResources.end() ? (*found).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotDocument::FindSubResource( const CORE::CString& id ) const
{GUCEF_TRACE;

    TSectionMap::const_iterator found = m_subResources.find( id );
    return found != m_subResources.end() ? (*found).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CGodotDocument::GetExtResourceInfo( const CORE::CString& id, CORE::CString& type, CORE::CString& path, CORE::CString& uid ) const
{GUCEF_TRACE;

    const CORE::CDataNode* section = FindExtResource( id );
    if ( GUCEF_NULL == section )
        return false;
    type = section->GetAttributeValue( "type" ).AsString();
    path = section->GetAttributeValue( "path" ).AsString();
    uid = section->GetAttributeValue( "uid" ).AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

const CGodotDocument::TSectionVector&
CGodotDocument::GetNodeSections( void ) const
{GUCEF_TRACE;

    return m_nodes;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotDocument::GetMainResource( void ) const
{GUCEF_TRACE;

    return m_main;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
