/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "assetcookerOGRE_COutputNames.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COutputNames::COutputNames( const CORE::CString& packId, const CORE::CString& assetPath )
    : m_folder()
    , m_stem()
    , m_files()
    , m_materials()
{GUCEF_TRACE;

    const CORE::CString pack = Sanitize( packId, "pack" );
    const CORE::CString asset = SanitizePath( ASSETCONVERTER::CVfsPaths::WithoutExtension( assetPath ) );
    m_folder = ASSETCONVERTER::CVfsPaths::Join( pack, asset.IsNULLOrEmpty() ? CORE::CString( "asset" ) : asset );
    m_stem = Sanitize( ASSETCONVERTER::CVfsPaths::StemOf( assetPath ), "asset" );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& COutputNames::GetAssetFolder( void ) const { return m_folder; }
const CORE::CString& COutputNames::GetAssetStem( void ) const { return m_stem; }

/*-------------------------------------------------------------------------*/

CORE::CString
COutputNames::Sanitize( const CORE::CString& name, const CORE::CString& fallback )
{GUCEF_TRACE;

    CORE::CString result;
    const char* c = name.C_String();
    for ( ; GUCEF_NULL != c && '\0' != *c; ++c )
    {
        const unsigned char ch = (unsigned char) *c;
        if ( ch < 32 || '"' == ch || '\\' == ch || '/' == ch || ':' == ch || '*' == ch || '?' == ch || '<' == ch || '>' == ch || '|' == ch || '&' == ch || '\'' == ch )
            result += '_';
        else
            result += *c;
    }
    result = result.Trim( true ).Trim( false );
    while ( result.Length() > 0 && '.' == result[ result.Length()-1 ] )
        result = result.CutChars( 1, false );
    if ( result.IsNULLOrEmpty() || "." == result || ".." == result )
        return fallback;
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COutputNames::SanitizePath( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString::StringVector parts = ASSETCONVERTER::CVfsPaths::Normalize( path ).ParseElements( '/', false );
    CORE::CString result;
    for ( size_t i=0; i<parts.size(); ++i )
        result = ASSETCONVERTER::CVfsPaths::Join( result, Sanitize( parts[ i ], "_" ) );
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COutputNames::MakeUnique( const CORE::CString& name, const CORE::CString& extension, CORE::CString::StringSet& used )
{GUCEF_TRACE;

    const CORE::CString suffix = extension.IsNULLOrEmpty() ? CORE::CString::Empty : "." + extension;
    CORE::CString candidate = name + suffix;
    UInt32 n = 2;
    while ( used.find( candidate.Lowercase() ) != used.end() )
        candidate = name + "_" + CORE::UInt32ToString( n++ ) + suffix;
    used.insert( candidate.Lowercase() );
    return candidate;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COutputNames::MakeFileName( const CORE::CString& name, const CORE::CString& extension, const CORE::CString& fallback )
{GUCEF_TRACE;

    return ASSETCONVERTER::CVfsPaths::Join( m_folder, MakeUnique( Sanitize( name, fallback ), extension, m_files ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COutputNames::MakeMaterialName( const CORE::CString& name, const CORE::CString& fallback )
{GUCEF_TRACE;

    return ASSETCONVERTER::CVfsPaths::Join( m_folder, MakeUnique( Sanitize( name, fallback ), CORE::CString::Empty, m_materials ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
