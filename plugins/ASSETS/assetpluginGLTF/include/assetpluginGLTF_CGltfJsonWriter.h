/*
 *  assetpluginGLTF: the deterministic JSON writer of the glTF exporter
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_MACROS_H
#include "assetpluginGLTF_macros.h"
#define GUCEF_ASSETPLUGIN_GLTF_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_MACROS_H ? */

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
 *  Writes the JSON text of a glTF file, member by member, in the order the caller gives. The text is deterministic:
 *  two spaces per level, one member per line, arrays of numbers on one line, numbers in the shortest form that reads
 *  back as the same value (CCanonicalJson), LF line ends. Arrays of objects hold one element per line.
 *
 *  The caller keeps the structure right: a Key() before every value inside an object, balanced Begin and End calls.
 *  The commas and the indentation are the writer's.
 */
class GUCEF_ASSETPLUGIN_GLTF_EXPORT_CPP CGltfJsonWriter
{
    public:

    CGltfJsonWriter( void );

    ~CGltfJsonWriter();

    void BeginObject( void );
    void EndObject( void );
    void BeginArray( void );
    void EndArray( void );

    /** The name of the next member of the object that is open */
    void Key( const char* name );

    void String( const CORE::CString& value );
    void Float( Float32 value );
    void Int( Int64 value );
    void Bool( bool value );

    /** A value that is JSON text already, for the data of the extensions and extras: the tree as canonical JSON */
    void Node( const CORE::CDataNode& tree );

    /** An array of numbers on one line */
    void FloatArray( const Float32* values, UInt32 count );
    void IntArray( const UInt32* values, UInt32 count );

    /** Members: the key and the value in one call */
    void Member( const char* name, const CORE::CString& value );
    void Member( const char* name, const char* value );
    void Member( const char* name, Float32 value );
    void Member( const char* name, Int64 value );
    void Member( const char* name, Int32 value );
    void Member( const char* name, UInt32 value );
    void Member( const char* name, bool value );
    void MemberFloatArray( const char* name, const Float32* values, UInt32 count );
    void MemberIntArray( const char* name, const UInt32* values, UInt32 count );
    void MemberNode( const char* name, const CORE::CDataNode& tree );

    /** Ends the text with a line break, as a file has it */
    void EndDocument( void );

    /** The JSON text so far */
    const CORE::CDynamicBuffer& GetText( void ) const;

    private:

    CGltfJsonWriter( const CGltfJsonWriter& src );              /**< not implemented */
    CGltfJsonWriter& operator=( const CGltfJsonWriter& src );   /**< not implemented */

    /** The comma and line break before a value or a key, and the indentation */
    void BeginItem( void );

    void Append( const char* text );
    void Append( const CORE::CString& text );

    private:

    struct SLevel
    {
        bool isObject;
        UInt32 count;
    };
    typedef GUCEF::vector< SLevel > TLevelVector;

    CORE::CDynamicBuffer m_text;
    TLevelVector m_levels;
    bool m_afterKey;
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

#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFJSONWRITER_H ? */
