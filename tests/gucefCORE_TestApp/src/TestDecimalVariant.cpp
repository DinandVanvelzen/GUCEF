/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <string.h>

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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#include "gucefCORE_CVariantBinarySerializer.h"
#define GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CVARIANTBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#include "gucefCORE_CDataNodeBinarySerializer.h"
#define GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CDATANODEBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_CUTF16STRING_H
#include "gucefCORE_CUtf16String.h"
#define GUCEF_CORE_CUTF16STRING_H
#endif /* GUCEF_CORE_CUTF16STRING_H ? */

#ifndef GUCEF_CORE_CUTF32STRING_H
#include "gucefCORE_CUtf32String.h"
#define GUCEF_CORE_CUTF32STRING_H
#endif /* GUCEF_CORE_CUTF32STRING_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDecimalVariant.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

typedef CORE::CDecimal  CDecimal;
typedef CORE::CVariant  CVariant;
typedef CORE::CDStoreCodecRegistry::TDStoreCodecPtr TCodecPtr;

#if ( GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN )
static const UInt8 g_nativeDecimalId = GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL;
static const UInt8 g_foreignDecimalId = GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL;
#else
static const UInt8 g_nativeDecimalId = GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL;
static const UInt8 g_foreignDecimalId = GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL;
#endif

/*-------------------------------------------------------------------------*/

static CDecimal
D( const char* text )
{
    return CDecimal::Parse( text );
}

/*-------------------------------------------------------------------------*/

static bool
Identical( const CVariant& var, const CDecimal& expected )
{
    CDecimal value;
    if ( CORE::DecimalStatusIsError( var.GetDecimal( value ) ) )
        return false;
    return value.IsIdentical( expected );
}

/*-------------------------------------------------------------------------*/

/** The 9 payload bytes of a decimal in the given byte order, written byte by byte */
static void
DecimalPayload( Int64 unscaled, UInt8 scale, bool bigEndian, UInt8* bytes )
{
    UInt64 bits = (UInt64) unscaled;
    for ( UInt32 i=0; i<8; ++i )
    {
        UInt8 byte = (UInt8) ( ( bits >> ( 8 * i ) ) & 0xFF );
        bytes[ bigEndian ? 7 - i : i ] = byte;
    }
    bytes[ 8 ] = scale;
}

/*-------------------------------------------------------------------------*/

static bool
GetCodec( const char* codecType, const char* pluginName, TCodecPtr& codec )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    if ( coreGlobal->GetDStoreCodecRegistry().TryLookup( codecType, codec, false ) )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    CORE::CString secondPluginPath = CORE::CString( "$MODULEDIR$/" ) + pluginName;
    #else
    CORE::CString firstPluginPath = CORE::CString( "$MODULEDIR$/" ) + pluginName;
    CORE::CString secondPluginPath = CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );

    return coreGlobal->GetDStoreCodecRegistry().TryLookup( codecType, codec, false );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
BufferText( const CORE::CDynamicBuffer& buffer )
{
    return CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
PerformDecimalVariantTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CDecimal CVariant and serializer TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CDecimalVariant" );

    GUCEF_TESTFW_TESTCASE( "Test: type ids, names and fixed sizes" )
        try
        {
            ASSERT_TRUE( 72 == GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL && 73 == GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL );
            ASSERT_TRUE( g_nativeDecimalId == GUCEF_DATATYPE_INT64_UINT8_DECIMAL );
            ASSERT_TRUE( 0 == strcmp( "INT64_UINT8_LE_DECIMAL", CORE::TypeNameForGucefTypeId( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL ) ) );
            ASSERT_TRUE( 0 == strcmp( "INT64_UINT8_BE_DECIMAL", CORE::TypeNameForGucefTypeId( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL ) ) );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL == CORE::GucefTypeIdForTypeName( "INT64_UINT8_LE_DECIMAL" ) );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL == CORE::GucefTypeIdForTypeName( GUCEF_DATATYPE_NAME_INT64_UINT8_BE_DECIMAL ) );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_DECIMAL == CORE::GucefTypeIdForTypeName( GUCEF_DATATYPE_NAME_INT64_UINT8_DECIMAL ) );
            ASSERT_TRUE( 9 == CORE::GucefByteSizeOfFixedSizeType( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL ) );
            ASSERT_TRUE( 9 == CORE::GucefByteSizeOfFixedSizeType( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL ) );
            ASSERT_TRUE( 9 == CVariant::ByteSizeOfFixedSizeType( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL ) );
            ASSERT_TRUE( CVariant::TypeIdForTypeName( "INT64_UINT8_BE_DECIMAL" ) == GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_DECIMAL == TryToGetGucefTypeIdForTType< CDecimal >() );

            // the decimal does not grow the variant
            ASSERT_TRUE( sizeof( CORE::TDecimalData ) <= sizeof( CORE::THeapData ) );
            ASSERT_TRUE( 8 == offsetof( CORE::TDecimalData, scale ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a decimal in a variant: classification, payload and conversions out" )
        try
        {
            CVariant var( D( "-12.99" ) );
            ASSERT_TRUE( var.IsDecimal() && var.IsNumber() );
            ASSERT_FALSE( var.IsInteger() || var.IsFloat() || var.IsFraction() || var.IsString() || var.IsBoolean() );
            ASSERT_FALSE( var.UsesDynamicMemory() || var.IsEndianAgnostic() );
            ASSERT_TRUE( g_nativeDecimalId == var.GetTypeId() );
            ASSERT_TRUE( 9 == var.ByteSize() );
            ASSERT_TRUE( var.GetTypeName() == CORE::CString( CORE::TypeNameForGucefTypeId( g_nativeDecimalId ) ) );
            ASSERT_TRUE( Identical( var, D( "-12.99" ) ) );
            ASSERT_TRUE( var.AsDecimal().IsIdentical( D( "-12.99" ) ) );
            ASSERT_TRUE( var.AsTValue< CDecimal >().IsIdentical( D( "-12.99" ) ) );
            ASSERT_TRUE( var.AsString() == "-12.99" && var.AsUtf8String() == "-12.99" && var.AsAsciiString() == "-12.99" );
            ASSERT_TRUE( var.AsString( CORE::CString::Empty, true ) == "-12.99" );
            ASSERT_TRUE( -12.99 == var.AsFloat64() );
            ASSERT_TRUE( -12.99f == var.AsFloat32() );
            ASSERT_TRUE( -12 == var.AsInt64() && -12 == var.AsInt32() && -12 == var.AsInt8() && -12 == var.AsInt16() );
            ASSERT_TRUE( 77 == var.AsUInt32( 77 ) && 77 == var.AsUInt64( 77 ) && 7 == var.AsUInt8( 7 ) && 77 == var.AsSizeT( 77 ) );
            ASSERT_TRUE( var.AsBool() );
            ASSERT_FALSE( CVariant( D( "0.000" ) ).AsBool( true ) );
            ASSERT_TRUE( 300 == CVariant( D( "300.7" ) ).AsUInt16() && 5 == CVariant( D( "300.7" ) ).AsInt8( 5 ) && 9 == CVariant( D( "300.7" ) ).AsUInt8( 9 ) );
            ASSERT_TRUE( 300 == CVariant( D( "300.7" ) ).AsSizeT() );
            ASSERT_TRUE( 9223372036854775807ULL == CVariant( D( "9223372036854775807" ) ).AsUInt64() );

            // the payload is contiguous and the rest of the union is zeroed
            const UInt8* bytes = static_cast< const UInt8* >( var.AsVoidPtr() );
            ASSERT_TRUE( bytes == reinterpret_cast< const UInt8* >( &var.CStyleAccess()->union_data.decimal_data ) );
            ASSERT_TRUE( 2 == bytes[ 8 ] );
            for ( UInt32 i=9; i<GUCEF_VARIANT_BSOB_SIZE; ++i )
                ASSERT_TRUE( 0 == var.CStyleAccess()->union_data.bsob_data[ i ] );

            CVariant assigned;
            assigned = D( "1.50" );
            ASSERT_TRUE( Identical( assigned, D( "1.50" ) ) && g_nativeDecimalId == assigned.GetTypeId() );
            CVariant viaTemplate;
            ASSERT_TRUE( viaTemplate.SetTValue( D( "0.0825" ) ) && Identical( viaTemplate, D( "0.0825" ) ) );
            ASSERT_FALSE( viaTemplate.TTypeUsesDynamicMemory< CDecimal >() );
            CVariant copied( var );
            ASSERT_TRUE( Identical( copied, D( "-12.99" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: byte order conversions round trip and the foreign byte order still converts" )
        try
        {
            CVariant var( D( "-1234.5678" ) );
            ASSERT_TRUE( GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN ? var.IsLittleEndian() : var.IsBigEndian() );
            var.ToBigEndian();
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL == var.GetTypeId() && var.IsBigEndian() && !var.IsLittleEndian() );
            ASSERT_TRUE( Identical( var, D( "-1234.5678" ) ) && var.AsString() == "-1234.5678" );
            UInt8 expected[ 9 ];
            DecimalPayload( -12345678, 4, true, expected );
            ASSERT_TRUE( 0 == memcmp( var.AsVoidPtr(), expected, 9 ) );
            var.ToLittleEndian();
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL == var.GetTypeId() && var.IsLittleEndian() );
            DecimalPayload( -12345678, 4, false, expected );
            ASSERT_TRUE( 0 == memcmp( var.AsVoidPtr(), expected, 9 ) );
            ASSERT_TRUE( Identical( var, D( "-1234.5678" ) ) );
            var.ToBigEndian();
            var.ToNativeEndian();
            ASSERT_TRUE( g_nativeDecimalId == var.GetTypeId() && Identical( var, D( "-1234.5678" ) ) );
            CVariant foreign( D( "42.42" ) );
            if ( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL == g_foreignDecimalId )
                foreign.ToLittleEndian();
            else
                foreign.ToBigEndian();
            ASSERT_TRUE( g_foreignDecimalId == foreign.GetTypeId() );
            ASSERT_TRUE( 42.42 == foreign.AsFloat64() && 42 == foreign.AsInt32() && foreign.AsDecimal().IsIdentical( D( "42.42" ) ) );
            ASSERT_TRUE( foreign == CVariant( D( "42.420" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: AsDecimal from every source type" )
        try
        {
            const CDecimal fallback = D( "-999.9" );
            CDecimal r;
            ASSERT_TRUE( CVariant( (Int8) -8 ).AsDecimal().IsIdentical( D( "-8" ) ) );
            ASSERT_TRUE( CVariant( (UInt8) 200 ).AsDecimal().IsIdentical( D( "200" ) ) );
            ASSERT_TRUE( CVariant( (Int16) -1600 ).AsDecimal().IsIdentical( D( "-1600" ) ) );
            ASSERT_TRUE( CVariant( (UInt16) 65535 ).AsDecimal().IsIdentical( D( "65535" ) ) );
            ASSERT_TRUE( CVariant( (Int32) -32 ).AsDecimal().IsIdentical( D( "-32" ) ) );
            ASSERT_TRUE( CVariant( (UInt32) 4294967295UL ).AsDecimal().IsIdentical( D( "4294967295" ) ) );
            ASSERT_TRUE( CVariant( (Int64) -9223372036854775807LL ).AsDecimal().IsIdentical( D( "-9223372036854775807" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CVariant( GUCEF_INT64MIN ).GetDecimal( r ) );
            ASSERT_TRUE( CVariant( (UInt64) 9223372036854775807ULL ).AsDecimal().IsIdentical( D( "9223372036854775807" ) ) );
            ASSERT_TRUE( CVariant( (UInt64) 9223372036854775808ULL ).AsDecimal( fallback ).IsIdentical( fallback ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CVariant( (UInt64) 18446744073709551615ULL ).GetDecimal( r ) );
            ASSERT_TRUE( CVariant( true ).AsDecimal().IsIdentical( D( "1" ) ) && CVariant( false ).AsDecimal( fallback ).IsIdentical( D( "0" ) ) );
            ASSERT_TRUE( CVariant( 0.1f ).AsDecimal().IsIdentical( D( "0.1" ) ) );
            ASSERT_TRUE( CVariant( 0.0825 ).AsDecimal().IsIdentical( D( "0.0825" ) ) );
            Float64 zero = 0.0;
            ASSERT_TRUE( CVariant( zero / zero ).AsDecimal( fallback ).IsIdentical( fallback ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CVariant( 1.0 / zero ).GetDecimal( r ) );

            ASSERT_TRUE( CVariant( "12.50" ).AsDecimal().IsIdentical( D( "12.50" ) ) );
            ASSERT_TRUE( CVariant( CORE::CAsciiString( "-0.0825" ) ).AsDecimal().IsIdentical( D( "-0.0825" ) ) );
            ASSERT_TRUE( CVariant( CORE::CUtf16String( "7.25" ) ).AsDecimal().IsIdentical( D( "7.25" ) ) );
            ASSERT_TRUE( CVariant( CORE::CUtf32String( "7.250" ) ).AsDecimal().IsIdentical( D( "7.250" ) ) );
            ASSERT_TRUE( CVariant( "1e5" ).AsDecimal( fallback ).IsIdentical( fallback ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == CVariant( " 12.5" ).GetDecimal( r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == CVariant( "" ).GetDecimal( r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CVariant( "0.0000000000000000001" ).GetDecimal( r ) );

            ASSERT_TRUE( CVariant( CORE::TInt32Fraction( 1, 8 ) ).AsDecimal().IsIdentical( D( "0.125" ) ) );
            ASSERT_TRUE( CVariant( CORE::TInt32Fraction( -3, 4 ) ).AsDecimal().IsIdentical( D( "-0.75" ) ) );
            ASSERT_TRUE( CVariant( CORE::TInt32Fraction( 6, 8 ) ).AsDecimal().IsIdentical( D( "0.75" ) ) );
            ASSERT_TRUE( CVariant( CORE::TUInt32Fraction( 7, 20 ) ).AsDecimal().IsIdentical( D( "0.35" ) ) );
            ASSERT_TRUE( CVariant( CORE::TInt64Int32Fraction( 5, 1 ) ).AsDecimal().IsIdentical( D( "5" ) ) );
            ASSERT_TRUE( CVariant( CORE::TUInt64UInt32Fraction( 1, 1024 ) ).AsDecimal().IsIdentical( D( "0.0009765625" ) ) );
            ASSERT_TRUE( CVariant( CORE::TInt64Fraction( -1, 16 ) ).AsDecimal().IsIdentical( D( "-0.0625" ) ) );
            ASSERT_TRUE( CVariant( CORE::TUInt64Fraction( 3, 2 ) ).AsDecimal().IsIdentical( D( "1.5" ) ) );
            ASSERT_TRUE( CVariant( CORE::TInt32Fraction( 1, 3 ) ).AsDecimal( fallback ).IsIdentical( fallback ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == CVariant( CORE::TInt32Fraction( 1, 3 ) ).GetDecimal( r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CVariant( CORE::TUInt32Fraction( 1, 524288 ) ).GetDecimal( r ) );    // 2^19

            CVariant nothing;
            ASSERT_TRUE( nothing.AsDecimal( fallback ).IsIdentical( fallback ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == nothing.GetDecimal( r ) );
            CORE::CDynamicBuffer blob;
            blob.CopyFrom( 4, "1.25" );
            ASSERT_TRUE( CVariant( blob ).AsDecimal( fallback ).IsIdentical( fallback ) );
            ASSERT_TRUE( CVariant( CORE::CDateTime::Empty ).AsDecimal( fallback ).IsIdentical( fallback ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Set from raw bytes validates and keeps the byte order id, SetFromString parses strictly" )
        try
        {
            UInt8 raw[ 16 ];
            memset( raw, 0, sizeof( raw ) );
            DecimalPayload( 825, 4, true, raw + 3 );     // misaligned on purpose
            CVariant var;
            ASSERT_TRUE( var.Set( raw + 3, 9, GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL ) );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL == var.GetTypeId() && Identical( var, D( "0.0825" ) ) );
            DecimalPayload( -825, 4, false, raw + 1 );
            ASSERT_TRUE( var.Set( raw + 1, 10, GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL ) );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL == var.GetTypeId() && Identical( var, D( "-0.0825" ) ) );

            ASSERT_FALSE( var.Set( raw + 1, 8, GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL ) );
            ASSERT_FALSE( var.Set( GUCEF_NULL, 9, GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL ) );
            DecimalPayload( 1, 19, false, raw );
            ASSERT_FALSE( var.Set( raw, 9, GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL ) );
            DecimalPayload( 1, 255, true, raw );
            ASSERT_FALSE( var.Set( raw, 9, GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL ) );
            DecimalPayload( GUCEF_INT64MIN, 2, true, raw );
            ASSERT_FALSE( var.Set( raw, 9, GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL ) );
            ASSERT_FALSE( var.IsDecimal() );

            CVariant parsed( (UInt8) GUCEF_DATATYPE_INT64_UINT8_DECIMAL, CORE::CString( "12.50" ) );
            ASSERT_TRUE( parsed.IsDecimal() && Identical( parsed, D( "12.50" ) ) );
            CVariant parsedForeign( g_foreignDecimalId, CORE::CString( "-3.000" ) );
            ASSERT_TRUE( g_foreignDecimalId == parsedForeign.GetTypeId() && Identical( parsedForeign, D( "-3.000" ) ) );
            CVariant target;
            ASSERT_TRUE( target.SetFromString( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL, "0.5" ) && GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL == target.GetTypeId() && Identical( target, D( "0.5" ) ) );
            ASSERT_FALSE( target.SetFromString( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL, "1,5" ) );
            ASSERT_TRUE( target.SetFromString( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL, "1,5", CVariant( D( "2.25" ) ) ) && Identical( target, D( "2.25" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: variant comparisons with decimals and use in variant containers" )
        try
        {
            ASSERT_TRUE( CVariant( D( "1.5" ) ) == CVariant( D( "1.50" ) ) );
            ASSERT_TRUE( CVariant( D( "1.49" ) ) < CVariant( D( "1.5" ) ) && CVariant( D( "1.5" ) ) > CVariant( D( "1.49" ) ) );
            ASSERT_TRUE( CVariant( D( "1.5" ) ) <= CVariant( D( "1.50" ) ) && CVariant( D( "1.5" ) ) >= CVariant( D( "1.50" ) ) );
            ASSERT_TRUE( CVariant( D( "1.5" ) ) != CVariant( D( "1.51" ) ) );

            ASSERT_TRUE( CVariant( D( "2.00" ) ) == CVariant( (Int32) 2 ) && CVariant( (Int32) 2 ) == CVariant( D( "2.00" ) ) );
            ASSERT_TRUE( CVariant( D( "2.01" ) ) > CVariant( (Int32) 2 ) && CVariant( (UInt8) 2 ) < CVariant( D( "2.01" ) ) );
            ASSERT_TRUE( CVariant( D( "-0.5" ) ) < CVariant( (UInt64) 0 ) );
            ASSERT_TRUE( CVariant( D( "9223372036854775807" ) ) < CVariant( (UInt64) 9223372036854775808ULL ) );
            ASSERT_TRUE( CVariant( (UInt64) 18446744073709551615ULL ) > CVariant( D( "9223372036854775807" ) ) );
            ASSERT_FALSE( CVariant( (UInt64) 18446744073709551615ULL ) == CVariant( D( "9223372036854775807" ) ) );
            ASSERT_TRUE( CVariant( D( "-9223372036854775807" ) ) > CVariant( GUCEF_INT64MIN ) );

            ASSERT_TRUE( CVariant( D( "1.5" ) ) == CVariant( "1.50" ) && CVariant( "1.50" ) == CVariant( D( "1.5" ) ) );
            ASSERT_TRUE( CVariant( D( "1.5" ) ) < CVariant( "1.6" ) && CVariant( "10" ) > CVariant( D( "9.99" ) ) );
            ASSERT_TRUE( CVariant( D( "0.5" ) ) == CVariant( 0.5 ) && CVariant( D( "0.25" ) ) < CVariant( 0.5f ) );
            ASSERT_TRUE( CVariant( 0.75 ) > CVariant( D( "0.5" ) ) );

            CVariant::VariantSet set;
            set.insert( CVariant( D( "1.5" ) ) );
            set.insert( CVariant( D( "1.50" ) ) );
            set.insert( CVariant( D( "2" ) ) );
            set.insert( CVariant( D( "-1" ) ) );
            ASSERT_TRUE( 3 == set.size() );
            ASSERT_TRUE( set.begin()->AsDecimal().IsIdentical( D( "-1" ) ) );

            CVariant::VariantMap map;
            map[ CVariant( D( "0.10" ) ) ] = CVariant( "ten cents" );
            map[ CVariant( D( "0.1" ) ) ] = CVariant( "a tenth" );
            ASSERT_TRUE( 1 == map.size() && map.begin()->second.AsString() == "a tenth" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CVariantBinarySerializer round trips decimals through a buffer at any offset" )
        try
        {
            const CDecimal values[ 4 ] = { D( "0.0825" ), D( "-92233720368547758.07" ), D( "0.000" ), D( "9.223372036854775807" ) };
            for ( UInt32 v=0; v<4; ++v )
            {
                for ( UInt32 order=0; order<2; ++order )
                {
                    CVariant var( values[ v ] );
                    if ( 0 == order )
                        var.ToLittleEndian();
                    else
                        var.ToBigEndian();

                    const UInt32 offsets[] = { 0, 1, 3, 7 };
                    for ( UInt32 o=0; o<4; ++o )
                    {
                        CORE::CDynamicBuffer buffer;
                        for ( UInt32 i=0; i<offsets[ o ]; ++i )
                            buffer.AppendValue( (UInt8) 0xEE );
                        UInt32 bytesWritten = 0;
                        ASSERT_TRUE( CORE::CVariantBinarySerializer::Serialize( var, offsets[ o ], buffer, bytesWritten ) );
                        ASSERT_TRUE( 10 == bytesWritten && offsets[ o ] + 10 == buffer.GetDataSize() );

                        // wire form: type id, 8 unscaled bytes in the id's byte order, scale
                        const UInt8* wire = static_cast< const UInt8* >( buffer.GetConstBufferPtr( offsets[ o ] ) );
                        UInt8 expected[ 9 ];
                        DecimalPayload( values[ v ].GetUnscaledValue(), values[ v ].GetScale(), 1 == order, expected );
                        ASSERT_TRUE( var.GetTypeId() == wire[ 0 ] && 0 == memcmp( wire + 1, expected, 9 ) );

                        CVariant back;
                        UInt32 bytesRead = 0;
                        ASSERT_TRUE( CORE::CVariantBinarySerializer::Deserialize( back, offsets[ o ], buffer, false, bytesRead ) );
                        ASSERT_TRUE( 10 == bytesRead && var.GetTypeId() == back.GetTypeId() && Identical( back, values[ v ] ) );
                    }

                    // raw form without the type id
                    CORE::CDynamicBuffer raw;
                    UInt32 rawWritten = 0;
                    ASSERT_TRUE( CORE::CVariantBinarySerializer::SerializeRaw( var, 0, raw, rawWritten ) && 9 == rawWritten );
                    CVariant rawBack;
                    UInt32 rawRead = 0;
                    ASSERT_TRUE( CORE::CVariantBinarySerializer::DeserializeRaw( rawBack, var.GetTypeId(), 0, raw, false, rawRead ) );
                    ASSERT_TRUE( 9 == rawRead && Identical( rawBack, values[ v ] ) );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CVariantBinarySerializer through CIOAccess, truncated and corrupt data" )
        try
        {
            // serialize to an IO access, read back raw from an IO access
            CORE::CDynamicBuffer output;
            {
                CORE::CDynamicBufferAccess access( &output, false );
                UInt32 written = 0;
                ASSERT_TRUE( CORE::CVariantBinarySerializer::Serialize( CVariant( D( "-0.0825" ) ), access, written ) && 10 == written );
                written = 0;
                CVariant bigEndian( D( "123.456" ) );
                bigEndian.ToBigEndian();
                ASSERT_TRUE( CORE::CVariantBinarySerializer::SerializeRaw( bigEndian, access, written ) && 9 == written );
            }
            ASSERT_TRUE( 19 == output.GetDataSize() );
            {
                CORE::CDynamicBufferAccess input( output );
                UInt8 typeId = 0;
                ASSERT_TRUE( 1 == input.Read( &typeId, 1, 1 ) && g_nativeDecimalId == typeId );
                CVariant first;
                UInt32 read = 0;
                ASSERT_TRUE( CORE::CVariantBinarySerializer::DeserializeRaw( first, typeId, input, read, 0 ) && 9 == read );
                ASSERT_TRUE( Identical( first, D( "-0.0825" ) ) );
                CVariant second;
                read = 0;
                ASSERT_TRUE( CORE::CVariantBinarySerializer::DeserializeRaw( second, GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL, input, read, 0 ) && 9 == read );
                ASSERT_TRUE( Identical( second, D( "123.456" ) ) );
            }

            // a truncated value: the type id and 8 of the 9 payload bytes
            CORE::CDynamicBuffer truncated;
            UInt32 written = 0;
            ASSERT_TRUE( CORE::CVariantBinarySerializer::Serialize( CVariant( D( "1.25" ) ), 0, truncated, written ) );
            truncated.SetDataSize( 9 );
            CVariant result;
            UInt32 bytesRead = 0;
            ASSERT_FALSE( CORE::CVariantBinarySerializer::Deserialize( result, 0, truncated, false, bytesRead ) );
            ASSERT_FALSE( result.IsDecimal() );
            bytesRead = 0;
            ASSERT_FALSE( CORE::CVariantBinarySerializer::DeserializeRaw( result, g_nativeDecimalId, 1, truncated, false, bytesRead ) );
            {
                CORE::CDynamicBufferAccess input( truncated );
                UInt8 typeId = 0;
                input.Read( &typeId, 1, 1 );
                bytesRead = 0;
                ASSERT_FALSE( CORE::CVariantBinarySerializer::DeserializeRaw( result, typeId, input, bytesRead, 0 ) );
            }

            // corrupt scale bytes
            const UInt8 badScales[ 2 ] = { 19, 255 };
            for ( UInt32 i=0; i<2; ++i )
            {
                CORE::CDynamicBuffer corrupt;
                UInt8 bytes[ 10 ];
                bytes[ 0 ] = GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL;
                DecimalPayload( 125, badScales[ i ], false, bytes + 1 );
                corrupt.CopyFrom( 10, bytes );
                bytesRead = 0;
                ASSERT_FALSE( CORE::CVariantBinarySerializer::Deserialize( result, 0, corrupt, false, bytesRead ) );

                // the raw IO access path assigns the data unvalidated: the conversions validate again
                CORE::CDynamicBufferAccess input( corrupt );
                UInt8 typeId = 0;
                input.Read( &typeId, 1, 1 );
                CVariant unvalidated;
                bytesRead = 0;
                ASSERT_TRUE( CORE::CVariantBinarySerializer::DeserializeRaw( unvalidated, typeId, input, bytesRead, 0 ) );
                ASSERT_TRUE( unvalidated.IsDecimal() );
                CDecimal value = D( "7.77" );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == unvalidated.GetDecimal( value ) && value.IsIdentical( D( "7.77" ) ) );
                ASSERT_TRUE( unvalidated.AsDecimal( D( "-1" ) ).IsIdentical( D( "-1" ) ) );
                ASSERT_TRUE( unvalidated.AsString( "fallback" ) == "fallback" );
                ASSERT_TRUE( 3.5 == unvalidated.AsFloat64( 3.5 ) && 4 == unvalidated.AsInt32( 4 ) && unvalidated.AsBool( true ) );
            }

            // INT64_MIN as unscaled value
            CORE::CDynamicBuffer minimum;
            UInt8 bytes[ 10 ];
            bytes[ 0 ] = GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL;
            DecimalPayload( GUCEF_INT64MIN, 0, true, bytes + 1 );
            minimum.CopyFrom( 10, bytes );
            bytesRead = 0;
            ASSERT_FALSE( CORE::CVariantBinarySerializer::Deserialize( result, 0, minimum, false, bytesRead ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CDataNodeBinarySerializer round trips decimal values and attributes" )
        try
        {
            CORE::CDataNode dom1;
            dom1.SetName( "ledger" );
            dom1.SetAttribute( "total", CVariant( D( "1234.50" ) ) );
            CVariant foreign( D( "-0.0825" ) );
            foreign.ToBigEndian();
            dom1.SetAttribute( "rate", foreign );
            CORE::CDataNode* entry = dom1.AddChild( "entry" );
            entry->SetValue( CVariant( D( "0.000000000000000001" ) ) );
            entry->SetAttribute( "quantity", CVariant( D( "123.456789" ) ) );

            CORE::CDataNodeBinarySerializerOptions options;
            CORE::CDynamicBuffer buffer;
            UInt32 bytesWritten = 0;
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Serialize( options, dom1, 0, buffer, bytesWritten ) );
            CORE::CDataNode dom2;
            UInt32 bytesRead = 0;
            ASSERT_TRUE( CORE::CDataNodeBinarySerializer::Deserialize( options, true, dom2, 0, buffer, bytesRead ) );
            ASSERT_TRUE( bytesRead == bytesWritten );

            ASSERT_TRUE( Identical( dom2.GetAttributeValue( "total" ), D( "1234.50" ) ) );
            ASSERT_TRUE( GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL == dom2.GetAttributeValue( "rate" ).GetTypeId() );
            ASSERT_TRUE( Identical( dom2.GetAttributeValue( "rate" ), D( "-0.0825" ) ) );
            const CORE::CDataNode* entry2 = dom2.FindChild( "entry" );
            ASSERT_TRUE( GUCEF_NULL != entry2 );
            if ( GUCEF_NULL != entry2 )
            {
                ASSERT_TRUE( Identical( entry2->GetValue(), D( "0.000000000000000001" ) ) );
                ASSERT_TRUE( Identical( entry2->GetAttributeValue( "quantity" ), D( "123.456789" ) ) );
            }
            ASSERT_TRUE( dom1 == dom2 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: JSON text carries decimals as canonical strings and reloads them exactly" )
        try
        {
            TCodecPtr codec;
            ASSERT_TRUE( GetCodec( "json", "dstorepluginJSONPARSER", codec ) );
            if ( !codec.IsNULL() )
            {
                CORE::CDataNode tree;
                CORE::CDataNode* main = tree.Structure( "Main", '/' );
                main->SetAttribute( "amount", CVariant( D( "12.50" ) ) );
                CVariant foreign( D( "-0.0825" ) );
                foreign.ToBigEndian();
                main->SetAttribute( "rate", foreign );
                main->AddValueAsChild( CVariant( D( "0.000000000000000001" ) ), false, "tiny" );

                CORE::CDynamicBuffer output;
                {
                    CORE::CDynamicBufferAccess outputAccess( &output, false );
                    ASSERT_TRUE( codec->StoreDataTree( &tree, &outputAccess ) );
                }
                CORE::CString text = BufferText( output );
                ASSERT_TRUE( text.HasSubstr( "\"12.50\"" ) >= 0 );
                ASSERT_TRUE( text.HasSubstr( "\"-0.0825\"" ) >= 0 );
                ASSERT_TRUE( text.HasSubstr( "\"0.000000000000000001\"" ) >= 0 );

                CORE::CDynamicBufferAccess rereadAccess( output );
                CORE::CDataNode reread;
                ASSERT_TRUE( codec->BuildDataTree( &reread, &rereadAccess ) );
                const CORE::CDataNode* readMain = reread.Search( "Main", '/', true, true );
                ASSERT_TRUE( GUCEF_NULL != readMain );
                if ( GUCEF_NULL != readMain )
                {
                    ASSERT_TRUE( readMain->GetAttributeValueOrChildValueByName( "amount" ).AsDecimal().IsIdentical( D( "12.50" ) ) );
                    ASSERT_TRUE( readMain->GetAttributeValueOrChildValueByName( "rate" ).AsDecimal().IsIdentical( D( "-0.0825" ) ) );
                    ASSERT_TRUE( readMain->GetAttributeValueOrChildValueByName( "tiny" ).AsDecimal().IsIdentical( D( "0.000000000000000001" ) ) );
                }

                // the streaming writer goes through the same conversion
                CORE::CDataNode record;
                record.SetName( "record" );
                record.SetAttribute( "price", CVariant( D( "99.990" ) ) );
                CORE::CDynamicBuffer streamed;
                {
                    CORE::CDynamicBufferAccess access( &streamed, false );
                    CORE::CDocumentStreamWriterPtr writer = codec->CreateDocumentWriter( access );
                    ASSERT_FALSE( writer.IsNULL() );
                    if ( !writer.IsNULL() )
                    {
                        ASSERT_TRUE( writer->WriteNode( record ) );
                        ASSERT_TRUE( writer->Close() );
                    }
                }
                ASSERT_TRUE( BufferText( streamed ).HasSubstr( "\"99.990\"" ) >= 0 );

                // a JSON number is accepted as tolerance: integers exactly, floats via the shortest decimal
                const char* numbers = "{ \"Main\": { \"rate\": 0.0825, \"count\": 12, \"neg\": -2.5 } }";
                CORE::CDynamicBuffer input;
                input.CopyFrom( (UInt32) strlen( numbers ), numbers );
                CORE::CDynamicBufferAccess inputAccess( input );
                CORE::CDataNode numberTree;
                ASSERT_TRUE( codec->BuildDataTree( &numberTree, &inputAccess ) );
                const CORE::CDataNode* numberMain = numberTree.Search( "Main", '/', true, true );
                ASSERT_TRUE( GUCEF_NULL != numberMain );
                if ( GUCEF_NULL != numberMain )
                {
                    ASSERT_TRUE( numberMain->GetAttributeValueOrChildValueByName( "rate" ).AsDecimal().IsIdentical( D( "0.0825" ) ) );
                    ASSERT_TRUE( numberMain->GetAttributeValueOrChildValueByName( "count" ).AsDecimal().IsIdentical( D( "12" ) ) );
                    ASSERT_TRUE( numberMain->GetAttributeValueOrChildValueByName( "neg" ).AsDecimal().IsIdentical( D( "-2.5" ) ) );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CDecimal CVariant and serializer TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
