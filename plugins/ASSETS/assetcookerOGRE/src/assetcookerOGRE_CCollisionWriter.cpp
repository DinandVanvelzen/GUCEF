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

#include "assetcookerOGRE_CCollisionWriter.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
NodePath( const ASSETCONVERTER::CAssetScene& scene, Int32 node )
{
    CORE::CString path;
    UInt32 guard = 0;
    while ( node >= 0 && (size_t) node < scene.nodes.size() && guard++ < scene.nodes.size() )
    {
        path = path.IsNULLOrEmpty() ? scene.nodes[ node ].name : scene.nodes[ node ].name + "/" + path;
        node = scene.nodes[ node ].parent;
    }
    return path;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CCollisionWriter::CountColliders( const ASSETCONVERTER::CAssetScene& scene )
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
        count += (UInt32) scene.nodes[ n ].colliders.size();
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CCollisionWriter::Build( const ASSETCONVERTER::CAssetScene& scene      ,
                         const CORE::CString& assetPath                ,
                         const CORE::CString& sceneName                ,
                         const TNameMap& nodeNames                     ,
                         CORE::CDataNode& document                     )
{GUCEF_TRACE;

    document = CORE::CDataNode( "collision", GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "asset", assetPath );
    document.SetAttribute( "scene", sceneName );
    CORE::CDataNode* colliders = document.AddChild( "colliders", GUCEF_DATATYPE_ARRAY );
    UInt32 count = 0;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        for ( size_t c=0; c<scene.nodes[ n ].colliders.size(); ++c )
        {
            const ASSETCONVERTER::CAssetCollider& collider = scene.nodes[ n ].colliders[ c ];
            CORE::CDataNode* entry = colliders->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            TNameMap::const_iterator name = nodeNames.find( (UInt32) n );
            entry->SetAttribute( "node", name != nodeNames.end() ? (*name).second : scene.nodes[ n ].name );
            entry->SetAttribute( "nodePath", NodePath( scene, (Int32) n ) );
            collider.Serialize( *entry );
            CORE::CDataNode::TVariantVector scale;
            scale.push_back( CORE::CVariant( collider.transform.scale.x ) );
            scale.push_back( CORE::CVariant( collider.transform.scale.y ) );
            scale.push_back( CORE::CVariant( collider.transform.scale.z ) );
            entry->SetValuesOfChildByName( "scale", scale );

            if ( collider.IsMeshShape() )
            {
                entry->DelAttribute( "mesh" );
                CORE::CDataNode* mesh = entry->AddChild( "mesh", GUCEF_DATATYPE_OBJECT );
                CORE::CDataNode::TVariantVector vertices;
                CORE::CDataNode::TVariantVector indices;
                if ( collider.meshIndex >= 0 && (size_t) collider.meshIndex < scene.meshes.size() )
                {
                    const ASSETCONVERTER::CAssetMesh& source = scene.meshes[ collider.meshIndex ];
                    UInt32 offset = 0;
                    for ( size_t s=0; s<source.subMeshes.size(); ++s )
                    {
                        const ASSETCONVERTER::CAssetSubMesh& subMesh = source.subMeshes[ s ];
                        for ( size_t i=0; i<subMesh.positions.size(); ++i )
                            vertices.push_back( CORE::CVariant( subMesh.positions[ i ] ) );
                        for ( size_t i=0; i<subMesh.indices.size(); ++i )
                            indices.push_back( CORE::CVariant( subMesh.indices[ i ] + offset ) );
                        offset += subMesh.GetVertexCount();
                    }
                }
                mesh->SetValuesOfChildByName( "vertices", vertices );
                mesh->SetValuesOfChildByName( "indices", indices );
            }
            ++count;
        }
    }
    return count > 0;
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
