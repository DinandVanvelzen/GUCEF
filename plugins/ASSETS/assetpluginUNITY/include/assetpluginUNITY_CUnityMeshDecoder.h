/*
 *  assetpluginUNITY: the decoder of serialised Unity Mesh objects
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMESHDECODER_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMESHDECODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETMESH_H
#include "AssetConverterLib_CAssetMesh.h"
#define ASSETCONVERTERLIB_CASSETMESH_H
#endif /* ASSETCONVERTERLIB_CASSETMESH_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_MACROS_H
#include "assetpluginUNITY_macros.h"
#define GUCEF_ASSETPLUGIN_UNITY_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decodes a Mesh object (class 43) of a Unity YAML file, as the generated collision meshes of a package hold them,
 *  into a mesh of the model, in Unity's coordinates (the frame of the object that uses it):
 *
 *  - m_IndexBuffer: hexadecimal, 16 bit indices, or 32 bit with m_IndexFormat 1;
 *  - m_SubMeshes: firstByte, indexCount, topology (0 triangles), baseVertex, firstVertex, vertexCount;
 *  - m_VertexData: m_VertexCount, m_Channels (stream, offset, format, dimension) and the hexadecimal _typelessdata.
 *    Eight channels are the layout of Unity 5 to 2018 (position, normal, colour, UV 0 to 3, tangent), fourteen that of
 *    2019 and later (position, normal, tangent, colour, UV 0 to 7, weights, indices). Streams follow each other,
 *    each starting on a 16 byte boundary. Floats of 32 and 16 bits and 8 bit colours are read;
 *  - a mesh stored compressed (m_CompressedMesh) is not decoded.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityMeshDecoder
{
    public:

    /** Decodes the Mesh object; false with the reason when it cannot */
    static bool Decode( const CORE::CDataNode& meshObject, ASSETCONVERTER::CAssetMesh& mesh, CORE::CString& problem );

    /** Decodes hexadecimal text into bytes; false when it holds other characters or has an odd length */
    static bool DecodeHex( const CORE::CString& text, GUCEF::vector< UInt8 >& bytes );

    private:

    CUnityMeshDecoder( void );
    ~CUnityMeshDecoder();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMESHDECODER_H ? */
