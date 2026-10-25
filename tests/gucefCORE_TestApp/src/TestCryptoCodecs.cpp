/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
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

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CBUILTINCODECS_H
#include "gucefCORE_CBuiltInCodecs.h"
#define GUCEF_CORE_CBUILTINCODECS_H
#endif /* GUCEF_CORE_CBUILTINCODECS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CSHA512_H
#include "gucefCORE_CSha512.h"
#define GUCEF_CORE_CSHA512_H
#endif /* GUCEF_CORE_CSHA512_H ? */

#ifndef GUCEF_CORE_CRSAPKCS1CODECS_H
#include "gucefCORE_CRsaPkcs1Codecs.h"
#define GUCEF_CORE_CRSAPKCS1CODECS_H
#endif /* GUCEF_CORE_CRSAPKCS1CODECS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAllocationCounter.h"
#include "TestRsaVectors.h"
#include "TestCryptoCodecs.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Ends the test case with a failure when a codec or instance that has to be there is not: the catch of the test case records it */
#define REQUIRE_CODEC(c) do { if ( (c).IsNULL() ) throw 0; } while (0)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

typedef CORE::CCodecRegistry::TICodecPtr TICodecPtr;
typedef CORE::CCodecRegistry::TICodecPtrVector TICodecPtrVector;

/*-------------------------------------------------------------------------*/

static CORE::CDynamicBuffer
FromHex( const char* hex )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    UInt32 length = (UInt32) strlen( hex );
    for ( UInt32 i=0; i+1<length; i+=2 )
    {
        char pair[ 3 ] = { hex[ i ], hex[ i+1 ], '\0' };
        UInt8 value = (UInt8) strtoul( pair, GUCEF_NULL, 16 );
        bytes.Append( &value, 1, true );
    }
    return bytes;
}

/*-------------------------------------------------------------------------*/

static CORE::CDynamicBuffer
FromText( const char* text )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    bytes.Append( text, (UInt32) strlen( text ), true );
    return bytes;
}

/*-------------------------------------------------------------------------*/

static CORE::CDynamicBuffer
Repeated( UInt8 value, UInt32 count )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    for ( UInt32 i=0; i<count; ++i )
        bytes.Append( &value, 1, true );
    return bytes;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ToHex( const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    static const char* digits = "0123456789abcdef";
    CORE::CString hex;
    const UInt8* p = static_cast< const UInt8* >( bytes.GetConstBufferPtr() );
    for ( UInt32 i=0; i<bytes.GetDataSize(); ++i )
    {
        char pair[ 3 ] = { digits[ p[ i ] >> 4 ], digits[ p[ i ] & 0xF ], '\0' };
        hex += pair;
    }
    return hex;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ToText( const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    if ( 0 == bytes.GetDataSize() )
        return CORE::CString::Empty;
    return CORE::CString( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

static bool
RunOneShot( TICodecPtr& codec, bool encode, const CORE::CDynamicBuffer& input, const CORE::CValueList& params, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    CORE::CDynamicBuffer inputCopy( input );
    CORE::CDynamicBufferAccess inputAccess( inputCopy );
    output.Clear( true );
    CORE::CDynamicBufferAccess outputAccess( output );
    return encode ? codec->Encode( inputAccess, outputAccess, params ) : codec->Decode( inputAccess, outputAccess, params );
}

/*-------------------------------------------------------------------------*/

static bool
RunChunked( CORE::CICodecInstancePtr& instance, const CORE::CDynamicBuffer& input, UInt32 chunkSize, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    UInt32 offset = 0;
    UInt32 flags = GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_ENCODE;
    output.Clear( true );
    do
    {
        UInt32 size = input.GetDataSize() - offset;
        if ( size > chunkSize )
            size = chunkSize;
        if ( offset + size == input.GetDataSize() )
            flags |= GUCEF_CODEC_PROCESS_FINAL;
        if ( !instance->Process( flags, input.GetConstBufferPtr( offset ), size, GUCEF_NULL, output ) )
            return false;
        offset += size;
        flags = GUCEF_CODEC_PROCESS_UPDATE | GUCEF_CODEC_PROCESS_ENCODE;
    }
    while ( offset < input.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ProviderName( const TICodecPtr& codec )
{GUCEF_TRACE;

    return codec->GetClassTypeName();
}

/*-------------------------------------------------------------------------*/

/** Whether the codec object is one of the codecs gucefCORE brings: software or OS */
static bool
IsBuiltInCodec( const TICodecPtr& codec )
{GUCEF_TRACE;

    TICodecPtrVector builtIns;
    CORE::CCoreGlobal::Instance()->GetBuiltInCodecs().GetOsCryptoCodecs( builtIns );
    CORE::CCoreGlobal::Instance()->GetBuiltInCodecs().GetSoftwareCodecs( builtIns );
    for ( UInt32 i=0; i<(UInt32) builtIns.size(); ++i )
        if ( builtIns[ i ] == codec )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** Whether the codec object is one of the OS crypto codecs that are registered */
static bool
IsOsCodec( const TICodecPtr& codec )
{GUCEF_TRACE;

    TICodecPtrVector osCodecs;
    CORE::CCoreGlobal::Instance()->GetBuiltInCodecs().GetOsCryptoCodecs( osCodecs );
    for ( UInt32 i=0; i<(UInt32) osCodecs.size(); ++i )
        if ( osCodecs[ i ] == codec )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** A codec for the registry tests: Encode writes its tag, the input and the "nonce" parameter */
class CTaggedTestCodec : public CORE::CICodec
{
    public:

    CTaggedTestCodec( const CORE::CString& family, const CORE::CString& type, const CORE::CString& tag )
        : CORE::CICodec(), m_family( family ), m_type( type ), m_tag( tag )
    {GUCEF_TRACE;
    }

    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return Encode( source, dest, CORE::CValueList() );
    }

    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return false;
    }

    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest, const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::CString text = m_tag + ":";
        char buffer[ 64 ];
        UInt32 bytesRead = 0;
        while ( 0 < ( bytesRead = source.Read( buffer, 1, sizeof( buffer ) ) ) )
            text += CORE::CString( buffer, bytesRead );
        if ( params.HasKey( "nonce" ) )
        {
            const CORE::CVariant& nonce = params.GetValue( "nonce" );
            text += CORE::CString( static_cast< const char* >( nonce.AsVoidPtr() ), nonce.ByteSize() );
        }
        dest.Write( text.C_String(), 1, text.Length() );
        SetLastStatus( CORE::GUCEF_CODEC_STATUS_OK );
        return true;
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_type; }

    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_family; }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTaggedTestCodec( m_family, m_type, m_tag ); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTaggedTestCodec";
        return classTypeName;
    }

    private:

    CORE::CString m_family;
    CORE::CString m_type;
    CORE::CString m_tag;
};

/*-------------------------------------------------------------------------*/

static CORE::CString
EncodeToText( TICodecPtr codec )
{GUCEF_TRACE;

    CORE::CDynamicBuffer output;
    if ( codec.IsNULL() || !RunOneShot( codec, true, CORE::CDynamicBuffer(), CORE::CValueList(), output ) )
        return CORE::CString::Empty;
    return ToText( output );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformCodecRegistryTests( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

    GUCEF_TESTFW_TESTCASE( "Test: The codec registry gives the provider with the highest priority and falls back when it goes" )
        try
        {
            TICodecPtr low( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( "RegistryTestCodec", "t", "low" ) ) );
            TICodecPtr high( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( "RegistryTestCodec", "t", "high" ) ) );
            TICodecPtr mid( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( "RegistryTestCodec", "t", "mid" ) ) );
            TICodecPtr sameAsMid( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( "RegistryTestCodec", "t", "midtoo" ) ) );

            ASSERT_TRUE( registry.RegisterCodec( low, 10 ) );
            ASSERT_TRUE( registry.RegisterCodec( high, 90 ) );
            ASSERT_TRUE( registry.RegisterCodec( mid, 50 ) );
            ASSERT_TRUE( registry.RegisterCodec( sameAsMid, 50 ) );
            ASSERT_FALSE( registry.RegisterCodec( mid, 70 ) );       // one registration per codec object and type

            ASSERT_TRUE( "high:" == EncodeToText( registry.TryGetCodec( "RegistryTestCodec", "t" ) ) );
            ASSERT_TRUE( "high:" == EncodeToText( registry.TryGetCodec( "registrytestcodec", "T" ) ) );    // no regard to case

            // the family registry has the one with the highest priority, for code that looks there
            CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
            TICodecPtr inFamily;
            ASSERT_TRUE( registry.TryLookup( "RegistryTestCodec", family, true ) && family->TryLookup( "t", inFamily, true ) && inFamily == high );

            // all of them, highest first, equal priorities in the order of registration
            TICodecPtrVector all;
            ASSERT_TRUE( registry.GetCodecs( "RegistryTestCodec", "t", all ) );
            ASSERT_TRUE( 4 == all.size() );
            if ( 4 == all.size() )
                ASSERT_TRUE( all[ 0 ] == high && all[ 1 ] == mid && all[ 2 ] == sameAsMid && all[ 3 ] == low );

            Int32 priority = 0;
            ASSERT_TRUE( registry.TryGetCodecPriority( "RegistryTestCodec", "t", low.GetPointerAlways(), priority ) && 10 == priority );
            ASSERT_TRUE( 2 == registry.GetHeldReferenceCount( high.GetPointerAlways() ) );     // the list and the family registry
            ASSERT_TRUE( 1 == registry.GetHeldReferenceCount( low.GetPointerAlways() ) );

            // taking one out keeps the others, the next one takes its place
            ASSERT_TRUE( registry.UnregisterCodec( high ) );
            ASSERT_FALSE( registry.UnregisterCodec( high ) );
            ASSERT_TRUE( "mid:" == EncodeToText( registry.TryGetCodec( "RegistryTestCodec", "t" ) ) );
            ASSERT_TRUE( family->TryLookup( "t", inFamily, true ) && inFamily == mid );
            ASSERT_TRUE( 0 == registry.GetHeldReferenceCount( high.GetPointerAlways() ) );

            ASSERT_TRUE( registry.UnregisterCodec( mid ) );
            ASSERT_TRUE( registry.UnregisterCodec( sameAsMid ) );
            ASSERT_TRUE( "low:" == EncodeToText( registry.TryGetCodec( "RegistryTestCodec", "t" ) ) );
            ASSERT_TRUE( registry.UnregisterCodec( low ) );
            ASSERT_TRUE( registry.TryGetCodec( "RegistryTestCodec", "t" ).IsNULL() );
            ASSERT_FALSE( family->TryLookup( "t", inFamily, true ) );
            all.clear();
            ASSERT_FALSE( registry.GetCodecs( "RegistryTestCodec", "t", all ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A codec put into a family registry directly stays found next to registrations with a priority" )
        try
        {
            TICodecPtr direct( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( "RegistryTestCodec", "d", "direct" ) ) );
            TICodecPtr fallback( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( "RegistryTestCodec", "d", "fallback" ) ) );
            TICodecPtr preferred( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( "RegistryTestCodec", "d", "preferred" ) ) );

            CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
            if ( !registry.TryLookup( "RegistryTestCodec", family, true ) )
            {
                family = CORE::CCodecRegistry::TCodecFamilyRegistryTypedPtr( GUCEF_NEW CORE::CCodecRegistry::TCodecFamilyRegistry() );
                registry.Register( "RegistryTestCodec", family );
            }
            family->Register( "d", direct );
            ASSERT_TRUE( "direct:" == EncodeToText( registry.TryGetCodec( "RegistryTestCodec", "d" ) ) );
            TICodecPtrVector all;
            ASSERT_TRUE( registry.GetCodecs( "RegistryTestCodec", "d", all ) && 1 == all.size() );

            // it counts as a plugin, so a lower priority comes after it and a higher one before it
            ASSERT_TRUE( registry.RegisterCodec( fallback, CORE::CCodecRegistry::SoftwareCodecPriority ) );
            ASSERT_TRUE( "direct:" == EncodeToText( registry.TryGetCodec( "RegistryTestCodec", "d" ) ) );
            ASSERT_TRUE( registry.RegisterCodec( preferred, CORE::CCodecRegistry::OsCodecPriority ) );
            ASSERT_TRUE( "preferred:" == EncodeToText( registry.TryGetCodec( "RegistryTestCodec", "d" ) ) );
            all.clear();
            ASSERT_TRUE( registry.GetCodecs( "RegistryTestCodec", "d", all ) && 3 == all.size() );

            ASSERT_TRUE( registry.UnregisterCodec( preferred ) );
            ASSERT_TRUE( registry.UnregisterCodec( direct ) );
            ASSERT_TRUE( "fallback:" == EncodeToText( registry.TryGetCodec( "RegistryTestCodec", "d" ) ) );
            ASSERT_TRUE( registry.UnregisterCodec( fallback ) );
            ASSERT_TRUE( registry.TryGetCodec( "RegistryTestCodec", "d" ).IsNULL() );
            registry.TryUnregister( "RegistryTestCodec" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The default instance gathers a message and runs the codec with the parameters and call arguments" )
        try
        {
            TICodecPtr codec( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( "RegistryTestCodec", "i", "tag" ) ) );
            CORE::CICodecInstancePtr instance;
            ASSERT_TRUE( codec->CreateInstance( CORE::CValueList(), instance ) && !instance.IsNULL() );
            if ( !instance.IsNULL() )
            {
                CORE::SCodecCallArgs args;
                memset( &args, 0, sizeof( args ) );
                args.structSize = sizeof( args );
                args.nonce = "N1";
                args.nonceSize = 2;

                CORE::CDynamicBuffer output;
                output.Append( "head|", 5, true );
                ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_ENCODE, "ab", 2, &args, output ) );
                ASSERT_TRUE( 5 == output.GetDataSize() );
                ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_FINAL | GUCEF_CODEC_PROCESS_ENCODE, "cd", 2, &args, output ) );
                ASSERT_TRUE( "head|tag:abcdN1" == ToText( output ) );     // appended after what was there
                if ( "head|tag:abcdN1" != ToText( output ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "The default instance gave \"" << ToText( output ) << "\"" );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_OK == instance->GetLastStatus() );
                ASSERT_TRUE( 0 == instance->GetOverhead() );

                ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_ENCODE | GUCEF_CODEC_PROCESS_DECODE, "x", 1, GUCEF_NULL, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == instance->GetLastStatus() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSoftwareCryptoPrimitiveTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: SHA-256, HMAC-SHA256 and HKDF-SHA256 in software give the FIPS 180-4, RFC 4231 and RFC 5869 answers" )
        try
        {
            UInt8 digest[ 32 ];
            CORE::CDynamicBuffer result;

            CORE::CSha256::Hash( "abc", 3, digest );
            result.CopyFrom( 32, digest );
            ASSERT_TRUE( "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" == ToHex( result ) );
            CORE::CSha256::Hash( "", 0, digest );
            result.CopyFrom( 32, digest );
            ASSERT_TRUE( "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" == ToHex( result ) );

            // incremental with every split of the 56 byte message, the padding needs a second block
            const char* message = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
            for ( UInt32 split=0; split<=56; ++split )
            {
                CORE::CSha256 sha;
                sha.Update( message, split );
                sha.Update( message + split, 56 - split );
                sha.Final( digest );
                result.CopyFrom( 32, digest );
                ASSERT_TRUE( "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" == ToHex( result ) );
            }

            CORE::CDynamicBuffer key = Repeated( 0x0b, 20 );
            CORE::CHmacSha256::Mac( key.GetConstBufferPtr(), 20, "Hi There", 8, digest );
            result.CopyFrom( 32, digest );
            ASSERT_TRUE( "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7" == ToHex( result ) );

            // the same key for two messages, Reset in between
            CORE::CHmacSha256 hmac( "Jefe", 4 );
            hmac.Update( "what do ya want ", 16 );
            hmac.Update( "for nothing?", 12 );
            hmac.Final( digest );
            result.CopyFrom( 32, digest );
            ASSERT_TRUE( "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843" == ToHex( result ) );
            hmac.Update( "what do ya want for nothing?", 28 );
            hmac.Final( digest );
            result.CopyFrom( 32, digest );
            ASSERT_TRUE( "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843" == ToHex( result ) );

            CORE::CDynamicBuffer longKey = Repeated( 0xaa, 131 );
            const char* longKeyMessage = "Test Using Larger Than Block-Size Key - Hash Key First";
            CORE::CHmacSha256::Mac( longKey.GetConstBufferPtr(), 131, longKeyMessage, (UInt32) strlen( longKeyMessage ), digest );
            result.CopyFrom( 32, digest );
            ASSERT_TRUE( "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54" == ToHex( result ) );

            UInt8 okm[ 42 ];
            CORE::CDynamicBuffer ikm = Repeated( 0x0b, 22 );
            CORE::CDynamicBuffer salt = FromHex( "000102030405060708090a0b0c" );
            CORE::CDynamicBuffer info = FromHex( "f0f1f2f3f4f5f6f7f8f9" );
            ASSERT_TRUE( CORE::CHkdfSha256::Derive( salt.GetConstBufferPtr(), 13, ikm.GetConstBufferPtr(), 22, info.GetConstBufferPtr(), 10, okm, 42 ) );
            result.CopyFrom( 42, okm );
            ASSERT_TRUE( "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865" == ToHex( result ) );
            ASSERT_TRUE( CORE::CHkdfSha256::Derive( GUCEF_NULL, 0, ikm.GetConstBufferPtr(), 22, GUCEF_NULL, 0, okm, 42 ) );
            result.CopyFrom( 42, okm );
            ASSERT_TRUE( "8da4e775a563c18f715f802a063c5a31b8a11f5c5ee1879ec3454e5f3c738d2d9d201395faa4b61a96c8" == ToHex( result ) );
            UInt8 tooMuch[ 1 ];
            ASSERT_FALSE( CORE::CHkdfSha256::Expand( digest, GUCEF_NULL, 0, tooMuch, CORE::CHkdfSha256::MaxOutputSize + 1 ) );

            ASSERT_TRUE( CORE::CSha256::ConstantTimeEquals( "abcd", "abcd", 4 ) );
            ASSERT_FALSE( CORE::CSha256::ConstantTimeEquals( "abcd", "abce", 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

struct SHashVector
{
    const char* type;
    const char* input;
    const char* expectedHex;
};

/*-------------------------------------------------------------------------*/

static void
PerformHashProviderTests( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

    GUCEF_TESTFW_TESTCASE( "Test: Every provider of sha256 and sha512 gives the known digests, one shot and incrementally" )
        try
        {
            static const SHashVector vectors[] =
            {
                { "sha256", "abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" },
                { "sha256", "", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" },
                { "sha256", "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" },
                { "sha512", "abc", "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f" },
                { "sha512", "", "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e" }
            };

            TICodecPtrVector sha256Providers;
            ASSERT_TRUE( registry.GetCodecs( CORE::CoreCodecTypes::CryptographicHashCodec, "sha256", sha256Providers ) );
            ASSERT_TRUE( sha256Providers.size() >= 1 );   // the software one is always there

            CORE::CValueList raw;
            raw.Set( "output", CORE::CVariant( "raw" ) );
            for ( UInt32 v=0; v<sizeof( vectors ) / sizeof( vectors[ 0 ] ); ++v )
            {
                TICodecPtrVector providers;
                registry.GetCodecs( CORE::CoreCodecTypes::CryptographicHashCodec, vectors[ v ].type, providers );
                for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
                {
                    TICodecPtr codec = providers[ p ];
                    CORE::CDynamicBuffer input = FromText( vectors[ v ].input );
                    CORE::CDynamicBuffer output;

                    // the default output is hex text, as the hash codecs always gave
                    ASSERT_TRUE( RunOneShot( codec, true, input, CORE::CValueList(), output ) );
                    ASSERT_TRUE( vectors[ v ].expectedHex == ToText( output ) );
                    ASSERT_TRUE( RunOneShot( codec, true, input, raw, output ) );
                    ASSERT_TRUE( vectors[ v ].expectedHex == ToHex( output ) );
                    if ( vectors[ v ].expectedHex != ToHex( output ) )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Hash provider " << ProviderName( codec ) << " gave a wrong digest for " << vectors[ v ].type );

                    CORE::CICodecInstancePtr instance;
                    ASSERT_TRUE( codec->CreateInstance( raw, instance ) && !instance.IsNULL() );
                    if ( !instance.IsNULL() )
                    {
                        for ( UInt32 chunkSize=1; chunkSize<70; chunkSize+=17 )
                        {
                            ASSERT_TRUE( RunChunked( instance, input, chunkSize, output ) );
                            ASSERT_TRUE( vectors[ v ].expectedHex == ToHex( output ) );
                        }
                    }
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every provider of hmac-sha256 gives the RFC 4231 answers" )
        try
        {
            struct SMacVector { CORE::CDynamicBuffer key; CORE::CDynamicBuffer data; const char* expectedHex; };
            SMacVector vectors[ 4 ];
            vectors[ 0 ].key = Repeated( 0x0b, 20 ); vectors[ 0 ].data = FromText( "Hi There" );
            vectors[ 0 ].expectedHex = "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7";
            vectors[ 1 ].key = FromText( "Jefe" ); vectors[ 1 ].data = FromText( "what do ya want for nothing?" );
            vectors[ 1 ].expectedHex = "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843";
            vectors[ 2 ].key = Repeated( 0xaa, 20 ); vectors[ 2 ].data = Repeated( 0xdd, 50 );
            vectors[ 2 ].expectedHex = "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe";
            vectors[ 3 ].key = Repeated( 0xaa, 131 ); vectors[ 3 ].data = FromText( "Test Using Larger Than Block-Size Key - Hash Key First" );
            vectors[ 3 ].expectedHex = "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54";

            TICodecPtrVector providers;
            ASSERT_TRUE( registry.GetCodecs( CORE::CoreCodecTypes::MacCodec, "hmac-sha256", providers ) );
            for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
            {
                TICodecPtr codec = providers[ p ];
                for ( UInt32 v=0; v<4; ++v )
                {
                    CORE::CValueList params;
                    params.Set( "key", CORE::CVariant( vectors[ v ].key ) );
                    CORE::CDynamicBuffer output;
                    ASSERT_TRUE( RunOneShot( codec, true, vectors[ v ].data, params, output ) );
                    ASSERT_TRUE( vectors[ v ].expectedHex == ToHex( output ) );

                    CORE::CICodecInstancePtr instance;
                    ASSERT_TRUE( codec->CreateInstance( params, instance ) && !instance.IsNULL() );
                    if ( !instance.IsNULL() )
                    {
                        ASSERT_TRUE( RunChunked( instance, vectors[ v ].data, 7, output ) );
                        ASSERT_TRUE( vectors[ v ].expectedHex == ToHex( output ) );
                        ASSERT_TRUE( RunChunked( instance, vectors[ v ].data, 64, output ) );   // the key stays for the next message
                        ASSERT_TRUE( vectors[ v ].expectedHex == ToHex( output ) );
                    }
                }

                // the key is required
                CORE::CDynamicBuffer output;
                ASSERT_FALSE( RunOneShot( codec, true, vectors[ 0 ].data, CORE::CValueList(), output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
                ASSERT_FALSE( codec->GetLastErrorMessage().IsNULLOrEmpty() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every provider of hkdf-sha256 gives the RFC 5869 answers" )
        try
        {
            TICodecPtrVector providers;
            ASSERT_TRUE( registry.GetCodecs( CORE::CoreCodecTypes::KeyDerivationCodec, "hkdf-sha256", providers ) );
            for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
            {
                TICodecPtr codec = providers[ p ];
                CORE::CDynamicBuffer ikm = Repeated( 0x0b, 22 );

                CORE::CValueList params;
                params.Set( "salt", CORE::CVariant( FromHex( "000102030405060708090a0b0c" ) ) );
                params.Set( "info", CORE::CVariant( FromHex( "f0f1f2f3f4f5f6f7f8f9" ) ) );
                params.Set( "length", CORE::CVariant( (UInt32) 42 ) );
                CORE::CDynamicBuffer output;
                ASSERT_TRUE( RunOneShot( codec, true, ikm, params, output ) );
                ASSERT_TRUE( "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865" == ToHex( output ) );

                CORE::CValueList noSaltNoInfo;
                noSaltNoInfo.Set( "length", CORE::CVariant( "42" ) );
                ASSERT_TRUE( RunOneShot( codec, true, ikm, noSaltNoInfo, output ) );
                ASSERT_TRUE( "8da4e775a563c18f715f802a063c5a31b8a11f5c5ee1879ec3454e5f3c738d2d9d201395faa4b61a96c8" == ToHex( output ) );

                // 32 bytes when no length is given
                ASSERT_TRUE( RunOneShot( codec, true, ikm, CORE::CValueList(), output ) && 32 == output.GetDataSize() );

                CORE::CValueList tooLong;
                tooLong.Set( "length", CORE::CVariant( (UInt32) 9000 ) );
                ASSERT_FALSE( RunOneShot( codec, true, ikm, tooLong, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
                CORE::CValueList notANumber;
                notANumber.Set( "length", CORE::CVariant( "4x2" ) );
                ASSERT_FALSE( RunOneShot( codec, true, ikm, notANumber, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
                ASSERT_FALSE( RunOneShot( codec, false, ikm, params, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED == codec->GetLastStatus() );

                CORE::CCodecParameterInfoList info;
                codec->GetParameterInfo( info );
                ASSERT_TRUE( 3 == info.size() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

struct SAeadVector
{
    const char* type;
    const char* keyHex;
    const char* nonceHex;
    const char* aadHex;
    const char* plaintextHex;
    const char* ciphertextAndTagHex;
};

/*-------------------------------------------------------------------------*/

static void
PerformAeadProviderTests( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

    static const SAeadVector vectors[] =
    {
        // RFC 8439 section 2.8.2
        { "chacha20-poly1305", "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f", "070000004041424344454647", "50515253c0c1c2c3c4c5c6c7",
          "4c616469657320616e642047656e746c656d656e206f662074686520636c617373206f66202739393a204966204920636f756c64206f6666657220796f75206f6e6c79206f6e652074697020666f7220746865206675747572652c2073756e73637265656e20776f756c642062652069742e",
          "d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee62d63dbea45e8ca9671282fafb69da92728b1a71de0a9e060b2905d6a5b67ecd3b3692ddbd7f2d778b8c9803aee328091b58fab324e4fad675945585808b4831d7bc3ff4def08e4b7a9de576d26586cec64b61161ae10b594f09e26a7e902ecbd0600691" },
        // draft-irtf-cfrg-xchacha section A.3.1
        { "xchacha20-poly1305", "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f", "404142434445464748494a4b4c4d4e4f5051525354555657", "50515253c0c1c2c3c4c5c6c7",
          "4c616469657320616e642047656e746c656d656e206f662074686520636c617373206f66202739393a204966204920636f756c64206f6666657220796f75206f6e6c79206f6e652074697020666f7220746865206675747572652c2073756e73637265656e20776f756c642062652069742e",
          "bd6d179d3e83d43b9576579493c0e939572a1700252bfaccbed2902c21396cbb731c7f1b0b4aa6440bf3a82f4eda7e39ae64c6708c54c216cb96b72e1213b4522f8c9ba40db5d945b11b69b982c1bb9e3f3fac2bc369488f76b2383565d3fff921f9664c97637da9768812f615c68b13b52ec0875924c1c7987947deafd8780acf49" },
        // The GCM specification (McGrew, Viega) test cases 13, 14 and 16
        { "aes-256-gcm", "0000000000000000000000000000000000000000000000000000000000000000", "000000000000000000000000", "", "", "530f8afbc74536b9a963b4f1c4cb738b" },
        { "aes-256-gcm", "0000000000000000000000000000000000000000000000000000000000000000", "000000000000000000000000", "", "00000000000000000000000000000000",
          "cea7403d4d606b6e074ec5d3baf39d18d0d1c8a799996bf0265b98b5d48ab919" },
        { "aes-256-gcm", "feffe9928665731c6d6a8f9467308308feffe9928665731c6d6a8f9467308308", "cafebabefacedbaddecaf888", "feedfacedeadbeeffeedfacedeadbeefabaddad2",
          "d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a721c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39",
          "522dc1f099567d07f47f37a32a84427d643a8cdcbfe5c0c97598a2bd2555d1aa8cb08e48590dbb3da7b08b1056828838c5f61e6393ba7a0abcc9f66276fc6ece0f4e1768cddf8853bb2d551b" }
    };
    static const UInt32 vectorCount = (UInt32) ( sizeof( vectors ) / sizeof( vectors[ 0 ] ) );

    GUCEF_TESTFW_TESTCASE( "Test: Every AEAD provider gives the RFC 8439 and GCM answers and refuses what is not authentic" )
    {
        TICodecPtrVector chachaProviders, gcmProviders, xchachaProviders;
        registry.GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, "chacha20-poly1305", chachaProviders );
        registry.GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, "xchacha20-poly1305", xchachaProviders );
        registry.GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, "aes-256-gcm", gcmProviders );
        GUCEF_TESTFW_SKIP_IF( chachaProviders.empty() && gcmProviders.empty(), "No AEAD codec is registered: no OS facility here and no crypto plugin loaded" );

        try
        {
            for ( UInt32 v=0; v<vectorCount; ++v )
            {
                const SAeadVector& vector = vectors[ v ];
                CORE::CDynamicBuffer key = FromHex( vector.keyHex );
                CORE::CDynamicBuffer nonce = FromHex( vector.nonceHex );
                CORE::CDynamicBuffer aad = FromHex( vector.aadHex );
                CORE::CDynamicBuffer plaintext = FromHex( vector.plaintextHex );

                TICodecPtrVector providers;
                registry.GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, vector.type, providers );
                for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
                {
                    TICodecPtr codec = providers[ p ];
                    ASSERT_TRUE( 0 != ( codec->GetCapabilities() & GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY ) );

                    // one shot, the nonce and associated data as parameters
                    CORE::CValueList params;
                    params.Set( "key", CORE::CVariant( key ) );
                    params.Set( "nonce", CORE::CVariant( nonce ) );
                    if ( aad.GetDataSize() > 0 )
                        params.Set( "aad", CORE::CVariant( aad ) );
                    CORE::CDynamicBuffer sealed, opened;
                    ASSERT_TRUE( RunOneShot( codec, true, plaintext, params, sealed ) );
                    ASSERT_TRUE( vector.ciphertextAndTagHex == ToHex( sealed ) );
                    if ( vector.ciphertextAndTagHex != ToHex( sealed ) )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AEAD provider " << ProviderName( codec ) << " gave a wrong answer for " << vector.type << ": " << ToHex( sealed ) );
                    bool decoded = RunOneShot( codec, false, sealed, params, opened );
                    ASSERT_TRUE( decoded );
                    if ( !decoded )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AEAD provider " << ProviderName( codec ) << " did not decode: status " << (Int32) codec->GetLastStatus() << ": " << codec->GetLastErrorMessage() );
                    ASSERT_TRUE( ToHex( plaintext ) == ToHex( opened ) );

                    // an instance: the key once, nonce and associated data per call
                    CORE::CValueList keyOnly;
                    keyOnly.Set( "key", CORE::CVariant( key ) );
                    CORE::CICodecInstancePtr instance;
                    ASSERT_TRUE( codec->CreateInstance( keyOnly, instance ) && !instance.IsNULL() );
                    if ( instance.IsNULL() )
                        continue;
                    ASSERT_TRUE( 16 == instance->GetOverhead() );

                    CORE::SCodecCallArgs args;
                    memset( &args, 0, sizeof( args ) );
                    args.structSize = sizeof( args );
                    args.nonce = nonce.GetConstBufferPtr();
                    args.nonceSize = nonce.GetDataSize();
                    args.aad = aad.GetDataSize() > 0 ? aad.GetConstBufferPtr() : GUCEF_NULL;
                    args.aadSize = aad.GetDataSize();

                    CORE::CDynamicBuffer packet;
                    packet.Append( "HDR", 3, true );        // the output is appended after what is there
                    ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, plaintext.GetConstBufferPtr(), plaintext.GetDataSize(), &args, packet ) );
                    ASSERT_TRUE( 3 + sealed.GetDataSize() == packet.GetDataSize() );
                    ASSERT_TRUE( 0 == memcmp( packet.GetConstBufferPtr( 3 ), sealed.GetConstBufferPtr(), sealed.GetDataSize() ) );
                    opened.Clear( true );
                    ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, sealed.GetConstBufferPtr(), sealed.GetDataSize(), &args, opened ) );
                    ASSERT_TRUE( ToHex( plaintext ) == ToHex( opened ) );

                    // a changed bit, a changed associated data, a wrong nonce: not authentic, nothing written
                    CORE::CDynamicBuffer tampered( sealed );
                    tampered.AsTypePtr< UInt8 >()[ tampered.GetDataSize() / 2 ] ^= 0x01;
                    opened.Clear( true );
                    ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, tampered.GetConstBufferPtr(), tampered.GetDataSize(), &args, opened ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == instance->GetLastStatus() );
                    if ( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED != instance->GetLastStatus() )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AEAD provider " << ProviderName( codec ) << " for " << vector.type << " on a changed message: status " << (Int32) instance->GetLastStatus() << ": " << instance->GetLastErrorMessage() );
                    ASSERT_TRUE( 0 == opened.GetDataSize() );
                    ASSERT_FALSE( RunOneShot( codec, false, tampered, params, opened ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );

                    CORE::SCodecCallArgs otherAad( args );
                    otherAad.aad = "x";
                    otherAad.aadSize = 1;
                    ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, sealed.GetConstBufferPtr(), sealed.GetDataSize(), &otherAad, opened ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == instance->GetLastStatus() );

                    // and the instance still works after refusals
                    opened.Clear( true );
                    ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, sealed.GetConstBufferPtr(), sealed.GetDataSize(), &args, opened ) );
                    ASSERT_TRUE( ToHex( plaintext ) == ToHex( opened ) );

                    // what is required
                    ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, "x", 1, GUCEF_NULL, opened ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == instance->GetLastStatus() );
                    CORE::SCodecCallArgs shortNonce( args );
                    shortNonce.nonceSize = 8;
                    ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, "x", 1, &shortNonce, opened ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == instance->GetLastStatus() );
                    ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_ENCODE, "x", 1, &args, opened ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED == instance->GetLastStatus() );
                    ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, "short", 5, &args, opened ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_INPUT == instance->GetLastStatus() );

                    CORE::CICodecInstancePtr noInstance;
                    ASSERT_FALSE( codec->CreateInstance( CORE::CValueList(), noInstance ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
                    CORE::CValueList shortKey;
                    shortKey.Set( "key", CORE::CVariant( Repeated( 1, 16 ) ) );
                    ASSERT_FALSE( codec->CreateInstance( shortKey, noInstance ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );

                    CORE::CCodecParameterInfoList info;
                    codec->GetParameterInfo( info );
                    ASSERT_TRUE( 3 == info.size() && "key" == info[ 0 ].GetName() && info[ 0 ].GetIsRequired() );
                }
            }

            // what one provider seals, another opens
            UInt32 crossChecks = 0;
            TICodecPtrVector* all[ 3 ] = { &chachaProviders, &gcmProviders, &xchachaProviders };
            const UInt32 nonceSizes[ 3 ] = { 12, 12, 24 };
            for ( UInt32 t=0; t<3; ++t )
            {
                TICodecPtrVector& providers = *all[ t ];
                for ( UInt32 a=0; a<(UInt32) providers.size(); ++a )
                {
                    for ( UInt32 b=0; b<(UInt32) providers.size(); ++b )
                    {
                        if ( a == b )
                            continue;
                        CORE::CValueList params;
                        params.Set( "key", CORE::CVariant( Repeated( 7, 32 ) ) );
                        params.Set( "nonce", CORE::CVariant( Repeated( 9, nonceSizes[ t ] ) ) );
                        CORE::CDynamicBuffer sealed, opened;
                        ASSERT_TRUE( RunOneShot( providers[ a ], true, FromText( "between providers" ), params, sealed ) );
                        ASSERT_TRUE( RunOneShot( providers[ b ], false, sealed, params, opened ) );
                        ASSERT_TRUE( "between providers" == ToText( opened ) );
                        if ( "between providers" != ToText( opened ) )
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AEAD providers " << ProviderName( providers[ a ] ) << " and " << ProviderName( providers[ b ] ) << " do not agree" );
                        ++crossChecks;
                    }
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AEAD: " << crossChecks << " cross provider round trips" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Runs messages through an instance with a buffer that has its working size, counting heap allocations */
static UInt32
CountAllocationsOfProcess( CORE::CICodecInstancePtr& instance, UInt32 flags, const CORE::CDynamicBuffer& input, const CORE::SCodecCallArgs* args, bool& allSucceeded )
{GUCEF_TRACE;

    CORE::CDynamicBuffer output;
    output.SetBufferSize( input.GetDataSize() + 256, false );
    allSucceeded = instance->Process( flags, input.GetConstBufferPtr(), input.GetDataSize(), args, output );

    CORETEST::StartAllocationCounting();
    for ( UInt32 i=0; i<100; ++i )
    {
        output.Clear( true );
        allSucceeded = instance->Process( flags, input.GetConstBufferPtr(), input.GetDataSize(), args, output ) && allSucceeded;
    }
    UInt32 allocations = CORETEST::StopAllocationCounting();
    if ( allocations > 0 )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Codec instance allocated " << allocations << " times:\n" << CORETEST::GetCountedAllocationStacks() );
    return allocations;
}

/*-------------------------------------------------------------------------*/

static void
PerformInstanceAllocationTests( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

    // The codecs of gucefCORE (software and OS); a plugin allocates as its library does
    GUCEF_TESTFW_TESTCASE( "Test: Instances of the crypto codecs (built in, OS, the AEADs and BLAKE2b of plugins) make no heap allocation per call once created" )
    {
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsAllocationCountingAvailable(), CORETEST::GetAllocationCountingUnavailableReason() );
        try
        {
            CORE::CDynamicBuffer packet = Repeated( 0x5a, 1200 );

            struct SInstanceCase { const CORE::CString* family; const char* type; };
            const SInstanceCase cases[] =
            {
                { &CORE::CoreCodecTypes::CryptographicHashCodec, "sha256" },
                { &CORE::CoreCodecTypes::CryptographicHashCodec, "sha512" },
                { &CORE::CoreCodecTypes::MacCodec, "hmac-sha256" },
                { &CORE::CoreCodecTypes::KeyDerivationCodec, "hkdf-sha256" },
                { &CORE::CoreCodecTypes::CryptographicHashCodec, "blake2b" }
            };
            CORE::CValueList params;
            params.Set( "key", CORE::CVariant( Repeated( 3, 32 ) ) );
            params.Set( "output", CORE::CVariant( "raw" ) );
            for ( UInt32 c=0; c<sizeof( cases ) / sizeof( cases[ 0 ] ); ++c )
            {
                TICodecPtrVector providers;
                registry.GetCodecs( *cases[ c ].family, cases[ c ].type, providers );
                for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
                {
                    // the hash plugin's library gives its digests as text, so of the plugins only Monocypher's hashes are counted
                    if ( !IsBuiltInCodec( providers[ p ] ) && 0 != strcmp( "blake2b", cases[ c ].type ) )
                        continue;
                    CORE::CICodecInstancePtr instance;
                    ASSERT_TRUE( providers[ p ]->CreateInstance( params, instance ) && !instance.IsNULL() );
                    if ( instance.IsNULL() )
                        continue;
                    bool succeeded = false;
                    UInt32 allocations = CountAllocationsOfProcess( instance, GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, packet, GUCEF_NULL, succeeded );
                    ASSERT_TRUE( succeeded );
                    ASSERT_TRUE( 0 == allocations );
                    if ( 0 != allocations )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Provider " << ProviderName( providers[ p ] ) << " of " << cases[ c ].type << " allocates per call" );
                }
            }

            // AEAD per packet, both ways, with a nonce and a header as associated data
            // every provider: the OS ones and the plugins
            const char* aeadTypes[] = { "chacha20-poly1305", "aes-256-gcm", "xchacha20-poly1305" };
            for ( UInt32 t=0; t<3; ++t )
            {
                TICodecPtrVector providers;
                registry.GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, aeadTypes[ t ], providers );
                for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
                {
                    CORE::CICodecInstancePtr instance;
                    ASSERT_TRUE( providers[ p ]->CreateInstance( params, instance ) && !instance.IsNULL() );
                    if ( instance.IsNULL() )
                        continue;

                    UInt8 nonce[ 24 ] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
                    UInt8 header[ 45 ];
                    memset( header, 0x11, sizeof( header ) );
                    CORE::SCodecCallArgs args;
                    memset( &args, 0, sizeof( args ) );
                    args.structSize = sizeof( args );
                    args.nonce = nonce;
                    args.nonceSize = 2 == t ? 24 : 12;
                    args.aad = header;
                    args.aadSize = sizeof( header );

                    bool succeeded = false;
                    ASSERT_TRUE( 0 == CountAllocationsOfProcess( instance, GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, packet, &args, succeeded ) );
                    ASSERT_TRUE( succeeded );

                    CORE::CDynamicBuffer sealed;
                    ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, packet.GetConstBufferPtr(), packet.GetDataSize(), &args, sealed ) );
                    ASSERT_TRUE( 0 == CountAllocationsOfProcess( instance, GUCEF_CODEC_PROCESS_DECODE_MESSAGE, sealed, &args, succeeded ) );
                    ASSERT_TRUE( succeeded );
                    if ( !succeeded )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AEAD provider " << ProviderName( providers[ p ] ) << " of " << aeadTypes[ t ] << " failed: " << instance->GetLastErrorMessage() );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBuiltInCodecTests( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CBuiltInCodecs& builtIns = CORE::CCoreGlobal::Instance()->GetBuiltInCodecs();

    GUCEF_TESTFW_TESTCASE( "Test: The built in codecs: software always, the OS ones switchable, the OS first" )
        try
        {
            TICodecPtrVector software;
            builtIns.GetSoftwareCodecs( software );
            ASSERT_TRUE( 6 == software.size() );

            TICodecPtrVector osCodecs;
            builtIns.GetOsCryptoCodecs( osCodecs );
            ASSERT_TRUE( builtIns.GetRegisterOsCryptoCodecs() );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Built in codecs: " << osCodecs.size() << " AF_ALG codec(s) registered" );
            #endif

            // with an OS sha256 that one is used, the software one is the fallback
            TICodecPtr sha256 = registry.TryGetCodec( CORE::CoreCodecTypes::CryptographicHashCodec, "sha256" );
            ASSERT_TRUE( !sha256.IsNULL() );
            bool haveOsSha256 = false;
            for ( UInt32 i=0; i<(UInt32) osCodecs.size(); ++i )
                if ( "sha256" == osCodecs[ i ]->GetType() )
                    haveOsSha256 = true;
            ASSERT_TRUE( haveOsSha256 == IsOsCodec( sha256 ) );

            // switched off by configuration: no OS codec is left in the registry
            CORE::CDataNode config( "Config" );
            CORE::CDataNode* node = config.AddChild( "BuiltInCodecs" );
            node->SetAttribute( "registerOsCryptoCodecs", false );
            ASSERT_TRUE( builtIns.LoadConfig( config ) );
            ASSERT_FALSE( builtIns.GetRegisterOsCryptoCodecs() );
            TICodecPtrVector none;
            builtIns.GetOsCryptoCodecs( none );
            ASSERT_TRUE( none.empty() );
            TICodecPtr softwareSha256 = registry.TryGetCodec( CORE::CoreCodecTypes::CryptographicHashCodec, "sha256" );
            ASSERT_TRUE( !softwareSha256.IsNULL() && !IsOsCodec( softwareSha256 ) );
            for ( UInt32 i=0; i<(UInt32) osCodecs.size(); ++i )
                ASSERT_TRUE( 0 == registry.GetHeldReferenceCount( osCodecs[ i ].GetPointerAlways() ) );

            // and on again
            node->SetAttribute( "registerOsCryptoCodecs", true );
            ASSERT_TRUE( builtIns.LoadConfig( config ) );
            TICodecPtrVector again;
            builtIns.GetOsCryptoCodecs( again );
            ASSERT_TRUE( osCodecs.size() == again.size() );
            if ( haveOsSha256 )
                ASSERT_TRUE( IsOsCodec( registry.TryGetCodec( CORE::CoreCodecTypes::CryptographicHashCodec, "sha256" ) ) );

            CORE::CDataNode saved( "Config" );
            ASSERT_TRUE( builtIns.SaveConfig( saved ) );
            const CORE::CDataNode* savedNode = saved.Find( "BuiltInCodecs" );
            ASSERT_TRUE( GUCEF_NULL != savedNode && savedNode->GetAttributeValueOrChildValueByName( "registerOsCryptoCodecs" ).AsBool( false, true ) );

            // the parameters the built in codecs describe
            CORE::CCodecParameterInfoList info;
            sha256->GetParameterInfo( info );
            ASSERT_TRUE( 1 == info.size() && "output" == info[ 0 ].GetName() && "hex" == info[ 0 ].GetDefaultValue().AsString() );
            info.clear();
            TICodecPtr hmac = registry.TryGetCodec( CORE::CoreCodecTypes::MacCodec, "hmac-sha256" );
            ASSERT_TRUE( !hmac.IsNULL() );
            if ( !hmac.IsNULL() )
            {
                hmac->GetParameterInfo( info );
                ASSERT_TRUE( 1 == info.size() && "key" == info[ 0 ].GetName() && info[ 0 ].GetIsRequired() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

// the group the crypto plugins are loaded in by these tests
static const char* CryptoPluginGroup = "cryptoplugin_tests";

static bool
LoadCryptoPlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, CryptoPluginGroup, true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, CryptoPluginGroup, true );
}

/*-------------------------------------------------------------------------*/

static void
PerformKeyAgreementAndSignatureTests( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

    GUCEF_TESTFW_TESTCASE( "Test: Every x25519 provider gives the RFC 7748 public keys and shared secret" )
    {
        TICodecPtrVector providers;
        registry.GetCodecs( CORE::CoreCodecTypes::KeyAgreementCodec, "x25519", providers );
        GUCEF_TESTFW_SKIP_IF( providers.empty(), "No x25519 codec is registered" );
        try
        {
            CORE::CDynamicBuffer alicePrivate = FromHex( "77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a" );
            CORE::CDynamicBuffer bobPrivate = FromHex( "5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb" );
            const char* alicePublicHex = "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a";
            const char* bobPublicHex = "de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f";
            const char* sharedHex = "4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742";

            for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
            {
                TICodecPtr codec = providers[ p ];
                CORE::CDynamicBuffer output;

                // own private key -> own public key
                ASSERT_TRUE( RunOneShot( codec, true, alicePrivate, CORE::CValueList(), output ) && alicePublicHex == ToHex( output ) );
                ASSERT_TRUE( RunOneShot( codec, true, bobPrivate, CORE::CValueList(), output ) && bobPublicHex == ToHex( output ) );

                // peer public key with the own private key -> the shared secret, the same both ways
                CORE::CValueList alice;
                alice.Set( "privateKey", CORE::CVariant( alicePrivate ) );
                CORE::CValueList bob;
                bob.Set( "privateKey", CORE::CVariant( bobPrivate ) );
                ASSERT_TRUE( RunOneShot( codec, false, FromHex( bobPublicHex ), alice, output ) && sharedHex == ToHex( output ) );
                ASSERT_TRUE( RunOneShot( codec, false, FromHex( alicePublicHex ), bob, output ) && sharedHex == ToHex( output ) );

                // a point of low order gives no secret
                ASSERT_FALSE( RunOneShot( codec, false, Repeated( 0, 32 ), alice, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_INPUT == codec->GetLastStatus() );
                ASSERT_FALSE( RunOneShot( codec, false, FromHex( bobPublicHex ), CORE::CValueList(), output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
                ASSERT_FALSE( RunOneShot( codec, true, Repeated( 1, 31 ), CORE::CValueList(), output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_INPUT == codec->GetLastStatus() );

                // between providers
                for ( UInt32 q=0; q<(UInt32) providers.size(); ++q )
                {
                    CORE::CDynamicBuffer bobPublic, secretA, secretB;
                    ASSERT_TRUE( RunOneShot( providers[ q ], true, bobPrivate, CORE::CValueList(), bobPublic ) );
                    ASSERT_TRUE( RunOneShot( codec, false, bobPublic, alice, secretA ) );
                    ASSERT_TRUE( RunOneShot( providers[ q ], false, FromHex( alicePublicHex ), bob, secretB ) );
                    ASSERT_TRUE( ToHex( secretA ) == ToHex( secretB ) );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every ed25519 provider gives the RFC 8032 public keys and signatures and verifies them" )
    {
        TICodecPtrVector providers;
        registry.GetCodecs( CORE::CoreCodecTypes::SignatureCodec, "ed25519", providers );
        GUCEF_TESTFW_SKIP_IF( providers.empty(), "No ed25519 codec is registered" );
        try
        {
            struct SSignatureVector { const char* seedHex; const char* publicHex; const char* messageHex; const char* signatureHex; };
            static const SSignatureVector vectors[] =
            {
                { "9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60", "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a", "",
                  "e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b" },
                { "4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb", "3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c", "72",
                  "92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00" }
            };

            for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
            {
                TICodecPtr codec = providers[ p ];
                for ( UInt32 v=0; v<2; ++v )
                {
                    CORE::CDynamicBuffer message = FromHex( vectors[ v ].messageHex );
                    CORE::CDynamicBuffer output;

                    // the public key of the private key (the 32 byte seed), the input is ignored
                    CORE::CValueList publicKeyParams;
                    publicKeyParams.Set( "privateKey", CORE::CVariant( FromHex( vectors[ v ].seedHex ) ) );
                    publicKeyParams.Set( "output", CORE::CVariant( "publicKey" ) );
                    ASSERT_TRUE( RunOneShot( codec, true, FromText( "ignored" ), publicKeyParams, output ) );
                    ASSERT_TRUE( vectors[ v ].publicHex == ToHex( output ) );

                    // signing, without the output parameter and with output=signature
                    CORE::CValueList signParams;
                    signParams.Set( "privateKey", CORE::CVariant( FromHex( vectors[ v ].seedHex ) ) );
                    ASSERT_TRUE( RunOneShot( codec, true, message, signParams, output ) && vectors[ v ].signatureHex == ToHex( output ) );
                    signParams.Set( "output", CORE::CVariant( "signature" ) );
                    ASSERT_TRUE( RunOneShot( codec, true, message, signParams, output ) && vectors[ v ].signatureHex == ToHex( output ) );

                    // verifying: no output, a changed message or signature is refused
                    CORE::CValueList verifyParams;
                    verifyParams.Set( "publicKey", CORE::CVariant( FromHex( vectors[ v ].publicHex ) ) );
                    verifyParams.Set( "signature", CORE::CVariant( FromHex( vectors[ v ].signatureHex ) ) );
                    ASSERT_TRUE( RunOneShot( codec, false, message, verifyParams, output ) && 0 == output.GetDataSize() );
                    CORE::CDynamicBuffer otherMessage( message );
                    otherMessage.Append( "!", 1, true );
                    ASSERT_FALSE( RunOneShot( codec, false, otherMessage, verifyParams, output ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );
                    CORE::CDynamicBuffer otherSignature = FromHex( vectors[ v ].signatureHex );
                    otherSignature.AsTypePtr< UInt8 >()[ 10 ] ^= 0x04;
                    CORE::CValueList forged;
                    forged.Set( "publicKey", CORE::CVariant( FromHex( vectors[ v ].publicHex ) ) );
                    forged.Set( "signature", CORE::CVariant( otherSignature ) );
                    ASSERT_FALSE( RunOneShot( codec, false, message, forged, output ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );
                }

                // what is required
                CORE::CDynamicBuffer output;
                CORE::CValueList longKey;
                longKey.Set( "privateKey", CORE::CVariant( Repeated( 1, 64 ) ) );
                ASSERT_FALSE( RunOneShot( codec, true, FromText( "x" ), longKey, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
                ASSERT_FALSE( RunOneShot( codec, true, FromText( "x" ), CORE::CValueList(), output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
                CORE::CValueList noSignature;
                noSignature.Set( "publicKey", CORE::CVariant( Repeated( 1, 32 ) ) );
                ASSERT_FALSE( RunOneShot( codec, false, FromText( "x" ), noSignature, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );

                // what one provider signs, every provider verifies
                for ( UInt32 q=0; q<(UInt32) providers.size(); ++q )
                {
                    CORE::CValueList signParams;
                    signParams.Set( "privateKey", CORE::CVariant( Repeated( 0x42, 32 ) ) );
                    CORE::CDynamicBuffer signature, publicKey, nothing;
                    ASSERT_TRUE( RunOneShot( codec, true, FromText( "between providers" ), signParams, signature ) );
                    signParams.Set( "output", CORE::CVariant( "publicKey" ) );
                    ASSERT_TRUE( RunOneShot( codec, true, CORE::CDynamicBuffer(), signParams, publicKey ) );
                    CORE::CValueList verifyParams;
                    verifyParams.Set( "publicKey", CORE::CVariant( publicKey ) );
                    verifyParams.Set( "signature", CORE::CVariant( signature ) );
                    ASSERT_TRUE( RunOneShot( providers[ q ], false, FromText( "between providers" ), verifyParams, nothing ) );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPluginHashAndKdfTests( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

    GUCEF_TESTFW_TESTCASE( "Test: Every blake2b provider gives the BLAKE2 answers, keyed and of other lengths, one shot and incrementally" )
    {
        TICodecPtrVector providers;
        registry.GetCodecs( CORE::CoreCodecTypes::CryptographicHashCodec, "blake2b", providers );
        GUCEF_TESTFW_SKIP_IF( providers.empty(), "No blake2b codec is registered" );
        try
        {
            for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
            {
                TICodecPtr codec = providers[ p ];
                CORE::CDynamicBuffer output;
                ASSERT_TRUE( RunOneShot( codec, true, FromText( "abc" ), CORE::CValueList(), output ) );
                ASSERT_TRUE( "ba80a53f981c4d0d6a2797b69f12f6e94c212f14685ac4b74b12bb6fdbffa2d17d87c5392aab792dc252d5de4533cc9518d38aa8dbf1925ab92386edd4009923" == ToText( output ) );
                ASSERT_TRUE( RunOneShot( codec, true, CORE::CDynamicBuffer(), CORE::CValueList(), output ) );
                ASSERT_TRUE( "786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce" == ToText( output ) );

                CORE::CValueList keyed;
                keyed.Set( "key", CORE::CVariant( FromHex( "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f" ) ) );
                keyed.Set( "output", CORE::CVariant( "raw" ) );
                ASSERT_TRUE( RunOneShot( codec, true, CORE::CDynamicBuffer(), keyed, output ) );
                ASSERT_TRUE( "10ebb67700b1868efb4417987acf4690ae9d972fb7a590c2f02871799aaa4786b5e996e8f0f4eb981fc214b005f42d2ff4233499391653df7aefcbc13fc51568" == ToHex( output ) );

                CORE::CValueList shortKeyed;
                shortKeyed.Set( "key", CORE::CVariant( FromText( "secret key" ) ) );
                shortKeyed.Set( "length", CORE::CVariant( (UInt32) 32 ) );
                shortKeyed.Set( "output", CORE::CVariant( "raw" ) );
                ASSERT_TRUE( RunOneShot( codec, true, FromText( "abc" ), shortKeyed, output ) );
                ASSERT_TRUE( "66c28e9d1dcd69d6756fc52125fe1838cf0c6a87d058545a9ff676bf51beaa6f" == ToHex( output ) );

                CORE::CICodecInstancePtr instance;
                ASSERT_TRUE( codec->CreateInstance( shortKeyed, instance ) && !instance.IsNULL() );
                if ( !instance.IsNULL() )
                {
                    for ( UInt32 chunkSize=1; chunkSize<4; ++chunkSize )
                    {
                        ASSERT_TRUE( RunChunked( instance, FromText( "abc" ), chunkSize, output ) );
                        ASSERT_TRUE( "66c28e9d1dcd69d6756fc52125fe1838cf0c6a87d058545a9ff676bf51beaa6f" == ToHex( output ) );
                    }
                }

                CORE::CValueList tooLong;
                tooLong.Set( "length", CORE::CVariant( (UInt32) 65 ) );
                ASSERT_FALSE( RunOneShot( codec, true, FromText( "abc" ), tooLong, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every hkdf-sha512 provider gives the RFC 5869 construction with SHA-512" )
    {
        TICodecPtrVector providers;
        registry.GetCodecs( CORE::CoreCodecTypes::KeyDerivationCodec, "hkdf-sha512", providers );
        GUCEF_TESTFW_SKIP_IF( providers.empty(), "No hkdf-sha512 codec is registered" );
        try
        {
            for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
            {
                CORE::CValueList params;
                params.Set( "salt", CORE::CVariant( FromHex( "000102030405060708090a0b0c" ) ) );
                params.Set( "info", CORE::CVariant( FromHex( "f0f1f2f3f4f5f6f7f8f9" ) ) );
                params.Set( "length", CORE::CVariant( (UInt32) 42 ) );
                CORE::CDynamicBuffer output;
                ASSERT_TRUE( RunOneShot( providers[ p ], true, Repeated( 0x0b, 22 ), params, output ) );
                ASSERT_TRUE( "832390086cda71fb47625bb5ceb168e4c8e26a1a16ed34d9fc7fe92c1481579338da362cb8d9f925d7cb" == ToHex( output ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every argon2i provider gives the RFC 9106 answer" )
    {
        TICodecPtrVector providers;
        registry.GetCodecs( CORE::CoreCodecTypes::KeyDerivationCodec, "argon2i", providers );
        GUCEF_TESTFW_SKIP_IF( providers.empty(), "No argon2i codec is registered" );
        try
        {
            for ( UInt32 p=0; p<(UInt32) providers.size(); ++p )
            {
                TICodecPtr codec = providers[ p ];
                CORE::CValueList params;
                params.Set( "salt", CORE::CVariant( Repeated( 0x02, 16 ) ) );
                params.Set( "key", CORE::CVariant( Repeated( 0x03, 8 ) ) );
                params.Set( "ad", CORE::CVariant( Repeated( 0x04, 12 ) ) );
                params.Set( "iterations", CORE::CVariant( (UInt32) 3 ) );
                params.Set( "memory", CORE::CVariant( (UInt32) 32 ) );
                params.Set( "lanes", CORE::CVariant( (UInt32) 4 ) );
                params.Set( "length", CORE::CVariant( (UInt32) 32 ) );
                CORE::CDynamicBuffer output;
                ASSERT_TRUE( RunOneShot( codec, true, Repeated( 0x01, 32 ), params, output ) );
                ASSERT_TRUE( "c814d9d1dc7f37aa13f0d77f2494bda1c8de6b016dd388d29952a4c4672b6ce8" == ToHex( output ) );

                CORE::CValueList noSalt;
                noSalt.Set( "memory", CORE::CVariant( (UInt32) 64 ) );
                ASSERT_FALSE( RunOneShot( codec, true, FromText( "password" ), noSalt, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
                CORE::CValueList tooLittleMemory;
                tooLittleMemory.Set( "salt", CORE::CVariant( Repeated( 0x02, 16 ) ) );
                tooLittleMemory.Set( "memory", CORE::CVariant( (UInt32) 4 ) );
                ASSERT_FALSE( RunOneShot( codec, true, FromText( "password" ), tooLittleMemory, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static TICodecPtr
GetRsaCodec( const char* type )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::SignatureCodec, type );
}

/*-------------------------------------------------------------------------*/

static CORE::CValueList
RsaParams( const CORE::CDynamicBuffer& modulus, const CORE::CDynamicBuffer& exponent, const CORE::CDynamicBuffer& signature )
{GUCEF_TRACE;

    CORE::CValueList params;
    params.Set( "modulus", CORE::CVariant( modulus ) );
    params.Set( "exponent", CORE::CVariant( exponent ) );
    params.Set( "signature", CORE::CVariant( signature ) );
    return params;
}

/*-------------------------------------------------------------------------*/

/** One shot verification, true when the signature is accepted. The outcome is in codec->GetLastStatus() */
static bool
RsaVerify( TICodecPtr& codec                    ,
           const CORE::CDynamicBuffer& message  ,
           const CORE::CDynamicBuffer& modulus  ,
           const CORE::CDynamicBuffer& exponent ,
           const CORE::CDynamicBuffer& signature,
           bool allowShortKey = false           )
{GUCEF_TRACE;

    CORE::CValueList params = RsaParams( modulus, exponent, signature );
    if ( allowShortKey )
        params.Set( "allowShortKey", CORE::CVariant( true ) );
    CORE::CDynamicBuffer output;
    return RunOneShot( codec, false, message, params, output ) && 0 == output.GetDataSize();
}

/*-------------------------------------------------------------------------*/

static CORE::CDynamicBuffer
Exponent65537( void )
{GUCEF_TRACE;

    return FromHex( "010001" );
}

/*-------------------------------------------------------------------------*/

static CORE::CDynamicBuffer
FlippedBit( const CORE::CDynamicBuffer& source, UInt32 byteIndex, UInt8 mask )
{GUCEF_TRACE;

    CORE::CDynamicBuffer copy( source );
    copy.AsTypePtr< UInt8 >()[ byteIndex ] ^= mask;
    return copy;
}

/*-------------------------------------------------------------------------*/

static void
PerformSoftwareSha512Tests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: SHA-512 and SHA-384 in software give the FIPS 180-4 answers, in one go and in pieces" )
        try
        {
            UInt8 digest[ 64 ];
            CORE::CDynamicBuffer result;

            CORE::CSha512::Hash( "abc", 3, digest );
            result.CopyFrom( 64, digest );
            result.SetDataSize( 64 );
            ASSERT_TRUE( "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f" == ToHex( result ) );
            CORE::CSha512::Hash( "", 0, digest );
            result.CopyFrom( 64, digest );
            result.SetDataSize( 64 );
            ASSERT_TRUE( "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e" == ToHex( result ) );
            CORE::CSha512::Hash384( "abc", 3, digest );
            result.CopyFrom( 48, digest );
            result.SetDataSize( 48 );
            ASSERT_TRUE( "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7" == ToHex( result ) );
            CORE::CSha512::Hash384( "", 0, digest );
            result.CopyFrom( 48, digest );
            result.SetDataSize( 48 );
            ASSERT_TRUE( "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b" == ToHex( result ) );

            // the 112 byte message needs a second block for the padding
            const char* message = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
            for ( UInt32 split=0; split<=112; ++split )
            {
                CORE::CSha512 sha512( false );
                sha512.Update( message, split );
                sha512.Update( message + split, 112 - split );
                sha512.Final( digest );
                result.CopyFrom( 64, digest );
                result.SetDataSize( 64 );
                ASSERT_TRUE( "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909" == ToHex( result ) );

                CORE::CSha512 sha384( true );
                ASSERT_TRUE( 48 == sha384.GetDigestSize() && 64 == sha512.GetDigestSize() );
                sha384.Update( message, split );
                sha384.Update( message + split, 112 - split );
                sha384.Final( digest );
                result.CopyFrom( 48, digest );
                result.SetDataSize( 48 );
                ASSERT_TRUE( "09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039" == ToHex( result ) );
            }

            // a million times 'a' in pieces that do not line up with the block size, and reuse after Reset
            CORE::CSha512 sha512( false );
            CORE::CSha512 sha384( true );
            for ( UInt32 round=0; round<2; ++round )
            {
                for ( UInt32 i=0; i<1000; ++i )
                {
                    UInt8 chunk[ 1000 ];
                    memset( chunk, 'a', sizeof( chunk ) );
                    sha512.Update( chunk, 1000 );
                    sha384.Update( chunk, 1000 );
                }
                sha512.Final( digest );
                result.CopyFrom( 64, digest );
                result.SetDataSize( 64 );
                ASSERT_TRUE( "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b" == ToHex( result ) );
                sha384.Final( digest );
                result.CopyFrom( 48, digest );
                result.SetDataSize( 48 );
                ASSERT_TRUE( "9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b07b8b3dc38ecc4ebae97ddd87f3d8985" == ToHex( result ) );
                sha512.Reset();
                sha384.Reset();
            }

            // 1000 bytes in steps of 1, 3, 127, 128 and 129 give one answer
            UInt8 pattern[ 1000 ];
            for ( UInt32 i=0; i<1000; ++i )
                pattern[ i ] = (UInt8) ( i * 7 + 3 );
            const UInt32 steps[ 5 ] = { 1, 3, 127, 128, 129 };
            for ( UInt32 q=0; q<5; ++q )
            {
                CORE::CSha512 shaA( false );
                CORE::CSha512 shaB( true );
                for ( UInt32 offset=0; offset<1000; offset+=steps[ q ] )
                {
                    UInt32 count = 1000 - offset < steps[ q ] ? 1000 - offset : steps[ q ];
                    shaA.Update( pattern + offset, count );
                    shaB.Update( pattern + offset, count );
                }
                shaA.Final( digest );
                result.CopyFrom( 64, digest );
                result.SetDataSize( 64 );
                ASSERT_TRUE( "00e36fccf193e59697a92b5ab24666ce6326d7fa16bf10832d0991ddc591112e9dfa6a636950ed9c4d67344a760654c2ff7785e1d60094d651038735b5dccabd" == ToHex( result ) );
                shaB.Final( digest );
                result.CopyFrom( 48, digest );
                result.SetDataSize( 48 );
                ASSERT_TRUE( "94c38db521ca733b8904c2d14b6e82d33dcfcc26e1318c579dbee1fc2f472019034e792263b9d46e90e700de2f6e7e91" == ToHex( result ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRsaSignatureTests( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    const CORE::CDynamicBuffer message = FromText( g_rsaMessage );
    const CORE::CDynamicBuffer modulusA = FromHex( g_rsaKeyAModulusHex );
    const CORE::CDynamicBuffer modulusB = FromHex( g_rsaKeyBModulusHex );
    const CORE::CDynamicBuffer e65537 = Exponent65537();

    GUCEF_TESTFW_TESTCASE( "Test: The RSA PKCS#1 v1.5 signature codecs are found by name in the SignatureCodec family, at the software priority, verify only" )
        try
        {
            const char* types[ 3 ] = { "rsa-pkcs1-sha256", "rsa-pkcs1-sha384", "rsa-pkcs1-sha512" };
            ASSERT_TRUE( 25 == CORE::CCodecRegistry::SoftwareCodecPriority );
            for ( UInt32 t=0; t<3; ++t )
            {
                TICodecPtr codec = GetRsaCodec( types[ t ] );
                ASSERT_TRUE( !codec.IsNULL() );
                if ( codec.IsNULL() )
                    continue;
                ASSERT_TRUE( types[ t ] == codec->GetType() );
                ASSERT_TRUE( CORE::CoreCodecTypes::SignatureCodec == codec->GetFamilyName() );
                ASSERT_TRUE( IsBuiltInCodec( codec ) && !IsOsCodec( codec ) );
                ASSERT_TRUE( types[ t ] == CORE::CRsaPkcs1VerifyCodec::GetTypeName( (CORE::CRsaPkcs1VerifyCodec::EHashAlgorithm) t ) );

                UInt32 capabilities = codec->GetCapabilities();
                ASSERT_TRUE( 0 != ( capabilities & GUCEF_CODEC_CAPABILITY_DECODE ) );
                ASSERT_TRUE( 0 == ( capabilities & GUCEF_CODEC_CAPABILITY_ENCODE ) );
                ASSERT_TRUE( 0 != ( capabilities & GUCEF_CODEC_CAPABILITY_INSTANCES ) );

                // the encode direction is not supported and says so
                CORE::CDynamicBuffer output;
                CORE::CValueList params = RsaParams( modulusA, e65537, FromHex( g_rsaKeyASignatureSha256Hex ) );
                ASSERT_FALSE( RunOneShot( codec, true, message, params, output ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED == codec->GetLastStatus() );
                ASSERT_TRUE( 0 == output.GetDataSize() );

                // the parameters are described
                CORE::CCodecParameterInfoList info;
                codec->GetParameterInfo( info );
                bool hasModulus = false, hasExponent = false, hasSignature = false, hasShortKey = false;
                for ( UInt32 i=0; i<(UInt32) info.size(); ++i )
                {
                    if ( "modulus" == info[ i ].GetName() ) hasModulus = info[ i ].GetIsRequired();
                    if ( "exponent" == info[ i ].GetName() ) hasExponent = info[ i ].GetIsRequired();
                    if ( "signature" == info[ i ].GetName() ) hasSignature = !info[ i ].GetIsRequired();
                    if ( "allowShortKey" == info[ i ].GetName() ) hasShortKey = !info[ i ].GetIsRequired();
                }
                ASSERT_TRUE( hasModulus && hasExponent && hasSignature && hasShortKey );

                TICodecPtrVector providers;
                ASSERT_TRUE( registry.GetCodecs( CORE::CoreCodecTypes::SignatureCodec, types[ t ], providers ) );
                ASSERT_TRUE( 1 == providers.size() );

                // priority 25: a provider below it is not used, one above it is, and the built in one is back after
                TICodecPtr lower( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( CORE::CoreCodecTypes::SignatureCodec, types[ t ], "lower" ) ) );
                TICodecPtr higher( CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CTaggedTestCodec( CORE::CoreCodecTypes::SignatureCodec, types[ t ], "higher" ) ) );
                ASSERT_TRUE( registry.RegisterCodec( lower, CORE::CCodecRegistry::SoftwareCodecPriority - 1 ) );
                ASSERT_TRUE( registry.TryGetCodec( CORE::CoreCodecTypes::SignatureCodec, types[ t ] ) == codec );
                ASSERT_TRUE( registry.RegisterCodec( higher, CORE::CCodecRegistry::SoftwareCodecPriority + 1 ) );
                ASSERT_TRUE( registry.TryGetCodec( CORE::CoreCodecTypes::SignatureCodec, types[ t ] ) == higher );
                providers.clear();
                ASSERT_TRUE( registry.GetCodecs( CORE::CoreCodecTypes::SignatureCodec, types[ t ], providers ) && 3 == providers.size() );
                ASSERT_TRUE( providers[ 0 ] == higher && providers[ 1 ] == codec && providers[ 2 ] == lower );
                ASSERT_TRUE( registry.UnregisterCodec( higher ) );
                ASSERT_TRUE( registry.UnregisterCodec( lower ) );
                ASSERT_TRUE( registry.TryGetCodec( CORE::CoreCodecTypes::SignatureCodec, types[ t ] ) == codec );
            }

            // the family keeps ed25519 and the others next to the RSA types
            ASSERT_TRUE( registry.TryGetCodec( CORE::CoreCodecTypes::SignatureCodec, "rsa-pkcs1-sha1" ).IsNULL() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: RFC 7515 appendix A.2: the RS256 example JWS verifies with its JWK, a changed message or signature does not" )
        try
        {
            TICodecPtr codec = GetRsaCodec( "rsa-pkcs1-sha256" );
            ASSERT_TRUE( !codec.IsNULL() );
            REQUIRE_CODEC( codec );

            CORE::CDynamicBuffer n, e, signature;
            ASSERT_TRUE( CORE::Base64UrlDecode( g_rfc7515A2Modulus, n ) && 256 == n.GetDataSize() );
            ASSERT_TRUE( CORE::Base64UrlDecode( "AQAB", e ) && 3 == e.GetDataSize() );
            ASSERT_TRUE( CORE::Base64UrlDecode( g_rfc7515A2Signature, signature ) && 256 == signature.GetDataSize() );
            CORE::CDynamicBuffer signingInput = FromText( g_rfc7515A2SigningInput );

            ASSERT_TRUE( RsaVerify( codec, signingInput, n, e, signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_OK == codec->GetLastStatus() );

            // the signature of the RFC is one of exactly the right size
            ASSERT_TRUE( 0x70 == static_cast< const UInt8* >( signature.GetConstBufferPtr() )[ 0 ] && 0x2e == static_cast< const UInt8* >( signature.GetConstBufferPtr() )[ 1 ] );

            // every byte of the signing input changed, one at a time
            for ( UInt32 i=0; i<signingInput.GetDataSize(); ++i )
            {
                ASSERT_FALSE( RsaVerify( codec, FlippedBit( signingInput, i, 0x01 ), n, e, signature ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );
            }

            // the signing input cut short or extended
            CORE::CDynamicBuffer shorter( signingInput );
            shorter.SetDataSize( signingInput.GetDataSize() - 1 );
            ASSERT_FALSE( RsaVerify( codec, shorter, n, e, signature ) );
            ASSERT_FALSE( RsaVerify( codec, CORE::CDynamicBuffer(), n, e, signature ) );
            CORE::CDynamicBuffer longer( signingInput );
            longer.Append( ".", 1, true );
            ASSERT_FALSE( RsaVerify( codec, longer, n, e, signature ) );

            // other hash functions of the same family do not take it
            TICodecPtr sha384 = GetRsaCodec( "rsa-pkcs1-sha384" );
            TICodecPtr sha512 = GetRsaCodec( "rsa-pkcs1-sha512" );
            ASSERT_FALSE( RsaVerify( sha384, signingInput, n, e, signature ) );
            ASSERT_FALSE( RsaVerify( sha512, signingInput, n, e, signature ) );

            // the key of another pair
            ASSERT_FALSE( RsaVerify( codec, signingInput, modulusB, e, signature ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A Google style RS256 ID token verifies from its JWK, and every tampered variant is refused" )
        try
        {
            TICodecPtr codec = GetRsaCodec( "rsa-pkcs1-sha256" );
            ASSERT_TRUE( !codec.IsNULL() );
            REQUIRE_CODEC( codec );

            // split the token the way a JWT consumer does: the signing input is everything before the last '.'
            CORE::CString token = g_googleStyleJwt;
            const char* lastDot = strrchr( token.C_String(), '.' );
            ASSERT_TRUE( GUCEF_NULL != lastDot );
            if ( GUCEF_NULL == lastDot )
                throw 0;
            const UInt32 signingInputLength = (UInt32) ( lastDot - token.C_String() );
            CORE::CDynamicBuffer signingInput;
            signingInput.Append( token.C_String(), signingInputLength, true );
            CORE::CDynamicBuffer signature, n, e;
            ASSERT_TRUE( CORE::Base64UrlDecode( lastDot + 1, signature ) && 256 == signature.GetDataSize() );
            ASSERT_TRUE( CORE::Base64UrlDecode( g_googleStyleJwkModulus, n ) && 256 == n.GetDataSize() );
            ASSERT_TRUE( CORE::Base64UrlDecode( "AQAB", e ) );
            ASSERT_TRUE( 0 == memcmp( n.GetConstBufferPtr(), modulusA.GetConstBufferPtr(), 256 ) );

            ASSERT_TRUE( RsaVerify( codec, signingInput, n, e, signature ) );

            // a bit flipped in each byte of the signature, in the low and the high bit
            for ( UInt32 i=0; i<256; ++i )
            {
                ASSERT_FALSE( RsaVerify( codec, signingInput, n, e, FlippedBit( signature, i, 0x01 ) ) );
                ASSERT_FALSE( RsaVerify( codec, signingInput, n, e, FlippedBit( signature, i, 0x80 ) ) );
            }

            // a bit flipped in each byte of the header and payload
            for ( UInt32 i=0; i<signingInput.GetDataSize(); ++i )
                ASSERT_FALSE( RsaVerify( codec, FlippedBit( signingInput, i, 0x04 ), n, e, signature ) );

            // the payload of another token with the signature of this one, and the header with the algorithm none
            const char* firstDot = strchr( token.C_String(), '.' );
            ASSERT_TRUE( GUCEF_NULL != firstDot && firstDot < lastDot );
            const UInt32 headerLength = (UInt32) ( firstDot - token.C_String() );
            CORE::CString otherPayload, noneHeader;
            const char* otherPayloadJson = "{\"iss\":\"https://accounts.google.com\",\"sub\":\"110169484474386276334\",\"email\":\"admin@example.com\",\"exp\":1700003600}";
            const char* noneHeaderJson = "{\"alg\":\"none\",\"typ\":\"JWT\"}";
            ASSERT_TRUE( CORE::Base64UrlEncode( otherPayloadJson, (UInt32) strlen( otherPayloadJson ), otherPayload ) );
            ASSERT_TRUE( CORE::Base64UrlEncode( noneHeaderJson, (UInt32) strlen( noneHeaderJson ), noneHeader ) );
            CORE::CString swapped = CORE::CString( token.C_String(), headerLength ) + "." + otherPayload;
            ASSERT_FALSE( RsaVerify( codec, FromText( swapped.C_String() ), n, e, signature ) );
            CORE::CString noneToken = noneHeader + CORE::CString( firstDot, (UInt32) ( lastDot - firstDot ) );
            ASSERT_FALSE( RsaVerify( codec, FromText( noneToken.C_String() ), n, e, signature ) );

            // the token cut short: the signing input and the signature both
            CORE::CDynamicBuffer cutInput( signingInput );
            cutInput.SetDataSize( signingInput.GetDataSize() - 1 );
            ASSERT_FALSE( RsaVerify( codec, cutInput, n, e, signature ) );
            for ( UInt32 cut=1; cut<=3; ++cut )
            {
                CORE::CDynamicBuffer cutSignature( signature );
                cutSignature.SetDataSize( 256 - cut );
                ASSERT_FALSE( RsaVerify( codec, signingInput, n, e, cutSignature ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            }
            ASSERT_FALSE( RsaVerify( codec, signingInput, n, e, CORE::CDynamicBuffer() ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() || CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );

            // a signature longer than the modulus: one byte more, whatever it is
            for ( UInt32 extra=0; extra<2; ++extra )
            {
                CORE::CDynamicBuffer longer;
                const UInt8 extraByte = (UInt8) ( 0 == extra ? 0x00 : 0xFF );
                longer.Append( &extraByte, 1, true );
                longer.Append( signature );
                ASSERT_FALSE( RsaVerify( codec, signingInput, n, e, longer ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
                CORE::CDynamicBuffer trailing( signature );
                trailing.Append( &extraByte, 1, true );
                ASSERT_FALSE( RsaVerify( codec, signingInput, n, e, trailing ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            }

            // the key of another pair
            ASSERT_FALSE( RsaVerify( codec, signingInput, modulusB, e, signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );

            // the same key written with leading zero bytes, as an ASN.1 integer would
            CORE::CDynamicBuffer paddedModulus = FromHex( "0000" );
            paddedModulus.Append( n );
            CORE::CDynamicBuffer paddedExponent = FromHex( "000000010001" );
            ASSERT_TRUE( RsaVerify( codec, signingInput, paddedModulus, paddedExponent, signature ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Public keys that are not acceptable are refused: exponent zero, even, too small or too large, a modulus that is small, even, empty or too large" )
        try
        {
            TICodecPtr codec = GetRsaCodec( "rsa-pkcs1-sha256" );
            ASSERT_TRUE( !codec.IsNULL() );
            REQUIRE_CODEC( codec );
            const CORE::CDynamicBuffer signature = FromHex( g_rsaKeyASignatureSha256Hex );
            ASSERT_TRUE( RsaVerify( codec, message, modulusA, e65537, signature ) );

            // the exponent
            const char* badExponents[] =
            {
                "00",                   // zero
                "0000",                 // zero, longer
                "01",                   // one
                "02",                   // even
                "03",                   // odd but below 65537
                "010000",               // 65536, even
                "010002",               // even
                "fffffffe",             // even
                "010000000000000001",   // 2^64 + 1: more than 64 bits
                "ffffffffffffffffff",   // 72 bits
                "0100000000000000000000000000000000000000000000000000000000000000001"   // 33 bytes
            };
            for ( UInt32 i=0; i<sizeof( badExponents ) / sizeof( badExponents[ 0 ] ); ++i )
            {
                CORE::CDynamicBuffer exponent = FromHex( strlen( badExponents[ i ] ) % 2 ? ( CORE::CString( "0" ) + badExponents[ i ] ).C_String() : badExponents[ i ] );
                ASSERT_FALSE( RsaVerify( codec, message, modulusA, exponent, signature ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            }
            ASSERT_FALSE( RsaVerify( codec, message, modulusA, CORE::CDynamicBuffer(), signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() || CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );

            // exponents that are fine as exponents but are not the one of the signature, the largest 64 bit odd one included
            const char* otherExponents[] = { "010003", "ffffffff", "ffffffffffffffff", "0000000000010001ff" };
            for ( UInt32 i=0; i<sizeof( otherExponents ) / sizeof( otherExponents[ 0 ] ); ++i )
            {
                ASSERT_FALSE( RsaVerify( codec, message, modulusA, FromHex( otherExponents[ i ] ), signature ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() || CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            }

            // the modulus: empty, zero, one, even, small, too large
            ASSERT_FALSE( RsaVerify( codec, message, CORE::CDynamicBuffer(), e65537, signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() || CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
            ASSERT_FALSE( RsaVerify( codec, message, FromHex( "00" ), e65537, signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            ASSERT_FALSE( RsaVerify( codec, message, FromHex( "01" ), e65537, signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            ASSERT_FALSE( RsaVerify( codec, message, FromHex( "0000" ), e65537, signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );

            CORE::CDynamicBuffer evenModulus = FlippedBit( modulusA, 255, 0x01 );
            ASSERT_FALSE( RsaVerify( codec, message, evenModulus, e65537, signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            ASSERT_FALSE( RsaVerify( codec, message, evenModulus, e65537, signature, true ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );

            // a modulus of 2047 bits is below the minimum, one of 2048 is not
            CORE::CDynamicBuffer shortModulus = FromHex( g_rsaKeyAModulusHex );
            shortModulus.AsTypePtr< UInt8 >()[ 0 ] &= 0x7F;
            shortModulus.AsTypePtr< UInt8 >()[ 0 ] |= 0x40;
            ASSERT_FALSE( RsaVerify( codec, message, shortModulus, e65537, signature ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );

            // a 1024 bit key is refused, unless the switch for short keys is set
            const CORE::CDynamicBuffer modulus1024 = FromHex( g_rsaKey1024ModulusHex );
            const CORE::CDynamicBuffer signature1024 = FromHex( g_rsaKey1024Signature256Hex );
            ASSERT_FALSE( RsaVerify( codec, message, modulus1024, e65537, signature1024 ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            ASSERT_TRUE( RsaVerify( codec, message, modulus1024, e65537, signature1024, true ) );
            CORE::CValueList switchOff = RsaParams( modulus1024, e65537, signature1024 );
            switchOff.Set( "allowShortKey", CORE::CVariant( false ) );
            CORE::CDynamicBuffer output;
            ASSERT_FALSE( RunOneShot( codec, false, message, switchOff, output ) );
            ASSERT_FALSE( RsaVerify( codec, FromText( "another message" ), modulus1024, e65537, signature1024, true ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );

            // the switch has a floor: 512 bits. A 511 bit modulus is refused with it, as is the 2048 bit one with 8200 bits
            CORE::CDynamicBuffer modulus511 = Repeated( 0xFF, 64 );
            modulus511.AsTypePtr< UInt8 >()[ 0 ] = 0x7F;
            ASSERT_FALSE( RsaVerify( codec, message, modulus511, e65537, FromHex( "00" ), true ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            CORE::CDynamicBuffer modulusHuge = Repeated( 0xFF, 1026 );
            ASSERT_FALSE( RsaVerify( codec, message, modulusHuge, e65537, Repeated( 0x01, 1026 ) ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            ASSERT_FALSE( RsaVerify( codec, message, modulusHuge, e65537, Repeated( 0x01, 1026 ), true ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );

            // the largest accepted size, 8192 bits, is a key; a signature that is not valid for it is refused on its merit
            CORE::CDynamicBuffer modulus8192 = Repeated( 0xFF, 1024 );
            ASSERT_FALSE( RsaVerify( codec, message, modulus8192, e65537, Repeated( 0x01, 1024 ) ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );

            // required parameters
            CORE::CDynamicBuffer out;
            CORE::CValueList noModulus;
            noModulus.Set( "exponent", CORE::CVariant( e65537 ) );
            noModulus.Set( "signature", CORE::CVariant( signature ) );
            ASSERT_FALSE( RunOneShot( codec, false, message, noModulus, out ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
            CORE::CValueList noExponent;
            noExponent.Set( "modulus", CORE::CVariant( modulusA ) );
            noExponent.Set( "signature", CORE::CVariant( signature ) );
            ASSERT_FALSE( RunOneShot( codec, false, message, noExponent, out ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
            CORE::CValueList noSignature;
            noSignature.Set( "modulus", CORE::CVariant( modulusA ) );
            noSignature.Set( "exponent", CORE::CVariant( e65537 ) );
            ASSERT_FALSE( RunOneShot( codec, false, message, noSignature, out ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
            ASSERT_FALSE( RunOneShot( codec, false, message, CORE::CValueList(), out ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == codec->GetLastStatus() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A signature that is not below the modulus, or is zero, or has the wrong length, is refused" )
        try
        {
            TICodecPtr codec = GetRsaCodec( "rsa-pkcs1-sha256" );
            ASSERT_TRUE( !codec.IsNULL() );
            REQUIRE_CODEC( codec );

            // equal to the modulus, above it by one and by a lot, all ones, zero, one
            CORE::CDynamicBuffer aboveByOne( modulusA );
            aboveByOne.AsTypePtr< UInt8 >()[ 255 ] += 1;
            const CORE::CDynamicBuffer candidates[ 6 ] = { modulusA, aboveByOne, Repeated( 0xFF, 256 ), Repeated( 0x00, 256 ), FromHex( "00" ), Repeated( 0x00, 255 ) };
            for ( UInt32 i=0; i<6; ++i )
            {
                ASSERT_FALSE( RsaVerify( codec, message, modulusA, e65537, candidates[ i ] ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() || CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
            }
            CORE::CDynamicBuffer one = Repeated( 0x00, 255 );
            one.Append( "\x01", 1, true );
            ASSERT_FALSE( RsaVerify( codec, message, modulusA, e65537, one ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );
            ASSERT_FALSE( RsaVerify( codec, message, modulusA, e65537, modulusA ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The padding is checked in full: other block types, padding bytes, DigestInfo forms, trailing bytes, a short padding and a missing separator are refused" )
        try
        {
            TICodecPtr codec = GetRsaCodec( "rsa-pkcs1-sha256" );
            ASSERT_TRUE( !codec.IsNULL() );
            REQUIRE_CODEC( codec );

            // control: the encoding the tests below start from is accepted, and is the one of the independent implementation
            ASSERT_TRUE( RsaVerify( codec, message, modulusA, e65537, FromHex( g_rsaCraftedControlHex ) ) );
            ASSERT_TRUE( g_rsaCraftedControlHex == ToHex( FromHex( g_rsaKeyASignatureSha256Hex ) ) );

            const char* crafted[] =
            {
                g_rsaCraftedBlockType2Hex,
                g_rsaCraftedPaddingByteChangedHex,
                g_rsaCraftedNoNullParamsHex,
                g_rsaCraftedTrailingGarbageHex,
                g_rsaCraftedShortPaddingHex,
                g_rsaCraftedLeadingOneHex,
                g_rsaCraftedNoZeroSeparatorHex,
                g_rsaCraftedWrongDigestLengthHex
            };
            for ( UInt32 i=0; i<sizeof( crafted ) / sizeof( crafted[ 0 ] ); ++i )
            {
                CORE::CDynamicBuffer signature = FromHex( crafted[ i ] );
                ASSERT_TRUE( 256 == signature.GetDataSize() );
                ASSERT_FALSE( RsaVerify( codec, message, modulusA, e65537, signature ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );
                if ( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED != codec->GetLastStatus() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Crafted signature " << i << " was not refused for the right reason" );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: RS384 and RS512 verify signatures of an independent implementation, 2048, 3072 and 4096 bit keys, and a signature of one hash is refused by the others" )
        try
        {
            TICodecPtr sha256 = GetRsaCodec( "rsa-pkcs1-sha256" );
            TICodecPtr sha384 = GetRsaCodec( "rsa-pkcs1-sha384" );
            TICodecPtr sha512 = GetRsaCodec( "rsa-pkcs1-sha512" );
            ASSERT_TRUE( !sha256.IsNULL() && !sha384.IsNULL() && !sha512.IsNULL() );
            REQUIRE_CODEC( sha256 );
            REQUIRE_CODEC( sha384 );
            REQUIRE_CODEC( sha512 );

            const CORE::CDynamicBuffer sig256 = FromHex( g_rsaKeyASignatureSha256Hex );
            const CORE::CDynamicBuffer sig384 = FromHex( g_rsaKeyASignatureSha384Hex );
            const CORE::CDynamicBuffer sig512 = FromHex( g_rsaKeyASignatureSha512Hex );
            ASSERT_TRUE( RsaVerify( sha256, message, modulusA, e65537, sig256 ) );
            ASSERT_TRUE( RsaVerify( sha384, message, modulusA, e65537, sig384 ) );
            ASSERT_TRUE( RsaVerify( sha512, message, modulusA, e65537, sig512 ) );

            ASSERT_FALSE( RsaVerify( sha256, message, modulusA, e65537, sig384 ) );
            ASSERT_FALSE( RsaVerify( sha256, message, modulusA, e65537, sig512 ) );
            ASSERT_FALSE( RsaVerify( sha384, message, modulusA, e65537, sig256 ) );
            ASSERT_FALSE( RsaVerify( sha384, message, modulusA, e65537, sig512 ) );
            ASSERT_FALSE( RsaVerify( sha512, message, modulusA, e65537, sig256 ) );
            ASSERT_FALSE( RsaVerify( sha512, message, modulusA, e65537, sig384 ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == sha512->GetLastStatus() );

            for ( UInt32 i=0; i<256; i+=5 )
            {
                ASSERT_FALSE( RsaVerify( sha384, message, modulusA, e65537, FlippedBit( sig384, i, 0x10 ) ) );
                ASSERT_FALSE( RsaVerify( sha512, message, modulusA, e65537, FlippedBit( sig512, i, 0x10 ) ) );
            }
            ASSERT_FALSE( RsaVerify( sha384, FromText( "RSA PKCS1 v1.5 test messagE" ), modulusA, e65537, sig384 ) );
            ASSERT_FALSE( RsaVerify( sha512, FromText( "RSA PKCS1 v1.5 test messagE" ), modulusA, e65537, sig512 ) );

            // other key sizes
            ASSERT_TRUE( RsaVerify( sha384, message, FromHex( g_rsaKey3072ModulusHex ), e65537, FromHex( g_rsaKey3072Signature384Hex ) ) );
            ASSERT_FALSE( RsaVerify( sha256, message, FromHex( g_rsaKey3072ModulusHex ), e65537, FromHex( g_rsaKey3072Signature384Hex ) ) );
            ASSERT_TRUE( RsaVerify( sha256, message, FromHex( g_rsaKey4096ModulusHex ), e65537, FromHex( g_rsaKey4096Signature256Hex ) ) );
            CORE::CDynamicBuffer sig4096 = FromHex( g_rsaKey4096Signature256Hex );
            ASSERT_TRUE( 512 == sig4096.GetDataSize() );
            ASSERT_FALSE( RsaVerify( sha256, message, FromHex( g_rsaKey4096ModulusHex ), e65537, FlippedBit( sig4096, 511, 0x01 ) ) );
            ASSERT_FALSE( RsaVerify( sha256, message, FromHex( g_rsaKey4096ModulusHex ), e65537, FlippedBit( sig4096, 0, 0x01 ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An instance keeps its key for many messages, takes the message in pieces, takes the signature per call and does not allocate per call" )
        try
        {
            TICodecPtr codec = GetRsaCodec( "rsa-pkcs1-sha256" );
            ASSERT_TRUE( !codec.IsNULL() );
            REQUIRE_CODEC( codec );

            CORE::CDynamicBuffer n, e, signature;
            ASSERT_TRUE( CORE::Base64UrlDecode( g_rfc7515A2Modulus, n ) && CORE::Base64UrlDecode( "AQAB", e ) && CORE::Base64UrlDecode( g_rfc7515A2Signature, signature ) );
            CORE::CDynamicBuffer signingInput = FromText( g_rfc7515A2SigningInput );
            const CORE::CDynamicBuffer sigA = FromHex( g_rsaKeyASignatureSha256Hex );

            // key and signature at creation
            CORE::CICodecInstancePtr instance;
            ASSERT_TRUE( codec->CreateInstance( RsaParams( n, e, signature ), instance ) && !instance.IsNULL() );
            REQUIRE_CODEC( instance );
            ASSERT_TRUE( 0 == instance->GetOverhead() );

            CORE::CDynamicBuffer output;
            ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput.GetConstBufferPtr(), signingInput.GetDataSize(), GUCEF_NULL, output ) );
            ASSERT_TRUE( 0 == output.GetDataSize() && CORE::GUCEF_CODEC_STATUS_OK == instance->GetLastStatus() );

            // a refused message does not spoil the next one
            ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, "something else", 14, GUCEF_NULL, output ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == instance->GetLastStatus() );
            ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput.GetConstBufferPtr(), signingInput.GetDataSize(), GUCEF_NULL, output ) );

            // the message in pieces of every size, the last piece carrying FINAL
            const UInt8* data = static_cast< const UInt8* >( signingInput.GetConstBufferPtr() );
            const UInt32 total = signingInput.GetDataSize();
            for ( UInt32 piece=1; piece<=total; piece+=7 )
            {
                UInt32 offset = 0;
                UInt32 flags = GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_DECODE;
                bool ok = true;
                while ( total - offset > piece )
                {
                    ok = instance->Process( flags, data + offset, piece, GUCEF_NULL, output ) && ok;
                    offset += piece;
                    flags = GUCEF_CODEC_PROCESS_UPDATE | GUCEF_CODEC_PROCESS_DECODE;
                }
                ok = instance->Process( flags | GUCEF_CODEC_PROCESS_FINAL, data + offset, total - offset, GUCEF_NULL, output ) && ok;
                ASSERT_TRUE( ok );
                ASSERT_TRUE( 0 == output.GetDataSize() );
            }

            // BEGIN drops a message that was not finished
            ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_DECODE, "unfinished", 10, GUCEF_NULL, output ) );
            ASSERT_TRUE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput.GetConstBufferPtr(), total, GUCEF_NULL, output ) );

            // the encode direction is refused cleanly
            ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, signingInput.GetConstBufferPtr(), total, GUCEF_NULL, output ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED == instance->GetLastStatus() );
            ASSERT_TRUE( 0 == output.GetDataSize() );
            ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_DECODE | GUCEF_CODEC_PROCESS_ENCODE | GUCEF_CODEC_PROCESS_FINAL, "x", 1, GUCEF_NULL, output ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == instance->GetLastStatus() );

            // a key without a signature: the signature comes per call as the call argument "aad", the one of the call wins
            CORE::CValueList keyOnly;
            keyOnly.Set( "modulus", CORE::CVariant( n ) );
            keyOnly.Set( "exponent", CORE::CVariant( e ) );
            CORE::CICodecInstancePtr keyInstance;
            ASSERT_TRUE( codec->CreateInstance( keyOnly, keyInstance ) && !keyInstance.IsNULL() );
            REQUIRE_CODEC( keyInstance );
            ASSERT_FALSE( keyInstance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput.GetConstBufferPtr(), total, GUCEF_NULL, output ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_MISSING_PARAMETER == keyInstance->GetLastStatus() );

            CORE::SCodecCallArgs args;
            memset( &args, 0, sizeof( args ) );
            args.structSize = sizeof( args );
            args.aad = signature.GetConstBufferPtr();
            args.aadSize = signature.GetDataSize();
            ASSERT_TRUE( keyInstance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput.GetConstBufferPtr(), total, &args, output ) );
            args.aad = sigA.GetConstBufferPtr();
            ASSERT_FALSE( keyInstance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput.GetConstBufferPtr(), total, &args, output ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == keyInstance->GetLastStatus() );
            args.aadSize = 100;
            ASSERT_FALSE( keyInstance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput.GetConstBufferPtr(), total, &args, output ) );
            ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == keyInstance->GetLastStatus() );

            CORE::CICodecInstancePtr withBoth;
            ASSERT_TRUE( codec->CreateInstance( RsaParams( n, e, signature ), withBoth ) && !withBoth.IsNULL() );
            args.aad = sigA.GetConstBufferPtr();
            args.aadSize = sigA.GetDataSize();
            if ( !withBoth.IsNULL() )
                ASSERT_FALSE( withBoth->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput.GetConstBufferPtr(), total, &args, output ) );

            // no allocation per call, with the signature at creation and per call
            if ( CORETEST::IsAllocationCountingAvailable() )
            {
                bool succeeded = false;
                ASSERT_TRUE( 0 == CountAllocationsOfProcess( instance, GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput, GUCEF_NULL, succeeded ) );
                ASSERT_TRUE( succeeded );
                args.aad = signature.GetConstBufferPtr();
                args.aadSize = signature.GetDataSize();
                ASSERT_TRUE( 0 == CountAllocationsOfProcess( keyInstance, GUCEF_CODEC_PROCESS_DECODE_MESSAGE, signingInput, &args, succeeded ) );
                ASSERT_TRUE( succeeded );

                // and a refused signature does not allocate either
                CORE::CDynamicBuffer wrongMessage = FromText( "not what was signed" );
                CORE::CDynamicBuffer scratch;
                keyInstance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, wrongMessage.GetConstBufferPtr(), wrongMessage.GetDataSize(), &args, scratch );
                CORETEST::StartAllocationCounting();
                for ( UInt32 i=0; i<50; ++i )
                    keyInstance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, wrongMessage.GetConstBufferPtr(), wrongMessage.GetDataSize(), &args, scratch );
                ASSERT_TRUE( 0 == CORETEST::StopAllocationCounting() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Verifying with a 2048 and a 4096 bit key and the exponent 65537 takes well under 10 milliseconds, key set up included" )
        try
        {
            TICodecPtr codec = GetRsaCodec( "rsa-pkcs1-sha256" );
            ASSERT_TRUE( !codec.IsNULL() );
            REQUIRE_CODEC( codec );

            struct SKeyCase { const char* modulusHex; const char* signatureHex; UInt32 rounds; };
            const SKeyCase cases[ 2 ] =
            {
                { g_rsaKeyAModulusHex, g_rsaKeyASignatureSha256Hex, 50 },
                { g_rsaKey4096ModulusHex, g_rsaKey4096Signature256Hex, 20 }
            };
            for ( UInt32 c=0; c<2; ++c )
            {
                const CORE::CDynamicBuffer modulus = FromHex( cases[ c ].modulusHex );
                const CORE::CDynamicBuffer signature = FromHex( cases[ c ].signatureHex );
                const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;

                // one shot: a new key set up for every verification
                UInt64 start = MT::PrecisionTickCount();
                bool allVerified = true;
                for ( UInt32 i=0; i<cases[ c ].rounds; ++i )
                    allVerified = RsaVerify( codec, message, modulus, e65537, signature ) && allVerified;
                Float64 oneShotMs = ( MT::PrecisionTickCount() - start ) / ticksPerMs / cases[ c ].rounds;
                ASSERT_TRUE( allVerified );

                // an instance with its key set up once
                CORE::CICodecInstancePtr instance;
                ASSERT_TRUE( codec->CreateInstance( RsaParams( modulus, e65537, signature ), instance ) && !instance.IsNULL() );
                REQUIRE_CODEC( instance );
                CORE::CDynamicBuffer output;
                start = MT::PrecisionTickCount();
                for ( UInt32 i=0; i<cases[ c ].rounds; ++i )
                    allVerified = instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, message.GetConstBufferPtr(), message.GetDataSize(), GUCEF_NULL, output ) && allVerified;
                Float64 instanceMs = ( MT::PrecisionTickCount() - start ) / ticksPerMs / cases[ c ].rounds;
                ASSERT_TRUE( allVerified );

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RSA " << modulus.GetDataSize() * 8 << " bit verification: " << oneShotMs << " ms one shot including key set up, " << instanceMs << " ms with a prepared instance" );
                ASSERT_TRUE( oneShotMs < 10.0 );
                ASSERT_TRUE( instanceMs < 10.0 );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformCryptoCodecTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Crypto Codec TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CryptoCodecs" );

    // the crypto plugins, so every provider of a type is put to the same tests
    if ( !LoadCryptoPlugin( "codecspluginMONOCYPHER" ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Crypto codec TESTS: codecspluginMONOCYPHER is not available" );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    if ( !LoadCryptoPlugin( "codecspluginOPENSSL" ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Crypto codec TESTS: codecspluginOPENSSL is not available" );
    #endif

    PerformCodecRegistryTests();
    PerformSoftwareCryptoPrimitiveTests();
    PerformHashProviderTests();
    PerformAeadProviderTests();
    PerformKeyAgreementAndSignatureTests();
    PerformPluginHashAndKdfTests();
    PerformInstanceAllocationTests();
    PerformBuiltInCodecTests();
    PerformSoftwareSha512Tests();
    PerformRsaSignatureTests();

    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( CryptoPluginGroup );

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Crypto Codec TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
