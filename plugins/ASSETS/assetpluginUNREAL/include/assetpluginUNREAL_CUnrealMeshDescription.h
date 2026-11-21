/*
 *  assetpluginUNREAL: The mesh description of a static mesh source: elements and attributes
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The decompressed payload of a mesh description (the import source of a static mesh in UE 5): a list of element kinds (Vertices,
 *  VertexInstances, UVs, Edges, Triangles, Polygons, PolygonGroups), each with one or more channels (the UV element has one per UV
 *  set), each channel a sparse set of ids (a bit per slot) and a list of named attributes with their values per id.
 *
 *  The layout was derived from the payloads of the owned packages (UE 5.3, 5.6 and 5.8), which describe themselves: every attribute
 *  carries its name, a type (0 vec4, 1 vec3, 2 vec2, 3 float, 4 int32, 5 bool, 6 name), the number of values per id and one block per
 *  attribute index (for example per UV set), then a default value and flags. All numbers are little endian:
 *
 *    u32 elementCount, then per element:  string name, u32 channelCount, then per channel:
 *      u32 slotCount, ceil( slotCount / 32 ) words of the allocation bits, u32 freeCount (only 0 is read), u32 slotCount again,
 *      u32 attributeCount, then per attribute:
 *        string name, u32 type, u32 values per id, u32 slotCount, u32 indexCount (blocks), per block: u32 values per id,
 *        u32 bytes per value (not for the type name), u32 valueCount, the values (strings for the type name), then the default value (a boolean as 32 bits) and u32 flags.
 *
 *  Strings are a u32 length (with the terminating zero) and the bytes. Sizes and counts are checked against the bytes that remain
 *  before anything is allocated; a payload that does not follow the layout is refused with the reason and the offset.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealMeshDescription
{
    public:

    enum EAttributeType
    {
        ATTRIBUTE_VECTOR4 = 0   ,
        ATTRIBUTE_VECTOR3       ,
        ATTRIBUTE_VECTOR2       ,
        ATTRIBUTE_FLOAT         ,
        ATTRIBUTE_INT32         ,
        ATTRIBUTE_BOOL          ,
        ATTRIBUTE_NAME
    };

    struct SAttribute
    {
        CORE::CString name;
        UInt32 type;
        UInt32 valuesPerId;                             /**< 3 for the corners of a triangle, 1 for most */
        UInt32 slotCount;
        UInt32 bytesPerValue;
        GUCEF::vector< GUCEF::vector< UInt8 > > blocks; /**< per attribute index (UV set), slotCount * valuesPerId * bytesPerValue bytes; empty for names */
        GUCEF::vector< GUCEF::vector< CORE::CString > > names;  /**< per block for the type name, one per id */

        SAttribute( void );

        /** The value (component) as a float of an id; index is the value of the id (0 to valuesPerId - 1) */
        Float32 GetFloat( UInt32 block, UInt32 id, UInt32 index, UInt32 component ) const;
        Int32 GetInt32( UInt32 block, UInt32 id, UInt32 index ) const;
    };

    struct SChannel
    {
        UInt32 slotCount;
        GUCEF::vector< UInt8 > allocated;               /**< one byte per slot, 1 when the id exists */
        GUCEF::vector< SAttribute > attributes;

        SChannel( void );
        const SAttribute* Find( const CORE::CString& name ) const;
        bool IsAllocated( UInt32 id ) const;
        UInt32 GetAllocatedCount( void ) const;
    };

    struct SElement
    {
        CORE::CString name;
        GUCEF::vector< SChannel > channels;
    };

    CUnrealMeshDescription( void );

    /** Reads the payload; false with GetError() when it does not follow the layout */
    bool Read( const UInt8* bytes, UInt64 size );

    const CORE::CString& GetError( void ) const;

    const SElement* FindElement( const CORE::CString& name ) const;

    const GUCEF::vector< SElement >& GetElements( void ) const;

    private:
    GUCEF::vector< SElement > m_elements;
    CORE::CString m_error;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMESHDESCRIPTION_H ? */
