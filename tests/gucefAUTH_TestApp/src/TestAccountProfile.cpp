/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSigner.h"
#include "TestAccountProfile.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counts how often a profile says it changed */
class CProfileChangeCounter : public CORE::CObserver
{
    public:

    UInt32 changes;

    explicit CProfileChangeCounter( AUTH::IAccountProfile& profile )
        : CORE::CObserver(), changes( 0 ), m_changedEvent( profile.GetProfileChangedEvent() )
    {
        if ( GUCEF_NULL != profile.GetProfileNotifier() )
            SubscribeTo( profile.GetProfileNotifier(), profile.GetProfileChangedEvent() );
    }

    virtual ~CProfileChangeCounter() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        // a new observer is also told that it subscribed
        if ( m_changedEvent == eventId )
            ++changes;
    }

    private:

    CORE::CEvent m_changedEvent;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAccountProfileTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING account profile TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "AccountProfile" );

    GUCEF_TESTFW_TESTCASE( "Test: The name of the person is part of the settings, written only when there is one" )
        try
        {
            AUTH::CAuthSettings settings;
            ASSERT_TRUE( settings.displayName.IsNULLOrEmpty() );
            CORE::CDataNode empty( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( empty ) );
            ASSERT_TRUE( GUCEF_NULL == empty.GetAttribute( "displayName" ) );

            settings.displayName = "Dinand";
            CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( node ) );
            ASSERT_TRUE( GUCEF_NULL != node.GetAttribute( "displayName" ) );
            AUTH::CAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( node ) );
            ASSERT_TRUE( "Dinand" == loaded.displayName );

            // copies keep it
            AUTH::CAuthSettings copy( settings );
            AUTH::CAuthSettings assigned;
            assigned = settings;
            ASSERT_TRUE( "Dinand" == copy.displayName && "Dinand" == assigned.displayName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A profile in the settings of the manager: name, id from the profile key, change event" )
        try
        {
            CTestSigner signer( "profile" );
            ASSERT_TRUE( signer.IsValid() );
            AUTH::CAuthManager manager;
            manager.SetSigner( &signer );

            AUTH::CAuthSettingsProfile profile( &manager );
            ASSERT_TRUE( profile.GetDisplayName().IsNULLOrEmpty() );
            ASSERT_TRUE( profile.GetProfileId() == signer.GetKeyId() && !profile.GetProfileId().IsNULLOrEmpty() );
            ASSERT_TRUE( &profile == profile.GetProfileNotifier() );

            CProfileChangeCounter counter( profile );
            ASSERT_TRUE( profile.SetDisplayName( "Dinand" ) );
            ASSERT_TRUE( "Dinand" == profile.GetDisplayName() && "Dinand" == manager.GetSettings().displayName );
            ASSERT_TRUE( 1 == counter.changes );

            // the same name is not a change, an empty name is not a name
            ASSERT_TRUE( profile.SetDisplayName( "Dinand" ) );
            ASSERT_TRUE( 1 == counter.changes );
            ASSERT_FALSE( profile.SetDisplayName( CORE::CString::Empty ) );
            ASSERT_TRUE( "Dinand" == profile.GetDisplayName() && 1 == counter.changes );

            // a name that comes with the settings
            AUTH::CAuthSettings settings( manager.GetSettings() );
            settings.displayName = "Other";
            manager.SetSettings( settings );
            ASSERT_TRUE( "Other" == profile.GetDisplayName() );

            // the manager goes: no name, no id, the profile notices
            const UInt32 before = counter.changes;
            {
                AUTH::CAuthManager* temporary = new AUTH::CAuthManager();
                AUTH::CAuthSettingsProfile bound( temporary );
                CProfileChangeCounter boundCounter( bound );
                delete temporary;
                ASSERT_TRUE( GUCEF_NULL == bound.GetAuthManager() && bound.GetDisplayName().IsNULLOrEmpty() && bound.GetProfileId().IsNULLOrEmpty() );
                ASSERT_TRUE( 1 == boundCounter.changes );
                ASSERT_FALSE( bound.SetDisplayName( "Nobody" ) );
            }
            ASSERT_TRUE( before == counter.changes );
            manager.SetSigner( GUCEF_NULL );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A profile without a manager has nothing and keeps nothing" )
        try
        {
            AUTH::CAuthSettingsProfile profile;
            ASSERT_TRUE( profile.GetDisplayName().IsNULLOrEmpty() && profile.GetProfileId().IsNULLOrEmpty() );
            ASSERT_FALSE( profile.SetDisplayName( "Dinand" ) );
            AUTH::IAccountProfile& asInterface = profile;
            ASSERT_TRUE( AUTH::CAuthSettingsProfile::ProfileChangedEvent == asInterface.GetProfileChangedEvent() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The names of providers and of the namespaces of accounts" )
        try
        {
            AUTH::CAuthProviderDescriptor descriptor;
            descriptor.name = "fakebrowser";
            descriptor.displayName = "Fake Browser";
            descriptor.accountProviderId = "fake";
            descriptor.protocol = AUTH::AUTHPROTOCOL_PLATFORM;
            ASSERT_TRUE( AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().RegisterDescriptor( descriptor ) );

            ASSERT_TRUE( "Fake Browser" == AUTH::GetProviderDisplayName( "fakebrowser" ) && "Plainprov" == AUTH::GetProviderDisplayName( "plainprov" ) );
            ASSERT_TRUE( "Fake Browser" == AUTH::GetAccountProviderDisplayName( "fake" ) && "Zzz" == AUTH::GetAccountProviderDisplayName( "zzz" ) );
            // the display name of a provider comes from its descriptor, which a plugin registered
            ASSERT_TRUE( "TestOpenId" == AUTH::GetProviderDisplayName( "testopenid" ) && "TestOpenId" == AUTH::GetAccountProviderDisplayName( "testopenid" ) );
            ASSERT_TRUE( "AWS" == AUTH::GetAccountProviderDisplayName( "aws" ) );
            ASSERT_TRUE( AUTH::GetProviderDisplayName( CORE::CString::Empty ).IsNULLOrEmpty() && AUTH::GetAccountProviderDisplayName( CORE::CString::Empty ).IsNULLOrEmpty() );

            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "fakebrowser" );
        }
        catch ( ... )
        {
            ERRORHERE;
            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "fakebrowser" );
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
