/*
 *  AssetWorkbenchLib: the files that a model of a remote store refers to by name
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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETFILESET_H
#include "gucefASSETSTORE_CAssetFileSet.h"
#define GUCEF_ASSETSTORE_CASSETFILESET_H
#endif /* GUCEF_ASSETSTORE_CASSETFILESET_H ? */

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#include "AssetWorkbenchLib_CAssetTypeResolver.h"
#define ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#endif /* ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#include "gucefASSETSTORE_CIRemoteFileSource.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */

#include "AssetWorkbenchLib_CRemoteReferencedDiscovery.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A model that is larger is not read: the names are looked for in what the reading of a model of that size would cost */
static const UInt64 MaxModelBytes = 64 * 1024 * 1024;
/** The sum of the lengths of the files that were searched, times the number of names that were looked for in them */
static const UInt64 MaxSearchWork = 4ULL * 1024 * 1024 * 1024;
static const UInt32 MaxFolderDepth = 6;
static const UInt32 MaxFolders = 256;
static const UInt32 MaxRounds = 3;

bool
Contains( const UInt8* data, UInt64 length, const CORE::CString& name )
{
    const UInt32 nameLength = name.Length();
    if ( 0 == nameLength || length < nameLength )
        return false;
    const UInt8 first = (UInt8) name[ 0 ];
    const UInt8* at = data;
    const UInt8* const last = data + ( length - nameLength );
    while ( at <= last )
    {
        at = (const UInt8*) memchr( at, first, (size_t) ( last - at + 1 ) );
        if ( GUCEF_NULL == at )
            return false;
        if ( 0 == memcmp( at, name.C_String(), nameLength ) )
            return true;
        ++at;
    }
    return false;
}

CORE::CString
NameOf( const CORE::CString& path )
{
    const Int32 slash = path.HasChar( '/', false );
    return slash < 0 ? path : path.CutChars( (UInt32) slash + 1, true );
}

CORE::CString
ExtensionOf( const CORE::CString& name )
{
    const Int32 dot = name.HasChar( '.', false );
    return dot < 0 ? CORE::CString::Empty : name.CutChars( (UInt32) dot + 1, true ).Lowercase();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteReferencedDiscovery::CRemoteReferencedDiscovery( const CORE::CString& clientPath, const CORE::CString& clientPackFolder )
    : ASSETSTORE::CIRemoteFileDiscovery()
    , m_clientPath( clientPath )
    , m_clientPackFolder( clientPackFolder )
    , m_servicePath()
    , m_servicePackFolder()
    , m_found( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteReferencedDiscovery::CRemoteReferencedDiscovery( void )
    : ASSETSTORE::CIRemoteFileDiscovery()
    , m_clientPath()
    , m_clientPackFolder()
    , m_servicePath()
    , m_servicePackFolder()
    , m_found( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteReferencedDiscovery::GetServicePackFolder( const CORE::CString& servicePath )
{GUCEF_TRACE;

    const CORE::CString path = ASSETSTORE::CIRemoteFileSource::Normalize( servicePath );
    const CORE::CString area = "/" + ASSETSTORE::CPackStorageLayout::OriginalArea + "/";
    const Int32 at = path.HasSubstr( area, true );
    if ( at >= 0 )
    {
        const UInt32 start = (UInt32) at + area.Length();
        const CORE::CString rest = path.CutChars( start, true );
        const Int32 slash = rest.HasChar( '/', true );
        if ( slash > 0 )
            return path.SubstrFromRange( 0, start + (UInt32) slash );
    }
    const Int32 slash = path.HasChar( '/', false );
    return slash < 0 ? CORE::CString::Empty : path.SubstrFromRange( 0, (UInt32) slash );
}

/*-------------------------------------------------------------------------*/

void
CRemoteReferencedDiscovery::OnAssetFileSet( const ASSETSTORE::CAssetFileSet& fileSet )
{GUCEF_TRACE;

    // a discovery made for a client path keeps it; else the first model of the sets is the one whose names are looked for
    if ( !m_clientPath.IsNULLOrEmpty() || !m_servicePath.IsNULLOrEmpty() )
        return;
    CORE::CString model = ASSETSTORE::ASSETSTORE_STATUS_OK == fileSet.mainStatus ? fileSet.mainVfsPath : CORE::CString::Empty;
    for ( size_t f=0; f<fileSet.files.size() && model.IsNULLOrEmpty(); ++f )
        if ( "main" == fileSet.files[ f ].role )
            model = fileSet.files[ f ].path;
    if ( model.IsNULLOrEmpty() || ASSETKIND_MODEL != CAssetTypeResolver::GetKind( model ) )
        return;
    m_servicePath = ASSETSTORE::CIRemoteFileSource::Normalize( model );
    m_servicePackFolder = GetServicePackFolder( m_servicePath );
}

/*-------------------------------------------------------------------------*/

CRemoteReferencedDiscovery::~CRemoteReferencedDiscovery()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteReferencedDiscovery::GetFoundCount( void ) const
{
    return m_found;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteReferencedDiscovery::IsReferenceableExtension( const CORE::CString& extension )
{GUCEF_TRACE;

    static const char* const extensions[] = { "png", "jpg", "jpeg", "tga", "dds", "bmp", "webp", "tif", "tiff", "exr", "hdr", "ktx", "ktx2", "gif", "mtl", "bin" };
    for ( size_t i=0; i<sizeof( extensions ) / sizeof( extensions[ 0 ] ); ++i )
        if ( extension == extensions[ i ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteReferencedDiscovery::Discover( ASSETSTORE::CIRemoteFileSource& system, CORE::CString::StringVector& servicePaths )
{GUCEF_TRACE;

    m_found = 0;
    CORE::CString mainService, packService;
    if ( m_clientPath.IsNULLOrEmpty() )
    {
        if ( m_servicePath.IsNULLOrEmpty() )
            return;
        mainService = m_servicePath;
        packService = m_servicePackFolder;
    }
    else
    if ( !system.ToServicePath( m_clientPath, mainService ) || !system.ToServicePath( m_clientPackFolder, packService ) )
        return;

    // the candidates: the files of the kinds that a model refers to, in the folder tree of the pack
    GUCEF::vector< std::pair< CORE::CString, UInt32 > > pending;
    CORE::CString::StringVector candidates;
    pending.push_back( std::make_pair( packService, (UInt32) 0 ) );
    UInt32 folders = 0;
    while ( !pending.empty() && folders < MaxFolders )
    {
        const CORE::CString folder = pending.back().first;
        const UInt32 depth = pending.back().second;
        pending.pop_back();
        CORE::CString::StringVector dirs;
        CORE::CString::StringVector files;
        if ( !system.GetFolderEntries( folder, dirs, files ) )
            continue;
        ++folders;
        for ( size_t i=0; i<files.size(); ++i )
            if ( IsReferenceableExtension( ExtensionOf( files[ i ] ) ) )
                candidates.push_back( folder.IsNULLOrEmpty() ? files[ i ] : folder + "/" + files[ i ] );
        if ( depth < MaxFolderDepth )
            for ( size_t i=0; i<dirs.size(); ++i )
                pending.push_back( std::make_pair( folder.IsNULLOrEmpty() ? dirs[ i ] : folder + "/" + dirs[ i ], depth + 1 ) );
    }
    if ( candidates.empty() )
        return;

    // the model first, then the material libraries that it led to
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString::StringVector toRead;
    toRead.push_back( mainService );
    CORE::CString::StringSet found;
    UInt64 work = 0;
    for ( UInt32 round=0; round<MaxRounds && !toRead.empty(); ++round )
    {
        CORE::CString::StringVector next;
        for ( size_t r=0; r<toRead.size(); ++r )
        {
            ASSETSTORE::CIRemoteFileSource::SFileInfo info;
            if ( !system.GetFileInfo( toRead[ r ], info ) || info.isDirectory || info.size > MaxModelBytes )
            {
                if ( info.size > MaxModelBytes )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteReferencedDiscovery: " << toRead[ r ] << " is too large to be read for the names of its files" );
                continue;
            }
            CORE::CDynamicBuffer content;
            if ( !vfs.LoadFile( content, system.ToClientPath( toRead[ r ] ), "rb" ) || 0 == content.GetDataSize() )
                continue;
            const UInt8* data = static_cast< const UInt8* >( content.GetConstBufferPtr() );
            const UInt64 length = content.GetDataSize();
            for ( size_t c=0; c<candidates.size(); ++c )
            {
                if ( found.find( candidates[ c ] ) != found.end() || candidates[ c ] == toRead[ r ] )
                    continue;
                if ( work > MaxSearchWork )
                    break;
                work += length;
                const CORE::CString name = NameOf( candidates[ c ] );
                if ( Contains( data, length, name ) || ( name.HasChar( ' ', true ) >= 0 && Contains( data, length, name.ReplaceSubstr( " ", "%20" ) ) ) )
                {
                    found.insert( candidates[ c ] );
                    servicePaths.push_back( candidates[ c ] );
                    ++m_found;
                    if ( "mtl" == ExtensionOf( name ) )
                        next.push_back( candidates[ c ] );
                }
            }
        }
        toRead = next;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteReferencedDiscovery: " << ( m_clientPath.IsNULLOrEmpty() ? mainService : m_clientPath ) << " names " << m_found << " of the " << (UInt32) candidates.size() << " pictures, material libraries and buffers in its pack" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
