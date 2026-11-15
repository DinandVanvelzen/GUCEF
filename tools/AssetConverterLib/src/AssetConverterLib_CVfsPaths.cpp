/*
 *  AssetConverterLib: the path arithmetic of VFS paths
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

#include "AssetConverterLib_CVfsPaths.h"

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

CORE::CString
CVfsPaths::Normalize( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString result;
    const CORE::CString unified = path.ReplaceChar( '\\', '/' );
    const char* c = unified.C_String();
    bool lastWasSeparator = true;
    for ( ; GUCEF_NULL != c && '\0' != *c; ++c )
    {
        if ( '/' == *c )
        {
            if ( !lastWasSeparator )
                result += '/';
            lastWasSeparator = true;
            continue;
        }
        result += *c;
        lastWasSeparator = false;
    }
    while ( result.Length() > 0 && '/' == result[ result.Length()-1 ] )
        result = result.CutChars( 1, false );
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsPaths::Join( const CORE::CString& folder, const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString first = Normalize( folder );
    const CORE::CString second = Normalize( name );
    if ( first.IsNULLOrEmpty() )
        return second;
    if ( second.IsNULLOrEmpty() )
        return first;
    return first + "/" + second;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsPaths::FolderOf( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString normalized = Normalize( path );
    const Int32 slash = normalized.HasChar( '/', false );
    if ( slash < 0 )
        return CORE::CString::Empty;
    return normalized.SubstrFromRange( 0, (UInt32) slash );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsPaths::NameOf( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString normalized = Normalize( path );
    const Int32 slash = normalized.HasChar( '/', false );
    if ( slash < 0 )
        return normalized;
    return normalized.CutChars( (UInt32) slash+1, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsPaths::StemOf( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString name = NameOf( path );
    const Int32 dot = name.HasChar( '.', false );
    if ( dot <= 0 )
        return name;
    return name.SubstrFromRange( 0, (UInt32) dot );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsPaths::WithoutExtension( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString folder = FolderOf( path );
    return folder.IsNULLOrEmpty() ? StemOf( path ) : folder + "/" + StemOf( path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsPaths::ExtensionOf( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString name = NameOf( path );
    const Int32 dot = name.HasChar( '.', false );
    if ( dot <= 0 )
        return CORE::CString::Empty;
    return name.CutChars( (UInt32) dot+1, true ).Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CVfsPaths::MakeRelative( const CORE::CString& folder, const CORE::CString& path, CORE::CString& relative )
{GUCEF_TRACE;

    const CORE::CString base = Normalize( folder );
    const CORE::CString full = Normalize( path );
    if ( base.IsNULLOrEmpty() )
    {
        relative = full;
        return true;
    }
    if ( full.Length() <= base.Length() || full.SubstrFromRange( 0, base.Length() ) != base || '/' != full[ base.Length() ] )
        return false;
    relative = full.CutChars( base.Length()+1, true );
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
