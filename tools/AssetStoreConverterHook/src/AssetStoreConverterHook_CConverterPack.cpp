/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetStoreConverterHook_CConverterPack.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const ManifestSuffix = ".assetpack.json";

/** Whether a member of a manifest (as written) has a ".." or "." segment */
bool
HasDotSegment( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString::StringVector segments = path.ReplaceChar( '\\', '/' ).ParseElements( '/', false );
    for ( size_t i=0; i<segments.size(); ++i )
        if ( ".." == segments[ i ] || "." == segments[ i ] )
            return true;
    return false;
}

/** Whether the folder lies in a root of the kinds; 'problem' says why not */
bool
CheckFolder( const ASSETSTORE::TAssetToolRootVector& roots, const char* member, const CORE::CString& written, const CORE::CString& resolved,
             const char* kinds, CORE::CString& problem )
{GUCEF_TRACE;

    if ( HasDotSegment( written ) )
    {
        problem = CORE::CString( "the member " ) + member + " of the manifest has a \"..\" or \".\" segment";
        return false;
    }
    CORE::CString relative;
    const ASSETSTORE::CAssetToolRoot* root = ASSETSTORE::CAssetToolRoot::FindHolder( roots, resolved, kinds, relative );
    if ( GUCEF_NULL == root )
    {
        problem = CORE::CString( "the member " ) + member + " of the manifest names a folder outside the roots of the kind " + CORE::CString( kinds ).ReplaceChar( '|', '/' );
        return false;
    }
    CORE::CString why;
    if ( !ASSETSTORE::CAssetRootedPath::IsInsideRootOnDisk( *root, relative, why ) )
    {
        problem = CORE::CString( "the member " ) + member + " of the manifest: " + why;
        return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CConverterPack::CConverterPack( void )
    : packId()
    , manifest()
    , manifestVfsPath()
    , manifestRooted()
    , ambiguous( false )
    , otherManifests()
    , runnable( false )
    , runnableReason()
    , licence()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CConverterPack::ToRooted( const ASSETSTORE::TAssetToolRootVector& roots, const CORE::CString& vfsPath, CORE::CString& rooted )
{GUCEF_TRACE;

    CORE::CString relative;
    const ASSETSTORE::CAssetToolRoot* root = ASSETSTORE::CAssetToolRoot::FindHolder( roots, vfsPath, CORE::CString::Empty, relative );
    if ( GUCEF_NULL == root )
        return false;
    rooted = root->id + ":" + relative;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConverterPack::CheckManifestPaths( const ASSETSTORE::TAssetToolRootVector& roots, const ASSETCONVERTER::CAssetPackManifest& pack, CORE::CString& problem )
{GUCEF_TRACE;

    if ( pack.source.IsNULLOrEmpty() )
    {
        problem = "the manifest names no source";
        return false;
    }
    if ( !CheckFolder( roots, "source", pack.source, pack.GetSourcePath(), "source|staging", problem ) )
        return false;
    if ( pack.intermediate.IsNULLOrEmpty() )
    {
        problem = "the manifest names no intermediate folder";
        return false;
    }
    if ( !CheckFolder( roots, "intermediate", pack.intermediate, pack.GetIntermediatePath(), "intermediate", problem ) )
        return false;
    if ( !pack.output.IsNULLOrEmpty() && !CheckFolder( roots, "output", pack.output, pack.GetOutputPath(), "output", problem ) )
        return false;
    if ( !pack.overrides.IsNULLOrEmpty() && !CheckFolder( roots, "overrides", pack.overrides, pack.GetOverridesPath(), "overrides", problem ) )
        return false;
    if ( !pack.licenceRecord.IsNULLOrEmpty() )
    {
        const CORE::CString recordFolder = ASSETCONVERTER::CVfsPaths::FolderOf( pack.GetLicenceRecordPath() );
        if ( !CheckFolder( roots, "licenceFolder", pack.licenceFolder, recordFolder, "licences", problem ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CConverterPack::Discover( const ASSETSTORE::TAssetToolRootVector& roots, TPackVector& packs )
{GUCEF_TRACE;

    packs.clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t r=0; r<roots.size(); ++r )
    {
        if ( ASSETSTORE::CAssetToolRoot::KindManifests != roots[ r ].kind )
            continue;
        VFS::CVFS::TStringVector files;
        vfs.GetFileList( files, roots[ r ].vfsPath, true, true, CORE::CString( "*" ) + ManifestSuffix, 100000 );
        for ( size_t f=0; f<files.size(); ++f )
        {
            const CORE::CString vfsPath = files[ f ];
            const UInt32 suffixLength = CORE::CString( ManifestSuffix ).Length();
            if ( vfsPath.Length() <= suffixLength || vfsPath.CutChars( vfsPath.Length() - suffixLength, true ) != ManifestSuffix )
                continue;
            ASSETSTORE::CAssetRootedPath rooted;
            if ( !ASSETSTORE::CAssetRootedPath::FromVfsPath( roots[ r ], vfsPath, rooted ) )
                continue;
            CORE::CString why;
            if ( !ASSETSTORE::CAssetRootedPath::IsInsideRootOnDisk( roots[ r ], rooted.relativePath, why ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CConverterPack: The manifest " << rooted.ToString() << " is left out: " << why );
                continue;
            }
            CConverterPack pack;
            if ( !pack.manifest.Load( vfsPath ) || pack.manifest.id.IsNULLOrEmpty() )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CConverterPack: The manifest " << rooted.ToString() << " cannot be read" );
                continue;
            }
            pack.packId = pack.manifest.id;
            pack.manifestVfsPath = vfsPath;
            pack.manifestRooted = rooted.ToString();
            pack.runnable = CheckManifestPaths( roots, pack.manifest, pack.runnableReason );
            if ( pack.runnable )
                pack.runnableReason.Clear();
            pack.licence.Evaluate( pack.manifest );
            packs.push_back( pack );
        }
    }
    for ( size_t i=0; i<packs.size(); ++i )
    {
        for ( size_t j=0; j<packs.size(); ++j )
        {
            if ( i != j && packs[ i ].packId == packs[ j ].packId )
            {
                packs[ i ].ambiguous = true;
                packs[ i ].otherManifests.push_back( packs[ j ].manifestRooted );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CConverterPack::GetReportPath( void ) const
{GUCEF_TRACE;

    return ASSETCONVERTER::CVfsPaths::Join( manifest.GetIntermediatePath(), "report.json" );
}

/*-------------------------------------------------------------------------*/

bool
CConverterPack::LoadReport( ASSETCONVERTER::CConversionReport& report ) const
{GUCEF_TRACE;

    report.Clear();
    const CORE::CString path = GetReportPath();
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( path ) && report.LoadJson( path );
}

/*-------------------------------------------------------------------------*/

bool
CConverterPack::LoadActivity( ASSETCONVERTER::CImportActivityLog& log ) const
{GUCEF_TRACE;

    const CORE::CString path = ASSETCONVERTER::CImportActivityLog::GetLogPath( manifest.GetIntermediatePath() );
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( path ) && log.Load( path );
}

/*-------------------------------------------------------------------------*/

bool
CConverterPack::HasIntermediate( void ) const
{GUCEF_TRACE;

    const CORE::CString path = manifest.GetIntermediatePath();
    return !path.IsNULLOrEmpty() && VFS::CVfsGlobal::Instance()->GetVfs().DirExists( path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CConverterPack::FindProfileFolder( const CORE::CString& profile ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString output = manifest.GetOutputPath();
    if ( output.IsNULLOrEmpty() || profile.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    const CORE::CString direct = ASSETCONVERTER::CVfsPaths::Join( output, profile );
    if ( vfs.DirExists( direct ) )
        return direct;
    for ( size_t t=0; t<manifest.cookTargets.size(); ++t )
    {
        const CORE::CString below = ASSETCONVERTER::CVfsPaths::Join( ASSETCONVERTER::CVfsPaths::Join( output, manifest.cookTargets[ t ] ), profile );
        if ( vfs.DirExists( below ) )
            return below;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
