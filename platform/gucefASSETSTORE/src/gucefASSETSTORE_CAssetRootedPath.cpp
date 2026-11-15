/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include <limits.h>
#include <stdlib.h>

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefASSETSTORE_CAssetRootedPath.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 MaxRootIdLength = 32;
const UInt32 MaxRelativePathLength = 1024;

/** Whether the kind is in the list of kinds separated by '|'; an empty list takes every kind */
bool
IsKindInList( const CORE::CString& kind, const CORE::CString& kinds )
{GUCEF_TRACE;

    if ( kinds.IsNULLOrEmpty() )
        return true;
    const CORE::CString::StringVector list = kinds.ParseElements( '|', false );
    for ( size_t i=0; i<list.size(); ++i )
        if ( list[ i ] == kind )
            return true;
    return false;
}

/**
 *  The path with every symbolic link resolved; of a path that does not exist (yet) the deepest folder that exists is
 *  resolved and the rest is put back behind it
 */
bool
ResolveOnDisk( const CORE::CString& path, CORE::CString& resolved )
{GUCEF_TRACE;

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    CORE::CString existing = path.ReplaceChar( '\\', '/' );
    while ( existing.Length() > 1 && '/' == existing[ existing.Length()-1 ] )
        existing = existing.CutChars( 1, false );
    CORE::CString rest;
    for ( UInt32 guard=0; guard<256; ++guard )
    {
        char buffer[ PATH_MAX ];
        if ( GUCEF_NULL != ::realpath( existing.C_String(), buffer ) )
        {
            resolved = buffer;
            if ( !rest.IsNULLOrEmpty() )
                resolved = ( '/' == resolved[ resolved.Length()-1 ] ? resolved : resolved + "/" ) + rest;
            return true;
        }
        const Int32 slash = existing.HasChar( '/', false );
        if ( slash <= 0 )
            return false;
        const CORE::CString name = existing.CutChars( (UInt32) slash + 1, true );
        rest = rest.IsNULLOrEmpty() ? name : name + "/" + rest;
        existing = existing.SubstrToIndex( (UInt32) slash, true );
    }
    return false;

    #else

    // no resolution of links here: the paths are compared as they are
    resolved = path.ReplaceChar( '\\', '/' );
    return true;

    #endif
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetRootedPath::CAssetRootedPath( void )
    : rootId()
    , relativePath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetRootedPath::IsValidRootId( const CORE::CString& id )
{GUCEF_TRACE;

    if ( id.IsNULLOrEmpty() || id.Length() > MaxRootIdLength )
        return false;
    const char first = id[ 0 ];
    if ( first < 'a' || first > 'z' )
        return false;
    for ( UInt32 i=1; i<id.Length(); ++i )
    {
        const char c = id[ i ];
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '-' == c ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetRootedPath::IsSafeRelativePath( const CORE::CString& path, bool allowEmpty, bool refuseLeadingDash, CORE::CString& problem )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
    {
        if ( allowEmpty )
            return true;
        problem = "the path is empty";
        return false;
    }
    if ( path.Length() > MaxRelativePathLength )
    {
        problem = "the path is longer than " + CORE::ToString( MaxRelativePathLength ) + " characters";
        return false;
    }
    for ( UInt32 i=0; i<path.Length(); ++i )
    {
        const unsigned char c = (unsigned char) path[ i ];
        if ( c < 0x20 || 0x7F == c )
        {
            problem = "the path holds a control character";
            return false;
        }
        if ( '\\' == c )
        {
            problem = "the path holds a backslash: use '/'";
            return false;
        }
        if ( '%' == c )
        {
            problem = "the path holds a percent sign: paths are given as they are, not percent-encoded";
            return false;
        }
    }
    if ( '/' == path[ 0 ] )
    {
        problem = "the path is absolute: give a path relative to its root";
        return false;
    }
    if ( path.Length() >= 2 && ':' == path[ 1 ] && ( ( path[ 0 ] >= 'a' && path[ 0 ] <= 'z' ) || ( path[ 0 ] >= 'A' && path[ 0 ] <= 'Z' ) ) )
    {
        problem = "the path starts with a drive letter";
        return false;
    }
    if ( refuseLeadingDash && '-' == path[ 0 ] )
    {
        problem = "the path starts with '-'";
        return false;
    }
    const CORE::CString::StringVector segments = path.ParseElements( '/', true );
    for ( size_t s=0; s<segments.size(); ++s )
    {
        if ( segments[ s ].IsNULLOrEmpty() )
        {
            problem = "the path has an empty segment";
            return false;
        }
        if ( ".." == segments[ s ] || "." == segments[ s ] )
        {
            problem = "the path has a \"" + segments[ s ] + "\" segment";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetRootedPath::Parse( const CORE::CString& text, CORE::CString& problem )
{GUCEF_TRACE;

    rootId.Clear();
    relativePath.Clear();
    const Int32 colon = text.HasChar( ':', true );
    if ( colon <= 0 )
    {
        problem = "\"" + text.SubstrToIndex( 64, true ) + "\" is no rooted path: give <rootId>:<relative path>";
        return false;
    }
    const CORE::CString id = text.SubstrToIndex( (UInt32) colon, true );
    const CORE::CString relative = text.CutChars( (UInt32) colon + 1, true );
    if ( !IsValidRootId( id ) )
    {
        problem = "the root id of the path does not match [a-z][a-z0-9-]{0,31}";
        return false;
    }
    if ( !IsSafeRelativePath( relative, true, false, problem ) )
        return false;
    rootId = id;
    relativePath = relative;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetRootedPath::ToString( void ) const
{GUCEF_TRACE;

    return rootId + ":" + relativePath;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetRootedPath::NormalizeVfsPath( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CString path = vfsPath.ReplaceChar( '\\', '/' );
    while ( path.Length() > 0 && '/' == path[ path.Length()-1 ] )
        path = path.CutChars( 1, false );
    while ( path.Length() > 0 && '/' == path[ 0 ] )
        path = path.CutChars( 1, true );
    return "/" + path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetRootedPath::JoinVfsPath( const CORE::CString& folder, const CORE::CString& relative )
{GUCEF_TRACE;

    const CORE::CString base = NormalizeVfsPath( folder );
    if ( relative.IsNULLOrEmpty() )
        return base;
    CORE::CString rest = relative.ReplaceChar( '\\', '/' );
    while ( rest.Length() > 0 && '/' == rest[ 0 ] )
        rest = rest.CutChars( 1, true );
    if ( "/" == base )
        return base + rest;
    return base + "/" + rest;
}

/*-------------------------------------------------------------------------*/

bool
CAssetRootedPath::FromVfsPath( const CAssetToolRoot& root, const CORE::CString& vfsPath, CAssetRootedPath& rooted )
{GUCEF_TRACE;

    const CORE::CString base = NormalizeVfsPath( root.vfsPath );
    const CORE::CString path = NormalizeVfsPath( vfsPath );
    rooted.rootId = root.id;
    if ( path == base )
    {
        rooted.relativePath.Clear();
        return true;
    }
    const CORE::CString prefix = "/" == base ? base : base + "/";
    if ( 0 != path.HasSubstr( prefix, true ) )
        return false;
    rooted.relativePath = path.CutChars( prefix.Length(), true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetRootedPath::IsInsideRootOnDisk( const CAssetToolRoot& root, const CORE::CString& relative, CORE::CString& problem )
{GUCEF_TRACE;

    if ( root.diskPath.IsNULLOrEmpty() )
        return true;
    CORE::CString rootResolved;
    if ( !ResolveOnDisk( CORE::RelativePath( root.diskPath ), rootResolved ) )
    {
        problem = "the folder of the root " + root.id + " cannot be found";
        return false;
    }
    CORE::CString candidate = CORE::RelativePath( root.diskPath ).ReplaceChar( '\\', '/' );
    if ( !relative.IsNULLOrEmpty() )
        candidate = ( candidate.Length() > 0 && '/' == candidate[ candidate.Length()-1 ] ? candidate : candidate + "/" ) + relative;
    CORE::CString candidateResolved;
    if ( !ResolveOnDisk( candidate, candidateResolved ) )
    {
        problem = "the path cannot be resolved in the root " + root.id;
        return false;
    }
    while ( rootResolved.Length() > 1 && '/' == rootResolved[ rootResolved.Length()-1 ] )
        rootResolved = rootResolved.CutChars( 1, false );
    if ( candidateResolved == rootResolved )
        return true;
    const CORE::CString prefix = "/" == rootResolved ? rootResolved : rootResolved + "/";
    if ( 0 == candidateResolved.HasSubstr( prefix, true ) )
        return true;
    problem = "the path leads out of the root " + root.id + " (a symbolic link)";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetRootedPath::Resolve( const TAssetToolRootVector& roots   ,
                           const CORE::CString& kinds          ,
                           const CAssetToolRoot*& root         ,
                           CORE::CString& vfsPath              ,
                           CORE::CString& problem              ) const
{GUCEF_TRACE;

    root = CAssetToolRoot::Find( roots, rootId );
    if ( GUCEF_NULL == root )
    {
        problem = "there is no root \"" + rootId + "\"";
        return false;
    }
    if ( !IsKindInList( root->kind, kinds ) )
    {
        problem = "the root " + rootId + " is a " + root->kind + " root; this takes a path in a root of the kind " + kinds.ReplaceChar( '|', '/' );
        root = GUCEF_NULL;
        return false;
    }
    CORE::CString check;
    if ( !IsSafeRelativePath( relativePath, true, false, check ) )
    {
        problem = check;
        root = GUCEF_NULL;
        return false;
    }
    if ( !IsInsideRootOnDisk( *root, relativePath, problem ) )
    {
        root = GUCEF_NULL;
        return false;
    }
    vfsPath = JoinVfsPath( root->vfsPath, relativePath );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetRootedPath::ParseAndResolve( const CORE::CString& text            ,
                                   const TAssetToolRootVector& roots    ,
                                   const CORE::CString& kinds           ,
                                   CAssetRootedPath& rooted             ,
                                   const CAssetToolRoot*& root          ,
                                   CORE::CString& vfsPath               ,
                                   CORE::CString& problem               )
{GUCEF_TRACE;

    root = GUCEF_NULL;
    if ( !rooted.Parse( text, problem ) )
        return false;
    return rooted.Resolve( roots, kinds, root, vfsPath, problem );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
