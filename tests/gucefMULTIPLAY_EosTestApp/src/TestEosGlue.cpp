/*
 *  gucefMULTIPLAY_EosTestApp: Application for testing the Epic Online Services plugins of gucefMULTIPLAY, gucefDRN and gucefAUTH
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


#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "authpluginEOS_CEosAuthProvider.h"

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H
#include "pluginglueEOS_CEosPlatformLease.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H ? */

#include "TestEosCommon.h"
#include "TestEosGlue.h"

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::EOS;
using namespace EosFake;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const TestEncryptionKey = "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff";

/** Leases the platform of the table and returns before anything else is done: the lease is what lets the platform go */
static bool
LeaseAndReturnEarly( const SEosPlatformShim* shim )
{
    CORE::CString reason;
    CEosPlatformLease lease( shim, GUCEF_NULL, reason );
    if ( lease.IsHeld() )
        return false;
    return true;
}

/** Leases the platform of the table and throws: the lease is what lets the platform go */
static void
LeaseAndThrow( const SEosPlatformShim* shim )
{
    CORE::CString reason;
    CEosPlatformLease lease( shim, GUCEF_NULL, reason );
    throw 42;
}

/** Counts the ticks and the login changes of a platform */
class CPlatformRecorder : public CORE::CObserver
{
    public:

    UInt32 ticks;
    UInt32 loginChanges;
    CEosLoginState lastState;
    UInt32 threadId;

    CPlatformRecorder( void ) : CORE::CObserver(), ticks( 0 ), loginChanges( 0 ), lastState(), threadId( 0 ) {}

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        threadId = MT::GetCurrentTaskID();
        if ( CEosPlatform::TickEvent == eventId )
        {
            ++ticks;
        }
        else
        if ( CEosPlatform::LoginStateChangedEvent == eventId )
        {
            ++loginChanges;
            const CEosLoginState* state = static_cast< const CEosLoginState* >( eventData );
            if ( GUCEF_NULL != state )
                lastState = *state;
        }
    }
};

/** A node as an application config has it: Auth/Providers/epic */
static CORE::CDataNode
MakeAuthSection( const char* productId = "product-1" )
{
    CORE::CDataNode auth( "Auth", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* providers = auth.AddChild( "Providers", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* epic = providers->AddChild( "epic", GUCEF_DATATYPE_OBJECT );
    epic->SetAttribute( "clientId", "xyz-client-id" );
    epic->SetAttribute( "clientSecret", "EOS-CLIENT-SECRET-5d8e0c47-DO-NOT-LOG" );
    CORE::CDataNode* parameters = epic->AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    parameters->SetAttribute( "productId", productId );
    parameters->SetAttribute( "sandboxId", "sandbox-1" );
    parameters->SetAttribute( "deploymentId", "deployment-1" );
    parameters->SetAttribute( "encryptionKey", TestEncryptionKey );
    return auth;
}

/** The node the auth manager hands the provider */
static CORE::CDataNode
MakeProviderNode( void )
{
    CORE::CDataNode node( "epic", GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "clientId", "xyz-client-id" );
    node.SetAttribute( "clientSecret", "EOS-CLIENT-SECRET-5d8e0c47-DO-NOT-LOG" );
    CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    parameters->SetAttribute( "productId", "product-1" );
    parameters->SetAttribute( "sandboxId", "sandbox-1" );
    parameters->SetAttribute( "deploymentId", "deployment-1" );
    parameters->SetAttribute( "encryptionKey", TestEncryptionKey );
    parameters->SetAttribute( "tickIntervalInMs", 5 );
    return node;
}

class CCounterAtLeast : public CCondition
{
    public:
    const UInt32& counter;
    UInt32 value;
    CCounterAtLeast( const UInt32& c, UInt32 v ) : counter( c ), value( v ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return counter >= value; }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEosGlueTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING EOS Glue TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "EosGlue" );

    GUCEF_TESTFW_TESTCASE( "Test: The settings come from the Epic provider of the auth section, are checked for their form and are not shown when saved to a log" )
        try
        {
            CEosPlatformSettings settings;
            ASSERT_FALSE( settings.IsConfigured() );
            ASSERT_FALSE( settings.LoadAuthSection( CORE::CDataNode( "Auth", GUCEF_DATATYPE_OBJECT ) ) );

            CORE::CDataNode auth = MakeAuthSection();
            ASSERT_TRUE( settings.LoadAuthSection( auth ) );
            ASSERT_TRUE( settings.IsConfigured() && settings.IsWellFormed() );
            ASSERT_TRUE( settings.clientId == "xyz-client-id" && settings.productId == "product-1" && settings.sandboxId == "sandbox-1" && settings.deploymentId == "deployment-1" );
            ASSERT_TRUE( settings.productName == "GUCEF" && settings.productVersion == "1.0" );
            ASSERT_TRUE( settings.clientSecret.Reveal() == "EOS-CLIENT-SECRET-5d8e0c47-DO-NOT-LOG" && settings.encryptionKey.Reveal() == TestEncryptionKey );

            // the section can be the node above it, too
            CORE::CDataNode root( "Root", GUCEF_DATATYPE_OBJECT );
            *root.AddChild( "Auth", GUCEF_DATATYPE_OBJECT ) = auth;
            CEosPlatformSettings fromRoot;
            ASSERT_TRUE( fromRoot.LoadAuthSection( root ) && fromRoot == settings );

            // what is given to the SDK
            SEosPlatformConfig config;
            settings.FillPlatformConfig( config );
            ASSERT_TRUE( 0 == strcmp( config.productId, "product-1" ) && 0 == strcmp( config.clientId, "xyz-client-id" ) && 0 == strcmp( config.clientSecret, "EOS-CLIENT-SECRET-5d8e0c47-DO-NOT-LOG" ) );

            // a key of the wrong form is not used
            CORE::CDataNode badKey = MakeAuthSection();
            badKey.FindChild( "Providers" )->FindChild( "epic" )->FindChild( "Parameters" )->SetAttribute( "encryptionKey", "not a key" );
            CEosPlatformSettings untouched = settings;
            ASSERT_FALSE( untouched.LoadAuthSection( badKey ) );
            ASSERT_TRUE( untouched == settings );

            // incomplete settings are not configured
            CEosPlatformSettings partial = settings;
            partial.sandboxId.Clear();
            ASSERT_FALSE( partial.IsConfigured() );
            CEosPlatformSettings noSecret = settings;
            noSecret.clientSecret.Clear();
            ASSERT_FALSE( noSecret.IsConfigured() );

            // the same product and another one
            CEosPlatformSettings other = settings;
            ASSERT_TRUE( settings.IsSameProduct( other ) );
            other.productId = "product-2";
            ASSERT_FALSE( settings.IsSameProduct( other ) );

            // saving and loading
            CORE::CDataNode saved( "epic", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( saved ) );
            CEosPlatformSettings reloaded;
            ASSERT_TRUE( reloaded.LoadConfig( saved ) && reloaded == settings );

            // the secrets do not show in a log of the settings
            CLogCapture capture;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Settings: " << settings.clientId << " " << settings.clientSecret << " " << settings.encryptionKey );
            ASSERT_FALSE( capture.Contains( "EOS-CLIENT-SECRET" ) || capture.Contains( TestEncryptionKey ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A platform is made once for a function table and shared: holders are counted and the last one releases it" )
        try
        {
            EnsureGlueSettings();
            CEosGlue* glue = CEosGlue::Instance();
            UInt32 platformsBefore = glue->GetPlatformCount();
            CFakeEosWorld world;
            CFakeEosPeer peer( world, "00112233445566778899aabbccdd10", "0123456789abcdef0123456789ab10" );
            CFakeEosPeer other( world, "00112233445566778899aabbccdd11", "0123456789abcdef0123456789ab11" );
            SEosPlatformShim shim = peer.PlatformShim();
            SEosPlatformShim otherShim = other.PlatformShim();

            CORE::CString reason;
            CEosPlatform* first = glue->AcquirePlatform( &shim, GUCEF_NULL, reason );
            REQUIRE_TRUE( GUCEF_NULL != first );
            ASSERT_TRUE( 1 == first->GetRefCount() && first->WasCreatedHere() && first->IsReady() && 1 == peer.createPlatformCalls );
            CEosPlatform* second = glue->AcquirePlatform( &shim, GUCEF_NULL, reason );
            ASSERT_TRUE( second == first && 2 == first->GetRefCount() && 1 == peer.createPlatformCalls );
            glue->AddRef( first );
            ASSERT_TRUE( 3 == first->GetRefCount() );

            // another user has a platform of its own
            CEosPlatform* third = glue->AcquirePlatform( &otherShim, GUCEF_NULL, reason );
            REQUIRE_TRUE( GUCEF_NULL != third );
            ASSERT_TRUE( third != first && platformsBefore + 2 == glue->GetPlatformCount() );

            glue->ReleasePlatform( first );
            glue->ReleasePlatform( first );
            ASSERT_TRUE( 0 == peer.releaseCalls && platformsBefore + 2 == glue->GetPlatformCount() );
            glue->ReleasePlatform( first );
            ASSERT_TRUE( 1 == peer.releaseCalls && platformsBefore + 1 == glue->GetPlatformCount() );
            glue->ReleasePlatform( third );
            ASSERT_TRUE( 1 == other.releaseCalls && platformsBefore == glue->GetPlatformCount() );

            // and a new one is made when it is needed again
            CEosPlatform* again = glue->AcquirePlatform( &shim, GUCEF_NULL, reason );
            REQUIRE_TRUE( GUCEF_NULL != again );
            ASSERT_TRUE( 2 == peer.createPlatformCalls && 1 == again->GetRefCount() );
            glue->ReleasePlatform( again );
            ASSERT_TRUE( 2 == peer.releaseCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A platform lease holds the platform for a scope and lets it go on every way out, unless the hold is kept" )
        try
        {
            EnsureGlueSettings();
            CEosGlue* glue = CEosGlue::Instance();
            const UInt32 platformsBefore = glue->GetPlatformCount();
            CFakeEosWorld world;
            CFakeEosPeer peer( world, "00112233445566778899aabbccdd20", "0123456789abcdef0123456789ab20" );
            SEosPlatformShim shim = peer.PlatformShim();
            CORE::CString reason;

            // the end of the block
            {
                CEosPlatformLease lease( &shim, GUCEF_NULL, reason );
                ASSERT_TRUE( lease.IsHeld() && GUCEF_NULL != lease.Get() && 1 == lease.Get()->GetRefCount() );
                ASSERT_TRUE( platformsBefore + 1 == glue->GetPlatformCount() );
            }
            ASSERT_TRUE( 1 == peer.releaseCalls && platformsBefore == glue->GetPlatformCount() );

            // an early return from the block
            ASSERT_FALSE( LeaseAndReturnEarly( &shim ) );
            ASSERT_TRUE( 2 == peer.releaseCalls && platformsBefore == glue->GetPlatformCount() );

            // an exception that unwinds the block
            bool caught = false;
            try
            {
                LeaseAndThrow( &shim );
            }
            catch ( int )
            {
                caught = true;
            }
            ASSERT_TRUE( caught && 3 == peer.releaseCalls && platformsBefore == glue->GetPlatformCount() );

            // a hold that is kept is the one of the keeper now: the lease lets nothing go
            CEosPlatform* kept = GUCEF_NULL;
            {
                CEosPlatformLease lease( &shim, GUCEF_NULL, reason );
                REQUIRE_TRUE( lease.IsHeld() );
                kept = lease.Keep();
                ASSERT_FALSE( lease.IsHeld() );
                ASSERT_TRUE( GUCEF_NULL == lease.Keep() );
            }
            ASSERT_TRUE( GUCEF_NULL != kept && 3 == peer.releaseCalls && 1 == kept->GetRefCount() && platformsBefore + 1 == glue->GetPlatformCount() );
            glue->ReleasePlatform( kept );
            ASSERT_TRUE( 4 == peer.releaseCalls && platformsBefore == glue->GetPlatformCount() );

            // released ahead of the end of the scope, and only once
            {
                CEosPlatformLease lease( &shim, GUCEF_NULL, reason );
                REQUIRE_TRUE( lease.IsHeld() );
                lease.Release();
                ASSERT_TRUE( !lease.IsHeld() && 5 == peer.releaseCalls );
                lease.Release();
            }
            ASSERT_TRUE( 5 == peer.releaseCalls && platformsBefore == glue->GetPlatformCount() );

            // nothing to hold when there is no platform for the user, and nothing to let go
            {
                CEosPlatformLease lease( "no-such-user", reason );
                ASSERT_FALSE( lease.IsHeld() );
            }
            ASSERT_TRUE( platformsBefore == glue->GetPlatformCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A platform that cannot be made says why: no SDK, no settings, the SDK refusing, another product, no default tables" )
        try
        {
            EnsureGlueSettings();
            CEosGlue* glue = CEosGlue::Instance();
            UInt32 platformsBefore = glue->GetPlatformCount();
            CFakeEosWorld world;
            CFakeEosPeer peer( world, "00112233445566778899aabbccdd10", "0123456789abcdef0123456789ab10" );
            SEosPlatformShim shim = peer.PlatformShim();
            CORE::CString reason;

            // no SDK
            peer.sdkAvailable = false;
            ASSERT_TRUE( GUCEF_NULL == glue->AcquirePlatform( &shim, GUCEF_NULL, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "SDK is not part of this build" ) >= 0 && 0 == peer.createPlatformCalls );
            peer.sdkAvailable = true;

            // the SDK refuses and says why
            peer.platformOk = false;
            peer.platformError = "EOS_Platform_Create gave null";
            reason.Clear();
            ASSERT_TRUE( GUCEF_NULL == glue->AcquirePlatform( &shim, GUCEF_NULL, reason ) );
            ASSERT_TRUE( reason == "EOS_Platform_Create gave null" && 1 == peer.createPlatformCalls );
            ASSERT_TRUE( platformsBefore == glue->GetPlatformCount() );
            peer.platformOk = true;

            // settings that are not complete
            CEosPlatformSettings incomplete;
            incomplete.clientId = "xyz-client-id";
            reason.Clear();
            ASSERT_TRUE( GUCEF_NULL == glue->AcquirePlatform( &shim, &incomplete, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "clientId" ) >= 0 );

            // the platform of one product is not given to settings of another
            CEosPlatform* platform = glue->AcquirePlatform( &shim, GUCEF_NULL, reason );
            REQUIRE_TRUE( GUCEF_NULL != platform );
            CEosPlatformSettings another = glue->GetSettings();
            another.productId = "another-product";
            reason.Clear();
            ASSERT_TRUE( GUCEF_NULL == glue->AcquirePlatform( &shim, &another, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "another product" ) >= 0 && 1 == platform->GetRefCount() );
            glue->ReleasePlatform( platform );

            // without a table nothing is acquired when the SDK is not compiled in and nobody set tables
            glue->ClearDefaultShims();
            reason.Clear();
            if ( !CEosGlue::IsSdkCompiledIn() )
            {
                ASSERT_TRUE( GUCEF_NULL == glue->AcquirePlatform( GUCEF_NULL, GUCEF_NULL, reason ) );
                ASSERT_TRUE( reason.HasSubstr( "SDK is not part of this build" ) >= 0 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Default tables are what a holder without a table gets, and the P2P of the default is part of that platform" )
        try
        {
            EnsureGlueSettings();
            CEosGlue* glue = CEosGlue::Instance();
            CFakeEosWorld world;
            CFakeEosPeer peer( world, "00112233445566778899aabbccdd10", "0123456789abcdef0123456789ab10" );
            glue->SetDefaultShims( peer.PlatformShim(), peer.P2PShim() );
            SEosPlatformShim defaultShim;
            SEosP2PShim defaultP2P;
            ASSERT_TRUE( glue->GetDefaultPlatformShim( defaultShim ) && glue->GetDefaultP2PShim( defaultP2P ) );
            ASSERT_TRUE( defaultShim.IsSamePlatform( peer.PlatformShim() ) );

            CORE::CString reason;
            CEosPlatform* viaDefault = glue->AcquirePlatform( GUCEF_NULL, GUCEF_NULL, reason );
            REQUIRE_TRUE( GUCEF_NULL != viaDefault );
            SEosPlatformShim shim = peer.PlatformShim();
            CEosPlatform* viaShim = glue->AcquirePlatform( &shim, GUCEF_NULL, reason );
            ASSERT_TRUE( viaShim == viaDefault && 2 == viaDefault->GetRefCount() && 1 == peer.createPlatformCalls );
            ASSERT_TRUE( GUCEF_NULL != viaDefault->GetP2PRouter() );

            glue->ReleasePlatform( viaShim );
            glue->ReleasePlatform( viaDefault );
            glue->ClearDefaultShims();
            ASSERT_FALSE( glue->GetDefaultPlatformShim( defaultShim ) && !CEosGlue::IsSdkCompiledIn() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The login state is one for everybody that holds the platform: a change is told once, the user that is logged in is found by its id" )
        try
        {
            EnsureGlueSettings();
            CEosGlue* glue = CEosGlue::Instance();
            CFakeEosWorld world;
            CFakeEosPeer peer( world, "00112233445566778899aabbccdd10", "0123456789abcdef0123456789ab10" );
            CFakeEosPeer other( world, "00112233445566778899aabbccdd11", "0123456789abcdef0123456789ab11" );
            SEosPlatformShim shim = peer.PlatformShim();
            SEosPlatformShim otherShim = other.PlatformShim();
            CORE::CString reason;

            CEosPlatform* platform = glue->AcquirePlatform( &shim, GUCEF_NULL, reason );
            REQUIRE_TRUE( GUCEF_NULL != platform );
            CPlatformRecorder recorder;
            recorder.SubscribeTo( platform );
            ASSERT_FALSE( platform->GetLoginState().IsLoggedIn() );
            ASSERT_TRUE( GUCEF_NULL == glue->AcquirePlatformOfUser( CORE::CString::Empty, reason ) );

            platform->SetLoginState( peer.epicAccountId, peer.puid );
            ASSERT_TRUE( 1 == recorder.loginChanges && recorder.lastState.IsLoggedIn() && recorder.lastState.productUserId == peer.puid && recorder.lastState.epicAccountId == peer.epicAccountId );
            ASSERT_TRUE( platform->GetLoginState().productUserId == peer.puid );
            ASSERT_TRUE( MT::GetCurrentTaskID() == recorder.threadId );

            // the same state again is not a change
            platform->SetLoginState( peer.epicAccountId, peer.puid );
            ASSERT_TRUE( 1 == recorder.loginChanges );

            // by empty id: the only user; by its id; by another one's id
            CEosPlatform* found = glue->AcquirePlatformOfUser( CORE::CString::Empty, reason );
            ASSERT_TRUE( found == platform && 2 == platform->GetRefCount() );
            glue->ReleasePlatform( found );
            found = glue->AcquirePlatformOfUser( peer.puid, reason );
            ASSERT_TRUE( found == platform );
            glue->ReleasePlatform( found );
            reason.Clear();
            ASSERT_TRUE( GUCEF_NULL == glue->AcquirePlatformOfUser( other.puid, reason ) );

            // two users: empty is not enough to choose
            CEosPlatform* second = glue->AcquirePlatform( &otherShim, GUCEF_NULL, reason );
            REQUIRE_TRUE( GUCEF_NULL != second );
            second->SetLoginState( other.epicAccountId, other.puid );
            reason.Clear();
            ASSERT_TRUE( GUCEF_NULL == glue->AcquirePlatformOfUser( CORE::CString::Empty, reason ) );
            ASSERT_TRUE( reason.HasSubstr( "More than one" ) >= 0 );
            found = glue->AcquirePlatformOfUser( other.puid, reason );
            ASSERT_TRUE( found == second );
            glue->ReleasePlatform( found );

            // logging out is a change too
            platform->ClearLoginState();
            ASSERT_TRUE( 2 == recorder.loginChanges && !recorder.lastState.IsLoggedIn() );
            platform->ClearLoginState();
            ASSERT_TRUE( 2 == recorder.loginChanges );

            recorder.UnsubscribeAllFromObserver();
            second->ClearLoginState();
            glue->ReleasePlatform( second );
            glue->ReleasePlatform( platform );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The SDK is ticked from the application pulse at the pace of the fastest holder, and not at all when nobody needs it" )
        try
        {
            EnsureGlueSettings();
            CEosGlue* glue = CEosGlue::Instance();
            CFakeEosWorld world;
            CFakeEosPeer peer( world, "00112233445566778899aabbccdd10", "0123456789abcdef0123456789ab10" );
            SEosPlatformShim shim = peer.PlatformShim();
            CORE::CString reason;
            CEosPlatform* platform = glue->AcquirePlatform( &shim, GUCEF_NULL, reason );
            REQUIRE_TRUE( GUCEF_NULL != platform );
            CPlatformRecorder recorder;
            recorder.SubscribeTo( platform );

            // nobody asked for ticks: it stays quiet
            ASSERT_TRUE( 0 == platform->GetTickDemandCount() && 0 == platform->GetTickInterval() );
            PumpFor( 60 );
            ASSERT_TRUE( 0 == peer.tickCalls && 0 == recorder.ticks );

            // a slow holder
            UInt32 slow = platform->AddTickDemand( 200 );
            ASSERT_TRUE( 200 == platform->GetTickInterval() );
            // a fast one sets the pace for all
            UInt32 fast = platform->AddTickDemand( 5 );
            ASSERT_TRUE( 5 == platform->GetTickInterval() && 2 == platform->GetTickDemandCount() );
            CCounterAtLeast enough( recorder.ticks, 8 );
            ASSERT_TRUE( PumpUntil( enough ) );
            ASSERT_TRUE( peer.tickCalls >= 8 && MT::GetCurrentTaskID() == recorder.threadId );

            // when it lets go the slow one is the pace again
            platform->RemoveTickDemand( fast );
            ASSERT_TRUE( 200 == platform->GetTickInterval() );
            platform->SetTickDemandInterval( slow, 10 );
            ASSERT_TRUE( 10 == platform->GetTickInterval() );
            platform->RemoveTickDemand( slow );
            ASSERT_TRUE( 0 == platform->GetTickDemandCount() );

            // and then it is quiet again
            PumpFor( 30 );
            UInt32 ticks = peer.tickCalls;
            PumpFor( 60 );
            ASSERT_TRUE( ticks == peer.tickCalls );

            // a tick by the application itself ticks the SDK and tells the holders
            UInt32 before = recorder.ticks;
            platform->Tick();
            ASSERT_TRUE( ticks + 1 == peer.tickCalls && before + 1 == recorder.ticks );

            recorder.UnsubscribeAllFromObserver();
            glue->ReleasePlatform( platform );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The auth plugin and the lobby service share one platform: the login of the auth plugin is the login of the lobby service" )
        try
        {
            EnsureGlueSettings();
            CEosGlue* glue = CEosGlue::Instance();
            UInt32 platformsBefore = glue->GetPlatformCount();
            CEosFixture fixture( 1, false );
            CEosPeerContext& context = fixture[ 0 ];

            {
                AUTHPLUGIN::EOS::CEosAuthProvider provider( context.peer.AuthShim() );
                ASSERT_TRUE( provider.LoadConfig( MakeProviderNode() ) );

                // not logged in yet: the lobby service has nothing to work with
                ASSERT_FALSE( context.Connect() );
                ASSERT_TRUE( context.reason.HasSubstr( "logged in" ) >= 0 );
                UInt32 createdBefore = context.peer.createPlatformCalls;
                ASSERT_TRUE( platformsBefore == glue->GetPlatformCount() && createdBefore == context.peer.releaseCalls );

                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( provider.TrySilentLogin( future ) && PumpUntilReady( future, 3000 ) );
                AUTH::CAuthResult result;
                ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
                CEosPlatform* platform = provider.GetPlatform();
                REQUIRE_TRUE( GUCEF_NULL != platform );
                ASSERT_TRUE( platform->GetLoginState().productUserId == context.peer.puid && createdBefore + 1 == context.peer.createPlatformCalls );

                // the lobby service finds the platform by the user and does not make one
                ASSERT_TRUE( context.Connect() );
                ASSERT_TRUE( createdBefore + 1 == context.peer.createPlatformCalls && platformsBefore + 1 == glue->GetPlatformCount() && platform->GetRefCount() >= 2 );

                MULTIPLAY::CILobbyPtr lobby = context.Create( CEosFixture::MakeParams() );
                ASSERT_TRUE( !lobby.IsNULL() );
                lobby = MULTIPLAY::CILobbyPtr();

                // the provider goes: the service still has the platform
                context.service.Disconnect();
            }
            ASSERT_TRUE( 2 == context.peer.releaseCalls && platformsBefore == glue->GetPlatformCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A user that logs out takes the lobby service with it: it disconnects and its lobbies are left" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            ASSERT_TRUE( fixture[ 1 ].service.IsConnected() );
            CORE::CString lobbyId = LobbyIdOf( pair.host );

            fixture[ 1 ].platform->ClearLoginState();
            ASSERT_TRUE( PumpUntilCount( fixture[ 1 ].serviceRecorder.disconnected, 1 ) );
            ASSERT_FALSE( fixture[ 1 ].service.IsConnected() );
            ASSERT_TRUE( PumpUntilCount( pair.guestEvents.closed, 1 ) );
            CMemberCount alone( pair.host, 1 );
            ASSERT_TRUE( PumpUntil( alone ) );

            // and the others go on
            ASSERT_TRUE( pair.host->SetSetting( "map", "ice" ) );
            ASSERT_TRUE( 1 == fixture.world.FindLobby( lobbyId )->members.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
