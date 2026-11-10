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

#include <stdlib.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CTABLECODECPLUGINMANAGER_H
#include "gucefCORE_CTableCodecPluginManager.h"
#define GUCEF_CORE_CTABLECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CTABLECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_TABLECODECPLUGINAPI_H
#include "gucefCORE_TableCodecPluginAPI.h"
#define GUCEF_CORE_TABLECODECPLUGINAPI_H
#endif /* GUCEF_CORE_TABLECODECPLUGINAPI_H ? */

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

#ifndef GUCEF_CORE_CGUARDEDTABLECODEC_H
#include "gucefCORE_CGuardedTableCodec.h"
#define GUCEF_CORE_CGUARDEDTABLECODEC_H
#endif /* GUCEF_CORE_CGUARDEDTABLECODEC_H ? */

#include "TestTableCodecRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A table codec with a type name and extensions of the test's choosing, that reads and writes nothing */
class CTestTableCodec : public CORE::CITableCodec
{
    public:

    CTestTableCodec( const CORE::CString& typeName, const CORE::CString& extensions ) : CORE::CITableCodec(), m_typeName( typeName ), m_extensions( extensions ) {}

    virtual CORE::CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_typeName; }
    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Test table codec " + m_typeName; }
    virtual CORE::CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE { return "none"; }

    virtual CORE::TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::TVersion version;
        version.major = 1; version.minor = 0; version.patch = 0; version.release = 0;
        return version;
    }

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_TABLECODEC_CAPABILITY_READ; }

    virtual void GetFileExtensions( CORE::CString::StringSet& extensions ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString::StringVector parts = m_extensions.ParseElements( ' ', false );
        for ( size_t i = 0; i < parts.size(); ++i )
            extensions.insert( parts[ i ] );
    }

    virtual CORE::TDataTableStatus ReadMetaData( CORE::CIOAccess&, const CORE::CValueList&, CORE::CITableFileMetaDataPtr&, CORE::CString& ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }
    virtual CORE::TDataTableStatus CreateReader( const CORE::IOAccessPtr&, const CORE::CITableFileMetaDataPtr&, const CORE::CValueList&, CORE::CITableReaderPtr&, CORE::CString& ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }
    virtual CORE::TDataTableStatus CreateWriter( const CORE::IOAccessPtr&, const CORE::CDataTableSchemaPtr&, const CORE::CDataTableWriterOptions&, CORE::CITableWriterPtr&, CORE::CString& ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestTableCodec( m_typeName, m_extensions ); }

    private:

    CORE::CString m_typeName;
    CORE::CString m_extensions;
};

/*-------------------------------------------------------------------------*/

CORE::CTableCodecRegistry::TTableCodecPtr
MakeCodec( const CORE::CString& typeName, const CORE::CString& extensions )
{
    CORE::CTSharedPtr< CORE::CITableCodec, MT::CMutex > typed( GUCEF_NEW CTestTableCodec( typeName, extensions ) );
    return CORE::CTableCodecRegistry::TTableCodecPtr( typed );
}

/*-------------------------------------------------------------------------*/

const char* const PluginGroup = "tablecodecplugin_tests";

int
Calls( const char* variable )
{
    const char* value = getenv( variable );
    return GUCEF_NULL != value ? atoi( value ) : 0;
}

void
SetVariable( const char* variable, const char* value )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( variable, value );
    #else
    if ( '\0' == value[ 0 ] )
        unsetenv( variable );
    else
        setenv( variable, value, 1 );
    #endif
}

void
ResetModuleVariables( void )
{
    SetVariable( "GUCEF_TESTPLUGIN_TC_TYPE", "" );
    SetVariable( "GUCEF_TESTPLUGIN_TC_API_VERSION", "" );
    SetVariable( "GUCEF_TESTPLUGIN_TC_INIT_FAILS", "" );
    SetVariable( "GUCEF_TESTPLUGIN_TC_MAKES_OBJECTS", "" );
}

#ifdef GUCEF_DEBUG_MODE
const char* const ModuleName = "gucefCORE_TestPlugin_d";
const char* const AltModuleName = "gucefCORE_TestPlugin";
#else
const char* const ModuleName = "gucefCORE_TestPlugin";
const char* const AltModuleName = "gucefCORE_TestPlugin_d";
#endif

/** The module of the tests as a table codec plugin in a group of its own; true when the plugin control took the configuration */
bool
LoadTableCodecPluginModule( void )
{
    CORE::CDataNode config( "Config" );
    CORE::CDataNode* group = config.AddChild( "PluginGroup" );
    group->SetAttribute( "GroupName", PluginGroup );
    group->SetAttribute( "LoadImmediately", "true" );
    CORE::CDataNode* block = group->AddChild( "PluginMetaData" );
    block->SetAttribute( "Description", "table codec plugin of the tests" );
    block->SetAttribute( "Path", "$MODULEDIR$" );
    block->SetAttribute( "Filename", ModuleName );
    block->SetAttribute( "AltFilename", AltModuleName );
    block->SetAttribute( "Type", "GucefTableCodecPlugin" );
    block->SetAttribute( "LoadFailAllowed", "true" );
    return CORE::CCoreGlobal::Instance()->GetPluginControl().LoadConfig( config );
}

/** Takes the group away, with what the plugin control knows of it */
void
RemovePluginGroup( void )
{
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    pluginControl.UnloadPluginGroup( PluginGroup );
    pluginControl.ProcessDeferredModuleUnloads( false );
    pluginControl.ClearMetaDataFromGroup( PluginGroup );
}

/** Releases the reader it was given on a thread of its own, the last reference of it */
class CReaderReleaseThread : public MT::CActiveObject
{
    public:

    CReaderReleaseThread( const CORE::CITableReaderPtr& reader ) : MT::CActiveObject(), m_reader( reader ), released( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_reader.Unlink();
        released.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    CORE::CITableReaderPtr m_reader;
    MT::CTAtomic< UInt32 > released;
};

void
WaitForReleased( MT::CTAtomic< UInt32 >& released )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( 0 == released.Load() && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 30000 )
        MT::PrecisionDelay( 1 );
}

bool
IsRegistered( const char* typeName )
{
    CORE::CTableCodecRegistry::TTableCodecPtr codec;
    return CORE::CCoreGlobal::Instance()->GetTableCodecRegistry().TryLookup( typeName, codec ) && !codec.IsNULL();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformTableCodecRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.TableCodecRegistry" );

    CORE::CTableCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetTableCodecRegistry();

    // the module of the tests is a table codec plugin as well; without it the cases that load it are skipped
    ResetModuleVariables();
    const bool pluginAvailable = LoadTableCodecPluginModule() && IsRegistered( "testtable" );
    RemovePluginGroup();

    GUCEF_TESTFW_TESTCASE( "TableCodecRegistry: codecs register once per type name and are found by name, case sensitive" )
    try
    {
        const UInt32 before = registry.GetCount();
        ASSERT_TRUE( registry.IsRegistered( "csv" ) );
        ASSERT_FALSE( registry.IsRegistered( "faketbl" ) );

        CORE::CTableCodecRegistry::TTableCodecPtr codec = MakeCodec( "faketbl", "fk1 fk2" );
        ASSERT_TRUE( registry.TryRegister( "faketbl", codec ) );
        ASSERT_TRUE( before + 1 == registry.GetCount() );
        ASSERT_FALSE( registry.TryRegister( "faketbl", MakeCodec( "faketbl", "other" ) ) );
        ASSERT_TRUE( before + 1 == registry.GetCount() );

        CORE::CTableCodecRegistry::TTableCodecPtr found;
        ASSERT_TRUE( registry.TryLookup( "faketbl", found ) && found.GetPointer() == codec.GetPointer() );
        ASSERT_FALSE( registry.TryLookup( "FAKETBL", found ) );
        ASSERT_TRUE( registry.TryLookup( "FAKETBL", found, false ) && found.GetPointer() == codec.GetPointer() );

        CORE::CString::StringVector names;
        registry.GetRegisteredObjNames( names );
        bool hasCsv = false, hasFake = false;
        for ( size_t i = 0; i < names.size(); ++i )
        {
            hasCsv = hasCsv || names[ i ] == "csv";
            hasFake = hasFake || names[ i ] == "faketbl";
        }
        ASSERT_TRUE( hasCsv && hasFake );

        ASSERT_TRUE( registry.TryUnregister( "faketbl" ) );
        ASSERT_FALSE( registry.TryUnregister( "faketbl" ) );
        ASSERT_FALSE( registry.IsRegistered( "faketbl" ) );
        ASSERT_TRUE( before == registry.GetCount() );
        ASSERT_TRUE( "faketbl" == codec->GetTypeName() );      // the codec lives on while somebody holds it
    }
    catch ( ... )
    {
        registry.TryUnregister( "faketbl" );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecRegistry: a codec is found by the extension it lists, in any case and with or without the dot" )
    try
    {
        ASSERT_TRUE( registry.TryRegister( "faketbl", MakeCodec( "faketbl", "fk1 FK2 .fk3" ) ) );
        ASSERT_TRUE( registry.TryRegister( "alsofake", MakeCodec( "alsofake", "fk1 other" ) ) );

        CORE::CTableCodecRegistry::TTableCodecPtr found;
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( "fk2", found ) && found->GetTypeName() == "faketbl" );
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( "FK2", found ) && found->GetTypeName() == "faketbl" );
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( ".Fk3", found ) && found->GetTypeName() == "faketbl" );
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( "..fk3", found ) && found->GetTypeName() == "faketbl" );
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( "other", found ) && found->GetTypeName() == "alsofake" );

        // two codecs that list it: the lowest type name
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( "fk1", found ) && found->GetTypeName() == "alsofake" );

        ASSERT_FALSE( registry.TryGetCodecForFileExtension( "nosuch", found ) );
        ASSERT_TRUE( found.IsNULL() );
        ASSERT_FALSE( registry.TryGetCodecForFileExtension( "", found ) );
        ASSERT_FALSE( registry.TryGetCodecForFileExtension( ".", found ) );
        ASSERT_FALSE( registry.TryGetCodecForFileExtension( "fk", found ) );
        ASSERT_FALSE( registry.TryGetCodecForFileExtension( "fk22", found ) );

        // csv and tsv belong to the built in codec
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( "csv", found ) && found->GetTypeName() == "csv" );
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( "tsv", found ) && found->GetTypeName() == "csv" );
        CORE::CTableCodecRegistry::TTableCodecPtr csv;
        ASSERT_TRUE( registry.TryGetCsvCodec( csv ) && csv.GetPointer() == found.GetPointer() );

        registry.TryUnregister( "faketbl" );
        registry.TryUnregister( "alsofake" );
        ASSERT_FALSE( registry.TryGetCodecForFileExtension( "fk2", found ) );
    }
    catch ( ... )
    {
        registry.TryUnregister( "faketbl" );
        registry.TryUnregister( "alsofake" );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecPluginManager: the manager has its own plugin type, the plugin API version rules are those of the design" )
    try
    {
        ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetTableCodecPluginManager().GetPluginType() == "GucefTableCodecPlugin" );

        const UInt32 host = GUCEF_TABLECODECPLUGIN_API_VERSION;
        ASSERT_TRUE( CORE::CTableCodecPlugin::IsApiVersionCompatible( host, host ) );
        ASSERT_FALSE( CORE::CTableCodecPlugin::IsApiVersionCompatible( host + ( 1 << 16 ), host ) );
        ASSERT_FALSE( CORE::CTableCodecPlugin::IsApiVersionCompatible( ( ( host >> 16 ) + 1 ) << 16, host ) );
        ASSERT_FALSE( CORE::CTableCodecPlugin::IsApiVersionCompatible( 0, host ) || ( host >> 16 ) == 0 );
        ASSERT_FALSE( CORE::CTableCodecPlugin::IsApiVersionCompatible( host + 1, host ) );      // a newer minor than the host's
        ASSERT_TRUE( CORE::CTableCodecPlugin::IsApiVersionCompatible( host, host + 3 ) );       // an older minor than the host's

        // members added in a minor version are only called on objects of modules built for it (1.1: VerifySource)
        CORE::CTableCodecModuleLifetime lifetime;
        ASSERT_FALSE( lifetime.ModuleHasApiMinorVersion( 1 ) );                                    // not known
        lifetime.SetModuleApiVersion( ( GUCEF_TABLECODECPLUGIN_API_VERSION_MAJOR << 16 ) | 0 );
        ASSERT_FALSE( lifetime.ModuleHasApiMinorVersion( 1 ) );
        lifetime.SetModuleApiVersion( ( GUCEF_TABLECODECPLUGIN_API_VERSION_MAJOR << 16 ) | 1 );
        ASSERT_TRUE( lifetime.ModuleHasApiMinorVersion( 1 ) && lifetime.ModuleHasApiMinorVersion( 0 ) && !lifetime.ModuleHasApiMinorVersion( 2 ) );
        lifetime.SetModuleApiVersion( ( ( GUCEF_TABLECODECPLUGIN_API_VERSION_MAJOR + 1 ) << 16 ) | 5 );
        ASSERT_FALSE( lifetime.ModuleHasApiMinorVersion( 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecPluginManager: a plugin module registers its codec at load and takes it out at unload, calling Init and Shutdown once each" )
    GUCEF_TESTFW_SKIP_IF( !pluginAvailable, "gucefCORE_TestPlugin is not available" );
    try
    {
        ResetModuleVariables();
        const int inits = Calls( "GUCEF_TESTPLUGIN_TC_INITS" ), shutdowns = Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ), destroyed = Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" );
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        ASSERT_TRUE( LoadTableCodecPluginModule() && IsRegistered( "testtable" ) );

        ASSERT_TRUE( inits + 1 == Calls( "GUCEF_TESTPLUGIN_TC_INITS" ) );
        ASSERT_TRUE( shutdowns == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
        CORE::CTableCodecRegistry::TTableCodecPtr codec;
        ASSERT_TRUE( registry.TryLookup( "testtable", codec ) );
        ASSERT_TRUE( codec->GetTypeName() == "testtable" );
        ASSERT_TRUE( codec->GetName() == "Table codec of the test plugin" );
        CORE::CTableCodecRegistry::TTableCodecPtr byExtension;
        ASSERT_TRUE( registry.TryGetCodecForFileExtension( "tstbl", byExtension ) && byExtension.GetPointer() == codec.GetPointer() );
        codec.Unlink();
        byExtension.Unlink();

        ASSERT_TRUE( pluginControl.UnloadPluginGroup( PluginGroup ) );
        ASSERT_FALSE( IsRegistered( "testtable" ) );
        ASSERT_TRUE( 0 == pluginControl.GetDeferredModuleUnloadCount() );
        ASSERT_TRUE( shutdowns + 1 == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
        ASSERT_TRUE( destroyed + 1 == Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" ) );
        RemovePluginGroup();
    }
    catch ( ... )
    {
        ResetModuleVariables();
        RemovePluginGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecPluginManager: a module stays loaded while a codec of it is referenced, and goes when the last reference is released" )
    GUCEF_TESTFW_SKIP_IF( !pluginAvailable, "gucefCORE_TestPlugin is not available" );
    try
    {
        ResetModuleVariables();
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        const int inits = Calls( "GUCEF_TESTPLUGIN_TC_INITS" ), shutdowns = Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ), destroyed = Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" );
        ASSERT_TRUE( LoadTableCodecPluginModule() && IsRegistered( "testtable" ) );

        // an open table, or a reader, would hold the codec like this
        CORE::CTableCodecRegistry::TTableCodecPtr held;
        ASSERT_TRUE( registry.TryLookup( "testtable", held ) );

        ASSERT_TRUE( pluginControl.UnloadPluginGroup( PluginGroup ) );
        ASSERT_FALSE( IsRegistered( "testtable" ) );                                        // nobody finds it any more
        ASSERT_TRUE( 1 == pluginControl.GetDeferredModuleUnloadCount() );                   // the unload waits
        ASSERT_TRUE( shutdowns == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );               // the module was not shut down
        ASSERT_TRUE( destroyed == Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" ) );
        ASSERT_TRUE( held->GetTypeName() == "testtable" );                                  // and its code is there for the holder
        ASSERT_TRUE( held->GetVersion().major == 1 );

        // asking again changes nothing as long as the codec is held
        ASSERT_TRUE( 1 == pluginControl.ProcessDeferredModuleUnloads( false ) );
        ASSERT_TRUE( shutdowns == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
        ASSERT_TRUE( held->GetTypeName() == "testtable" );

        // the holder lets go: the next time the plugin control looks, the module is shut down and unloaded
        held.Unlink();
        ASSERT_TRUE( 0 == pluginControl.ProcessDeferredModuleUnloads( false ) );
        ASSERT_TRUE( 0 == pluginControl.GetDeferredModuleUnloadCount() );
        ASSERT_TRUE( shutdowns + 1 == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
        ASSERT_TRUE( destroyed + 1 == Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" ) );

        // the module can be loaded again
        ASSERT_TRUE( pluginControl.LoadPluginGroup( PluginGroup ) );
        ASSERT_TRUE( IsRegistered( "testtable" ) );
        ASSERT_TRUE( inits + 2 == Calls( "GUCEF_TESTPLUGIN_TC_INITS" ) );
        RemovePluginGroup();
        ASSERT_TRUE( shutdowns + 2 == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
    }
    catch ( ... )
    {
        ResetModuleVariables();
        RemovePluginGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecPluginManager: a module stays loaded while a reader, metadata or writer made by its code is alive, whatever became of the codec" )
    GUCEF_TESTFW_SKIP_IF( !pluginAvailable, "gucefCORE_TestPlugin is not available" );
    try
    {
        // what the code of the module makes has its destructor in the module: the object that is left alive is kept on the
        // heap so that a failure of the checks below does not run a destructor of unmapped code
        enum EKind { KIND_READER, KIND_METADATA, KIND_WRITER, KIND_CLONE, KIND_READER_ON_METADATA, KIND_COUNT };
        static const char* const destroyedCounter[ KIND_COUNT ] = { "GUCEF_TESTPLUGIN_TC_READERSDESTROYED", "GUCEF_TESTPLUGIN_TC_METADATADESTROYED",
                                                                    "GUCEF_TESTPLUGIN_TC_WRITERSDESTROYED", "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED",
                                                                    "GUCEF_TESTPLUGIN_TC_READERSDESTROYED" };
        for ( int kind=0; kind<KIND_COUNT; ++kind )
        {
            ResetModuleVariables();
            SetVariable( "GUCEF_TESTPLUGIN_TC_MAKES_OBJECTS", "1" );
            CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
            const int shutdowns = Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" );
            ASSERT_TRUE( LoadTableCodecPluginModule() && IsRegistered( "testtable" ) );

            CORE::CITableReaderPtr* reader = GUCEF_NULL;
            CORE::CITableFileMetaDataPtr* metaData = GUCEF_NULL;
            CORE::CITableWriterPtr* writer = GUCEF_NULL;
            CORE::CICloneable* clone = GUCEF_NULL;
            {
                CORE::CTableCodecRegistry::TTableCodecPtr codec;
                ASSERT_TRUE( registry.TryLookup( "testtable", codec ) );
                CORE::CString error;
                switch ( kind )
                {
                    case KIND_READER:
                    {
                        reader = GUCEF_NEW CORE::CITableReaderPtr();
                        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == codec->CreateReader( CORE::IOAccessPtr(), CORE::CITableFileMetaDataPtr(), CORE::CValueList(), *reader, error ) );
                        break;
                    }
                    case KIND_METADATA:
                    {
                        metaData = GUCEF_NEW CORE::CITableFileMetaDataPtr();
                        CORE::CDynamicBuffer nothing;
                        CORE::CDynamicBufferAccess source( nothing );
                        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == codec->ReadMetaData( source, CORE::CValueList(), *metaData, error ) );
                        break;
                    }
                    case KIND_WRITER:
                    {
                        writer = GUCEF_NEW CORE::CITableWriterPtr();
                        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == codec->CreateWriter( CORE::IOAccessPtr(), CORE::CDataTableSchemaPtr(), CORE::CDataTableWriterOptions(), *writer, error ) );
                        break;
                    }
                    case KIND_CLONE:
                    {
                        clone = codec->Clone();
                        ASSERT_TRUE( GUCEF_NULL != clone );
                        break;
                    }
                    default:
                    {
                        // a reader over metadata that the same codec read: the module gets its own metadata object back
                        CORE::CDynamicBuffer nothing;
                        CORE::CDynamicBufferAccess source( nothing );
                        CORE::CITableFileMetaDataPtr read;
                        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == codec->ReadMetaData( source, CORE::CValueList(), read, error ) );
                        reader = GUCEF_NEW CORE::CITableReaderPtr();
                        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == codec->CreateReader( CORE::IOAccessPtr(), read, CORE::CValueList(), *reader, error ) );
                        break;
                    }
                }
            }                                                                               // the codec is released

            const int destroyed = Calls( destroyedCounter[ kind ] );
            ASSERT_TRUE( pluginControl.UnloadPluginGroup( PluginGroup ) );
            ASSERT_FALSE( IsRegistered( "testtable" ) );
            const bool deferred = 1 == pluginControl.GetDeferredModuleUnloadCount();
            ASSERT_TRUE( deferred );                                                        // the unload waits
            ASSERT_TRUE( shutdowns == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
            ASSERT_TRUE( destroyed == Calls( destroyedCounter[ kind ] ) );
            if ( !deferred )
                continue;                                                                   // the module is gone: the object stays where it is

            // the code of the module is there for the holder
            if ( GUCEF_NULL != reader )   ASSERT_TRUE( (*reader)->GetLastErrorMessage() == "test reader" );
            if ( GUCEF_NULL != metaData ) ASSERT_TRUE( (*metaData)->GetRowGroupCount() == 0 );
            if ( GUCEF_NULL != writer )   ASSERT_TRUE( (*writer)->GetLastErrorMessage() == "test writer" );
            ASSERT_TRUE( 1 == pluginControl.ProcessDeferredModuleUnloads( false ) );
            ASSERT_TRUE( shutdowns == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );

            // the holder lets go: the destructor ran in the module, only then can the module go
            GUCEF_DELETE reader;
            GUCEF_DELETE metaData;
            GUCEF_DELETE writer;
            GUCEF_DELETE clone;
            ASSERT_TRUE( destroyed + 1 <= Calls( destroyedCounter[ kind ] ) );
            ASSERT_TRUE( shutdowns == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );          // not unloaded from inside the destructor
            ASSERT_TRUE( 1 == pluginControl.GetDeferredModuleUnloadCount() );
            ASSERT_TRUE( 0 == pluginControl.ProcessDeferredModuleUnloads( false ) );
            ASSERT_TRUE( shutdowns + 1 == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
            RemovePluginGroup();
        }
        ResetModuleVariables();
    }
    catch ( ... )
    {
        ResetModuleVariables();
        RemovePluginGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecPluginManager: the last reader of a module released on another thread while the module is being unloaded: the module goes only after the destructor has finished" )
    GUCEF_TESTFW_SKIP_IF( !pluginAvailable, "gucefCORE_TestPlugin is not available" );
    try
    {
        ResetModuleVariables();
        SetVariable( "GUCEF_TESTPLUGIN_TC_MAKES_OBJECTS", "1" );
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        bool failed = false;
        for ( UInt32 round=0; round<60 && !failed; ++round )
        {
            const int shutdowns = Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ), destroyed = Calls( "GUCEF_TESTPLUGIN_TC_READERSDESTROYED" );
            ASSERT_TRUE( LoadTableCodecPluginModule() && IsRegistered( "testtable" ) );
            CORE::CITableReaderPtr reader;
            {
                CORE::CTableCodecRegistry::TTableCodecPtr codec;
                ASSERT_TRUE( registry.TryLookup( "testtable", codec ) );
                CORE::CString error;
                ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == codec->CreateReader( CORE::IOAccessPtr(), CORE::CITableFileMetaDataPtr(), CORE::CValueList(), reader, error ) );
            }
            ASSERT_TRUE( pluginControl.UnloadPluginGroup( PluginGroup ) );
            if ( 1 != pluginControl.GetDeferredModuleUnloadCount() )
            {
                ASSERT_TRUE( 1 == pluginControl.GetDeferredModuleUnloadCount() );
                failed = true;                          // the module is gone with the reader still alive: nothing may touch it
                GUCEF_NEW CORE::CITableReaderPtr( reader );
                break;
            }

            // another thread releases the reader while this one asks for the unload again and again
            CReaderReleaseThread* thread = GUCEF_NEW CReaderReleaseThread( reader );
            reader.Unlink();
            ASSERT_TRUE( thread->Activate() );
            UInt32 pending = 1;
            for ( UInt32 spin=0; spin<2000000 && 0 != pending; ++spin )
                pending = pluginControl.ProcessDeferredModuleUnloads( false );
            WaitForReleased( thread->released );
            thread->Deactivate( false, true );
            GUCEF_DELETE thread;
            pending = pluginControl.ProcessDeferredModuleUnloads( false );
            ASSERT_TRUE( 0 == pending );
            ASSERT_TRUE( shutdowns + 1 == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
            ASSERT_TRUE( destroyed + 1 == Calls( "GUCEF_TESTPLUGIN_TC_READERSDESTROYED" ) );
            RemovePluginGroup();
        }
        ResetModuleVariables();
    }
    catch ( ... )
    {
        ResetModuleVariables();
        RemovePluginGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecPluginManager: the next load or unload that the plugin control does lets a module go that is free, and a module in use is not initialized a second time" )
    GUCEF_TESTFW_SKIP_IF( !pluginAvailable, "gucefCORE_TestPlugin is not available" );
    try
    {
        ResetModuleVariables();
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        const int inits = Calls( "GUCEF_TESTPLUGIN_TC_INITS" ), shutdowns = Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" );
        ASSERT_TRUE( LoadTableCodecPluginModule() && IsRegistered( "testtable" ) );

        CORE::CTableCodecRegistry::TTableCodecPtr held;
        ASSERT_TRUE( registry.TryLookup( "testtable", held ) );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( PluginGroup ) );
        ASSERT_TRUE( 1 == pluginControl.GetDeferredModuleUnloadCount() );

        // loaded again while the codec of the first load is in use: refused, the module has not been shut down
        pluginControl.LoadPluginGroup( PluginGroup );
        ASSERT_FALSE( IsRegistered( "testtable" ) );
        ASSERT_TRUE( inits + 1 == Calls( "GUCEF_TESTPLUGIN_TC_INITS" ) );
        ASSERT_TRUE( shutdowns == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
        ASSERT_TRUE( held->GetTypeName() == "testtable" );

        // once it is free, loading the group finishes the unload first and then loads the module anew
        held.Unlink();
        ASSERT_TRUE( pluginControl.LoadPluginGroup( PluginGroup ) );
        ASSERT_TRUE( 0 == pluginControl.GetDeferredModuleUnloadCount() );
        ASSERT_TRUE( IsRegistered( "testtable" ) );
        ASSERT_TRUE( inits + 2 == Calls( "GUCEF_TESTPLUGIN_TC_INITS" ) );
        ASSERT_TRUE( shutdowns + 1 == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
        RemovePluginGroup();
    }
    catch ( ... )
    {
        ResetModuleVariables();
        RemovePluginGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecPluginManager: a module of another API version is refused and none of it is called" )
    GUCEF_TESTFW_SKIP_IF( !pluginAvailable, "gucefCORE_TestPlugin is not available" );
    try
    {
        ResetModuleVariables();
        const int inits = Calls( "GUCEF_TESTPLUGIN_TC_INITS" ), shutdowns = Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ), destroyed = Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" );

        const UInt32 host = GUCEF_TABLECODECPLUGIN_API_VERSION;
        const UInt32 otherMajor = host + ( 1 << 16 );
        const UInt32 newerMinor = host + 1;
        const UInt32 versions[] = { otherMajor, newerMinor, 0 };
        for ( size_t i = 0; i < sizeof( versions ) / sizeof( versions[ 0 ] ); ++i )
        {
            char text[ 24 ];
            snprintf( text, sizeof( text ), "%u", (unsigned) versions[ i ] );
            SetVariable( "GUCEF_TESTPLUGIN_TC_API_VERSION", text );
            LoadTableCodecPluginModule();
            ASSERT_FALSE( IsRegistered( "testtable" ) );
            ASSERT_TRUE( inits == Calls( "GUCEF_TESTPLUGIN_TC_INITS" ) );
            ASSERT_TRUE( shutdowns == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
            ASSERT_TRUE( destroyed == Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" ) );
            RemovePluginGroup();
        }

        // the version of this host is accepted
        SetVariable( "GUCEF_TESTPLUGIN_TC_API_VERSION", "" );
        if ( LoadTableCodecPluginModule() && IsRegistered( "testtable" ) )
            ASSERT_TRUE( inits + 1 == Calls( "GUCEF_TESTPLUGIN_TC_INITS" ) );
        RemovePluginGroup();
    }
    catch ( ... )
    {
        ResetModuleVariables();
        RemovePluginGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TableCodecPluginManager: a module whose Init fails, or whose codec has a type name that is taken, is shut down and not registered" )
    GUCEF_TESTFW_SKIP_IF( !pluginAvailable, "gucefCORE_TestPlugin is not available" );
    try
    {
        ResetModuleVariables();
        int inits = Calls( "GUCEF_TESTPLUGIN_TC_INITS" ), shutdowns = Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" );

        SetVariable( "GUCEF_TESTPLUGIN_TC_INIT_FAILS", "1" );
        LoadTableCodecPluginModule();
        ASSERT_FALSE( IsRegistered( "testtable" ) );
        ASSERT_TRUE( inits + 1 == Calls( "GUCEF_TESTPLUGIN_TC_INITS" ) );
        ASSERT_TRUE( shutdowns + 1 == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
        RemovePluginGroup();
        ResetModuleVariables();

        // the name csv is the built in codec's: the one of the module is left out and so is the module
        inits = Calls( "GUCEF_TESTPLUGIN_TC_INITS" );
        shutdowns = Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" );
        const int destroyed = Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" );
        SetVariable( "GUCEF_TESTPLUGIN_TC_TYPE", "csv" );
        LoadTableCodecPluginModule();
        CORE::CTableCodecRegistry::TTableCodecPtr csv;
        ASSERT_TRUE( registry.TryGetCsvCodec( csv ) );
        ASSERT_TRUE( csv->GetName() == "CSV (RFC 4180)" );
        ASSERT_TRUE( inits + 1 == Calls( "GUCEF_TESTPLUGIN_TC_INITS" ) );
        ASSERT_TRUE( shutdowns + 1 == Calls( "GUCEF_TESTPLUGIN_TC_SHUTDOWNS" ) );
        ASSERT_TRUE( destroyed + 1 == Calls( "GUCEF_TESTPLUGIN_TC_CODECSDESTROYED" ) );
        ASSERT_TRUE( 0 == CORE::CCoreGlobal::Instance()->GetPluginControl().GetDeferredModuleUnloadCount() );
        RemovePluginGroup();
        ResetModuleVariables();

        // and the built in codec is as it was
        ASSERT_TRUE( registry.TryGetCsvCodec( csv ) && csv->GetName() == "CSV (RFC 4180)" );
    }
    catch ( ... )
    {
        ResetModuleVariables();
        RemovePluginGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
