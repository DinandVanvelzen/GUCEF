/*
 *  assetpluginUNREAL: Decoders for the structs that are stored in binary form inside tagged properties
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H
#include "assetpluginUNREAL_CUnrealNativeStructs.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

enum ENativeKind
{
    NK_VEC2, NK_VEC3, NK_VEC4, NK_ROTATOR, NK_QUAT, NK_PLANE, NK_MATRIX, NK_TRANSFORM, NK_BOX, NK_BOX2D,
    NK_LINEARCOLOR, NK_COLOR, NK_INTPOINT, NK_INTVECTOR, NK_INTVECTOR4, NK_GUID, NK_DATETIME, NK_TIMESPAN,
    NK_FRAMENUMBER, NK_SOFTPATH, NK_PERPLATFORM_FLOAT, NK_PERPLATFORM_INT, NK_PERPLATFORM_BOOL, NK_PERPLATFORM_FRAMERATE,
    NK_PERQUALITY_FLOAT, NK_PERQUALITY_INT
};

/** width: 0 depends on the version, 1 always float, 2 always double */
struct SNativeDef
{
    const char* name;
    ENativeKind kind;
    int width;
};

static const SNativeDef g_nativeDefs[] =
{
    { "Vector2D", NK_VEC2, 0 }, { "Vector2f", NK_VEC2, 1 }, { "Vector2d", NK_VEC2, 2 },
    { "Vector", NK_VEC3, 0 }, { "Vector3f", NK_VEC3, 1 }, { "Vector3d", NK_VEC3, 2 },
    { "Vector4", NK_VEC4, 0 }, { "Vector4f", NK_VEC4, 1 }, { "Vector4d", NK_VEC4, 2 },
    { "Rotator", NK_ROTATOR, 0 }, { "Rotator3f", NK_ROTATOR, 1 }, { "Rotator3d", NK_ROTATOR, 2 },
    { "Quat", NK_QUAT, 0 }, { "Quat4f", NK_QUAT, 1 }, { "Quat4d", NK_QUAT, 2 },
    { "Plane", NK_PLANE, 0 }, { "Plane4f", NK_PLANE, 1 }, { "Plane4d", NK_PLANE, 2 },
    { "Matrix", NK_MATRIX, 0 }, { "Matrix44f", NK_MATRIX, 1 }, { "Matrix44d", NK_MATRIX, 2 },
    { "Transform", NK_TRANSFORM, 0 }, { "Transform3f", NK_TRANSFORM, 1 }, { "Transform3d", NK_TRANSFORM, 2 },
    { "Box", NK_BOX, 0 }, { "Box3f", NK_BOX, 1 }, { "Box3d", NK_BOX, 2 },
    { "Box2D", NK_BOX2D, 0 }, { "Box2f", NK_BOX2D, 1 }, { "Box2d", NK_BOX2D, 2 },
    { "LinearColor", NK_LINEARCOLOR, 1 }, { "Color", NK_COLOR, 1 },
    { "IntPoint", NK_INTPOINT, 1 }, { "Int32Point", NK_INTPOINT, 1 }, { "IntVector", NK_INTVECTOR, 1 }, { "Int32Vector", NK_INTVECTOR, 1 },
    { "IntVector4", NK_INTVECTOR4, 1 }, { "Int32Vector4", NK_INTVECTOR4, 1 },
    { "Guid", NK_GUID, 1 }, { "DateTime", NK_DATETIME, 1 }, { "Timespan", NK_TIMESPAN, 1 },
    { "FrameNumber", NK_FRAMENUMBER, 1 }, { "SoftObjectPath", NK_SOFTPATH, 1 }, { "SoftClassPath", NK_SOFTPATH, 1 },
    { "PerPlatformFloat", NK_PERPLATFORM_FLOAT, 1 }, { "PerPlatformInt", NK_PERPLATFORM_INT, 1 }, { "PerPlatformBool", NK_PERPLATFORM_BOOL, 1 },
    { "PerPlatformFrameRate", NK_PERPLATFORM_FRAMERATE, 1 }, { "PerQualityLevelFloat", NK_PERQUALITY_FLOAT, 1 }, { "PerQualityLevelInt", NK_PERQUALITY_INT, 1 },
    { GUCEF_NULL, NK_VEC2, 0 }
};

/*-------------------------------------------------------------------------*/

static const SNativeDef*
FindDef( const CORE::CString& structName )
{
    for ( const SNativeDef* def = g_nativeDefs; GUCEF_NULL != def->name; ++def )
        if ( structName == def->name )
            return def;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static void
SetNumber( CORE::CDataNode& node, const char* name, Float64 value )
{
    node.SetAttribute( CORE::CString( name ), CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

static Float64
ReadReal( CUnrealArchiveReader& reader, bool wide )
{
    return wide ? reader.ReadFloat64() : (Float64) reader.ReadFloat32();
}

/*-------------------------------------------------------------------------*/

static void
ReadComponents( CUnrealArchiveReader& reader, bool wide, const char* const* labels, UInt32 count, CORE::CDataNode& node )
{
    for ( UInt32 i=0; i<count; ++i )
        SetNumber( node, labels[ i ], ReadReal( reader, wide ) );
}

/*-------------------------------------------------------------------------*/

static void
ReadVec3Node( CUnrealArchiveReader& reader, bool wide, CORE::CDataNode& node )
{
    static const char* const labels[] = { "x", "y", "z" };
    ReadComponents( reader, wide, labels, 3, node );
}

/*-------------------------------------------------------------------------*/

/**
 *  The per platform and per quality level values: a boolean that says the value is cooked, the default value, and in editor
 *  packages the table of the values that differ (a count and entries of a key and a value). The key is a name for platforms
 *  and a 32 bit integer for quality levels.
 */
static bool
ReadPerValue( CUnrealArchiveReader& reader, ENativeKind kind, const CUnrealNameTable& names, CORE::CDataNode& node )
{
    const bool cooked = 0 != reader.ReadUInt32();
    node.SetAttribute( CORE::CString( "cooked" ), CORE::CVariant( cooked ) );
    const bool real = NK_PERPLATFORM_FLOAT == kind || NK_PERQUALITY_FLOAT == kind;
    const bool frameRate = NK_PERPLATFORM_FRAMERATE == kind;
    const bool qualityKey = NK_PERQUALITY_FLOAT == kind || NK_PERQUALITY_INT == kind;
    if ( frameRate )
    {
        node.SetAttribute( CORE::CString( "numerator" ), CORE::CVariant( reader.ReadInt32() ) );
        node.SetAttribute( CORE::CString( "denominator" ), CORE::CVariant( reader.ReadInt32() ) );
    }
    else
    if ( real )
        node.SetAttribute( CORE::CString( "default" ), CORE::CVariant( reader.ReadFloat32() ) );
    else
    if ( NK_PERPLATFORM_BOOL == kind )
        node.SetAttribute( CORE::CString( "default" ), CORE::CVariant( 0 != reader.ReadUInt32() ) );
    else
        node.SetAttribute( CORE::CString( "default" ), CORE::CVariant( reader.ReadInt32() ) );
    if ( cooked )
        return !reader.HasError();
    const Int32 count = reader.ReadInt32();
    if ( reader.HasError() || count < 0 || !reader.CheckCount( (UInt64) count, 8, "per platform values" ) )
        return reader.SetError( "A per platform value with a table size that is not valid" );
    if ( count > 0 )
    {
        CORE::CDataNode* table = node.AddChild( "values", GUCEF_DATATYPE_OBJECT );
        for ( Int32 i=0; i<count; ++i )
        {
            CORE::CString key;
            if ( qualityKey )
                key = CORE::Int32ToString( reader.ReadInt32() );
            else
                names.ReadName( reader, key );
            if ( frameRate )
            {
                const Int32 numerator = reader.ReadInt32();
                const Int32 denominator = reader.ReadInt32();
                table->SetAttribute( key, CORE::CVariant( CORE::Int32ToString( numerator ) + "/" + CORE::Int32ToString( denominator ) ) );
            }
            else
            if ( real )
                table->SetAttribute( key, CORE::CVariant( reader.ReadFloat32() ) );
            else
            if ( NK_PERPLATFORM_BOOL == kind )
                table->SetAttribute( key, CORE::CVariant( 0 != reader.ReadUInt32() ) );
            else
                table->SetAttribute( key, CORE::CVariant( reader.ReadInt32() ) );
        }
    }
    return !reader.HasError();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealNativeStructs::IsNative( const CORE::CString& structName )
{GUCEF_TRACE;

    return GUCEF_NULL != FindDef( structName );
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealNativeStructs::GetFixedSize( const CORE::CString& structName, const CUnrealVersionContext& versions )
{GUCEF_TRACE;

    const SNativeDef* def = FindDef( structName );
    if ( GUCEF_NULL == def )
        return 0;
    const UInt32 real = ( 2 == def->width || ( 0 == def->width && versions.IsAtLeast( UE5_LARGE_WORLD_COORDINATES ) ) ) ? 8 : 4;
    switch ( def->kind )
    {
        case NK_VEC2: return 2 * real;
        case NK_VEC3: case NK_ROTATOR: return 3 * real;
        case NK_VEC4: case NK_QUAT: case NK_PLANE: return 4 * real;
        case NK_MATRIX: return 16 * real;
        case NK_TRANSFORM: return 10 * real;
        case NK_BOX: return 6 * real + 1;
        case NK_BOX2D: return 4 * real + 1;
        case NK_LINEARCOLOR: return 16;
        case NK_COLOR: return 4;
        case NK_INTPOINT: return 8;
        case NK_INTVECTOR: return 12;
        case NK_INTVECTOR4: return 16;
        case NK_GUID: return 16;
        case NK_DATETIME: case NK_TIMESPAN: return 8;
        case NK_FRAMENUMBER: return 4;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnrealNativeStructs::Read( CUnrealArchiveReader& reader, const CORE::CString& structName, const CUnrealNameTable& names, const CUnrealVersionContext& versions, const CUnrealSoftObjectPath::TSoftObjectPathVector* softPaths, Int64 declaredSize, CORE::CDataNode& node )
{GUCEF_TRACE;

    const SNativeDef* def = FindDef( structName );
    if ( GUCEF_NULL == def )
        return reader.SetError( "Struct " + structName + " is not a known binary struct" );
    node.SetAttribute( CORE::CString( "struct" ), CORE::CVariant( structName ) );

    // the width of the vector family
    bool wide = versions.IsAtLeast( UE5_LARGE_WORLD_COORDINATES );
    if ( 1 == def->width ) wide = false;
    if ( 2 == def->width ) wide = true;
    if ( 0 == def->width && declaredSize >= 0 )
    {
        UInt32 components = 0;
        switch ( def->kind )
        {
            case NK_VEC2: components = 2; break;
            case NK_VEC3: case NK_ROTATOR: components = 3; break;
            case NK_VEC4: case NK_QUAT: case NK_PLANE: components = 4; break;
            case NK_MATRIX: components = 16; break;
            case NK_TRANSFORM: components = 10; break;
            case NK_BOX: components = 6; break;
            case NK_BOX2D: components = 4; break;
            default: break;
        }
        const Int64 extra = ( NK_BOX == def->kind || NK_BOX2D == def->kind ) ? 1 : 0;
        if ( components > 0 )
        {
            if ( declaredSize == (Int64) components * 8 + extra ) wide = true;
            else
            if ( declaredSize == (Int64) components * 4 + extra ) wide = false;
        }
    }

    switch ( def->kind )
    {
        case NK_VEC2:
        {
            static const char* const labels[] = { "x", "y" };
            ReadComponents( reader, wide, labels, 2, node );
            break;
        }
        case NK_VEC3: ReadVec3Node( reader, wide, node ); break;
        case NK_VEC4: case NK_QUAT: case NK_PLANE:
        {
            static const char* const xyzw[] = { "x", "y", "z", "w" };
            ReadComponents( reader, wide, xyzw, 4, node );
            break;
        }
        case NK_ROTATOR:
        {
            static const char* const labels[] = { "pitch", "yaw", "roll" };
            ReadComponents( reader, wide, labels, 3, node );
            break;
        }
        case NK_MATRIX:
        {
            for ( UInt32 row=0; row<4; ++row )
            {
                for ( UInt32 column=0; column<4; ++column )
                {
                    char label[ 8 ];
                    label[ 0 ] = 'm'; label[ 1 ] = (char) ( '0' + row ); label[ 2 ] = (char) ( '0' + column ); label[ 3 ] = 0;
                    SetNumber( node, label, ReadReal( reader, wide ) );
                }
            }
            break;
        }
        case NK_TRANSFORM:
        {
            static const char* const xyzw[] = { "x", "y", "z", "w" };
            ReadComponents( reader, wide, xyzw, 4, *node.AddChild( "rotation", GUCEF_DATATYPE_OBJECT ) );
            ReadVec3Node( reader, wide, *node.AddChild( "translation", GUCEF_DATATYPE_OBJECT ) );
            ReadVec3Node( reader, wide, *node.AddChild( "scale3D", GUCEF_DATATYPE_OBJECT ) );
            break;
        }
        case NK_BOX:
            ReadVec3Node( reader, wide, *node.AddChild( "min", GUCEF_DATATYPE_OBJECT ) );
            ReadVec3Node( reader, wide, *node.AddChild( "max", GUCEF_DATATYPE_OBJECT ) );
            node.SetAttribute( CORE::CString( "valid" ), CORE::CVariant( 0 != reader.ReadUInt8() ) );
            break;
        case NK_BOX2D:
        {
            static const char* const labels[] = { "x", "y" };
            ReadComponents( reader, wide, labels, 2, *node.AddChild( "min", GUCEF_DATATYPE_OBJECT ) );
            ReadComponents( reader, wide, labels, 2, *node.AddChild( "max", GUCEF_DATATYPE_OBJECT ) );
            node.SetAttribute( CORE::CString( "valid" ), CORE::CVariant( 0 != reader.ReadUInt8() ) );
            break;
        }
        case NK_LINEARCOLOR:
        {
            static const char* const labels[] = { "r", "g", "b", "a" };
            ReadComponents( reader, false, labels, 4, node );
            break;
        }
        case NK_COLOR:
        {
            // stored as blue, green, red, alpha
            const UInt8 b = reader.ReadUInt8();
            const UInt8 g = reader.ReadUInt8();
            const UInt8 r = reader.ReadUInt8();
            const UInt8 a = reader.ReadUInt8();
            SetNumber( node, "r", r ); SetNumber( node, "g", g ); SetNumber( node, "b", b ); SetNumber( node, "a", a );
            break;
        }
        case NK_INTPOINT:
            SetNumber( node, "x", reader.ReadInt32() ); SetNumber( node, "y", reader.ReadInt32() );
            break;
        case NK_INTVECTOR:
            SetNumber( node, "x", reader.ReadInt32() ); SetNumber( node, "y", reader.ReadInt32() ); SetNumber( node, "z", reader.ReadInt32() );
            break;
        case NK_INTVECTOR4:
            SetNumber( node, "x", reader.ReadInt32() ); SetNumber( node, "y", reader.ReadInt32() ); SetNumber( node, "z", reader.ReadInt32() ); SetNumber( node, "w", reader.ReadInt32() );
            break;
        case NK_GUID:
        {
            CUnrealGuid guid;
            reader.ReadGuid( guid );
            node.SetValue( CORE::CVariant( guid.ToString() ) );
            break;
        }
        case NK_DATETIME: case NK_TIMESPAN:
            node.SetValue( CORE::CVariant( reader.ReadInt64() ) );
            break;
        case NK_FRAMENUMBER:
            node.SetValue( CORE::CVariant( reader.ReadInt32() ) );
            break;
        case NK_PERPLATFORM_FLOAT: case NK_PERPLATFORM_INT: case NK_PERPLATFORM_BOOL: case NK_PERPLATFORM_FRAMERATE: case NK_PERQUALITY_FLOAT: case NK_PERQUALITY_INT:
            ReadPerValue( reader, def->kind, names, node );
            break;
        case NK_SOFTPATH:
        {
            CUnrealSoftObjectPath path;
            path.Read( reader, names, versions, softPaths );
            node.SetValue( CORE::CVariant( path.ToString() ) );
            break;
        }
    }
    return !reader.HasError();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
