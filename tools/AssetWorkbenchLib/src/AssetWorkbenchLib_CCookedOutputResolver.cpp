/*
 *  AssetWorkbenchLib: finds the cooked Ogre output of an asset and says whether it is current
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#include "AssetConverterLib_CAssetPipeline.h"
#define ASSETCONVERTERLIB_CASSETPIPELINE_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#include "AssetConverterLib_CConversionSidecar.h"
#define ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetWorkbenchLib_CCookedOutputResolver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The parts of a path between the separators */
void
SplitSegments( const CORE::CString& path, CORE::CString::StringVector& segments )
{
    segments.clear();
    CORE::CString current;
    for ( UInt32 i=0; i<path.Length(); ++i )
    {
        if ( '/' == path[ i ] )
        {
            if ( !current.IsNULLOrEmpty() )
                segments.push_back( current );
            current.Clear();
        }
        else
        {
            current += path[ i ];
        }
    }
    if ( !current.IsNULLOrEmpty() )
        segments.push_back( current );
}

CORE::CString
JoinSegments( const CORE::CString::StringVector& segments, size_t first, size_t end )
{
    CORE::CString joined;
    for ( size_t i=first; i<end && i<segments.size(); ++i )
    {
        if ( !joined.IsNULLOrEmpty() )
            joined += "/";
        joined += segments[ i ];
    }
    return joined;
}

bool
StartsWithFolder( const CORE::CString& path, const CORE::CString& folder )
{
    return path.Length() > folder.Length() && path.SubstrFromRange( 0, folder.Length() ) == folder && '/' == path[ folder.Length() ];
}

}

/*-------------------------------------------------------------------------*/

CCookedOutputResolver::SOptions::SOptions( void )
    : profile( "desktop" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCookedOutputResolver::SCookedOutput::SCookedOutput( void )
    : state( STATE_ABSENT )
    , verified( false )
    , sourceVerified( false )
    , origin()
    , assetPath()
    , sourceVfsPath()
    , intermediateFolder()
    , cookedFolder()
    , profile()
    , cacheKey()
    , mainFile()
    , sidecarFile()
    , files()
    , reason()
    , entry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCookedOutputResolver::SCookedOutput::IsCurrent( void ) const
{GUCEF_TRACE;

    return STATE_CURRENT == state;
}

/*-------------------------------------------------------------------------*/

bool
CCookedOutputResolver::SCookedOutput::IsLocated( void ) const
{GUCEF_TRACE;

    return !mainFile.IsNULLOrEmpty() && !cacheKey.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCookedOutputResolver::GetCookedLabel( void )
{GUCEF_TRACE;

    return "cooked Ogre v1 output (gucef-ogre3d)";
}

/*-------------------------------------------------------------------------*/

bool
CCookedOutputResolver::IsCookedMainExtension( const CORE::CString& extension )
{GUCEF_TRACE;

    const CORE::CString lower = extension.Lowercase();
    return "mesh" == lower || "scene" == lower;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCookedOutputResolver::Describe( const SCookedOutput& output )
{GUCEF_TRACE;

    switch ( output.state )
    {
        case STATE_CURRENT:
            return "cooked output: current" + ( output.verified && !output.sourceVerified ? CORE::CString( " (the source is not there to compare with)" ) : CORE::CString::Empty );
        case STATE_STALE:
            return "cooked output: stale" + ( output.reason.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + output.reason + ")" );
        default:
            return "cooked output: none" + ( output.reason.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + output.reason + ")" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CCookedOutputResolver::SplitStorePath( const CORE::CString& vfsPath, CORE::CString& locationRoot, CORE::CString& packFolder, CORE::CString& relative )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString::StringVector segments;
    SplitSegments( ASSETCONVERTER::CVfsPaths::Normalize( vfsPath ), segments );
    // the last area folder of that name that has a pack of this name beside it in the other areas
    for ( Int32 i=(Int32) segments.size()-2; i>=0; --i )
    {
        if ( segments[ (size_t) i ] != ASSETSTORE::CPackStorageLayout::OriginalArea )
            continue;
        const CORE::CString root = JoinSegments( segments, 0, (size_t) i );
        const CORE::CString& pack = segments[ (size_t) i + 1 ];
        if ( vfs.DirExists( ASSETSTORE::CPackStorageLayout::Join( ASSETSTORE::CPackStorageLayout::Join( root, ASSETSTORE::CPackStorageLayout::IntermediateArea ), pack ) ) ||
             vfs.DirExists( ASSETSTORE::CPackStorageLayout::Join( ASSETSTORE::CPackStorageLayout::Join( root, ASSETSTORE::CPackStorageLayout::CookedArea ), pack ) ) )
        {
            locationRoot = root;
            packFolder = pack;
            relative = JoinSegments( segments, (size_t) i + 2, segments.size() );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCookedOutputResolver::Reroot( const CORE::CString& recorded, const CORE::CString& cookedFolder, const CORE::CString& profile )
{GUCEF_TRACE;

    const CORE::CString file = ASSETCONVERTER::CVfsPaths::Normalize( recorded );
    const CORE::CString folder = ASSETCONVERTER::CVfsPaths::Normalize( cookedFolder );
    if ( StartsWithFolder( file, folder ) )
        return file;
    // the entry was written where the VFS had other names for the places: the last two parts of the cooked folder
    // ("gucef-ogre3d/<pack folder>") say where the cooked area starts in what it recorded
    CORE::CString::StringVector segments;
    SplitSegments( folder, segments );
    if ( segments.size() >= 2 )
    {
        const CORE::CString marker = "/" + JoinSegments( segments, segments.size() - 2, segments.size() ) + "/";
        const Int32 at = ( "/" + file ).HasSubstr( marker, true );
        if ( at >= 0 )
            return ASSETCONVERTER::CVfsPaths::Join( folder, ( "/" + file ).CutChars( (UInt32) at + marker.Length(), true ) );
    }
    // a conversion that wrote to a mount of its own ("out/<profile>/..."): the folder of the profile is below the cooked folder
    const CORE::CString profileMarker = "/" + profile + "/";
    const Int32 inProfile = ( "/" + file ).HasSubstr( profileMarker, true );
    if ( inProfile >= 0 )
        return ASSETCONVERTER::CVfsPaths::Join( folder, ( "/" + file ).CutChars( (UInt32) inProfile + 1, true ) );
    return file;
}

/*-------------------------------------------------------------------------*/

void
CCookedOutputResolver::LocateInFolders( const CORE::CString& intermediateFolder, const CORE::CString& cookedFolder, const CORE::CString& assetPath, const CORE::CString& sourceVfsPath, const SOptions& options, SCookedOutput& output )
{GUCEF_TRACE;

    output = SCookedOutput();
    output.assetPath = ASSETCONVERTER::CVfsPaths::Normalize( assetPath );
    output.sourceVfsPath = sourceVfsPath;
    output.intermediateFolder = ASSETCONVERTER::CVfsPaths::Normalize( intermediateFolder );
    output.cookedFolder = ASSETCONVERTER::CVfsPaths::Normalize( cookedFolder );
    output.profile = options.profile;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( output.assetPath.IsNULLOrEmpty() || output.intermediateFolder.IsNULLOrEmpty() )
    {
        output.reason = "the asset has no place in a pack";
        return;
    }
    const CORE::CString entryPath = ASSETCONVERTER::CAssetCacheEntry::GetEntryPath( output.intermediateFolder, output.assetPath );
    ASSETCONVERTER::CAssetCacheEntry entry;
    if ( !vfs.FileExists( entryPath ) )
    {
        output.reason = "not converted yet";
        return;
    }
    if ( !entry.Load( entryPath ) )
    {
        output.reason = "the cache entry of the converter cannot be read";
        return;
    }

    // the cooked files of the profile
    CORE::CString::StringVector referenced;
    bool otherProfile = false;
    CORE::CString::StringVector missing;
    for ( size_t f=0; f<entry.files.size(); ++f )
    {
        const ASSETCONVERTER::CConversionReportFile& file = entry.files[ f ];
        if ( !file.IsCooked() )
            continue;
        if ( file.profile != options.profile )
        {
            otherProfile = true;
            continue;
        }
        const CORE::CString path = Reroot( file.path, output.cookedFolder, options.profile );
        output.files.push_back( path );
        if ( !vfs.FileExists( path ) )
            missing.push_back( path );
        for ( size_t r=0; r<file.references.size(); ++r )
            referenced.push_back( Reroot( file.references[ r ], output.cookedFolder, options.profile ) );
    }
    if ( output.files.empty() )
    {
        output.reason = otherProfile ? "cooked, but not for the profile " + options.profile : CORE::CString( "converted, not cooked for Ogre yet" );
        return;
    }

    // what to show: the scene of the asset, else the mesh that no other cooked file refers to (the first level of detail, not the others)
    CORE::CString scene, root, anyMesh;
    for ( size_t i=0; i<output.files.size(); ++i )
    {
        const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( output.files[ i ] ).Lowercase();
        if ( "scene" == extension && scene.IsNULLOrEmpty() )
            scene = output.files[ i ];
        if ( "mesh" == extension )
        {
            if ( anyMesh.IsNULLOrEmpty() )
                anyMesh = output.files[ i ];
            bool isReferenced = false;
            for ( size_t r=0; r<referenced.size(); ++r )
                if ( referenced[ r ] == output.files[ i ] )
                    isReferenced = true;
            if ( !isReferenced && root.IsNULLOrEmpty() )
                root = output.files[ i ];
        }
    }
    output.mainFile = !scene.IsNULLOrEmpty() ? scene : ( !root.IsNULLOrEmpty() ? root : anyMesh );
    if ( output.mainFile.IsNULLOrEmpty() )
    {
        output.reason = "the cooked files hold no mesh or scene";
        return;
    }
    const CORE::CString sidecar = ASSETCONVERTER::CConversionSidecar::GetPath( output.intermediateFolder, output.assetPath );
    if ( vfs.FileExists( sidecar ) )
        output.sidecarFile = sidecar;
    output.cacheKey = entry.key;
    output.entry = entry;
    output.state = STATE_CURRENT;
    if ( entry.cookPending )
    {
        output.state = STATE_STALE;
        output.reason = "the cooking did not finish";
    }
    else
    if ( !missing.empty() )
    {
        output.state = STATE_STALE;
        output.reason = "the cooked file " + missing[ 0 ] + " is gone";
    }
}

/*-------------------------------------------------------------------------*/

void
CCookedOutputResolver::LocateInStore( const CORE::CString& vfsPath, const CORE::CString& assetPath, const SOptions& options, SCookedOutput& output )
{GUCEF_TRACE;

    CORE::CString root, pack, relative;
    if ( !SplitStorePath( vfsPath, root, pack, relative ) )
    {
        output = SCookedOutput();
        output.reason = "not in the layout of a store";
        return;
    }
    const CORE::CString asset = assetPath.IsNULLOrEmpty() ? relative : ASSETCONVERTER::CVfsPaths::Normalize( assetPath );
    const CORE::CString intermediate = ASSETSTORE::CPackStorageLayout::Join( ASSETSTORE::CPackStorageLayout::Join( root, ASSETSTORE::CPackStorageLayout::IntermediateArea ), pack );
    const CORE::CString cooked = ASSETSTORE::CPackStorageLayout::Join( ASSETSTORE::CPackStorageLayout::Join( root, ASSETSTORE::CPackStorageLayout::CookedArea ), pack );
    const CORE::CString source = ASSETSTORE::CPackStorageLayout::Join( ASSETSTORE::CPackStorageLayout::Join( ASSETSTORE::CPackStorageLayout::Join( root, ASSETSTORE::CPackStorageLayout::OriginalArea ), pack ), asset );
    LocateInFolders( intermediate, cooked, asset, source, options, output );
    output.origin = "store";
}

/*-------------------------------------------------------------------------*/

void
CCookedOutputResolver::Verify( SCookedOutput& output )
{GUCEF_TRACE;

    output.verified = true;
    if ( !output.IsLocated() || STATE_STALE == output.state )
        return;
    CORE::CString detail;
    bool sourceVerified = false;
    const ASSETCONVERTER::TActivityReason reason = ASSETCONVERTER::CAssetPipeline::CheckStoredEntry( output.entry, output.sourceVfsPath, CORE::CString::Empty, detail, sourceVerified );
    output.sourceVerified = sourceVerified;
    if ( ASSETCONVERTER::ACTIVITYREASON_UNCHANGED != reason )
    {
        output.state = STATE_STALE;
        output.reason = ASSETCONVERTER::ToString( reason ) + ( detail.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + detail );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CCookedOutputResolver: The cooked output of " << output.assetPath << " is stale: " << output.reason );
    }
}

/*-------------------------------------------------------------------------*/

void
CCookedOutputResolver::ResolveInStore( const CORE::CString& vfsPath, const CORE::CString& assetPath, const SOptions& options, SCookedOutput& output )
{GUCEF_TRACE;

    LocateInStore( vfsPath, assetPath, options, output );
    Verify( output );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
