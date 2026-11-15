/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

#ifndef GUCEF_GEOMETRY_CORESTRUCTINTEROP_H
#include "gucefGEOMETRY_CoreStructInterop.h"
#define GUCEF_GEOMETRY_CORESTRUCTINTEROP_H
#endif /* GUCEF_GEOMETRY_CORESTRUCTINTEROP_H ? */

#include "TestGeometryReference.h"
#include "TestShapeSerialization.h"

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
using namespace GUCEF::GEOMETRY;
using namespace GeometryReference;

/** The value types implement the members that the SFINAE helpers of CIDataNodeSerializable detect (no vtable) */
#define GUCEF_TEST_SHAPE_HAS_SERIALIZE( shape ) \
    GUCEF_GEOMETRY_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< shape >::value, shape##_has_Serialize ); \
    GUCEF_GEOMETRY_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeDeserialization< shape >::value, shape##_has_Deserialize )

GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Ray3D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Line3D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Segment3D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Plane );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Sphere );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32AxisAlignedBox3D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Triangle3D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Capsule );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Ray2D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Line2D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Segment2D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Circle );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Rectangle );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32OrientedRectangle );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Triangle2D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Polygon2D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32PolygonWithHoles2D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Polyline2D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TFloat32Polyline3D );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TInt32IntegerRect );
GUCEF_TEST_SHAPE_HAS_SERIALIZE( TInt32IntegerBox3D );

typedef CORE::CDStoreCodecRegistry::TDStoreCodecPtr TCodecPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/** The first node of the tree (depth first, the root included) that has a child or an attribute 'name' */
static const CORE::CDataNode*
FindNodeWithMember( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    if ( GUCEF_NULL != node.GetAttribute( name ) || GUCEF_NULL != node.FindChild( name ) )
        return &node;
    CORE::CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        const CORE::CDataNode* found = FindNodeWithMember( **i, name );
        if ( GUCEF_NULL != found )
            return found;
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** Serialize, write JSON text, parse it and deserialize at the node that has the member 'member': true when the value comes back equal */
template < typename TShape >
static bool
JsonRoundTrip( const TShape& value, const char* member, TShape& back )
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
    const CORE::CDataNode* holder = FindNodeWithMember( parsed, member );
    return GUCEF_NULL != holder && back.Deserialize( *holder, settings ) && back == value;
}

/*-------------------------------------------------------------------------*/

static Float32
RandomFiniteFloat32( CSeededSequence& generator )
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

/*-------------------------------------------------------------------------*/

static Float64
RandomFiniteFloat64( CSeededSequence& generator )
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

template < typename T > static T RandomValue( CSeededSequence& generator );
template <> Float32 RandomValue< Float32 >( CSeededSequence& generator ) { return RandomFiniteFloat32( generator ); }
template <> Float64 RandomValue< Float64 >( CSeededSequence& generator ) { return RandomFiniteFloat64( generator ); }
template <> Int32 RandomValue< Int32 >( CSeededSequence& generator ) { return (Int32) generator.NextUInt32(); }

/*-------------------------------------------------------------------------*/

template < typename T > static CTPoint2D< T > RandomPoint2D( CSeededSequence& g ) { return CTPoint2D< T >( RandomValue< T >( g ), RandomValue< T >( g ) ); }
template < typename T > static CTPoint3D< T > RandomPoint3D( CSeededSequence& g ) { return CTPoint3D< T >( RandomValue< T >( g ), RandomValue< T >( g ), RandomValue< T >( g ) ); }
template < typename T > static CTVector2D< T > RandomVector2D( CSeededSequence& g ) { return CTVector2D< T >( RandomValue< T >( g ), RandomValue< T >( g ) ); }
template < typename T > static CTVector3D< T > RandomVector3D( CSeededSequence& g ) { return CTVector3D< T >( RandomValue< T >( g ), RandomValue< T >( g ), RandomValue< T >( g ) ); }

template < typename T >
static CTDirectionVector3D< T >
RandomDirection3D( CSeededSequence& g )
{
    for ( ;; )
    {
        CTVector3D< T > v( (T) g.NextFloat64( -1, 1 ), (T) g.NextFloat64( -1, 1 ), (T) g.NextFloat64( -1, 1 ) );
        CTDirectionVector3D< T > d;
        if ( CTDirectionVector3D< T >::TryCreate( v, d ) )
            return d;
    }
}

template < typename T >
static CTDirectionVector2D< T >
RandomDirection2D( CSeededSequence& g )
{
    for ( ;; )
    {
        CTVector2D< T > v( (T) g.NextFloat64( -1, 1 ), (T) g.NextFloat64( -1, 1 ) );
        CTDirectionVector2D< T > d;
        if ( CTDirectionVector2D< T >::TryCreate( v, d ) )
            return d;
    }
}

/*-------------------------------------------------------------------------*/

/** Random values of every shape of the kind T, the polygons with a random number of points */
template < typename T >
struct SShapeSet
{
    CTRay3D< T > ray3;
    CTLine3D< T > line3;
    CTSegment3D< T > segment3;
    CTPlane< T > plane;
    CTSphere< T > sphere;
    CTAxisAlignedBox3D< T > box;
    CTTriangle3D< T > triangle3;
    CTCapsule< T > capsule;
    CTRay2D< T > ray2;
    CTLine2D< T > line2;
    CTSegment2D< T > segment2;
    CTCircle< T > circle;
    CTRectangle< T > rectangle;
    CTOrientedRectangle< T > oriented;
    CTTriangle2D< T > triangle2;
    CTPolygon2D< T > polygon;
    CTPolygonWithHoles2D< T > region;
    CTPolyline2D< T > polyline2;
    CTPolyline3D< T > polyline3;

    explicit SShapeSet( CSeededSequence& g )
    {
        ray3 = CTRay3D< T >( RandomPoint3D< T >( g ), RandomDirection3D< T >( g ) );
        line3 = CTLine3D< T >( RandomPoint3D< T >( g ), RandomDirection3D< T >( g ) );
        segment3 = CTSegment3D< T >( RandomPoint3D< T >( g ), RandomPoint3D< T >( g ) );
        plane = CTPlane< T >( RandomDirection3D< T >( g ), RandomValue< T >( g ) );
        sphere = CTSphere< T >( RandomPoint3D< T >( g ), RandomValue< T >( g ) );
        box = CTAxisAlignedBox3D< T >( RandomPoint3D< T >( g ), RandomPoint3D< T >( g ) );
        triangle3 = CTTriangle3D< T >( RandomPoint3D< T >( g ), RandomPoint3D< T >( g ), RandomPoint3D< T >( g ) );
        capsule = CTCapsule< T >( RandomPoint3D< T >( g ), RandomPoint3D< T >( g ), RandomValue< T >( g ) );
        ray2 = CTRay2D< T >( RandomPoint2D< T >( g ), RandomDirection2D< T >( g ) );
        line2 = CTLine2D< T >( RandomPoint2D< T >( g ), RandomDirection2D< T >( g ) );
        segment2 = CTSegment2D< T >( RandomPoint2D< T >( g ), RandomPoint2D< T >( g ) );
        circle = CTCircle< T >( RandomPoint2D< T >( g ), RandomValue< T >( g ) );
        rectangle = CTRectangle< T >( RandomPoint2D< T >( g ), RandomPoint2D< T >( g ) );
        oriented = CTOrientedRectangle< T >( RandomPoint2D< T >( g ), CTRadian< T >( RandomValue< T >( g ) ), RandomVector2D< T >( g ) );
        triangle2 = CTTriangle2D< T >( RandomPoint2D< T >( g ), RandomPoint2D< T >( g ), RandomPoint2D< T >( g ) );
        const UInt32 count = 2 + g.NextUInt32() % 6;
        for ( UInt32 i=0; i<count; ++i )
        {
            polygon.AddPoint( RandomPoint2D< T >( g ) );
            polyline2.AddPoint( RandomPoint2D< T >( g ) );
            polyline3.AddPoint( RandomPoint3D< T >( g ) );
        }
        region.outer = polygon;
        const UInt32 holes = g.NextUInt32() % 3;
        for ( UInt32 h=0; h<holes; ++h )
        {
            CTPolygon2D< T > hole;
            for ( UInt32 i=0; i<3; ++i )
                hole.AddPoint( RandomPoint2D< T >( g ) );
            region.AddHole( hole );
        }
    }
};

/*-------------------------------------------------------------------------*/

/** ToString then FromString into a different default value: true when the value comes back equal */
template < typename TShape >
static bool
TextRoundTrip( const TShape& value )
{
    TShape back;
    return back.FromString( value.ToString() ) && back == value;
}

/*-------------------------------------------------------------------------*/

/** Serialize into a node then Deserialize into a default value: true when the value comes back equal */
template < typename TShape >
static bool
NodeRoundTrip( const TShape& value )
{
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode node( "value", GUCEF_DATATYPE_OBJECT );
    TShape back;
    return value.Serialize( node, settings ) && back.Deserialize( node, settings ) && back == value;
}

/*-------------------------------------------------------------------------*/

/** Text that must be refused and leave the shape unchanged */
template < typename TShape >
static UInt32
CountAcceptedMalformed( const TShape& original, const char* const* malformed, UInt32 count )
{
    UInt32 accepted = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TShape value = original;
        if ( value.FromString( malformed[ i ] ) || !( value == original ) )
        {
            ++accepted;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ShapeSerialization: accepted malformed text '" << malformed[ i ] << "'" );
        }
    }
    return accepted;
}

/*-------------------------------------------------------------------------*/

template < typename T >
static UInt32
CountRoundTripFailures( UInt64 seed, UInt32 count )
{
    CSeededSequence generator( seed );
    UInt32 failures = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const SShapeSet< T > s( generator );
        failures += TextRoundTrip( s.ray3 ) ? 0 : 1;
        failures += TextRoundTrip( s.line3 ) ? 0 : 1;
        failures += TextRoundTrip( s.segment3 ) ? 0 : 1;
        failures += TextRoundTrip( s.plane ) ? 0 : 1;
        failures += TextRoundTrip( s.sphere ) ? 0 : 1;
        failures += TextRoundTrip( s.box ) ? 0 : 1;
        failures += TextRoundTrip( s.triangle3 ) ? 0 : 1;
        failures += TextRoundTrip( s.capsule ) ? 0 : 1;
        failures += TextRoundTrip( s.ray2 ) ? 0 : 1;
        failures += TextRoundTrip( s.line2 ) ? 0 : 1;
        failures += TextRoundTrip( s.segment2 ) ? 0 : 1;
        failures += TextRoundTrip( s.circle ) ? 0 : 1;
        failures += TextRoundTrip( s.rectangle ) ? 0 : 1;
        failures += TextRoundTrip( s.oriented ) ? 0 : 1;
        failures += TextRoundTrip( s.triangle2 ) ? 0 : 1;
        failures += TextRoundTrip( s.polygon ) ? 0 : 1;
        failures += TextRoundTrip( s.region ) ? 0 : 1;
        failures += TextRoundTrip( s.polyline2 ) ? 0 : 1;
        failures += TextRoundTrip( s.polyline3 ) ? 0 : 1;

        failures += NodeRoundTrip( s.ray3 ) ? 0 : 1;
        failures += NodeRoundTrip( s.line3 ) ? 0 : 1;
        failures += NodeRoundTrip( s.segment3 ) ? 0 : 1;
        failures += NodeRoundTrip( s.plane ) ? 0 : 1;
        failures += NodeRoundTrip( s.sphere ) ? 0 : 1;
        failures += NodeRoundTrip( s.box ) ? 0 : 1;
        failures += NodeRoundTrip( s.triangle3 ) ? 0 : 1;
        failures += NodeRoundTrip( s.capsule ) ? 0 : 1;
        failures += NodeRoundTrip( s.ray2 ) ? 0 : 1;
        failures += NodeRoundTrip( s.line2 ) ? 0 : 1;
        failures += NodeRoundTrip( s.segment2 ) ? 0 : 1;
        failures += NodeRoundTrip( s.circle ) ? 0 : 1;
        failures += NodeRoundTrip( s.rectangle ) ? 0 : 1;
        failures += NodeRoundTrip( s.oriented ) ? 0 : 1;
        failures += NodeRoundTrip( s.triangle2 ) ? 0 : 1;
        failures += NodeRoundTrip( s.polygon ) ? 0 : 1;
        failures += NodeRoundTrip( s.region ) ? 0 : 1;
        failures += NodeRoundTrip( s.polyline2 ) ? 0 : 1;
        failures += NodeRoundTrip( s.polyline3 ) ? 0 : 1;
    }
    return failures;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformShapeSerializationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ShapeSerialization" );

    GUCEF_TESTFW_TESTCASE( "ShapeSerialization: text forms of the shapes" )
    try
    {
        typedef TFloat32Point3D P3;
        typedef TFloat32Point2D P2;
        ASSERT_TRUE( TFloat32Sphere( P3( 1, 2, 3 ), 2 ).ToString() == "Sphere(centre (1, 2, 3), radius 2)" );
        ASSERT_TRUE( TFloat32Plane().ToString() == "Plane(normal (0, 1, 0), offset 0)" );
        ASSERT_TRUE( TFloat32Ray3D().ToString() == "Ray3D(origin (0, 0, 0), direction (0, 0, -1))" );
        ASSERT_TRUE( TFloat32Line3D().ToString() == "Line3D(point (0, 0, 0), direction (0, 0, -1))" );
        ASSERT_TRUE( TFloat32Segment3D( P3( 0, 0, 0 ), P3( 1, 2, 3 ) ).ToString() == "Segment3D(start (0, 0, 0), end (1, 2, 3))" );
        ASSERT_TRUE( TFloat32AxisAlignedBox3D( P3( 0, 0, 0 ), P3( 1, 2, 3 ) ).ToString() == "AxisAlignedBox3D(min (0, 0, 0), max (1, 2, 3))" );
        ASSERT_TRUE( TFloat32Triangle3D( P3( 0, 0, 0 ), P3( 1, 0, 0 ), P3( 0, 1, 0 ) ).ToString() == "Triangle3D(a (0, 0, 0), b (1, 0, 0), c (0, 1, 0))" );
        ASSERT_TRUE( TFloat32Capsule( P3( 0, 0, 0 ), P3( 0, 4, 0 ), 1 ).ToString() == "Capsule(start (0, 0, 0), end (0, 4, 0), radius 1)" );
        ASSERT_TRUE( TFloat32Ray2D().ToString() == "Ray2D(origin (0, 0), direction (1, 0))" );
        ASSERT_TRUE( TFloat32Segment2D( P2( 0, 0 ), P2( 1, 2 ) ).ToString() == "Segment2D(start (0, 0), end (1, 2))" );
        ASSERT_TRUE( TFloat32Circle( P2( 1, 2 ), 3 ).ToString() == "Circle(centre (1, 2), radius 3)" );
        ASSERT_TRUE( TFloat32Rectangle( P2( 0, 0 ), P2( 1, 2 ) ).ToString() == "Rectangle(min (0, 0), max (1, 2))" );
        ASSERT_TRUE( TFloat32OrientedRectangle().ToString() == "OrientedRectangle(centre (0, 0), radians 0, halfExtents (0.5, 0.5))" );
        ASSERT_TRUE( TFloat32Triangle2D( P2( 0, 0 ), P2( 1, 0 ), P2( 0, 1 ) ).ToString() == "Triangle2D(a (0, 0), b (1, 0), c (0, 1))" );
        TFloat32Polygon2D polygon;
        ASSERT_TRUE( polygon.ToString() == "Polygon2D()" );
        polygon.AddPoint( P2( 0, 0 ) );
        polygon.AddPoint( P2( 1, 0 ) );
        polygon.AddPoint( P2( 1, 1 ) );
        ASSERT_TRUE( polygon.ToString() == "Polygon2D((0, 0), (1, 0), (1, 1))" );
        TFloat32PolygonWithHoles2D region;
        region.outer = polygon;
        region.AddHole( polygon );
        ASSERT_TRUE( region.ToString() == "PolygonWithHoles2D(outer Polygon2D((0, 0), (1, 0), (1, 1)), hole Polygon2D((0, 0), (1, 0), (1, 1)))" );
        TFloat32Polyline3D line;
        line.AddPoint( P3( 1, 2, 3 ) );
        ASSERT_TRUE( line.ToString() == "Polyline3D((1, 2, 3))" );
        ASSERT_TRUE( TInt32IntegerRect( 0, 0, 4, 3 ).ToString() == "IntegerRect(position (0, 0), size (4, 3))" );
        ASSERT_TRUE( TInt32IntegerBox3D( TInt32Point3D( -1, 2, 3 ), TInt32Vector3D( 4, 5, 6 ) ).ToString() == "IntegerBox3D(position (-1, 2, 3), size (4, 5, 6))" );

        // white space is accepted between the tokens
        TFloat32Sphere sphere;
        ASSERT_TRUE( sphere.FromString( "  Sphere ( centre ( 1 ,2,3 ) , radius  2 )  \n" ) && sphere == TFloat32Sphere( P3( 1, 2, 3 ), 2 ) );
        TFloat32PolygonWithHoles2D parsed;
        ASSERT_TRUE( parsed.FromString( region.ToString() ) && parsed == region );
        ASSERT_TRUE( parsed.FromString( " PolygonWithHoles2D ( outer Polygon2D ( ( 0 , 0 ) , ( 1 , 0 ) , ( 1 , 1 ) ) ) " ) && 0 == parsed.GetHoleCount() && 3 == parsed.outer.GetPointCount() );
        TFloat32Polygon2D emptyPolygon;
        ASSERT_TRUE( polygon.FromString( "Polygon2D()" ) && polygon == emptyPolygon );
        TFloat32AxisAlignedBox3D box;
        ASSERT_TRUE( box.FromString( "AxisAlignedBox3D(min (1e38, -1e-38, 0), max (3.4028235e38, 0, 0.1))" ) && box.min.x == 1e38f && box.max.x == 3.4028235e38f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ShapeSerialization: text and DataNode round trips of every shape are exact, seeded values of the whole range" )
    try
    {
        const UInt64 seed = 0x5E2141;
        const UInt32 failures32 = CountRoundTripFailures< Float32 >( seed, 400 );
        const UInt32 failures64 = CountRoundTripFailures< Float64 >( seed + 1, 400 );
        if ( failures32 > 0 || failures64 > 0 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ShapeSerialization: round trip failures Float32 " << failures32 << " Float64 " << failures64 << ", seed " << seed );
        ASSERT_TRUE( 0 == failures32 && 0 == failures64 );

        CSeededSequence generator( seed + 2 );
        UInt32 failuresInt = 0;
        for ( UInt32 i=0; i<500; ++i )
        {
            const TInt32IntegerRect rect( RandomPoint2D< Int32 >( generator ), RandomVector2D< Int32 >( generator ) );
            const TInt32IntegerBox3D box( RandomPoint3D< Int32 >( generator ), RandomVector3D< Int32 >( generator ) );
            failuresInt += TextRoundTrip( rect ) && NodeRoundTrip( rect ) && TextRoundTrip( box ) && NodeRoundTrip( box ) ? 0 : 1;
        }
        ASSERT_TRUE( 0 == failuresInt );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ShapeSerialization: malformed text is rejected and leaves the shape unchanged" )
    try
    {
        typedef TFloat32Point3D P3;
        const char* sphereText[] =
        {
            "", " ", "Sphere", "Sphere()", "Sphere(centre (1, 2, 3))", "Sphere(centre (1, 2), radius 2)", "Sphere(centre (1, 2, 3), radius)",
            "Sphere(centre (1, 2, 3), radius 2) x", "x Sphere(centre (1, 2, 3), radius 2)", "sphere(centre (1, 2, 3), radius 2)",
            "Sphere(radius 2, centre (1, 2, 3))", "Sphere(centre (a, 2, 3), radius 2)", "Sphere(centre (1, 2, 3), radius 2",
            "Sphere(centre (1, 2, 3) radius 2)", "Sphere(centre (1, 2, 3), radius 2, 3)", "Sphere(centre 1, 2, 3, radius 2)",
            "Sphere(centre (1, 2, 3), radius 2))", "Circle(centre (1, 2), radius 2)", "Sphere(centre (1, 2, 3), radius 0x10)"
        };
        ASSERT_TRUE( 0 == CountAcceptedMalformed( TFloat32Sphere( P3( 7, 8, 9 ), 5 ), sphereText, sizeof( sphereText ) / sizeof( sphereText[ 0 ] ) ) );

        const char* planeText[] = { "Plane(normal (1, 1, 0), offset 1)", "Plane(normal (0, 0, 0), offset 1)", "Plane(normal (0, 1, 0))", "Plane(offset 1, normal (0, 1, 0))" };
        TFloat32Plane plane;
        ASSERT_TRUE( 0 == CountAcceptedMalformed( plane, planeText, 4 ) );

        const char* polygonText[] = { "Polygon2D((0, 0), (1, 0)", "Polygon2D((0, 0) (1, 0))", "Polygon2D(0, 0)", "Polygon2D((0, 0), )", "Polygon2D((0, 0, 0))", "Polygon2D", "Polygon2D((0, 0)), x", "Polyline2D((0, 0))" };
        TFloat32Polygon2D polygon;
        polygon.AddPoint( TFloat32Point2D( 5, 6 ) );
        ASSERT_TRUE( 0 == CountAcceptedMalformed( polygon, polygonText, sizeof( polygonText ) / sizeof( polygonText[ 0 ] ) ) );

        const char* regionText[] = { "PolygonWithHoles2D(outer Polygon2D(), hole)", "PolygonWithHoles2D(hole Polygon2D())", "PolygonWithHoles2D(outer Polygon2D(), hole Circle(centre (0, 0), radius 1))", "PolygonWithHoles2D(outer Polygon2D()", "PolygonWithHoles2D(outer Polygon2D(),)" };
        TFloat32PolygonWithHoles2D region;
        region.outer = polygon;
        ASSERT_TRUE( 0 == CountAcceptedMalformed( region, regionText, sizeof( regionText ) / sizeof( regionText[ 0 ] ) ) );

        const char* rectText[] = { "IntegerRect(position (0, 0), size (1.5, 2))", "IntegerRect(position (0, 0), size (99999999999, 2))", "IntegerRect(position (0, 0, 0), size (1, 2))", "IntegerRect(size (1, 2), position (0, 0))", "IntegerBox3D(position (0, 0), size (1, 2))" };
        ASSERT_TRUE( 0 == CountAcceptedMalformed( TInt32IntegerRect( 1, 2, 3, 4 ), rectText, sizeof( rectText ) / sizeof( rectText[ 0 ] ) ) );

        const char* orientedText[] = { "OrientedRectangle(centre (0, 0), halfExtents (1, 1), radians 0)", "OrientedRectangle(centre (0, 0), radians 1 rad, halfExtents (1, 1))", "OrientedRectangle(centre (0, 0), radians 0)" };
        ASSERT_TRUE( 0 == CountAcceptedMalformed( TFloat32OrientedRectangle(), orientedText, 3 ) );

        const char* boxText[] = { "AxisAlignedBox3D(min (0, 0, 0))", "AxisAlignedBox3D(min (0, 0, 0), max (1, 1))", "AxisAlignedBox3D(max (1, 1, 1), min (0, 0, 0))" };
        ASSERT_TRUE( 0 == CountAcceptedMalformed( TFloat32AxisAlignedBox3D( P3( 1, 2, 3 ), P3( 4, 5, 6 ) ), boxText, 3 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ShapeSerialization: DataNode forms have the member names and fail on missing members without changing the shape" )
    try
    {
        typedef TFloat64Point3D P3;
        CORE::CDataNodeSerializableSettings settings;

        const TFloat64Sphere sphere( P3( 1, 2, 3 ), 2 );
        CORE::CDataNode node( "value", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( sphere.Serialize( node, settings ) );
        const CORE::CDataNode* centre = node.FindChild( "centre" );
        ASSERT_TRUE( GUCEF_NULL != centre && GUCEF_NULL != centre->GetAttribute( "x" ) && GUCEF_NULL != centre->GetAttribute( "y" ) && GUCEF_NULL != centre->GetAttribute( "z" ) );
        ASSERT_TRUE( GUCEF_NULL != node.GetAttribute( "radius" ) );

        TFloat64Sphere target( P3( 9, 9, 9 ), 9 );
        CORE::CDataNode empty( "value", GUCEF_DATATYPE_OBJECT );
        ASSERT_FALSE( target.Deserialize( empty, settings ) );
        ASSERT_TRUE( target == TFloat64Sphere( P3( 9, 9, 9 ), 9 ) );
        CORE::CDataNode partial( "value", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( partial.AddChild( "centre", GUCEF_DATATYPE_OBJECT ) != GUCEF_NULL );
        ASSERT_TRUE( P3( 1, 2, 3 ).Serialize( *partial.FindChild( "centre" ), settings ) );
        ASSERT_FALSE( target.Deserialize( partial, settings ) );                    // no radius
        ASSERT_TRUE( target == TFloat64Sphere( P3( 9, 9, 9 ), 9 ) );
        ASSERT_TRUE( target.Deserialize( node, settings ) && target == sphere );

        // a polygon is a child node points with one child node point per vertex
        TFloat64Polygon2D polygon;
        polygon.AddPoint( TFloat64Point2D( 0, 0 ) );
        polygon.AddPoint( TFloat64Point2D( 1, 0 ) );
        polygon.AddPoint( TFloat64Point2D( 1, 1 ) );
        CORE::CDataNode polygonNode( "value", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( polygon.Serialize( polygonNode, settings ) );
        const CORE::CDataNode* points = polygonNode.FindChild( "points" );
        ASSERT_TRUE( GUCEF_NULL != points && 3 == points->GetNrOfDirectChildNodes() );
        TFloat64Polygon2D readBack;
        ASSERT_TRUE( readBack.Deserialize( polygonNode, settings ) && readBack == polygon );
        TFloat64Polygon2D keep = polygon;
        ASSERT_FALSE( keep.Deserialize( empty, settings ) );
        ASSERT_TRUE( keep == polygon );
        // a vertex without its y attribute fails the whole polygon
        CORE::CDataNode broken( "value", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* list = broken.AddChild( "points", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( GUCEF_NULL != list );
        CORE::CDataNode* point = list->AddChild( "point", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( GUCEF_NULL != point );
        point->SetAttribute( CORE::CString( "x" ), CORE::CVariant( (Float64) 1 ), false );
        ASSERT_FALSE( keep.Deserialize( broken, settings ) );
        ASSERT_TRUE( keep == polygon );

        // a direction that is not of unit length is refused
        TFloat64Ray3D ray;
        CORE::CDataNode rayNode( "value", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( ray.Serialize( rayNode, settings ) );
        CORE::CDataNode* direction = rayNode.FindChild( "direction" );
        ASSERT_TRUE( GUCEF_NULL != direction );
        direction->SetAttribute( CORE::CString( "x" ), CORE::CVariant( (Float64) 5 ), false );
        TFloat64Ray3D unchanged;
        ASSERT_FALSE( unchanged.Deserialize( rayNode, settings ) );
        ASSERT_TRUE( unchanged == TFloat64Ray3D() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ShapeSerialization: JSON round trips of shapes with nested nodes and lists" )
    GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "the JSON codec plugin is not available in this build" );
    try
    {
        typedef TFloat32Point3D P3;
        TFloat32Sphere sphere;
        ASSERT_TRUE( JsonRoundTrip( TFloat32Sphere( P3( 0.1f, -2.5f, 3e10f ), 0.3f ), "centre", sphere ) );
        TFloat64AxisAlignedBox3D box;
        ASSERT_TRUE( JsonRoundTrip( TFloat64AxisAlignedBox3D( TFloat64Point3D( 0.1, -2.5, 3e10 ), TFloat64Point3D( 1, 2, 3e20 ) ), "min", box ) );
        TFloat32Plane plane;
        ASSERT_TRUE( JsonRoundTrip( TFloat32Plane( TFloat32DirectionVector3D::UnitX(), 7.25f ), "normal", plane ) );
        TInt32IntegerRect rect;
        ASSERT_TRUE( JsonRoundTrip( TInt32IntegerRect( -3, 5, 7, 11 ), "position", rect ) );

        TFloat32Polygon2D polygon;
        polygon.AddPoint( TFloat32Point2D( 0, 0 ) );
        polygon.AddPoint( TFloat32Point2D( 1.5f, 0 ) );
        polygon.AddPoint( TFloat32Point2D( 1, 1.25f ) );
        polygon.AddPoint( TFloat32Point2D( 0, 1 ) );
        TFloat32Polygon2D polygonBack;
        ASSERT_TRUE( JsonRoundTrip( polygon, "points", polygonBack ) );

        TFloat32PolygonWithHoles2D region;
        region.outer = polygon;
        region.AddHole( polygon );
        region.AddHole( polygon );
        TFloat32PolygonWithHoles2D regionBack;
        ASSERT_TRUE( JsonRoundTrip( region, "outer", regionBack ) );
        TFloat32Polyline3D line;
        line.AddPoint( TFloat32Point3D( 1, 2, 3 ) );
        line.AddPoint( TFloat32Point3D( -4, 5, 6.5f ) );
        TFloat32Polyline3D lineBack;
        ASSERT_TRUE( JsonRoundTrip( line, "points", lineBack ) );
        TFloat32Capsule capsule( TFloat32Point3D( 1, 2, 3 ), TFloat32Point3D( 4, 5, 6 ), 0.5f );
        TFloat32Capsule capsuleBack;
        ASSERT_TRUE( JsonRoundTrip( capsule, "start", capsuleBack ) );
        TFloat32OrientedRectangle oriented( TFloat32Point2D( 1, 2 ), CTRadian< Float32 >( 0.75f ), TFloat32Vector2D( 3, 4 ) );
        TFloat32OrientedRectangle orientedBack;
        ASSERT_TRUE( JsonRoundTrip( oriented, "halfExtents", orientedBack ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ShapeSerialization: interop of rectangles and boxes with CORE::TArea2D and CORE::TArea3D" )
    try
    {
        GUCEF_GEOMETRY_STATIC_ASSERT( sizeof( CORE::TArea2D ) == 4 * sizeof( Float32 ), TArea2D_is_four_floats );
        GUCEF_GEOMETRY_STATIC_ASSERT( sizeof( CORE::TArea3D ) == 6 * sizeof( Float32 ), TArea3D_is_six_floats );

        CORE::TArea2D area2;
        area2.min[ 0 ] = -1.5f;
        area2.min[ 1 ] = 2.0f;
        area2.max[ 0 ] = 3.25f;
        area2.max[ 1 ] = 9.0f;
        const TFloat32Rectangle rectangle = ToShape( area2 );
        ASSERT_TRUE( rectangle.min == TFloat32Point2D( -1.5f, 2.0f ) && rectangle.max == TFloat32Point2D( 3.25f, 9.0f ) );
        const CORE::TArea2D back2 = ToCore( rectangle );
        ASSERT_TRUE( 0 == memcmp( &area2, &back2, sizeof( area2 ) ) );

        CORE::TArea3D area3;
        area3.min[ 0 ] = -1.5f; area3.min[ 1 ] = 2.0f; area3.min[ 2 ] = -7.0f;
        area3.max[ 0 ] = 3.25f; area3.max[ 1 ] = 9.0f; area3.max[ 2 ] = 0.125f;
        const TFloat32AxisAlignedBox3D box = ToShape( area3 );
        ASSERT_TRUE( box.min == TFloat32Point3D( -1.5f, 2.0f, -7.0f ) && box.max == TFloat32Point3D( 3.25f, 9.0f, 0.125f ) );
        const CORE::TArea3D back3 = ToCore( box );
        ASSERT_TRUE( 0 == memcmp( &area3, &back3, sizeof( area3 ) ) );

        // the empty rectangle and box keep min above max across the conversion
        const CORE::TArea2D emptyArea = ToCore( TFloat32Rectangle() );
        ASSERT_TRUE( emptyArea.min[ 0 ] > emptyArea.max[ 0 ] && ToShape( emptyArea ).IsEmpty() );
        ASSERT_TRUE( ToShape( ToCore( TFloat32AxisAlignedBox3D() ) ).IsEmpty() );

        // seeded values of the whole range, bit for bit
        CSeededSequence generator( 0xA2EA );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            const TFloat32Rectangle r( RandomPoint2D< Float32 >( generator ), RandomPoint2D< Float32 >( generator ) );
            const TFloat32AxisAlignedBox3D b( RandomPoint3D< Float32 >( generator ), RandomPoint3D< Float32 >( generator ) );
            const CORE::TArea2D a2 = ToCore( r );
            const CORE::TArea3D a3 = ToCore( b );
            if ( !( ToShape( a2 ) == r ) || !( ToShape( a3 ) == b ) || a2.min[ 0 ] != r.min.x || a3.max[ 2 ] != b.max.z )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
