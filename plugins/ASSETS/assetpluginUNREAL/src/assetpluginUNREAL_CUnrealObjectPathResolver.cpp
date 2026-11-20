/*
 *  assetpluginUNREAL: Maps package names to the files that hold them
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

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#include "assetpluginUNREAL_CUnrealImporter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#include "assetpluginUNREAL_CUnrealObjectPathResolver.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */

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

CUnrealObjectPathResolver::CUnrealObjectPathResolver( void )
    : m_context( GUCEF_NULL )
    , m_recorder( GUCEF_NULL )
    , m_paths()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealObjectPathResolver::SetContext( ASSETCONVERTER::CAssetPackContext* context )
{GUCEF_TRACE;

    m_context = context;
}

/*-------------------------------------------------------------------------*/

void
CUnrealObjectPathResolver::SetRecorder( ASSETCONVERTER::CAssetScene* scene )
{GUCEF_TRACE;

    m_recorder = scene;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealObjectPathResolver::ResolveReference( const CORE::CString& objectPath, const CORE::CString& kind, CORE::CString& vfsPath, bool optional ) const
{GUCEF_TRACE;

    const bool resolved = Resolve( objectPath, vfsPath );
    if ( GUCEF_NULL != m_recorder && !objectPath.IsNULLOrEmpty() )
    {
        if ( resolved )
            m_recorder->AddReference( kind, vfsPath, objectPath, optional );
        else
            m_recorder->AddUnresolvedReference( kind, objectPath, optional );
    }
    return resolved;
}

/*-------------------------------------------------------------------------*/

void
CUnrealObjectPathResolver::AddPackage( const CORE::CString& packageName, const CORE::CString& vfsPath )
{GUCEF_TRACE;

    m_paths[ packageName ] = vfsPath;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealObjectPathResolver::AddPackagesOf( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    VFS::CVFS::TStringVector files;
    VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( files, vfsFolder, true, true, CORE::CString::EmptyStringSet, 1000000 );
    UInt32 added = 0;
    for ( size_t i=0; i<files.size(); ++i )
    {
        const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( files[ i ] ).Lowercase();
        if ( "uasset" != extension && "umap" != extension )
            continue;
        CUnrealPackage package;
        if ( !package.Open( files[ i ] ) )
            continue;
        const CORE::CString packageName = PackageNameOfFile( package.GetSummary().packageName, files[ i ] );
        if ( !packageName.IsNULLOrEmpty() )
        {
            AddPackage( packageName, files[ i ] );
            ++added;
        }
    }
    return added;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealObjectPathResolver::PackageNameOf( const CORE::CString& objectPath )
{GUCEF_TRACE;

    const Int32 dot = objectPath.HasChar( '.' );
    return dot > 0 ? objectPath.SubstrFromRange( 0, (UInt32) dot ) : objectPath;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealObjectPathResolver::PackageNameOfFile( const CORE::CString& summaryPackageName, const CORE::CString& path )
{GUCEF_TRACE;

    if ( !summaryPackageName.IsNULLOrEmpty() && '/' == summaryPackageName[ 0 ] )
        return summaryPackageName;

    // the place of the file: <...>/Content/<rest> is /Game/<rest>, <...>/Plugins/<Name>/Content/<rest> is /<Name>/<rest>
    const CORE::CString normalized = "/" + path.ReplaceChar( '\\', '/' );
    const Int32 content = normalized.HasSubstr( "/Content/" );
    if ( content < 0 )
        return CORE::CString();
    CORE::CString root = "/Game";
    const CORE::CString before = normalized.SubstrFromRange( 0, (UInt32) content );
    const Int32 plugins = before.HasSubstr( "/Plugins/" );
    if ( plugins >= 0 )
    {
        const CORE::CString pluginPath = before.SubstrFromRange( (UInt32) plugins + 9, before.Length() );
        const CORE::CString pluginName = pluginPath.HasChar( '/', false ) >= 0 ? pluginPath.SubstrFromRange( (UInt32) pluginPath.HasChar( '/', false ) + 1, pluginPath.Length() ) : pluginPath;
        if ( !pluginName.IsNULLOrEmpty() )
            root = "/" + pluginName;
    }
    CORE::CString rest = normalized.SubstrFromRange( (UInt32) content + 9, normalized.Length() );
    const Int32 dot = rest.HasChar( '.', false );
    if ( dot > 0 )
        rest = rest.SubstrFromRange( 0, (UInt32) dot );
    return rest.IsNULLOrEmpty() ? CORE::CString() : root + "/" + rest;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealObjectPathResolver::Resolve( const CORE::CString& objectPath, CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    const CORE::CString packageName = PackageNameOf( objectPath );
    TPathMap::const_iterator i = m_paths.find( packageName );
    if ( i != m_paths.end() )
    {
        vfsPath = (*i).second;
        return true;
    }
    CORE::CString assetPath;
    if ( GUCEF_NULL != m_context && m_context->TryResolve( CUnrealImporter::PackageIndexName, packageName, assetPath ) )
    {
        vfsPath = m_context->ToVfsPath( assetPath );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
