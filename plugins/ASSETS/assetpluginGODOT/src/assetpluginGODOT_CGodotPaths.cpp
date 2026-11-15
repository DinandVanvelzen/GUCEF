/*
 *  assetpluginGODOT: paths and identities of a Godot project
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

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#include "assetpluginGODOT_CGodotDocument.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#include "assetpluginGODOT_CGodotVariant.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H
#include "assetpluginGODOT_CGodotBinaryReader.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#include "assetpluginGODOT_CGodotPaths.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CGodotPaths::UidIndexName = "uid";
const CORE::CString CGodotPaths::LowerIndexName = "lower";
const CORE::CString CGodotPaths::IndexBuiltKey = "<built>";

static MT::CMutex g_indexLock;
const CORE::CString CGodotPaths::ResPrefix = "res://";
const CORE::CString CGodotPaths::UidPrefix = "uid://";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CGodotPaths::StripScheme( const CORE::CString& path )
{GUCEF_TRACE;

    if ( path.StartsWith( ResPrefix ) )
        return ASSETCONVERTER::CVfsPaths::Normalize( path.SubstrFromRange( ResPrefix.Length(), path.Length() ) );
    return ASSETCONVERTER::CVfsPaths::Normalize( path );
}

/*-------------------------------------------------------------------------*/

bool
CGodotPaths::FindProjectRoot( const ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& assetPath, CORE::CString& projectRoot )
{GUCEF_TRACE;

    CORE::CString folder = ASSETCONVERTER::CVfsPaths::FolderOf( assetPath );
    for ( ; ; )
    {
        if ( context.HasFile( ASSETCONVERTER::CVfsPaths::Join( folder, "project.godot" ) ) )
        {
            projectRoot = folder;
            return true;
        }
        if ( folder.IsNULLOrEmpty() )
            return false;
        folder = ASSETCONVERTER::CVfsPaths::FolderOf( folder );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGodotPaths::Resolve( ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& referrer, const CORE::CString& path, const CORE::CString& uid, CORE::CString& assetPath )
{GUCEF_TRACE;

    if ( !uid.IsNULLOrEmpty() && context.TryResolve( UidIndexName, uid, assetPath ) && context.HasFile( assetPath ) )
        return true;
    if ( path.IsNULLOrEmpty() && uid.IsNULLOrEmpty() )
        return false;

    // the places of the path: against the project of the referrer, then the root of the pack, then the folders above the referrer
    const CORE::CString relative = StripScheme( path );
    CORE::CString::StringVector candidates;
    if ( !relative.IsNULLOrEmpty() )
    {
        CORE::CString root;
        if ( FindProjectRoot( context, referrer, root ) )
            candidates.push_back( ASSETCONVERTER::CVfsPaths::Join( root, relative ) );
        candidates.push_back( relative );
        CORE::CString folder = ASSETCONVERTER::CVfsPaths::FolderOf( referrer );
        while ( !folder.IsNULLOrEmpty() )
        {
            candidates.push_back( ASSETCONVERTER::CVfsPaths::Join( folder, relative ) );
            folder = ASSETCONVERTER::CVfsPaths::FolderOf( folder );
        }
    }
    for ( size_t c=0; c<candidates.size(); ++c )
    {
        if ( context.HasFile( candidates[ c ] ) )
        {
            assetPath = candidates[ c ];
            return true;
        }
    }

    // the same places in another letter case
    if ( !candidates.empty() )
    {
        EnsureLowerIndex( context );
        for ( size_t c=0; c<candidates.size(); ++c )
        {
            if ( context.TryResolve( LowerIndexName, candidates[ c ].Lowercase(), assetPath ) )
                return true;
        }
    }

    // the uids of the files of the pack are read when no path leads to the file
    if ( !uid.IsNULLOrEmpty() )
    {
        IndexUids( context );
        if ( context.TryResolve( UidIndexName, uid, assetPath ) && context.HasFile( assetPath ) )
            return true;
    }
    if ( relative.IsNULLOrEmpty() )
        return false;

    // by the tail
    const CORE::CString::StringVector& files = context.GetFiles();
    const CORE::CString tail = "/" + relative;
    const CORE::CString lowerTail = tail.Lowercase();
    CORE::CString caseless;
    for ( size_t f=0; f<files.size(); ++f )
    {
        if ( files[ f ].EndsWith( tail ) )
        {
            assetPath = files[ f ];
            return true;
        }
        if ( caseless.IsNULLOrEmpty() && files[ f ].Lowercase().EndsWith( lowerTail ) )
            caseless = files[ f ];
    }
    if ( !caseless.IsNULLOrEmpty() )
    {
        assetPath = caseless;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CGodotPaths::EnsureLowerIndex( ASSETCONVERTER::CAssetPackContext& context )
{GUCEF_TRACE;

    CORE::CString built;
    if ( context.TryResolve( LowerIndexName, IndexBuiltKey, built ) )
        return;
    MT::CScopeMutex lock( g_indexLock );
    if ( context.TryResolve( LowerIndexName, IndexBuiltKey, built ) )
        return;
    const CORE::CString::StringVector files = context.GetFiles();
    for ( size_t f=0; f<files.size(); ++f )
        context.SetIndexEntry( LowerIndexName, files[ f ].Lowercase(), files[ f ] );
    context.SetIndexEntry( LowerIndexName, IndexBuiltKey, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotPaths::ReadHeaderLine( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr resource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( vfsPath, "rb", false );
    if ( resource.IsNULL() )
        return CORE::CString::Empty;
    CORE::IOAccessPtr access = resource->GetAccess();
    if ( access.IsNULL() )
        return CORE::CString::Empty;

    char head[ 513 ];
    const UInt32 read = access->Read( head, 1, 512 );
    head[ read ] = '\0';

    // a binary resource: its type and uid in the form of the first line of a text file
    CORE::CString type;
    CORE::CString uid;
    if ( CGodotBinaryReader::ReadHeader( head, read, type, uid ) )
        return "[gd_resource type=\"" + type + "\" format=3" + ( uid.IsNULLOrEmpty() ? CORE::CString::Empty : " uid=\"" + uid + "\"" ) + "]";

    CORE::CString text( head );
    const Int32 lineEnd = text.HasSubstr( "\n" );
    if ( lineEnd >= 0 )
        text = text.SubstrFromRange( 0, (UInt32) lineEnd );
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotPaths::ReadHeaderUid( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    const CORE::CString text = ReadHeaderLine( vfsPath );
    const Int32 start = text.HasSubstr( "uid=\"uid://" );
    if ( start < 0 )
        return CORE::CString::Empty;
    const CORE::CString rest = text.SubstrFromRange( (UInt32) start + 5, text.Length() );
    const Int32 end = rest.HasSubstr( "\"" );
    if ( end < 0 )
        return CORE::CString::Empty;
    return rest.SubstrFromRange( 0, (UInt32) end );
}

/*-------------------------------------------------------------------------*/

UInt32
CGodotPaths::IndexUids( ASSETCONVERTER::CAssetPackContext& context )
{GUCEF_TRACE;

    CORE::CString built;
    if ( context.TryResolve( UidIndexName, IndexBuiltKey, built ) )
        return 0;
    MT::CScopeMutex lock( g_indexLock );
    if ( context.TryResolve( UidIndexName, IndexBuiltKey, built ) )
        return 0;
    UInt32 count = 0;
    const CORE::CString::StringVector files = context.GetFiles();
    for ( size_t f=0; f<files.size(); ++f )
    {
        const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( files[ f ] );
        if ( "import" == extension )
        {
            CGodotDocument document;
            if ( !document.Load( context, files[ f ] ) )
                continue;
            const CORE::CDataNode* remap = document.FindSection( "remap" );
            if ( GUCEF_NULL == remap )
                continue;
            const CORE::CString uid = CGodotVariant::GetString( CGodotVariant::FindProperty( *remap, "uid" ) );
            if ( uid.IsNULLOrEmpty() )
                continue;
            context.SetIndexEntry( UidIndexName, uid, ASSETCONVERTER::CVfsPaths::WithoutExtension( files[ f ] ) );
            ++count;
        }
        else
        if ( "tscn" == extension || "tres" == extension || "res" == extension || "mesh" == extension )
        {
            const CORE::CString uid = ReadHeaderUid( context.ToVfsPath( files[ f ] ) );
            if ( !uid.IsNULLOrEmpty() )
            {
                context.SetIndexEntry( UidIndexName, uid, files[ f ] );
                ++count;
            }
        }
    }
    context.SetIndexEntry( UidIndexName, IndexBuiltKey, CORE::CString::Empty );
    return count;
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
