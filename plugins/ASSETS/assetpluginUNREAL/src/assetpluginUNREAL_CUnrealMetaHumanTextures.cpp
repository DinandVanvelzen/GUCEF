/*
 *  assetpluginUNREAL: The textures of a MetaHuman character as images of the scene, and the record of the character as extras
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

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H
#include "assetpluginUNREAL_CUnrealMetaHumanTextures.h"

#include <math.h>
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANTEXTURES_H ? */

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

static const char* const SourceName = "unreal";

/*-------------------------------------------------------------------------*/

static ASSETCONVERTER::TTextureUsage
UsageOf( const CUnrealMetaHumanCharacter::STexture& texture )
{GUCEF_TRACE;

    if ( texture.type.HasSubstr( "Normal" ) >= 0 )
        return ASSETCONVERTER::TEXTUREUSAGE_NORMAL;
    if ( texture.type.HasSubstr( "Cavity" ) >= 0 || texture.type.HasSubstr( "Mask" ) >= 0 )
        return ASSETCONVERTER::TEXTUREUSAGE_MASK;
    return ASSETCONVERTER::TEXTUREUSAGE_COLOUR;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
TextureName( const CORE::CString& characterName, const CUnrealMetaHumanCharacter::STexture& texture )
{GUCEF_TRACE;

    CORE::CString group = texture.group;
    if ( group.Length() > 0 )
        group = group.SubstrFromRange( 0, 1 ).Uppercase() + group.SubstrFromRange( 1, group.Length() );
    return characterName + "_" + group + "_" + texture.type;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealMetaHumanTextures::AddToScene( CUnrealPackage& package                       ,
                                      const CUnrealMetaHumanCharacter& character    ,
                                      const CORE::CString& intermediateFolder       ,
                                      const CORE::CString& assetName                ,
                                      const CORE::CString& characterName            ,
                                      ASSETCONVERTER::CAssetScene& scene            ,
                                      ASSETCONVERTER::CConversionReport& report     ,
                                      GUCEF::vector< Int32 >& textureIndices        ,
                                      CIUnrealOodleProvider* provider               )
{GUCEF_TRACE;

    textureIndices.assign( character.textures.size(), -1 );
    UInt32 extractable = 0;
    UInt32 oodle = 0;
    UInt32 added = 0;
    const bool oodleReadable = GUCEF_NULL != provider && provider->IsAvailable();
    for ( size_t t=0; t<character.textures.size(); ++t )
    {
        const CUnrealMetaHumanCharacter::STexture& texture = character.textures[ t ];
        if ( texture.IsExtractable() || ( oodleReadable && texture.IsOodle() ) )
            ++extractable;
        else if ( texture.hasPayload && 3 == texture.method )
            ++oodle;
    }
    const CORE::CString oodleReason = GUCEF_NULL != provider && !oodleReadable ? provider->GetUnavailableReason() : CORE::CString( "their payloads are compressed with Oodle, which this build cannot decompress" );
    if ( 0 == extractable )
    {
        if ( oodle > 0 )
            report.AddSkipped( SourceName, characterName, CORE::UInt32ToString( oodle ) + " textures of the character need the oodle provider: " + oodleReason );
        return 0;
    }
    if ( intermediateFolder.IsNULLOrEmpty() )
    {
        report.AddWarning( SourceName, characterName, CORE::UInt32ToString( extractable ) + " textures are stored as PNG files in the character asset and the manifest has no intermediate folder to extract them to" );
        return 0;
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = ASSETCONVERTER::CVfsPaths::Join( intermediateFolder, "embedded/" + assetName );
    vfs.CreateDir( folder );
    for ( size_t t=0; t<character.textures.size(); ++t )
    {
        const CUnrealMetaHumanCharacter::STexture& texture = character.textures[ t ];
        if ( !texture.IsExtractable() && !( oodleReadable && texture.IsOodle() ) )
            continue;
        CORE::CDynamicBuffer png;
        if ( texture.IsExtractable() )
        {
            if ( !character.ReadPng( package, texture, png ) )
            {
                report.AddWarning( SourceName, TextureName( characterName, texture ), "The PNG file of the texture cannot be read from the package" );
                continue;
            }
        }
        else
        {
            CORE::CString error;
            bool providerMissing = false;
            if ( !character.ReadPngThrough( package, texture, provider, &report, png, error, providerMissing ) )
            {
                if ( providerMissing )
                    report.AddSkipped( SourceName, TextureName( characterName, texture ), error );
                else
                    report.AddWarning( SourceName, TextureName( characterName, texture ), "The PNG file of the texture cannot be read: " + error );
                continue;
            }
        }
        const CORE::CString name = TextureName( characterName, texture );
        const CORE::CString path = ASSETCONVERTER::CVfsPaths::Join( folder, name + ".png" );
        if ( !vfs.StoreAsFile( path, png, 0, true ) )
        {
            report.AddWarning( SourceName, name, "The PNG file cannot be written to " + path );
            continue;
        }
        report.AddOutputFile( path, CORE::CString::StringVector() );
        ASSETCONVERTER::CAssetTexture assetTexture;
        assetTexture.name = name;
        assetTexture.sources.push_back( path );
        assetTexture.usage = UsageOf( texture );
        assetTexture.wrapS = IMAGE::WRAPMODE_CLAMP;
        assetTexture.wrapT = IMAGE::WRAPMODE_CLAMP;
        scene.textures.push_back( assetTexture );
        textureIndices[ t ] = (Int32) scene.textures.size() - 1;
        ++added;
    }
    if ( oodle > 0 )
        report.AddSkipped( SourceName, characterName, CORE::UInt32ToString( oodle ) + " more textures of the character need the oodle provider: " + oodleReason );
    report.AddInfo( SourceName, characterName, CORE::UInt32ToString( added ) + " textures of the character asset (PNG files, stored or compressed) are images of the scene" );
    return added;
}

/*-------------------------------------------------------------------------*/

/** The UV tile (UDIM column, 0 for u in [0,1)) of a triangle from UV set 0, -1 when its corners lie in different tiles */
static Int32
UvTileOf( const ASSETCONVERTER::CAssetSubMesh& subMesh, size_t triangle )
{GUCEF_TRACE;

    Int32 tile = -1;
    for ( UInt32 c=0; c<3; ++c )
    {
        const UInt32 vertex = subMesh.indices[ triangle * 3 + c ];
        const Int32 corner = (Int32) floor( subMesh.uvSets[ 0 ][ (size_t) vertex * 2 ] );
        if ( 0 == c )
            tile = corner;
        else if ( corner != tile )
            return -1;
    }
    return tile;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealMetaHumanTextures::SplitHeadTile( UInt32 bodyMaterial, Int32 faceBaseColour, Int32 faceNormal, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    if ( bodyMaterial >= scene.materials.size() || ( faceBaseColour < 0 && faceNormal < 0 ) )
        return 0;

    // only a body whose submeshes use both tiles is laid out this way
    bool headTile = false;
    bool bodyTile = false;
    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
        {
            const ASSETCONVERTER::CAssetSubMesh& subMesh = scene.meshes[ m ].subMeshes[ s ];
            if ( subMesh.materialIndex != (Int32) bodyMaterial || subMesh.uvSets[ 0 ].size() != (size_t) subMesh.GetVertexCount() * 2 )
                continue;
            for ( size_t t=0; t<subMesh.GetTriangleCount() && !( headTile && bodyTile ); ++t )
            {
                const Int32 tile = UvTileOf( subMesh, t );
                headTile = headTile || 0 == tile;
                bodyTile = bodyTile || 1 == tile;
            }
        }
    }
    if ( !headTile || !bodyTile )
        return 0;

    // the material of the head: the body material with the face textures
    ASSETCONVERTER::CAssetMaterial head( scene.materials[ bodyMaterial ] );
    head.name = scene.materials[ bodyMaterial ].name + "_UDIM1001";
    if ( faceBaseColour >= 0 )
        head.baseColorTexture = ASSETCONVERTER::CAssetTextureReference( faceBaseColour );
    if ( faceNormal >= 0 )
        head.normalTexture = ASSETCONVERTER::CAssetTextureReference( faceNormal );
    scene.materials.push_back( head );
    const Int32 headMaterial = (Int32) scene.materials.size() - 1;

    UInt32 moved = 0;
    UInt32 spanning = 0;
    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        ASSETCONVERTER::CAssetMesh& mesh = scene.meshes[ m ];
        const size_t subMeshCount = mesh.subMeshes.size();
        for ( size_t s=0; s<subMeshCount; ++s )
        {
            ASSETCONVERTER::CAssetSubMesh& subMesh = mesh.subMeshes[ s ];
            if ( subMesh.materialIndex != (Int32) bodyMaterial || subMesh.uvSets[ 0 ].size() != (size_t) subMesh.GetVertexCount() * 2 )
                continue;
            ASSETCONVERTER::CAssetSubMesh::TUInt32Vector headTriangles;
            ASSETCONVERTER::CAssetSubMesh::TUInt32Vector otherTriangles;
            for ( size_t t=0; t<subMesh.GetTriangleCount(); ++t )
            {
                const Int32 tile = UvTileOf( subMesh, t );
                if ( 0 == tile )
                    headTriangles.push_back( (UInt32) t );
                else
                    otherTriangles.push_back( (UInt32) t );
                if ( tile < 0 )
                    ++spanning;
            }
            if ( headTriangles.empty() )
                continue;
            moved += (UInt32) headTriangles.size();
            if ( otherTriangles.empty() )
            {
                subMesh.materialIndex = headMaterial;
                continue;
            }
            ASSETCONVERTER::CAssetSubMesh headPart;
            ASSETCONVERTER::CAssetSubMesh bodyPart;
            subMesh.ExtractTriangles( headTriangles, headPart );
            subMesh.ExtractTriangles( otherTriangles, bodyPart );
            headPart.materialIndex = headMaterial;
            mesh.subMeshes[ s ] = bodyPart;
            mesh.subMeshes.push_back( headPart );
        }
    }
    // the textures of a tile cover that tile: the UVs of tile 1002 become those of the texture (u - 1), as a UDIM lookup does;
    // the textures of the character clamp at their edges, so without this the body would show the edge of its texture
    UInt32 shifted = 0;
    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
        {
            ASSETCONVERTER::CAssetSubMesh& subMesh = scene.meshes[ m ].subMeshes[ s ];
            if ( subMesh.materialIndex != (Int32) bodyMaterial || subMesh.uvSets[ 0 ].size() != (size_t) subMesh.GetVertexCount() * 2 )
                continue;
            // the vertices of the triangles that lie in tile 1002 (a triangle that crosses tiles keeps its UVs)
            GUCEF::vector< bool > inTile( subMesh.GetVertexCount(), false );
            for ( size_t t=0; t<subMesh.GetTriangleCount(); ++t )
                if ( 1 == UvTileOf( subMesh, t ) )
                    for ( UInt32 c=0; c<3; ++c )
                        inTile[ subMesh.indices[ t * 3 + c ] ] = true;
            for ( size_t v=0; v<inTile.size(); ++v )
            {
                if ( inTile[ v ] )
                {
                    subMesh.uvSets[ 0 ][ v * 2 ] -= 1.0f;
                    ++shifted;
                }
            }
        }
    }
    report.AddInfo( SourceName, scene.materials[ bodyMaterial ].name, "the material takes its textures by UV tile: tile 1001 (u 0 to 1) is the head with the face textures, tile 1002 (u 1 to 2) the body with the body textures; " +
        CORE::UInt32ToString( moved ) + " triangles of tile 1001 use the material " + head.name + ", the UVs of " + CORE::UInt32ToString( shifted ) + " vertices of tile 1002 are those of its texture (u - 1)" +
        ( spanning > 0 ? ", " + CORE::UInt32ToString( spanning ) + " triangles that cross tiles stay with the body" : CORE::CString() ) );
    if ( spanning > 0 )
        report.AddWarning( SourceName, scene.materials[ bodyMaterial ].name, CORE::UInt32ToString( spanning ) + " triangles cross the UV tiles of head and body: they keep the body textures" );
    return moved;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealMetaHumanTextures::AttachToMaterials( const CUnrealMetaHumanCharacter& character      ,
                                             const GUCEF::vector< Int32 >& textureIndices    ,
                                             ASSETCONVERTER::CAssetScene& scene              ,
                                             ASSETCONVERTER::CConversionReport& report       ,
                                             CORE::CString::StringVector& unassigned         )
{GUCEF_TRACE;

    UInt32 filled = 0;
    GUCEF::vector< bool > used( scene.textures.size(), false );
    for ( size_t m=0; m<scene.materials.size(); ++m )
    {
        ASSETCONVERTER::CAssetMaterial& material = scene.materials[ m ];
        if ( material.name.Lowercase().HasSubstr( "body_shader" ) < 0 )
            continue;
        for ( size_t t=0; t<character.textures.size() && t<textureIndices.size(); ++t )
        {
            if ( textureIndices[ t ] < 0 || "body" != character.textures[ t ].group )
                continue;
            if ( "Body_Basecolor" == character.textures[ t ].type )
            {
                material.baseColorTexture = ASSETCONVERTER::CAssetTextureReference( textureIndices[ t ] );
                used[ (size_t) textureIndices[ t ] ] = true;
                ++filled;
                report.AddInfo( SourceName, material.name, "the base colour texture is " + scene.textures[ (size_t) textureIndices[ t ] ].name );
            }
            else if ( "Body_Normal" == character.textures[ t ].type )
            {
                material.normalTexture = ASSETCONVERTER::CAssetTextureReference( textureIndices[ t ] );
                used[ (size_t) textureIndices[ t ] ] = true;
                ++filled;
                report.AddInfo( SourceName, material.name, "the normal texture is " + scene.textures[ (size_t) textureIndices[ t ] ].name );
            }
        }
    }
    // the combined body lays out the head in UV tile 1001 and the body in 1002: the face textures go on the head
    Int32 faceBaseColour = -1;
    Int32 faceNormal = -1;
    for ( size_t t=0; t<character.textures.size() && t<textureIndices.size(); ++t )
    {
        if ( textureIndices[ t ] < 0 || "face" != character.textures[ t ].group )
            continue;
        if ( "Basecolor" == character.textures[ t ].type )
            faceBaseColour = textureIndices[ t ];
        else if ( "Normal" == character.textures[ t ].type )
            faceNormal = textureIndices[ t ];
    }
    const size_t materialCount = scene.materials.size();
    for ( size_t m=0; m<materialCount; ++m )
    {
        if ( scene.materials[ m ].name.Lowercase().HasSubstr( "body_shader" ) < 0 )
            continue;
        if ( SplitHeadTile( (UInt32) m, faceBaseColour, faceNormal, scene, report ) > 0 )
        {
            if ( faceBaseColour >= 0 )
                used[ (size_t) faceBaseColour ] = true;
            if ( faceNormal >= 0 )
                used[ (size_t) faceNormal ] = true;
            filled += ( faceBaseColour >= 0 ? 1 : 0 ) + ( faceNormal >= 0 ? 1 : 0 );
        }
    }

    unassigned.clear();
    for ( size_t t=0; t<character.textures.size() && t<textureIndices.size(); ++t )
        if ( textureIndices[ t ] >= 0 && !used[ (size_t) textureIndices[ t ] ] )
            unassigned.push_back( scene.textures[ (size_t) textureIndices[ t ] ].name );
    if ( !unassigned.empty() )
    {
        CORE::CString list;
        for ( size_t u=0; u<unassigned.size(); ++u )
            list += ( 0 == u ? "" : ", " ) + unassigned[ u ];
        report.AddInfo( SourceName, character.name, "Unassigned textures (images of the scene that no material uses; the layout of their UV set belongs to a mesh that is not readable yet): " + list );
    }
    return filled;
}

/*-------------------------------------------------------------------------*/

void
CUnrealMetaHumanTextures::DescribeCharacter( const CUnrealMetaHumanCharacter& character      ,
                                             const CORE::CString& characterName              ,
                                             const GUCEF::vector< Int32 >& textureIndices    ,
                                             const ASSETCONVERTER::CAssetScene& scene        ,
                                             CORE::CDataNode& node                           )
{GUCEF_TRACE;

    node.SetAttribute( "schema", "gucefCharacter/1" );
    node.SetAttribute( "name", characterName );
    node.SetAttribute( "assetName", character.name );
    node.SetAttribute( "package", character.packageName );
    node.SetAttribute( "nameOverride", character.nameOverride );
    node.SetAttribute( "previewMaterial", character.previewMaterial );
    node.SetAttribute( "hasHighResolutionTextures", character.hasHighResolutionTextures );
    node.SetAttribute( "hasFaceDnaBlendshapes", character.hasFaceDnaBlendshapes );

    CORE::CDataNode* wardrobe = node.AddChild( "wardrobe" );
    for ( size_t i=0; i<character.wardrobeItems.size(); ++i )
        wardrobe->SetAttribute( CORE::UInt32ToString( (UInt32) i ), character.wardrobeItems[ i ] );
    CORE::CDataNode* presets = node.AddChild( "presetCharacters" );
    for ( size_t i=0; i<character.presetCharacters.size(); ++i )
        presets->SetAttribute( CORE::UInt32ToString( (UInt32) i ), character.presetCharacters[ i ] );

    CORE::CDataNode* settings = node.AddChild( character.settings );
    settings->SetName( "settings" );

    CORE::CDataNode* textures = node.AddChild( "textures" );
    for ( size_t t=0; t<character.textures.size(); ++t )
    {
        const CUnrealMetaHumanCharacter::STexture& texture = character.textures[ t ];
        CORE::CDataNode* entry = textures->AddChild( TextureName( characterName, texture ) );
        entry->SetAttribute( "group", texture.group );
        entry->SetAttribute( "type", texture.type );
        entry->SetAttribute( "width", texture.width );
        entry->SetAttribute( "height", texture.height );
        const Int32 index = t < textureIndices.size() ? textureIndices[ t ] : -1;
        if ( index >= 0 && (size_t) index < scene.textures.size() )
        {
            entry->SetAttribute( "state", "image" );
            entry->SetAttribute( "textureIndex", index );
        }
        else if ( texture.IsExtractable() )
            entry->SetAttribute( "state", "stored PNG, not extracted" );
        else if ( texture.hasPayload && 3 == texture.method )
            entry->SetAttribute( "state", "needs the oodle provider" );
        else
            entry->SetAttribute( "state", "no payload" );
    }
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
