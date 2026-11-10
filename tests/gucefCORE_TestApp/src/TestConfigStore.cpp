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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CCONFIGSTORE_H
#endif /* GUCEF_CORE_CCONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H
#include "gucefCORE_CFaultTolerantConfigScope.h"
#define GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H
#endif /* GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CINIPARSER_H
#include "gucefCORE_CIniParser.h"
#define GUCEF_CORE_CINIPARSER_H
#endif /* GUCEF_CORE_CINIPARSER_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "TestConfigStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** Writes one value into its own section of the config */
class CTestConfigurable : public CORE::CGloballyConfigurable
{
    public:

    CORE::CString value;

    CTestConfigurable( void ) : CORE::CGloballyConfigurable( false ), value() {}

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDataNode* section = cfg.Structure( "TestConfigStore/Section", '/' );
        section->SetAttribute( "value", value );
        return true;
    }

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTestConfigurable";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

/** Reads one value from its own section of the config */
class CReadingConfigurable : public CORE::CGloballyConfigurable
{
    public:

    CORE::CString value;

    CReadingConfigurable( void ) : CORE::CGloballyConfigurable( false ), value() {}

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::CDataNode* section = cfg.Search( "Managed", '/', true, true );
        if ( GUCEF_NULL != section )
            value = section->GetAttributeValueOrChildValueByName( "value" ).AsString( value, true );
        return true;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CReadingConfigurable";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

/** A configurable whose config cannot be loaded */
class CFailingConfigurable : public CORE::CGloballyConfigurable
{
    public:

    CFailingConfigurable( void ) : CORE::CGloballyConfigurable( false ) {}

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CFailingConfigurable";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

/** Whether the config applies as a whole while the configurables exist that there are now, the ones that cannot load their config included */
bool
ConfigAppliesAsAWhole( CORE::CConfigStore& configStore )
{
    CORE::CDataNode config;
    return configStore.ApplyConsolidatedConfig( config );
}

/*-------------------------------------------------------------------------*/

const UInt32 LockOrderRunTimeInMs = 3000;
const UInt32 LockOrderStallTimeInMs = 2000;
const Int32 LockOrderTaskEndTimeInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS + 15000;

UInt64
LockOrderElapsedMs( UInt64 startTicks )
{
    return ( MT::PrecisionTickCount() - startTicks ) * 1000 / MT::PrecisionTimerResolution();
}

/*-------------------------------------------------------------------------*/

/** What the tasks of the lock order case share with the test thread. The tasks may outlive a case that saw a
 *  deadlock, so it is made on the heap and given up then */
struct SConfigLockOrderCase
{
    MT::CTAtomic< bool > stop;
    MT::CTAtomic< UInt32 > pluginRounds;
    MT::CTAtomic< UInt32 > configRounds;
    MT::CTAtomic< UInt32 > exceptions;

    SConfigLockOrderCase( void ) : stop( false ), pluginRounds( 0 ), configRounds( 0 ), exceptions( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/** Does under the lock of the plugin control what loading a plugin does around the module: a fault tolerant config scope */
class CPluginSideTask : public CORE::CTaskConsumer
{
    public:

    explicit CPluginSideTask( SConfigLockOrderCase& lockCase ) : CORE::CTaskConsumer(), m_case( lockCase ) {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestConfigStore::CPluginSideTask"; }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
        CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
        CORE::CConfigStore& configStore = coreGlobal->GetConfigStore();
        while ( !m_case.stop.Load( MT::MEMORDER_ACQUIRE ) )
        {
            try
            {
                MT::CObjectScopeLock pluginControlLock( pluginControl.AsLockable() );
                CORE::CFaultTolerantConfigScope faultTolerantScope( configStore, true );
                m_case.pluginRounds.FetchAdd( 1 );
            }
            catch ( ... )
            {
                m_case.exceptions.FetchAdd( 1 );
                break;
            }
            MT::ThreadDelay( 0 );
        }
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE {}

    private:

    SConfigLockOrderCase& m_case;
};

typedef CORE::CTSharedPtr< CPluginSideTask, MT::CMutex > TPluginSideTaskPtr;

/*-------------------------------------------------------------------------*/

/** Applies a config to every globally configurable object, the plugin control included, as loading the config does */
class CConfigSideTask : public CORE::CTaskConsumer
{
    public:

    explicit CConfigSideTask( SConfigLockOrderCase& lockCase ) : CORE::CTaskConsumer(), m_case( lockCase ) {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestConfigStore::CConfigSideTask"; }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
        while ( !m_case.stop.Load( MT::MEMORDER_ACQUIRE ) )
        {
            try
            {
                ConfigAppliesAsAWhole( configStore );
                m_case.configRounds.FetchAdd( 1 );
            }
            catch ( ... )
            {
                m_case.exceptions.FetchAdd( 1 );
                break;
            }
            MT::ThreadDelay( 0 );
        }
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE {}

    private:

    SConfigLockOrderCase& m_case;
};

typedef CORE::CTSharedPtr< CConfigSideTask, MT::CMutex > TConfigSideTaskPtr;

/*-------------------------------------------------------------------------*/

/** What a load on another thread shares with the test thread: the hold of the configurable that is being loaded */
struct SConfigInUseCase
{
    MT::CTAtomic< bool > hold;              /**< the holding configurable waits in its LoadConfig() until release */
    MT::CTAtomic< bool > inLoad;
    MT::CTAtomic< bool > release;
    MT::CTAtomic< bool > loadDone;
    MT::CTAtomic< bool > unregistered;
    MT::CTAtomic< bool > intactAfterRelease;
    CORE::CGloballyConfigurable* toUnregister;

    SConfigInUseCase( void ) : hold( true ), inLoad( false ), release( false ), loadDone( false ), unregistered( false ), intactAfterRelease( false ), toUnregister( GUCEF_NULL ) {}
};

/*-------------------------------------------------------------------------*/

bool
ConfigWaitFor( const MT::CTAtomic< bool >& flag, UInt32 timeoutInMs )
{
    const UInt64 startTicks = MT::PrecisionTickCount();
    while ( !flag.Load( MT::MEMORDER_ACQUIRE ) )
    {
        if ( LockOrderElapsedMs( startTicks ) > timeoutInMs )
            return false;
        MT::ThreadDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Counts its loads; while the case holds, its LoadConfig() waits for the release and then looks at itself again */
class CHoldingConfigurable : public CORE::CGloballyConfigurable
{
    public:

    static const UInt32 AliveMark = 0xC0F1C0F1;

    MT::CTAtomic< UInt32 > loads;

    explicit CHoldingConfigurable( SConfigInUseCase* inUseCase ) : CORE::CGloballyConfigurable( false ), loads( 0 ), m_case( inUseCase ), m_alive( AliveMark ) {}

    virtual ~CHoldingConfigurable()
    {
        // a configurable leaves the store in its own destructor: a load that calls it is waited for while it is whole
        UnregisterFromConfigStore();
        m_alive = 0;
    }

    void LeaveConfigStore( void ) { UnregisterFromConfigStore(); }

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE
    {
        loads.FetchAdd( 1 );
        if ( GUCEF_NULL != m_case && m_case->hold.Load( MT::MEMORDER_ACQUIRE ) )
        {
            m_case->inLoad.Store( true, MT::MEMORDER_RELEASE );
            ConfigWaitFor( m_case->release, 20000 );
            m_case->intactAfterRelease.Store( AliveMark == m_alive, MT::MEMORDER_RELEASE );
        }
        return true;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CHoldingConfigurable";
        return classTypeName;
    }

    private:

    SConfigInUseCase* m_case;
    UInt32 m_alive;
};

/*-------------------------------------------------------------------------*/

/** Applies a config to every globally configurable object, on a thread of its own */
UInt32 GUCEF_CALLSPEC_PREFIX
ConfigLoadThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SConfigInUseCase& inUseCase = *static_cast< SConfigInUseCase* >( data );
    ConfigAppliesAsAWhole( CORE::CCoreGlobal::Instance()->GetConfigStore() );
    inUseCase.loadDone.Store( true, MT::MEMORDER_RELEASE );
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Destroys the configurable of the case on a thread of its own */
UInt32 GUCEF_CALLSPEC_PREFIX
ConfigDestroyThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SConfigInUseCase& inUseCase = *static_cast< SConfigInUseCase* >( data );
    GUCEF_DELETE inUseCase.toUnregister;
    inUseCase.unregistered.Store( true, MT::MEMORDER_RELEASE );
    return 0;
}

/*-------------------------------------------------------------------------*/

MT::SThreadData*
StartConfigThread( MT::TThreadFunc threadMain, SConfigInUseCase* inUseCase )
{
    MT::SThreadData* thread = MT::ThreadDataReserve();
    if ( GUCEF_NULL == thread )
        return GUCEF_NULL;
    if ( 0 == MT::ThreadCreate( thread, threadMain, inUseCase ) )
    {
        MT::ThreadDataCleanup( thread );
        return GUCEF_NULL;
    }
    return thread;
}

/*-------------------------------------------------------------------------*/

bool
EndConfigThread( MT::SThreadData* thread )
{
    if ( GUCEF_NULL == thread )
        return true;
    if ( 1 != MT::ThreadWait( thread, LockOrderTaskEndTimeInMs ) )
        return false;
    MT::ThreadDataCleanup( thread );
    return true;
}

/*-------------------------------------------------------------------------*/

/** A scope that ends on a return from the block it is in */
bool
FaultTolerantEarlyReturn( CORE::CConfigStore& configStore, bool returnEarly )
{
    CORE::CFaultTolerantConfigScope faultTolerantScope( configStore );
    if ( returnEarly )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** A scope that ends on an exception that unwinds the block it is in */
void
FaultTolerantThrowing( CORE::CConfigStore& configStore )
{
    CORE::CFaultTolerantConfigScope faultTolerantScope( configStore );
    throw 42;
}

/*-------------------------------------------------------------------------*/

/** The content of a text file, empty when there is none */
CORE::CString
ReadText( const CORE::CString& path )
{
    CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path ) )
        return CORE::CString::Empty;
    return CORE::CString( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
GetJsonCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{
    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    if ( coreGlobal->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;
    CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( "$MODULEDIR$/dstorepluginJSONPARSER_d", "test", true ) )
        pluginControl.AddPluginFromDir( "$MODULEDIR$/dstorepluginJSONPARSER", "test", true );
    return coreGlobal->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformConfigStoreTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ConfigStore" );

    GUCEF_TESTFW_TESTCASE( "ConfigStore: Saving writes what the configurables give and keeps the rest of the file" )
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        GUCEF_TESTFW_SKIP( "the JSON codec plugin is not available" );
    try
    {
        CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
        const CORE::CString previousConfigFile = configStore.GetConsolidatedConfigFile();

        // a config file with a section of another part of the application
        const CORE::CString path = CORE::CombinePath( CORE::RelativePath( "$TEMPDIR$" ), TESTFW::UniqueTestName( "gucefCORE_TestApp_ConfigStore" ) + ".json" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( path, "{ \"Other\": { \"keep\": \"yes\" }, \"TestConfigStore\": { \"Section\": { \"value\": \"old\" } } }" ) );
        configStore.SetConfigFile( path );

        {
            CTestConfigurable configurable;
            configurable.value = "new";
            ASSERT_TRUE( configStore.SaveConsolidatedConfig( "Config", true ) );
        }

        CORE::CDataNode written;
        ASSERT_TRUE( codec->BuildDataTree( &written, path ) );
        const CORE::CDataNode* other = written.Search( "Other", '/', true, true );
        const CORE::CDataNode* section = written.Search( "TestConfigStore/Section", '/', true, true );
        ASSERT_TRUE( GUCEF_NULL != other && "yes" == other->GetAttributeValueOrChildValueByName( "keep" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL != section && "new" == section->GetAttributeValueOrChildValueByName( "value" ).AsString() );

        // without preserving only what the configurables give is written
        {
            CTestConfigurable configurable;
            configurable.value = "only";
            ASSERT_TRUE( configStore.SaveConsolidatedConfig( "Config", false ) );
        }
        written.Clear();
        ASSERT_TRUE( codec->BuildDataTree( &written, path ) );
        ASSERT_TRUE( GUCEF_NULL == written.Search( "Other", '/', true, true ) );
        section = written.Search( "TestConfigStore/Section", '/', true, true );
        ASSERT_TRUE( GUCEF_NULL != section && "only" == section->GetAttributeValueOrChildValueByName( "value" ).AsString() );

        CORE::DeleteFile( path );
        configStore.SetConfigFile( previousConfigFile );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConfigStore: A path is written with the variable of its directory and resolves back; the settings directory of applications is there" )
    try
    {
        const CORE::CString appConfigDir = CORE::ResolveVars( "$APPCONFIGDIR$" );
        ASSERT_FALSE( appConfigDir.IsNULLOrEmpty() || appConfigDir.HasSubstr( "$APPCONFIGDIR$", true ) >= 0 );
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        const CORE::CString xdgConfig = CORE::GetEnv( "XDG_CONFIG_HOME" );
        ASSERT_TRUE( appConfigDir == ( xdgConfig.IsNULLOrEmpty() ? CORE::CombinePath( CORE::GetEnv( "HOME" ), ".config" ) : xdgConfig ) );
        #endif
        CORE::TSpecialDirSet available;
        ASSERT_TRUE( CORE::ListAvailableSpecialDirs( available ) && available.find( CORE::SPECIALDIR_APP_CONFIG_DIR ) != available.end() );
        ASSERT_TRUE( CORE::CString( "SPECIALDIR_APP_CONFIG_DIR" ) == CORE::SpecialDirToString( CORE::SPECIALDIR_APP_CONFIG_DIR ) );

        // to the variable and back
        const CORE::CString inConfig = CORE::CombinePath( appConfigDir, "MyApp" ).ReplaceChar( '\\', '/' );
        ASSERT_TRUE( "$APPCONFIGDIR$/MyApp" == CORE::UnresolveVars( inConfig ) );
        ASSERT_TRUE( inConfig == CORE::ResolveVars( CORE::UnresolveVars( inConfig ) ).ReplaceChar( '\\', '/' ) );
        const CORE::CString temp = CORE::ResolveVars( "$TEMPDIR$" ).ReplaceChar( '\\', '/' );
        ASSERT_TRUE( CORE::CString( "$TEMPDIR$/a/b" ) == CORE::UnresolveVars( temp + "/a/b" ) );
        // a directory whose name only starts the same is not in it, a path in no directory of a variable stays
        ASSERT_TRUE( CORE::UnresolveVars( temp + "2/x" ) == temp + "2/x" );
        ASSERT_TRUE( "/nowhere/of/them" == CORE::UnresolveVars( "/nowhere/of/them" ) );

        // copying a file: the destination is only replaced when asked
        const CORE::CString from = CORE::CombinePath( temp, TESTFW::UniqueTestName( "gucefCORE_TestApp_CopyFrom" ) + ".txt" );
        const CORE::CString to = CORE::CombinePath( temp, TESTFW::UniqueTestName( "gucefCORE_TestApp_CopyTo" ) + ".txt" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( from, "first" ) && CORE::CopyFile( from, to, true ) && "first" == ReadText( to ) );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( from, "second" ) && !CORE::CopyFile( from, to, false ) && "first" == ReadText( to ) );
        ASSERT_TRUE( CORE::CopyFile( from, to, true ) && "second" == ReadText( to ) );
        ASSERT_FALSE( CORE::CopyFile( CORE::CombinePath( temp, "gucefCORE_TestApp_NoSuchFile.txt" ), to, true ) );
        CORE::DeleteFile( from );
        CORE::DeleteFile( to );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConfigStore: One value of an ini file is set and the rest of the file stays: comments, order, quotes and line endings" )
    try
    {
        const CORE::CString path = CORE::CombinePath( CORE::RelativePath( "$TEMPDIR$" ), TESTFW::UniqueTestName( "gucefCORE_TestApp_Update" ) + ".ini" );
        const CORE::CString original = "; the plugins and where the config is\r\n"
                                       "[Bootstrap\\PluginGroup]\r\n"
                                       "\"GroupName\"=\"bootstrap\"\r\n"
                                       "\r\n"
                                       "[ConfigStore]\r\n"
                                       "; kept per user\r\n"
                                       "\"ConfigLocation\"=\"$APPCONFIGDIR$/MyApp\"\r\n"
                                       "\"ConfigFileName\"=\"MyApp.json\"\r\n";
        {
            CORE::CFileAccess out( path, "wb" );
            out.Write( original.C_String(), original.Length(), 1 );
        }
        ASSERT_TRUE( CORE::CIniParser::UpdateValueInIniFile( path, "configstore", "configlocation", "$TEMPDIR$/Moved" ) );
        ASSERT_TRUE( original.ReplaceSubstr( "$APPCONFIGDIR$/MyApp", "$TEMPDIR$/Moved" ) == ReadText( path ) );

        // a key that is not there goes at the end of its section, a section that is not there at the end of the file
        ASSERT_TRUE( CORE::CIniParser::UpdateValueInIniFile( path, "ConfigStore", "DefaultConfigFile", "$MODULEDIR$/MyApp_DefaultConfig.json" ) );
        ASSERT_TRUE( CORE::CIniParser::UpdateValueInIniFile( path, "Other", "key", "value" ) );
        const CORE::CString text = ReadText( path );
        ASSERT_TRUE( text.EndsWith( "\"ConfigFileName\"=\"MyApp.json\"\r\n\"DefaultConfigFile\"=\"$MODULEDIR$/MyApp_DefaultConfig.json\"\r\n\r\n[Other]\r\n\"key\"=\"value\"\r\n" ) );
        ASSERT_TRUE( text.StartsWith( "; the plugins and where the config is\r\n[Bootstrap\\PluginGroup]\r\n" ) && text.HasSubstr( "; kept per user", true ) > 0 );

        // and the parser reads what was written
        CORE::CIniParser parser;
        ASSERT_TRUE( parser.LoadFrom( text ) );
        CORE::DeleteFile( path );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConfigStore: The bootstrap config names the config location: the user gets a copy of the default config, a broken one is made again, it can be moved" )
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
        GUCEF_TESTFW_SKIP( "the JSON codec plugin is not available" );
    try
    {
        CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
        const CORE::CString previousBootstrap = configStore.GetBootstrapConfigFile();
        const CORE::CString::StringVector previousConfigFiles = configStore.GetConfigFiles();

        const CORE::CString dir = CORE::CombinePath( CORE::RelativePath( "$TEMPDIR$" ), TESTFW::UniqueTestName( "gucefCORE_TestApp_ManagedConfig" ) );
        CORE::DeleteDir( dir, true );
        ASSERT_TRUE( CORE::CreateDirs( dir ) );
        const CORE::CString bootstrap = CORE::CombinePath( dir, "MyApp_bootstrap.ini" );
        const CORE::CString defaultConfig = CORE::CombinePath( dir, "MyApp_DefaultConfig.json" );
        const CORE::CString userDir = CORE::CombinePath( dir, "user" );
        const CORE::CString userConfig = CORE::CombinePath( userDir, "MyApp.json" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( bootstrap, "; where the config of the user is\n[ConfigStore]\n\"ConfigLocation\"=\"" + userDir + "\"\n\"ConfigFileName\"=\"MyApp.json\"\n\"DefaultConfigFile\"=\"" + defaultConfig + "\"\n" ) );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( defaultConfig, "{ \"Managed\": { \"value\": \"default\" } }" ) );

        CReadingConfigurable reader;
        configStore.SetBootstrapConfigFile( bootstrap );
        configStore.SetConfigFiles( CORE::CString::StringVector() );

        // the first start: the default config is copied to the user
        ASSERT_TRUE( configStore.LoadConfig() );
        ASSERT_TRUE( configStore.IsConfigLocationManaged() && userDir == configStore.GetConfigLocation() );
        ASSERT_TRUE( CORE::FileExists( userConfig ) && "default" == reader.value && userConfig == configStore.GetConsolidatedConfigFile() );

        // what the user has is used from then on
        ASSERT_TRUE( CORE::WriteStringAsTextFile( userConfig, "{ \"Managed\": { \"value\": \"mine\" } }" ) );
        ASSERT_TRUE( configStore.LoadConfig() && "mine" == reader.value );

        // a config that cannot be read is put aside and made again
        ASSERT_TRUE( CORE::WriteStringAsTextFile( userConfig, "{ \"Managed\": { broken" ) );
        ASSERT_TRUE( configStore.LoadConfig() && "default" == reader.value );
        ASSERT_TRUE( CORE::FileExists( userConfig + ".corrupt" ) && "{ \"Managed\": { broken" == ReadText( userConfig + ".corrupt" ) );

        // moved: the config goes along, the bootstrap config remembers the new place (with a variable for a directory that has one)
        ASSERT_TRUE( CORE::WriteStringAsTextFile( userConfig, "{ \"Managed\": { \"value\": \"moving\" } }" ) );
        const CORE::CString movedDir = CORE::CombinePath( dir, "moved" );
        ASSERT_TRUE( configStore.SetConfigLocation( movedDir ) );
        const CORE::CString movedConfig = CORE::CombinePath( movedDir, "MyApp.json" );
        ASSERT_TRUE( CORE::FileExists( movedConfig ) && movedConfig == configStore.GetConsolidatedConfigFile() );
        const CORE::CString stored = CORE::UnresolveVars( movedDir.ReplaceChar( '\\', '/' ) );
        ASSERT_TRUE( stored.StartsWith( "$TEMPDIR$" ) && stored == configStore.GetConfigLocation() );
        const CORE::CString bootstrapText = ReadText( bootstrap );
        ASSERT_TRUE( bootstrapText.StartsWith( "; where the config of the user is\n" ) && bootstrapText.HasSubstr( "\"ConfigLocation\"=\"" + stored + "\"", true ) > 0 );
        ASSERT_TRUE( configStore.LoadConfig() && "moving" == reader.value && movedDir.ReplaceChar( '\\', '/' ) == configStore.GetResolvedConfigLocation().ReplaceChar( '\\', '/' ) );

        // an application that names its config file itself is not managed
        configStore.SetBootstrapConfigFile( previousBootstrap );
        configStore.SetConfigFiles( previousConfigFiles );
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConfigStore: A fault tolerant config scope makes the configurables of it optional, ends on every way out of the block and nests with the innermost counting" )
    try
    {
        CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
        ASSERT_TRUE( ConfigAppliesAsAWhole( configStore ) );

        // no scope: a configurable that cannot load its config fails the config
        {
            CFailingConfigurable plain;
            ASSERT_FALSE( ConfigAppliesAsAWhole( configStore ) );
        }
        ASSERT_TRUE( ConfigAppliesAsAWhole( configStore ) );

        // in a scope the configurable is optional, after the block the scope is gone
        {
            CORE::CFaultTolerantConfigScope faultTolerantScope;
            CFailingConfigurable optional;
            ASSERT_TRUE( ConfigAppliesAsAWhole( configStore ) );
        }
        {
            CFailingConfigurable afterScope;
            ASSERT_FALSE( ConfigAppliesAsAWhole( configStore ) );
        }

        // the scope ends when the block is left by a return
        ASSERT_FALSE( FaultTolerantEarlyReturn( configStore, true ) );
        ASSERT_TRUE( FaultTolerantEarlyReturn( configStore, false ) );
        {
            CFailingConfigurable afterReturn;
            ASSERT_FALSE( ConfigAppliesAsAWhole( configStore ) );
        }

        // and when an exception unwinds it
        bool caught = false;
        try
        {
            FaultTolerantThrowing( configStore );
        }
        catch ( int )
        {
            caught = true;
        }
        ASSERT_TRUE( caught );
        {
            CFailingConfigurable afterThrow;
            ASSERT_FALSE( ConfigAppliesAsAWhole( configStore ) );
        }

        // nesting: the innermost counts, the outer one is back in effect when the inner one ends
        {
            CORE::CFaultTolerantConfigScope outerScope( configStore );
            CFailingConfigurable inOuter;
            ASSERT_TRUE( ConfigAppliesAsAWhole( configStore ) );
            {
                CORE::CFaultTolerantConfigScope innerScope( configStore, false );
                {
                    CFailingConfigurable inInner;
                    ASSERT_FALSE( ConfigAppliesAsAWhole( configStore ) );
                }
                ASSERT_TRUE( ConfigAppliesAsAWhole( configStore ) );
                {
                    CORE::CFaultTolerantConfigScope innermostScope;
                    CFailingConfigurable inInnermost;
                    ASSERT_TRUE( ConfigAppliesAsAWhole( configStore ) );
                }
            }
            {
                CFailingConfigurable backInOuter;
                ASSERT_TRUE( ConfigAppliesAsAWhole( configStore ) );
            }
        }
        {
            CFailingConfigurable afterAll;
            ASSERT_FALSE( ConfigAppliesAsAWhole( configStore ) );
        }
        ASSERT_TRUE( ConfigAppliesAsAWhole( configStore ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // Loading a plugin opens a config scope, and a plugin may make configurables, under the lock of the plugin control,
    // while loading the config calls every configurable, the plugin control included, which takes that lock. With
    // the config store's lock taken for the scope and the registrations the two threads deadlocked
    GUCEF_TESTFW_TESTCASE( "ConfigStore: A plugin loaded on one thread while another thread loads the config: no deadlock" )
    try
    {
        SConfigLockOrderCase* lockCase = GUCEF_NEW SConfigLockOrderCase();
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "TestConfigStoreLockOrder" );
        TPluginSideTaskPtr pluginSide( GUCEF_NEW CPluginSideTask( *lockCase ) );
        TConfigSideTaskPtr configSide( GUCEF_NEW CConfigSideTask( *lockCase ) );
        ASSERT_FALSE( pool->StartTaskWithConsumer( pluginSide ).HasNoFuture() );
        ASSERT_FALSE( pool->StartTaskWithConsumer( configSide ).HasNoFuture() );

        // both tasks have to keep making progress for the run time
        const UInt64 startTicks = MT::PrecisionTickCount();
        UInt64 pluginProgressTicks = startTicks;
        UInt64 configProgressTicks = startTicks;
        UInt32 lastPluginRounds = 0;
        UInt32 lastConfigRounds = 0;
        bool stalled = false;
        while ( LockOrderElapsedMs( startTicks ) < LockOrderRunTimeInMs )
        {
            MT::ThreadDelay( 10 );
            const UInt64 nowTicks = MT::PrecisionTickCount();
            if ( lockCase->pluginRounds.Load() != lastPluginRounds )
            {
                lastPluginRounds = lockCase->pluginRounds.Load();
                pluginProgressTicks = nowTicks;
            }
            if ( lockCase->configRounds.Load() != lastConfigRounds )
            {
                lastConfigRounds = lockCase->configRounds.Load();
                configProgressTicks = nowTicks;
            }
            if ( LockOrderElapsedMs( pluginProgressTicks ) > LockOrderStallTimeInMs || LockOrderElapsedMs( configProgressTicks ) > LockOrderStallTimeInMs )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ConfigStore lock order: a task made no progress for " << LockOrderStallTimeInMs
                    << " ms, plugin side rounds " << lastPluginRounds << ", config side rounds " << lastConfigRounds );
                stalled = true;
                break;
            }
        }

        lockCase->stop.Store( true, MT::MEMORDER_RELEASE );
        const bool pluginSideEnded = pluginSide->WaitForTaskToFinish( LockOrderTaskEndTimeInMs );
        const bool configSideEnded = configSide->WaitForTaskToFinish( LockOrderTaskEndTimeInMs );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ConfigStore lock order: plugin side rounds " << lockCase->pluginRounds.Load()
            << ", config side rounds " << lockCase->configRounds.Load() );

        ASSERT_FALSE( stalled );
        ASSERT_TRUE( pluginSideEnded );
        ASSERT_TRUE( configSideEnded );
        ASSERT_TRUE( lockCase->pluginRounds.Load() > 0 );
        ASSERT_TRUE( lockCase->configRounds.Load() > 0 );
        ASSERT_TRUE( 0 == lockCase->exceptions.Load() );

        if ( pluginSideEnded && configSideEnded )
            GUCEF_DELETE lockCase;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConfigStore: A configurable destroyed on one thread while a load on another thread calls it: UnregisterFromConfigStore() in its destructor waits for that call" )
    try
    {
        // the case and the configurable stay when a thread did not end: it may still use them
        SConfigInUseCase* inUseCase = GUCEF_NEW SConfigInUseCase();
        inUseCase->toUnregister = GUCEF_NEW CHoldingConfigurable( inUseCase );

        MT::SThreadData* loader = StartConfigThread( &ConfigLoadThreadMain, inUseCase );
        ASSERT_TRUE( GUCEF_NULL != loader );
        ASSERT_TRUE( ConfigWaitFor( inUseCase->inLoad, 10000 ) );

        MT::SThreadData* destroyer = StartConfigThread( &ConfigDestroyThreadMain, inUseCase );
        ASSERT_TRUE( GUCEF_NULL != destroyer );
        MT::ThreadDelay( 200 );
        ASSERT_FALSE( inUseCase->unregistered.Load() );

        inUseCase->release.Store( true, MT::MEMORDER_RELEASE );
        ASSERT_TRUE( ConfigWaitFor( inUseCase->unregistered, 10000 ) );
        ASSERT_TRUE( ConfigWaitFor( inUseCase->loadDone, 10000 ) );
        const bool loaderEnded = EndConfigThread( loader );
        const bool destroyerEnded = EndConfigThread( destroyer );
        ASSERT_TRUE( loaderEnded && destroyerEnded );
        ASSERT_TRUE( inUseCase->intactAfterRelease.Load() );

        if ( loaderEnded && destroyerEnded )
            GUCEF_DELETE inUseCase;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConfigStore: A configurable unregistered during a load that has not called it yet is not called by that load" )
    try
    {
        SConfigInUseCase* inUseCase = GUCEF_NEW SConfigInUseCase();
        CHoldingConfigurable* holding = GUCEF_NEW CHoldingConfigurable( inUseCase );
        CHoldingConfigurable* leaving = GUCEF_NEW CHoldingConfigurable( GUCEF_NULL );

        MT::SThreadData* loader = StartConfigThread( &ConfigLoadThreadMain, inUseCase );
        ASSERT_TRUE( GUCEF_NULL != loader );
        ASSERT_TRUE( ConfigWaitFor( inUseCase->inLoad, 10000 ) );

        // the load waits in the other configurable: this one is not in use, it goes at once
        const UInt32 loadsBefore = leaving->loads.Load();
        const UInt64 startTicks = MT::PrecisionTickCount();
        leaving->LeaveConfigStore();
        ASSERT_TRUE( LockOrderElapsedMs( startTicks ) < 1000 );
        ASSERT_FALSE( inUseCase->loadDone.Load() );

        inUseCase->release.Store( true, MT::MEMORDER_RELEASE );
        ASSERT_TRUE( ConfigWaitFor( inUseCase->loadDone, 10000 ) );
        const bool loaderEnded = EndConfigThread( loader );
        ASSERT_TRUE( loaderEnded );
        ASSERT_TRUE( loadsBefore == leaving->loads.Load() );
        ASSERT_TRUE( 1 == holding->loads.Load() );

        if ( loaderEnded )
        {
            GUCEF_DELETE leaving;
            GUCEF_DELETE holding;
            GUCEF_DELETE inUseCase;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
