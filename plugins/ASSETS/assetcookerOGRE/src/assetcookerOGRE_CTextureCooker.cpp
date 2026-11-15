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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#include "gucefIMAGE_CPixelMapResampler.h"
#define GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#endif /* GUCEF_IMAGE_CPIXELMAPRESAMPLER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "assetcookerOGRE_COutputNames.h"
#include "assetcookerOGRE_CTextureCooker.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "ogre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTextureCooker::CTextureCooker( const CORE::CString& profileDir      ,
                                const CORE::CString& packId          ,
                                const CORE::CString& intermediateDir ,
                                const CORE::CString& sourceRoot      ,
                                const ASSETCONVERTER::CCookProfile& profile )
    : m_profileDir( ASSETCONVERTER::CVfsPaths::Normalize( profileDir ) )
    , m_packId( COutputNames::Sanitize( packId, "pack" ) )
    , m_intermediateDir( ASSETCONVERTER::CVfsPaths::Normalize( intermediateDir ) )
    , m_sourceRoot( ASSETCONVERTER::CVfsPaths::Normalize( sourceRoot ) )
    , m_profile( profile )
    , m_baker( ASSETCONVERTER::CVfsPaths::Join( ASSETCONVERTER::CVfsPaths::Join( profileDir, COutputNames::Sanitize( packId, "pack" ) ), "textures/derived" ) )
    , m_cooked()
    , m_owners()
    , m_written( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTextureCooker::~CTextureCooker()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CTextureCooker::ToVfsPath( const CORE::CString& name ) const
{GUCEF_TRACE;

    return ASSETCONVERTER::CVfsPaths::Join( m_profileDir, name );
}

/*-------------------------------------------------------------------------*/

UInt32
CTextureCooker::GetWrittenCount( void ) const
{GUCEF_TRACE;

    return m_written;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTextureCooker::GetOutputName( const CORE::CString& sourcePath ) const
{GUCEF_TRACE;

    CORE::CString relative;
    if ( !m_intermediateDir.IsNULLOrEmpty() && ASSETCONVERTER::CVfsPaths::MakeRelative( m_intermediateDir, sourcePath, relative ) )
        return ASSETCONVERTER::CVfsPaths::Join( m_packId, COutputNames::SanitizePath( relative ) );
    if ( !m_sourceRoot.IsNULLOrEmpty() && ASSETCONVERTER::CVfsPaths::MakeRelative( m_sourceRoot, sourcePath, relative ) )
        return ASSETCONVERTER::CVfsPaths::Join( m_packId, COutputNames::SanitizePath( relative ) );
    return ASSETCONVERTER::CVfsPaths::Join( m_packId, "textures/" + COutputNames::Sanitize( ASSETCONVERTER::CVfsPaths::NameOf( sourcePath ), "texture" ) );
}

/*-------------------------------------------------------------------------*/

bool
CTextureCooker::FitToProfile( const CORE::CString& vfsPath, const ASSETCONVERTER::CAssetTexture& texture, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    if ( 0 == m_profile.maxTextureSize )
        return true;
    IMAGE::TPixelMapPtr picture;
    if ( !ASSETCONVERTER::CTextureBaker::LoadPicture( vfsPath, picture ) || picture.IsNULL() )
        return false;
    const UInt32 width = picture->GetWidthInPixels();
    const UInt32 height = picture->GetHeightInPixels();
    const UInt32 largest = width > height ? width : height;
    if ( largest <= m_profile.maxTextureSize )
        return true;
    const Float64 factor = (Float64) m_profile.maxTextureSize / (Float64) largest;
    UInt32 newWidth = (UInt32) ( width * factor + 0.5 );
    UInt32 newHeight = (UInt32) ( height * factor + 0.5 );
    if ( 0 == newWidth ) newWidth = 1;
    if ( 0 == newHeight ) newHeight = 1;
    IMAGE::TPixelMapPtr resampled;
    if ( !IMAGE::CPixelMapResampler::Resample( *picture, newWidth, newHeight, IMAGE::RESAMPLEFILTER_KAISER, texture.GetColourSpace(), texture.wrapS, resampled ) ||
         resampled.IsNULL() || !ASSETCONVERTER::CTextureBaker::SavePng( vfsPath, *resampled ) )
    {
        report.AddError( SourceName, texture.name, "the texture cannot be brought to the largest size of the profile" );
        return false;
    }
    report.AddInfo( SourceName, texture.name, "scaled from " + CORE::UInt32ToString( width ) + "x" + CORE::UInt32ToString( height ) + " to " +
                    CORE::UInt32ToString( newWidth ) + "x" + CORE::UInt32ToString( newHeight ) + " for profile " + m_profile.name );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTextureCooker::Cook( const ASSETCONVERTER::CAssetTexture& texture, CORE::CString& name, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    if ( texture.sources.empty() || texture.sources[ 0 ].IsNULLOrEmpty() )
    {
        report.AddWarning( SourceName, texture.name, "the texture has no file: the material is cooked without it" );
        return false;
    }
    const CORE::CString source = ASSETCONVERTER::CVfsPaths::Normalize( texture.sources[ 0 ] );
    TStringMap::const_iterator done = m_cooked.find( source );
    if ( done != m_cooked.end() )
    {
        name = (*done).second;
        return true;
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.FileExists( source ) )
    {
        report.AddWarning( SourceName, texture.name, "the file " + source + " of the texture is not there: the material is cooked without it" );
        return false;
    }

    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( source );
    bool copyAsIs = "png" == extension || "jpg" == extension || "jpeg" == extension;
    if ( copyAsIs && m_profile.maxTextureSize > 0 )
    {
        // a file larger than the profile allows is written resampled, as PNG
        IMAGE::TPixelMapPtr picture;
        if ( ASSETCONVERTER::CTextureBaker::LoadPicture( source, picture ) && !picture.IsNULL() &&
             ( picture->GetWidthInPixels() > m_profile.maxTextureSize || picture->GetHeightInPixels() > m_profile.maxTextureSize ) )
            copyAsIs = false;
    }
    CORE::CString outputName = GetOutputName( source );
    if ( !copyAsIs )
        outputName = ASSETCONVERTER::CVfsPaths::WithoutExtension( outputName ) + ".png";

    // two different sources could map to one name (a.tga next to a.png): the later one keeps its own extension in the name
    TStringMap::const_iterator owner = m_owners.find( outputName.Lowercase() );
    if ( owner != m_owners.end() && (*owner).second != source )
        outputName = ASSETCONVERTER::CVfsPaths::WithoutExtension( outputName ) + "_" + extension + ".png";

    const CORE::CString format = m_profile.GetTextureFormat( texture.usage );
    if ( !format.Equals( "png", false ) )
        report.AddApproximation( SourceName, texture.name, "the profile " + m_profile.name + " asks for " + format + ", which is not made yet: written as " + ( copyAsIs ? extension : CORE::CString( "png" ) ) );

    const CORE::CString target = ToVfsPath( outputName );
    bool ok = false;
    if ( copyAsIs )
    {
        CORE::CDynamicBuffer bytes;
        vfs.CreateDir( ASSETCONVERTER::CVfsPaths::FolderOf( target ) );
        ok = vfs.LoadFile( bytes, source, "rb" ) && vfs.StoreAsFile( target, bytes, 0, true );
    }
    else
    {
        IMAGE::TPixelMapPtr picture;
        ok = ASSETCONVERTER::CTextureBaker::LoadPicture( source, picture ) && !picture.IsNULL() && ASSETCONVERTER::CTextureBaker::SavePng( target, *picture );
        if ( ok )
            report.AddInfo( SourceName, texture.name, "written as PNG from " + extension );
    }
    if ( !ok )
    {
        report.AddError( SourceName, texture.name, "the texture " + source + " cannot be written to " + target );
        return false;
    }
    FitToProfile( target, texture, report );

    ++m_written;
    m_cooked[ source ] = outputName;
    m_owners[ outputName.Lowercase() ] = source;
    name = outputName;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTextureCooker::CookDerived( const ASSETCONVERTER::CAssetTexture& derived, CORE::CString& name, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetTexture baked( derived );
    baked.bakedPath.Clear();
    if ( !m_baker.Bake( baked, report ) || baked.missing || baked.bakedPath.IsNULLOrEmpty() )
    {
        report.AddWarning( SourceName, derived.name, "the derived texture cannot be made" );
        return false;
    }
    const CORE::CString path = ASSETCONVERTER::CVfsPaths::Normalize( baked.bakedPath );
    CORE::CString relative;
    if ( !ASSETCONVERTER::CVfsPaths::MakeRelative( m_profileDir, path, relative ) )
        return false;
    TStringMap::const_iterator done = m_cooked.find( path );
    if ( done == m_cooked.end() )
    {
        FitToProfile( path, baked, report );
        m_cooked[ path ] = relative;
        m_owners[ relative.Lowercase() ] = path;
        ++m_written;
    }
    name = relative;
    return true;
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
