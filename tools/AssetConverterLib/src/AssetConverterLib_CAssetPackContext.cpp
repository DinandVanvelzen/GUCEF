/*
 *  AssetConverterLib: a pack while it is converted: its source, its files and its index
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
#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CAssetPackContext.h"

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

CAssetPackContext::CAssetPackContext( void )
    : m_manifest()
    , m_sourceRoot()
    , m_mountName()
    , m_files()
    , m_fileSet()
    , m_index()
    , m_nameIndex()
    , m_nameIndexBuilt( false )
    , m_indexLock()
    , m_open( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackContext::~CAssetPackContext()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackContext::Open( const CAssetPackManifest& manifest )
{GUCEF_TRACE;

    Close();
    m_manifest = manifest;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString source = manifest.GetSourcePath();
    if ( vfs.DirExists( source ) )
    {
        m_sourceRoot = source;
    }
    else
    if ( vfs.FileExists( source ) )
    {
        CORE::CString archiveType;
        if ( !vfs.TryGetArchiveTypeForFile( source, archiveType ) || archiveType.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetPackContext: The source " << source << " of pack " << manifest.id << " is no archive the VFS can read (is the plugin of its format loaded?)" );
            return false;
        }
        // by its path, which every archive type supports; else from the opened file, which works for archives in archives
        const CORE::CString mountName = "AssetConverter/" + manifest.id;
        const CORE::CString mountPoint = "assetpacks/" + manifest.id;
        bool mounted = vfs.MountArchive( mountName, source, archiveType, mountPoint, false, false );
        if ( !mounted )
        {
            VFS::TBasicVfsResourcePtr resource = vfs.GetFile( source, "rb", false );
            mounted = !resource.IsNULL() && vfs.MountArchive( mountName, resource, archiveType, mountPoint, false );
        }
        if ( !mounted )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetPackContext: The source " << source << " of pack " << manifest.id << " cannot be mounted as " << archiveType );
            return false;
        }
        m_mountName = mountName;
        m_sourceRoot = mountPoint;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetPackContext: The source " << source << " of pack " << manifest.id << " is neither a folder nor a file of the VFS" );
        return false;
    }

    VFS::CVFS::TStringVector listed;
    vfs.GetFileList( listed, m_sourceRoot, true, true, CORE::CString::Empty, GUCEF_UINT32MAX );
    for ( size_t i=0; i<listed.size(); ++i )
    {
        CORE::CString assetPath;
        const CORE::CString path = CVfsPaths::Normalize( listed[ i ] );
        if ( !CVfsPaths::MakeRelative( m_sourceRoot, path, assetPath ) )
            assetPath = path;
        if ( assetPath.IsNULLOrEmpty() || !manifest.IsIncluded( assetPath ) )
            continue;
        if ( m_fileSet.insert( assetPath ).second )
            m_files.push_back( assetPath );
    }
    std::sort( m_files.begin(), m_files.end() );
    m_open = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackContext: Pack " << manifest.id << " has " << (UInt32) m_files.size() << " files in " << m_sourceRoot );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackContext::Close( void )
{GUCEF_TRACE;

    if ( !m_mountName.IsNULLOrEmpty() )
    {
        VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( m_mountName );
        m_mountName.Clear();
    }
    m_sourceRoot.Clear();
    m_files.clear();
    m_fileSet.clear();
    {
        MT::CScopeMutex lock( m_indexLock );
        m_index.clear();
        m_nameIndex.clear();
        m_nameIndexBuilt = false;
    }
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool CAssetPackContext::IsOpen( void ) const { return m_open; }
const CAssetPackManifest& CAssetPackContext::GetManifest( void ) const { return m_manifest; }
const CORE::CString& CAssetPackContext::GetSourceRoot( void ) const { return m_sourceRoot; }
bool CAssetPackContext::IsArchiveMounted( void ) const { return !m_mountName.IsNULLOrEmpty(); }
const CORE::CString::StringVector& CAssetPackContext::GetFiles( void ) const { return m_files; }

/*-------------------------------------------------------------------------*/

bool
CAssetPackContext::HasFile( const CORE::CString& assetPath ) const
{GUCEF_TRACE;

    return m_fileSet.find( assetPath ) != m_fileSet.end();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackContext::ToVfsPath( const CORE::CString& assetPath ) const
{GUCEF_TRACE;

    return CVfsPaths::Join( m_sourceRoot, assetPath );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackContext::ToAssetPath( const CORE::CString& vfsPath, CORE::CString& assetPath ) const
{GUCEF_TRACE;

    return CVfsPaths::MakeRelative( m_sourceRoot, vfsPath, assetPath );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackContext::SetIndexEntry( const CORE::CString& indexName, const CORE::CString& key, const CORE::CString& assetPath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_indexLock );
    m_index[ indexName ][ key ] = assetPath;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackContext::TryResolve( const CORE::CString& indexName, const CORE::CString& key, CORE::CString& assetPath ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_indexLock );
    TIndexMap::const_iterator index = m_index.find( indexName );
    if ( index == m_index.end() )
        return false;
    TKeyMap::const_iterator entry = (*index).second.find( key );
    if ( entry == (*index).second.end() )
        return false;
    assetPath = (*entry).second;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetPackContext::GetIndexSize( const CORE::CString& indexName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_indexLock );
    TIndexMap::const_iterator index = m_index.find( indexName );
    return index == m_index.end() ? 0 : (UInt32) (*index).second.size();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackContext::FindFilesByName( const CORE::CString& fileName, CORE::CString::StringVector& vfsPaths ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_indexLock );
    if ( !m_nameIndexBuilt )
    {
        for ( size_t f=0; f<m_files.size(); ++f )
            m_nameIndex[ CVfsPaths::NameOf( m_files[ f ] ).Lowercase() ].push_back( ToVfsPath( m_files[ f ] ) );
        m_nameIndexBuilt = true;
    }
    TNameMap::const_iterator found = m_nameIndex.find( fileName.Lowercase() );
    if ( found == m_nameIndex.end() )
        return false;
    vfsPaths = (*found).second;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
