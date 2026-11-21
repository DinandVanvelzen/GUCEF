/*
 *  assetpluginUNREAL: The root asset of a MetaHuman character package: properties and the texture payloads
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#include "assetpluginUNREAL_CUnrealEditorBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#include "assetpluginUNREAL_CUnrealCompressedBufferHeader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#include "assetpluginUNREAL_CUnrealCompressedBuffer.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#include "assetpluginUNREAL_CUnrealPngWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#include "assetpluginUNREAL_CUnrealMetaHumanCharacter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H ? */

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

/** A PNG file: the signature, then the chunk IHDR with the sizes in big endian */
static bool
ReadPngHeader( const UInt8* bytes, UInt32 size, UInt32& width, UInt32& height )
{GUCEF_TRACE;

    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    if ( size < 24 || 0 != memcmp( bytes, signature, 8 ) || 'I' != bytes[ 12 ] || 'H' != bytes[ 13 ] || 'D' != bytes[ 14 ] || 'R' != bytes[ 15 ] )
        return false;
    width = ( (UInt32) bytes[ 16 ] << 24 ) | ( (UInt32) bytes[ 17 ] << 16 ) | ( (UInt32) bytes[ 18 ] << 8 ) | bytes[ 19 ];
    height = ( (UInt32) bytes[ 20 ] << 24 ) | ( (UInt32) bytes[ 21 ] << 16 ) | ( (UInt32) bytes[ 22 ] << 8 ) | bytes[ 23 ];
    return true;
}

/*-------------------------------------------------------------------------*/

/** "EFaceTextureType::Basecolor" gives "Basecolor" */
static CORE::CString
WithoutEnumType( const CORE::CString& value )
{GUCEF_TRACE;

    const Int32 colons = value.HasSubstr( "::" );
    return colons < 0 ? value : value.SubstrFromRange( (UInt32) colons + 2, value.Length() );
}

/*-------------------------------------------------------------------------*/

/** Copies the values of a property tree: scalars become attributes, structs and containers children; the type texts and the object paths stay out */
static void
CopySettings( const CORE::CDataNode& from, CORE::CDataNode& to, UInt32 depth, UInt32& budget )
{GUCEF_TRACE;

    if ( depth > 10 )
        return;
    for ( CORE::CDataNode::const_iterator i = from.ConstBegin(); i != from.ConstEnd(); ++i )
    {
        const CORE::CDataNode* child = *i;
        if ( 0 == budget )
            return;
        --budget;
        if ( child->HasChildren() || child->GetAttCount() > 1 || ( 1 == child->GetAttCount() && GUCEF_NULL == child->GetAttribute( "type" ) ) )
        {
            CORE::CDataNode* copy = to.AddChild( child->GetName() );
            // the components of a binary struct (x, y, z, r, g, b, a ...) stay attributes
            for ( CORE::CDataNode::TAttributeMap::const_iterator a = child->AttributeBegin(); a != child->AttributeEnd(); ++a )
                if ( "type" != (*a).first && "index" != (*a).first )
                    copy->SetAttribute( (*a).first, (*a).second );
            if ( !child->GetValue().IsNULLOrEmpty() )
                copy->SetAttribute( "value", child->GetValue() );
            CopySettings( *child, *copy, depth + 1, budget );
        }
        else
            to.SetAttribute( child->GetName(), child->GetValue() );
    }
}

/*-------------------------------------------------------------------------*/

CUnrealMetaHumanCharacter::STexture::STexture( void )
    : group()
    , type()
    , width( 0 )
    , height( 0 )
    , slices( 0 )
    , format( 0 )
    , gammaSpace( 0 )
    , record( -1 )
    , hasPayload( false )
    , payload()
    , method( -1 )
    , isPng( false )
    , pngWidth( 0 )
    , pngHeight( 0 )
    , pngSize( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealMetaHumanCharacter::STexture::IsExtractable( void ) const
{GUCEF_TRACE;

    return hasPayload && isPng;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealMetaHumanCharacter::STexture::IsOodle( void ) const
{GUCEF_TRACE;

    return hasPayload && !isPng && 3 == method;
}

/*-------------------------------------------------------------------------*/

CUnrealMetaHumanCharacter::SRecord::SRecord( void )
    : payload()
    , method( -1 )
    , isPng( false )
    , pngWidth( 0 )
    , pngHeight( 0 )
    , pngSize( 0 )
    , texture( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealMetaHumanCharacter::CUnrealMetaHumanCharacter( void )
    : name()
    , packageName()
    , nameOverride()
    , cameraFrame()
    , characterEnvironment()
    , previewMaterial()
    , hasLightRotation( false )
    , lightRotation( 0.0f )
    , hasHighResolutionTextures( false )
    , hasFaceDnaBlendshapes( false )
    , wardrobeItems()
    , presetCharacters()
    , settings( "settings" )
    , textures()
    , records()
    , payloadsAssigned( 0 )
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealMetaHumanCharacter::Clear( void )
{GUCEF_TRACE;

    name.Clear();
    packageName.Clear();
    nameOverride.Clear();
    cameraFrame.Clear();
    characterEnvironment.Clear();
    previewMaterial.Clear();
    hasLightRotation = false;
    lightRotation = 0.0f;
    hasHighResolutionTextures = false;
    hasFaceDnaBlendshapes = false;
    wardrobeItems.clear();
    presetCharacters.clear();
    settings.Clear();
    settings.SetName( "settings" );
    textures.clear();
    records.clear();
    payloadsAssigned = 0;
    m_error.Clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealMetaHumanCharacter::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

/** The entries of a map property: the key text and the value node of every element */
static void
AddTextures( const CORE::CDataNode* map, const char* group, CUnrealMetaHumanCharacter::TTextureVector& textures )
{GUCEF_TRACE;

    if ( GUCEF_NULL == map )
        return;
    for ( CORE::CDataNode::const_iterator i = map->ConstBegin(); i != map->ConstEnd(); ++i )
    {
        const CORE::CDataNode* key = (*i)->FindChild( "key" );
        const CORE::CDataNode* value = (*i)->FindChild( "value" );
        if ( GUCEF_NULL == key )
            continue;
        CUnrealMetaHumanCharacter::STexture texture;
        texture.group = group;
        texture.type = WithoutEnumType( key->GetValue().AsString() );
        if ( GUCEF_NULL != value )
        {
            const CUnrealObjectProperties info( *value );
            texture.width = (UInt32) info.GetInt32( "SizeX", 0 );
            texture.height = (UInt32) info.GetInt32( "SizeY", 0 );
            texture.slices = (UInt32) info.GetInt32( "NumSlices", 1 );
            texture.format = info.GetInt32( "Format", 0 );
            texture.gammaSpace = info.GetInt32( "GammaSpace", 0 );
        }
        textures.push_back( texture );
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnrealMetaHumanCharacter::Read( CUnrealPackage& package, UInt32 exportIndex )
{GUCEF_TRACE;

    Clear();
    if ( exportIndex >= package.GetExports().size() )
    {
        m_error = "The export does not exist";
        return false;
    }
    name = package.GetExports()[ exportIndex ].objectName;
    packageName = package.GetSummary().packageName;

    CUnrealArchiveReader reader;
    if ( !package.OpenExportRange( exportIndex, reader ) )
    {
        m_error = "The data of the character export cannot be opened";
        return false;
    }
    CORE::CDataNode root( "root" );
    CUnrealPropertyReader properties( package );
    if ( !properties.ReadExport( reader, root, "MetaHumanCharacter" ) )
    {
        m_error = "The properties of the character cannot be read: " + reader.GetError();
        return false;
    }
    const CUnrealObjectProperties values( root );

    nameOverride = values.GetString( "AssemblySettings/NameOverride" );
    cameraFrame = WithoutEnumType( values.GetString( "ViewportSettings/CameraFrame" ) );
    characterEnvironment = WithoutEnumType( values.GetString( "ViewportSettings/CharacterEnvironment" ) );
    previewMaterial = WithoutEnumType( values.GetString( "PreviewMaterialType" ) );
    hasLightRotation = values.Has( "ViewportSettings/LightRotation" );
    lightRotation = (Float32) values.GetDouble( "ViewportSettings/LightRotation", 0.0 );
    hasHighResolutionTextures = values.GetBool( "bHasHighResolutionTextures", false );
    hasFaceDnaBlendshapes = values.GetBool( "bHasFaceDNABlendshapes", false );

    // the wardrobe: the items of every group, and the presets of the library
    const CORE::CDataNode* wardrobe = values.Find( "WardrobeIndividualAssets" );
    if ( GUCEF_NULL != wardrobe )
        for ( CORE::CDataNode::const_iterator g = wardrobe->ConstBegin(); g != wardrobe->ConstEnd(); ++g )
        {
            const CORE::CDataNode* items = (*g)->FindChild( "value" ) ? (*g)->FindChild( "value" )->FindChild( "Items" ) : GUCEF_NULL;
            if ( GUCEF_NULL != items )
                for ( CORE::CDataNode::const_iterator i = items->ConstBegin(); i != items->ConstEnd(); ++i )
                    wardrobeItems.push_back( (*i)->GetValue().AsString() );
        }
    const CORE::CDataNode* library = values.Find( "CharacterIndividualAssets" );
    if ( GUCEF_NULL != library )
        for ( CORE::CDataNode::const_iterator g = library->ConstBegin(); g != library->ConstEnd(); ++g )
        {
            const CORE::CDataNode* items = (*g)->FindChild( "value" ) ? (*g)->FindChild( "value" )->FindChild( "Characters" ) : GUCEF_NULL;
            if ( GUCEF_NULL != items )
                for ( CORE::CDataNode::const_iterator i = items->ConstBegin(); i != items->ConstEnd(); ++i )
                    presetCharacters.push_back( (*i)->GetValue().AsString() );
        }

    // the settings that describe the person: skin, eyes, makeup and the head model
    static const char* const settingNames[] = { "SkinSettings", "EyesSettings", "MakeupSettings", "HeadModelSettings", GUCEF_NULL };
    UInt32 budget = 4000;
    for ( UInt32 s=0; GUCEF_NULL != settingNames[ s ]; ++s )
    {
        const CORE::CDataNode* node = values.Find( settingNames[ s ] );
        if ( GUCEF_NULL == node )
            continue;
        CORE::CDataNode* copy = settings.AddChild( settingNames[ s ] );
        CopySettings( *node, *copy, 0, budget );
    }

    // the textures, in the order of the two maps
    AddTextures( values.Find( "SynthesizedFaceTexturesInfo" ), "face", textures );
    AddTextures( values.Find( "HighResBodyTexturesInfo" ), "body", textures );

    // the native data: the flag of the object guid, then editor bulk data records
    if ( reader.ReadBool32() )
    {
        CUnrealGuid guid;
        reader.ReadGuid( guid );
    }
    const CUnrealPackageTrailer* trailer = package.HasTrailer() ? &package.GetTrailer() : GUCEF_NULL;
    while ( !reader.HasError() && reader.GetRemaining() >= 48 )
    {
        CUnrealEditorBulkData bulk;
        if ( !bulk.Read( reader, package.GetVersions() ) )
            break;
        SRecord record;
        bulk.ToReference( trailer, record.payload );
        if ( record.payload.IsReadable() )
        {
            // the header of the compressed buffer tells how the payload is stored; a stored one is a PNG file
            CUnrealArchiveReader payload;
            CUnrealCompressedBufferHeader header;
            if ( package.OpenPayload( record.payload, payload ) && header.Read( payload, record.payload.storedSize ) )
            {
                record.method = (Int32) header.method;
                if ( header.IsStored() && header.rawSize >= 24 )
                {
                    UInt8 start[ 24 ];
                    if ( payload.Read( start, sizeof( start ) ) && ReadPngHeader( start, sizeof( start ), record.pngWidth, record.pngHeight ) )
                    {
                        record.isPng = true;
                        record.pngSize = header.rawSize;
                    }
                }
            }
        }
        records.push_back( record );
    }
    m_error.Clear();
    AssignPayloads();
    return true;
}

/*-------------------------------------------------------------------------*/

/** The order in which the packages store the pixels of the face textures (the order of the enumeration, which is not the order of the property map) */
static const char* const FaceStorageOrder[] =
{
    "Basecolor", "Basecolor_Animated_CM1", "Basecolor_Animated_CM2", "Basecolor_Animated_CM3",
    "Normal", "Normal_Animated_WM1", "Normal_Animated_WM2", "Normal_Animated_WM3", "Cavity", GUCEF_NULL
};

/** The order for the body textures: the same as the property map */
static const char* const BodyStorageOrder[] =
{
    "Body_Basecolor", "Body_Normal", "Body_Cavity", "Body_Underwear_Basecolor", "Body_Underwear_Normal", "Body_Underwear_Mask",
    "Chest_Basecolor", "Chest_Normal", "Chest_Cavity", "Chest_Underwear_Basecolor", "Chest_Underwear_Normal", GUCEF_NULL
};

/*-------------------------------------------------------------------------*/

void
CUnrealMetaHumanCharacter::AssignPayloads( void )
{GUCEF_TRACE;

    // The native data holds a few records of state data (the definition of face and body) and then the textures: the face textures
    // in the order of the face enumeration, the body textures in the order of the body enumeration. Verified on seven packages of
    // two engine releases: wherever a payload is a stored PNG file, its sizes are those of the texture information of that texture.
    // Only a complete set of known texture types is assigned, and only when every PNG payload has the sizes the properties state.
    static const UInt32 LeadingStateRecords = 4;
    GUCEF::vector< size_t > order;
    for ( UInt32 g=0; g<2; ++g )
    {
        const char* const* names = 0 == g ? FaceStorageOrder : BodyStorageOrder;
        const char* group = 0 == g ? "face" : "body";
        UInt32 known = 0;
        UInt32 inGroup = 0;
        for ( size_t t=0; t<textures.size(); ++t )
            if ( textures[ t ].group == group )
                ++inGroup;
        for ( UInt32 n=0; GUCEF_NULL != names[ n ]; ++n )
            for ( size_t t=0; t<textures.size(); ++t )
                if ( textures[ t ].group == group && textures[ t ].type == names[ n ] )
                {
                    order.push_back( t );
                    ++known;
                    break;
                }
        if ( known != inGroup )
            return;
    }
    if ( LeadingStateRecords + order.size() > records.size() )
        return;
    for ( size_t i=0; i<order.size(); ++i )
    {
        const SRecord& record = records[ LeadingStateRecords + i ];
        const STexture& texture = textures[ order[ i ] ];
        if ( record.isPng && ( record.pngWidth != texture.width || record.pngHeight != texture.height ) )
            return;
    }
    for ( size_t i=0; i<order.size(); ++i )
    {
        SRecord& record = records[ LeadingStateRecords + i ];
        STexture& texture = textures[ order[ i ] ];
        record.texture = (Int32) order[ i ];
        texture.record = (Int32) ( LeadingStateRecords + i );
        texture.hasPayload = record.payload.IsReadable();
        texture.payload = record.payload;
        texture.method = record.method;
        texture.isPng = record.isPng;
        texture.pngWidth = record.pngWidth;
        texture.pngHeight = record.pngHeight;
        texture.pngSize = record.pngSize;
        ++payloadsAssigned;
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnrealMetaHumanCharacter::ReadPng( CUnrealPackage& package, const STexture& texture, CORE::CDynamicBuffer& png ) const
{GUCEF_TRACE;

    if ( !texture.IsExtractable() )
        return false;
    CUnrealArchiveReader payload;
    CUnrealCompressedBufferHeader header;
    if ( !package.OpenPayload( texture.payload, payload ) || !header.Read( payload, texture.payload.storedSize ) || !header.IsStored() || header.rawSize > 0xFFFFFFF0ULL )
        return false;
    png.SetDataSize( (UInt32) header.rawSize );
    return payload.Read( png.GetBufferPtr(), header.rawSize );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealMetaHumanCharacter::ReadPngThrough( CUnrealPackage& package, const STexture& texture, CIUnrealOodleProvider* provider, const ASSETCONVERTER::CConversionReport* cancellation, CORE::CDynamicBuffer& png, CORE::CString& error, bool& providerMissing ) const
{GUCEF_TRACE;

    providerMissing = false;
    if ( !texture.hasPayload )
    {
        error = "The texture has no payload";
        return false;
    }
    CUnrealCompressedBuffer buffer( package, provider );
    buffer.SetCancellationSource( cancellation );
    if ( !buffer.Open( texture.payload ) || !buffer.ReadAll( png ) )
    {
        error = buffer.GetError();
        providerMissing = buffer.IsProviderMissing();
        return false;
    }
    UInt32 width = 0, height = 0, channels = 0, bits = 0;
    if ( !CUnrealPngWriter::ReadPngInfo( static_cast< const UInt8* >( png.GetConstBufferPtr() ), png.GetDataSize(), width, height, channels, bits ) )
    {
        error = "The payload is not a PNG file";
        return false;
    }
    if ( width != texture.width || height != texture.height )
    {
        error = "The PNG file is " + CORE::UInt32ToString( width ) + " x " + CORE::UInt32ToString( height ) + " where the texture information says " + CORE::UInt32ToString( texture.width ) + " x " + CORE::UInt32ToString( texture.height );
        return false;
    }
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
