/*
 *  assetpluginUNREAL: The Texture2D export: the properties and the place of the source image
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#include "assetpluginUNREAL_CUnrealObjectProperties.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#include "assetpluginUNREAL_CUnrealStripDataFlags.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#include "assetpluginUNREAL_CUnrealEditorBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#include "assetpluginUNREAL_CUnrealLegacyBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H
#include "assetpluginUNREAL_CUnrealTextureReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H ? */

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

CUnrealTextureReader::STextureSource::STextureSource( void )
    : name()
    , width( 0 )
    , height( 0 )
    , slices( 1 )
    , mips( 1 )
    , format()
    , compressionFormat()
    , srgb( true )
    , compressionSettings()
    , lodGroup()
    , flipGreenChannel( false )
    , hasPayload( false )
    , isLegacyPayload( false )
    , payload()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealTextureReader::STextureSource::GetBytesPerPixel( void ) const
{GUCEF_TRACE;

    if ( "TSF_G8" == format ) return 1;
    if ( "TSF_G16" == format || "TSF_R16F" == format ) return 2;
    if ( "TSF_BGRA8" == format || "TSF_RGBA8" == format || "TSF_BGRE8" == format || "TSF_RGBE8" == format || "TSF_R32F" == format ) return 4;
    if ( "TSF_RGBA16" == format || "TSF_RGBA16F" == format ) return 8;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealTextureReader::STextureSource::IsNormalMap( void ) const
{GUCEF_TRACE;

    return "TC_Normalmap" == compressionSettings;
}

/*-------------------------------------------------------------------------*/

CUnrealTextureReader::CUnrealTextureReader( void )
    : m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealTextureReader::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealTextureReader::Read( CUnrealPackage& package, UInt32 exportIndex, STextureSource& out )
{GUCEF_TRACE;

    out = STextureSource();
    m_error.Clear();
    if ( exportIndex >= package.GetExports().size() )
    {
        m_error = "The export does not exist";
        return false;
    }
    out.name = package.GetExports()[ exportIndex ].objectName;

    CUnrealArchiveReader reader;
    if ( !package.OpenExportRange( exportIndex, reader ) )
    {
        m_error = "The data of the texture export cannot be opened";
        return false;
    }
    CORE::CDataNode root( "root" );
    CUnrealPropertyReader properties( package );
    if ( !properties.ReadExport( reader, root, "Texture2D" ) )
    {
        m_error = "The properties of the texture cannot be read: " + reader.GetError();
        return false;
    }
    const CUnrealObjectProperties values( root );
    out.width = (UInt32) values.GetInt32( "Source/SizeX", 0 );
    out.height = (UInt32) values.GetInt32( "Source/SizeY", 0 );
    out.slices = (UInt32) values.GetInt32( "Source/NumSlices", 1 );
    out.mips = (UInt32) values.GetInt32( "Source/NumMips", 1 );
    out.format = values.GetString( "Source/Format", "TSF_BGRA8" );
    // the engines before UE 5.0 have a flag that says the source is a PNG file; later ones name the compression format
    out.compressionFormat = values.Has( "Source/CompressionFormat" ) ? values.GetString( "Source/CompressionFormat", "TSCF_None" ) : ( values.GetBool( "Source/bPNGCompressed", false ) ? CORE::CString( "TSCF_PNG" ) : CORE::CString( "TSCF_None" ) );
    out.srgb = values.GetBool( "SRGB", true );
    out.compressionSettings = values.GetString( "CompressionSettings", "TC_Default" );
    out.lodGroup = values.GetString( "LODGroup", "TEXTUREGROUP_World" );
    out.flipGreenChannel = values.GetBool( "bFlipGreenChannel", false );
    if ( 0 == out.width || 0 == out.height )
    {
        m_error = "The texture has no source image (its size is 0)";
        return false;
    }

    // the native data: the guid, the strip flags and the bulk data of the source
    if ( reader.ReadBool32() )
    {
        CUnrealGuid guid;
        reader.ReadGuid( guid );
    }
    CUnrealStripDataFlags strip;
    if ( !strip.Read( reader ) )
    {
        m_error = "The strip flags of the texture cannot be read: " + reader.GetError();
        return false;
    }
    if ( strip.IsEditorDataStripped() )
    {
        m_error = "The source image is stripped from the package (it holds no editor data)";
        return false;
    }
    const CUnrealVersionContext& versions = package.GetVersions();
    if ( versions.GetUe5Version() >= 1009 )
    {
        CUnrealEditorBulkData bulk;
        if ( !bulk.Read( reader, versions ) )
        {
            m_error = "The bulk data record of the source cannot be read: " + reader.GetError();
            return false;
        }
        bulk.ToReference( package.HasTrailer() ? &package.GetTrailer() : GUCEF_NULL, out.payload );
    }
    else
    {
        CUnrealLegacyBulkData bulk;
        if ( !bulk.Read( reader, versions ) )
        {
            m_error = "The bulk data header of the source cannot be read: " + reader.GetError();
            return false;
        }
        bulk.ToReference( package.GetSummary().bulkDataStartOffset, reader.GetPosition(), package.GetSource().GetSize(), out.payload );
        out.isLegacyPayload = true;
    }
    out.hasPayload = CUnrealPayloadReference::KIND_NONE != out.payload.kind;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
