/*
 *  AssetViewerLib: the opening of a folder, an archive or a file of the disk in the viewer
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#include "AssetViewerLib_CAssetTypeResolver.h"
#define ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#endif /* ASSETVIEWERLIB_CASSETTYPERESOLVER_H ? */

#include "AssetViewerLib_CAssetOpener.h"

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

/** The formats of the archives that are offered in the dialogs, whether or not the plugin of the VFS that reads them is loaded */
const char* const ArchivePatterns[] =
{
    "*.zip", "*.7z", "*.rar", "*.tar", "*.tar.gz", "*.tgz", "*.tar.bz2", "*.tbz2", "*.tar.xz", "*.txz", "*.unitypackage"
};

void
AddExtensionPatterns( EAssetKind kind, CORE::CString::StringVector& patterns )
{
    CORE::CString::StringVector extensions;
    CAssetTypeResolver::GetExtensions( kind, extensions );
    for ( size_t i=0; i<extensions.size(); ++i )
        patterns.push_back( "*." + extensions[ i ] );
}

/** The folder part of a path of the disk, without the '/' at the end; the path itself when it has no folder */
CORE::CString
FolderOfDiskPath( const CORE::CString& path )
{
    const Int32 slash = path.HasChar( '/', false );
    if ( slash < 0 )
        return path;
    return 0 == slash ? CORE::CString( "/" ) : path.SubstrFromRange( 0, (UInt32) slash );
}

CORE::CString
FileNameOfPath( const CORE::CString& path )
{
    const Int32 slash = path.HasChar( '/', false );
    return slash < 0 ? path : path.SubstrFromRange( (UInt32) slash+1, path.Length() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetOpener::SOpenResult::SOpenResult( void )
    : kind( OPENKIND_NONE )
    , diskPath()
    , mountPoint()
    , vfsFolder()
    , vfsAsset()
    , problem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetOpener::CAssetOpener( CAssetMounter& mounter )
    : m_mounter( mounter )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetOpener::~CAssetOpener()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetOpener::EOpenKind
CAssetOpener::Classify( const CORE::CString& diskPath )
{GUCEF_TRACE;

    const CORE::CString path = CAssetMounter::NormalizeDiskPath( diskPath );
    if ( path.IsNULLOrEmpty() )
        return OPENKIND_NONE;
    if ( CORE::DirExists( path ) )
        return OPENKIND_FOLDER;
    if ( !CORE::FileExists( path ) )
        return OPENKIND_NONE;
    if ( !CAssetMounter::GetArchiveType( path ).IsNULLOrEmpty() )
        return OPENKIND_ARCHIVE;
    return ASSETKIND_OTHER != CAssetTypeResolver::GetKind( path ) ? OPENKIND_FILE : OPENKIND_NONE;
}

/*-------------------------------------------------------------------------*/

bool
CAssetOpener::Open( const CORE::CString& diskPath, SOpenResult& result )
{GUCEF_TRACE;

    result = SOpenResult();
    result.diskPath = CAssetMounter::NormalizeDiskPath( diskPath );
    if ( result.diskPath.IsNULLOrEmpty() )
    {
        result.problem = "There is no place to open";
        return false;
    }
    result.kind = Classify( result.diskPath );

    switch ( result.kind )
    {
        case OPENKIND_FOLDER:
        case OPENKIND_ARCHIVE:
        {
            if ( !m_mounter.MountUnique( result.diskPath, result.mountPoint ) )
            {
                result.problem = result.diskPath + " cannot be mounted";
                result.kind = OPENKIND_NONE;
                return false;
            }
            result.vfsFolder = result.mountPoint;
            return true;
        }
        case OPENKIND_FILE:
        {
            // in a folder that is mounted already, else the folder of the file is mounted
            CORE::CString relative;
            const Int32 mounted = m_mounter.FindMountOfDiskPath( result.diskPath, relative );
            if ( mounted >= 0 )
            {
                result.mountPoint = m_mounter.GetMount( (UInt32) mounted ).mountPoint;
                result.vfsAsset = result.mountPoint + "/" + relative;
                const CORE::CString folder = FolderOfDiskPath( relative );
                result.vfsFolder = relative.Length() == folder.Length() || folder.IsNULLOrEmpty() ? result.mountPoint : result.mountPoint + "/" + folder;
                return true;
            }
            if ( !m_mounter.MountUnique( FolderOfDiskPath( result.diskPath ), result.mountPoint ) )
            {
                result.problem = "The folder of " + result.diskPath + " cannot be mounted";
                result.kind = OPENKIND_NONE;
                return false;
            }
            result.vfsFolder = result.mountPoint;
            result.vfsAsset = result.mountPoint + "/" + FileNameOfPath( result.diskPath );
            return true;
        }
        default:
        {
            result.problem = CORE::FileExists( result.diskPath ) ?
                result.diskPath + " is not an archive, pack or asset that the viewer has a view for" :
                result.diskPath + " is not there";
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetOpener::GetArchivePatterns( CORE::CString::StringVector& patterns )
{GUCEF_TRACE;

    for ( size_t i=0; i<sizeof( ArchivePatterns ) / sizeof( ArchivePatterns[ 0 ] ); ++i )
        patterns.push_back( ArchivePatterns[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CAssetOpener::GetModelPatterns( CORE::CString::StringVector& patterns )
{GUCEF_TRACE;

    AddExtensionPatterns( ASSETKIND_MODEL, patterns );
    AddExtensionPatterns( ASSETKIND_SCENE, patterns );
    AddExtensionPatterns( ASSETKIND_SKELETON, patterns );
    AddExtensionPatterns( ASSETKIND_MATERIAL, patterns );
    AddExtensionPatterns( ASSETKIND_PARTICLE, patterns );
}

/*-------------------------------------------------------------------------*/

void
CAssetOpener::GetTexturePatterns( CORE::CString::StringVector& patterns )
{GUCEF_TRACE;

    AddExtensionPatterns( ASSETKIND_TEXTURE, patterns );
}

/*-------------------------------------------------------------------------*/

void
CAssetOpener::GetSupportedPatterns( CORE::CString::StringVector& patterns )
{GUCEF_TRACE;

    GetModelPatterns( patterns );
    GetTexturePatterns( patterns );
    GetArchivePatterns( patterns );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
