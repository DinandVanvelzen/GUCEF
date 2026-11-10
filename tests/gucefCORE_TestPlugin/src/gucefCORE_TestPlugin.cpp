/*
 *  gucefCORE_TestPlugin: generic plugin used by the gucefCORE tests of the plugin system
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#include "gucefCORE_TestPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A configurable of the process, as the globals that plugins bring along are, whose config always fails */
class CFailingConfigurable : public GUCEF::CORE::CGloballyConfigurable
{
    public:

    CFailingConfigurable( void ) : GUCEF::CORE::CGloballyConfigurable( false ) {}

    virtual bool SaveConfig( GUCEF::CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "GUCEF::CORE::TestPlugin::CFailingConfigurable";
        return classTypeName;
    }
};

static CFailingConfigurable* g_failingConfigurable = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

static bool
IsFlagSet( const char* variable )
{
    const char* value = getenv( variable );
    return GUCEF_NULL != value && 0 == strcmp( value, "1" );
}

/*-------------------------------------------------------------------------*/

/** Counts calls in an environment variable, which the tests read; the module itself may be gone by then */
static void
CountCall( const char* variable )
{
    const char* current = getenv( variable );
    char text[ 32 ];
    snprintf( text, sizeof( text ), "%d", ( GUCEF_NULL != current ? atoi( current ) : 0 ) + 1 );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( variable, text );
    #else
    setenv( variable, text, 1 );
    #endif
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( GUCEF::CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CountCall( "GUCEF_TESTPLUGIN_LOADS" );
    if ( IsFlagSet( "GUCEF_TESTPLUGIN_LOAD_THROWS" ) )
        throw std::runtime_error( "gucefCORE_TestPlugin: Load fails on purpose" );
    if ( IsFlagSet( "GUCEF_TESTPLUGIN_LOAD_FAILS" ) )
        return 0;
    if ( IsFlagSet( "GUCEF_TESTPLUGIN_LOAD_ADDS_FAILING_CONFIGURABLE" ) && GUCEF_NULL == g_failingConfigurable )
        g_failingConfigurable = new CFailingConfigurable();
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CountCall( "GUCEF_TESTPLUGIN_UNLOADS" );
    delete g_failingConfigurable;
    g_failingConfigurable = GUCEF_NULL;
    const char* throws = getenv( "GUCEF_TESTPLUGIN_UNLOAD_THROWS" );
    if ( GUCEF_NULL != throws && 0 == strcmp( throws, "1" ) )
        throw std::runtime_error( "gucefCORE_TestPlugin: Unload fails on purpose" );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( GUCEF::CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    versionInfo->major = 1;
    versionInfo->minor = 0;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Plugin used by the gucefCORE tests of the plugin system";
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TABLE CODEC PLUGIN                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

/** File metadata made by the code of this module; its destructor is in the module */
class CTestPluginMetaData : public GUCEF::CORE::CITableFileMetaData
{
    public:

    CTestPluginMetaData( void ) : GUCEF::CORE::CITableFileMetaData(), m_schema() { CountCall( "GUCEF_TESTPLUGIN_TC_METADATACREATED" ); }
    virtual ~CTestPluginMetaData() GUCEF_VIRTUAL_OVERRIDE { CountCall( "GUCEF_TESTPLUGIN_TC_METADATADESTROYED" ); }

    virtual const GUCEF::CORE::CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_schema; }
    virtual GUCEF::UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual GUCEF::UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual bool GetRowGroupInfo( GUCEF::UInt32, GUCEF::CORE::CDataTableRowGroupInfo& ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual GUCEF::UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE { return sizeof( *this ); }
    virtual void GetKeyValueMetaData( GUCEF::CORE::CValueList& keyValues ) const GUCEF_VIRTUAL_OVERRIDE { keyValues.Set( "testplugin", "1" ); }

    private:

    GUCEF::CORE::CDataTableSchemaPtr m_schema;
};

/** A reader made by the code of this module; it reads nothing */
class CTestPluginReader : public GUCEF::CORE::CITableReader
{
    public:

    CTestPluginReader( const GUCEF::CORE::CITableFileMetaDataPtr& metaData ) : GUCEF::CORE::CITableReader(), m_metaData( metaData ) { CountCall( "GUCEF_TESTPLUGIN_TC_READERSCREATED" ); }
    virtual ~CTestPluginReader() GUCEF_VIRTUAL_OVERRIDE { CountCall( "GUCEF_TESTPLUGIN_TC_READERSDESTROYED" ); }

    virtual const GUCEF::CORE::CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData; }
    virtual GUCEF::CORE::TDataTableStatus OpenRowGroup( GUCEF::UInt32, const GUCEF::CORE::CDataTableReadRequest& ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF::CORE::DATATABLE_STATUS_END; }
    virtual GUCEF::CORE::TDataTableStatus ReadNextBatch( GUCEF::CORE::CDataTableBatchPtr& ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF::CORE::DATATABLE_STATUS_END; }
    virtual GUCEF::CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return "test reader"; }

    private:

    GUCEF::CORE::CITableFileMetaDataPtr m_metaData;
};

/** A writer made by the code of this module; it writes nothing */
class CTestPluginWriter : public GUCEF::CORE::CITableWriter
{
    public:

    CTestPluginWriter( void ) : GUCEF::CORE::CITableWriter() { CountCall( "GUCEF_TESTPLUGIN_TC_WRITERSCREATED" ); }
    virtual ~CTestPluginWriter() GUCEF_VIRTUAL_OVERRIDE { CountCall( "GUCEF_TESTPLUGIN_TC_WRITERSDESTROYED" ); }

    virtual GUCEF::CORE::TDataTableStatus WriteBatch( const GUCEF::CORE::CDataTableBatch& ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF::CORE::DATATABLE_STATUS_OK; }
    virtual GUCEF::CORE::TDataTableStatus FlushRowGroup( void ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF::CORE::DATATABLE_STATUS_OK; }
    virtual GUCEF::CORE::TDataTableStatus Finish( GUCEF::CORE::CDataTableFileSummary& ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF::CORE::DATATABLE_STATUS_OK; }
    virtual void Abort( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual GUCEF::CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return "test writer"; }
};

/** A table codec that registers, is referenced and released; with GUCEF_TESTPLUGIN_TC_MAKES_OBJECTS set it makes metadata, readers and writers that do nothing */
class CTestPluginTableCodec : public GUCEF::CORE::CITableCodec
{
    public:

    CTestPluginTableCodec( const GUCEF::CORE::CString& typeName ) : GUCEF::CORE::CITableCodec(), m_typeName( typeName ) {}

    virtual ~CTestPluginTableCodec() GUCEF_VIRTUAL_OVERRIDE { CountCall( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" ); }

    virtual GUCEF::CORE::CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_typeName; }
    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Table codec of the test plugin"; }
    virtual GUCEF::CORE::CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Copyright (C) Dinand Vanvelzen, Apache License v2"; }

    virtual GUCEF::CORE::TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::TVersion version;
        version.major = 1; version.minor = 0; version.patch = 0; version.release = 0;
        return version;
    }

    virtual GUCEF::UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_TABLECODEC_CAPABILITY_READ; }

    virtual void GetFileExtensions( GUCEF::CORE::CString::StringSet& extensions ) const GUCEF_VIRTUAL_OVERRIDE { extensions.insert( "tstbl" ); }

    virtual GUCEF::CORE::TDataTableStatus ReadMetaData( GUCEF::CORE::CIOAccess&, const GUCEF::CORE::CValueList&,
                                                        GUCEF::CORE::CITableFileMetaDataPtr& metaData, GUCEF::CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( IsFlagSet( "GUCEF_TESTPLUGIN_TC_MAKES_OBJECTS" ) )
        {
            GUCEF::CORE::CTSharedPtr< CTestPluginMetaData, GUCEF::MT::CMutex > made( new CTestPluginMetaData() );
            metaData = GUCEF::CORE::CITableFileMetaDataPtr( made );
            return GUCEF::CORE::DATATABLE_STATUS_OK;
        }
        error = "The test codec reads nothing";
        return GUCEF::CORE::DATATABLE_STATUS_UNSUPPORTED;
    }

    virtual GUCEF::CORE::TDataTableStatus CreateReader( const GUCEF::CORE::IOAccessPtr&, const GUCEF::CORE::CITableFileMetaDataPtr& metaData,
                                                        const GUCEF::CORE::CValueList&, GUCEF::CORE::CITableReaderPtr& reader, GUCEF::CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( IsFlagSet( "GUCEF_TESTPLUGIN_TC_MAKES_OBJECTS" ) )
        {
            // like the readers of real codecs, this one needs the metadata object of its own codec and not another
            if ( !metaData.IsNULL() && 0 != metaData->GetRowGroupCount() )
            {
                error = "The metadata is not the one of this codec";
                return GUCEF::CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
            }
            GUCEF::CORE::CITableFileMetaDataPtr used = metaData;
            if ( used.IsNULL() )
            {
                GUCEF::CORE::CTSharedPtr< CTestPluginMetaData, GUCEF::MT::CMutex > made( new CTestPluginMetaData() );
                used = GUCEF::CORE::CITableFileMetaDataPtr( made );
            }
            reader = GUCEF::CORE::CITableReaderPtr( new CTestPluginReader( used ) );
            return GUCEF::CORE::DATATABLE_STATUS_OK;
        }
        error = "The test codec reads nothing";
        return GUCEF::CORE::DATATABLE_STATUS_UNSUPPORTED;
    }

    virtual GUCEF::CORE::TDataTableStatus CreateWriter( const GUCEF::CORE::IOAccessPtr&, const GUCEF::CORE::CDataTableSchemaPtr&,
                                                        const GUCEF::CORE::CDataTableWriterOptions&, GUCEF::CORE::CITableWriterPtr& writer, GUCEF::CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( IsFlagSet( "GUCEF_TESTPLUGIN_TC_MAKES_OBJECTS" ) )
        {
            writer = GUCEF::CORE::CITableWriterPtr( new CTestPluginWriter() );
            return GUCEF::CORE::DATATABLE_STATUS_OK;
        }
        error = "The test codec writes nothing";
        return GUCEF::CORE::DATATABLE_STATUS_UNSUPPORTED;
    }

    virtual GUCEF::CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return new CTestPluginTableCodec( m_typeName ); }

    private:

    GUCEF::CORE::CString m_typeName;
};

/*-------------------------------------------------------------------------*/

GUCEF::CORE::UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_GetApiVersion( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const char* version = getenv( "GUCEF_TESTPLUGIN_TC_API_VERSION" );
    return GUCEF_NULL != version ? (GUCEF::CORE::UInt32) strtoul( version, GUCEF_NULL, 10 ) : (GUCEF::CORE::UInt32) GUCEF_TABLECODECPLUGIN_API_VERSION;
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Init( const GUCEF::CORE::CValueList* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CountCall( "GUCEF_TESTPLUGIN_TC_INITS" );
    return IsFlagSet( "GUCEF_TESTPLUGIN_TC_INIT_FAILS" ) ? -1 : 0;
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_GetCodecCount( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return 1;
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::CITableCodec* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_CreateCodec( GUCEF::CORE::UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( 0 != index )
        return GUCEF_NULL;
    const char* typeName = getenv( "GUCEF_TESTPLUGIN_TC_TYPE" );
    return new CTestPluginTableCodec( GUCEF_NULL != typeName && '\0' != typeName[ 0 ] ? typeName : "testtable" );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Shutdown( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CountCall( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" );
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Description( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Table codec plugin used by the gucefCORE tests of the table codec plugin manager";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Copyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Version( GUCEF::CORE::TVersion* version ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    version->major = 1;
    version->minor = 0;
    version->patch = 0;
    version->release = 0;
}

/*-------------------------------------------------------------------------*/
