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

#ifndef GUCEF_MULTIPLAY_EOSTESTAPP_TESTEOSCOMMON_H
#define GUCEF_MULTIPLAY_EOSTESTAPP_TESTEOSCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <exception>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#include "pluginglueEOS_CEosGlue.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H
#include "lobbyservicepluginEOS_CEosLobbyService.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H
#include "lobbyservicepluginEOS_CEosLobby.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H ? */

#include "TestFakeEos.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Pulses the global pulse generator from the test thread for the given time */
void PumpFor( GUCEF::UInt32 durationInMs );

/** A condition the pump helpers wait for */
class CCondition
{
    public:
    virtual ~CCondition() {}
    virtual bool IsMet( void ) = 0;
};

/** Pulses until the condition is met or the time is up; true when it is met */
bool PumpUntil( CCondition& condition, GUCEF::UInt32 timeoutInMs = 5000 );

/** Pulses until the future has ended; true if it did in time */
bool PumpUntilReady( const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutInMs = 5000 );

/** Pulses until the counter has reached the value */
bool PumpUntilCount( const GUCEF::UInt32& counter, GUCEF::UInt32 value, GUCEF::UInt32 timeoutInMs = 5000 );

/** Loads a plugin from the directory of the module, debug or release build, into a group of its own */
bool LoadTestPlugin( const GUCEF::CORE::CString& pluginName, const GUCEF::CORE::CString& group = "test" );

/** The settings of the product that every platform of the tests is made with, in the glue */
void EnsureGlueSettings( void );

/** Collects everything that is logged while it exists, to prove what does and does not end up in a log */
class CLogCapture
{
    public:

    CLogCapture( void );
    ~CLogCapture();

    bool Contains( const GUCEF::CORE::CString& text );
    GUCEF::CORE::CString Text( void );

    private:

    CLogCapture( const CLogCapture& src );
    CLogCapture& operator=( const CLogCapture& src );

    GUCEF::CORE::CDynamicBuffer m_buffer;
    GUCEF::CORE::CDynamicBufferAccess m_access;
    GUCEF::CORE::CStdLogger m_logger;
};

/*-------------------------------------------------------------------------*/

/** Records what a lobby tells the application */
class CLobbyRecorder : public GUCEF::CORE::CObserver
{
    public:

    GUCEF::UInt32 stateChanged;
    GUCEF::CORE::CString::StringVector paths;
    GUCEF::CORE::CString::StringVector joined;
    GUCEF::CORE::CString::StringVector left;
    GUCEF::UInt32 launches;
    GUCEF::MULTIPLAY::CLaunchDescription lastLaunch;
    GUCEF::UInt32 closed;
    GUCEF::CORE::CString closedReason;
    GUCEF::UInt32 refusals;
    GUCEF::CORE::CString::StringVector refusalWhat;
    GUCEF::CORE::CString::StringVector refusalReasons;
    GUCEF::UInt32 votesStarted;
    GUCEF::UInt32 votesEnded;
    bool lastVotePassed;
    GUCEF::CORE::CString lastVoteId;
    GUCEF::vector< GUCEF::Int32 > countdowns;
    GUCEF::UInt32 eventThreadId;
    GUCEF::UInt32 wrongThreadEvents;
    GUCEF::UInt32 expectedThreadId;

    CLobbyRecorder( void );
    virtual ~CLobbyRecorder();

    bool HasPath( const GUCEF::CORE::CString& path ) const;
    bool HasRefusal( const GUCEF::CORE::CString& what ) const;
    void Clear( void );

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** Records what a service tells the application */
class CServiceRecorder : public GUCEF::CORE::CObserver
{
    public:

    GUCEF::UInt32 connected;
    GUCEF::UInt32 disconnected;
    GUCEF::UInt32 lobbyJoined;
    GUCEF::MULTIPLAY::CILobbyPtr lastLobby;
    GUCEF::UInt32 invites;
    GUCEF::CORE::CString lastInviteLobby;
    GUCEF::CORE::CString lastInviteSender;
    class CLobbyRecorder* watcher;      /**< when set, it is subscribed to every lobby the moment the service has it joined */

    CServiceRecorder( void );
    virtual ~CServiceRecorder();

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** One user of the fake: its peer, the platform it holds, the lobby service on it */
class CEosPeerContext
{
    public:

    EosFake::CFakeEosPeer peer;
    GUCEF::PLUGINGLUE::EOS::CEosPlatform* platform;
    GUCEF::MULTIPLAY::CPlayerIdentity identity;
    GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobbyService service;
    CServiceRecorder serviceRecorder;

    CEosPeerContext( EosFake::CFakeEosWorld& world, GUCEF::UInt32 index );
    ~CEosPeerContext();

    /** The platform is made and the user logged in, the way the auth plugin does it */
    bool Login( void );

    /** The service connects */
    bool Connect( void );

    /** Creates a lobby and waits for it; NULL when it failed */
    GUCEF::MULTIPLAY::CILobbyPtr Create( const GUCEF::MULTIPLAY::CLobbyCreateParams& params );

    /** Joins a lobby and waits for it; NULL when it failed */
    GUCEF::MULTIPLAY::CILobbyPtr Join( const GUCEF::CORE::CString& lobbyId, const GUCEF::CORE::CString& password = GUCEF::CORE::CString::Empty, bool asSpectator = false );

    GUCEF::CORE::CString reason;     /**< why the last Create(), Join() or Connect() failed */
};

/*-------------------------------------------------------------------------*/

/** A world and its users, logged in and connected */
class CEosFixture
{
    public:

    EosFake::CFakeEosWorld world;
    GUCEF::vector< CEosPeerContext* > peers;

    CEosFixture( GUCEF::UInt32 peerCount, bool connect = true );
    ~CEosFixture();

    CEosPeerContext& operator[]( size_t index ) { return *peers[ index ]; }

    /** The settings of the game for a lobby: a name, 4 players, a map and a speed setting */
    static GUCEF::MULTIPLAY::CLobbyCreateParams MakeParams( const GUCEF::CORE::CString& name = "Fake lobby" );

    /** The same settings for every service of the fixture */
    void ApplySettings( const GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobbyServiceSettings& settings );

    static GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobbyServiceSettings MakeFastSettings( void );
};

/*-------------------------------------------------------------------------*/

/** Waits until a lobby of a member has a member count */
class CMemberCount : public CCondition
{
    public:
    GUCEF::MULTIPLAY::CILobbyPtr lobby;
    size_t count;
    CMemberCount( GUCEF::MULTIPLAY::CILobbyPtr l, size_t c ) : lobby( l ), count( c ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return lobby->GetState().members.size() == count; }
};

/** Waits until a setting of a lobby has a value */
class CSettingIs : public CCondition
{
    public:
    GUCEF::MULTIPLAY::CILobbyPtr lobby;
    GUCEF::CORE::CString key;
    GUCEF::CORE::CString value;
    CSettingIs( GUCEF::MULTIPLAY::CILobbyPtr l, const GUCEF::CORE::CString& k, const GUCEF::CORE::CString& v ) : lobby( l ), key( k ), value( v ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return lobby->GetState().GetSetting( key ) == value; }
};

/** Waits until the lobby is in a phase */
class CPhaseIs : public CCondition
{
    public:
    GUCEF::MULTIPLAY::CILobbyPtr lobby;
    GUCEF::MULTIPLAY::TLobbyPhase phase;
    CPhaseIs( GUCEF::MULTIPLAY::CILobbyPtr l, GUCEF::MULTIPLAY::TLobbyPhase p ) : lobby( l ), phase( p ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return lobby->GetState().phase == phase; }
};

/** Waits until a member is ready in the view of a lobby */
class CMemberReady : public CCondition
{
    public:
    GUCEF::MULTIPLAY::CILobbyPtr lobby;
    GUCEF::CORE::CString memberId;
    bool ready;
    CMemberReady( GUCEF::MULTIPLAY::CILobbyPtr l, const GUCEF::CORE::CString& m, bool r ) : lobby( l ), memberId( m ), ready( r ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        const GUCEF::MULTIPLAY::CLobbyMember* member = lobby->GetState().FindMember( memberId );
        return GUCEF_NULL != member && member->ready == ready;
    }
};

/** Waits until the lobby has a boss */
class CBossIs : public CCondition
{
    public:
    GUCEF::MULTIPLAY::CILobbyPtr lobby;
    GUCEF::CORE::CString bossId;
    CBossIs( GUCEF::MULTIPLAY::CILobbyPtr l, const GUCEF::CORE::CString& b ) : lobby( l ), bossId( b ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return lobby->GetState().bossId == bossId; }
};

/*-------------------------------------------------------------------------*/

/** The text of why a future failed */
GUCEF::CORE::CString FailureOf( const GUCEF::CORE::CFutureResult& future );

/** Browses and gives the listings; false with the reason when the browse failed */
bool Browse( CEosPeerContext& context, const GUCEF::MULTIPLAY::CGameListingFilter& filter, GUCEF::MULTIPLAY::CGameListingList& list, GUCEF::CORE::CString* failure = GUCEF_NULL );

GUCEF::CORE::CString LobbyIdOf( const GUCEF::MULTIPLAY::CILobbyPtr& lobby );

GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobby& Eos( const GUCEF::MULTIPLAY::CILobbyPtr& lobby );

/** Two players in a lobby of the first */
struct SPair
{
    GUCEF::MULTIPLAY::CILobbyPtr host;
    GUCEF::MULTIPLAY::CILobbyPtr guest;
    CLobbyRecorder hostEvents;
    CLobbyRecorder guestEvents;
};

/** The first player makes the lobby, the second joins it; false when one of them did not get through */
bool MakePair( CEosFixture& fixture, SPair& pair, const GUCEF::MULTIPLAY::CLobbyCreateParams& params );

/** Waits until the lobby of EOS has a number of members */
class CEosMembers : public CCondition
{
    public:
    EosFake::CFakeEosWorld& world;
    GUCEF::CORE::CString lobbyId;
    size_t count;
    CEosMembers( EosFake::CFakeEosWorld& w, const GUCEF::CORE::CString& l, size_t c ) : world( w ), lobbyId( l ), count( c ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        EosFake::SFakeLobby* lobby = world.FindLobby( lobbyId );
        return GUCEF_NULL != lobby && lobby->members.size() == count;
    }
};

/** Whether the text is in an attribute of the lobby or of a member of it */
bool IsInAnyAttribute( EosFake::SFakeLobby& lobby, const GUCEF::CORE::CString& text );

#endif /* GUCEF_MULTIPLAY_EOSTESTAPP_TESTEOSCOMMON_H ? */
