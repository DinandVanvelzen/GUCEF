/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#include <float.h>
#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

#ifndef GUCEF_MATH_SPROJECTIONCONVENTION_H
#include "gucefMATH_SProjectionConvention.h"
#define GUCEF_MATH_SPROJECTIONCONVENTION_H
#endif /* GUCEF_MATH_SPROJECTIONCONVENTION_H ? */

#include "TestMathReference.h"
#include "TestRotationSerialization.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The value types implement the members that the SFINAE helpers of CIDataNodeSerializable detect (no vtable) */
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat32Matrix4D >::value, TFloat32Matrix4D_has_Serialize );
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeDeserialization< TFloat64Matrix3D >::value, TFloat64Matrix3D_has_Deserialize );
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat32Quaternion >::value, TFloat32Quaternion_has_Serialize );
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat64Transform3D >::value, TFloat64Transform3D_has_Serialize );
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat32EulerAngles >::value, TFloat32EulerAngles_has_Serialize );
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< CCoordinateSystem >::value, CCoordinateSystem_has_Serialize );
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< SProjectionConvention >::value, SProjectionConvention_has_Serialize );

typedef CORE::CDStoreCodecRegistry::TDStoreCodecPtr TCodecPtr;

/*-------------------------------------------------------------------------*/

static bool
GetJsonCodec( TCodecPtr& codec )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

/** Loads the JSON codec plugin from the module directory when it is not registered yet */
static bool
JsonCodecAvailable( void )
{GUCEF_TRACE;

    TCodecPtr codec;
    if ( GetJsonCodec( codec ) )
        return true;
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( "$MODULEDIR$/dstorepluginJSONPARSER", "test", true ) )
        pluginControl.AddPluginFromDir( "$MODULEDIR$/dstorepluginJSONPARSER_d", "test", true );
    return GetJsonCodec( codec );
}

/*-------------------------------------------------------------------------*/

static bool
WriteJson( const CORE::CDataNode& tree, CORE::CString& text )
{GUCEF_TRACE;

    TCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        return false;
    CORE::CDynamicBuffer buffer;
    {
        CORE::CDynamicBufferAccess access( &buffer, false );
        if ( !codec->StoreDataTree( &tree, &access ) )
            return false;
    }
    UInt32 size = buffer.GetDataSize();
    const char* data = static_cast< const char* >( buffer.GetConstBufferPtr() );
    while ( size > 0 && '\0' == data[ size-1 ] )
        --size;
    text = CORE::CString( data, size );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ParseJson( const CORE::CString& text, CORE::CDataNode& tree )
{GUCEF_TRACE;

    TCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        return false;
    tree.Clear();
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &tree, &access );
}

/*-------------------------------------------------------------------------*/

/** The first node of the tree (depth first, the root included) with an attribute or a child node called 'name' */
static const CORE::CDataNode*
FindNodeWith( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    if ( GUCEF_NULL != node.GetAttribute( name ) || GUCEF_NULL != node.FindChild( name ) )
        return &node;
    CORE::CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        const CORE::CDataNode* found = FindNodeWith( **i, name );
        if ( GUCEF_NULL != found )
            return found;
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** Serialize, write JSON text, parse it and deserialize; 'key' names an attribute or child of the value's node */
template < typename TValue >
static bool
JsonRoundTrip( const TValue& value, TValue& back, const char* key )
{
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode node( "value", GUCEF_DATATYPE_OBJECT );
    if ( !value.Serialize( node, settings ) )
        return false;
    CORE::CString text;
    if ( !WriteJson( node, text ) )
        return false;
    CORE::CDataNode parsed;
    if ( !ParseJson( text, parsed ) )
        return false;
    const CORE::CDataNode* holder = FindNodeWith( parsed, key );
    return GUCEF_NULL != holder && back.Deserialize( *holder, settings );
}

/*-------------------------------------------------------------------------*/

/** Serialize into a DataNode and deserialize directly (no codec) */
template < typename TValue >
static bool
NodeRoundTrip( const TValue& value, TValue& back )
{
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode node( "value", GUCEF_DATATYPE_OBJECT );
    return value.Serialize( node, settings ) && back.Deserialize( node, settings );
}

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
SameBits( const T& a, const T& b )
{
    return 0 == memcmp( &a, &b, sizeof( T ) );
}

/*-------------------------------------------------------------------------*/

static Float32
RandomFinite( MathReference::CSeededSequence& generator, Float32 )
{
    for ( ;; )
    {
        UInt32 bits = generator.NextUInt32();
        Float32 value;
        memcpy( &value, &bits, 4 );
        if ( CTNumericTraits< Float32 >::IsFinite( value ) )
            return value;
    }
}

static Float64
RandomFinite( MathReference::CSeededSequence& generator, Float64 )
{
    for ( ;; )
    {
        UInt64 bits = generator.NextUInt64();
        Float64 value;
        memcpy( &value, &bits, 8 );
        if ( CTNumericTraits< Float64 >::IsFinite( value ) )
            return value;
    }
}

/*-------------------------------------------------------------------------*/

/** Seeded values of every new class of kind T with arbitrary finite components (unit axes for the axis-angle) */
template < typename T >
struct SRandomValues
{
    CTMatrix2D< T > m2;
    CTMatrix3D< T > m3;
    CTMatrix4D< T > m4;
    CTQuaternion< T > q;
    CTEulerAngles< T > e;
    CTAxisAngle< T > a;
    CTTransform2D< T > t2;
    CTTransform3D< T > t3;

    void Fill( MathReference::CSeededSequence& generator )
    {
        T v[ 16 ];
        for ( UInt32 i=0; i<16; ++i )
            v[ i ] = RandomFinite( generator, T() );
        m2 = CTMatrix2D< T >::FromColumnMajor( v );
        m3 = CTMatrix3D< T >::FromColumnMajor( v );
        m4 = CTMatrix4D< T >::FromColumnMajor( v );
        q = CTQuaternion< T >::FromXYZW( v[ 0 ], v[ 1 ], v[ 2 ], v[ 3 ] );
        e = CTEulerAngles< T >( CTRadian< T >( v[ 4 ] ), CTRadian< T >( v[ 5 ] ), CTRadian< T >( v[ 6 ] ), (TEulerOrder) ( generator.NextUInt32() % 12 ), (TEulerFrame) ( generator.NextUInt32() % 2 ) );
        CTDirectionVector3D< T > axis;
        if ( !CTDirectionVector3D< T >::TryCreate( CTVector3D< T >( (T) generator.NextFloat64( -1, 1 ), (T) generator.NextFloat64( -1, 1 ), (T) generator.NextFloat64( 0.1, 1 ) ), axis ) )
            axis = CTDirectionVector3D< T >::UnitX();
        a = CTAxisAngle< T >( axis, CTRadian< T >( v[ 7 ] ) );
        t2 = CTTransform2D< T >( CTVector2D< T >( v[ 8 ], v[ 9 ] ), CTRadian< T >( v[ 10 ] ), CTVector2D< T >( v[ 11 ], v[ 12 ] ) );
        t3 = CTTransform3D< T >( CTVector3D< T >( v[ 13 ], v[ 14 ], v[ 15 ] ), q, CTVector3D< T >( v[ 2 ], v[ 0 ], v[ 9 ] ) );
    }
};

/*-------------------------------------------------------------------------*/

/** ToString / FromString round trips of every class, bit for bit; returns the number of failures */
template < typename T >
static UInt32
TextRoundTrips( const SRandomValues< T >& in )
{
    UInt32 failures = 0;
    SRandomValues< T > out;
    if ( !out.m2.FromString( in.m2.ToString() ) || !SameBits( out.m2, in.m2 ) ) ++failures;
    if ( !out.m3.FromString( in.m3.ToString() ) || !SameBits( out.m3, in.m3 ) ) ++failures;
    if ( !out.m4.FromString( in.m4.ToString() ) || !SameBits( out.m4, in.m4 ) ) ++failures;
    if ( !out.q.FromString( in.q.ToString() ) || !SameBits( out.q, in.q ) ) ++failures;
    if ( !out.e.FromString( in.e.ToString() ) || !( out.e == in.e ) || !SameBits( out.e.first, in.e.first ) ) ++failures;
    if ( !out.a.FromString( in.a.ToString() ) || !( out.a == in.a ) ) ++failures;
    if ( !out.t2.FromString( in.t2.ToString() ) || !SameBits( out.t2, in.t2 ) ) ++failures;
    if ( !out.t3.FromString( in.t3.ToString() ) || !SameBits( out.t3, in.t3 ) ) ++failures;
    return failures;
}

/*-------------------------------------------------------------------------*/

/** DataNode round trips (JSON text when 'json', else directly) of every class, bit for bit */
template < typename T >
static UInt32
NodeRoundTrips( const SRandomValues< T >& in, bool json )
{
    UInt32 failures = 0;
    SRandomValues< T > out;
    if ( !( json ? JsonRoundTrip( in.m2, out.m2, "m00" ) : NodeRoundTrip( in.m2, out.m2 ) ) || !SameBits( out.m2, in.m2 ) ) ++failures;
    if ( !( json ? JsonRoundTrip( in.m3, out.m3, "m22" ) : NodeRoundTrip( in.m3, out.m3 ) ) || !SameBits( out.m3, in.m3 ) ) ++failures;
    if ( !( json ? JsonRoundTrip( in.m4, out.m4, "m33" ) : NodeRoundTrip( in.m4, out.m4 ) ) || !SameBits( out.m4, in.m4 ) ) ++failures;
    if ( !( json ? JsonRoundTrip( in.q, out.q, "w" ) : NodeRoundTrip( in.q, out.q ) ) || !SameBits( out.q, in.q ) ) ++failures;
    if ( !( json ? JsonRoundTrip( in.e, out.e, "order" ) : NodeRoundTrip( in.e, out.e ) ) || !( out.e == in.e ) ) ++failures;
    if ( !( json ? JsonRoundTrip( in.a, out.a, "radians" ) : NodeRoundTrip( in.a, out.a ) ) || !( out.a == in.a ) ) ++failures;
    if ( !( json ? JsonRoundTrip( in.t2, out.t2, "position" ) : NodeRoundTrip( in.t2, out.t2 ) ) || !SameBits( out.t2, in.t2 ) ) ++failures;
    if ( !( json ? JsonRoundTrip( in.t3, out.t3, "position" ) : NodeRoundTrip( in.t3, out.t3 ) ) || !SameBits( out.t3, in.t3 ) ) ++failures;
    return failures;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformRotationSerializationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "RotationSerialization" );

    GUCEF_TESTFW_TESTCASE( "RotationSerialization: text forms of section 9.1" )
    try
    {
        TFloat32Matrix2D m2 = TFloat32Matrix2D::FromColumns( TFloat32Vector2D( 1, 3 ), TFloat32Vector2D( 2, 4 ) );
        ASSERT_TRUE( m2.ToString() == "[[1, 2], [3, 4]]" );
        ASSERT_TRUE( TFloat32Quaternion::FromXYZW( 0, 0.5f, 0, 1 ).ToString() == "(0, 0.5, 0, 1)" );
        ASSERT_TRUE( TFloat32Transform3D().ToString() == "T(0, 0, 0) R(0, 0, 0, 1) S(1, 1, 1)" );
        ASSERT_TRUE( TFloat32Transform2D().ToString() == "T(0, 0) R(0 rad) S(1, 1)" );
        ASSERT_TRUE( TFloat32EulerAngles().ToString() == "(YXZ, intrinsic, 0 rad, 0 rad, 0 rad)" );
        ASSERT_TRUE( TFloat32AxisAngle().ToString() == "((1, 0, 0), 0 rad)" );
        ASSERT_TRUE( CCoordinateSystem::Unreal().ToString() == "(+Y, +Z, +X, 100)" );
        ASSERT_TRUE( SProjectionConvention::Vulkan().ToString() == "(right, 0..1, flipY)" && SProjectionConvention::ReversedZInfinite( HANDEDNESS_LEFT ).ToString() == "(left, 0..1, reversedZ, infiniteFar)" );

        // white space, degrees on input
        TFloat64Matrix3D m3;
        ASSERT_TRUE( m3.FromString( "  [ [1,2 ,3] ,[4, 5, 6],\t[7, 8, 9] ]  " ) && 6.0 == m3( 1, 2 ) && 7.0 == m3( 2, 0 ) );
        TFloat64EulerAngles e;
        ASSERT_TRUE( e.FromString( " ( ZYX , extrinsic, 90 deg, 0.5 rad,-1 rad ) " ) && EULERORDER_ZYX == e.order && EULERFRAME_EXTRINSIC == e.frame && fabs( e.first.GetValue() - 1.5707963267948966 ) < 1e-15 && 0.5 == e.second.GetValue() );
        TFloat64AxisAngle a;
        ASSERT_TRUE( a.FromString( "( (0, 1, 0) , 180 deg )" ) && a.axis == TFloat64DirectionVector3D::UnitY() && fabs( a.angle.GetValue() - 3.141592653589793 ) < 1e-15 );
        // the axis must be of unit length, as for CTDirectionVector3D
        ASSERT_FALSE( a.FromString( "( (0, 2, 0) , 1 rad )" ) );
        TFloat64Transform2D t2;
        ASSERT_TRUE( t2.FromString( "T(1, 2)R( 45 deg )S(3,4)" ) && fabs( t2.rotation.GetValue() - 0.7853981633974483 ) < 1e-15 && 4.0 == t2.scale.y );
        TFloat64Transform3D t3;
        ASSERT_TRUE( t3.FromString( " T (1, 2, 3)  R (0, 0, 0, 1) S (2, 2, 2) " ) && 3.0 == t3.position.z && 2.0 == t3.scale.x );
        CCoordinateSystem system;
        ASSERT_TRUE( system.FromString( "( -X , +Z, -Y, 0.0254 )" ) && SIGNEDAXIS_NEGATIVE_Y == system.GetForwardAxis() && 0.0254 == system.GetUnitsPerMetre() );
        SProjectionConvention convention = SProjectionConvention::OpenGL();
        ASSERT_TRUE( convention.FromString( "( left , -1..1, flipY , reversedZ )" ) && HANDEDNESS_LEFT == convention.handedness && convention.flipY && convention.reversedZ && !convention.infiniteFar );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RotationSerialization: malformed text is rejected and leaves the value unchanged" )
    try
    {
        const TFloat64Matrix4D m4 = TFloat64Matrix4D::Translation( TFloat64Vector3D( 1, 2, 3 ) );
        TFloat64Matrix4D target = m4;
        const char* const badMatrices[] = { "", "[]", "[[1, 2, 3, 4]]", "[[1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3]]",
                                            "[[1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3, 4]] x", "[(1, 2, 3, 4), [1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3, 4]]",
                                            "[[1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3, a]]", "[[1, 2, 3, 4] [1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3, 4]]" };
        UInt32 accepted = 0;
        for ( UInt32 i=0; i<sizeof( badMatrices ) / sizeof( badMatrices[ 0 ] ); ++i )
            if ( target.FromString( badMatrices[ i ] ) )
                ++accepted;
        ASSERT_TRUE( 0 == accepted && target == m4 );

        TFloat64Quaternion q = TFloat64Quaternion::FromXYZW( 1, 2, 3, 4 );
        ASSERT_FALSE( q.FromString( "(1, 2, 3)" ) );
        ASSERT_FALSE( q.FromString( "[1, 2, 3, 4]" ) );
        ASSERT_TRUE( TFloat64Quaternion::FromXYZW( 1, 2, 3, 4 ) == q );
        TFloat64EulerAngles e;
        ASSERT_FALSE( e.FromString( "(XYW, intrinsic, 0 rad, 0 rad, 0 rad)" ) );
        ASSERT_FALSE( e.FromString( "(XYZ, sideways, 0 rad, 0 rad, 0 rad)" ) );
        ASSERT_FALSE( e.FromString( "(XYZ, intrinsic, 0 rad, 0 rad, 0)" ) );
        ASSERT_FALSE( e.FromString( "(XYZ, intrinsic, 0 rad, 0 rad)" ) );
        ASSERT_TRUE( e == TFloat64EulerAngles() );
        TFloat64AxisAngle a;
        ASSERT_FALSE( a.FromString( "((0, 0, 0), 1 rad)" ) );
        ASSERT_FALSE( a.FromString( "((1, 0, 0), 1 grad)" ) );
        ASSERT_TRUE( a == TFloat64AxisAngle() );
        TFloat64Transform3D t3;
        ASSERT_FALSE( t3.FromString( "T(1, 2, 3) S(1, 1, 1) R(0, 0, 0, 1)" ) );
        ASSERT_FALSE( t3.FromString( "T(1, 2, 3) R(0, 0, 0, 1) S(1, 1, 1) extra" ) );
        ASSERT_FALSE( t3.FromString( "T(1, 2) R(0, 0, 0, 1) S(1, 1, 1)" ) );
        ASSERT_TRUE( t3 == TFloat64Transform3D() );
        TFloat64Transform2D t2;
        ASSERT_FALSE( t2.FromString( "T(1, 2) R(1) S(1, 1)" ) );
        ASSERT_TRUE( t2 == TFloat64Transform2D() );
        CCoordinateSystem system = CCoordinateSystem::Unity();
        ASSERT_FALSE( system.FromString( "(+X, -X, +Z, 1)" ) );
        ASSERT_FALSE( system.FromString( "(+X, +Y, +Z, 0)" ) );
        ASSERT_FALSE( system.FromString( "(+X, +Y, +W, 1)" ) );
        ASSERT_FALSE( system.FromString( "(+X, +Y, +Z)" ) );
        ASSERT_TRUE( system == CCoordinateSystem::Unity() );
        SProjectionConvention convention = SProjectionConvention::Vulkan();
        ASSERT_FALSE( convention.FromString( "(right)" ) );
        ASSERT_FALSE( convention.FromString( "(right, 0..1, flipY, flipY)" ) );
        ASSERT_FALSE( convention.FromString( "(up, 0..1)" ) );
        ASSERT_TRUE( convention == SProjectionConvention::Vulkan() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RotationSerialization: 10^4 seeded values of each float kind round trip through text bit for bit" )
    try
    {
        // round trip of every rotation kind
        const UInt64 seed = 0x7E47;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures32 = 0;
        UInt32 failures64 = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            SRandomValues< Float32 > v32;
            v32.Fill( generator );
            failures32 += TextRoundTrips( v32 );
            SRandomValues< Float64 > v64;
            v64.Fill( generator );
            failures64 += TextRoundTrips( v64 );
        }
        ASSERT_TRUE( 0 == failures32 && 0 == failures64 );
        if ( 0 != failures32 || 0 != failures64 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RotationSerialization: text round trip failures Float32 " << failures32 << ", Float64 " << failures64 << ", seed " << seed );

        // fixed point, the mixed-precision transform, every coordinate system and every projection convention
        TFixed32Matrix4D fixedMatrix = TFixed32Matrix4D::FromTRS( TFixed32Vector3D( TFixed32( 1.5 ), TFixed32( -2.25 ), TFixed32( 3.0 ) ), TFixed32Quaternion(), TFixed32Vector3D::One() );
        TFixed32Matrix4D fixedBack;
        ASSERT_TRUE( fixedBack.FromString( fixedMatrix.ToString() ) && fixedBack == fixedMatrix );
        ASSERT_TRUE( NodeRoundTrip( fixedMatrix, fixedBack ) && fixedBack == fixedMatrix );
        TFixed64Quaternion fixedQ( TFloat64Quaternion::FromXYZW( 0.125, -0.5, 0.25, 0.8 ) );
        TFixed64Quaternion fixedQBack;
        ASSERT_TRUE( fixedQBack.FromString( fixedQ.ToString() ) && fixedQBack == fixedQ );
        TFloat32Float64Transform3D mixed( TFloat64Vector3D( 1e7 + 0.01, -3, 0.1 ), TFloat32Quaternion::FromXYZW( 0, 0.6f, 0, 0.8f ), TFloat32Vector3D( 1, 2, 3 ) );
        TFloat32Float64Transform3D mixedBack;
        // compared member-wise: the mixed transform has padding bytes
        ASSERT_TRUE( mixedBack.FromString( mixed.ToString() ) && mixedBack == mixed );
        mixedBack = TFloat32Float64Transform3D();
        ASSERT_TRUE( NodeRoundTrip( mixed, mixedBack ) && mixedBack == mixed );
        UInt32 conventionFailures = 0;
        for ( UInt32 bits=0; bits<32; ++bits )
        {
            SProjectionConvention c = SProjectionConvention::Make( (THandedness) ( bits & 1 ), (TDepthRange) ( ( bits >> 1 ) & 1 ), 0 != ( bits & 4 ), 0 != ( bits & 8 ), 0 != ( bits & 16 ) );
            SProjectionConvention back = SProjectionConvention::OpenGL();
            if ( !back.FromString( c.ToString() ) || back != c )
                ++conventionFailures;
            back = SProjectionConvention::OpenGL();
            if ( !NodeRoundTrip( c, back ) || back != c )
                ++conventionFailures;
        }
        UInt32 systemFailures = 0;
        for ( UInt32 r=0; r<6; ++r )
            for ( UInt32 u=0; u<6; ++u )
                for ( UInt32 f=0; f<6; ++f )
                {
                    if ( r / 2 == u / 2 || r / 2 == f / 2 || u / 2 == f / 2 )
                        continue;
                    CCoordinateSystem s( (TSignedAxis) r, (TSignedAxis) u, (TSignedAxis) f, 0.3048 * ( 1 + r ) );
                    CCoordinateSystem back;
                    if ( !back.FromString( s.ToString() ) || back != s )
                        ++systemFailures;
                    back = CCoordinateSystem();
                    if ( !NodeRoundTrip( s, back ) || back != s )
                        ++systemFailures;
                }
        ASSERT_TRUE( 0 == conventionFailures && 0 == systemFailures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RotationSerialization: DataNode attributes and child nodes, missing attributes fail" )
    try
    {
        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node( "value", GUCEF_DATATYPE_OBJECT );
        TFloat64Matrix4D m = TFloat64Matrix4D::Translation( TFloat64Vector3D( 1, 2, 3 ) );
        ASSERT_TRUE( m.Serialize( node, settings ) );
        // attributes by row and column, independent of the storage order
        ASSERT_TRUE( GUCEF_NULL != node.GetAttribute( "m03" ) && 1.0 == node.GetAttribute( "m03" )->second.AsFloat64() && GUCEF_NULL != node.GetAttribute( "m33" ) );
        CORE::CDataNode partial( "value", GUCEF_DATATYPE_OBJECT );
        partial.SetAttribute( "m00", CORE::CVariant( 5.0 ) );
        TFloat64Matrix4D unchanged = m;
        ASSERT_FALSE( unchanged.Deserialize( partial, settings ) );
        ASSERT_TRUE( unchanged == m );

        CORE::CDataNode transformNode( "value", GUCEF_DATATYPE_OBJECT );
        TFloat64Transform3D t( TFloat64Vector3D( 4, 5, 6 ), TFloat64Quaternion(), TFloat64Vector3D( 2, 2, 2 ) );
        ASSERT_TRUE( t.Serialize( transformNode, settings ) );
        ASSERT_TRUE( GUCEF_NULL != transformNode.FindChild( "position" ) && GUCEF_NULL != transformNode.FindChild( "rotation" ) && GUCEF_NULL != transformNode.FindChild( "scale" ) );
        // serialising again does not duplicate the child nodes
        ASSERT_TRUE( t.Serialize( transformNode, settings ) && 3 == transformNode.GetNrOfChildNodes() );

        // angles: radians written, degrees accepted
        CORE::CDataNode eulerNode( "value", GUCEF_DATATYPE_OBJECT );
        eulerNode.SetAttribute( "order", CORE::CVariant( "ZXZ" ) );
        eulerNode.SetAttribute( "frame", CORE::CVariant( "extrinsic" ) );
        eulerNode.SetAttribute( "firstDegrees", CORE::CVariant( 90.0 ) );
        eulerNode.SetAttribute( "second", CORE::CVariant( 0.25 ) );
        eulerNode.SetAttribute( "thirdDegrees", CORE::CVariant( -180.0 ) );
        TFloat64EulerAngles e;
        ASSERT_TRUE( e.Deserialize( eulerNode, settings ) && EULERORDER_ZXZ == e.order && fabs( e.first.GetValue() - 1.5707963267948966 ) < 1e-15 && 0.25 == e.second.GetValue() );
        eulerNode.SetAttribute( "order", CORE::CVariant( "ZZZ" ) );
        TFloat64EulerAngles untouched;
        ASSERT_FALSE( untouched.Deserialize( eulerNode, settings ) );
        ASSERT_TRUE( untouched == TFloat64EulerAngles() );

        CORE::CDataNode conventionNode( "value", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( SProjectionConvention::Direct3D().Serialize( conventionNode, settings ) );
        ASSERT_TRUE( conventionNode.GetAttribute( "handedness" )->second.AsString() == "left" );
        conventionNode.SetAttribute( "depthRange", CORE::CVariant( "0..2" ) );
        SProjectionConvention convention = SProjectionConvention::OpenGL();
        ASSERT_FALSE( convention.Deserialize( conventionNode, settings ) );
        ASSERT_TRUE( convention == SProjectionConvention::OpenGL() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RotationSerialization: seeded values round trip through DataNode bit for bit" )
    try
    {
        const UInt64 seed = 0xDA7A;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<2000; ++i )
        {
            SRandomValues< Float32 > v32;
            v32.Fill( generator );
            failures += NodeRoundTrips( v32, false );
            SRandomValues< Float64 > v64;
            v64.Fill( generator );
            failures += NodeRoundTrips( v64, false );
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RotationSerialization: " << failures << " DataNode round trips failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RotationSerialization: seeded values round trip through JSON text bit for bit" )
    GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "the JSON codec plugin (dstorepluginJSONPARSER) is not available next to the test application" );
    try
    {
        const UInt64 seed = 0x150E;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<300; ++i )
        {
            SRandomValues< Float32 > v32;
            v32.Fill( generator );
            failures += NodeRoundTrips( v32, true );
            SRandomValues< Float64 > v64;
            v64.Fill( generator );
            failures += NodeRoundTrips( v64, true );
        }
        CCoordinateSystem back;
        if ( !JsonRoundTrip( CCoordinateSystem::Unreal(), back, "right" ) || back != CCoordinateSystem::Unreal() )
            ++failures;
        SProjectionConvention convention = SProjectionConvention::OpenGL();
        if ( !JsonRoundTrip( SProjectionConvention::Vulkan(), convention, "handedness" ) || convention != SProjectionConvention::Vulkan() )
            ++failures;
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RotationSerialization: " << failures << " JSON round trips failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
