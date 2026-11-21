/*
 *  assetpluginUNREAL_TestApp: Tests of the tagged property reader of the Unreal reader
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

#ifndef GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#include "assetpluginUNREAL_CUnrealObjectProperties.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#ifndef GUCEF_CORE_TESTUNREALWRITER_H
#include "TestUnrealWriter.h"
#define GUCEF_CORE_TESTUNREALWRITER_H
#endif /* GUCEF_CORE_TESTUNREALWRITER_H ? */

#include "TestUnrealProperties.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define VCHECK(c)       if ( !( c ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Verify failed: " << #c ); ok = false; }

/*-------------------------------------------------------------------------*/

typedef void (*TFillFunction)( CTestPackageWriter& package, CTestPropertyWriter& props, CTestByteWriter& body );

static CTestType T( const char* name ) { return CTestType( name ); }
static CTestType T( const char* name, const CTestType& a ) { return CTestType( name, a ); }
static CTestType T( const char* name, const CTestType& a, const CTestType& b ) { return CTestType( name, a, b ); }

/*-------------------------------------------------------------------------*/

/** A package with one export whose body is made by the function, read back property by property */
class CPropertyFixture
{
    public:

    CTestPackageWriter writer;
    CUnrealPackage package;
    CORE::CDataNode root;
    CORE::CString::StringVector mismatches;
    CORE::CString::StringVector unknown;
    bool readOk;
    UInt32 propertyCount;

    CPropertyFixture( Int32 legacy, Int32 ue4, Int32 ue5 )
        : writer( legacy, ue4, ue5 )
        , package()
        , root( "root" )
        , readOk( false )
        , propertyCount( 0 )
    {
    }

    /** Builds, opens and reads; the limits apply to the package and the reader */
    bool Run( TFillFunction fill, const CUnrealReaderLimits& limits = CUnrealReaderLimits() )
    {
        // two imports for object references, one soft path for the list form
        const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
        writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "StaticMesh" );
        CTestByteWriter body;
        writer.BeginBody( body );
        CTestPropertyWriter props( body, writer.names, writer.ue4, writer.ue5 );
        fill( writer, props, body );
        props.None();
        writer.AddExport( -2, 0, "Obj", body );
        GUCEF::vector< UInt8 > bytes;
        writer.Build( bytes );
        if ( !package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "props", limits ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Properties test package: " << package.GetError() );
            return false;
        }
        CUnrealArchiveReader reader;
        if ( !package.OpenPropertyRange( 0, reader ) )
            return false;
        CUnrealPropertyReader propertyReader( package );
        readOk = propertyReader.ReadExport( reader, root, "StaticMesh" );
        mismatches = propertyReader.GetMismatches();
        unknown = propertyReader.GetUnknownProperties();
        propertyCount = propertyReader.GetPropertyCount();
        return readOk;
    }
};

/*-------------------------------------------------------------------------*/

static void
WriteVector( CTestByteWriter& b, bool wide, double x, double y, double z )
{
    if ( wide ) { b.F64( x ); b.F64( y ); b.F64( z ); }
    else { b.F32( (Float32) x ); b.F32( (Float32) y ); b.F32( (Float32) z ); }
}

/*-------------------------------------------------------------------------*/

static void
WriteSoftPathValue( CTestPackageWriter& package, CTestByteWriter& b )
{
    CUnrealSoftObjectPath path;
    path.packageName = "/Game/Soft/Package";
    path.assetName = "SoftAsset";
    path.subPath = "Sub:Path";
    if ( package.ue5 >= UE5_ADD_SOFTOBJECTPATH_LIST )
    {
        package.softPaths.push_back( path );
        b.I32( (Int32) package.softPaths.size() - 1 );
    }
    else
    if ( package.ue5 >= UE5_SOFTOBJECTPATH_REMOVE_ASSET_PATH_FNAMES )
    {
        package.names.WriteName( b, path.packageName );
        package.names.WriteName( b, path.assetName );
        b.Str( path.subPath );
    }
    else
    {
        package.names.WriteName( b, path.packageName + "." + path.assetName );
        b.Str( path.subPath );
    }
}

/*-------------------------------------------------------------------------*/

/** One property of every kind that can stand alone */
static void
WriteScalars( CTestPackageWriter& package, CTestPropertyWriter& p, CTestByteWriter& b )
{
    CTestPropertyWriter::SOpen o;
    p.Begin( "bTrue", T( "BoolProperty" ), true ); // no value bytes: the value is in the tag
    p.Begin( "bFalse", T( "BoolProperty" ), false );
    o = p.Begin( "Byte", T( "ByteProperty" ) ); b.U8( 200 ); p.End( o );
    o = p.Begin( "EnumByte", T( "ByteProperty", T( "ETest" ) ) ); package.names.WriteName( b, "ETest::Second" ); p.End( o );
    o = p.Begin( "Enum", T( "EnumProperty", T( "ETest" ), T( "ByteProperty" ) ) ); package.names.WriteName( b, "ETest::Third" ); p.End( o );
    o = p.Begin( "I8", T( "Int8Property" ) ); b.U8( 0xFE ); p.End( o );
    o = p.Begin( "I16", T( "Int16Property" ) ); b.U16( 0xFFFE ); p.End( o );
    o = p.Begin( "I32", T( "IntProperty" ) ); b.I32( -123456 ); p.End( o );
    o = p.Begin( "I64", T( "Int64Property" ) ); b.I64( -5000000000LL ); p.End( o );
    o = p.Begin( "U16", T( "UInt16Property" ) ); b.U16( 65000 ); p.End( o );
    o = p.Begin( "U32", T( "UInt32Property" ) ); b.U32( 4000000000U ); p.End( o );
    o = p.Begin( "U64", T( "UInt64Property" ) ); b.U64( 18000000000000000000ULL ); p.End( o );
    o = p.Begin( "F32", T( "FloatProperty" ) ); b.F32( 0.5f ); p.End( o );
    o = p.Begin( "F64", T( "DoubleProperty" ) ); b.F64( -1.25e10 ); p.End( o );
    o = p.Begin( "Str", T( "StrProperty" ) ); b.Str( "hello" ); p.End( o );
    o = p.Begin( "StrWide", T( "StrProperty" ) ); b.Str( "h\xC3\xA9llo \xE2\x82\xAC \xF0\x9F\x98\x80" ); p.End( o );
    o = p.Begin( "Name", T( "NameProperty" ) ); package.names.WriteName( b, "SomeName_4" ); p.End( o );
    o = p.Begin( "TextNone", T( "TextProperty" ) ); b.U32( 0 ); b.U8( 0xFF ); b.Bool32( true ); b.Str( "invariant" ); p.End( o );
    o = p.Begin( "TextBase", T( "TextProperty" ) ); b.U32( 2 ); b.U8( 0 ); b.Str( "NS" ); b.Str( "Key" ); b.Str( "Source text" ); p.End( o );
    o = p.Begin( "TextTable", T( "TextProperty" ) ); b.U32( 0 ); b.U8( 11 ); package.names.WriteName( b, "Table" ); b.Str( "TableKey" ); p.End( o );
    o = p.Begin( "ObjImport", T( "ObjectProperty" ) ); b.I32( -2 ); p.End( o );
    o = p.Begin( "ObjExport", T( "ObjectProperty" ) ); b.I32( 1 ); p.End( o );
    o = p.Begin( "ObjNull", T( "ObjectProperty" ) ); b.I32( 0 ); p.End( o );
    o = p.Begin( "Weak", T( "WeakObjectProperty" ) ); b.I32( -1 ); p.End( o );
    o = p.Begin( "Soft", T( "SoftObjectProperty" ) ); WriteSoftPathValue( package, b ); p.End( o );
    o = p.Begin( "SoftClass", T( "SoftClassProperty" ) ); WriteSoftPathValue( package, b ); p.End( o );
    o = p.Begin( "Lazy", T( "LazyObjectProperty" ) ); b.Guid( CUnrealGuid( 0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10 ) ); p.End( o );
    o = p.Begin( "Delegate", T( "DelegateProperty" ) ); b.I32( 1 ); package.names.WriteName( b, "OnThing" ); p.End( o );
    o = p.Begin( "Multicast", T( "MulticastInlineDelegateProperty" ) ); b.I32( 2 ); b.I32( 1 ); package.names.WriteName( b, "A" ); b.I32( -2 ); package.names.WriteName( b, "B" ); p.End( o );
}

/*-------------------------------------------------------------------------*/

static bool
VerifyScalars( const CORE::CDataNode& root )
{
    CUnrealObjectProperties v( root );
    bool ok = true;
    VCHECK( v.GetBool( "bTrue", false ) );
    VCHECK( !v.GetBool( "bFalse", true ) );
    VCHECK( "BoolProperty" == v.GetType( "bTrue" ) );
    VCHECK( 200 == v.GetInt32( "Byte" ) );
    VCHECK( "ETest::Second" == v.GetString( "EnumByte" ) );
    VCHECK( "ETest::Third" == v.GetString( "Enum" ) );
    VCHECK( -2 == v.GetInt32( "I8" ) );
    VCHECK( -2 == v.GetInt32( "I16" ) );
    VCHECK( -123456 == v.GetInt32( "I32" ) );
    VCHECK( -5000000000LL == v.GetInt64( "I64" ) );
    VCHECK( 65000 == v.GetInt32( "U16" ) );
    VCHECK( 4000000000LL == v.GetInt64( "U32" ) );
    VCHECK( 0.5 == v.GetDouble( "F32" ) );
    VCHECK( -1.25e10 == v.GetDouble( "F64" ) );
    VCHECK( 18000000000000000000ULL == root.FindChild( "U64" )->GetValue().AsUInt64() );
    VCHECK( "hello" == v.GetString( "Str" ) );
    VCHECK( "h\xC3\xA9llo \xE2\x82\xAC \xF0\x9F\x98\x80" == v.GetString( "StrWide" ) );
    VCHECK( "SomeName_4" == v.GetString( "Name" ) );
    VCHECK( "invariant" == v.GetString( "TextNone" ) );
    VCHECK( "Source text" == v.GetString( "TextBase" ) );
    VCHECK( "TableKey" == root.FindChild( "TextTable" )->GetAttributeValue( "key" ).AsString() );
    VCHECK( "Table" == root.FindChild( "TextTable" )->GetAttributeValue( "table" ).AsString() );
    VCHECK( "Key" == root.FindChild( "TextBase" )->GetAttributeValue( "key" ).AsString() );
    VCHECK( -2 == v.GetInt32( "ObjImport" ) );
    VCHECK( "/Script/Engine.StaticMesh" == v.GetString( "ObjImport" ) );
    VCHECK( "/Game/Test/Package.Obj" == v.GetString( "ObjExport" ) );
    VCHECK( 0 == v.GetInt32( "ObjNull", 5 ) && !v.Find( "ObjNull" )->GetAttribute( "path" ) );
    VCHECK( "/Script/Engine" == v.GetString( "Weak" ) );
    VCHECK( "/Game/Soft/Package.SoftAsset:Sub:Path" == v.GetString( "Soft" ) );
    VCHECK( "/Game/Soft/Package.SoftAsset:Sub:Path" == v.GetString( "SoftClass" ) );
    VCHECK( "01020304-0506-0708-090A-0B0C0D0E0F10" == v.GetString( "Lazy" ) );
    VCHECK( "OnThing" == root.FindChild( "Delegate" )->GetAttributeValue( "function" ).AsString() );
    VCHECK( 2 == v.GetCount( "Multicast" ) );
    const CORE::CDataNode* second = v.Find( "Multicast/1" );
    VCHECK( GUCEF_NULL != second );
    VCHECK( "B" == second->GetAttributeValue( "function" ).AsString() );
    VCHECK( -2 == second->GetValue().AsInt32() );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The binary structs: their width depends on the version for the vector family */
static void
WriteNativeStructs( CTestPackageWriter& package, CTestPropertyWriter& p, CTestByteWriter& b )
{
    const bool wide = package.ue5 >= UE5_LARGE_WORLD_COORDINATES;
    const bool flag = p.IsNewLayout();
    CTestPropertyWriter::SOpen o;
    o = p.Begin( "Loc", StructType( "Vector" ), false, 0, flag ); WriteVector( b, wide, 1.5, -2.5, 3.5 ); p.End( o );
    o = p.Begin( "Loc3f", StructType( "Vector3f" ), false, 0, flag ); b.F32( 4 ); b.F32( 5 ); b.F32( 6 ); p.End( o );
    o = p.Begin( "Loc3d", StructType( "Vector3d" ), false, 0, flag ); b.F64( 7 ); b.F64( 8 ); b.F64( 9 ); p.End( o );
    o = p.Begin( "Loc2", StructType( "Vector2D" ), false, 0, flag ); if ( wide ) { b.F64( 10 ); b.F64( 11 ); } else { b.F32( 10 ); b.F32( 11 ); } p.End( o );
    o = p.Begin( "Loc4", StructType( "Vector4" ), false, 0, flag ); if ( wide ) { b.F64( 1 ); b.F64( 2 ); b.F64( 3 ); b.F64( 4 ); } else { b.F32( 1 ); b.F32( 2 ); b.F32( 3 ); b.F32( 4 ); } p.End( o );
    o = p.Begin( "Rot", StructType( "Rotator" ), false, 0, flag ); WriteVector( b, wide, 10, 20, 30 ); p.End( o );
    o = p.Begin( "Quat", StructType( "Quat" ), false, 0, flag ); if ( wide ) { b.F64( 0 ); b.F64( 0 ); b.F64( 0.5 ); b.F64( 1 ); } else { b.F32( 0 ); b.F32( 0 ); b.F32( 0.5f ); b.F32( 1 ); } p.End( o );
    o = p.Begin( "Tint", StructType( "LinearColor" ), false, 0, flag ); b.F32( 0.25f ); b.F32( 0.5f ); b.F32( 0.75f ); b.F32( 1.0f ); p.End( o );
    o = p.Begin( "Col", StructType( "Color" ), false, 0, flag ); b.U8( 30 ); b.U8( 20 ); b.U8( 10 ); b.U8( 255 ); p.End( o );
    o = p.Begin( "Pt", StructType( "IntPoint" ), false, 0, flag ); b.I32( -3 ); b.I32( 4 ); p.End( o );
    o = p.Begin( "IV", StructType( "IntVector" ), false, 0, flag ); b.I32( 1 ); b.I32( 2 ); b.I32( 3 ); p.End( o );
    o = p.Begin( "Id", StructType( "Guid" ), false, 0, flag ); b.Guid( CUnrealGuid( 0xAABBCCDD, 0x11223344, 0x55667788, 0x99AABBCC ) ); p.End( o );
    o = p.Begin( "When", StructType( "DateTime" ), false, 0, flag ); b.I64( 638000000000000000LL ); p.End( o );
    o = p.Begin( "Frame", StructType( "FrameNumber" ), false, 0, flag ); b.I32( 24 ); p.End( o );
    o = p.Begin( "Xf", StructType( "Transform" ), false, 0, flag );
    if ( wide ) { b.F64( 0 ); b.F64( 0 ); b.F64( 0 ); b.F64( 1 ); } else { b.F32( 0 ); b.F32( 0 ); b.F32( 0 ); b.F32( 1 ); }
    WriteVector( b, wide, 100, 200, 300 ); WriteVector( b, wide, 1, 2, 3 ); p.End( o );
    o = p.Begin( "Bounds", StructType( "Box" ), false, 0, flag ); WriteVector( b, wide, -1, -2, -3 ); WriteVector( b, wide, 1, 2, 3 ); b.U8( 1 ); p.End( o );
    o = p.Begin( "Mat", StructType( "Matrix" ), false, 0, flag ); for ( UInt32 i=0; i<16; ++i ) { if ( wide ) b.F64( i ); else b.F32( (Float32) i ); } p.End( o );
    o = p.Begin( "SoftStruct", StructType( "SoftObjectPath" ), false, 0, flag ); WriteSoftPathValue( package, b ); p.End( o );
}

/*-------------------------------------------------------------------------*/

static bool
VerifyNativeStructs( const CORE::CDataNode& root )
{
    CUnrealObjectProperties v( root );
    bool ok = true;
    VCHECK( 1.5 == v.GetComponent( "Loc", "x" ) );
    VCHECK( -2.5 == v.GetComponent( "Loc", "y" ) );
    VCHECK( 3.5 == v.GetComponent( "Loc", "z" ) );
    VCHECK( v.GetType( "Loc" ).HasSubstr( "StructProperty(Vector" ) == 0 );
    VCHECK( 4 == v.GetComponent( "Loc3f", "x" ) );
    VCHECK( 6 == v.GetComponent( "Loc3f", "z" ) );
    VCHECK( 7 == v.GetComponent( "Loc3d", "x" ) );
    VCHECK( 9 == v.GetComponent( "Loc3d", "z" ) );
    VCHECK( 10 == v.GetComponent( "Loc2", "x" ) );
    VCHECK( 11 == v.GetComponent( "Loc2", "y" ) );
    VCHECK( 4 == v.GetComponent( "Loc4", "w" ) );
    VCHECK( 10 == v.GetComponent( "Rot", "pitch" ) );
    VCHECK( 20 == v.GetComponent( "Rot", "yaw" ) );
    VCHECK( 30 == v.GetComponent( "Rot", "roll" ) );
    VCHECK( 0.5 == v.GetComponent( "Quat", "z" ) );
    VCHECK( 1 == v.GetComponent( "Quat", "w" ) );
    VCHECK( 0.25 == v.GetComponent( "Tint", "r" ) );
    VCHECK( 1.0 == v.GetComponent( "Tint", "a" ) );
    VCHECK( 30 == v.GetComponent( "Col", "b" ) );
    VCHECK( 10 == v.GetComponent( "Col", "r" ) );
    VCHECK( 255 == v.GetComponent( "Col", "a" ) );
    VCHECK( -3 == v.GetComponent( "Pt", "x" ) );
    VCHECK( 4 == v.GetComponent( "Pt", "y" ) );
    VCHECK( 3 == v.GetComponent( "IV", "z" ) );
    VCHECK( "AABBCCDD-1122-3344-5566-778899AABBCC" == v.GetString( "Id" ) );
    VCHECK( 638000000000000000LL == v.GetInt64( "When" ) );
    VCHECK( 24 == v.GetInt32( "Frame" ) );
    VCHECK( 100 == v.GetComponent( "Xf/translation", "x" ) );
    VCHECK( 3 == v.GetComponent( "Xf/scale3D", "z" ) );
    VCHECK( 1 == v.GetComponent( "Xf/rotation", "w" ) );
    VCHECK( -3 == v.GetComponent( "Bounds/min", "z" ) );
    VCHECK( 3 == v.GetComponent( "Bounds/max", "z" ) );
    VCHECK( v.Find( "Bounds" )->GetAttributeValue( "valid" ).AsBool() );
    VCHECK( 15 == v.GetComponent( "Mat", "m33" ) );
    VCHECK( 7 == v.GetComponent( "Mat", "m13" ) );
    VCHECK( "/Game/Soft/Package.SoftAsset:Sub:Path" == v.GetString( "SoftStruct" ) );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A tagged struct with a nested struct, containers of every shape */
static void
WriteContainers( CTestPackageWriter& package, CTestPropertyWriter& p, CTestByteWriter& b )
{
    const bool flag = p.IsNewLayout();
    const bool wide = package.ue5 >= UE5_LARGE_WORLD_COORDINATES;
    CTestPropertyWriter::SOpen o;
    CTestPropertyWriter::SOpen inner;

    // a struct that is read as a list of tags, with a struct in it
    o = p.Begin( "Settings", StructType( "MySettings" ) );
    {
        CTestPropertyWriter::SOpen a = p.Begin( "Level", T( "IntProperty" ) ); b.I32( 7 ); p.End( a );
        CTestPropertyWriter::SOpen n = p.Begin( "Nested", StructType( "MyNested" ) );
        CTestPropertyWriter::SOpen f = p.Begin( "Gain", T( "FloatProperty" ) ); b.F32( 2.5f ); p.End( f );
        p.Begin( "bOn", T( "BoolProperty" ), true );
        p.None();
        p.End( n );
        CTestPropertyWriter::SOpen s = p.Begin( "Label", T( "StrProperty" ) ); b.Str( "inner label" ); p.End( s );
        p.None();
    }
    p.End( o );

    o = p.Begin( "Ints", T( "ArrayProperty", T( "IntProperty" ) ) ); b.I32( 3 ); b.I32( 10 ); b.I32( 20 ); b.I32( 30 ); p.End( o );
    o = p.Begin( "Empty", T( "ArrayProperty", T( "FloatProperty" ) ) ); b.I32( 0 ); p.End( o );
    o = p.Begin( "Bytes", T( "ArrayProperty", T( "ByteProperty" ) ) ); b.I32( 3 ); b.U8( 1 ); b.U8( 2 ); b.U8( 3 ); p.End( o );
    o = p.Begin( "Names", T( "ArrayProperty", T( "NameProperty" ) ) ); b.I32( 2 ); package.names.WriteName( b, "Alpha" ); package.names.WriteName( b, "Beta_9" ); p.End( o );
    o = p.Begin( "Strings", T( "ArrayProperty", T( "StrProperty" ) ) ); b.I32( 2 ); b.Str( "one" ); b.Str( "two" ); p.End( o );
    o = p.Begin( "Flags", T( "ArrayProperty", T( "BoolProperty" ) ) ); b.I32( 3 ); b.U8( 1 ); b.U8( 0 ); b.U8( 1 ); p.End( o );
    o = p.Begin( "Refs", T( "ArrayProperty", T( "ObjectProperty" ) ) ); b.I32( 2 ); b.I32( -2 ); b.I32( 1 ); p.End( o );

    // arrays of structs: in the classic layout one tag names the struct
    o = p.Begin( "Points", T( "ArrayProperty", StructType( "Vector" ) ), false, 0, flag );
    b.I32( 2 );
    if ( !p.IsNewLayout() ) inner = p.BeginInnerStructTag( "Points", "Vector" );
    WriteVector( b, wide, 1, 2, 3 ); WriteVector( b, wide, 4, 5, 6 );
    if ( !p.IsNewLayout() ) p.End( inner );
    p.End( o );

    o = p.Begin( "Items", T( "ArrayProperty", StructType( "MyItem" ) ) );
    b.I32( 2 );
    if ( !p.IsNewLayout() ) inner = p.BeginInnerStructTag( "Items", "MyItem" );
    for ( UInt32 i=0; i<2; ++i )
    {
        CTestPropertyWriter::SOpen a = p.Begin( "Id", T( "IntProperty" ) ); b.I32( 100 + i ); p.End( a );
        p.None();
    }
    if ( !p.IsNewLayout() ) p.End( inner );
    p.End( o );

    o = p.Begin( "NoItems", T( "ArrayProperty", StructType( "MyItem" ) ) );
    b.I32( 0 );
    if ( !p.IsNewLayout() ) { inner = p.BeginInnerStructTag( "NoItems", "MyItem" ); p.End( inner ); }
    p.End( o );

    o = p.Begin( "Unique", T( "SetProperty", T( "IntProperty" ) ) ); b.I32( 0 ); b.I32( 2 ); b.I32( 5 ); b.I32( 6 ); p.End( o );
    o = p.Begin( "Lookup", T( "MapProperty", T( "NameProperty" ), T( "IntProperty" ) ) ); b.I32( 0 ); b.I32( 2 );
    package.names.WriteName( b, "K1" ); b.I32( 11 ); package.names.WriteName( b, "K2" ); b.I32( 22 ); p.End( o );
    o = p.Begin( "Table", T( "MapProperty", T( "StrProperty" ), StructType( "MyItem" ) ) ); b.I32( 0 ); b.I32( 1 );
    b.Str( "row" );
    { CTestPropertyWriter::SOpen a = p.Begin( "Id", T( "IntProperty" ) ); b.I32( 9 ); p.End( a ); p.None(); }
    p.End( o );
}

/*-------------------------------------------------------------------------*/

static bool
VerifyContainers( const CORE::CDataNode& root )
{
    CUnrealObjectProperties v( root );
    bool ok = true;
    VCHECK( 7 == v.GetInt32( "Settings/Level" ) );
    VCHECK( 2.5 == v.GetDouble( "Settings/Nested/Gain" ) );
    VCHECK( v.GetBool( "Settings/Nested/bOn" ) );
    VCHECK( "inner label" == v.GetString( "Settings/Label" ) );
    VCHECK( "MySettings" == v.Find( "Settings" )->GetAttributeValue( "struct" ).AsString() );
    VCHECK( 3 == v.GetCount( "Ints" ) );
    VCHECK( 20 == v.GetInt32( "Ints/1" ) );
    VCHECK( 0 == v.GetCount( "Empty" ) );
    VCHECK( v.Has( "Empty" ) );
    VCHECK( 3 == v.GetCount( "Bytes" ) );
    VCHECK( 3 == v.GetInt32( "Bytes/2" ) );
    VCHECK( "Alpha" == v.GetString( "Names/0" ) );
    VCHECK( "Beta_9" == v.GetString( "Names/1" ) );
    VCHECK( "two" == v.GetString( "Strings/1" ) );
    VCHECK( 3 == v.GetCount( "Flags" ) );
    VCHECK( !v.GetBool( "Flags/1" ) );
    VCHECK( v.GetBool( "Flags/2" ) );
    VCHECK( "/Script/Engine.StaticMesh" == v.GetString( "Refs/0" ) );
    VCHECK( "/Game/Test/Package.Obj" == v.GetString( "Refs/1" ) );
    VCHECK( 2 == v.GetCount( "Points" ) );
    VCHECK( 4 == v.GetComponent( "Points/1", "x" ) );
    VCHECK( 6 == v.GetComponent( "Points/1", "z" ) );
    VCHECK( 2 == v.GetCount( "Items" ) );
    VCHECK( 100 == v.GetInt32( "Items/0/Id" ) );
    VCHECK( 101 == v.GetInt32( "Items/1/Id" ) );
    VCHECK( 0 == v.GetCount( "NoItems" ) );
    VCHECK( v.Has( "NoItems" ) );
    VCHECK( 2 == v.GetCount( "Unique" ) );
    VCHECK( 6 == v.GetInt32( "Unique/1" ) );
    VCHECK( 2 == v.GetCount( "Lookup" ) );
    VCHECK( "K2" == v.GetString( "Lookup/1/key" ) );
    VCHECK( 22 == v.GetInt32( "Lookup/1/value" ) );
    VCHECK( "row" == v.GetString( "Table/0/key" ) );
    VCHECK( 9 == v.GetInt32( "Table/0/value/Id" ) );
    return ok;
}

/*-------------------------------------------------------------------------*/

static void
WriteAll( CTestPackageWriter& package, CTestPropertyWriter& p, CTestByteWriter& b )
{
    WriteScalars( package, p, b );
    WriteNativeStructs( package, p, b );
    WriteContainers( package, p, b );
}

/*-------------------------------------------------------------------------*/

static void
WriteExtras( CTestPackageWriter& /* package */, CTestPropertyWriter& p, CTestByteWriter& b )
{
    CTestPropertyWriter::SOpen o;
    // a static array: one tag per element, the index in the tag
    for ( Int32 i=0; i<3; ++i )
    {
        o = p.Begin( "Slots", T( "IntProperty" ), false, i ); b.I32( 1000 + i ); p.End( o );
    }
    // a tag with a property GUID and the overridable extension
    o = p.Begin( "WithGuid", T( "IntProperty" ), false, 0, false, true, true ); b.I32( 5 ); p.End( o );
    // a property of a kind that is not known is skipped by its size
    o = p.Begin( "Strange", T( "ZzzProperty" ) ); b.U32( 0xDEADBEEF ); b.U32( 0xCAFEF00D ); b.U32( 1 ); p.End( o );
    // a value that is shorter than the tag says: the reader goes to the declared end
    o = p.Begin( "Short", T( "IntProperty" ) ); b.I32( 42 ); b.I32( 0x7777 ); b.I32( 0x7777 ); p.End( o );
    o = p.Begin( "AfterShort", T( "IntProperty" ) ); b.I32( 43 ); p.End( o );
    // an array whose count cannot fit in its tag: the value is dropped and the next property is read
    o = p.Begin( "Liar", T( "ArrayProperty", T( "IntProperty" ) ) ); b.I32( 0x40000000 ); b.I32( 1 ); p.End( o );
    o = p.Begin( "AfterLiar", T( "IntProperty" ) ); b.I32( 44 ); p.End( o );
    // a string that claims more than the tag holds
    o = p.Begin( "BadString", T( "StrProperty" ) ); b.I32( 5000 ); b.U32( 0 ); p.End( o );
    o = p.Begin( "Last", T( "IntProperty" ) ); b.I32( 45 ); p.End( o );
}

/*-------------------------------------------------------------------------*/

static void
WritePackedArrays( CTestPackageWriter& package, CTestPropertyWriter& p, CTestByteWriter& b )
{
    CTestPropertyWriter::SOpen o;
    o = p.Begin( "Samples", T( "ArrayProperty", T( "FloatProperty" ) ) ); b.I32( 1000 ); for ( UInt32 i=0; i<1000; ++i ) b.F32( (Float32) i * 0.5f ); p.End( o );
    o = p.Begin( "Indices", T( "ArrayProperty", T( "IntProperty" ) ) ); b.I32( 300 ); for ( UInt32 i=0; i<300; ++i ) b.I32( 7 * i ); p.End( o );
    o = p.Begin( "Blob", T( "ArrayProperty", T( "ByteProperty" ) ) ); b.I32( 5000 ); for ( UInt32 i=0; i<5000; ++i ) b.U8( (UInt8) i ); p.End( o );
    {
        // many vectors: one block as well; the classic layout names the struct once in front of the elements
        const bool wide = package.ue5 >= UE5_LARGE_WORLD_COORDINATES;
        CTestPropertyWriter::SOpen inner;
        o = p.Begin( "Cloud", T( "ArrayProperty", StructType( "Vector" ) ), false, 0, p.IsNewLayout() );
        b.I32( 500 );
        if ( !p.IsNewLayout() ) inner = p.BeginInnerStructTag( "Cloud", "Vector" );
        for ( UInt32 i=0; i<500; ++i ) WriteVector( b, wide, i, 2.0 * i, -1.0 * i );
        if ( !p.IsNewLayout() ) p.End( inner );
        p.End( o );
    }
    o = p.Begin( "Short", T( "ArrayProperty", T( "IntProperty" ) ) ); b.I32( 256 ); for ( UInt32 i=0; i<256; ++i ) b.I32( (Int32) i ); p.End( o );
}

/*-------------------------------------------------------------------------*/

static void
WriteDeepStructs( CTestPackageWriter& /* package */, CTestPropertyWriter& p, CTestByteWriter& b )
{
    const UInt32 depth = 100;
    CTestPropertyWriter::SOpen opens[ 100 ];
    for ( UInt32 i=0; i<depth; ++i )
        opens[ i ] = p.Begin( "Deeper", StructType( "Nest" ) );
    CTestPropertyWriter::SOpen leaf = p.Begin( "Leaf", T( "IntProperty" ) ); b.I32( 1 ); p.End( leaf );
    for ( UInt32 i=depth; i>0; --i )
    {
        p.None();
        p.End( opens[ i - 1 ] );
    }
    CTestPropertyWriter::SOpen after = p.Begin( "AfterDeep", T( "IntProperty" ) ); b.I32( 77 ); p.End( after );
}

/*-------------------------------------------------------------------------*/

static void
WriteOptionals( CTestPackageWriter& /* package */, CTestPropertyWriter& p, CTestByteWriter& b )
{
    CTestPropertyWriter::SOpen o;
    o = p.Begin( "OptSet", T( "OptionalProperty", T( "IntProperty" ) ) ); b.U8( 1 ); b.I32( 12 ); p.End( o );
    o = p.Begin( "OptUnset", T( "OptionalProperty", T( "IntProperty" ) ) ); b.U8( 0 ); p.End( o );
    o = p.Begin( "OptWide", T( "OptionalProperty", T( "FloatProperty" ) ) ); b.U32( 1 ); b.F32( 1.5f ); p.End( o );
}

/*-------------------------------------------------------------------------*/

static void
WriteUnknownBinaryStruct( CTestPackageWriter& /* package */, CTestPropertyWriter& p, CTestByteWriter& b )
{
    CTestPropertyWriter::SOpen o;
    o = p.Begin( "Weird", StructType( "SomethingBinary" ), false, 0, true ); b.U32( 1 ); b.U32( 2 ); b.U32( 3 ); p.End( o );
    o = p.Begin( "AfterWeird", T( "IntProperty" ), false ); b.I32( 8 ); p.End( o );
}

/*-------------------------------------------------------------------------*/

void
BuildRichPackage( Int32 legacy, Int32 ue4, Int32 ue5, GUCEF::vector< UInt8 >& bytes )
{GUCEF_TRACE;

    CTestPackageWriter writer( legacy, ue4, ue5 );
    writer.customVersions.push_back( CUnrealCustomVersion( CUnrealGuid( 1, 2, 3, 4 ), 9 ) );
    const Int32 scriptPackage = writer.AddImport( "/Script/CoreUObject", "Package", 0, "/Script/Engine" );
    writer.AddImport( "/Script/CoreUObject", "Class", scriptPackage, "StaticMesh" );
    UInt32 payload = 0;
    if ( ue5 >= UE5_PAYLOAD_TOC )
        payload = writer.AddPayload( GUCEF::vector< UInt8 >( 5000, 9 ) );
    for ( UInt32 e=0; e<2; ++e )
    {
        CTestByteWriter body;
        writer.BeginBody( body );
        CTestPropertyWriter props( body, writer.names, writer.ue4, writer.ue5 );
        WriteAll( writer, props, body );
        WriteExtras( writer, props, body );
        props.None();
        if ( ue5 >= UE5_PAYLOAD_TOC )
            writer.WriteEditorBulkData( body, payload );
        writer.AddExport( -2, 0, 0 == e ? "ObjA" : "ObjB", body, (Int64) body.Size() - ( ue5 >= UE5_PAYLOAD_TOC ? 48 : 0 ) );
    }
    writer.Build( bytes );
}

/*-------------------------------------------------------------------------*/

void
PerformUnrealPropertyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Properties" );

    struct SVersion { Int32 legacy; Int32 ue4; Int32 ue5; const char* name; };
    const SVersion versions[] =
    {
        { -7, 508, 0, "4.14 (classic tags, float vectors)" },
        { -7, 522, 0, "4.27 (classic tags, float vectors)" },
        { -8, 522, 1003, "UE5 1003 (classic tags, float vectors)" },
        { -8, 522, 1009, "5.3 (classic tags, double vectors)" },
        { -8, 522, 1011, "UE5 1011 (classic tags with extension byte and control byte)" },
        { -9, 522, 1012, "UE5 1012 (complete type name tags)" },
        { -9, 522, 1017, "5.6 (complete type name tags)" },
        { -9, 522, 1018, "5.8 (complete type name tags)" }
    };

    GUCEF_TESTFW_TESTCASE( "Properties: every scalar kind in both tag layouts, from 4.14 to 5.8" )
    try
    {
        for ( UInt32 i=0; i<8; ++i )
        {
            CPropertyFixture fixture( versions[ i ].legacy, versions[ i ].ue4, versions[ i ].ue5 );
            const bool ok = fixture.Run( WriteScalars );
            if ( !ok || !fixture.mismatches.empty() || !VerifyScalars( fixture.root ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Scalars failed for " << versions[ i ].name << ": " << ( fixture.mismatches.empty() ? CORE::CString( "values" ) : fixture.mismatches[ 0 ] ) );
            ASSERT_TRUE( ok && fixture.mismatches.empty() && fixture.unknown.empty() );
            ASSERT_TRUE( VerifyScalars( fixture.root ) );
            ASSERT_TRUE( 29 == fixture.propertyCount );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: binary structs, float or double by version, and the types that name their width" )
    try
    {
        for ( UInt32 i=0; i<8; ++i )
        {
            CPropertyFixture fixture( versions[ i ].legacy, versions[ i ].ue4, versions[ i ].ue5 );
            const bool ok = fixture.Run( WriteNativeStructs );
            if ( !ok || !fixture.mismatches.empty() || !VerifyNativeStructs( fixture.root ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Native structs failed for " << versions[ i ].name << ": " << ( fixture.mismatches.empty() ? CORE::CString( "values" ) : fixture.mismatches[ 0 ] ) );
            ASSERT_TRUE( ok && fixture.mismatches.empty() );
            ASSERT_TRUE( VerifyNativeStructs( fixture.root ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: tagged structs, arrays, sets and maps, arrays of structs with the classic inner tag" )
    try
    {
        for ( UInt32 i=0; i<8; ++i )
        {
            CPropertyFixture fixture( versions[ i ].legacy, versions[ i ].ue4, versions[ i ].ue5 );
            const bool ok = fixture.Run( WriteContainers );
            if ( !ok || !fixture.mismatches.empty() || !VerifyContainers( fixture.root ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Containers failed for " << versions[ i ].name << ": " << ( fixture.mismatches.empty() ? CORE::CString( "values" ) : fixture.mismatches[ 0 ] ) );
            ASSERT_TRUE( ok && fixture.mismatches.empty() );
            ASSERT_TRUE( VerifyContainers( fixture.root ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: all kinds together in one export, the tree has one child per property" )
    try
    {
        for ( UInt32 i=0; i<8; ++i )
        {
            CPropertyFixture fixture( versions[ i ].legacy, versions[ i ].ue4, versions[ i ].ue5 );
            ASSERT_TRUE( fixture.Run( WriteAll ) );
            ASSERT_TRUE( fixture.mismatches.empty() && VerifyScalars( fixture.root ) && VerifyNativeStructs( fixture.root ) && VerifyContainers( fixture.root ) );
            ASSERT_TRUE( fixture.root.GetNrOfDirectChildNodes() == fixture.propertyCount - 4 || fixture.root.GetNrOfDirectChildNodes() > 40 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: static array indices, property GUIDs, extensions, unknown kinds, wrong sizes and bad values are contained by the tag size" )
    try
    {
        for ( UInt32 i=0; i<8; ++i )
        {
            CPropertyFixture fixture( versions[ i ].legacy, versions[ i ].ue4, versions[ i ].ue5 );
            ASSERT_TRUE( fixture.Run( WriteExtras ) );
            CUnrealObjectProperties v( fixture.root );
            // the three elements of the static array are three nodes with the index attribute
            UInt32 slots = 0;
            CORE::CDataNode::const_iterator c = fixture.root.ConstBegin();
            while ( c != fixture.root.ConstEnd() )
            {
                if ( "Slots" == (*c)->GetName() )
                {
                    ASSERT_TRUE( (Int32) slots == (*c)->GetAttributeValue( "index" ).AsInt32( 0 ) && 1000 + (Int32) slots == (*c)->GetValue().AsInt32() );
                    ++slots;
                }
                ++c;
            }
            ASSERT_TRUE( 3 == slots );
            ASSERT_TRUE( 5 == v.GetInt32( "WithGuid" ) );
            ASSERT_TRUE( 1 == fixture.unknown.size() && fixture.unknown[ 0 ].HasSubstr( "ZzzProperty" ) >= 0 && fixture.root.FindChild( "Strange" )->GetAttributeValue( "skipped" ).AsBool() );
            ASSERT_TRUE( 42 == v.GetInt32( "Short" ) && fixture.root.FindChild( "Short" )->GetAttributeValue( "mismatch" ).AsBool() && 43 == v.GetInt32( "AfterShort" ) );
            ASSERT_TRUE( fixture.root.FindChild( "Liar" )->GetAttributeValue( "unread" ).AsBool() && 44 == v.GetInt32( "AfterLiar" ) );
            ASSERT_TRUE( fixture.root.FindChild( "BadString" )->GetAttributeValue( "unread" ).AsBool() && 45 == v.GetInt32( "Last" ) );
            // each problem is reported once, with the class and the property
            ASSERT_TRUE( 3 == fixture.mismatches.size() );
            ASSERT_TRUE( fixture.mismatches[ 0 ].HasSubstr( "Property Short of class StaticMesh: value read 4 bytes, tag size 12 (skipped)" ) >= 0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: long arrays of numbers are one block of bytes, short ones are nodes" )
    try
    {
        for ( UInt32 i=0; i<8; i+=2 )
        {
            CPropertyFixture fixture( versions[ i ].legacy, versions[ i ].ue4, versions[ i ].ue5 );
            ASSERT_TRUE( fixture.Run( WritePackedArrays ) );
            CUnrealObjectProperties v( fixture.root );
            ASSERT_TRUE( fixture.mismatches.empty() );
            ASSERT_TRUE( 1000 == v.GetCount( "Samples" ) && 300 == v.GetCount( "Indices" ) && 5000 == v.GetCount( "Blob" ) && 256 == v.GetCount( "Short" ) );
            const CORE::CDataNode* samples = v.Find( "Samples" );
            ASSERT_TRUE( GUCEF_NULL != samples && samples->GetAttributeValue( "packed" ).AsBool() && 4 == samples->GetAttributeValue( "elementBytes" ).AsInt32() && "FloatProperty" == samples->GetAttributeValue( "elementType" ).AsString() );
            ASSERT_TRUE( 4000 == samples->GetValue().ByteSize( false ) );
            Float32 third = 0;
            memcpy( &third, static_cast< const UInt8* >( samples->GetValue().AsVoidPtr() ) + 12, 4 );
            ASSERT_TRUE( 1.5f == third );
            const CORE::CDataNode* cloud = v.Find( "Cloud" );
            const UInt32 vectorBytes = fixture.writer.ue5 >= UE5_LARGE_WORLD_COORDINATES ? 24 : 12;
            ASSERT_TRUE( GUCEF_NULL != cloud && 500 == v.GetCount( "Cloud" ) && cloud->GetAttributeValue( "packed" ).AsBool() && "Vector" == cloud->GetAttributeValue( "struct" ).AsString() );
            ASSERT_TRUE( vectorBytes == cloud->GetAttributeValue( "elementBytes" ).AsUInt32() && 500 * vectorBytes == cloud->GetValue().ByteSize( false ) );
            ASSERT_FALSE( v.Find( "Short" )->GetAttributeValue( "packed" ).AsBool() );
            ASSERT_TRUE( 255 == v.GetInt32( "Short/255" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: struct nesting beyond the limit is dropped without crashing, the next property is read" )
    try
    {
        for ( UInt32 i=0; i<8; i+=3 )
        {
            CPropertyFixture fixture( versions[ i ].legacy, versions[ i ].ue4, versions[ i ].ue5 );
            ASSERT_TRUE( fixture.Run( WriteDeepStructs ) );
            CUnrealObjectProperties v( fixture.root );
            ASSERT_TRUE( 77 == v.GetInt32( "AfterDeep" ) );
            ASSERT_TRUE( !fixture.mismatches.empty() );
            // the tree is cut at the limit of 64
            UInt32 levels = 0;
            const CORE::CDataNode* node = fixture.root.FindChild( "Deeper" );
            while ( GUCEF_NULL != node && levels < 200 ) { ++levels; node = node->FindChild( "Deeper" ); }
            ASSERT_TRUE( levels <= 66 && levels >= 60 );
        }
        // with a lower limit it is cut earlier
        CUnrealReaderLimits limits;
        limits.maxDepth = 8;
        CPropertyFixture shallow( -9, 522, 1018 );
        ASSERT_TRUE( shallow.Run( WriteDeepStructs, limits ) );
        UInt32 levels = 0;
        const CORE::CDataNode* node = shallow.root.FindChild( "Deeper" );
        while ( GUCEF_NULL != node && levels < 200 ) { ++levels; node = node->FindChild( "Deeper" ); }
        ASSERT_TRUE( levels <= 10 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: optional values with a one byte or a 32 bit flag, binary structs that are not known are skipped by size" )
    try
    {
        CPropertyFixture optionals( -9, 522, 1018 );
        ASSERT_TRUE( optionals.Run( WriteOptionals ) );
        CUnrealObjectProperties v( optionals.root );
        ASSERT_TRUE( optionals.mismatches.empty() );
        ASSERT_TRUE( 12 == v.GetInt32( "OptSet/value" ) );
        ASSERT_TRUE( optionals.root.FindChild( "OptSet" )->GetAttributeValue( "set" ).AsBool() );
        ASSERT_TRUE( !optionals.root.FindChild( "OptUnset" )->GetAttributeValue( "set" ).AsBool() );
        ASSERT_TRUE( 1.5 == v.GetDouble( "OptWide/value" ) );
        CPropertyFixture weird( -9, 522, 1018 );
        ASSERT_TRUE( weird.Run( WriteUnknownBinaryStruct ) );
        CUnrealObjectProperties w( weird.root );
        ASSERT_TRUE( weird.root.FindChild( "Weird" )->GetAttributeValue( "skipped" ).AsBool() && 8 == w.GetInt32( "AfterWeird" ) && weird.mismatches.empty() && 1 == weird.unknown.size() && weird.unknown[ 0 ].HasSubstr( "SomethingBinary" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: the type name tree: parameters, nesting and text form" )
    try
    {
        CUnrealPropertyTypeName type;
        type.AddNode( "MapProperty", 2 );
        type.AddNode( "NameProperty", 0 );
        type.AddNode( "StructProperty", 1 );
        type.AddNode( "Foo", 1 );
        type.AddNode( "/Script/Bar", 0 );
        ASSERT_TRUE( "MapProperty(NameProperty,StructProperty(Foo(/Script/Bar)))" == type.ToString() );
        ASSERT_TRUE( 2 == type.GetParameterCount() && "NameProperty" == type.GetParameterName( 0 ) && "StructProperty" == type.GetParameterName( 1 ) );
        const CUnrealPropertyTypeName value = type.GetParameter( 1 );
        ASSERT_TRUE( "StructProperty(Foo(/Script/Bar))" == value.ToString() && "Foo" == value.GetParameterName( 0 ) && value.GetParameter( 5 ).IsEmpty() );
        ASSERT_TRUE( type.GetParameterName( 9 ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

