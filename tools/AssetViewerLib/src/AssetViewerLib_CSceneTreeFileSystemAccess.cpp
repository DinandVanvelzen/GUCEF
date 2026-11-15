/*
 *  AssetViewerLib: a scene node tree as a read only file system, for the content browser
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

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#include "AssetViewerLib_CSceneTreeFileSystemAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const ObjectKinds[] = { "entity", "light", "camera", "particles", "object" };

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSceneTreeFileSystemAccess::CSceneTreeFileSystemAccess( const CORE::CString& displayName )
    : CORE::IFileSystemAccess()
    , m_displayName( displayName )
    , m_nodes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSceneTreeFileSystemAccess::~CSceneTreeFileSystemAccess()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSceneTreeFileSystemAccess::SetTree( const TNodeVector& nodes )
{GUCEF_TRACE;

    m_nodes = nodes;
}

/*-------------------------------------------------------------------------*/

const CSceneTreeFileSystemAccess::TNodeVector&
CSceneTreeFileSystemAccess::GetNodes( void ) const
{GUCEF_TRACE;

    return m_nodes;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSceneTreeFileSystemAccess::Conform( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString conformed = path.ReplaceChar( '\\', '/' );
    while ( conformed.Length() > 1 && '/' == conformed[ conformed.Length()-1 ] )
        conformed = conformed.CutChars( 1, false );
    if ( conformed.IsNULLOrEmpty() || '/' != conformed[ 0 ] )
        conformed = "/" + conformed;
    return conformed;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSceneTreeFileSystemAccess::JoinPath( const CORE::CString& parent, const CORE::CString& name )
{GUCEF_TRACE;

    if ( parent.IsNULLOrEmpty() || "/" == parent )
        return "/" + name;
    return parent + "/" + name;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSceneTreeFileSystemAccess::ParentOf( const CORE::CString& path )
{GUCEF_TRACE;

    const Int32 slash = path.HasChar( '/', false );
    return slash <= 0 ? CORE::CString( "/" ) : path.SubstrFromRange( 0, (UInt32) slash );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSceneTreeFileSystemAccess::GetObjectKind( const CORE::CString& fileName )
{GUCEF_TRACE;

    for ( size_t i=0; i<sizeof( ObjectKinds ) / sizeof( ObjectKinds[ 0 ] ); ++i )
    {
        const CORE::CString suffix = CORE::CString( "." ) + ObjectKinds[ i ];
        if ( fileName.Length() > suffix.Length() && fileName.SubstrFromRange( fileName.Length() - suffix.Length(), fileName.Length() ) == suffix )
            return ObjectKinds[ i ];
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CSceneTreeFileSystemAccess::SNode*
CSceneTreeFileSystemAccess::FindNode( const CORE::CString& path ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_nodes.size(); ++i )
        if ( m_nodes[ i ].path == path )
            return &m_nodes[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSceneTreeFileSystemAccess::GetDisplayName( void ) const
{GUCEF_TRACE;

    return m_displayName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSceneTreeFileSystemAccess::GetRootPath( void ) const
{GUCEF_TRACE;

    return "/";
}

/*-------------------------------------------------------------------------*/

bool
CSceneTreeFileSystemAccess::IsWriteable( const CORE::CString& path ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryCreateDirectory( const CORE::CString& path, bool createParents )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryDeleteDirectory( const CORE::CString& path, bool deleteContents )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryMoveFiles( const CORE::CString::StringVector& filePaths     ,
                                          const CORE::CString& destinationDir             ,
                                          bool overwrite                                  ,
                                          TFileSystemAccessResultVector* perFileResults   )
{GUCEF_TRACE;

    if ( GUCEF_NULL != perFileResults )
        perFileResults->assign( filePaths.size(), CORE::FILESYSTEMACCESS_READ_ONLY );
    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryRenameFile( const CORE::CString& path, const CORE::CString& newName )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryDeleteFile( const CORE::CString& path )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( path );
    entryType = CORE::FILESYSTEMENTRYTYPE_NONE;
    if ( "/" == conformed || GUCEF_NULL != FindNode( conformed ) )
    {
        entryType = CORE::FILESYSTEMENTRYTYPE_DIRECTORY;
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }
    const SNode* parent = FindNode( ParentOf( conformed ) );
    if ( GUCEF_NULL == parent && "/" == ParentOf( conformed ) )
        parent = FindNode( "/" );
    if ( GUCEF_NULL != parent )
    {
        const CORE::CString fileName = conformed.SubstrFromRange( conformed.HasChar( '/', false ) + 1, conformed.Length() );
        for ( size_t i=0; i<parent->objects.size(); ++i )
            if ( fileName == parent->objects[ i ].name + "." + parent->objects[ i ].kind )
                entryType = CORE::FILESYSTEMENTRYTYPE_FILE;
    }
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    CORE::TFileSystemEntryType entryType = CORE::FILESYSTEMENTRYTYPE_NONE;
    TryGetEntryType( path, entryType );
    if ( CORE::FILESYSTEMENTRYTYPE_NONE == entryType )
        return CORE::FILESYSTEMACCESS_NOT_FOUND;
    const CORE::CString conformed = Conform( path );
    metaData.resourceExists = true;
    metaData.name = "/" == conformed ? m_displayName : conformed.SubstrFromRange( conformed.HasChar( '/', false ) + 1, conformed.Length() );
    metaData.hasName = true;
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( dirPath );
    CORE::TFileSystemEntryType entryType = CORE::FILESYSTEMENTRYTYPE_NONE;
    TryGetEntryType( conformed, entryType );
    if ( CORE::FILESYSTEMENTRYTYPE_DIRECTORY != entryType )
        return CORE::FILESYSTEMENTRYTYPE_FILE == entryType ? CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY : CORE::FILESYSTEMACCESS_NOT_FOUND;

    CORE::CFileSystemEntryListIterator::TEntryVector entries;
    // the child nodes: the nodes whose parent this is
    for ( size_t i=0; i<m_nodes.size(); ++i )
    {
        const CORE::CString& nodePath = m_nodes[ i ].path;
        if ( "/" == nodePath || ParentOf( nodePath ) != conformed )
            continue;
        CORE::CFileSystemEntryListIterator::CEntry entry;
        entry.name = nodePath.SubstrFromRange( nodePath.HasChar( '/', false ) + 1, nodePath.Length() );
        entry.path = nodePath;
        entry.isDirectory = true;
        entries.push_back( entry );
    }
    // the objects attached to this node
    const SNode* node = FindNode( conformed );
    if ( GUCEF_NULL != node )
    {
        for ( size_t i=0; i<node->objects.size(); ++i )
        {
            CORE::CFileSystemEntryListIterator::CEntry entry;
            entry.name = node->objects[ i ].name + "." + node->objects[ i ].kind;
            entry.path = JoinPath( conformed, entry.name );
            entry.isFile = true;
            entries.push_back( entry );
        }
    }
    iterator = CORE::TFileSystemIteratorPtr( GUCEF_NEW CORE::CFileSystemEntryListIterator( conformed, entries ) );
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const
{GUCEF_TRACE;

    specialDirs.clear();
    return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CSceneTreeFileSystemAccess::TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
