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

#include <stdlib.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CCONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CCONFIGSTORE_H
#endif /* GUCEF_CORE_CCONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CIPLUGINLOADLOGIC_H
#include "gucefCORE_CIPluginLoadLogic.h"
#define GUCEF_CORE_CIPLUGINLOADLOGIC_H
#endif /* GUCEF_CORE_CIPLUGINLOADLOGIC_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPluginUnload.h"

using namespace GUCEF;

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

namespace {

const char* TestPluginGroup = "testplugin_tests";

bool
LoadTestPlugin( void )
{
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = "$MODULEDIR$/gucefCORE_TestPlugin_d";
    const CORE::CString second = "$MODULEDIR$/gucefCORE_TestPlugin";
    #else
    const CORE::CString first = "$MODULEDIR$/gucefCORE_TestPlugin";
    const CORE::CString second = "$MODULEDIR$/gucefCORE_TestPlugin_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, TestPluginGroup, true ) || pluginControl.AddPluginFromDir( second, TestPluginGroup, true );
}

/** How often the plugin was asked to load and to unload, as it counted */
int
Calls( const char* variable )
{
    const char* value = getenv( variable );
    return GUCEF_NULL != value ? atoi( value ) : 0;
}

/** The plugin is still known to the group, so it can be loaded again */
bool
IsKnownToGroup( void )
{
    CORE::CPluginControl::TStringSet modules;
    CORE::CCoreGlobal::Instance()->GetPluginControl().GetAvailablePluginsForGroup( TestPluginGroup, modules );
    return 1 == modules.size();
}

void
SetFlag( const char* variable, bool on )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( variable, on ? "1" : "0" );
    #else
    setenv( variable, on ? "1" : "0", 1 );
    #endif
}

void
SetUnloadThrows( bool throws )
{
    SetFlag( "GUCEF_TESTPLUGIN_UNLOAD_THROWS", throws );
}

void
SetLoadThrows( bool throws )
{
    SetFlag( "GUCEF_TESTPLUGIN_LOAD_THROWS", throws );
}

void
SetLoadFails( bool fails )
{
    SetFlag( "GUCEF_TESTPLUGIN_LOAD_FAILS", fails );
}

void
SetLoadAddsFailingConfigurable( bool adds )
{
    SetFlag( "GUCEF_TESTPLUGIN_LOAD_ADDS_FAILING_CONFIGURABLE", adds );
}

/** The plugin does what the tests of this file ask of it, and nothing more */
void
ResetTestPluginFlags( void )
{
    SetUnloadThrows( false );
    SetLoadThrows( false );
    SetLoadFails( false );
    SetLoadAddsFailingConfigurable( false );
}

const char* const FailAllowedGroup = "testplugin_failallowed";

const CORE::CString&
TestPluginFilename( void )
{
    #ifdef GUCEF_DEBUG_MODE
    static const CORE::CString filename = "gucefCORE_TestPlugin_d";
    #else
    static const CORE::CString filename = "gucefCORE_TestPlugin";
    #endif
    return filename;
}

const CORE::CString&
TestPluginAltFilename( void )
{
    #ifdef GUCEF_DEBUG_MODE
    static const CORE::CString filename = "gucefCORE_TestPlugin";
    #else
    static const CORE::CString filename = "gucefCORE_TestPlugin_d";
    #endif
    return filename;
}

/** A plugin as the bootstrap config of an application lists it */
void
AddPluginBlock( CORE::CDataNode& group, const CORE::CString& filename, const CORE::CString& altFilename, bool loadFailAllowed, const CORE::CString& loaderLogic = CORE::CString::Empty )
{
    CORE::CDataNode* block = group.AddChild( "PluginMetaData" );
    block->SetAttribute( "Description", "plugin of the tests" );
    block->SetAttribute( "Path", "$MODULEDIR$" );
    block->SetAttribute( "Filename", filename );
    block->SetAttribute( "AltFilename", altFilename );
    block->SetAttribute( "Type", "GucefGenericPlugin" );
    block->SetAttribute( "LoadFailAllowed", loadFailAllowed ? "true" : "false" );
    if ( !loaderLogic.IsNULLOrEmpty() )
        block->SetAttribute( "LoaderLogicType", loaderLogic );
}

/** The bootstrap config with one group in it, of which the plugins can be added */
CORE::CDataNode*
AddPluginGroup( CORE::CDataNode& root )
{
    CORE::CDataNode* group = root.AddChild( "PluginGroup" );
    group->SetAttribute( "GroupName", FailAllowedGroup );
    group->SetAttribute( "LoadImmediately", "true" );
    return group;
}

/** The plugin of the tests as the one plugin of the group, with the given way it can fail */
void
MakeConfigWithTestPlugin( CORE::CDataNode& root, bool loadFailAllowed, const CORE::CString& loaderLogic = CORE::CString::Empty )
{
    AddPluginBlock( *AddPluginGroup( root ), TestPluginFilename(), TestPluginAltFilename(), loadFailAllowed, loaderLogic );
}

/** The group goes, with what the plugin control knows of its plugins */
void
RemoveFailAllowedGroup( void )
{
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    pluginControl.UnloadPluginGroup( FailAllowedGroup );
    pluginControl.ClearMetaDataFromGroup( FailAllowedGroup );
}

/** Loads and unloads modules as the default logic does, under a name that the test can make unknown to the plugin control again */
class CTestLoadLogic : public CORE::CIPluginLoadLogic
{
    public:

    virtual void* LoadPlugin( const CORE::CString& rootDir, const CORE::CString& moduleName, const CORE::CString& groupName, const CORE::TVersion* pluginVersion = 0 ) GUCEF_VIRTUAL_OVERRIDE
    {
        return CORE::LoadModuleDynamicly( CORE::RelativePath( CORE::CombinePath( rootDir, moduleName ) ) );
    }

    virtual void UnloadPlugin( void* modulePtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::UnloadModuleDynamicly( modulePtr );
    }

    virtual CORE::CString GetLoaderLogicTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return "TestLoadLogic";
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPluginUnloadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.PluginUnload" );

    GUCEF_TESTFW_TESTCASE( "LoadModuleDynamicly: A module that is not there is reported with the reason of the operating system, one that is there loads without a reason" )
    try
    {
        CORE::CString reason;
        void* missing = CORE::LoadModuleDynamicly( "gucefCORE_NoSuchModule.so.99", reason );
        ASSERT_TRUE( GUCEF_NULL == missing );
        ASSERT_FALSE( reason.IsNULLOrEmpty() );
        ASSERT_TRUE( reason.HasSubstr( "gucefCORE_NoSuchModule" ) >= 0 );

        // the variant without a reason keeps working as before
        ASSERT_TRUE( GUCEF_NULL == CORE::LoadModuleDynamicly( "gucefCORE_NoSuchModule.so.99" ) );

        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        reason = "stale";
        void* present = CORE::LoadModuleDynamicly( "libc.so.6", reason );
        ASSERT_TRUE( GUCEF_NULL != present );
        ASSERT_TRUE( reason.IsNULLOrEmpty() );
        CORE::UnloadModuleDynamicly( present );
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !LoadTestPlugin() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Plugin unload TESTS skipped: gucefCORE_TestPlugin is not available" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "PluginUnload: A generic plugin unloads and loads again" )
    try
    {
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        const int loads = Calls( "GUCEF_TESTPLUGIN_LOADS" ), unloads = Calls( "GUCEF_TESTPLUGIN_UNLOADS" );
        ASSERT_TRUE( loads >= 1 );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( TestPluginGroup ) );
        ASSERT_TRUE( unloads + 1 == Calls( "GUCEF_TESTPLUGIN_UNLOADS" ) );
        ASSERT_TRUE( IsKnownToGroup() );
        ASSERT_TRUE( pluginControl.LoadPluginGroup( TestPluginGroup ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginUnload: A plugin whose Unload throws is still unloaded, is not called again, and loads again" )
    try
    {
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        const int loads = Calls( "GUCEF_TESTPLUGIN_LOADS" ), unloads = Calls( "GUCEF_TESTPLUGIN_UNLOADS" );
        SetUnloadThrows( true );
        pluginControl.UnloadPluginGroup( TestPluginGroup );        // reports the broken plugin, it must not take the process down
        SetUnloadThrows( false );
        ASSERT_TRUE( unloads + 1 == Calls( "GUCEF_TESTPLUGIN_UNLOADS" ) );    // asked once, not again by the plugin object

        ASSERT_TRUE( IsKnownToGroup() );
        ASSERT_TRUE( pluginControl.LoadPluginGroup( TestPluginGroup ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( TestPluginGroup ) );
    }
    catch ( ... )
    {
        SetUnloadThrows( false );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginUnload: A plugin whose Load throws is reported as not loaded, and loads once it works" )
    try
    {
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        const int loads = Calls( "GUCEF_TESTPLUGIN_LOADS" );
        SetLoadThrows( true );
        ASSERT_FALSE( pluginControl.LoadPluginGroup( TestPluginGroup ) );
        SetLoadThrows( false );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( IsKnownToGroup() );

        ASSERT_TRUE( pluginControl.LoadPluginGroup( TestPluginGroup ) );
        ASSERT_TRUE( loads + 2 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( TestPluginGroup ) );
    }
    catch ( ... )
    {
        SetLoadThrows( false );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginLoadFailAllowed: A plugin that is not there does not fail the load of the config when it may fail, and does when it may not" )
    try
    {
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

        CORE::CDataNode allowed( "Config" );
        AddPluginBlock( *AddPluginGroup( allowed ), "gucefCORE_NoSuchPlugin_zz", "gucefCORE_NoSuchPlugin_zz_alt", true );
        ASSERT_TRUE( pluginControl.LoadConfig( allowed ) );
        RemoveFailAllowedGroup();

        CORE::CDataNode strict( "Config" );
        AddPluginBlock( *AddPluginGroup( strict ), "gucefCORE_NoSuchPlugin_zz", "gucefCORE_NoSuchPlugin_zz_alt", false );
        ASSERT_FALSE( pluginControl.LoadConfig( strict ) );
        RemoveFailAllowedGroup();
    }
    catch ( ... )
    {
        RemoveFailAllowedGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginLoadFailAllowed: A plugin that does not register, or throws when it does, does not fail the load of the config when it may fail, and does when it may not" )
    try
    {
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

        // Load() says it failed: the plugin is loaded as a module and refuses to register
        const int loads = Calls( "GUCEF_TESTPLUGIN_LOADS" );
        SetLoadFails( true );
        CORE::CDataNode refusesAllowed( "Config" );
        MakeConfigWithTestPlugin( refusesAllowed, true );
        ASSERT_TRUE( pluginControl.LoadConfig( refusesAllowed ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        RemoveFailAllowedGroup();

        CORE::CDataNode refusesStrict( "Config" );
        MakeConfigWithTestPlugin( refusesStrict, false );
        ASSERT_FALSE( pluginControl.LoadConfig( refusesStrict ) );
        ASSERT_TRUE( loads + 2 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        RemoveFailAllowedGroup();
        SetLoadFails( false );

        // Load() throws
        SetLoadThrows( true );
        CORE::CDataNode throwsAllowed( "Config" );
        MakeConfigWithTestPlugin( throwsAllowed, true );
        ASSERT_TRUE( pluginControl.LoadConfig( throwsAllowed ) );
        RemoveFailAllowedGroup();

        CORE::CDataNode throwsStrict( "Config" );
        MakeConfigWithTestPlugin( throwsStrict, false );
        ASSERT_FALSE( pluginControl.LoadConfig( throwsStrict ) );
        RemoveFailAllowedGroup();
        SetLoadThrows( false );

        // and the same plugin works when it can
        const int unloads = Calls( "GUCEF_TESTPLUGIN_UNLOADS" );
        CORE::CDataNode works( "Config" );
        MakeConfigWithTestPlugin( works, true );
        ASSERT_TRUE( pluginControl.LoadConfig( works ) );
        RemoveFailAllowedGroup();
        ASSERT_TRUE( unloads + 1 == Calls( "GUCEF_TESTPLUGIN_UNLOADS" ) );
    }
    catch ( ... )
    {
        ResetTestPluginFlags();
        RemoveFailAllowedGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginLoadFailAllowed: Loading a group, or all plugins of a type, or everything, goes on past a plugin that may fail and reports success" )
    try
    {
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        const CORE::CString genericType = "GucefGenericPlugin";

        // the plugins are known, none is loaded yet: the group does not load immediately
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* group = AddPluginGroup( config );
        group->SetAttribute( "LoadImmediately", "false" );
        AddPluginBlock( *group, "gucefCORE_NoSuchPlugin_zz", "gucefCORE_NoSuchPlugin_zz_alt", true );
        AddPluginBlock( *group, TestPluginFilename(), TestPluginAltFilename(), true );
        int loads = Calls( "GUCEF_TESTPLUGIN_LOADS" );
        ASSERT_TRUE( pluginControl.LoadConfig( config ) );
        ASSERT_TRUE( loads == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );

        // loading the group, whatever order the plugins are in: the one that is there is loaded, the group is loaded
        ASSERT_TRUE( pluginControl.LoadPluginGroup( FailAllowedGroup ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( FailAllowedGroup ) );

        loads = Calls( "GUCEF_TESTPLUGIN_LOADS" );
        ASSERT_TRUE( pluginControl.LoadAllPluginsOfTypeInGroup( genericType, FailAllowedGroup ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( FailAllowedGroup ) );

        loads = Calls( "GUCEF_TESTPLUGIN_LOADS" );
        ASSERT_TRUE( pluginControl.LoadAllPluginsOfType( genericType ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( FailAllowedGroup ) );

        loads = Calls( "GUCEF_TESTPLUGIN_LOADS" );
        ASSERT_TRUE( pluginControl.LoadAll( true, false ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( FailAllowedGroup ) );

        // with a plugin that registers late and refuses to, the steps that follow loading report success as well
        SetLoadFails( true );
        ASSERT_TRUE( pluginControl.LoadAll( false, false ) );
        ASSERT_TRUE( pluginControl.RegisterAll( false ) );
        SetLoadFails( false );
        RemoveFailAllowedGroup();

        // a plugin that may not fail does fail the group, and the one after it is loaded all the same
        CORE::CDataNode strictConfig( "Config" );
        CORE::CDataNode* strictGroup = AddPluginGroup( strictConfig );
        strictGroup->SetAttribute( "LoadImmediately", "false" );
        AddPluginBlock( *strictGroup, "gucefCORE_NoSuchPlugin_zz", "gucefCORE_NoSuchPlugin_zz_alt", false );
        AddPluginBlock( *strictGroup, TestPluginFilename(), TestPluginAltFilename(), true );
        ASSERT_TRUE( pluginControl.LoadConfig( strictConfig ) );
        loads = Calls( "GUCEF_TESTPLUGIN_LOADS" );
        ASSERT_FALSE( pluginControl.LoadPluginGroup( FailAllowedGroup ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        RemoveFailAllowedGroup();
    }
    catch ( ... )
    {
        ResetTestPluginFlags();
        RemoveFailAllowedGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginLoadFailAllowed: A plugin that cannot be unloaded does not fail the unload when it may fail, and does when it may not" )
    try
    {
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        CTestLoadLogic loadLogic;
        pluginControl.RegisterPluginLoadLogic( &loadLogic );

        // the plugin is loaded by a logic that is not there when it is time to unload: it cannot be unloaded
        CORE::CDataNode allowed( "Config" );
        MakeConfigWithTestPlugin( allowed, true, "TestLoadLogic" );
        const int loads = Calls( "GUCEF_TESTPLUGIN_LOADS" ), unloads = Calls( "GUCEF_TESTPLUGIN_UNLOADS" );
        ASSERT_TRUE( pluginControl.LoadConfig( allowed ) );
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        pluginControl.UnregisterPluginLoadLogic( "TestLoadLogic" );
        ASSERT_TRUE( pluginControl.UnloadPluginGroup( FailAllowedGroup ) );
        ASSERT_TRUE( unloads + 1 == Calls( "GUCEF_TESTPLUGIN_UNLOADS" ) );
        pluginControl.ClearMetaDataFromGroup( FailAllowedGroup );

        pluginControl.RegisterPluginLoadLogic( &loadLogic );
        CORE::CDataNode strict( "Config" );
        MakeConfigWithTestPlugin( strict, false, "TestLoadLogic" );
        ASSERT_TRUE( pluginControl.LoadConfig( strict ) );
        ASSERT_TRUE( loads + 2 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        pluginControl.UnregisterPluginLoadLogic( "TestLoadLogic" );
        ASSERT_FALSE( pluginControl.UnloadPluginGroup( FailAllowedGroup ) );
        pluginControl.ClearMetaDataFromGroup( FailAllowedGroup );
    }
    catch ( ... )
    {
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnregisterPluginLoadLogic( "TestLoadLogic" );
        RemoveFailAllowedGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginLoadFailAllowed: The config of the application loads with a plugin that may fail, also when it brings a global that cannot load its config" )
    try
    {
        CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
        const CORE::CString previousBootstrap = configStore.GetBootstrapConfigFile();
        const CORE::CString::StringVector previousConfigFiles = configStore.GetConfigFiles();
        const CORE::CString path = CORE::CombinePath( CORE::RelativePath( "$TEMPDIR$" ), "gucefCORE_TestApp_FailAllowed.ini" );
        configStore.SetConfigFiles( CORE::CString::StringVector() );

        // the plugin starts a global, as the plugins that start the module they are for do: its config fails
        SetLoadAddsFailingConfigurable( true );
        const CORE::CString ini = "[Bootstrap\\PluginGroup]\n"
                                  "\"GroupName\"=\"" + CORE::CString( FailAllowedGroup ) + "\"\n"
                                  "\"LoadImmediately\"=\"true\"\n"
                                  "\n"
                                  "[Bootstrap\\PluginGroup\\PluginMetaData]\n"
                                  "\"Description\"=\"plugin of the tests\"\n"
                                  "\"Path\"=\"$MODULEDIR$\"\n"
                                  "\"Filename\"=\"" + TestPluginFilename() + "\"\n"
                                  "\"AltFilename\"=\"" + TestPluginAltFilename() + "\"\n"
                                  "\"Type\"=\"GucefGenericPlugin\"\n";

        ASSERT_TRUE( CORE::WriteStringAsTextFile( path, ini + "\"LoadFailAllowed\"=\"true\"\n" ) );
        configStore.SetBootstrapConfigFile( path );
        const bool allowedLoaded = configStore.LoadConfig();
        RemoveFailAllowedGroup();

        // the same plugin that may not fail is a plugin that does
        ASSERT_TRUE( CORE::WriteStringAsTextFile( path, ini + "\"LoadFailAllowed\"=\"false\"\n" ) );
        const bool strictLoaded = configStore.LoadConfig();
        RemoveFailAllowedGroup();

        SetLoadAddsFailingConfigurable( false );
        configStore.SetBootstrapConfigFile( previousBootstrap );
        configStore.SetConfigFiles( previousConfigFiles );
        CORE::DeleteFile( path );
        ASSERT_TRUE( allowedLoaded );
        ASSERT_FALSE( strictLoaded );
    }
    catch ( ... )
    {
        ResetTestPluginFlags();
        RemoveFailAllowedGroup();
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
