/*
 *  assetpluginUNITY: the reader of Unity prefabs and scenes
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

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#include "assetpluginUNITY_CUnityModel.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMESHDECODER_H
#include "assetpluginUNITY_CUnityMeshDecoder.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMESHDECODER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMESHDECODER_H ? */

#include "assetpluginUNITY_CUnityPrefabReader.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int32 ClassGameObject = 1;
const Int32 ClassTransform = 4;
const Int32 ClassRectTransform = 224;
const Int32 ClassMeshRenderer = 23;
const Int32 ClassMeshFilter = 33;
const Int32 ClassRigidbody = 54;
const Int32 ClassMeshCollider = 64;
const Int32 ClassBoxCollider = 65;
const Int32 ClassLight = 108;
const Int32 ClassSphereCollider = 135;
const Int32 ClassCapsuleCollider = 136;
const Int32 ClassSkinnedMeshRenderer = 137;
const Int32 ClassParticleSystem = 198;
const Int32 ClassParticleSystemRenderer = 199;
const Int32 ClassLodGroup = 205;
const Int32 ClassPrefab = 1001;
const Int32 ClassEditorExtension = 1002;
const UInt32 MaxNesting = 16;

/** The body of a document: its override when the instance being read modifies it */
template < class TOverrideMap >
const CORE::CDataNode&
BodyOf( const CUnityYamlDocument& document, const TOverrideMap* overrides )
{
    if ( GUCEF_NULL != overrides )
    {
        typename TOverrideMap::const_iterator i = overrides->find( document.fileId );
        if ( i != overrides->end() )
            return (*i).second;
    }
    return document.body;
}

/** The first child mapping of a component list item: "- component: {fileID: n}" or the older "- 4: {fileID: n}" */
CUnityReference
ComponentReference( const CORE::CDataNode& item )
{
    CUnityReference reference;
    if ( !reference.Read( &item ) )
        reference.Read( item.GetFirstChild() );
    return reference;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityPrefabReader::CUnityPrefabReader( CUnityProject& project, CUnityMaterialReader& materials )
    : m_project( project )
    , m_materials( materials )
    , m_particles( project, materials )
    , m_stack()
    , m_inactive()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CUnityPrefabReader::NewNode( const CORE::CString& name )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    scene.nodes.push_back( ASSETCONVERTER::CAssetNode() );
    scene.nodes.back().name = name;
    return (UInt32) scene.nodes.size() - 1;
}

/*-------------------------------------------------------------------------*/

void
CUnityPrefabReader::Link( UInt32 child, Int32 parent )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    scene.nodes[ child ].parent = parent;
    if ( parent >= 0 )
        scene.nodes[ parent ].children.push_back( child );
    else
        scene.rootNodes.push_back( child );
}

/*-------------------------------------------------------------------------*/

void
CUnityPrefabReader::ReadTransform( const CORE::CDataNode& body, ASSETCONVERTER::CAssetNode& node )
{GUCEF_TRACE;

    Float32 position[ 3 ] = { 0.0f, 0.0f, 0.0f };
    Float32 rotation[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
    Float32 scale[ 3 ] = { 1.0f, 1.0f, 1.0f };
    CUnityYamlReader::GetComponents( body, "m_LocalPosition", "xyz", position );
    CUnityYamlReader::GetComponents( body, "m_LocalRotation", "xyzw", rotation );
    CUnityYamlReader::GetComponents( body, "m_LocalScale", "xyz", scale );
    node.transform.position = MATH::TFloat32Transform3D::TPositionVector( position[ 0 ], position[ 1 ], position[ 2 ] );
    node.transform.rotation.x = rotation[ 0 ];
    node.transform.rotation.y = rotation[ 1 ];
    node.transform.rotation.z = rotation[ 2 ];
    node.transform.rotation.w = rotation[ 3 ];
    const Float32 length = sqrtf( rotation[ 0 ] * rotation[ 0 ] + rotation[ 1 ] * rotation[ 1 ] + rotation[ 2 ] * rotation[ 2 ] + rotation[ 3 ] * rotation[ 3 ] );
    if ( length > 0.0f && fabsf( length - 1.0f ) > 1e-6f )
        node.transform.rotation = node.transform.rotation.Normalized();
    else
    if ( 0.0f == length )
        node.transform.rotation.w = 1.0f;
    node.transform.scale.x = scale[ 0 ];
    node.transform.scale.y = scale[ 1 ];
    node.transform.scale.z = scale[ 2 ];
}

/*-------------------------------------------------------------------------*/

bool
CUnityPrefabReader::ApplyPropertyPath( CORE::CDataNode& object, const CORE::CString& propertyPath, const CORE::CString& value, const CORE::CDataNode* objectReference )
{GUCEF_TRACE;

    const CORE::CString::StringVector segments = propertyPath.ParseElements( '.', false );
    if ( segments.empty() )
        return false;
    const bool isReference = GUCEF_NULL != objectReference && ( 0 != CUnityYamlReader::GetInt64( *objectReference, "fileID", 0 ) || ( value.IsNULLOrEmpty() && CUnityYamlReader::HasScalar( *objectReference, "guid" ) ) );

    CORE::CDataNode* current = &object;
    for ( size_t i=0; i<segments.size(); ++i )
    {
        const CORE::CString& segment = segments[ i ];
        const bool last = i + 1 == segments.size();
        if ( "Array" == segment && !last )
        {
            const CORE::CString& next = segments[ i+1 ];
            current->SetNodeType( GUCEF_DATATYPE_ARRAY );
            if ( "size" == next )
            {
                Int64 size = 0;
                if ( !CUnityYamlReader::ParseInt64( value, size ) || size < 0 )
                    return false;
                while ( (Int64) current->GetNrOfDirectChildNodes() < size )
                {
                    CORE::CDataNode* item = current->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                    item->SetAttribute( "fileID", "0" );
                }
                while ( (Int64) current->GetNrOfDirectChildNodes() > size )
                    current->GetLastChild()->Delete();
                return true;
            }
            if ( !next.StartsWith( "data[" ) || !next.EndsWith( "]" ) )
                return false;
            Int64 index = 0;
            if ( !CUnityYamlReader::ParseInt64( next.SubstrFromRange( 5, next.Length() - 1 ), index ) || index < 0 )
                return false;
            while ( (Int64) current->GetNrOfDirectChildNodes() <= index )
            {
                CORE::CDataNode* item = current->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                item->SetAttribute( "fileID", "0" );
            }
            CORE::CDataNode* item = current->GetChildAtIndex( (UInt32) index );
            if ( i + 2 == segments.size() )
            {
                if ( isReference )
                {
                    item->DelSubTree();
                    item->ClearAttributes();
                    item->SetNodeType( GUCEF_DATATYPE_OBJECT );
                    CORE::CDataNode::TAttributeMap::const_iterator a = objectReference->AttributeBegin();
                    while ( a != objectReference->AttributeEnd() )
                    {
                        item->SetAttribute( (*a).first, (*a).second );
                        ++a;
                    }
                }
                else
                {
                    item->SetValue( CORE::CVariant( value ) );
                }
                return true;
            }
            current = item;
            ++i;
            continue;
        }
        if ( last )
        {
            if ( isReference )
            {
                current->DelAttribute( segment );
                CORE::CDataNode* member = current->FindChild( segment );
                if ( GUCEF_NULL == member )
                    member = current->AddChild( segment, GUCEF_DATATYPE_OBJECT );
                member->DelSubTree();
                member->ClearAttributes();
                CORE::CDataNode::TAttributeMap::const_iterator a = objectReference->AttributeBegin();
                while ( a != objectReference->AttributeEnd() )
                {
                    member->SetAttribute( (*a).first, (*a).second );
                    ++a;
                }
            }
            else
            {
                while ( GUCEF_NULL != current->FindChild( segment ) )
                    current->DelChild( segment );
                current->SetAttribute( segment, value );
            }
            return true;
        }
        CORE::CDataNode* child = current->FindChild( segment );
        if ( GUCEF_NULL == child )
        {
            current->DelAttribute( segment );
            child = current->AddChild( segment, GUCEF_DATATYPE_OBJECT );
        }
        current = child;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPrefabReader::IsInstance( const CUnityYamlDocument& document, const CORE::CDataNode& body )
{GUCEF_TRACE;

    if ( ClassPrefab != document.classId || document.stripped )
        return false;
    if ( !CUnityReference::Get( body, "m_SourcePrefab" ).IsNull() )
        return true;
    const CUnityReference parent = CUnityReference::Get( body, "m_ParentPrefab" );
    return !parent.IsNull() && !parent.guid.IsNULLOrEmpty() && 1 != CUnityYamlReader::GetInt32( body, "m_IsPrefabParent", 0 );
}

/*-------------------------------------------------------------------------*/

bool
CUnityPrefabReader::ExpandModelInstance( const CORE::CString& modelPath, const CUnityYamlReader::TNodeList& modifications, const CUnityYamlReader::TNodeList& removedItems,
                                         const CORE::CString& subject, SFileResult& inner, ASSETCONVERTER::CAssetPrefabInstance& record )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    CUnityModel* model = m_project.GetModel( modelPath );
    if ( GUCEF_NULL == model )
    {
        const UInt32 node = NewNode( ASSETCONVERTER::CVfsPaths::StemOf( modelPath ) );
        inner.roots.push_back( node );
        return true;
    }
    const ASSETCONVERTER::CAssetScene& source = model->scene;

    // the hierarchy of the model; several top-level nodes go below a root named after the file, as in Unity
    GUCEF::vector< Int32 > nodeOf( source.nodes.size(), -1 );
    for ( size_t n=0; n<source.nodes.size(); ++n )
    {
        nodeOf[ n ] = (Int32) NewNode( source.nodes[ n ].name );
        scene.nodes[ nodeOf[ n ] ].transform = source.nodes[ n ].transform;
    }
    for ( size_t n=0; n<source.nodes.size(); ++n )
        if ( source.nodes[ n ].parent >= 0 )
            Link( (UInt32) nodeOf[ n ], nodeOf[ source.nodes[ n ].parent ] );
    Int32 root = model->GetRootNode() >= 0 ? nodeOf[ model->GetRootNode() ] : -1;
    if ( root < 0 )
    {
        root = (Int32) NewNode( ASSETCONVERTER::CVfsPaths::StemOf( modelPath ) );
        for ( size_t r=0; r<source.rootNodes.size(); ++r )
            Link( (UInt32) nodeOf[ source.rootNodes[ r ] ], root );
    }
    inner.roots.push_back( (UInt32) root );

    // the objects by their IDs: the root objects, and the entries of the ID table
    const Int64 rootIds[ 3 ] = { (Int64) 919132149155446097LL, (Int64) -8679921383154817045LL, (Int64) -7511558181221131132LL };
    for ( UInt32 i=0; i<3; ++i )
        inner.nodes[ rootIds[ i ] ] = root;
    if ( model->hasMeta )
    {
        for ( CUnityMeta::TIdTable::const_iterator e = model->meta.idTable.begin(); e != model->meta.idTable.end(); ++e )
        {
            if ( 74 == (*e).second.classId || 43 == (*e).second.classId || 21 == (*e).second.classId || 95 == (*e).second.classId )
                continue;
            const Int32 found = "//RootNode" == (*e).second.name ? -2 : model->FindNode( (*e).first );
            if ( -2 == found )
                inner.nodes[ (*e).first ] = root;
            else
            if ( found >= 0 )
                inner.nodes[ (*e).first ] = nodeOf[ found ];
        }
    }

    // the modifications, per object
    typedef GUCEF::map< Int32, GUCEF::vector< Int32 > > TMaterialMap;
    TMaterialMap materialOverrides;
    GUCEF::set< Int32 > noRenderer;
    CORE::CDataNode* recorded = record.overrides.FindChild( "modifications" );
    UInt32 unapplied = 0;
    for ( size_t m=0; m<modifications.size(); ++m )
    {
        const CORE::CDataNode& item = *modifications[ m ];
        CUnityReference target;
        target.Read( item.FindChild( "target" ) );
        const CORE::CString propertyPath = CUnityYamlReader::GetString( item, "propertyPath" );
        const CORE::CString value = CUnityYamlReader::GetString( item, "value" );
        TObjectNodeMap::const_iterator found = inner.nodes.find( target.fileId );
        Int32 node = found != inner.nodes.end() ? (*found).second : ( 1 == source.nodes.size() ? root : -1 );
        bool applied = false;
        if ( node >= 0 )
        {
            ASSETCONVERTER::CAssetNode& object = scene.nodes[ node ];
            Float64 number = 0.0;
            const bool numeric = CUnityYamlReader::ParseFloat( value, number );
            const char last = propertyPath.Length() > 0 ? propertyPath[ propertyPath.Length() - 1 ] : '\0';
            if ( numeric && propertyPath.StartsWith( "m_LocalPosition." ) )
            {
                if ( 'x' == last ) object.transform.position.x = (Float32) number;
                else if ( 'y' == last ) object.transform.position.y = (Float32) number;
                else if ( 'z' == last ) object.transform.position.z = (Float32) number;
                applied = true;
            }
            else
            if ( numeric && propertyPath.StartsWith( "m_LocalRotation." ) )
            {
                if ( 'x' == last ) object.transform.rotation.x = (Float32) number;
                else if ( 'y' == last ) object.transform.rotation.y = (Float32) number;
                else if ( 'z' == last ) object.transform.rotation.z = (Float32) number;
                else if ( 'w' == last ) object.transform.rotation.w = (Float32) number;
                applied = true;
            }
            else
            if ( numeric && propertyPath.StartsWith( "m_LocalScale." ) )
            {
                if ( 'x' == last ) object.transform.scale.x = (Float32) number;
                else if ( 'y' == last ) object.transform.scale.y = (Float32) number;
                else if ( 'z' == last ) object.transform.scale.z = (Float32) number;
                applied = true;
            }
            else
            if ( "m_Name" == propertyPath )
            {
                object.name = value;
                applied = true;
            }
            else
            if ( "m_IsActive" == propertyPath )
            {
                if ( numeric && 0.0 == number )
                {
                    object.extras.SetAttribute( "active", false );
                    m_inactive.insert( node );
                }
                applied = true;
            }
            else
            if ( propertyPath.StartsWith( "m_Materials.Array.data[" ) )
            {
                Int64 index = -1;
                const CORE::CString number = propertyPath.SubstrFromRange( 23, propertyPath.Length() - 1 );
                CUnityReference reference;
                reference.Read( item.FindChild( "objectReference" ) );
                if ( CUnityYamlReader::ParseInt64( number, index ) && index >= 0 && index < 256 )
                {
                    GUCEF::vector< Int32 >& list = materialOverrides[ node ];
                    while ( (Int64) list.size() <= index )
                        list.push_back( -2 );
                    list[ (size_t) index ] = m_materials.GetMaterial( reference, subject );
                    applied = true;
                }
            }
            else
            if ( "m_Materials.Array.size" == propertyPath && numeric && number >= 0.0 && number < 256.0 )
            {
                // fewer materials than submeshes leaves the others undrawn in Unity; they keep the default material here
                GUCEF::vector< Int32 >& list = materialOverrides[ node ];
                const size_t size = (size_t) number;
                while ( list.size() < size )
                    list.push_back( -2 );
                if ( list.size() > size )
                    list.resize( size );
                applied = true;
            }
            else
            if ( propertyPath.StartsWith( "m_LocalEulerAnglesHint." ) || "m_RootOrder" == propertyPath || "m_Layer" == propertyPath || "m_StaticEditorFlags" == propertyPath ||
                 "m_TagString" == propertyPath || "m_CastShadows" == propertyPath || "m_ReceiveShadows" == propertyPath || "m_ConstrainProportionsScale" == propertyPath )
            {
                applied = true;     // editor and lighting state without a counterpart in the intermediate
            }
        }
        if ( !applied )
            ++unapplied;
        if ( GUCEF_NULL != recorded && m < recorded->GetNrOfDirectChildNodes() )
            recorded->GetChildAtIndex( (UInt32) m )->SetAttribute( "applied", applied );
    }
    for ( size_t r=0; r<removedItems.size(); ++r )
    {
        CUnityReference removed;
        removed.Read( removedItems[ r ] );
        TObjectNodeMap::const_iterator found = inner.nodes.find( removed.fileId );
        if ( found != inner.nodes.end() && (Int64) -7511558181221131132LL == removed.fileId )
            noRenderer.insert( (*found).second );
    }
    if ( unapplied > 0 )
        m_project.Warn( subject, CORE::UInt32ToString( unapplied ) + " modification(s) of the prefab instance name objects of " + m_project.Describe( modelPath ) +
                        " that are not known (IDs computed from names without a table): they stay in the sidecar" );
    record.overrides.SetAttribute( "unapplied", unapplied );

    // the meshes with the materials of the model, or of the modifications
    for ( size_t n=0; n<source.nodes.size(); ++n )
    {
        const Int32 meshIndex = source.nodes[ n ].mesh;
        if ( meshIndex < 0 || noRenderer.find( nodeOf[ n ] ) != noRenderer.end() )
            continue;
        const ASSETCONVERTER::CAssetMesh& mesh = source.meshes[ meshIndex ];
        GUCEF::vector< Int32 > materials;
        TMaterialMap::const_iterator overridden = materialOverrides.find( nodeOf[ n ] );
        CORE::CString key = "modelInstanceMesh|" + modelPath + "|" + CORE::Int32ToString( meshIndex );
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            Int32 material = -2;
            if ( overridden != materialOverrides.end() && s < (*overridden).second.size() )
                material = (*overridden).second[ s ];
            if ( -2 == material )
                material = mesh.subMeshes[ s ].materialIndex >= 0 ? m_materials.GetModelMaterial( *model, (UInt32) mesh.subMeshes[ s ].materialIndex ) : m_materials.GetDefaultMaterial();
            if ( material < 0 )
                material = m_materials.GetDefaultMaterial();
            materials.push_back( material );
            key += "|" + CORE::Int32ToString( material );
        }
        Int32 index = -1;
        if ( !m_project.TryGetCached( key, index ) )
        {
            ASSETCONVERTER::CAssetMesh copy = mesh;
            for ( size_t s=0; s<copy.subMeshes.size(); ++s )
                copy.subMeshes[ s ].materialIndex = materials[ s ];
            scene.meshes.push_back( copy );
            index = (Int32) scene.meshes.size() - 1;
            m_project.SetCached( key, index );
        }
        scene.nodes[ nodeOf[ n ] ].mesh = index;
        if ( source.nodes[ n ].skin >= 0 )
        {
            // the joints are the nodes of the instance
            ASSETCONVERTER::CAssetSkin skin = source.skins[ source.nodes[ n ].skin ];
            for ( size_t j=0; j<skin.joints.size(); ++j )
                skin.joints[ j ] = (UInt32) nodeOf[ skin.joints[ j ] ];
            if ( skin.skeletonRoot >= 0 )
                skin.skeletonRoot = nodeOf[ skin.skeletonRoot ];
            scene.skins.push_back( skin );
            scene.nodes[ nodeOf[ n ] ].skin = (Int32) scene.skins.size() - 1;
        }
    }
    for ( size_t n=0; n<source.nodes.size(); ++n )
        scene.nodes[ nodeOf[ n ] ].transform.rotation = scene.nodes[ nodeOf[ n ] ].transform.rotation.Normalized();
    scene.nodes[ root ].transform.rotation = scene.nodes[ root ].transform.rotation.Normalized();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPrefabReader::ExpandInstance( const CUnityYamlDocument& instance, const CORE::CDataNode& body, const CORE::CString& vfsPath, UInt32 depth, SFileResult& inner,
                                    ASSETCONVERTER::CAssetPrefabInstance& record )
{GUCEF_TRACE;

    CUnityReference source = CUnityReference::Get( body, "m_SourcePrefab" );
    if ( source.IsNull() )
        source = CUnityReference::Get( body, "m_ParentPrefab" );
    const CORE::CString subject = m_project.Describe( vfsPath ) + " &" + CORE::Int64ToString( instance.fileId );

    const CORE::CDataNode* modification = body.FindChild( "m_Modification" );
    CUnityYamlReader::TNodeList modifications;
    CUnityYamlReader::TNodeList removedItems;
    if ( GUCEF_NULL != modification )
    {
        CUnityYamlReader::GetItems( modification->FindChild( "m_Modifications" ), modifications );
        CUnityYamlReader::GetItems( modification->FindChild( "m_RemovedComponents" ), removedItems );
    }

    // the record of the instance: Unity's modifications as they are
    record.overrides = CORE::CDataNode( "overrides", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* recorded = record.overrides.AddChild( "modifications", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* recordedRemoved = record.overrides.AddChild( "removedComponents", GUCEF_DATATYPE_ARRAY );
    for ( size_t r=0; r<removedItems.size(); ++r )
    {
        CUnityReference removed;
        removed.Read( removedItems[ r ] );
        recordedRemoved->AddValueAsChild( removed.ToString() );
    }

    CORE::CString sourcePath;
    const bool resolved = !source.guid.IsNULLOrEmpty() && m_project.ResolveGuid( source.guid, sourcePath );
    bool cycle = false;
    for ( size_t s=0; s<m_stack.size(); ++s )
        cycle = cycle || m_stack[ s ] == sourcePath;
    const bool modelSource = resolved && CUnityModel::IsModelExtension( ASSETCONVERTER::CVfsPaths::ExtensionOf( sourcePath ) );
    const CUnityYamlFile* sourceFile = resolved && !modelSource && !cycle && depth < MaxNesting ? m_project.GetYaml( sourcePath ) : GUCEF_NULL;
    record.sourcePrefab = resolved ? m_project.Describe( sourcePath ) : source.ToString();

    // the modifications, on copies of the objects of the source prefab
    TObjectOverrideMap overrides;
    TIdSet removed;
    UInt32 unapplied = 0;
    for ( size_t m=0; m<modifications.size(); ++m )
    {
        const CORE::CDataNode& item = *modifications[ m ];
        CUnityReference target;
        target.Read( item.FindChild( "target" ) );
        const CORE::CString propertyPath = CUnityYamlReader::GetString( item, "propertyPath" );
        const CORE::CString value = CUnityYamlReader::GetString( item, "value" );
        const CORE::CDataNode* objectReference = item.FindChild( "objectReference" );

        bool applied = false;
        const CUnityYamlDocument* document = GUCEF_NULL != sourceFile ? sourceFile->FindDocument( target.fileId ) : GUCEF_NULL;
        if ( GUCEF_NULL != document )
        {
            TObjectOverrideMap::iterator copy = overrides.find( target.fileId );
            if ( copy == overrides.end() )
                copy = overrides.insert( std::pair< const Int64, CORE::CDataNode >( target.fileId, document->body ) ).first;
            applied = ApplyPropertyPath( (*copy).second, propertyPath, value, objectReference );
        }
        if ( !applied )
            ++unapplied;

        CORE::CDataNode* entry = recorded->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "target", target.ToString() );
        entry->SetAttribute( "propertyPath", propertyPath );
        entry->SetAttribute( "value", value );
        CUnityReference reference;
        if ( reference.Read( objectReference ) && !reference.IsNull() )
            entry->SetAttribute( "objectReference", reference.ToString() );
        entry->SetAttribute( "applied", applied );
    }
    for ( size_t r=0; r<removedItems.size(); ++r )
    {
        CUnityReference reference;
        if ( reference.Read( removedItems[ r ] ) )
            removed.insert( reference.fileId );
    }
    if ( modelSource )
        return ExpandModelInstance( sourcePath, modifications, removedItems, subject, inner, record );
    if ( unapplied > 0 )
        m_project.Warn( subject, CORE::UInt32ToString( unapplied ) + " modification(s) of the prefab instance name objects that are not in its source prefab: they stay in the sidecar" );
    record.overrides.SetAttribute( "unapplied", unapplied );

    if ( GUCEF_NULL == sourceFile )
    {
        if ( !resolved )
            m_project.Warn( subject, "the prefab instance uses the prefab " + source.ToString() + ", which is not in the package: an empty node stands in for it" );
        else
        if ( cycle || depth >= MaxNesting )
            m_project.Warn( subject, "the prefab instance of " + m_project.Describe( sourcePath ) + " nests too deep or in a cycle: an empty node stands in for it" );
        const UInt32 node = NewNode( resolved ? ASSETCONVERTER::CVfsPaths::StemOf( sourcePath ) : "MissingPrefab" );
        // the transform of the root, as far as the modifications give it
        CORE::CDataNode transform( "Transform", GUCEF_DATATYPE_OBJECT );
        for ( size_t m=0; m<modifications.size(); ++m )
        {
            const CORE::CString propertyPath = CUnityYamlReader::GetString( *modifications[ m ], "propertyPath" );
            if ( propertyPath.StartsWith( "m_LocalPosition." ) || propertyPath.StartsWith( "m_LocalRotation." ) || propertyPath.StartsWith( "m_LocalScale." ) )
                ApplyPropertyPath( transform, propertyPath, CUnityYamlReader::GetString( *modifications[ m ], "value" ), GUCEF_NULL );
            else
            if ( "m_Name" == propertyPath )
                m_project.GetScene().nodes[ node ].name = CUnityYamlReader::GetString( *modifications[ m ], "value" );
        }
        ReadTransform( transform, m_project.GetScene().nodes[ node ] );
        inner.roots.push_back( node );
        return true;
    }

    m_stack.push_back( sourcePath );
    const bool ok = ReadFile( sourcePath, &overrides, &removed, depth + 1, inner, GUCEF_NULL );
    m_stack.pop_back();
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPrefabReader::GetSourceMesh( const CUnityReference& reference, const CORE::CString& vfsPath, const CORE::CString& subject, const CORE::CString& userName,
                                   ASSETCONVERTER::CAssetMesh& mesh, const ASSETCONVERTER::CAssetSkin*& skin, const CUnityModel*& model )
{GUCEF_TRACE;

    skin = GUCEF_NULL;
    model = GUCEF_NULL;
    if ( reference.IsNull() )
        return false;
    if ( reference.IsBuiltin() )
    {
        CORE::CString name;
        if ( CUnityBuiltins::MakeBuiltinMesh( reference.fileId, mesh ) )
        {
            m_project.Info( subject, "uses the built-in mesh " + mesh.name + ", which is made as Unity has it" );
            return true;
        }
        if ( CUnityBuiltins::GetBuiltinMeshName( reference.fileId, name ) )
            m_project.Warn( subject, "uses the built-in mesh " + name + " " + reference.ToString() + ", which is not made: the node has no mesh" );
        else
            m_project.Warn( subject, "uses the built-in mesh " + reference.ToString() + ", which is not known: the node has no mesh" );
        return false;
    }

    CORE::CString path = vfsPath;
    if ( !reference.guid.IsNULLOrEmpty() && !m_project.ResolveGuid( reference.guid, path ) )
    {
        m_project.Warn( subject, "uses the mesh " + reference.ToString() + ", which is not in the package: the node has no mesh" );
        return false;
    }
    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( path );
    if ( CUnityModel::IsModelExtension( extension ) )
    {
        CUnityModel* source = m_project.GetModel( path );
        if ( GUCEF_NULL == source )
            return false;
        bool guessed = false;
        const Int32 node = source->FindMeshNode( reference.fileId, userName, &guessed );
        if ( guessed && node >= 0 )
            m_project.Approximate( subject, "the mesh " + reference.ToString() + " is not in the ID table of " + m_project.Describe( path ) + ".meta (IDs computed from names): " +
                                   ( 1 == source->scene.meshes.size() ? CORE::CString( "the only mesh of the model is used" ) : "the mesh of the node " + source->scene.nodes[ node ].name + " is used" ) );
        if ( node < 0 || source->scene.nodes[ node ].mesh < 0 )
        {
            m_project.Warn( subject, "uses the mesh " + reference.ToString() + ", which the ID table of " + m_project.Describe( path ) + ".meta does not tie to a mesh of the model: the node has no mesh" );
            return false;
        }
        mesh = source->scene.meshes[ source->scene.nodes[ node ].mesh ];
        if ( source->scene.nodes[ node ].skin >= 0 )
            skin = &source->scene.skins[ source->scene.nodes[ node ].skin ];
        model = source;
        return true;
    }

    // a Mesh document of a YAML file (.asset, or the file itself)
    const CUnityYamlFile* file = m_project.GetYaml( path );
    const CUnityYamlDocument* document = GUCEF_NULL != file ? file->FindDocument( reference.fileId ) : GUCEF_NULL;
    if ( GUCEF_NULL == document || 43 != document->classId )
    {
        m_project.Warn( subject, "uses the mesh " + reference.ToString() + ", which is no Mesh object of " + m_project.Describe( path ) + ": the node has no mesh" );
        return false;
    }
    CORE::CString problem;
    if ( !CUnityMeshDecoder::Decode( document->body, mesh, problem ) )
    {
        m_project.Warn( subject, "the mesh " + reference.ToString() + " of " + m_project.Describe( path ) + " cannot be decoded: " + problem );
        return false;
    }
    if ( !problem.IsNULLOrEmpty() )
        m_project.Warn( subject, "the mesh " + reference.ToString() + " of " + m_project.Describe( path ) + ": " + problem );
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityPrefabReader::GetRenderMesh( const CUnityReference& reference, const CORE::CString& vfsPath, const GUCEF::vector< Int32 >& materials, const CORE::CString& subject,
                                   const CORE::CString& userName, const ASSETCONVERTER::CAssetSkin*& skin, const CUnityModel*& model )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetMesh mesh;
    if ( !GetSourceMesh( reference, vfsPath, subject, userName, mesh, skin, model ) )
        return -1;

    CORE::CString key = "mesh|" + ( reference.guid.IsNULLOrEmpty() ? vfsPath + "|" : CORE::CString::Empty ) + reference.ToString() + "|" + userName;
    for ( size_t m=0; m<materials.size(); ++m )
        key += "|" + CORE::Int32ToString( materials[ m ] );
    Int32 index = -1;
    if ( m_project.TryGetCached( key, index ) )
        return index;

    if ( materials.size() < mesh.subMeshes.size() && !materials.empty() )
        m_project.Info( subject, "the renderer has " + CORE::UInt32ToString( (UInt32) materials.size() ) + " material(s) for " + CORE::UInt32ToString( (UInt32) mesh.subMeshes.size() ) +
                        " submeshes: Unity does not draw the others, they get the default material" );
    else
    if ( materials.size() > mesh.subMeshes.size() )
        m_project.Info( subject, "the renderer has more materials than the mesh has submeshes: Unity draws the last submesh again with them, which is not converted" );
    for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        mesh.subMeshes[ s ].materialIndex = s < materials.size() && materials[ s ] >= 0 ? materials[ s ] : m_materials.GetDefaultMaterial();

    m_project.GetScene().meshes.push_back( mesh );
    index = (Int32) m_project.GetScene().meshes.size() - 1;
    m_project.SetCached( key, index );
    return index;
}

/*-------------------------------------------------------------------------*/

void
CUnityPrefabReader::ReadRenderer( const CORE::CDataNode& renderer, const CUnityReference& meshReference, UInt32 node, const CORE::CString& vfsPath, bool skinned,
                                  const SFileResult& result )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    const CORE::CString subject = m_project.Describe( vfsPath ) + ": " + scene.nodes[ node ].name;
    if ( meshReference.IsNull() )
    {
        m_project.Warn( subject, "the renderer has no mesh" );
        return;
    }
    GUCEF::vector< Int32 > materials;
    CUnityYamlReader::TNodeList items;
    CUnityYamlReader::GetItems( renderer.FindChild( "m_Materials" ), items );
    for ( size_t i=0; i<items.size(); ++i )
    {
        CUnityReference reference;
        reference.Read( items[ i ] );
        materials.push_back( m_materials.GetMaterial( reference, subject ) );
    }
    const ASSETCONVERTER::CAssetSkin* modelSkin = GUCEF_NULL;
    const CUnityModel* model = GUCEF_NULL;
    const Int32 mesh = GetRenderMesh( meshReference, vfsPath, materials, subject, scene.nodes[ node ].name, modelSkin, model );
    if ( mesh < 0 )
        return;
    scene.nodes[ node ].mesh = mesh;
    if ( 0 == CUnityYamlReader::GetInt32( renderer, "m_Enabled", 1 ) )
        scene.nodes[ node ].extras.SetAttribute( "rendererEnabled", false );

    if ( !skinned )
        return;
    if ( GUCEF_NULL == modelSkin || GUCEF_NULL == model )
    {
        m_project.Warn( subject, "the skinned renderer's mesh has no skin in its model: it is drawn unskinned" );
        return;
    }

    // the joints: the bones of the renderer by the names of the joints of the model, else any node of the file by name
    GUCEF::vector< Int32 > bones;
    CUnityYamlReader::GetItems( renderer.FindChild( "m_Bones" ), items );
    for ( size_t i=0; i<items.size(); ++i )
    {
        CUnityReference reference;
        reference.Read( items[ i ] );
        TObjectNodeMap::const_iterator found = result.nodes.find( reference.fileId );
        bones.push_back( found != result.nodes.end() ? (*found).second : -1 );
    }
    ASSETCONVERTER::CAssetSkin skin;
    skin.name = modelSkin->name;
    skin.inverseBindMatrices = modelSkin->inverseBindMatrices;
    UInt32 unmapped = 0;

    // m_Bones in the order of the joints of the model, when every name agrees
    bool inOrder = bones.size() == modelSkin->joints.size();
    for ( size_t j=0; j<modelSkin->joints.size() && inOrder; ++j )
        inOrder = bones[ j ] >= 0 && scene.nodes[ bones[ j ] ].name == model->scene.nodes[ modelSkin->joints[ j ] ].name;
    GUCEF::set< Int32 > used;
    for ( size_t j=0; j<modelSkin->joints.size(); ++j )
    {
        if ( inOrder )
        {
            skin.joints.push_back( (UInt32) bones[ j ] );
            continue;
        }
        // by name: the bones of the renderer and the nodes of the file with the name of the joint; of several, the one
        // whose ancestors have the names of those of the joint for the longest, one not taken yet first
        const ASSETCONVERTER::CAssetNode& jointNode = model->scene.nodes[ modelSkin->joints[ j ] ];
        GUCEF::vector< Int32 > candidates;
        for ( size_t b=0; b<bones.size(); ++b )
            if ( bones[ b ] >= 0 && scene.nodes[ bones[ b ] ].name == jointNode.name )
                candidates.push_back( bones[ b ] );
        for ( TObjectNodeMap::const_iterator n = result.nodes.begin(); n != result.nodes.end(); ++n )
            if ( scene.nodes[ (*n).second ].name == jointNode.name )
                candidates.push_back( (*n).second );
        Int32 joint = -1;
        Int32 bestScore = -1;
        for ( size_t c=0; c<candidates.size(); ++c )
        {
            Int32 score = 0;
            Int32 a = scene.nodes[ candidates[ c ] ].parent;
            Int32 b = jointNode.parent;
            while ( a >= 0 && b >= 0 && scene.nodes[ a ].name == model->scene.nodes[ b ].name && score < 64 )
            {
                ++score;
                a = scene.nodes[ a ].parent;
                b = model->scene.nodes[ b ].parent;
            }
            score = score * 2 + ( used.find( candidates[ c ] ) == used.end() ? 1 : 0 );
            if ( score > bestScore )
            {
                bestScore = score;
                joint = candidates[ c ];
            }
        }
        if ( joint < 0 )
        {
            joint = (Int32) node;
            ++unmapped;
        }
        used.insert( joint );
        skin.joints.push_back( (UInt32) joint );
    }
    if ( unmapped > 0 )
        m_project.Warn( subject, CORE::UInt32ToString( unmapped ) + " joint(s) of the skin have no bone of the same name in the prefab: they follow the renderer" );
    const CUnityReference rootBone = CUnityReference::Get( renderer, "m_RootBone" );
    TObjectNodeMap::const_iterator root = result.nodes.find( rootBone.fileId );
    skin.skeletonRoot = root != result.nodes.end() ? (*root).second : -1;
    scene.skins.push_back( skin );
    scene.nodes[ node ].skin = (Int32) scene.skins.size() - 1;
}

/*-------------------------------------------------------------------------*/

void
CUnityPrefabReader::ReadCollider( const CUnityYamlDocument& document, const CORE::CDataNode& body, UInt32 node, const CORE::CString& vfsPath )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    const CORE::CString subject = m_project.Describe( vfsPath ) + ": " + scene.nodes[ node ].name;
    ASSETCONVERTER::CAssetCollider collider;
    collider.name = scene.nodes[ node ].name + "_" + document.typeName;
    collider.trigger = CUnityYamlReader::GetBool( body, "m_IsTrigger", false );
    Float32 centre[ 3 ] = { 0.0f, 0.0f, 0.0f };
    CUnityYamlReader::GetComponents( body, "m_Center", "xyz", centre );
    collider.transform.position = MATH::TFloat32Transform3D::TPositionVector( centre[ 0 ], centre[ 1 ], centre[ 2 ] );
    switch ( document.classId )
    {
        case ClassBoxCollider:
        {
            Float32 size[ 3 ] = { 1.0f, 1.0f, 1.0f };
            CUnityYamlReader::GetComponents( body, "m_Size", "xyz", size );
            collider.shape = ASSETCONVERTER::COLLIDERSHAPE_BOX;
            for ( UInt32 a=0; a<3; ++a )
                collider.halfExtents[ a ] = 0.5f * fabsf( size[ a ] );
            break;
        }
        case ClassSphereCollider:
        {
            collider.shape = ASSETCONVERTER::COLLIDERSHAPE_SPHERE;
            collider.radius = CUnityYamlReader::GetFloat( body, "m_Radius", 0.5f );
            break;
        }
        case ClassCapsuleCollider:
        {
            collider.shape = ASSETCONVERTER::COLLIDERSHAPE_CAPSULE;
            collider.radius = CUnityYamlReader::GetFloat( body, "m_Radius", 0.5f );
            collider.height = CUnityYamlReader::GetFloat( body, "m_Height", 2.0f );
            const Int32 direction = CUnityYamlReader::GetInt32( body, "m_Direction", 1 );
            const Float32 half = 0.70710678f;
            if ( 0 == direction )
            {
                // the local Y of the shape along X: a quarter turn about Z
                collider.transform.rotation = MATH::TFloat32Quaternion::FromXYZW( 0.0f, 0.0f, -half, half );
            }
            else
            if ( 2 == direction )
            {
                collider.transform.rotation = MATH::TFloat32Quaternion::FromXYZW( half, 0.0f, 0.0f, half );
            }
            break;
        }
        case ClassMeshCollider:
        {
            collider.shape = CUnityYamlReader::GetBool( body, "m_Convex", false ) ? ASSETCONVERTER::COLLIDERSHAPE_CONVEX : ASSETCONVERTER::COLLIDERSHAPE_TRIMESH;
            collider.transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 0.0f, 0.0f );
            const CUnityReference meshReference = CUnityReference::Get( body, "m_Mesh" );
            if ( meshReference.IsNull() )
            {
                m_project.Warn( subject, "the mesh collider has no mesh: it is left out" );
                return;
            }
            CORE::CString key = "collider|" + ( meshReference.guid.IsNULLOrEmpty() ? vfsPath + "|" : CORE::CString::Empty ) + meshReference.ToString() + "|" + scene.nodes[ node ].name;
            Int32 index = -1;
            if ( !m_project.TryGetCached( key, index ) )
            {
                ASSETCONVERTER::CAssetMesh mesh;
                const ASSETCONVERTER::CAssetSkin* skin = GUCEF_NULL;
                const CUnityModel* model = GUCEF_NULL;
                if ( GetSourceMesh( meshReference, vfsPath, subject, scene.nodes[ node ].name, mesh, skin, model ) )
                {
                    // a collision shape needs only the geometry
                    for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
                    {
                        mesh.subMeshes[ s ].materialIndex = -1;
                        mesh.subMeshes[ s ].joints.clear();
                        mesh.subMeshes[ s ].weights.clear();
                        mesh.subMeshes[ s ].influencesPerVertex = 0;
                        mesh.subMeshes[ s ].morphTargets.clear();
                    }
                    mesh.name = ( mesh.name.IsNULLOrEmpty() ? scene.nodes[ node ].name : mesh.name ) + "_collision";
                    scene.meshes.push_back( mesh );
                    index = (Int32) scene.meshes.size() - 1;
                }
                m_project.SetCached( key, index );
            }
            if ( index < 0 )
                return;
            collider.meshIndex = index;
            break;
        }
        default:
            return;
    }
    if ( 0 == CUnityYamlReader::GetInt32( body, "m_Enabled", 1 ) )
        m_project.Info( subject, "the " + document.typeName + " is disabled in Unity: it is converted" );
    scene.nodes[ node ].colliders.push_back( collider );
}

/*-------------------------------------------------------------------------*/

void
CUnityPrefabReader::ReadLight( const CORE::CDataNode& body, UInt32 node )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    const Int32 type = CUnityYamlReader::GetInt32( body, "m_Type", 2 );
    if ( 3 == type || 4 == type )
    {
        m_project.Skip( scene.nodes[ node ].name, "area lights are not punctual lights: the light is left out" );
        return;
    }
    ASSETCONVERTER::CAssetLight light;
    light.name = scene.nodes[ node ].name;
    light.type = 1 == type ? ASSETCONVERTER::LIGHTTYPE_DIRECTIONAL : ( 0 == type ? ASSETCONVERTER::LIGHTTYPE_SPOT : ASSETCONVERTER::LIGHTTYPE_POINT );
    Float32 colour[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    CUnityYamlReader::GetComponents( body, "m_Color", "rgba", colour );
    for ( UInt32 c=0; c<3; ++c )
        light.colour[ c ] = CUnityMaterialReader::SrgbToLinear( colour[ c ] );
    light.intensity = CUnityYamlReader::GetFloat( body, "m_Intensity", 1.0f );
    light.range = ASSETCONVERTER::LIGHTTYPE_DIRECTIONAL == light.type ? 0.0f : CUnityYamlReader::GetFloat( body, "m_Range", 10.0f );
    const Float32 degreesToRadians = 0.0174532925f;
    light.outerConeAngle = 0.5f * CUnityYamlReader::GetFloat( body, "m_SpotAngle", 30.0f ) * degreesToRadians;
    light.innerConeAngle = CUnityYamlReader::HasScalar( body, "m_InnerSpotAngle" ) ? 0.5f * CUnityYamlReader::GetFloat( body, "m_InnerSpotAngle", 0.0f ) * degreesToRadians : 0.0f;
    scene.lights.push_back( light );
    scene.nodes[ node ].light = (Int32) scene.lights.size() - 1;
    m_project.Approximate( light.name, "Unity's light intensity has no physical unit: it is written as the intensity of KHR_lights_punctual" );
}

/*-------------------------------------------------------------------------*/

void
CUnityPrefabReader::ReadLodGroup( const CORE::CDataNode& body, UInt32 node, const SFileResult& result )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    ASSETCONVERTER::CAssetLodGroup group;
    group.name = scene.nodes[ node ].name;
    CUnityYamlReader::TNodeList levels;
    CUnityYamlReader::GetItems( body.FindChild( "m_LODs" ), levels );
    for ( size_t l=0; l<levels.size(); ++l )
    {
        const Float32 height = CUnityYamlReader::GetFloat( *levels[ l ], "screenRelativeHeight", -1.0f );
        CUnityYamlReader::TNodeList renderers;
        CUnityYamlReader::GetItems( levels[ l ]->FindChild( "renderers" ), renderers );
        Int32 levelNode = -1;
        for ( size_t r=0; r<renderers.size() && levelNode < 0; ++r )
        {
            CUnityReference reference;
            reference.Read( renderers[ r ]->FindChild( "renderer" ) );
            TObjectNodeMap::const_iterator found = result.nodes.find( reference.fileId );
            if ( found != result.nodes.end() )
                levelNode = (*found).second;
        }
        if ( renderers.size() > 1 )
            m_project.Approximate( group.name, "LOD level " + CORE::UInt32ToString( (UInt32) l ) + " has " + CORE::UInt32ToString( (UInt32) renderers.size() ) + " renderers: the node of the first is the level" );
        if ( levelNode < 0 )
        {
            m_project.Warn( group.name, "LOD level " + CORE::UInt32ToString( (UInt32) l ) + " has no renderer of the prefab: it is left out" );
            continue;
        }
        group.AddLevel( (UInt32) levelNode, height );
    }
    if ( group.levels.empty() )
        return;
    scene.lodGroups.push_back( group );
    scene.nodes[ node ].lodGroup = (Int32) scene.lodGroups.size() - 1;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPrefabReader::ReadFile( const CORE::CString& vfsPath, const TObjectOverrideMap* overrides, const TIdSet* removed, UInt32 depth, SFileResult& result,
                              ASSETCONVERTER::CAssetPrefab* record )
{GUCEF_TRACE;

    const CUnityYamlFile* file = m_project.GetYaml( vfsPath );
    if ( GUCEF_NULL == file )
        return false;
    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    const CORE::CString fileSubject = m_project.Describe( vfsPath );
    const UInt32 count = file->GetDocumentCount();

    // the prefab instances, each read with its modifications
    typedef GUCEF::map< Int64, SFileResult > TInstanceMap;
    TInstanceMap instances;
    GUCEF::vector< Int64 > instanceOrder;
    GUCEF::map< Int64, ASSETCONVERTER::CAssetPrefabInstance > instanceRecords;
    for ( UInt32 d=0; d<count; ++d )
    {
        const CUnityYamlDocument& document = file->GetDocument( d );
        const CORE::CDataNode& body = BodyOf( document, overrides );
        if ( !IsInstance( document, body ) )
            continue;
        SFileResult inner;
        ASSETCONVERTER::CAssetPrefabInstance instance;
        if ( ExpandInstance( document, body, vfsPath, depth, inner, instance ) )
        {
            instances[ document.fileId ] = inner;
            instanceOrder.push_back( document.fileId );
            instanceRecords[ document.fileId ] = instance;
        }
    }

    // the GameObjects
    for ( UInt32 d=0; d<count; ++d )
    {
        const CUnityYamlDocument& document = file->GetDocument( d );
        if ( ClassGameObject != document.classId || document.stripped )
            continue;
        const CORE::CDataNode& body = BodyOf( document, overrides );
        const UInt32 node = NewNode( CUnityYamlReader::GetString( body, "m_Name" ) );
        result.nodes[ document.fileId ] = (Int32) node;
        if ( 0 == CUnityYamlReader::GetInt32( body, "m_IsActive", 1 ) )
        {
            scene.nodes[ node ].extras.SetAttribute( "active", false );
            m_inactive.insert( node );
        }
    }

    // the placeholders of the objects of instances
    for ( UInt32 d=0; d<count; ++d )
    {
        const CUnityYamlDocument& document = file->GetDocument( d );
        if ( !document.stripped )
            continue;
        const CORE::CDataNode& body = document.body;
        CUnityReference instance = CUnityReference::Get( body, "m_PrefabInstance" );
        if ( instance.IsNull() )
            instance = CUnityReference::Get( body, "m_PrefabInternal" );
        CUnityReference source = CUnityReference::Get( body, "m_CorrespondingSourceObject" );
        if ( source.IsNull() )
            source = CUnityReference::Get( body, "m_PrefabParentObject" );
        TInstanceMap::const_iterator expanded = instances.find( instance.fileId );
        if ( expanded == instances.end() )
            continue;
        TObjectNodeMap::const_iterator node = (*expanded).second.nodes.find( source.fileId );
        if ( node != (*expanded).second.nodes.end() )
            result.nodes[ document.fileId ] = (*node).second;
        else
            m_project.Warn( fileSubject, "the placeholder &" + CORE::Int64ToString( document.fileId ) + " names " + source.ToString() + ", which its prefab instance does not have" );
    }

    // the Transforms: the node of their GameObject
    GUCEF::vector< const CUnityYamlDocument* > transforms;
    for ( UInt32 d=0; d<count; ++d )
    {
        const CUnityYamlDocument& document = file->GetDocument( d );
        if ( ( ClassTransform != document.classId && ClassRectTransform != document.classId ) || document.stripped )
            continue;
        const CORE::CDataNode& body = BodyOf( document, overrides );
        const CUnityReference gameObject = CUnityReference::Get( body, "m_GameObject" );
        TObjectNodeMap::const_iterator node = result.nodes.find( gameObject.fileId );
        if ( node == result.nodes.end() )
        {
            m_project.Warn( fileSubject, "the Transform &" + CORE::Int64ToString( document.fileId ) + " has no GameObject: it is left out" );
            continue;
        }
        result.nodes[ document.fileId ] = (*node).second;
        ReadTransform( body, scene.nodes[ (*node).second ] );
        transforms.push_back( &document );
    }

    // the hierarchy: the children lists of the Transforms, then the parents, then the parents of the instances
    GUCEF::set< UInt32 > linked;
    for ( size_t t=0; t<transforms.size(); ++t )
    {
        const CORE::CDataNode& body = BodyOf( *transforms[ t ], overrides );
        const UInt32 parent = (UInt32) result.nodes[ transforms[ t ]->fileId ];
        CUnityYamlReader::TNodeList children;
        CUnityYamlReader::GetItems( body.FindChild( "m_Children" ), children );
        for ( size_t c=0; c<children.size(); ++c )
        {
            CUnityReference child;
            child.Read( children[ c ] );
            TObjectNodeMap::const_iterator node = result.nodes.find( child.fileId );
            if ( node == result.nodes.end() || (UInt32) (*node).second == parent || linked.find( (*node).second ) != linked.end() )
                continue;
            if ( scene.nodes[ (*node).second ].parent >= 0 )
                continue;   // a node of an instance that already has its parent inside the instance
            Link( (UInt32) (*node).second, (Int32) parent );
            linked.insert( (*node).second );
        }
    }
    for ( size_t t=0; t<transforms.size(); ++t )
    {
        const UInt32 node = (UInt32) result.nodes[ transforms[ t ]->fileId ];
        if ( linked.find( node ) != linked.end() )
            continue;
        const CUnityReference father = CUnityReference::Get( BodyOf( *transforms[ t ], overrides ), "m_Father" );
        TObjectNodeMap::const_iterator parent = result.nodes.find( father.fileId );
        if ( !father.IsNull() && parent != result.nodes.end() && (UInt32) (*parent).second != node )
            Link( node, (*parent).second );
        else
            result.roots.push_back( node );
        linked.insert( node );
    }
    for ( size_t i=0; i<instanceOrder.size(); ++i )
    {
        const SFileResult& inner = instances[ instanceOrder[ i ] ];
        const CUnityYamlDocument* document = file->FindDocument( instanceOrder[ i ] );
        const CORE::CDataNode* modification = GUCEF_NULL != document ? BodyOf( *document, overrides ).FindChild( "m_Modification" ) : GUCEF_NULL;
        const CUnityReference parentReference = GUCEF_NULL != modification ? CUnityReference::Get( *modification, "m_TransformParent" ) : CUnityReference();
        TObjectNodeMap::const_iterator parent = result.nodes.find( parentReference.fileId );
        for ( size_t r=0; r<inner.roots.size(); ++r )
        {
            if ( linked.find( inner.roots[ r ] ) != linked.end() )
                continue;
            if ( !parentReference.IsNull() && parent != result.nodes.end() )
                Link( inner.roots[ r ], (*parent).second );
            else
                result.roots.push_back( inner.roots[ r ] );
            linked.insert( inner.roots[ r ] );
        }
        // the objects of the instance are objects of this file too, by their IDs in the instance
        for ( TObjectNodeMap::const_iterator n = inner.nodes.begin(); n != inner.nodes.end(); ++n )
            if ( result.nodes.find( (*n).first ) == result.nodes.end() )
                result.nodes[ (*n).first ] = (*n).second;
    }
    for ( TObjectNodeMap::const_iterator n = result.nodes.begin(); n != result.nodes.end(); ++n )
    {
        const CUnityYamlDocument* document = file->FindDocument( (*n).first );
        if ( GUCEF_NULL != document && ClassGameObject == document->classId && !document->stripped && linked.find( (*n).second ) == linked.end() &&
             scene.nodes[ (*n).second ].parent < 0 )
        {
            result.roots.push_back( (*n).second );
            linked.insert( (*n).second );
        }
    }

    // the components
    typedef GUCEF::map< UInt32, CUnityReference > TMeshMap;
    TMeshMap meshFilters;
    GUCEF::vector< std::pair< UInt32, const CUnityYamlDocument* > > renderers;
    GUCEF::vector< std::pair< UInt32, const CUnityYamlDocument* > > lodGroups;
    GUCEF::vector< std::pair< UInt32, const CUnityYamlDocument* > > particleSystems;
    GUCEF::map< UInt32, const CUnityYamlDocument* > particleRenderers;
    CORE::CString::StringSet skippedKinds;
    for ( UInt32 d=0; d<count; ++d )
    {
        const CUnityYamlDocument& document = file->GetDocument( d );
        if ( document.stripped || ClassGameObject == document.classId || ClassTransform == document.classId || ClassRectTransform == document.classId ||
             ClassPrefab == document.classId || ClassEditorExtension == document.classId )
            continue;
        const CORE::CDataNode& body = BodyOf( document, overrides );
        const CUnityReference gameObject = CUnityReference::Get( body, "m_GameObject" );
        if ( gameObject.IsNull() )
            continue;   // no component: settings of a scene, assets inside the file
        TObjectNodeMap::const_iterator owner = result.nodes.find( gameObject.fileId );
        if ( owner == result.nodes.end() )
        {
            m_project.Warn( fileSubject, "the " + document.typeName + " &" + CORE::Int64ToString( document.fileId ) + " belongs to no GameObject of the file: it is left out" );
            continue;
        }
        if ( GUCEF_NULL != removed && removed->find( document.fileId ) != removed->end() )
            continue;
        const UInt32 node = (UInt32) (*owner).second;
        result.nodes[ document.fileId ] = (Int32) node;
        switch ( document.classId )
        {
            case ClassMeshFilter:
                meshFilters[ node ] = CUnityReference::Get( body, "m_Mesh" );
                break;
            case ClassMeshRenderer:
            case ClassSkinnedMeshRenderer:
                renderers.push_back( std::pair< UInt32, const CUnityYamlDocument* >( node, &document ) );
                break;
            case ClassBoxCollider:
            case ClassSphereCollider:
            case ClassCapsuleCollider:
            case ClassMeshCollider:
                ReadCollider( document, body, node, vfsPath );
                break;
            case ClassLight:
                ReadLight( body, node );
                break;
            case ClassLodGroup:
                lodGroups.push_back( std::pair< UInt32, const CUnityYamlDocument* >( node, &document ) );
                break;
            case ClassParticleSystem:
                particleSystems.push_back( std::pair< UInt32, const CUnityYamlDocument* >( node, &document ) );
                break;
            case ClassParticleSystemRenderer:
                particleRenderers[ node ] = &document;
                break;
            case ClassRigidbody:
            {
                CORE::CDataNode* rigidbody = scene.nodes[ node ].extras.AddChild( "rigidbody", GUCEF_DATATYPE_OBJECT );
                rigidbody->SetAttribute( "mass", CUnityYamlReader::GetFloat( body, "m_Mass", 1.0f ) );
                rigidbody->SetAttribute( "isKinematic", CUnityYamlReader::GetBool( body, "m_IsKinematic", false ) );
                rigidbody->SetAttribute( "useGravity", CUnityYamlReader::GetBool( body, "m_UseGravity", true ) );
                m_project.Skip( fileSubject, "the Rigidbody of " + scene.nodes[ node ].name + " is kept in the extras of its node, not as a physics body" );
                break;
            }
            default:
                skippedKinds.insert( document.typeName.IsNULLOrEmpty() ? "class " + CORE::Int32ToString( document.classId ) : document.typeName );
                break;
        }
    }
    for ( CORE::CString::StringSet::const_iterator k = skippedKinds.begin(); k != skippedKinds.end(); ++k )
        m_project.Skip( fileSubject, "the " + (*k) + " components are not part of the intermediate" );
    for ( size_t r=0; r<renderers.size(); ++r )
    {
        const UInt32 node = renderers[ r ].first;
        const CUnityYamlDocument& document = *renderers[ r ].second;
        const CORE::CDataNode& body = BodyOf( document, overrides );
        const bool skinned = ClassSkinnedMeshRenderer == document.classId;
        CUnityReference meshReference;
        if ( skinned )
        {
            meshReference = CUnityReference::Get( body, "m_Mesh" );
        }
        else
        {
            TMeshMap::const_iterator filter = meshFilters.find( node );
            if ( filter != meshFilters.end() )
                meshReference = (*filter).second;
        }
        ReadRenderer( body, meshReference, node, vfsPath, skinned, result );
    }
    for ( size_t l=0; l<lodGroups.size(); ++l )
        ReadLodGroup( BodyOf( *lodGroups[ l ].second, overrides ), lodGroups[ l ].first, result );

    // the particle systems with the renderers of their GameObjects; then the sub-emitters, by the file IDs of systems
    GUCEF::map< Int64, Int32 > systemsById;
    GUCEF::vector< std::pair< Int32, CUnityParticleReader::TSubEmitterReferenceVector > > subEmitterLists;
    for ( size_t p=0; p<particleSystems.size(); ++p )
    {
        const UInt32 node = particleSystems[ p ].first;
        const CUnityYamlDocument& document = *particleSystems[ p ].second;
        GUCEF::map< UInt32, const CUnityYamlDocument* >::const_iterator renderer = particleRenderers.find( node );
        const CORE::CDataNode* rendererBody = renderer != particleRenderers.end() ? &BodyOf( *(*renderer).second, overrides ) : GUCEF_NULL;
        CUnityReference meshReference;
        CUnityParticleReader::TSubEmitterReferenceVector subEmitters;
        const Int32 system = m_particles.Read( BodyOf( document, overrides ), rendererBody, scene.nodes[ node ].name, meshReference, subEmitters );
        if ( system < 0 )
            continue;
        scene.nodes[ node ].particleSystem = system;
        systemsById[ document.fileId ] = system;
        if ( !subEmitters.empty() )
            subEmitterLists.push_back( std::pair< Int32, CUnityParticleReader::TSubEmitterReferenceVector >( system, subEmitters ) );
        if ( !meshReference.IsNull() )
        {
            GUCEF::vector< Int32 > materials;
            materials.push_back( scene.particleSystems[ system ].renderer.material );
            const ASSETCONVERTER::CAssetSkin* modelSkin = GUCEF_NULL;
            const CUnityModel* model = GUCEF_NULL;
            scene.particleSystems[ system ].renderer.mesh = GetRenderMesh( meshReference, vfsPath, materials, fileSubject, scene.nodes[ node ].name, modelSkin, model );
        }
    }
    for ( size_t s=0; s<subEmitterLists.size(); ++s )
    {
        ASSETCONVERTER::CAssetParticleSystem& system = scene.particleSystems[ subEmitterLists[ s ].first ];
        for ( size_t e=0; e<subEmitterLists[ s ].second.size(); ++e )
        {
            const CUnityParticleReader::SSubEmitterReference& reference = subEmitterLists[ s ].second[ e ];
            GUCEF::map< Int64, Int32 >::const_iterator target = systemsById.find( reference.emitter );
            if ( target == systemsById.end() )
            {
                m_project.Warn( fileSubject, "the sub-emitter &" + CORE::Int64ToString( reference.emitter ) + " of " + system.name + " is not a particle system of the file" );
                continue;
            }
            ASSETCONVERTER::CAssetParticleSystem::SSubEmitter sub;
            sub.system = (*target).second;
            sub.trigger = reference.trigger;
            sub.probability = reference.probability;
            sub.inherit = reference.inherit;
            system.subEmitters.push_back( sub );
        }
    }

    // the record of the asset: its root and its direct instances
    if ( GUCEF_NULL != record )
    {
        for ( size_t i=0; i<instanceOrder.size(); ++i )
        {
            const SFileResult& inner = instances[ instanceOrder[ i ] ];
            if ( inner.roots.empty() )
                continue;
            ASSETCONVERTER::CAssetPrefabInstance instance = instanceRecords[ instanceOrder[ i ] ];
            instance.node = inner.roots[ 0 ];

            // the cooker's material overrides: the materials per submesh of the root, when the modifications change them
            bool materialsChanged = false;
            const CORE::CDataNode* modifications = instance.overrides.FindChild( "modifications" );
            if ( GUCEF_NULL != modifications )
            {
                CORE::CDataNode::const_iterator m = modifications->ConstBegin();
                while ( m != modifications->ConstEnd() )
                {
                    if ( (*m)->GetAttributeValueOrChildValueByName( "propertyPath" ).AsString().StartsWith( "m_Materials." ) )
                        materialsChanged = true;
                    ++m;
                }
            }
            const Int32 mesh = scene.nodes[ instance.node ].mesh;
            if ( materialsChanged && mesh >= 0 )
            {
                CORE::CString::StringVector names;
                for ( size_t s=0; s<scene.meshes[ mesh ].subMeshes.size(); ++s )
                {
                    const Int32 material = scene.meshes[ mesh ].subMeshes[ s ].materialIndex;
                    names.push_back( material >= 0 ? scene.materials[ material ].name : CORE::CString::Empty );
                }
                instance.overrides.SetValuesOfChildByName( "materials", names );
            }
            record->instances.push_back( instance );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPrefabReader::ReadAsset( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    ASSETCONVERTER::CAssetPrefab prefab;
    prefab.name = ASSETCONVERTER::CVfsPaths::StemOf( vfsPath );
    SFileResult result;
    m_stack.push_back( vfsPath );
    const bool ok = ReadFile( vfsPath, GUCEF_NULL, GUCEF_NULL, 0, result, &prefab );
    m_stack.pop_back();
    if ( !ok )
        return false;
    if ( result.roots.empty() )
    {
        m_project.Warn( m_project.Describe( vfsPath ), "the file holds no GameObject" );
        return false;
    }

    // one root: the prefab's; several (a scene): a root named after the asset holds them
    UInt32 root = result.roots[ 0 ];
    if ( 1 == result.roots.size() )
    {
        Link( root, -1 );
    }
    else
    {
        root = NewNode( prefab.name );
        Link( root, -1 );
        for ( size_t r=0; r<result.roots.size(); ++r )
            Link( result.roots[ r ], (Int32) root );
    }
    prefab.rootNode = root;
    scene.prefabs.push_back( prefab );

    // node names made unique, in the order of the hierarchy: skeletons and the cookers' lookups by name need it
    GUCEF::set< CORE::CString > taken;
    UInt32 renamed = 0;
    GUCEF::vector< UInt32 > order( 1, root );
    for ( size_t i=0; i<order.size(); ++i )
    {
        ASSETCONVERTER::CAssetNode& node = scene.nodes[ order[ i ] ];
        const CORE::CString original = node.name;
        for ( UInt32 suffix=1; taken.find( node.name ) != taken.end(); ++suffix )
            node.name = original + " " + CORE::UInt32ToString( suffix );
        if ( node.name != original )
            ++renamed;
        taken.insert( node.name );
        order.insert( order.end(), node.children.begin(), node.children.end() );
    }
    if ( renamed > 0 )
        m_project.Info( m_project.Describe( vfsPath ), CORE::UInt32ToString( renamed ) + " node(s) share the name of another and get a number appended, as Unity names duplicates" );

    // objects that are not active, when the settings leave them out
    if ( !m_inactive.empty() && m_project.GetSettingBool( "skipInactive", false ) )
    {
        ASSETCONVERTER::CAssetScene::TUInt32Set remove;
        for ( TIdSet::const_iterator i = m_inactive.begin(); i != m_inactive.end(); ++i )
            if ( (UInt32) (*i) != root )
                remove.insert( (UInt32) (*i) );
        const UInt32 removed = scene.RemoveNodes( remove );
        scene.RemoveUnusedMeshes();
        scene.RemoveUnusedMaterials();
        scene.RemoveUnusedTextures();
        m_project.Info( m_project.Describe( vfsPath ), CORE::UInt32ToString( removed ) + " node(s) of inactive objects are left out (skipInactive)" );
    }
    else
    if ( !m_inactive.empty() )
    {
        m_project.Info( m_project.Describe( vfsPath ), CORE::UInt32ToString( (UInt32) m_inactive.size() ) + " object(s) are inactive in Unity: they are converted with the extra \"active\": false" );
    }
    return true;
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
