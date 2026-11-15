/*
 *  assetpluginGODOT: the values of the Godot text formats
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MATH_H
#include "gucefMATH.h"
#define GUCEF_MATH_H
#endif /* GUCEF_MATH_H ? */

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
 *  Reads the value nodes that CGodotTextReader makes: the type of a literal, strings, numbers and booleans, the numbers
 *  of the geometric types and the packed arrays, references to resources, the properties of a section and the entries
 *  of a dictionary. Every function takes a node that is NULL or of another type without harm and gives the fallback.
 *  The conversions of Godot's geometric types to the model (colours, transforms) are here as well.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotVariant
{
    public:

    typedef GUCEF::vector< Float64 > TNumberVector;
    typedef GUCEF::vector< UInt8 > TByteVector;

    /** The type of the literal: "String", "Vector3", "PackedByteArray", ...; empty for NULL */
    static CORE::CString GetType( const CORE::CDataNode* value );

    static bool IsType( const CORE::CDataNode* value, const CORE::CString& type );

    /** The text of a String, StringName, NodePath or the id of a reference; the fallback for other types */
    static CORE::CString GetString( const CORE::CDataNode* value, const CORE::CString& fallback = CORE::CString::Empty );

    /** The number of an int, float or bool */
    static Float64 GetNumber( const CORE::CDataNode* value, Float64 fallback );

    static Int64 GetInteger( const CORE::CDataNode* value, Int64 fallback );

    static bool GetBool( const CORE::CDataNode* value, bool fallback );

    /** Whether the value is an int, float, bool or text (the types a scalar attribute holds) */
    static bool IsScalar( const CORE::CDataNode* value );

    /**
     *  The numbers of a geometric literal (Vector3, Color, Transform3D, ...), of an Array of numbers and of the packed
     *  numeric arrays, flattened in the order of the file. False for other values.
     */
    static bool GetNumbers( const CORE::CDataNode* value, TNumberVector& numbers );

    /** The bytes of a PackedByteArray */
    static bool GetBytes( const CORE::CDataNode* value, TByteVector& bytes );

    /** The strings of a PackedStringArray or of an Array of strings */
    static bool GetStrings( const CORE::CDataNode* value, CORE::CString::StringVector& strings );

    /** The reference of an ExtResource or SubResource: its kind and id */
    static bool GetReference( const CORE::CDataNode* value, CORE::CString& kind, CORE::CString& id );

    /** The property of a section by its key (a child of "properties"); NULL when there is none */
    static const CORE::CDataNode* FindProperty( const CORE::CDataNode& section, const CORE::CString& key );

    /** The entry of a Dictionary or the property of an Object by its key; NULL when there is none */
    static const CORE::CDataNode* FindMember( const CORE::CDataNode* container, const CORE::CString& key );

    /** A Vector3 (or any literal with three or more numbers) */
    static bool GetVector3( const CORE::CDataNode* value, Float32 out[ 3 ] );

    /** A Vector2 */
    static bool GetVector2( const CORE::CDataNode* value, Float32 out[ 2 ] );

    /** A Color: red, green, blue and alpha (alpha 1 when the literal has three numbers) */
    static bool GetColour( const CORE::CDataNode* value, Float32 out[ 4 ] );

    /**
     *  A Transform3D (the nine numbers of the basis by rows and the origin) or a Basis (rotation and scale only) as
     *  translation, rotation and scale. False when the numbers are not there or the basis cannot be split into them.
     */
    static bool GetTransform( const CORE::CDataNode* value, MATH::TFloat32Transform3D& transform );

    /** The colour as linear values from the sRGB values Godot writes for a colour with the source colour hint */
    static Float32 SrgbToLinear( Float32 value );

    /**
     *  Adds the value to the object node as a plain data tree for the sidecar: a scalar is an attribute, the numbers of
     *  a geometric literal or a packed array (up to 'maxElements', else only its count) and the elements of an Array
     *  are an array child, a Dictionary an object child. A reference is its text, "ExtResource(id)"; a null value is left out.
     */
    static void AddPlain( CORE::CDataNode& object, const CORE::CString& key, const CORE::CDataNode& value, UInt32 maxElements = 256 );

    private:

    CGodotVariant( void );
    ~CGodotVariant();
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

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */
