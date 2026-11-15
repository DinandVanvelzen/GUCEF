/*
 *  AssetViewerLib: what a mesh holds: statistics and the geometry read back for overlays and the UV layout
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

#ifndef ASSETVIEWERLIB_CMESHINSPECTOR_H
#define ASSETVIEWERLIB_CMESHINSPECTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreMesh.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a mesh holds, as the inspector lists it */
struct ASSETVIEWERLIB_PUBLIC_CPP SMeshStatistics
{
    UInt32 subMeshCount;
    UInt32 vertexCount;
    UInt32 triangleCount;
    UInt32 boneCount;
    UInt32 animationCount;
    UInt32 lodLevelCount;
    UInt32 materialCount;
    UInt32 textureCount;
    UInt32 uvSetCount;              /**< the most texture coordinate sets of a sub mesh */
    UInt64 textureMemory;           /**< bytes of the textures the materials use, as the render system holds them */
    bool hasNormals;                /**< every sub mesh has them */
    bool hasTangents;
    bool hasVertexColours;
    bool hasSkeleton;
    Float32 boundsMin[ 3 ];
    Float32 boundsMax[ 3 ];
    Float32 boundingRadius;
    CORE::CString::StringVector materialNames;
    CORE::CString::StringVector textureNames;
    SMeshStatistics( void );
};

/** The geometry of a sub mesh as plain arrays, in the space of the mesh */
struct ASSETVIEWERLIB_PUBLIC_CPP SSubMeshGeometry
{
    GUCEF::vector< Float32 > positions;     /**< three per vertex */
    GUCEF::vector< Float32 > normals;       /**< three per vertex, empty when the sub mesh has none */
    GUCEF::vector< Float32 > tangents;      /**< three per vertex, empty when the sub mesh has none */
    GUCEF::vector< Float32 > texCoords;     /**< two per vertex of the first set, empty when there is none */
    GUCEF::vector< UInt32 > indices;        /**< three per triangle, strips and fans written out as lists */
    UInt32 vertexCount;
    UInt32 triangleCount;
    SSubMeshGeometry( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads what a loaded mesh holds. The vertex and index buffers are read back from the render system with readData(), which
 *  the buffers of every render system of Ogre allow (also the ones made as write only for the GPU).
 */
class ASSETVIEWERLIB_PUBLIC_CPP CMeshInspector
{
    public:

    /** The counts, features, bounds and the materials and textures of a mesh; loads the textures of its materials to know their size */
    static void GetStatistics( const Ogre::MeshPtr& mesh, SMeshStatistics& statistics );

    /** The geometry of a sub mesh; false for a sub mesh that does not exist or buffers that cannot be read */
    static bool ReadSubMesh( const Ogre::MeshPtr& mesh, UInt32 subMeshIndex, SSubMeshGeometry& geometry );

    /**
     *  The bounds of the vertex positions of all sub meshes. Unlike the bounds of the mesh these have no padding (Ogre pads the
     *  bounds it is given by its bounds padding factor): the size of the model as its file has it. Read from the shadow buffers
     *  of a mesh loaded with them (CModelView loads meshes so); false without positions or for a mesh without shadow buffers
     */
    static bool GetGeometryBounds( const Ogre::MeshPtr& mesh, Ogre::AxisAlignedBox& bounds );

    /** The vertex data a sub mesh draws from (its own or the shared data of the mesh) */
    static Ogre::VertexData* GetVertexData( const Ogre::MeshPtr& mesh, UInt32 subMeshIndex );

    /** The triangles an index count makes with the operation type of the sub mesh (0 for lines and points) */
    static UInt32 CountTriangles( Ogre::RenderOperation::OperationType operationType, UInt32 indexCount );

    /** The statistics as the data tree the inspector shows, in the groups Statistics and Features */
    static void WriteToDataNode( const SMeshStatistics& statistics, CORE::CDataNode& node );

    /** The size of bytes as a short text, "1.5 MB" */
    static CORE::CString FormatByteCount( UInt64 bytes );

    private:

    CMeshInspector( void );
    CMeshInspector( const CMeshInspector& src );
    CMeshInspector& operator=( const CMeshInspector& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CMESHINSPECTOR_H ? */
