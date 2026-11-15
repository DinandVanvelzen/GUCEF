/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <stdlib.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPluginLoadOrder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CORE::CPluginControl::TStringVector TStringVector;

/** Enough entries that an order by address, or by name, does not match the order of the config by chance */
const UInt32 ManyEntries = 40;

CORE::CPluginControl&
PluginControl( void )
{
    return CORE::CCoreGlobal::Instance()->GetPluginControl();
}

/** The name of entry i of a test: counting down, so that the order by name is the reverse of the order of the config */
CORE::CString
NameOfEntry( const char* prefix, UInt32 i )
{
    const UInt32 number = 1000 - i;
    return CORE::CString( prefix ) + CORE::UInt32ToString( number );
}

/** A group as a bootstrap config lists it, without its plugins loaded when the config is loaded */
CORE::CDataNode*
AddGroup( CORE::CDataNode& root, const CORE::CString& groupName, UInt32 groupPriority = 0 )
{
    CORE::CDataNode* group = root.AddChild( "PluginGroup" );
    group->SetAttribute( "GroupName", groupName );
    group->SetAttribute( "LoadImmediately", "false" );
    if ( groupPriority > 0 )
        group->SetAttribute( "GroupPriority", groupPriority );
    return group;
}

/** A plugin of a module that is not there; it may fail */
void
AddPlugin( CORE::CDataNode& group, const CORE::CString& filename, UInt32 priorityInGroup = 0 )
{
    CORE::CDataNode* block = group.AddChild( "PluginMetaData" );
    block->SetAttribute( "Description", "plugin of the load order tests" );
    block->SetAttribute( "Path", "$MODULEDIR$" );
    block->SetAttribute( "Filename", filename );
    block->SetAttribute( "Type", "GucefGenericPlugin" );
    block->SetAttribute( "LoadFailAllowed", "true" );
    if ( priorityInGroup > 0 )
        block->SetAttribute( "PriorityInGroup", priorityInGroup );
}

/** The groups of the plugin control in their load order, those with names that start with the prefix */
TStringVector
GroupsInLoadOrder( const char* prefix )
{
    TStringVector all;
    PluginControl().GetPluginGroupsInLoadOrder( all );
    TStringVector groups;
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
        if ( all[ i ].HasSubstr( prefix, true ) == 0 )
            groups.push_back( all[ i ] );
    return groups;
}

TStringVector
PluginsInLoadOrder( const CORE::CString& groupName )
{
    TStringVector plugins;
    PluginControl().GetPluginsInLoadOrder( groupName, plugins );
    return plugins;
}

void
RemoveGroups( const TStringVector& groupNames )
{
    for ( UInt32 i=0; i<(UInt32)groupNames.size(); ++i )
    {
        PluginControl().UnloadPluginGroup( groupNames[ i ] );
        PluginControl().ClearMetaDataFromGroup( groupNames[ i ] );
    }
}

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

int
Calls( const char* variable )
{
    const char* value = getenv( variable );
    return GUCEF_NULL != value ? atoi( value ) : 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPluginLoadOrderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.PluginLoadOrder" );

    GUCEF_TESTFW_TESTCASE( "PluginLoadOrder: the groups of a config load from top to bottom" )
    TStringVector expected;
    try
    {
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* bootstrap = config.AddChild( "Bootstrap" );
        for ( UInt32 i=0; i<ManyEntries; ++i )
        {
            expected.push_back( NameOfEntry( "loadorder_groups_", i ) );
            AddPlugin( *AddGroup( *bootstrap, expected.back() ), NameOfEntry( "loadorder_missing_module_", i ) );
        }
        ASSERT_TRUE( PluginControl().LoadConfig( config ) );
        ASSERT_TRUE( expected == GroupsInLoadOrder( "loadorder_groups_" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    RemoveGroups( expected );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginLoadOrder: the plugins of a group load from top to bottom" )
    try
    {
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* group = AddGroup( config, "loadorder_plugins" );
        TStringVector plugins;
        for ( UInt32 i=0; i<ManyEntries; ++i )
        {
            plugins.push_back( NameOfEntry( "loadorder_missing_module_", i ) );
            AddPlugin( *group, plugins.back() );
        }
        ASSERT_TRUE( PluginControl().LoadConfig( config ) );
        ASSERT_TRUE( plugins == PluginsInLoadOrder( "loadorder_plugins" ) );

        // a plugin added afterwards comes after what the group has
        CORE::CPluginMetaData late;
        late.SetModuleFilename( "loadorder_late_module" );
        late.SetPluginType( "GucefGenericPlugin" );
        late.SetLoadFailAllowed( true );
        ASSERT_TRUE( PluginControl().AddPluginMetaData( late, "loadorder_plugins", false, false ) );
        plugins.push_back( "loadorder_late_module" );
        ASSERT_TRUE( plugins == PluginsInLoadOrder( "loadorder_plugins" ) );

        // as does a block of the group in a later config, in its own order
        CORE::CDataNode more( "Config" );
        CORE::CDataNode* moreGroup = AddGroup( more, "loadorder_plugins" );
        AddPlugin( *moreGroup, "loadorder_more_b" );
        AddPlugin( *moreGroup, "loadorder_more_a" );
        ASSERT_TRUE( PluginControl().LoadConfig( more ) );
        plugins.push_back( "loadorder_more_b" );
        plugins.push_back( "loadorder_more_a" );
        ASSERT_TRUE( plugins == PluginsInLoadOrder( "loadorder_plugins" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    PluginControl().ClearMetaDataFromGroup( "loadorder_plugins" );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginLoadOrder: GroupPriority and PriorityInGroup place a group and a plugin; lower first, the implied place is the position in the config, equal places keep the order of the config" )
    TStringVector groups;
    try
    {
        // groups: a at 1, b listed at 10, c at 3, d listed at 2 (a group is made for the plugins it lists)
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* a = AddGroup( config, "loadorder_priority_a" );
        AddPlugin( *AddGroup( config, "loadorder_priority_b", 10 ), "loadorder_missing_b" );
        AddPlugin( *AddGroup( config, "loadorder_priority_c" ), "loadorder_missing_c" );
        AddPlugin( *AddGroup( config, "loadorder_priority_d", 2 ), "loadorder_missing_d" );
        groups.push_back( "loadorder_priority_a" );
        groups.push_back( "loadorder_priority_b" );
        groups.push_back( "loadorder_priority_c" );
        groups.push_back( "loadorder_priority_d" );

        // plugins of a: p1 at 1, p2 listed at 5, p3 at 3, p4 listed at 1 (the place of p1, after it)
        AddPlugin( *a, "loadorder_p1" );
        AddPlugin( *a, "loadorder_p2", 5 );
        AddPlugin( *a, "loadorder_p3" );
        AddPlugin( *a, "loadorder_p4", 1 );
        ASSERT_TRUE( PluginControl().LoadConfig( config ) );

        TStringVector expectedGroups;
        expectedGroups.push_back( "loadorder_priority_a" );
        expectedGroups.push_back( "loadorder_priority_d" );
        expectedGroups.push_back( "loadorder_priority_c" );
        expectedGroups.push_back( "loadorder_priority_b" );
        ASSERT_TRUE( expectedGroups == GroupsInLoadOrder( "loadorder_priority_" ) );

        TStringVector expectedPlugins;
        expectedPlugins.push_back( "loadorder_p1" );
        expectedPlugins.push_back( "loadorder_p4" );
        expectedPlugins.push_back( "loadorder_p3" );
        expectedPlugins.push_back( "loadorder_p2" );
        ASSERT_TRUE( expectedPlugins == PluginsInLoadOrder( "loadorder_priority_a" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    RemoveGroups( groups );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PluginLoadOrder: a plugin keeps its place in the group when it is loaded, unloaded and loaded again" )
    try
    {
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* group = AddGroup( config, "loadorder_loaded" );
        AddPlugin( *group, "loadorder_before" );
        CORE::CDataNode* block = group->AddChild( "PluginMetaData" );
        block->SetAttribute( "Description", "plugin of the tests" );
        block->SetAttribute( "Path", "$MODULEDIR$" );
        block->SetAttribute( "Filename", TestPluginFilename() );
        block->SetAttribute( "AltFilename", TestPluginAltFilename() );
        block->SetAttribute( "Type", "GucefGenericPlugin" );
        AddPlugin( *group, "loadorder_after" );
        ASSERT_TRUE( PluginControl().LoadConfig( config ) );

        TStringVector expected;
        expected.push_back( "loadorder_before" );
        expected.push_back( TestPluginFilename() );
        expected.push_back( "loadorder_after" );
        ASSERT_TRUE( expected == PluginsInLoadOrder( "loadorder_loaded" ) );

        const int loads = Calls( "GUCEF_TESTPLUGIN_LOADS" ), unloads = Calls( "GUCEF_TESTPLUGIN_UNLOADS" );
        PluginControl().LoadPluginGroup( "loadorder_loaded" );   // the two modules that are not there fail, as they may
        ASSERT_TRUE( loads + 1 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( expected == PluginsInLoadOrder( "loadorder_loaded" ) );
        ASSERT_TRUE( PluginControl().UnloadPluginGroup( "loadorder_loaded" ) );
        ASSERT_TRUE( unloads + 1 == Calls( "GUCEF_TESTPLUGIN_UNLOADS" ) );
        ASSERT_TRUE( expected == PluginsInLoadOrder( "loadorder_loaded" ) );
        PluginControl().LoadPluginGroup( "loadorder_loaded" );
        ASSERT_TRUE( loads + 2 == Calls( "GUCEF_TESTPLUGIN_LOADS" ) );
        ASSERT_TRUE( expected == PluginsInLoadOrder( "loadorder_loaded" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    PluginControl().UnloadPluginGroup( "loadorder_loaded" );
    PluginControl().ClearMetaDataFromGroup( "loadorder_loaded" );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
