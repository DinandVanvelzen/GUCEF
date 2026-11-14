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

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#include "gucefMATH_fractiontypes.h"
#define GUCEF_MATH_FRACTIONTYPES_H
#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#include "TestMathReference.h"
#include "TestVectorSerialization.h"

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
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat32Vector3D >::value, TFloat32Vector3D_has_Serialize );
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeDeserialization< TFloat32Vector3D >::value, TFloat32Vector3D_has_Deserialize );

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

/** The first node of the tree (depth first, the root included) that has an attribute 'name' */
static const CORE::CDataNode*
FindNodeWithAttribute( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    if ( GUCEF_NULL != node.GetAttribute( name ) )
        return &node;
    CORE::CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        const CORE::CDataNode* found = FindNodeWithAttribute( **i, name );
        if ( GUCEF_NULL != found )
            return found;
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** Serialize, write JSON text, parse it and deserialize: true when the value comes back bit for bit */
template < typename TValue >
static bool
JsonRoundTrip( const TValue& value, TValue& back )
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
    const CORE::CDataNode* holder = FindNodeWithAttribute( parsed, "x" );
    return GUCEF_NULL != holder && back.Deserialize( *holder, settings );
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
RandomFiniteFloat32( MathReference::CSeededSequence& generator )
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
RandomFiniteFloat64( MathReference::CSeededSequence& generator )
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVectorSerializationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "VectorSerialization" );

    GUCEF_TESTFW_TESTCASE( "VectorSerialization: text forms of every kind" )
    try
    {
        ASSERT_TRUE( TFloat32Vector3D( 1, -2.5f, 0.1f ).ToString() == "(1, -2.5, 0.1)" );
        ASSERT_TRUE( TFloat64Vector2D( 0.1, 1e300 ).ToString() == "(0.1, 1e+300)" );
        ASSERT_TRUE( TInt32Vector4D( 1, -2, 3, 2147483647 ).ToString() == "(1, -2, 3, 2147483647)" );
        ASSERT_TRUE( TUInt8Point2D( 0, 255 ).ToString() == "(0, 255)" );
        ASSERT_TRUE( TFixed32Vector3D( TFixed32::FromRaw( 1 ), TFixed32( -3 ), TFixed32::FromRaw( 98304 ) ).ToString() == "(0.0000152587890625, -3, 1.5)" );
        ASSERT_TRUE( TFloat32DirectionVector3D::Forward().ToString() == "(0, 0, -1)" );
        ASSERT_TRUE( TFloat32Vector3D::Forward().ToString() == "(0, 0, -1)" && TFloat32Vector2D::Left().ToString() == "(-1, 0)" );

        TFloat32Vector3D v;
        ASSERT_TRUE( v.FromString( " ( 1 ,\t-2.5 ,0.1 )\n" ) && v == TFloat32Vector3D( 1, -2.5f, 0.1f ) );
        ASSERT_TRUE( v.FromString( "(nan, inf, -inf)" ) && !( v.x == v.x ) && !v.IsFinite() );

        typedef CORE::TInt64Fraction F;
        TInt64FractionVector3D q( F( 1, 3 ), F( -7, 2 ), F( 5 ) );
        TInt64FractionVector3D qBack;
        ASSERT_TRUE( qBack.FromString( q.ToString() ) && qBack == q );
        TBigFractionPoint3D b( TMathBigFraction( 1, 3 ), TMathBigFraction( -7, 2 ), TMathBigFraction( 5 ) );
        TBigFractionPoint3D bBack;
        ASSERT_TRUE( bBack.FromString( b.ToString() ) && bBack == b );
        TFixed64Point2D f( TFixed64::FromRaw( -1 ), TFixed64( 123456 ) );
        TFixed64Point2D fBack;
        ASSERT_TRUE( fBack.FromString( f.ToString() ) && fBack == f );
        TInt64Vector2D i( (Int64) 0x8000000000000000ULL, 9 );
        TInt64Vector2D iBack;
        ASSERT_TRUE( iBack.FromString( i.ToString() ) && iBack == i );
        TFloat64Vector4D w( 1, 2, 3, 4 );
        TFloat64Vector4D wBack;
        ASSERT_TRUE( wBack.FromString( w.ToString() ) && wBack == w );

        TFloat32DirectionVector3D d = TFloat32DirectionVector3D::Up();
        ASSERT_TRUE( d.FromString( "(1, 0, 0)" ) && d == TFloat32DirectionVector3D::UnitX() );
        ASSERT_FALSE( d.FromString( "(1, 1, 0)" ) );
        ASSERT_FALSE( d.FromString( "(0, 0, 0)" ) );
        ASSERT_TRUE( d == TFloat32DirectionVector3D::UnitX() );
        TFloat32DirectionVector2D d2;
        ASSERT_TRUE( d2.FromString( "(0, -1)" ) && d2 == TFloat32DirectionVector2D::Down() );
        ASSERT_FALSE( d2.FromString( "(0, -2)" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VectorSerialization: malformed text is rejected and leaves the value unchanged" )
    try
    {
        const char* malformed[] =
        {
            "", " ", "1, 2, 3", "(1, 2)", "(1, 2, 3, 4)", "(1, 2, 3", "1, 2, 3)", "(1,, 3)", "(, 2, 3)", "(1, 2, )",
            "(a, 2, 3)", "(1, 2, 3) x", "x (1, 2, 3)", "(1 2, 3)", "[1, 2, 3]", "(1, 2, 3))", "((1, 2, 3)", "(1; 2; 3)",
            "(0x10, 2, 3)", "(1e, 2, 3)"
        };
        const TFloat32Vector3D original( 7, 8, 9 );
        UInt32 accepted = 0;
        for ( UInt32 i=0; i<sizeof( malformed ) / sizeof( malformed[ 0 ] ); ++i )
        {
            TFloat32Vector3D v = original;
            if ( v.FromString( malformed[ i ] ) || v != original )
            {
                ++accepted;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VectorSerialization: accepted malformed text '" << malformed[ i ] << "'" );
            }
        }
        ASSERT_TRUE( 0 == accepted );

        TInt32Vector2D integer( 1, 2 );
        ASSERT_FALSE( integer.FromString( "(1.5, 2)" ) );
        ASSERT_FALSE( integer.FromString( "(1, 99999999999)" ) );
        ASSERT_TRUE( integer == TInt32Vector2D( 1, 2 ) );
        TUInt8Vector2D small( 1, 2 );
        ASSERT_FALSE( small.FromString( "(1, 256)" ) );
        ASSERT_FALSE( small.FromString( "(-1, 2)" ) );
        ASSERT_TRUE( small == TUInt8Vector2D( 1, 2 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VectorSerialization: 10^4 seeded float values round-trip through text bit for bit" )
    try
    {
        const UInt64 seed = 77777;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            TFloat32Vector3D v( RandomFiniteFloat32( generator ), RandomFiniteFloat32( generator ), RandomFiniteFloat32( generator ) );
            TFloat32Vector3D back;
            if ( !back.FromString( v.ToString() ) || !SameBits( v, back ) )
                ++failures;
            TFloat64Vector3D w( RandomFiniteFloat64( generator ), RandomFiniteFloat64( generator ), RandomFiniteFloat64( generator ) );
            TFloat64Vector3D wBack;
            if ( !wBack.FromString( w.ToString() ) || !SameBits( w, wBack ) )
                ++failures;
            TFloat32Point2D p( RandomFiniteFloat32( generator ), RandomFiniteFloat32( generator ) );
            TFloat32Point2D pBack;
            if ( !pBack.FromString( p.ToString() ) || !SameBits( p, pBack ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VectorSerialization: text round trip seed " << seed << " failures " << failures );
        TFloat32Vector3D negativeZero( -0.0f, 0.0f, -0.0f );
        TFloat32Vector3D back;
        ASSERT_TRUE( back.FromString( negativeZero.ToString() ) && SameBits( back, negativeZero ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VectorSerialization: DataNode attributes per kind, round trip, missing and invalid attributes" )
    try
    {
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat32Vector3D >::value );
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeDeserialization< TFloat32Vector3D >::value );
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat64Vector2D >::value );
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeSerialization< TInt32Vector4D >::value );
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat32Point3D >::value );
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeDeserialization< TFloat32Point2D >::value );
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFloat32DirectionVector3D >::value );
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeDeserialization< TFloat32DirectionVector2D >::value );
        ASSERT_TRUE( CORE::TypeHasMemberFunctionForDataNodeSerialization< TFixed32Vector3D >::value );

        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node( "position", GUCEF_DATATYPE_OBJECT );
        TFloat32Vector3D v( 1.5f, -2.0f, 0.1f );
        ASSERT_TRUE( v.Serialize( node, settings ) );
        ASSERT_TRUE( GUCEF_NULL != node.GetAttribute( "x" ) && GUCEF_NULL != node.GetAttribute( "y" ) && GUCEF_NULL != node.GetAttribute( "z" ) );
        ASSERT_TRUE( node.GetAttribute( "x" )->second.IsFloat() && 0.1f == node.GetAttribute( "z" )->second.AsFloat32() );
        TFloat32Vector3D back;
        ASSERT_TRUE( back.Deserialize( node, settings ) && back == v );

        CORE::CDataNode node64( "v", GUCEF_DATATYPE_OBJECT );
        TFloat64Vector4D v64( 0.1, 0.2, 1e-300, -4 );
        TFloat64Vector4D back64;
        ASSERT_TRUE( v64.Serialize( node64, settings ) && GUCEF_NULL != node64.GetAttribute( "w" ) && back64.Deserialize( node64, settings ) && back64 == v64 );

        CORE::CDataNode intNode( "i", GUCEF_DATATYPE_OBJECT );
        TInt64Point2D i( (Int64) 0x8000000000000000ULL, 42 );
        TInt64Point2D iBack;
        ASSERT_TRUE( i.Serialize( intNode, settings ) && intNode.GetAttribute( "x" )->second.IsInteger() );
        ASSERT_TRUE( iBack.Deserialize( intNode, settings ) && iBack == i );

        CORE::CDataNode fixedNode( "f", GUCEF_DATATYPE_OBJECT );
        TFixed32Vector2D f( TFixed32::FromRaw( 1 ), TFixed32( -3 ) );
        TFixed32Vector2D fBack;
        ASSERT_TRUE( f.Serialize( fixedNode, settings ) && fixedNode.GetAttribute( "x" )->second.IsString() );
        ASSERT_TRUE( fixedNode.GetAttribute( "x" )->second.AsString() == "0.0000152587890625" );
        ASSERT_TRUE( fBack.Deserialize( fixedNode, settings ) && fBack == f );

        CORE::CDataNode fractionNode( "q", GUCEF_DATATYPE_OBJECT );
        typedef CORE::TInt64Fraction F;
        TInt64FractionVector3D q( F( 1, 3 ), F( -7, 2 ), F( 5 ) );
        TInt64FractionVector3D qBack;
        ASSERT_TRUE( q.Serialize( fractionNode, settings ) && fractionNode.GetAttribute( "y" )->second.IsString() );
        ASSERT_TRUE( qBack.Deserialize( fractionNode, settings ) && qBack == q );

        CORE::CDataNode directionNode( "d", GUCEF_DATATYPE_OBJECT );
        TFloat32DirectionVector3D d = TFloat32DirectionVector3D::Left();
        TFloat32DirectionVector3D dBack;
        ASSERT_TRUE( d.Serialize( directionNode, settings ) && dBack.Deserialize( directionNode, settings ) && dBack == d );
        directionNode.SetAttribute( "x", CORE::CVariant( 2.0f ) );
        ASSERT_FALSE( dBack.Deserialize( directionNode, settings ) );
        ASSERT_TRUE( dBack == d );

        // a missing or invalid attribute fails and leaves the value unchanged
        CORE::CDataNode partial( "p", GUCEF_DATATYPE_OBJECT );
        partial.SetAttribute( "x", CORE::CVariant( 1.0f ) );
        partial.SetAttribute( "y", CORE::CVariant( 2.0f ) );
        TFloat32Vector3D unchanged( 7, 8, 9 );
        ASSERT_FALSE( unchanged.Deserialize( partial, settings ) );
        ASSERT_TRUE( unchanged == TFloat32Vector3D( 7, 8, 9 ) );
        partial.SetAttribute( "z", CORE::CVariant( CORE::CString( "not a number" ) ) );
        ASSERT_FALSE( unchanged.Deserialize( partial, settings ) );
        ASSERT_TRUE( unchanged == TFloat32Vector3D( 7, 8, 9 ) );
        partial.SetAttribute( "z", CORE::CVariant( CORE::CString( "3.5" ) ) );
        ASSERT_TRUE( unchanged.Deserialize( partial, settings ) && unchanged == TFloat32Vector3D( 1, 2, 3.5f ) );
        TInt32Point3D intUnchanged( 4, 5, 6 );
        partial.SetAttribute( "z", CORE::CVariant( CORE::CString( "3.5" ) ) );
        ASSERT_FALSE( intUnchanged.Deserialize( partial, settings ) );
        ASSERT_TRUE( intUnchanged == TInt32Point3D( 4, 5, 6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VectorSerialization: DataNode round trip through JSON text" )
    GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "the JSON codec plugin dstorepluginJSONPARSER is not available next to the module" );
    try
    {
        // Float values with at most 6 significant digits come back bit for bit
        TFloat32Vector3D exact32( 1.5f, -0.25f, 1024.0f );
        TFloat32Vector3D exact32Back;
        ASSERT_TRUE( JsonRoundTrip( exact32, exact32Back ) && SameBits( exact32, exact32Back ) );
        TFloat64Point2D exact64( 0.1, -123456.0 );
        TFloat64Point2D exact64Back;
        ASSERT_TRUE( JsonRoundTrip( exact64, exact64Back ) && SameBits( exact64, exact64Back ) );
        TFloat32Vector4D exact4( 0.6f, 0.8f, -3.0f, 1e-20f );
        TFloat32Vector4D exact4Back;
        ASSERT_TRUE( JsonRoundTrip( exact4, exact4Back ) && SameBits( exact4, exact4Back ) );

        // Arbitrary floats: the JSON codec writes numbers with printf "%g", 6 significant digits
        // (dependencies/json-builder/json-builder.c), so they come back within a relative 5e-6 until that is changed
        const UInt64 seed = 4711;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            TFloat32Vector3D v( RandomFiniteFloat32( generator ), RandomFiniteFloat32( generator ), RandomFiniteFloat32( generator ) );
            TFloat32Vector3D back;
            if ( !JsonRoundTrip( v, back ) || !TFloat64Vector3D( back ).Equals( TFloat64Vector3D( v ), 1e-300, 5e-6 ) )
                ++failures;
            TFloat64Point2D p( generator.NextFloat64( -1e6, 1e6 ), generator.NextFloat64( -1e-6, 1e-6 ) );
            TFloat64Point2D pBack;
            if ( !JsonRoundTrip( p, pBack ) || !pBack.Equals( p, 1e-300, 5e-6 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VectorSerialization: JSON round trip seed " << seed << " failures " << failures );

        TInt32Vector4D i( -1, 2, (Int32) 0x80000000u, 2147483647 );
        TInt32Vector4D iBack;
        ASSERT_TRUE( JsonRoundTrip( i, iBack ) && iBack == i );
        TFixed64Vector3D f( TFixed64::FromRaw( 1 ), TFixed64( -3 ), TFixed64::FromRaw( (Int64) 0x7FFFFFFFFFFFFFFFULL ) );
        TFixed64Vector3D fBack;
        ASSERT_TRUE( JsonRoundTrip( f, fBack ) && fBack == f );
        typedef CORE::TInt64Fraction F;
        TInt64FractionPoint3D q( F( 1, 3 ), F( -7, 2 ), F( 5 ) );
        TInt64FractionPoint3D qBack;
        ASSERT_TRUE( JsonRoundTrip( q, qBack ) && qBack == q );
        TFloat32DirectionVector3D d = TFloat32DirectionVector3D::FromNormalized( TFloat32Vector3D( 0.6f, 0, -0.8f ) );
        TFloat32DirectionVector3D dBack;
        ASSERT_TRUE( JsonRoundTrip( d, dBack ) && dBack == d );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
