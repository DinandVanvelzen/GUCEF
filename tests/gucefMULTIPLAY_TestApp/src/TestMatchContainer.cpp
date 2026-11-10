/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#include "TestMatchSupport.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREECODECTYPES_H
#endif /* GUCEF_CORE_COREECODECTYPES_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#include "TestMatchCommon.h"
#include "TestMatchContainer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A signed first turn of a small match and the package it names */
struct SContainerSample
{
    GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
    MULTIPLAY::CMatchInfo info;
    MULTIPLAY::CMatchPackage package;
    MULTIPLAY::CMatchPreview preview;

    bool Make( UInt32 payloadSize, bool compressible )
    {
        ids.clear();
        for ( int i=0; i<3; ++i )
        {
            MULTIPLAY::CPlayerIdentity identity;
            if ( !TESTFW::MakeTestIdentity( i == 0 ? "Alice" : ( i == 1 ? "Bob" : "Carol" ), identity ) )
                return false;
            ids.push_back( identity );
        }
        info = TESTFW::MakeTestMatchInfo( ids, "container", true );
        SetPayload( 1, GUCEF_NULL, payloadSize, compressible );
        return true;
    }

    /** A payload like the saved game of a galaxy, of the size, from the seed: records of 32 bytes of which about one field in seven holds a number, the rest is zero; it compresses about nine times */
    void SetRecordPayload( UInt32 payloadSize, UInt32 seed )
    {
        CORE::CDynamicBuffer payload;
        payload.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
        CTestRandom random( seed );
        while ( payload.GetDataSize() < payloadSize )
        {
            UInt32 record[ 8 ];
            for ( UInt32 f=0; f<8; ++f )
                record[ f ] = 0 == random.Next() % 7 ? 1 + random.Next() % 999 : 0;
            UInt32 count = payloadSize - payload.GetDataSize();
            payload.Append( record, count > sizeof( record ) ? (UInt32) sizeof( record ) : count );
        }
        package = MULTIPLAY::CMatchPackage();
        package.SetPayload( payload.GetConstBufferPtr(), payload.GetDataSize() );
        package.revision = 1;
        preview = TESTFW::MakeTestPreview( info, ids[ 0 ], package, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 5000 );
    }

    void SetPayload( UInt32 revision, const MULTIPLAY::CMatchPackage* previous, UInt32 payloadSize, bool compressible )
    {
        // the previous package may be the one that is replaced here
        MULTIPLAY::CMatchPackage base;
        if ( GUCEF_NULL != previous )
            base = *previous;
        CORE::CDynamicBuffer payload;
        payload.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
        CTestRandom random( 99 + revision );
        for ( UInt32 i=0; i<payloadSize; ++i )
        {
            UInt8 b = compressible ? (UInt8)( 'a' + ( i / 7 ) % 5 ) : (UInt8)( random.Next() & 0xFF );
            payload.Append( &b, 1 );
        }
        package = MULTIPLAY::CMatchPackage();
        package.SetPayload( payload.GetConstBufferPtr(), payload.GetDataSize() );
        package.revision = revision;
        if ( GUCEF_NULL != previous )
        {
            package.baseRevision = base.revision;
            package.baseSha256 = base.sha256;
        }
        preview = TESTFW::MakeTestPreview( info, ids[ 0 ], package, &ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 5000 );
    }
};

/*-------------------------------------------------------------------------*/

static bool
BuffersAreEqual( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{GUCEF_TRACE;

    return a.GetDataSize() == b.GetDataSize() && ( 0 == a.GetDataSize() || 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

/** Whether the decoded container is exactly the sample, for the check that nothing that was changed is accepted as valid */
static bool
IsSameAsSample( const MULTIPLAY::CMatchContainer& c, const SContainerSample& sample )
{GUCEF_TRACE;

    CORE::CDynamicBuffer a, b;
    c.preview.ToBinary( a );
    sample.preview.ToBinary( b );
    return BuffersAreEqual( a, b ) && BuffersAreEqual( c.package.payload, sample.package.payload ) && c.package.sha256 == sample.package.sha256 &&
           c.package.revision == sample.package.revision && c.package.baseRevision == sample.package.baseRevision && c.package.baseSha256 == sample.package.baseSha256;
}

/*-------------------------------------------------------------------------*/

/** Gzip of the bytes by the codec, no limits: for building files that are hostile */
static bool
GzipBytes( const CORE::CDynamicBuffer& raw, CORE::CDynamicBuffer& compressed )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtr codec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip" );
    CORE::CICodecInstancePtr instance;
    if ( codec.IsNULL() || !codec->CreateInstance( CORE::CValueList(), instance ) || instance.IsNULL() )
        return false;
    compressed.Clear();
    return instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, raw.GetConstBufferPtr(), raw.GetDataSize(), GUCEF_NULL, compressed );
}

/*-------------------------------------------------------------------------*/

/** A container made by hand, for what the encoder would never write */
static void
CraftContainer( CORE::CDynamicBuffer& bytes, UInt8 flags, UInt32 rawSize, const CORE::CDynamicBuffer& stored, const UInt8* payloadSha, const MULTIPLAY::CMatchPreview& preview, UInt32 revision )
{GUCEF_TRACE;

    CORE::CDynamicBuffer previewBytes;
    preview.ToBinary( previewBytes );
    UInt8 zero[ 32 ];
    memset( zero, 0, sizeof( zero ) );
    bytes.Clear();
    MULTIPLAY::CMatchBinaryWriter writer( bytes );
    writer.WriteRaw( "GMTC", 4 );
    writer.WriteUInt8( 1 );
    writer.WriteUInt8( flags );
    writer.WriteUInt16( (UInt16) previewBytes.GetDataSize() );
    writer.WriteUInt32( stored.GetDataSize() );
    writer.WriteUInt32( rawSize );
    writer.WriteUInt32( revision );
    writer.WriteUInt32( 0 );
    writer.WriteRaw( zero, 32 );
    writer.WriteRaw( payloadSha, 32 );
    writer.WriteRaw( previewBytes.GetConstBufferPtr(), previewBytes.GetDataSize() );
    writer.WriteRaw( stored.GetConstBufferPtr(), stored.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** Text is appended to a buffer: a string is rescanned on every change, which is far too slow for hundreds of KB */
static void
AppendText( CORE::CDynamicBuffer& buffer, const char* text )
{GUCEF_TRACE;

    buffer.Append( text, (UInt32) strlen( text ) );
}

/*-------------------------------------------------------------------------*/

static CORE::CString::StringVector
SplitLines( const CORE::CString& text )
{GUCEF_TRACE;

    return text.ParseElements( '\n', true );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
JoinLines( const CORE::CString::StringVector& lines, const char* lineEnd )
{GUCEF_TRACE;

    CORE::CString result;
    for ( size_t i=0; i<lines.size(); ++i )
    {
        result += lines[ i ];
        if ( i + 1 < lines.size() )
            result += lineEnd;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/** What a mail client does when a text is quoted in a reply */
static CORE::CString
QuoteAll( const CORE::CString& text, const char* prefix )
{GUCEF_TRACE;

    CORE::CString::StringVector lines = SplitLines( text );
    for ( size_t i=0; i<lines.size(); ++i )
        lines[ i ] = CORE::CString( prefix ) + lines[ i ];
    return JoinLines( lines, "\n" );
}

/*-------------------------------------------------------------------------*/

/** Folds the lines of data to another width, the header and marker lines stay */
static CORE::CString
RefoldData( const CORE::CString& text, UInt32 width )
{GUCEF_TRACE;

    CORE::CString::StringVector lines = SplitLines( text );
    CORE::CString::StringVector result;
    CORE::CString data;
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString& line = lines[ i ];
        bool isData = !line.IsNULLOrEmpty() && line.HasChar( ':' ) < 0 && 0 != line.HasSubstr( "-----" );
        if ( isData )
            data += line;
        else
        {
            if ( !data.IsNULLOrEmpty() )
            {
                for ( UInt32 p=0; p<data.Length(); p+=width )
                    result.push_back( data.SubstrFromRange( p, p + width < data.Length() ? p + width : data.Length() ) );
                data.Clear();
            }
            result.push_back( line );
        }
    }
    return JoinLines( result, "\n" );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
AddTrailingBlanks( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString::StringVector lines = SplitLines( text );
    for ( size_t i=0; i<lines.size(); ++i )
        lines[ i ] += ( i % 2 ) ? "  " : "\t ";
    return JoinLines( lines, "\n" );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Replace( const CORE::CString& text, const char* what, const char* with )
{GUCEF_TRACE;

    return text.ReplaceSubstr( what, with );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMatchContainerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMULTIPLAY matches: container and text armor" );

    // Before the gzip codec is loaded: what happens without one
    GUCEF_TESTFW_TESTCASE( "Test: Without a gzip codec the payload is stored and a gzip payload is refused with a clear result" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        GUCEF_TESTFW_SKIP_IF( MULTIPLAY::CMatchContainer::IsCompressionAvailable(), "A gzip codec is loaded already" );
        try
        {
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 400, true ) );
            CORE::CDynamicBuffer bytes;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, true, bytes ) );
            ASSERT_TRUE( 0 == ( ((const UInt8*) bytes.GetConstBufferPtr())[ 5 ] & 1 ) );               // the flags: stored
            MULTIPLAY::CMatchContainer decoded;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == decoded.Decode( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) && IsSameAsSample( decoded, sample ) );

            // a container from a machine that has the codec
            UInt8 sha[ 32 ];
            DRN::CDRNCrypto::FromHex( sample.package.sha256, sha, 32 );
            CORE::CDynamicBuffer stored;
            stored.Append( "not really gzip, but flagged as such", 36 );
            CORE::CDynamicBuffer crafted;
            CraftContainer( crafted, 1, 400, stored, sha, sample.preview, 1 );
            MULTIPLAY::CMatchContainer other;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_CODEC_UNAVAILABLE == other.Decode( crafted.GetConstBufferPtr(), crafted.GetDataSize() ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Containers of many sizes like the saved game of a galaxy read back exactly after gzip, whatever the size" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            GUCEF_TESTFW_SKIP_IF( !GzipCodecAvailable(), "There is no gzip codec" );
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 100, true ) );
            UInt32 failures = 0;
            CTestRandom sizes( 7 );
            for ( UInt32 round=0; round<80; ++round )
            {
                const UInt32 size = 100000 + sizes.Next() % 300000;
                sample.SetRecordPayload( size, 1000 + round );
                CORE::CDynamicBuffer compressed;
                MULTIPLAY::CMatchContainer decoded;
                if ( MULTIPLAY::MATCHCONTAINER_OK != MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, true, compressed ) ||
                     MULTIPLAY::MATCHCONTAINER_OK != decoded.Decode( compressed.GetConstBufferPtr(), compressed.GetDataSize() ) || !IsSameAsSample( decoded, sample ) )
                {
                    ++failures;
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Container round " << round << " of " << size << " bytes did not read back" );
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A container holds the signed preview and the package, stored or gzip, and reads back exactly" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            bool gzip = GzipCodecAvailable();
            SContainerSample first;
            REQUIRE_TRUE( first.Make( 4000, true ) );
            SContainerSample second = first;
            second.SetPayload( 2, &first.package, 4000, true );

            for ( int variant=0; variant<2; ++variant )
            {
                const SContainerSample& sample = 0 == variant ? first : second;
                CORE::CDynamicBuffer stored, compressed;
                ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, false, stored ) );
                ASSERT_TRUE( stored.GetDataSize() > sample.package.payload.GetDataSize() );
                MULTIPLAY::CMatchContainer decoded;
                ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == decoded.Decode( stored.GetConstBufferPtr(), stored.GetDataSize() ) );
                ASSERT_TRUE( IsSameAsSample( decoded, sample ) && decoded.preview.VerifySignature() && decoded.package.IsConsistent() );

                if ( gzip )
                {
                    ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, true, compressed ) );
                    ASSERT_TRUE( compressed.GetDataSize() < stored.GetDataSize() / 2 );
                    MULTIPLAY::CMatchContainer fromGzip;
                    ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == fromGzip.Decode( compressed.GetConstBufferPtr(), compressed.GetDataSize() ) );
                    ASSERT_TRUE( IsSameAsSample( fromGzip, sample ) );
                    // the same input gives the same bytes
                    CORE::CDynamicBuffer again;
                    ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, true, again ) && BuffersAreEqual( again, compressed ) );
                }
            }
            ASSERT_TRUE( 0 != second.package.baseRevision && !second.package.baseSha256.IsNULLOrEmpty() );

            // the encoder refuses what does not belong together
            CORE::CDynamicBuffer bytes;
            MULTIPLAY::CMatchPackage otherPackage = second.package;
            otherPackage.SetPayload( "different", 9 );
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_INCONSISTENT == MULTIPLAY::CMatchContainer::Encode( second.preview, otherPackage, false, bytes ) );
            MULTIPLAY::CMatchPackage wrongRevision = second.package;
            wrongRevision.revision = 9;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_INCONSISTENT == MULTIPLAY::CMatchContainer::Encode( second.preview, wrongRevision, false, bytes ) );
            MULTIPLAY::CMatchPackage baseWithoutRevision = second.package;
            baseWithoutRevision.baseRevision = 0;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_INCONSISTENT == MULTIPLAY::CMatchContainer::Encode( second.preview, baseWithoutRevision, false, bytes ) );
            MULTIPLAY::CMatchPreview unsignedPreview = second.preview;
            unsignedPreview.signature.Clear();
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_BAD_PREVIEW == MULTIPLAY::CMatchContainer::Encode( unsignedPreview, second.package, false, bytes ) );
            ASSERT_TRUE( 0 == bytes.GetDataSize() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The container is capped at 2 MB, and a size above the cap is refused before it is read" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SContainerSample big;
            REQUIRE_TRUE( big.Make( 1900 * 1024, false ) );
            CORE::CDynamicBuffer bytes;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( big.preview, big.package, false, bytes ) );
            ASSERT_TRUE( bytes.GetDataSize() <= MULTIPLAY::CMatchContainer::MaxContainerSize );
            MULTIPLAY::CMatchContainer decoded;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == decoded.Decode( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) && IsSameAsSample( decoded, big ) );

            SContainerSample tooBig;
            REQUIRE_TRUE( tooBig.Make( 2100 * 1024, false ) );
            CORE::CDynamicBuffer none;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_TOO_LARGE == MULTIPLAY::CMatchContainer::Encode( tooBig.preview, tooBig.package, true, none ) );
            ASSERT_TRUE( 0 == none.GetDataSize() );

            // a reader handed more than the cap does not look at it
            CORE::CDynamicBuffer huge;
            huge.SetBufferSize( MULTIPLAY::CMatchContainer::MaxContainerSize + 1 );
            huge.SetDataSize( MULTIPLAY::CMatchContainer::MaxContainerSize + 1 );
            memcpy( huge.GetBufferPtr(), "GMTC", 4 );
            MULTIPLAY::CMatchContainer refused;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_TOO_LARGE == refused.Decode( huge.GetConstBufferPtr(), huge.GetDataSize() ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every truncation of a container is refused, stored and gzip" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 600, true ) );
            sample.SetPayload( 2, &sample.package, 600, true );
            bool gzip = GzipCodecAvailable();
            for ( int variant=0; variant<( gzip ? 2 : 1 ); ++variant )
            {
                CORE::CDynamicBuffer bytes;
                REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, 1 == variant, bytes ) );
                REQUIRE_TRUE( bytes.GetDataSize() < 4096 );
                for ( UInt32 n=0; n<bytes.GetDataSize(); ++n )
                {
                    MULTIPLAY::CMatchContainer c;
                    ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK != c.Decode( bytes.GetConstBufferPtr(), n ) );
                }
                // and bytes behind the end
                CORE::CDynamicBuffer longer = bytes;
                UInt8 extra = 0;
                longer.Append( &extra, 1 );
                MULTIPLAY::CMatchContainer c;
                ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_BAD_HEADER == c.Decode( longer.GetConstBufferPtr(), longer.GetDataSize() ) );
                ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK != c.Decode( GUCEF_NULL, 10 ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A byte of a container that was changed never gives a different valid container, and nothing crashes" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 500, true ) );
            sample.SetPayload( 2, &sample.package, 500, true );
            bool gzip = GzipCodecAvailable();
            for ( int variant=0; variant<( gzip ? 2 : 1 ); ++variant )
            {
                CORE::CDynamicBuffer bytes;
                REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, 1 == variant, bytes ) );
                UInt32 silentlyChanged = 0;
                UInt32 refused = 0;
                for ( UInt32 i=0; i<bytes.GetDataSize(); ++i )
                {
                    for ( int pattern=0; pattern<2; ++pattern )
                    {
                        CORE::CDynamicBuffer copy = bytes;
                        copy.GetUByte( i ) ^= 0 == pattern ? 0xFF : 0x01;
                        MULTIPLAY::CMatchContainer c;
                        if ( MULTIPLAY::MATCHCONTAINER_OK != c.Decode( copy.GetConstBufferPtr(), copy.GetDataSize() ) )
                            ++refused;
                        else
                        if ( c.preview.VerifySignature() && !IsSameAsSample( c, sample ) )
                            ++silentlyChanged;
                    }
                }
                ASSERT_TRUE( 0 == silentlyChanged );
                ASSERT_TRUE( refused > 0 );
            }

            // random garbage and random changes of several bytes: bounded, deterministic
            CORE::CDynamicBuffer valid;
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, gzip, valid ) );
            CTestRandom random( 4242 );
            UInt32 silentlyChanged = 0;
            for ( int round=0; round<1500; ++round )
            {
                CORE::CDynamicBuffer copy = valid;
                UInt32 changes = 1 + random.Next() % 6;
                for ( UInt32 c=0; c<changes; ++c )
                    copy.GetUByte( random.Next() % copy.GetDataSize() ) = (UInt8)( random.Next() & 0xFF );
                MULTIPLAY::CMatchContainer c;
                if ( MULTIPLAY::MATCHCONTAINER_OK == c.Decode( copy.GetConstBufferPtr(), copy.GetDataSize() ) && c.preview.VerifySignature() && !IsSameAsSample( c, sample ) )
                    ++silentlyChanged;
            }
            ASSERT_TRUE( 0 == silentlyChanged );
            for ( int round=0; round<500; ++round )
            {
                CORE::CDynamicBuffer garbage;
                UInt32 size = random.Next() % 3000;
                for ( UInt32 i=0; i<size; ++i )
                {
                    UInt8 b = (UInt8)( random.Next() & 0xFF );
                    garbage.Append( &b, 1 );
                }
                if ( size >= 4 && 0 == round % 2 )
                    memcpy( garbage.GetBufferPtr(), "GMTC", 4 );
                MULTIPLAY::CMatchContainer c;
                ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK != c.Decode( garbage.GetConstBufferPtr(), garbage.GetDataSize() ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A small gzip file cannot make a large expansion: the reader stops at the size in the header" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        GUCEF_TESTFW_SKIP_IF( !GzipCodecAvailable(), "There is no gzip codec" );
        try
        {
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 1000, true ) );
            CORE::CDynamicBuffer thousandZeros;
            for ( int i=0; i<1000; ++i ) { UInt8 z = 0; thousandZeros.Append( &z, 1 ); }
            sample.package = MULTIPLAY::CMatchPackage();
            sample.package.SetPayload( thousandZeros.GetConstBufferPtr(), 1000 );
            sample.package.revision = 1;
            sample.preview = TESTFW::MakeTestPreview( sample.info, sample.ids[ 0 ], sample.package, &sample.ids[ 1 ], MULTIPLAY::MATCHSTATUS_ACTIVE, 1 );
            UInt8 sha[ 32 ];
            DRN::CDRNCrypto::FromHex( sample.package.sha256, sha, 32 );

            // 64 MB of zeros is a few tens of KB of gzip
            CORE::CDynamicBuffer bomb;
            {
                CORE::CDynamicBuffer zeros;
                zeros.SetBufferSize( 64 * 1024 * 1024 );
                zeros.SetDataSize( 64 * 1024 * 1024 );
                memset( zeros.GetBufferPtr(), 0, zeros.GetDataSize() );
                REQUIRE_TRUE( GzipBytes( zeros, bomb ) );
            }
            REQUIRE_TRUE( bomb.GetDataSize() < 200000 );
            CORE::CDynamicBuffer crafted;
            CraftContainer( crafted, 1, 1000, bomb, sha, sample.preview, 1 );
            MULTIPLAY::CMatchContainer c;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_BAD_COMPRESSION == c.Decode( crafted.GetConstBufferPtr(), crafted.GetDataSize() ) );

            // the honest gzip of the same 1000 bytes is read
            CORE::CDynamicBuffer honest;
            REQUIRE_TRUE( GzipBytes( thousandZeros, honest ) );
            CraftContainer( crafted, 1, 1000, honest, sha, sample.preview, 1 );
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == c.Decode( crafted.GetConstBufferPtr(), crafted.GetDataSize() ) );
            // the header says more than the gzip holds
            CraftContainer( crafted, 1, 2000, honest, sha, sample.preview, 1 );
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK != c.Decode( crafted.GetConstBufferPtr(), crafted.GetDataSize() ) );
            // a raw size above the cap
            CraftContainer( crafted, 1, MULTIPLAY::CMatchPackage::MaxPayloadSize + 1, honest, sha, sample.preview, 1 );
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_BAD_HEADER == c.Decode( crafted.GetConstBufferPtr(), crafted.GetDataSize() ) );
            // stored data with sizes that disagree
            CraftContainer( crafted, 0, 999, thousandZeros, sha, sample.preview, 1 );
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_BAD_HEADER == c.Decode( crafted.GetConstBufferPtr(), crafted.GetDataSize() ) );
            // a preview of another revision than the header
            CraftContainer( crafted, 0, 1000, thousandZeros, sha, sample.preview, 5 );
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_INCONSISTENT == c.Decode( crafted.GetConstBufferPtr(), crafted.GetDataSize() ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The text armor has marker lines, a header block, base64 in 76 columns and a checksum, and reads back the same bytes" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 1500, false ) );
            CORE::CDynamicBuffer bytes;
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, false, bytes ) );

            MULTIPLAY::CMatchTextArmor armor( "MATCH TURN" );
            CORE::CString text;
            REQUIRE_TRUE( armor.Encode( sample.info.matchId, 1, bytes.GetConstBufferPtr(), bytes.GetDataSize(), text ) );
            CORE::CString::StringVector lines = SplitLines( text );
            REQUIRE_TRUE( lines.size() > 10 );
            ASSERT_TRUE( "-----BEGIN MATCH TURN-----" == lines[ 0 ] );
            ASSERT_TRUE( ( "Match: " + sample.info.matchId ) == lines[ 1 ] );
            ASSERT_TRUE( "Revision: 1" == lines[ 2 ] );
            ASSERT_TRUE( ( CORE::CString( "Length: " ) + CORE::UInt32ToString( bytes.GetDataSize() ) ) == lines[ 3 ] );
            ASSERT_TRUE( 0 == memcmp( lines[ 4 ].C_String(), "SHA-256: ", 9 ) && 64 + 9 == lines[ 4 ].Length() );
            UInt32 dataLines = 0;
            UInt32 longest = 0;
            bool sawChecksum = false;
            bool sawEnd = false;
            for ( size_t i=5; i<lines.size(); ++i )
            {
                if ( lines[ i ].IsNULLOrEmpty() || 0 == memcmp( lines[ i ].C_String(), "Version", 7 ) )
                    continue;
                if ( 0 == memcmp( lines[ i ].C_String(), "Checksum: ", 10 ) ) { sawChecksum = true; ASSERT_TRUE( 10 + 6 == lines[ i ].Length() ); continue; }
                if ( "-----END MATCH TURN-----" == lines[ i ] ) { sawEnd = true; continue; }
                ++dataLines;
                if ( lines[ i ].Length() > longest )
                    longest = lines[ i ].Length();
            }
            ASSERT_TRUE( sawChecksum && sawEnd && 76 == longest && dataLines == ( ( bytes.GetDataSize() + 2 ) / 3 * 4 + 75 ) / 76 );
            ASSERT_TRUE( text.Length() < MULTIPLAY::CMatchTextArmor::MaxEncodedTextSize );

            // the same container bytes come out of the text as are in the file
            CORE::CDynamicBuffer fromText;
            UInt32 revision = 0;
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( text.C_String(), text.Length(), sample.info.matchId, 1, fromText, &revision ) );
            ASSERT_TRUE( BuffersAreEqual( fromText, bytes ) && 1 == revision );
            MULTIPLAY::CMatchContainer container;
            ASSERT_TRUE( MULTIPLAY::MATCHCONTAINER_OK == container.Decode( fromText.GetConstBufferPtr(), fromText.GetDataSize() ) && IsSameAsSample( container, sample ) );

            // a file with the same bytes is read to the same result
            CORE::CString dir = MatchTestDirectory( "armorfile" );
            CORE::CString path = CORE::CombinePath( dir, "turn.bin" );
            REQUIRE_TRUE( bytes.WriteContentToFile( path, 0, true ) );
            CORE::CDynamicBuffer fromFile;
            REQUIRE_TRUE( fromFile.LoadContentFromFile( path ) );
            ASSERT_TRUE( BuffersAreEqual( fromFile, fromText ) );

            // any match and any revision when none is asked for
            CORE::CDynamicBuffer anyBytes;
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( text.C_String(), text.Length(), CORE::CString::Empty, 0, anyBytes ) && BuffersAreEqual( anyBytes, bytes ) );
            // another label is another text
            MULTIPLAY::CMatchTextArmor other( "OTHER GAME TURN" );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_NO_BLOCK == other.Decode( text.C_String(), text.Length(), CORE::CString::Empty, 0, anyBytes ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The reader of the text copes with what mail does to text" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 900, false ) );
            CORE::CDynamicBuffer bytes;
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, false, bytes ) );
            MULTIPLAY::CMatchTextArmor armor( "MATCH TURN" );
            CORE::CString text;
            REQUIRE_TRUE( armor.Encode( sample.info.matchId, 1, bytes.GetConstBufferPtr(), bytes.GetDataSize(), text ) );

            CORE::CString variants[ 14 ];
            variants[ 0 ] = QuoteAll( text, "> " );
            variants[ 1 ] = QuoteAll( QuoteAll( text, "> " ), "> " );
            variants[ 2 ] = QuoteAll( text, ">" );
            variants[ 3 ] = QuoteAll( text, "   >> > " );
            variants[ 4 ] = AddTrailingBlanks( text );
            variants[ 5 ] = Replace( text, "\n", "\r\n" );
            variants[ 6 ] = Replace( text, "\n", "\r" );
            variants[ 7 ] = RefoldData( text, 60 );
            variants[ 8 ] = RefoldData( text, 20 );
            variants[ 9 ] = RefoldData( text, 200 );
            variants[ 10 ] = "Hi Bob,\n\nmy turn is done, the game is below. Open it from the game.\n\n" + text + "\nThanks! See you on Friday.\n-- \nAlice\n";
            variants[ 11 ] = "On Tue, 3 Nov 2020 Alice wrote:\n" + QuoteAll( text, "> " ) + "\nGreat, my reply follows after the quote\n";
            variants[ 12 ] = Replace( QuoteAll( AddTrailingBlanks( RefoldData( text, 64 ) ), "> " ), "\n", "\r\n" );
            variants[ 13 ] = "   \n" + QuoteAll( text, "\t> " ) + "\n";

            for ( int v=0; v<14; ++v )
            {
                CORE::CDynamicBuffer fromText;
                ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( variants[ v ].C_String(), variants[ v ].Length(), sample.info.matchId, 0, fromText ) );
                ASSERT_TRUE( BuffersAreEqual( fromText, bytes ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With several blocks in a text the one of the wanted match and revision is taken" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SContainerSample a;
            REQUIRE_TRUE( a.Make( 300, false ) );
            SContainerSample b;
            REQUIRE_TRUE( b.Make( 350, false ) );
            SContainerSample a2 = a;
            a2.SetPayload( 2, &a.package, 320, false );
            SContainerSample a3 = a;
            a3.SetPayload( 3, &a2.package, 330, false );

            MULTIPLAY::CMatchTextArmor armor;
            CORE::CDynamicBuffer ba1, ba2, ba3, bb1;
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( a.preview, a.package, false, ba1 ) );
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( a2.preview, a2.package, false, ba2 ) );
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( a3.preview, a3.package, false, ba3 ) );
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( b.preview, b.package, false, bb1 ) );
            CORE::CString ta1, ta2, ta3, tb1;
            REQUIRE_TRUE( armor.Encode( a.info.matchId, 1, ba1.GetConstBufferPtr(), ba1.GetDataSize(), ta1 ) );
            REQUIRE_TRUE( armor.Encode( a.info.matchId, 2, ba2.GetConstBufferPtr(), ba2.GetDataSize(), ta2 ) );
            REQUIRE_TRUE( armor.Encode( a.info.matchId, 3, ba3.GetConstBufferPtr(), ba3.GetDataSize(), ta3 ) );
            REQUIRE_TRUE( armor.Encode( b.info.matchId, 1, bb1.GetConstBufferPtr(), bb1.GetDataSize(), tb1 ) );

            // a thread: the newest on top, older ones quoted below, and a block of another game in between
            CORE::CString thread = ta3 + "\nOn Monday Bob wrote:\n" + QuoteAll( tb1, "> " ) + "\nAnd before that:\n" + QuoteAll( ta2, "> " ) + "\n" + QuoteAll( QuoteAll( ta1, "> " ), "> " ) + "\n";
            CORE::CDynamicBuffer got;
            UInt32 revision = 0;
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( thread.C_String(), thread.Length(), b.info.matchId, 0, got, &revision ) && BuffersAreEqual( got, bb1 ) && 1 == revision );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( thread.C_String(), thread.Length(), a.info.matchId, 0, got, &revision ) && BuffersAreEqual( got, ba3 ) && 3 == revision );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( thread.C_String(), thread.Length(), a.info.matchId, 2, got, &revision ) && BuffersAreEqual( got, ba2 ) && 2 == revision );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( thread.C_String(), thread.Length(), a.info.matchId, 1, got, &revision ) && BuffersAreEqual( got, ba1 ) && 1 == revision );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_NO_BLOCK == armor.Decode( thread.C_String(), thread.Length(), a.info.matchId, 4, got ) );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_NO_BLOCK == armor.Decode( thread.C_String(), thread.Length(), MULTIPLAY::CMatchId::Generate(), 0, got ) );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_NO_BLOCK == armor.Decode( thread.C_String(), thread.Length(), "not-a-match-id", 0, got ) );
            // upper case ids as people type them
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( thread.C_String(), thread.Length(), a.info.matchId.Uppercase(), 1, got ) && BuffersAreEqual( got, ba1 ) );

            // the newest is damaged: the older good one of the same match is taken
            CORE::CString damagedTop = ta3;
            CORE::CString::StringVector lines = SplitLines( damagedTop );
            lines[ 8 ].C_String()[ 5 ] = ( lines[ 8 ][ 5 ] == 'A' ? 'B' : 'A' );
            CORE::CString damagedThread = JoinLines( lines, "\n" ) + "\n" + ta1;
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( damagedThread.C_String(), damagedThread.Length(), a.info.matchId, 0, got, &revision ) && BuffersAreEqual( got, ba1 ) && 1 == revision );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_BAD_CHECKSUM == armor.Decode( damagedThread.C_String(), damagedThread.Length(), a.info.matchId, 3, got ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A damaged or cut text is refused with the reason, never repaired" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 700, false ) );
            CORE::CDynamicBuffer bytes;
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, false, bytes ) );
            MULTIPLAY::CMatchTextArmor armor;
            CORE::CString text;
            REQUIRE_TRUE( armor.Encode( sample.info.matchId, 1, bytes.GetConstBufferPtr(), bytes.GetDataSize(), text ) );
            CORE::CDynamicBuffer got;

            // a character of the data changed
            CORE::CString::StringVector lines = SplitLines( text );
            REQUIRE_TRUE( lines.size() > 12 );
            CORE::CString::StringVector damaged = lines;
            damaged[ 8 ].C_String()[ 10 ] = ( 'A' == damaged[ 8 ][ 10 ] ) ? 'B' : 'A';
            CORE::CString damagedText = JoinLines( damaged, "\n" );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_BAD_CHECKSUM == armor.Decode( damagedText.C_String(), damagedText.Length(), CORE::CString::Empty, 0, got ) );
            ASSERT_TRUE( 0 == got.GetDataSize() );
            // a line of the data lost
            damaged = lines;
            damaged.erase( damaged.begin() + 8 );
            damagedText = JoinLines( damaged, "\n" );
            MULTIPLAY::TMatchArmorResult lost = armor.Decode( damagedText.C_String(), damagedText.Length(), CORE::CString::Empty, 0, got );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK != lost && 0 == got.GetDataSize() );
            // the hash line says something else
            damaged = lines;
            damaged[ 4 ] = CORE::CString( "SHA-256: " ) + CORE::CString( "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" );
            damagedText = JoinLines( damaged, "\n" );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_BAD_HASH == armor.Decode( damagedText.C_String(), damagedText.Length(), CORE::CString::Empty, 0, got ) );
            // the length line says something else
            damaged = lines;
            damaged[ 3 ] = "Length: 5";
            damagedText = JoinLines( damaged, "\n" );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK != armor.Decode( damagedText.C_String(), damagedText.Length(), CORE::CString::Empty, 0, got ) );
            damaged[ 3 ] = "Length: 99999999999";
            damagedText = JoinLines( damaged, "\n" );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK != armor.Decode( damagedText.C_String(), damagedText.Length(), CORE::CString::Empty, 0, got ) );
            damaged[ 3 ] = "Length: -1";
            damagedText = JoinLines( damaged, "\n" );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK != armor.Decode( damagedText.C_String(), damagedText.Length(), CORE::CString::Empty, 0, got ) );
            // a character that is not base64 in the data
            damaged = lines;
            damaged[ 8 ].C_String()[ 3 ] = '*';
            damagedText = JoinLines( damaged, "\n" );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_BAD_ENCODING == armor.Decode( damagedText.C_String(), damagedText.Length(), CORE::CString::Empty, 0, got ) );
            // no end line, and not even a begin line
            CORE::CString withoutEnd = text.SubstrFromRange( 0, text.Length() - 26 );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_TRUNCATED == armor.Decode( withoutEnd.C_String(), withoutEnd.Length(), CORE::CString::Empty, 0, got ) );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_NO_BLOCK == armor.Decode( "just a mail\nwithout a game\n", 27, CORE::CString::Empty, 0, got ) );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_NO_BLOCK == armor.Decode( GUCEF_NULL, 0, CORE::CString::Empty, 0, got ) );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_NO_BLOCK == armor.Decode( "", 0, CORE::CString::Empty, 0, got ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every truncation and every changed character of the text is refused or reads the very same bytes" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            SContainerSample sample;
            REQUIRE_TRUE( sample.Make( 500, false ) );
            CORE::CDynamicBuffer bytes;
            REQUIRE_TRUE( MULTIPLAY::MATCHCONTAINER_OK == MULTIPLAY::CMatchContainer::Encode( sample.preview, sample.package, false, bytes ) );
            MULTIPLAY::CMatchTextArmor armor;
            CORE::CString text;
            REQUIRE_TRUE( armor.Encode( sample.info.matchId, 1, bytes.GetConstBufferPtr(), bytes.GetDataSize(), text ) );
            const UInt32 endLineEnd = text.Length() - 1;                      // the final line end is not part of the block
            REQUIRE_TRUE( text.Length() < 3000 );

            // every shorter text
            for ( UInt32 n=0; n<endLineEnd; ++n )
            {
                CORE::CDynamicBuffer got;
                ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK != armor.Decode( text.C_String(), n, CORE::CString::Empty, 0, got ) );
            }
            // every changed character (two replacements) and every removed character
            UInt32 wrong = 0;
            for ( UInt32 i=0; i<text.Length(); ++i )
            {
                for ( int mode=0; mode<3; ++mode )
                {
                    CORE::CString copy = text;
                    if ( 0 == mode )
                        copy.C_String()[ i ] = 'Q';
                    else
                    if ( 1 == mode )
                        copy.C_String()[ i ] = '\0' == copy[ i ] ? 'a' : (char)( copy[ i ] ^ 0x20 );
                    else
                        copy = text.SubstrFromRange( 0, i ) + text.SubstrFromRange( i + 1, text.Length() );
                    CORE::CDynamicBuffer got;
                    if ( MULTIPLAY::MATCHARMOR_OK == armor.Decode( copy.C_String(), copy.Length(), CORE::CString::Empty, 0, got ) && !BuffersAreEqual( got, bytes ) )
                        ++wrong;
                }
            }
            ASSERT_TRUE( 0 == wrong );

            // random garbage with a begin line
            CTestRandom random( 31337 );
            for ( int round=0; round<80; ++round )
            {
                CORE::CString junk = "-----BEGIN MATCH TURN-----\n";
                UInt32 size = random.Next() % 600;
                for ( UInt32 i=0; i<size; ++i )
                {
                    char c = (char)( 9 + random.Next() % 100 );
                    junk += c;
                }
                CORE::CDynamicBuffer got;
                ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK != armor.Decode( junk.C_String(), junk.Length(), CORE::CString::Empty, 0, got ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The text is capped at 512 KB encoded and the reader refuses larger blocks and texts" )
        GUCEF_TESTFW_SKIP_IF( !MULTIPLAY::CPlayerIdentity::IsKeyPairAvailable(), "There is no ed25519 signature codec" );
        try
        {
            MULTIPLAY::CMatchTextArmor armor;
            const CORE::CString matchId = MULTIPLAY::CMatchId::Generate();

            CORE::CDynamicBuffer payload;
            payload.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
            CTestRandom random( 5 );
            for ( UInt32 i=0; i<300 * 1024; ++i ) { UInt8 b = (UInt8)( random.Next() & 0xFF ); payload.Append( &b, 1 ); }
            CORE::CString text;
            ASSERT_TRUE( armor.Encode( matchId, 1, payload.GetConstBufferPtr(), payload.GetDataSize(), text ) );
            ASSERT_TRUE( text.Length() <= MULTIPLAY::CMatchTextArmor::MaxEncodedTextSize );
            CORE::CDynamicBuffer got;
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_OK == armor.Decode( text.C_String(), text.Length(), matchId, 1, got ) && BuffersAreEqual( got, payload ) );

            // 400 KB of container is 533 KB of text: refused by the writer
            for ( UInt32 i=0; i<100 * 1024; ++i ) { UInt8 b = (UInt8)( random.Next() & 0xFF ); payload.Append( &b, 1 ); }
            ASSERT_FALSE( armor.Encode( matchId, 1, payload.GetConstBufferPtr(), payload.GetDataSize(), text ) );
            ASSERT_TRUE( text.IsNULLOrEmpty() );
            ASSERT_FALSE( armor.Encode( matchId, 1, payload.GetConstBufferPtr(), 0, text ) );
            ASSERT_FALSE( armor.Encode( "bad id", 1, payload.GetConstBufferPtr(), 10, text ) );
            CORE::CDynamicBuffer overTwoMb;
            overTwoMb.SetBufferSize( MULTIPLAY::CMatchContainer::MaxContainerSize + 1 );
            overTwoMb.SetDataSize( MULTIPLAY::CMatchContainer::MaxContainerSize + 1 );
            ASSERT_FALSE( armor.Encode( matchId, 1, overTwoMb.GetConstBufferPtr(), overTwoMb.GetDataSize(), text ) );

            // a block with 600 KB of data in it: refused by the reader
            CORE::CDynamicBuffer huge;
            huge.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
            AppendText( huge, ( "-----BEGIN MATCH TURN-----\nMatch: " + matchId + "\nRevision: 1\nLength: 440000\nSHA-256: " +
                                "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" + "\nVersion: 1\n\n" ).C_String() );
            for ( int i=0; i<8000; ++i )
                AppendText( huge, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n" );
            AppendText( huge, "Checksum: 000000\n-----END MATCH TURN-----\n" );
            ASSERT_TRUE( huge.GetDataSize() > MULTIPLAY::CMatchTextArmor::MaxEncodedTextSize );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_TOO_LARGE == armor.Decode( huge.GetConstBufferPtr(), huge.GetDataSize(), matchId, 1, got ) );

            // a text above the scan cap is not looked at
            CORE::CDynamicBuffer big;
            big.SetBufferSize( MULTIPLAY::CMatchTextArmor::MaxScannedTextSize + 1 );
            big.SetDataSize( MULTIPLAY::CMatchTextArmor::MaxScannedTextSize + 1 );
            memset( big.GetBufferPtr(), 'x', big.GetDataSize() );
            ASSERT_TRUE( MULTIPLAY::MATCHARMOR_TOO_LARGE == armor.Decode( big.GetConstBufferPtr(), big.GetDataSize(), matchId, 1, got ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}
