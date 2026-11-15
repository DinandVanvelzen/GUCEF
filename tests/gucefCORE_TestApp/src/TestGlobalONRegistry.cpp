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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestGlobalONRegistry.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

class CTestRegisteredItem
{
    public:

    explicit CTestRegisteredItem( int value ) : m_value( value ) {}
    int m_value;
};

typedef CORE::CTGlobalONRegistry< CTestRegisteredItem, MT::CMutex >     TTestRegistry;

CORE::CTSharedPtr< CTestRegisteredItem, MT::CMutex >
NewItem( int value )
{
    return CORE::CTSharedPtr< CTestRegisteredItem, MT::CMutex >( GUCEF_NEW CTestRegisteredItem( value ) );
}

/** Counts the registration events of a registry and remembers the names they carry */
class CTestRegistryObserver : public CORE::CObserver
{
    public:

    CTestRegistryObserver( void ) : CORE::CObserver(), m_registered( 0 ), m_unregistered( 0 ), m_lastRegistered(), m_lastUnregistered() {}

    virtual ~CTestRegistryObserver() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == eventData )
            return;
        if ( eventId == CORE::CTONRegistryEvents::ItemRegisteredEvent )
        {
            ++m_registered;
            m_lastRegistered = static_cast< CORE::TCloneableString* >( eventData )->GetData();
        }
        else
        if ( eventId == CORE::CTONRegistryEvents::ItemUnregisteredEvent )
        {
            ++m_unregistered;
            m_lastUnregistered = static_cast< CORE::TCloneableString* >( eventData )->GetData();
        }
    }

    UInt32 m_registered;
    UInt32 m_unregistered;
    CORE::CString m_lastRegistered;
    CORE::CString m_lastUnregistered;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGlobalONRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GlobalONRegistry" );

    GUCEF_TESTFW_TESTCASE( "GlobalONRegistry: registrations and unregistrations each give their own event, also TryUnregister" )
    try
    {
        TTestRegistry registry;
        CTestRegistryObserver observer;
        observer.SubscribeTo( &registry.AsNotifier(), CORE::CTONRegistryEvents::ItemRegisteredEvent );
        observer.SubscribeTo( &registry.AsNotifier(), CORE::CTONRegistryEvents::ItemUnregisteredEvent );

        registry.Register( "a", NewItem( 1 ) );
        ASSERT_TRUE( registry.TryRegister( "b", NewItem( 2 ) ) );
        ASSERT_FALSE( registry.TryRegister( "b", NewItem( 3 ) ) );       // taken: no event
        registry.RegisterBuiltIn( "c", NewItem( 4 ) );
        ASSERT_TRUE( registry.TryRegisterBuiltIn( "d", NewItem( 5 ) ) );
        ASSERT_FALSE( registry.TryRegisterBuiltIn( "d", NewItem( 6 ) ) );
        ASSERT_TRUE( 4 == observer.m_registered && 0 == observer.m_unregistered );
        ASSERT_TRUE( observer.m_lastRegistered == "d" );
        ASSERT_TRUE( 4 == registry.GetCount() );

        // an unregistration is not announced as a registration (it used to be)
        registry.Unregister( "a" );
        ASSERT_TRUE( 4 == observer.m_registered );
        ASSERT_TRUE( 1 == observer.m_unregistered && observer.m_lastUnregistered == "a" );

        ASSERT_TRUE( registry.TryUnregister( "b" ) );
        ASSERT_TRUE( 4 == observer.m_registered );
        ASSERT_TRUE( 2 == observer.m_unregistered && observer.m_lastUnregistered == "b" );

        ASSERT_FALSE( registry.TryUnregister( "b" ) );                   // already gone: no event
        ASSERT_TRUE( 2 == observer.m_unregistered );

        registry.UnregisterAll();
        ASSERT_TRUE( 0 == registry.GetCount() );
        ASSERT_TRUE( 4 == observer.m_unregistered );
        ASSERT_TRUE( 4 == observer.m_registered );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GlobalONRegistry: built-in entries are known as such until they are unregistered" )
    try
    {
        TTestRegistry registry;
        registry.Register( "plugin", NewItem( 1 ) );
        registry.RegisterBuiltIn( "builtin", NewItem( 2 ) );
        ASSERT_FALSE( registry.IsBuiltIn( "plugin" ) );
        ASSERT_TRUE( registry.IsBuiltIn( "builtin" ) );
        ASSERT_FALSE( registry.IsBuiltIn( "unknown" ) );

        // a name in use cannot be registered again, as built-in or otherwise, and does not change the kind of the entry
        bool threw = false;
        try { registry.RegisterBuiltIn( "plugin", NewItem( 3 ) ); } catch ( ... ) { threw = true; }
        ASSERT_TRUE( threw );
        ASSERT_FALSE( registry.IsBuiltIn( "plugin" ) );
        ASSERT_TRUE( 1 == registry.Lookup( "plugin" )->m_value );

        // gone is gone: a later registration of the same name by a plugin is not a built-in entry, through every way out
        registry.Unregister( "builtin" );
        ASSERT_FALSE( registry.IsBuiltIn( "builtin" ) );
        registry.Register( "builtin", NewItem( 4 ) );
        ASSERT_FALSE( registry.IsBuiltIn( "builtin" ) );
        ASSERT_TRUE( registry.TryUnregister( "builtin" ) );
        ASSERT_TRUE( registry.TryRegisterBuiltIn( "builtin", NewItem( 5 ) ) );
        ASSERT_TRUE( registry.IsBuiltIn( "builtin" ) );
        ASSERT_TRUE( registry.TryUnregister( "builtin" ) );
        ASSERT_FALSE( registry.IsBuiltIn( "builtin" ) );
        registry.Register( "builtin", NewItem( 6 ) );
        ASSERT_FALSE( registry.IsBuiltIn( "builtin" ) );

        // everything goes when the application shuts down, built-in entries included
        registry.RegisterBuiltIn( "again", NewItem( 7 ) );
        registry.UnregisterAll();
        ASSERT_TRUE( 0 == registry.GetCount() );
        ASSERT_FALSE( registry.IsBuiltIn( "again" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GlobalONRegistry: the unregistration of all plugins removes the entries of plugins and keeps the built-in ones" )
    try
    {
        TTestRegistry registry;
        CTestRegistryObserver observer;
        observer.SubscribeTo( &registry.AsNotifier(), CORE::CTONRegistryEvents::ItemUnregisteredEvent );

        registry.RegisterBuiltIn( "builtin1", NewItem( 1 ) );
        registry.Register( "plugin1", NewItem( 2 ) );
        ASSERT_TRUE( registry.TryRegister( "plugin2", NewItem( 3 ) ) );
        ASSERT_TRUE( registry.TryRegisterBuiltIn( "builtin2", NewItem( 4 ) ) );
        ASSERT_TRUE( 4 == registry.GetCount() );

        // what the plugin control does when all plugins are unregistered
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnregisterAll();

        ASSERT_TRUE( 2 == registry.GetCount() );
        ASSERT_TRUE( registry.IsRegistered( "builtin1" ) && registry.IsRegistered( "builtin2" ) );
        ASSERT_FALSE( registry.IsRegistered( "plugin1" ) || registry.IsRegistered( "plugin2" ) );
        ASSERT_TRUE( registry.IsBuiltIn( "builtin1" ) && registry.IsBuiltIn( "builtin2" ) );
        ASSERT_TRUE( 2 == observer.m_unregistered );
        ASSERT_TRUE( 1 == registry.Lookup( "builtin1" )->m_value && 4 == registry.Lookup( "builtin2" )->m_value );

        // plugins can come back: the registry works as before and a second round does the same
        registry.Register( "plugin1", NewItem( 5 ) );
        ASSERT_TRUE( 3 == registry.GetCount() );
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnregisterAll();
        ASSERT_TRUE( 2 == registry.GetCount() );
        ASSERT_TRUE( registry.IsRegistered( "builtin1" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
