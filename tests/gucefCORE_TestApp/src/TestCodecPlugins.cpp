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
#include <stdlib.h>
#include <new>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CSTDCODECPLUGIN_H
#include "CStdCodecPlugin.h"
#define GUCEF_CORE_CSTDCODECPLUGIN_H
#endif /* GUCEF_CORE_CSTDCODECPLUGIN_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#include "TestCodecPlugins.h"

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

/*-------------------------------------------------------------------------*/

/**
 *  Buffer access which accepts at most a given number of bytes per write call,
 *  used to verify that codecs handle partial writes correctly
 */
class CPartialWriteBufferAccess : public CORE::CDynamicBufferAccess
{
    public:

    CPartialWriteBufferAccess( CORE::CDynamicBuffer& buffer, UInt32 maxBytesPerWrite )
        : CORE::CDynamicBufferAccess( buffer )
        , m_maxBytesPerWrite( maxBytesPerWrite )
        , m_partialWriteCount( 0 )
    {}

    virtual UInt32 Write( const void* srcdata ,
                          UInt32 esize        ,
                          UInt32 elements     ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 1 == esize && elements > m_maxBytesPerWrite )
        {
            ++m_partialWriteCount;
            elements = m_maxBytesPerWrite;
        }
        return CORE::CDynamicBufferAccess::Write( srcdata, esize, elements );
    }

    UInt32 GetPartialWriteCount( void ) const { return m_partialWriteCount; }

    private:

    UInt32 m_maxBytesPerWrite;
    UInt32 m_partialWriteCount;
};

/*-------------------------------------------------------------------------*/

// the group the plugins are loaded in by these tests, their own so unloading it touches nothing else
static const char* CodecPluginGroup = "codecplugin_tests";

static bool
EnsureCodecPluginIsLoaded( const CORE::CString& pluginName )
{GUCEF_TRACE;

    // Normally this would be managed via config however to keep the test self contained we do it programmatically
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, CodecPluginGroup, true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, CodecPluginGroup, true );
}

/*-------------------------------------------------------------------------*/

static bool
GetCodec( const CORE::CString& familyName                 ,
          const CORE::CString& codecName                  ,
          CORE::CCodecRegistry::TICodecPtr& codec         )
{GUCEF_TRACE;

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TCodecFamilyRegistryPtr codecFamilyRegistry;
    if ( codecRegistry.TryLookup( familyName, codecFamilyRegistry, false ) && codecFamilyRegistry )
        return codecFamilyRegistry->TryLookup( codecName, codec, false ) && codec;
    return false;
}

/*-------------------------------------------------------------------------*/

static void
CreateCompressibleTestData( CORE::CDynamicBuffer& data, UInt32 size )
{GUCEF_TRACE;

    // Mix of repetitive and pseudo random content so the data compresses but not trivially
    data.SetDataSize( size );
    UInt8* bytes = data.AsTypePtr< UInt8 >();
    UInt32 state = 12345;
    for ( UInt32 i=0; i<size; ++i )
    {
        state = state * 1103515245 + 12345;
        bytes[ i ] = ( 0 == ( i / 4096 ) % 2 ) ? (UInt8) ( 'a' + ( i % 26 ) ) : (UInt8) ( state >> 16 );
    }
}

/*-------------------------------------------------------------------------*/

static bool
BuffersAreEqual( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{GUCEF_TRACE;

    if ( a.GetDataSize() != b.GetDataSize() )
        return false;
    if ( 0 == a.GetDataSize() )
        return true;
    return 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

static void
PerformCompressionRoundTripTest( CORE::CCodecRegistry::TICodecPtr& codec ,
                                 const CORE::CDynamicBuffer& original    ,
                                 UInt32 maxBytesPerWrite                 )
{GUCEF_TRACE;

    CORE::CDynamicBuffer inputBuffer( original );
    CORE::CDynamicBufferAccess input( inputBuffer );

    CORE::CDynamicBuffer compressed;
    CPartialWriteBufferAccess compressedAccess( compressed, maxBytesPerWrite );
    ASSERT_TRUE( codec->Encode( input, compressedAccess ) );
    ASSERT_TRUE( compressed.GetDataSize() > 0 );
    ASSERT_TRUE( compressed.GetDataSize() < original.GetDataSize() );

    compressedAccess.Setpos( 0 );
    CORE::CDynamicBuffer decompressed;
    CPartialWriteBufferAccess decompressedAccess( decompressed, maxBytesPerWrite );
    ASSERT_TRUE( codec->Decode( compressedAccess, decompressedAccess ) );
    ASSERT_TRUE( BuffersAreEqual( original, decompressed ) );

    if ( maxBytesPerWrite < original.GetDataSize() )
    {
        // Make sure the test actually exercised partial writes
        ASSERT_TRUE( decompressedAccess.GetPartialWriteCount() > 0 );
    }
}

/*-------------------------------------------------------------------------*/

/** A codec plugin that is not linked to any module, with its function table in view */
class CStdCodecPluginProbe : public CORE::CStdCodecPlugin
{
    public:

    CStdCodecPluginProbe( void )
        : CORE::CStdCodecPlugin()
    {GUCEF_TRACE;
    }

    bool IsFunctionTableCleared( void ) const
    {GUCEF_TRACE;

        const UInt8* table = reinterpret_cast< const UInt8* >( &m_api );
        for ( UInt32 i=0; i<(UInt32) sizeof( m_api ); ++i )
            if ( 0 != table[ i ] )
                return false;
        return true;
    }
};

/*-------------------------------------------------------------------------*/

static void
PerformStdCodecPluginTests( void )
{GUCEF_TRACE;

    // Regression: the constructor cleared as many bytes as the table has entries instead of the whole table
    GUCEF_TESTFW_TESTCASE( "Test: A new codec plugin starts with a cleared function table" )
        try
        {
            void* memory = malloc( sizeof( CStdCodecPluginProbe ) );
            ASSERT_TRUE( GUCEF_NULL != memory );
            if ( GUCEF_NULL != memory )
            {
                memset( memory, 0xAB, sizeof( CStdCodecPluginProbe ) );
                CStdCodecPluginProbe* probe = new( memory ) CStdCodecPluginProbe();
                ASSERT_TRUE( probe->IsFunctionTableCleared() );
                probe->~CStdCodecPluginProbe();
                free( memory );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/*
 *  A codec plugin made of functions in this test, linked through CStdCodecPlugin::LinkApi() as a module would be.
 *  Its encode writes what parameters reached it, so the tests can see what crossed the C interface.
 */
namespace {

struct SFakePluginState
{
    UInt32 apiVersion;
    UInt32 initCalls;
    UInt32 shutdownCalls;
    UInt32 otherCalls;
    CORE::CString initParamAnswer;
};
SFakePluginState g_fakePlugin;

/*-------------------------------------------------------------------------*/

CORE::CString
FakeGetTextParam( const CORE::TVariantMapApi* params, const char* name )
{GUCEF_TRACE;

    if ( GUCEF_NULL == params || GUCEF_NULL == params->at_key )
        return CORE::CString::Empty;

    CORE::CVariant keyVar( name );
    CORE::TVariantData key = *keyVar.CStyleAccess();
    CORE::TVariantData value;
    memset( &value, 0, sizeof( value ) );
    params->at_key( params->privateData, &key, &value );
    if ( GUCEF_DATATYPE_UNKNOWN == value.containedType )
        return CORE::CString::Empty;
    CORE::CVariant valueVar;
    valueVar.LinkTo( &value );
    return valueVar.AsString();
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX FakeGetApiVersion( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return g_fakePlugin.apiVersion;
}

CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX FakeInit( void** plugdata, TGucefCoreCApi* coreApi, const CORE::TVariantMapApi* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    ++g_fakePlugin.initCalls;
    g_fakePlugin.initParamAnswer = FakeGetTextParam( pluginParams, "answer" );
    *plugdata = &g_fakePlugin;
    return GUCEF_NULL != coreApi && GUCEF_NULL != coreApi->Log ? CORE::GUCEF_CODEC_STATUS_OK : CORE::GUCEF_CODEC_STATUS_FAILED;
}

CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX FakeShutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    ++g_fakePlugin.shutdownCalls;
    return CORE::GUCEF_CODEC_STATUS_OK;
}

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX FakeDescription( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    ++g_fakePlugin.otherCalls;
    return "fake codec plugin";
}

CORE::TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX FakeVersion( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    ++g_fakePlugin.otherCalls;
    CORE::TVersion version = { 1, 2, 3, 4 };
    return version;
}

CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
FakeEncode( void* pluginData, void* codecData, const CORE::TVariantMapApi* params, CORE::TIOAccess* input, CORE::TIOAccess* output, CORE::TCodecErrorInfo* error ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( !FakeGetTextParam( params, "fail" ).IsNULLOrEmpty() )
    {
        strncpy( error->message, "tag mismatch", GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 );
        return CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED;
    }
    CORE::CString text = "level=" + FakeGetTextParam( params, "level" );
    output->write( output, text.C_String(), 1, text.ByteSize() - 1 );
    return CORE::GUCEF_CODEC_STATUS_OK;
}

CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
FakeGetParameterInfo( void* pluginData, void* codecData, UInt32 index, CORE::TCodecParameterInfo* info ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( 0 != index )
        return CORE::GUCEF_CODEC_STATUS_UNSUPPORTED;
    info->name = "level";
    info->typeId = GUCEF_DATATYPE_INT32;
    info->required = 1;
    info->defaultValue = "5";
    info->description = "how much";
    return CORE::GUCEF_CODEC_STATUS_OK;
}

const CORE::TCodecPluginLink g_fakeLink =
{
    sizeof( CORE::TCodecPluginLink ), "FakeCodecFamily", "fake", GUCEF_NULL, GUCEF_CODEC_CAPABILITY_ENCODE, 0, 0,
    FakeEncode, GUCEF_NULL, FakeGetParameterInfo, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL
};

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX FakeGetCodecCount( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return 1;
}

CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX FakeGetCodecLink( void* plugdata, UInt32 index, const CORE::TCodecPluginLink** codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( 0 != index )
        return CORE::GUCEF_CODEC_STATUS_UNSUPPORTED;
    *codecLink = &g_fakeLink;
    return CORE::GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** Links the fake plugin as a module would be linked */
class CFakeCodecPlugin : public CORE::CStdCodecPlugin
{
    public:

    bool LinkFake( CORE::TPluginMetaDataPtr metaData )
    {GUCEF_TRACE;

        CORE::TGucefCoreCodecPluginApi api;
        memset( &api, 0, sizeof( api ) );
        api.GetApiVersion = FakeGetApiVersion;
        api.Init = FakeInit;
        api.Shutdown = FakeShutdown;
        api.Description = FakeDescription;
        api.Copyright = FakeDescription;
        api.Version = FakeVersion;
        api.GetCodecCount = FakeGetCodecCount;
        api.GetCodecLink = FakeGetCodecLink;
        return LinkApi( &g_fakePlugin, api, metaData );
    }
};

/*-------------------------------------------------------------------------*/

void
ResetFakePlugin( UInt32 apiVersion )
{GUCEF_TRACE;

    g_fakePlugin.apiVersion = apiVersion;
    g_fakePlugin.initCalls = 0;
    g_fakePlugin.shutdownCalls = 0;
    g_fakePlugin.otherCalls = 0;
    g_fakePlugin.initParamAnswer.Clear();
}

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

static void
PerformCodecPluginInterfaceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A codec plugin of another interface major version is refused before anything else is called" )
        try
        {
            ResetFakePlugin( ( ( GUCEF_CODECPLUGIN_API_VERSION_MAJOR - 1 ) << 16 ) | 7 );
            CFakeCodecPlugin plugin;
            ASSERT_FALSE( plugin.LinkFake( CORE::TPluginMetaDataPtr() ) );
            ASSERT_TRUE( 0 == g_fakePlugin.initCalls && 0 == g_fakePlugin.otherCalls );
            ASSERT_TRUE( 0 == plugin.GetApiVersion() );

            ResetFakePlugin( ( ( GUCEF_CODECPLUGIN_API_VERSION_MAJOR + 1 ) << 16 ) );
            CFakeCodecPlugin newerPlugin;
            ASSERT_FALSE( newerPlugin.LinkFake( CORE::TPluginMetaDataPtr() ) );
            ASSERT_TRUE( 0 == g_fakePlugin.initCalls && 0 == g_fakePlugin.otherCalls );

            CORE::CCodecRegistry::TICodecPtr codec;
            ASSERT_FALSE( GetCodec( "FakeCodecFamily", "fake", codec ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A codec plugin gets its metadata parameters at Init, codec parameters per call and reports status and error text" )
        try
        {
            ResetFakePlugin( GUCEF_CODECPLUGIN_API_VERSION + 3 );    // a newer minor version of the same major is fine
            CORE::TPluginMetaDataStoragePtr metaData( GUCEF_NEW CORE::CPluginMetaData() );
            CORE::CValueList pluginParams;
            pluginParams.Set( "answer", CORE::CVariant( "42" ) );
            pluginParams.Set( CORE::CStdCodecPlugin::CodecPriorityParamName, CORE::CVariant( "77" ) );
            metaData->SetParams( pluginParams );

            {
                CFakeCodecPlugin plugin;
                ASSERT_TRUE( plugin.LinkFake( metaData ) );
                ASSERT_TRUE( 1 == g_fakePlugin.initCalls );
                ASSERT_TRUE( "42" == g_fakePlugin.initParamAnswer );
                ASSERT_TRUE( GUCEF_CODECPLUGIN_API_VERSION + 3 == plugin.GetApiVersion() );

                CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
                CORE::CCodecRegistry::TICodecPtr codec = registry.TryGetCodec( "FakeCodecFamily", "fake" );
                ASSERT_TRUE( !codec.IsNULL() );
                if ( !codec.IsNULL() )
                {
                    Int32 priority = 0;
                    ASSERT_TRUE( registry.TryGetCodecPriority( "FakeCodecFamily", "fake", codec.GetPointerAlways(), priority ) && 77 == priority );

                    // codec parameters reach the plugin
                    CORE::CDynamicBuffer inputBuffer;
                    CORE::CDynamicBufferAccess input( inputBuffer );
                    CORE::CDynamicBuffer outputBuffer;
                    CORE::CDynamicBufferAccess output( outputBuffer );
                    CORE::CValueList params;
                    params.Set( "level", CORE::CVariant( (Int32) 6 ) );
                    ASSERT_TRUE( codec->Encode( input, output, params ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_OK == codec->GetLastStatus() );
                    ASSERT_TRUE( "level=6" == CORE::CString( outputBuffer.AsConstTypePtr< char >(), outputBuffer.GetDataSize() ) );

                    // the plugin's status and error text arrive on the C++ side
                    params.Set( "fail", CORE::CVariant( "yes" ) );
                    ASSERT_FALSE( codec->Encode( input, output, params ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED == codec->GetLastStatus() );
                    ASSERT_TRUE( "tag mismatch" == codec->GetLastErrorMessage() );

                    // a capability the plugin lacks
                    ASSERT_FALSE( codec->Decode( input, output, params ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED == codec->GetLastStatus() );

                    // the parameter description
                    CORE::CCodecParameterInfoList info;
                    codec->GetParameterInfo( info );
                    ASSERT_TRUE( 1 == info.size() && "level" == info[ 0 ].GetName() && info[ 0 ].GetIsRequired() &&
                                 "5" == info[ 0 ].GetDefaultValue().AsString() && GUCEF_DATATYPE_INT32 == info[ 0 ].GetTypeId() );

                    // without instances of its own the plugin codec gives the default instance, which keeps the plugin linked
                    CORE::CICodecInstancePtr instance;
                    ASSERT_TRUE( codec->CreateInstance( params, instance ) && !instance.IsNULL() );
                    codec.Unlink();
                    ASSERT_FALSE( plugin.Unlink() );
                    instance.Unlink();
                }

                ASSERT_TRUE( plugin.Unlink() );
                ASSERT_TRUE( 1 == g_fakePlugin.shutdownCalls );
                CORE::CCodecRegistry::TICodecPtr gone;
                ASSERT_FALSE( GetCodec( "FakeCodecFamily", "fake", gone ) );
            }
            ASSERT_TRUE( 1 == g_fakePlugin.shutdownCalls );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Runs the whole input through the codec with parameters, one shot */
static bool
RunCodec( CORE::CCodecRegistry::TICodecPtr& codec, bool encode, const CORE::CDynamicBuffer& input, const CORE::CValueList& params, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    CORE::CDynamicBuffer inputCopy( input );
    CORE::CDynamicBufferAccess inputAccess( inputCopy );
    output.Clear( true );
    CORE::CDynamicBufferAccess outputAccess( output );
    return encode ? codec->Encode( inputAccess, outputAccess, params ) : codec->Decode( inputAccess, outputAccess, params );
}

/*-------------------------------------------------------------------------*/

/** Runs the input through an instance in chunks of the given size, the output of each step appended */
static bool
RunInstanceInChunks( CORE::CICodecInstancePtr& instance, bool encode, const CORE::CDynamicBuffer& input, UInt32 chunkSize, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    const UInt32 direction = encode ? GUCEF_CODEC_PROCESS_ENCODE : GUCEF_CODEC_PROCESS_DECODE;
    UInt32 offset = 0;
    UInt32 flags = GUCEF_CODEC_PROCESS_BEGIN | direction;
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
        flags = GUCEF_CODEC_PROCESS_UPDATE | direction;
    }
    while ( offset < input.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

static void
PerformZLibParameterTests( void )
{GUCEF_TRACE;

    CORE::CDynamicBuffer original;
    CreateCompressibleTestData( original, 256 * 1024 + 7 );

    GUCEF_TESTFW_TESTCASE( "Test: ZLib compression levels reach the plugin" )
        try
        {
            static const char* compressionCodecs[] = { "deflate", "zlib", "gzip" };
            for ( UInt32 i=0; i<3; ++i )
            {
                CORE::CCodecRegistry::TICodecPtr codec;
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CompressionCodec, compressionCodecs[ i ], codec ) );
                if ( codec.IsNULL() )
                    continue;

                CORE::CValueList stored;
                stored.Set( "level", CORE::CVariant( (Int32) 0 ) );
                CORE::CValueList fastest;
                fastest.Set( "level", CORE::CVariant( "1" ) );
                CORE::CValueList smallest;
                smallest.Set( "level", CORE::CVariant( (UInt32) 9 ) );

                CORE::CDynamicBuffer storedOutput, fastestOutput, smallestOutput, defaultOutput, decoded;
                ASSERT_TRUE( RunCodec( codec, true, original, stored, storedOutput ) );
                ASSERT_TRUE( RunCodec( codec, true, original, fastest, fastestOutput ) );
                ASSERT_TRUE( RunCodec( codec, true, original, smallest, smallestOutput ) );
                ASSERT_TRUE( RunCodec( codec, true, original, CORE::CValueList(), defaultOutput ) );

                // level 0 stores, so the output is larger than the input; the default is the smallest output
                ASSERT_TRUE( storedOutput.GetDataSize() > original.GetDataSize() );
                ASSERT_TRUE( smallestOutput.GetDataSize() < fastestOutput.GetDataSize() );
                ASSERT_TRUE( BuffersAreEqual( smallestOutput, defaultOutput ) );

                ASSERT_TRUE( RunCodec( codec, false, storedOutput, CORE::CValueList(), decoded ) && BuffersAreEqual( original, decoded ) );
                ASSERT_TRUE( RunCodec( codec, false, fastestOutput, CORE::CValueList(), decoded ) && BuffersAreEqual( original, decoded ) );

                // a level zlib does not have
                CORE::CValueList invalid;
                invalid.Set( "level", CORE::CVariant( (Int32) 12 ) );
                ASSERT_FALSE( RunCodec( codec, true, original, invalid, decoded ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == codec->GetLastStatus() );
                ASSERT_FALSE( codec->GetLastErrorMessage().IsNULLOrEmpty() );

                CORE::CCodecParameterInfoList info;
                codec->GetParameterInfo( info );
                ASSERT_TRUE( 1 == info.size() && "level" == info[ 0 ].GetName() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ZLib instances compress and decompress in chunks" )
        try
        {
            CORE::CCodecRegistry::TICodecPtr codec;
            ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip", codec ) );
            if ( !codec.IsNULL() )
            {
                ASSERT_TRUE( 0 != ( codec->GetCapabilities() & GUCEF_CODEC_CAPABILITY_INSTANCES ) );
                CORE::CValueList params;
                params.Set( "level", CORE::CVariant( (Int32) 6 ) );
                CORE::CICodecInstancePtr compressor;
                CORE::CICodecInstancePtr decompressor;
                ASSERT_TRUE( codec->CreateInstance( params, compressor ) && !compressor.IsNULL() );
                ASSERT_TRUE( codec->CreateInstance( params, decompressor ) && !decompressor.IsNULL() );
                if ( !compressor.IsNULL() && !decompressor.IsNULL() )
                {
                    static const UInt32 chunkSizes[] = { 1000, 64 * 1024, 300 * 1024 };
                    for ( UInt32 i=0; i<3; ++i )
                    {
                        // an instance is reused for each message
                        CORE::CDynamicBuffer compressed, decompressed;
                        ASSERT_TRUE( RunInstanceInChunks( compressor, true, original, chunkSizes[ i ], compressed ) );
                        ASSERT_TRUE( compressed.GetDataSize() > 0 && compressed.GetDataSize() < original.GetDataSize() );

                        // the one shot decoder takes what the instance made
                        CORE::CDynamicBuffer oneShotDecoded;
                        ASSERT_TRUE( RunCodec( codec, false, compressed, CORE::CValueList(), oneShotDecoded ) && BuffersAreEqual( original, oneShotDecoded ) );

                        // small input steps give much more output than input: the host collects it in several passes
                        ASSERT_TRUE( RunInstanceInChunks( decompressor, false, compressed, 100, decompressed ) );
                        ASSERT_TRUE( BuffersAreEqual( original, decompressed ) );
                    }

                    // truncated data is reported at the end of the message
                    CORE::CDynamicBuffer compressed, decompressed;
                    ASSERT_TRUE( RunInstanceInChunks( compressor, true, original, 4096, compressed ) );
                    compressed.SetDataSize( compressed.GetDataSize() / 2 );
                    ASSERT_FALSE( RunInstanceInChunks( decompressor, false, compressed, 4096, decompressed ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_INPUT == decompressor->GetLastStatus() );
                }
            }

            // the checksums incrementally
            CORE::CCodecRegistry::TICodecPtr crc;
            ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::ChecksumCodec, "crc-32", crc ) );
            if ( !crc.IsNULL() )
            {
                CORE::CICodecInstancePtr instance;
                ASSERT_TRUE( crc->CreateInstance( CORE::CValueList(), instance ) );
                CORE::CDynamicBuffer input;
                input.CopyFrom( 9, "123456789" );
                CORE::CDynamicBuffer checksum;
                ASSERT_TRUE( RunInstanceInChunks( instance, true, input, 2, checksum ) );
                ASSERT_TRUE( 4 == checksum.GetDataSize() && 0xCBF43926 == checksum.AsConstType< UInt32 >() );
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
PerformHashParameterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Hash codecs take the output parameter and hash incrementally" )
        try
        {
            static const char* hashes[] = { "md5", "sha1", "sha3", "keccak" };
            CORE::CDynamicBuffer input;
            CreateCompressibleTestData( input, 10000 );

            for ( UInt32 i=0; i<4; ++i )
            {
                CORE::CCodecRegistry::TICodecPtr hexCodec;
                CORE::CCodecRegistry::TICodecPtr rawCodec;
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CryptographicHashCodec, hashes[ i ], hexCodec ) );
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CryptographicHashCodec, CORE::CString( hashes[ i ] ) + "-raw", rawCodec ) );
                if ( hexCodec.IsNULL() || rawCodec.IsNULL() )
                    continue;

                CORE::CValueList asRaw;
                asRaw.Set( "output", CORE::CVariant( "raw" ) );
                CORE::CValueList asHex;
                asHex.Set( "output", CORE::CVariant( "hex" ) );

                CORE::CDynamicBuffer hexDigest, rawDigest, rawByParam, hexByParam, incremental;
                ASSERT_TRUE( RunCodec( hexCodec, true, input, CORE::CValueList(), hexDigest ) );
                ASSERT_TRUE( RunCodec( rawCodec, true, input, CORE::CValueList(), rawDigest ) );
                ASSERT_TRUE( RunCodec( hexCodec, true, input, asRaw, rawByParam ) );
                ASSERT_TRUE( RunCodec( rawCodec, true, input, asHex, hexByParam ) );
                ASSERT_TRUE( BuffersAreEqual( rawDigest, rawByParam ) );
                ASSERT_TRUE( BuffersAreEqual( hexDigest, hexByParam ) );
                ASSERT_TRUE( 2 * rawDigest.GetDataSize() == hexDigest.GetDataSize() );

                // incremental through an instance is the same as one shot
                CORE::CICodecInstancePtr instance;
                ASSERT_TRUE( hexCodec->CreateInstance( asRaw, instance ) && !instance.IsNULL() );
                if ( !instance.IsNULL() )
                {
                    ASSERT_TRUE( RunInstanceInChunks( instance, true, input, 333, incremental ) );
                    ASSERT_TRUE( BuffersAreEqual( rawDigest, incremental ) );
                    ASSERT_TRUE( RunInstanceInChunks( instance, true, input, 4096, incremental ) );
                    ASSERT_TRUE( BuffersAreEqual( rawDigest, incremental ) );
                    ASSERT_FALSE( instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, "x", 1, GUCEF_NULL, incremental ) );
                    ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED == instance->GetLastStatus() );
                }

                CORE::CValueList invalid;
                invalid.Set( "output", CORE::CVariant( "base64" ) );
                ASSERT_FALSE( RunCodec( hexCodec, true, input, invalid, incremental ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER == hexCodec->GetLastStatus() );
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
PerformZLibCodecTests( void )
{GUCEF_TRACE;

    if ( !EnsureCodecPluginIsLoaded( "codecspluginZLIB" ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ZLib codec plugin TESTS skipped: codecspluginZLIB is not available" );
        return;
    }

    static const char* compressionCodecs[] = { "deflate", "zlib", "gzip" };

    // Large enough to need multiple internal buffer cycles of the codec
    CORE::CDynamicBuffer original;
    CreateCompressibleTestData( original, 1024 * 1024 + 123 );

    GUCEF_TESTFW_TESTCASE( "Test: ZLib compression codecs round trip" )
        try
        {
            for ( UInt32 i=0; i<3; ++i )
            {
                CORE::CCodecRegistry::TICodecPtr codec;
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CompressionCodec, compressionCodecs[ i ], codec ) );
                if ( !codec.IsNULL() )
                    PerformCompressionRoundTripTest( codec, original, GUCEF_UINT32MAX );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: after a partial write the codec continued with the wrong output buffer position
    // which corrupted the output, and the end of stream flush wrote more bytes than were produced
    GUCEF_TESTFW_TESTCASE( "Test: ZLib compression codecs handle partial writes" )
        try
        {
            for ( UInt32 i=0; i<3; ++i )
            {
                CORE::CCodecRegistry::TICodecPtr codec;
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CompressionCodec, compressionCodecs[ i ], codec ) );
                if ( !codec.IsNULL() )
                    PerformCompressionRoundTripTest( codec, original, 1000 );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ZLib codec handles empty input" )
        try
        {
            CORE::CCodecRegistry::TICodecPtr codec;
            ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip", codec ) );
            if ( !codec.IsNULL() )
            {
                CORE::CDynamicBuffer emptyBuffer;
                CORE::CDynamicBufferAccess input( emptyBuffer );
                CORE::CDynamicBuffer compressed;
                CORE::CDynamicBufferAccess compressedAccess( compressed );
                ASSERT_TRUE( codec->Encode( input, compressedAccess ) );
                ASSERT_TRUE( compressed.GetDataSize() > 0 );

                compressedAccess.Setpos( 0 );
                CORE::CDynamicBuffer decompressed;
                CORE::CDynamicBufferAccess decompressedAccess( decompressed );
                ASSERT_TRUE( codec->Decode( compressedAccess, decompressedAccess ) );
                ASSERT_TRUE( 0 == decompressed.GetDataSize() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ZLib codec rejects truncated and corrupt input" )
        try
        {
            CORE::CCodecRegistry::TICodecPtr codec;
            ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip", codec ) );
            if ( !codec.IsNULL() )
            {
                CORE::CDynamicBuffer inputBuffer( original );
                CORE::CDynamicBufferAccess input( inputBuffer );
                CORE::CDynamicBuffer compressed;
                CORE::CDynamicBufferAccess compressedAccess( compressed );
                ASSERT_TRUE( codec->Encode( input, compressedAccess ) );

                // truncated: the stream ends before the end of the compressed data
                CORE::CDynamicBuffer truncated;
                truncated.CopyFrom( compressed.GetDataSize() / 2, compressed.GetConstBufferPtr() );
                CORE::CDynamicBufferAccess truncatedAccess( truncated );
                CORE::CDynamicBuffer output;
                CORE::CDynamicBufferAccess outputAccess( output );
                ASSERT_FALSE( codec->Decode( truncatedAccess, outputAccess ) );

                // corrupt: not compressed data at all
                CORE::CDynamicBuffer corrupt;
                corrupt.CopyFrom( 64, "this is definitely not a valid gzip or zlib stream, not at all..." );
                CORE::CDynamicBufferAccess corruptAccess( corrupt );
                CORE::CDynamicBuffer output2;
                CORE::CDynamicBufferAccess outputAccess2( output2 );
                ASSERT_FALSE( codec->Decode( corruptAccess, outputAccess2 ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: the checksum codecs reported failure because the result of writing
    // the 4 byte checksum was compared against the wrong element count
    GUCEF_TESTFW_TESTCASE( "Test: ZLib checksum codecs known answers" )
        try
        {
            static const char* checkInput = "123456789";
            static const char* checksumCodecs[] = { "crc-32", "adler-32" };
            static const UInt32 expectedChecksums[] = { 0xCBF43926, 0x091E01DE };

            for ( UInt32 i=0; i<2; ++i )
            {
                CORE::CCodecRegistry::TICodecPtr codec;
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::ChecksumCodec, checksumCodecs[ i ], codec ) );
                if ( !codec.IsNULL() )
                {
                    CORE::CDynamicBuffer inputBuffer;
                    inputBuffer.CopyFrom( (UInt32) strlen( checkInput ), checkInput );
                    CORE::CDynamicBufferAccess input( inputBuffer );
                    CORE::CDynamicBuffer checksum;
                    CORE::CDynamicBufferAccess checksumAccess( checksum );
                    ASSERT_TRUE( codec->Encode( input, checksumAccess ) );
                    ASSERT_TRUE( 4 == checksum.GetDataSize() );
                    if ( 4 == checksum.GetDataSize() )
                        ASSERT_TRUE( expectedChecksums[ i ] == checksum.AsConstType< UInt32 >() );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    PerformZLibParameterTests();
}

/*-------------------------------------------------------------------------*/

static void
PerformHashCodecTests( void )
{GUCEF_TRACE;

    if ( !EnsureCodecPluginIsLoaded( "codecspluginSTBRUMMEHASH" ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Hash codec plugin TESTS skipped: codecspluginSTBRUMMEHASH is not available" );
        return;
    }

    struct SHashVector { const char* codecName; const char* expectedHex; };
    // Known answers for the input "abc"
    static const SHashVector vectors[] = {
        { "md5"    , "900150983cd24fb0d6963f7d28e17f72" },
        { "sha1"   , "a9993e364706816aba3e25717850c26c9cd0d89d" },
        { "sha256" , "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" },
        { "sha3"   , "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532" },
        { "keccak" , "4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45" }
    };
    static const UInt32 vectorCount = (UInt32) ( sizeof( vectors ) / sizeof( vectors[ 0 ] ) );

    GUCEF_TESTFW_TESTCASE( "Test: Hash codecs known answers as hex and raw digests" )
        try
        {
            for ( UInt32 i=0; i<vectorCount; ++i )
            {
                CORE::CString hexName = vectors[ i ].codecName;
                CORE::CString rawName = hexName + "-raw";
                CORE::CString expectedHex = vectors[ i ].expectedHex;

                CORE::CCodecRegistry::TICodecPtr hexCodec;
                CORE::CCodecRegistry::TICodecPtr rawCodec;
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CryptographicHashCodec, hexName, hexCodec ) );
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CryptographicHashCodec, rawName, rawCodec ) );
                if ( hexCodec.IsNULL() || rawCodec.IsNULL() )
                    continue;

                CORE::CDynamicBuffer inputBuffer;
                inputBuffer.CopyFrom( 3, "abc" );
                CORE::CDynamicBufferAccess input( inputBuffer );
                CORE::CDynamicBuffer hexResult;
                CORE::CDynamicBufferAccess hexResultAccess( hexResult );
                ASSERT_TRUE( hexCodec->Encode( input, hexResultAccess ) );
                ASSERT_TRUE( expectedHex.Length() == hexResult.GetDataSize() );
                ASSERT_TRUE( CORE::CString( hexResult.AsConstTypePtr< char >(), hexResult.GetDataSize() ) == expectedHex );

                input.Setpos( 0 );
                CORE::CDynamicBuffer rawResult;
                CORE::CDynamicBufferAccess rawResultAccess( rawResult );
                ASSERT_TRUE( rawCodec->Encode( input, rawResultAccess ) );
                ASSERT_TRUE( expectedHex.Length() / 2 == rawResult.GetDataSize() );

                // compare the raw bytes against the hex representation
                static const char* hexDigits = "0123456789abcdef";
                CORE::CString rawAsHex;
                const UInt8* rawBytes = rawResult.AsConstTypePtr< UInt8 >();
                for ( UInt32 n=0; n<rawResult.GetDataSize(); ++n )
                {
                    rawAsHex += CORE::CString( hexDigits + ( rawBytes[ n ] >> 4 ), 1 );
                    rawAsHex += CORE::CString( hexDigits + ( rawBytes[ n ] & 0xF ), 1 );
                }
                ASSERT_TRUE( rawAsHex == expectedHex );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    PerformHashParameterTests();
}

/*-------------------------------------------------------------------------*/

void
PerformCodecPluginTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Codec Plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CodecPlugins" );

    PerformStdCodecPluginTests();
    PerformCodecPluginInterfaceTests();
    PerformZLibCodecTests();
    PerformHashCodecTests();

    // C plugins found by trial and error (no plugin type given) must still be unloaded through their plugin manager,
    // which takes their codecs out of the registry, and they must load again afterwards
    CORE::CCodecRegistry::TICodecPtr deflate;
    if ( GetCodec( CORE::CoreCodecTypes::CompressionCodec, "deflate", deflate ) )
    {
        deflate.Unlink();
        GUCEF_TESTFW_TESTCASE( "Test: Codec plugins unload cleanly and load again" )
            try
            {
                CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
                ASSERT_TRUE( pluginControl.UnloadPluginGroup( CodecPluginGroup ) );
                CORE::CCodecRegistry::TICodecPtr gone;
                ASSERT_FALSE( GetCodec( CORE::CoreCodecTypes::CompressionCodec, "deflate", gone ) );

                ASSERT_TRUE( pluginControl.LoadPluginGroup( CodecPluginGroup ) );
                CORE::CCodecRegistry::TICodecPtr reloaded;
                ASSERT_TRUE( GetCodec( CORE::CoreCodecTypes::CompressionCodec, "deflate", reloaded ) );
                if ( !reloaded.IsNULL() )
                {
                    CORE::CDynamicBuffer original;
                    CreateCompressibleTestData( original, 64 * 1024 );
                    PerformCompressionRoundTripTest( reloaded, original, 1024 * 1024 );
                }
            }
            catch( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Codec Plugin TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
