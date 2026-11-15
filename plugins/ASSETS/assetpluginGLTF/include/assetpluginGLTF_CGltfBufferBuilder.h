/*
 *  assetpluginGLTF: the binary buffer of a glTF file with its buffer views and accessors
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFBUFFERBUILDER_H
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFBUFFERBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H
#include "assetpluginGLTF_CGltfJsonWriter.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The one binary buffer of a glTF file. Every accessor gets a buffer view of its own, at a 4 byte aligned offset, so
 *  nothing depends on strides. The data is little endian, as glTF has it. The arrays of buffer views and accessors
 *  are written to the JSON after everything else has been added.
 */
class GUCEF_ASSETPLUGIN_GLTF_EXPORT_CPP CGltfBufferBuilder
{
    public:

    /** The component types of accessors, with their glTF numbers */
    enum EComponentType
    {
        COMPONENT_UBYTE  = 5121,
        COMPONENT_USHORT = 5123,
        COMPONENT_UINT   = 5125,
        COMPONENT_FLOAT  = 5126
    };

    /** The element types of accessors */
    enum EElementType
    {
        ELEMENT_SCALAR = 0,
        ELEMENT_VEC2      ,
        ELEMENT_VEC3      ,
        ELEMENT_VEC4      ,
        ELEMENT_MAT4
    };

    /** The targets of buffer views, with their glTF numbers; 0 for none */
    enum ETarget
    {
        TARGET_NONE          = 0    ,
        TARGET_ARRAY         = 34962,
        TARGET_ELEMENT_ARRAY = 34963
    };

    CGltfBufferBuilder( void );

    ~CGltfBufferBuilder();

    /** The number of components of an element type */
    static UInt32 GetComponentCount( EElementType type );

    /** The bytes of one component */
    static UInt32 GetComponentSize( EComponentType type );

    /** Adds 'elementCount' elements of 'data' (tightly packed); the index of the accessor. 'min' and 'max' have one value per component, or are NULL */
    UInt32 AddAccessor( const void* data, UInt32 elementCount, EComponentType componentType, EElementType elementType, ETarget target, const Float32* min, const Float32* max );

    /** Adds float elements; the bounds per component are computed and written when 'withBounds' is set */
    UInt32 AddFloats( const Float32* values, UInt32 elementCount, EElementType elementType, ETarget target, bool withBounds );

    /** Adds triangle indices: 16 bit when every index fits, else 32 bit */
    UInt32 AddIndices( const UInt32* indices, UInt32 count );

    /** Adds 16 bit values with 4 per element (joint indices) */
    UInt32 AddUInt16Vec4( const UInt32* values, UInt32 elementCount, ETarget target );

    UInt32 GetAccessorCount( void ) const;

    /** The bytes of the buffer, padded to a multiple of 4 */
    const CORE::CDynamicBuffer& GetBinary( void ) const;

    UInt32 GetByteLength( void ) const;

    /** Writes the array "bufferViews" and the array "accessors" as members of the object that is open */
    void WriteTo( CGltfJsonWriter& writer ) const;

    private:

    CGltfBufferBuilder( const CGltfBufferBuilder& src );                /**< not implemented */
    CGltfBufferBuilder& operator=( const CGltfBufferBuilder& src );     /**< not implemented */

    struct SAccessor
    {
        UInt32 byteOffset;
        UInt32 byteLength;
        UInt32 target;
        UInt32 componentType;
        UInt32 elementType;
        UInt32 count;
        bool hasBounds;
        Float32 min[ 16 ];
        Float32 max[ 16 ];
    };
    typedef GUCEF::vector< SAccessor > TAccessorVector;

    private:

    CORE::CDynamicBuffer m_binary;
    TAccessorVector m_accessors;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFBUFFERBUILDER_H ? */
