/*
 *  assetpluginGODOT: the reader of the Godot text formats
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTTEXTREADER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTTEXTREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#include "assetpluginGODOT_macros.h"
#define GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the text formats of Godot 4 into a CDataNode tree: scene files (.tscn), resource files (.tres), the import
 *  settings of a file (.import) and the project file (project.godot). All of them are sections of key = value lines
 *  with variant literals as values.
 *
 *  The file:
 *  - the root node "godot" with the attribute "kind", the tag of the first section ("gd_scene", "gd_resource" for the
 *    scenes and resources; "" for files without such a tag);
 *  - one child "section" per section, in file order, with the attribute "tag" ("gd_scene", "ext_resource",
 *    "sub_resource", "node", "connection", "editable", "resource", "remap", "params", ...). The values of the section
 *    header are attributes of the section when they are scalars (strings, numbers, booleans; a reference
 *    ExtResource("x") is the attribute with the text of the id), and all of them are values under the child "header".
 *    Lines before the first section belong to a section with the tag "";
 *  - the lines of a section are the children of its child "properties", named by the key.
 *
 *  A value is a node with the attribute "t", the type of the literal, and:
 *  - "null", "bool", "int", "float", "String", "StringName", "NodePath", "ExtResource", "SubResource": the value of the
 *    node (Int64 and Float64 for the numbers; the text for strings, paths and the id of a reference);
 *  - "Vector2", "Vector3", "Vector4", the "i" variants, "Color", "Quaternion", "Plane", "Rect2", "Rect2i", "AABB",
 *    "Transform2D", "Basis", "Transform3D", "Projection" and every other call syntax: the arguments as children
 *    without a name, each a value (numbers for the geometric types), in the order of the file;
 *  - "Array": the elements as children without a name; the attribute "elementType" for a typed array;
 *  - "Dictionary": a child per entry, named by the key as text, with the attribute "key" the key as text and "keyType"
 *    its type; the child is the value of the entry;
 *  - "Object": the attribute "class" and a child per property;
 *  - "PackedByteArray", "PackedInt32Array", "PackedInt64Array", "PackedFloat32Array", "PackedFloat64Array",
 *    "PackedVector2Array", "PackedVector3Array", "PackedVector4Array", "PackedColorArray": the elements as a binary
 *    value of the node in the element type of the array (byte order of the host), and the attributes "count" (the number of
 *    elements), "components" (the numbers per element) and "elem" ("u8", "i32", "i64", "f32", "f64"); a PackedByteArray written as text in base64 gives the
 *    same; "PackedStringArray": the strings as children.
 *  CGodotVariant reads these.
 *
 *  Comments (';' to the end of the line) and line breaks inside values are accepted. A reader instance is not shared
 *  between threads.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotTextReader
{
    public:

    CGodotTextReader( void );

    ~CGodotTextReader();

    /** Parses the text of a file; false with the problem (and its line) for text that is not valid */
    bool Parse( const CORE::CString& text, CORE::CDataNode& result );

    /** Reads a file of the VFS and parses it */
    bool ParseFile( const CORE::CString& vfsPath, CORE::CDataNode& result );

    /** Parses a single variant literal into a value node */
    bool ParseValue( const CORE::CString& text, CORE::CDataNode& value );

    /** What went wrong in the last call, with the line */
    const CORE::CString& GetProblem( void ) const;

    /** Parses the bytes of a text file */
    bool ParseData( const char* data, UInt32 size, CORE::CDataNode& result );

    private:

    CGodotTextReader( const CGodotTextReader& src );            /**< not implemented */
    CGodotTextReader& operator=( const CGodotTextReader& src ); /**< not implemented */

    void Start( const char* data, UInt32 size );
    bool Fail( const CORE::CString& message );
    bool AtEnd( void ) const;
    char Peek( UInt32 ahead = 0 ) const;
    void SkipSpace( void );
    bool IsIdentifierChar( char c ) const;
    CORE::CString ReadIdentifier( void );
    bool ParseSection( CORE::CDataNode& root, CORE::CDataNode*& current, CORE::CDataNode*& properties );
    bool ParseAssignment( CORE::CDataNode& container, bool isHeader, CORE::CDataNode* section );
    bool ParseKey( CORE::CString& key );
    bool ParseVariant( CORE::CDataNode& node );
    bool ParseString( CORE::CString& text );
    bool ParseNumber( CORE::CDataNode& node );
    bool ParseNumberValue( Float64& number, bool& isInteger, Int64& integer );
    bool ParseArray( CORE::CDataNode& node );
    bool ParseDictionary( CORE::CDataNode& node );
    bool ParseIdentifierValue( CORE::CDataNode& node );
    bool ParseCall( const CORE::CString& name, CORE::CDataNode& node );
    bool ParsePackedCall( const CORE::CString& name, CORE::CDataNode& node );
    bool ParseObject( CORE::CDataNode& node );
    bool ParseTypedContainer( const CORE::CString& name, CORE::CDataNode& node );
    bool ParseTypeSpec( CORE::CString& spec );
    bool Expect( char c );

    static bool DecodeBase64( const CORE::CString& text, GUCEF::vector< UInt8 >& bytes );
    static void SetPacked( CORE::CDataNode& node, const CORE::CString& name, const GUCEF::vector< Float64 >& numbers );
    static void SetScalarAttribute( CORE::CDataNode& section, const CORE::CString& key, const CORE::CDataNode& value );

    private:

    const char* m_data;
    UInt32 m_size;
    UInt32 m_pos;
    UInt32 m_line;
    CORE::CString m_problem;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTTEXTREADER_H ? */
