/*
 *  assetpluginUNITY: a model file of the project as Unity imports it
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

#include <math.h>

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETNORMALISER_H
#include "AssetConverterLib_CAssetNormaliser.h"
#define ASSETCONVERTERLIB_CASSETNORMALISER_H
#endif /* ASSETCONVERTERLIB_CASSETNORMALISER_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#include "assetpluginUNITY_CUnityProject.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H
#include "assetpluginUNITY_CUnityBuiltins.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYBUILTINS_H ? */

#include "assetpluginUNITY_CUnityModel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline MATH::TSignedAxis Opposite( MATH::TSignedAxis axis )
{
    return (MATH::TSignedAxis) ( ( (Int32) axis ) ^ 1 );
}

/** The value of a channel at a time, interpolated linearly between its keys (rotations normalised) */
void
SampleChannel( const ASSETCONVERTER::CAssetAnimationChannel& channel, Float32 time, GUCEF::vector< Float32 >& value )
{
    const UInt32 components = channel.GetComponentCount();
    const bool cubic = ASSETCONVERTER::ANIMATIONINTERPOLATION_CUBICSPLINE == channel.interpolation;
    const UInt32 stride = cubic ? components * 3 : components;
    const UInt32 offset = cubic ? components : 0;
    value.assign( components, 0.0f );
    const size_t keys = channel.times.size();
    if ( 0 == keys || channel.values.size() < keys * stride )
        return;
    size_t k = 0;
    while ( k + 1 < keys && channel.times[ k + 1 ] <= time )
        ++k;
    if ( time <= channel.times[ 0 ] || k + 1 >= keys || ASSETCONVERTER::ANIMATIONINTERPOLATION_STEP == channel.interpolation )
    {
        const size_t key = time <= channel.times[ 0 ] ? 0 : k;
        for ( UInt32 c=0; c<components; ++c )
            value[ c ] = channel.values[ key * stride + offset + c ];
        return;
    }
    const Float32 span = channel.times[ k + 1 ] - channel.times[ k ];
    const Float32 t = span > 0.0f ? ( time - channel.times[ k ] ) / span : 0.0f;
    Float32 dot = 0.0f;
    if ( ASSETCONVERTER::ANIMATIONPATH_ROTATION == channel.path )
        for ( UInt32 c=0; c<components; ++c )
            dot += channel.values[ k * stride + offset + c ] * channel.values[ ( k + 1 ) * stride + offset + c ];
    const Float32 sign = dot < 0.0f ? -1.0f : 1.0f;
    Float32 length = 0.0f;
    for ( UInt32 c=0; c<components; ++c )
    {
        const Float32 a = channel.values[ k * stride + offset + c ];
        const Float32 b = channel.values[ ( k + 1 ) * stride + offset + c ] * ( ASSETCONVERTER::ANIMATIONPATH_ROTATION == channel.path ? sign : 1.0f );
        value[ c ] = a + ( b - a ) * t;
        length += value[ c ] * value[ c ];
    }
    if ( ASSETCONVERTER::ANIMATIONPATH_ROTATION == channel.path && length > 0.0f )
    {
        length = sqrtf( length );
        for ( UInt32 c=0; c<components; ++c )
            value[ c ] /= length;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityModel::CUnityModel( void )
    : scene()
    , meta()
    , hasMeta( false )
    , vfsPath()
    , importerName()
    , fileSystem()
    , fileUnitMetres( 1.0 )
    , scale( 1.0 )
    , m_fileNames()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityModel::GetImporterNameForExtension( const CORE::CString& extension )
{GUCEF_TRACE;

    const CORE::CString lower = extension.Lowercase();
    if ( "fbx" == lower )
        return "fbx";
    if ( "gltf" == lower || "glb" == lower )
        return "gltf";
    if ( "obj" == lower )
        return "obj";
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CUnityModel::IsModelExtension( const CORE::CString& extension )
{GUCEF_TRACE;

    return !GetImporterNameForExtension( extension ).IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CUnityModel::Load( CUnityProject& project, const CORE::CString& modelVfsPath )
{GUCEF_TRACE;

    vfsPath = modelVfsPath;
    const CORE::CString subject = project.Describe( modelVfsPath );
    importerName = GetImporterNameForExtension( ASSETCONVERTER::CVfsPaths::ExtensionOf( modelVfsPath ) );
    ASSETCONVERTER::TAssetImporterPtr importer;
    if ( importerName.IsNULLOrEmpty() || !ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( importerName, importer ) )
    {
        project.Warn( subject, "no importer of the converter reads this model file (is the plugin of its format loaded?)" );
        return false;
    }

    const CUnityMeta* modelMeta = project.GetMeta( modelVfsPath );
    hasMeta = GUCEF_NULL != modelMeta;
    if ( hasMeta )
        meta = *modelMeta;

    scene = ASSETCONVERTER::CAssetScene();
    scene.name = ASSETCONVERTER::CVfsPaths::StemOf( modelVfsPath );
    scene.settings = CORE::CDataNode( "settings", GUCEF_DATATYPE_OBJECT );
    // the findings of the importer go to the asset, except those about the texture files the model names: Unity links
    // textures through .mat files only, so they are summed up in one item
    ASSETCONVERTER::CConversionReport modelReport;
    modelReport.SelectAsset( subject );
    const bool imported = importer->Import( modelVfsPath, project.GetContext(), scene, modelReport );
    UInt32 textureItems = 0;
    const ASSETCONVERTER::CConversionReport::TEntryVector& entries = modelReport.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
    {
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
        {
            const ASSETCONVERTER::CConversionReportItem& item = entries[ e ].items[ i ];
            if ( item.message.HasSubstr( "texture file" ) >= 0 || item.message.HasSubstr( "texture " ) == 0 )
            {
                ++textureItems;
                continue;
            }
            project.GetReport().AddItem( item.kind, item.source, item.subject, item.message );
        }
        for ( size_t f=0; f<entries[ e ].files.size(); ++f )
            project.GetReport().AddOutputFile( entries[ e ].files[ f ].path, entries[ e ].files[ f ].references, entries[ e ].files[ f ].profile );
    }
    if ( textureItems > 0 )
        project.Info( subject, CORE::UInt32ToString( textureItems ) + " finding(s) of the model importer about the texture files the model names are left out: textures are linked through .mat files only" );
    if ( !imported )
    {
        project.Warn( subject, "the model file cannot be read by the importer \"" + importerName + "\"" );
        return false;
    }
    project.AddDependency( modelVfsPath );
    for ( size_t d=0; d<scene.dependencies.size(); ++d )
        project.AddDependency( scene.dependencies[ d ] );
    project.AddNestedReferences( scene );

    // the scale: that of the .meta file over the unit of the file
    fileSystem = scene.coordinateSystem;
    fileUnitMetres = fileSystem.GetUnitsPerMetre() > 0.0 ? 1.0 / fileSystem.GetUnitsPerMetre() : 1.0;
    scale = hasMeta ? meta.GetEffectiveScale( fileUnitMetres ) : fileUnitMetres;
    if ( !( scale > 0.0 ) )
        scale = 1.0;
    if ( hasMeta && meta.bakeAxisConversion )
        project.Approximate( subject, "bakeAxisConversion is set: the axis conversion is kept on the top-level nodes instead of in the mesh data" );

    // mirrored on X and scaled: the mesh data and the frames of every node
    const MATH::CCoordinateSystem unity = MATH::CCoordinateSystem::Unity();
    const MATH::CCoordinateSystem meshSystem( MATH::SIGNEDAXIS_NEGATIVE_X, MATH::SIGNEDAXIS_POSITIVE_Y, MATH::SIGNEDAXIS_POSITIVE_Z, 1.0 / scale );
    ASSETCONVERTER::CAssetNormaliser::ConvertCoordinateSystem( scene, meshSystem, unity );

    // the rotation of the axis conversion of the file, on the top-level nodes: R = P(file to Unity) * P(mesh frame to Unity)^T
    if ( fileSystem.IsValid() )
    {
        const MATH::CCoordinateSystem fileInUnity( Opposite( fileSystem.GetRightAxis() ), fileSystem.GetUpAxis(), Opposite( fileSystem.GetForwardAxis() ), 1.0 / scale );
        Int32 full[ 9 ];
        Int32 mirror[ 9 ];
        fileInUnity.GetConversionPermutation( unity, full );
        meshSystem.GetConversionPermutation( unity, mirror );
        Float32 rotation[ 9 ];
        bool identity = true;
        for ( UInt32 c=0; c<3; ++c )
        {
            for ( UInt32 r=0; r<3; ++r )
            {
                Int32 sum = 0;
                for ( UInt32 k=0; k<3; ++k )
                    sum += full[ k*3 + r ] * mirror[ k*3 + c ];     // full * mirror^T
                rotation[ c*3 + r ] = (Float32) sum;
                identity = identity && ( sum == ( r == c ? 1 : 0 ) );
            }
        }
        if ( !identity )
        {
            const MATH::TFloat32Matrix3D matrix = MATH::TFloat32Matrix3D::FromColumnMajor( rotation );
            const MATH::TFloat32Quaternion q = matrix.ToQuaternion().Normalized();
            GUCEF::set< UInt32 > roots( scene.rootNodes.begin(), scene.rootNodes.end() );
            for ( size_t r=0; r<scene.rootNodes.size(); ++r )
            {
                ASSETCONVERTER::CAssetNode& node = scene.nodes[ scene.rootNodes[ r ] ];
                const MATH::TFloat32Vector3D position( node.transform.position.x, node.transform.position.y, node.transform.position.z );
                const MATH::TFloat32Vector3D moved = matrix * position;
                node.transform.position = MATH::TFloat32Transform3D::TPositionVector( moved.x, moved.y, moved.z );
                node.transform.rotation = ( q * node.transform.rotation ).Normalized();
            }
            for ( size_t a=0; a<scene.animations.size(); ++a )
            {
                for ( size_t c=0; c<scene.animations[ a ].channels.size(); ++c )
                {
                    ASSETCONVERTER::CAssetAnimationChannel& channel = scene.animations[ a ].channels[ c ];
                    if ( roots.find( channel.targetNode ) == roots.end() )
                        continue;
                    if ( ASSETCONVERTER::ANIMATIONPATH_TRANSLATION == channel.path )
                    {
                        for ( size_t k=0; k+2<channel.values.size(); k+=3 )
                        {
                            const MATH::TFloat32Vector3D moved = matrix * MATH::TFloat32Vector3D( channel.values[ k ], channel.values[ k+1 ], channel.values[ k+2 ] );
                            channel.values[ k ] = moved.x;
                            channel.values[ k+1 ] = moved.y;
                            channel.values[ k+2 ] = moved.z;
                        }
                    }
                    else
                    if ( ASSETCONVERTER::ANIMATIONPATH_ROTATION == channel.path )
                    {
                        for ( size_t k=0; k+3<channel.values.size(); k+=4 )
                        {
                            const MATH::TFloat32Quaternion key = MATH::TFloat32Quaternion::FromXYZW( channel.values[ k ], channel.values[ k+1 ], channel.values[ k+2 ], channel.values[ k+3 ] );
                            const MATH::TFloat32Quaternion turned = q * key;
                            channel.values[ k ] = turned.x;
                            channel.values[ k+1 ] = turned.y;
                            channel.values[ k+2 ] = turned.z;
                            channel.values[ k+3 ] = turned.w;
                        }
                    }
                }
            }
        }
    }
    scene.coordinateSystem = unity;
    MakeNamesUnique();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUnityModel::MakeNamesUnique( void )
{GUCEF_TRACE;

    m_fileNames.clear();
    GUCEF::set< CORE::CString > taken;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        m_fileNames.push_back( scene.nodes[ n ].name );
        CORE::CString name = scene.nodes[ n ].name;
        for ( UInt32 suffix=1; taken.find( name ) != taken.end(); ++suffix )
            name = scene.nodes[ n ].name + " " + CORE::UInt32ToString( suffix );
        taken.insert( name );
        scene.nodes[ n ].name = name;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CUnityModel::FindNamedNode( const CORE::CString& name, UInt32 rank, bool withMesh ) const
{GUCEF_TRACE;

    // the unique name Unity gives the node, else the name of the file with the rank among equal names
    for ( size_t n=0; n<scene.nodes.size(); ++n )
        if ( ( !withMesh || scene.nodes[ n ].mesh >= 0 ) && scene.nodes[ n ].name == name && ( 0 == rank || n >= m_fileNames.size() || m_fileNames[ n ] != name ) )
            return (Int32) n;
    UInt32 seen = 0;
    for ( size_t n=0; n<scene.nodes.size() && n<m_fileNames.size(); ++n )
    {
        if ( withMesh && scene.nodes[ n ].mesh < 0 )
            continue;
        if ( m_fileNames[ n ] != name )
            continue;
        if ( seen == rank )
            return (Int32) n;
        ++seen;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityModel::FindMeshNode( Int64 fileId, const CORE::CString& nameHint, bool* guessed ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != guessed )
        *guessed = false;
    CUnityMeta::SIdEntry entry;
    if ( !hasMeta || !meta.TryGetIdEntry( fileId, entry ) )
    {
        // without an entry: the only mesh of the model, else the mesh node with the name of the user
        Int32 found = -1;
        bool single = true;
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            if ( scene.nodes[ n ].mesh < 0 )
                continue;
            if ( found >= 0 && scene.nodes[ found ].mesh != scene.nodes[ n ].mesh )
                single = false;
            if ( found < 0 )
                found = (Int32) n;
        }
        if ( !single )
            found = nameHint.IsNULLOrEmpty() ? -1 : FindNamedNode( nameHint, 0, true );
        if ( GUCEF_NULL != guessed )
            *guessed = found >= 0;
        return found;
    }
    if ( 43 != entry.classId )
        return -1;
    const UInt32 rank = meta.GetNameRank( fileId );
    Int32 node = FindNamedNode( entry.name, rank, true );
    if ( node < 0 && rank > 0 )
        node = FindNamedNode( entry.name, 0, true );
    if ( node >= 0 )
        return node;

    // the name of the mesh rather than that of its node
    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        if ( scene.meshes[ m ].name != entry.name )
            continue;
        for ( size_t n=0; n<scene.nodes.size(); ++n )
            if ( scene.nodes[ n ].mesh == (Int32) m )
                return (Int32) n;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityModel::GetRootNode( void ) const
{GUCEF_TRACE;

    return 1 == scene.rootNodes.size() ? (Int32) scene.rootNodes[ 0 ] : -1;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityModel::FindNode( Int64 fileId ) const
{GUCEF_TRACE;

    CUnityMeta::SIdEntry entry;
    if ( !hasMeta || !meta.TryGetIdEntry( fileId, entry ) )
        return CUnityBuiltins::IsModelRootObjectId( fileId ) ? GetRootNode() : -1;
    if ( "//RootNode" == entry.name )
        return GetRootNode();
    if ( 43 == entry.classId )
        return FindMeshNode( fileId );
    return FindNamedNode( entry.name, meta.GetNameRank( fileId ), false );
}

/*-------------------------------------------------------------------------*/

void
CUnityModel::CutAnimation( const ASSETCONVERTER::CAssetAnimation& take, Float32 from, Float32 to, ASSETCONVERTER::CAssetAnimation& clip )
{GUCEF_TRACE;

    clip = take;
    clip.channels.clear();
    if ( to < from )
    {
        const Float32 swap = to;
        to = from;
        from = swap;
    }
    for ( size_t c=0; c<take.channels.size(); ++c )
    {
        const ASSETCONVERTER::CAssetAnimationChannel& source = take.channels[ c ];
        ASSETCONVERTER::CAssetAnimationChannel cut;
        cut.targetNode = source.targetNode;
        cut.path = source.path;
        cut.interpolation = ASSETCONVERTER::ANIMATIONINTERPOLATION_CUBICSPLINE == source.interpolation ? ASSETCONVERTER::ANIMATIONINTERPOLATION_LINEAR : source.interpolation;
        GUCEF::vector< Float32 > value;

        // the value at the start, the keys inside, the value at the end
        SampleChannel( source, from, value );
        cut.times.push_back( 0.0f );
        cut.values.insert( cut.values.end(), value.begin(), value.end() );
        const UInt32 components = source.GetComponentCount();
        const bool cubic = ASSETCONVERTER::ANIMATIONINTERPOLATION_CUBICSPLINE == source.interpolation;
        const UInt32 stride = cubic ? components * 3 : components;
        const UInt32 offset = cubic ? components : 0;
        for ( size_t k=0; k<source.times.size(); ++k )
        {
            const Float32 time = source.times[ k ];
            if ( time <= from + 1e-6f || time >= to - 1e-6f || source.values.size() < ( k + 1 ) * stride )
                continue;
            cut.times.push_back( time - from );
            for ( UInt32 v=0; v<components; ++v )
                cut.values.push_back( source.values[ k * stride + offset + v ] );
        }
        if ( to - from > 1e-6f )
        {
            SampleChannel( source, to, value );
            cut.times.push_back( to - from );
            cut.values.insert( cut.values.end(), value.begin(), value.end() );
        }
        clip.channels.push_back( cut );
    }
}

/*-------------------------------------------------------------------------*/

void
CUnityModel::ApplyClips( CUnityProject& project, CORE::CDataNode& clipIds )
{GUCEF_TRACE;

    const CORE::CString subject = project.Describe( vfsPath );
    clipIds = CORE::CDataNode( "clips", GUCEF_DATATYPE_ARRAY );
    if ( hasMeta && !meta.importAnimation )
    {
        if ( !scene.animations.empty() )
            project.Skip( subject, "the .meta file does not import the animation of the model: " + CORE::UInt32ToString( (UInt32) scene.animations.size() ) + " take(s) left out" );
        scene.animations.clear();
        return;
    }
    if ( scene.animations.empty() )
        return;
    if ( hasMeta && 3 == meta.animationType )
        project.Info( subject, "the model is a humanoid rig in Unity: its takes are converted as generic transform curves, the retargeting is not converted" );

    GUCEF::vector< Int64 > clipEntryIds;
    if ( hasMeta )
        meta.GetIdsOfClass( 74, clipEntryIds );

    if ( !hasMeta || meta.clips.empty() )
    {
        // the takes as they are, with the ID of the entry that names them
        for ( size_t a=0; a<scene.animations.size(); ++a )
        {
            for ( size_t i=0; i<clipEntryIds.size(); ++i )
            {
                CUnityMeta::SIdEntry entry;
                if ( meta.TryGetIdEntry( clipEntryIds[ i ], entry ) && entry.name == scene.animations[ a ].name )
                {
                    CORE::CDataNode* item = clipIds.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                    item->SetAttribute( "name", entry.name );
                    item->SetAttribute( "fileID", clipEntryIds[ i ] );
                    item->SetAttribute( "take", scene.animations[ a ].name );
                }
            }
        }
        return;
    }

    ASSETCONVERTER::CAssetScene::TAnimationVector takes;
    takes.swap( scene.animations );
    for ( size_t c=0; c<meta.clips.size(); ++c )
    {
        const CUnityClipAnimation& split = meta.clips[ c ];
        Int32 take = -1;
        for ( size_t t=0; t<takes.size() && take < 0; ++t )
            if ( takes[ t ].name == split.takeName || takes[ t ].sourceTake == split.takeName )
                take = (Int32) t;
        for ( size_t t=0; t<takes.size() && take < 0; ++t )
            if ( takes[ t ].name.Lowercase() == split.takeName.Lowercase() )
                take = (Int32) t;
        if ( take < 0 && 1 == takes.size() )
        {
            take = 0;
            project.Approximate( subject, "the clip " + split.name + " names the take \"" + split.takeName + "\", which the file does not have: its only take \"" + takes[ 0 ].name + "\" is used" );
        }
        if ( take < 0 )
        {
            project.Warn( subject, "the clip " + split.name + " names the take \"" + split.takeName + "\", which the file does not have: the clip is left out" );
            continue;
        }
        const ASSETCONVERTER::CAssetAnimation& source = takes[ take ];
        const Float32 rate = source.frameRate > 0.0f ? source.frameRate : 30.0f;
        const Float32 takeStart = source.firstFrame / rate;
        ASSETCONVERTER::CAssetAnimation clip;
        CutAnimation( source, split.firstFrame / rate - takeStart, split.lastFrame / rate - takeStart, clip );
        clip.name = split.name;
        clip.sourceTake = source.name;
        clip.firstFrame = split.firstFrame;
        clip.lastFrame = split.lastFrame;
        clip.frameRate = rate;
        clip.loop = split.loopTime;
        if ( split.mirror )
            project.Approximate( subject, "the clip " + split.name + " is mirrored in Unity: the mirror is not applied" );
        if ( split.eventCount > 0 )
            project.Skip( subject, "the clip " + split.name + " has " + CORE::UInt32ToString( split.eventCount ) + " animation event(s): events are not part of the intermediate" );
        scene.animations.push_back( clip );

        CORE::CDataNode* item = clipIds.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "name", split.name );
        item->SetAttribute( "take", source.name );
        Int64 id = split.internalId;
        for ( size_t i=0; i<clipEntryIds.size() && 0 == id; ++i )
        {
            CUnityMeta::SIdEntry entry;
            if ( meta.TryGetIdEntry( clipEntryIds[ i ], entry ) && entry.name == split.name )
                id = clipEntryIds[ i ];
        }
        if ( 0 != id )
            item->SetAttribute( "fileID", id );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
