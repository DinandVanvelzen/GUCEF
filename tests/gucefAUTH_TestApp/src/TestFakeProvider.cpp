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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#include "TestFakeProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

const CORE::CString CFakeAuthProvider::SecretAccessToken = "SECRET-ACCESS-TOKEN-7f3a91c2";
const CORE::CString CFakeAuthProvider::SecretIdToken = "SECRET-ID-TOKEN-b04d55e8";
const CORE::CString CFakeAuthProvider::SecretRefreshTokenPrefix = "SECRET-REFRESH-TOKEN-";
const CORE::CString CFakeAuthProvider::SecretProof = "SECRET-PROOF-d1e6a7c0";
const CORE::CString CFakeLoginTaskConsumer::TaskType = "GUCEF::AUTH::TestApp::CFakeLoginTaskConsumer";

static CORE::CTFactory< CORE::CTaskConsumer, CFakeLoginTaskConsumer, MT::CMutex > g_fakeLoginTaskFactory;
static MT::CMutex g_fakeLock;
static UInt32 g_fakeResultSequence = 0;
static UInt32 g_fakeRunningTasks = 0;
static bool g_fakeLoginTaskRegistered = false;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CFakeLoginTaskData : public CORE::CICloneable
{
    public:

    CFakeAuthProvider* provider;
    CORE::CFutureResultSource source;
    UInt32 delayMs;
    bool fail;
    bool hang;

    CFakeLoginTaskData( void ) : CORE::CICloneable(), provider( GUCEF_NULL ), source(), delayMs( 0 ), fail( false ), hang( false ) {}

    CFakeLoginTaskData( const CFakeLoginTaskData& src )
        : CORE::CICloneable( src ), provider( src.provider ), source( src.source ), delayMs( src.delayMs ), fail( src.fail ), hang( src.hang ) {}

    virtual ~CFakeLoginTaskData() {}

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return GUCEF_NEW CFakeLoginTaskData( *this );
    }

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return CloneableTypeName();
    }

    static const char* CloneableTypeName( void )
    {
        return "CFakeLoginTaskData";
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt32
NextResultSequence( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_fakeLock );
    return ++g_fakeResultSequence;
}

/*-------------------------------------------------------------------------*/

static UInt32
GetRunningTaskCount( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_fakeLock );
    return g_fakeRunningTasks;
}

/*-------------------------------------------------------------------------*/

CFakeAuthProvider::CFakeAuthProvider( const CORE::CString& providerName )
    : AUTH::CIAuthProvider()
    , name( providerName )
    , accountNamespace( "fake" )
    , subject( "player-1" )
    , displayName( "Fake Player" )
    , resultLoginHint()
    , capabilities( AUTH::AUTHCAP_BROWSER | AUTH::AUTHCAP_REFRESHABLE )
    , available( true )
    , announceAction( false )
    , giveRefreshToken( true )
    , loginBehaviour( FAKE_COMPLETE )
    , silentBehaviour( FAKE_COMPLETE )
    , refreshBehaviour( FAKE_COMPLETE )
    , failReason( AUTH::AUTHFAIL_DENIED )
    , delayMs( 100 )
    , explainsUnavailability( false )
    , unavailability()
    , beginCount( 0 )
    , silentCount( 0 )
    , refreshCount( 0 )
    , logoutCount( 0 )
    , verifyCount( 0 )
    , lastRefreshTokenSeen()
    , lastLogoutToken()
    , lastLogoutAccount()
    , lastOptions()
    , configuredClientId()
    , callLog( GUCEF_NULL )
    , m_taskSources()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFakeAuthProvider::~CFakeAuthProvider()
{GUCEF_TRACE;

    // Whatever runs on the pool for this provider is told to stop, and waited for: the tasks hold on to the provider
    for ( size_t i=0; i<m_taskSources.size(); ++i )
        m_taskSources[ i ].Cancel();
    for ( UInt32 waited=0; waited<5000 && GetRunningTaskCount() > 0; waited+=5 )
        MT::ThreadDelay( 5 );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFakeAuthProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return name;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeAuthProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    return capabilities;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::IsAvailable( void ) const
{GUCEF_TRACE;

    return available;
}

/*-------------------------------------------------------------------------*/

AUTH::CAuthResult
CFakeAuthProvider::MakeResult( void ) const
{GUCEF_TRACE;

    AUTH::CAuthResult result;
    UInt32 sequence = NextResultSequence();
    result.accountRef = AUTH::CAccountRef( accountNamespace, subject );
    result.displayName = displayName;
    result.loginHint = resultLoginHint;
    result.avatarUrl = "https://example.invalid/avatar.png";
    result.accessToken.Set( SecretAccessToken );
    result.idToken.Set( SecretIdToken );
    if ( giveRefreshToken )
        result.refreshToken.Set( SecretRefreshTokenPrefix + CORE::UInt32ToString( sequence ) );
    result.expiresAtMs = AUTH::GetEpochTimeInMs() + 3600000;
    result.scopes.push_back( "openid" );
    result.proof = AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, SecretProof + "." + name + "." + subject );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::Finish( CORE::CFutureResultSource& source, bool fail )
{GUCEF_TRACE;

    if ( fail )
        return FailLogin( source, failReason, "fake failure" );
    return CompleteLogin( source, MakeResult() );
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::Cancelled( CORE::CFutureResultSource& source )
{GUCEF_TRACE;

    return FailLogin( source, AUTH::AUTHFAIL_CANCELLED, "cancelled" );
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::Run( EFakeBehaviour behaviour, bool interactive, CORE::CFutureResult& future )
{GUCEF_TRACE;

    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( FAKE_REFUSE == behaviour )
        return false;

    CORE::CFutureResultSource source;
    future = source.GetFuture();

    if ( interactive && announceAction )
    {
        AUTH::CAuthActionRequired action;
        action.kind = AUTH::AUTHACTION_OPEN_URL;
        action.url = "https://example.invalid/login?state=abc";
        RaiseActionRequired( action );
    }

    switch ( behaviour )
    {
        case FAKE_COMPLETE :
            Finish( source, false );
            return true;
        case FAKE_FAIL :
            Finish( source, true );
            return true;
        case FAKE_NO_EVENT :
            source.Complete( MakeResult().Clone(), true );
            return true;
        case FAKE_IGNORE_CANCEL :
            return true;
        case FAKE_SLOW :
        case FAKE_SLOW_FAIL :
        case FAKE_HANG :
        {
            CFakeLoginTaskData taskData;
            taskData.provider = this;
            taskData.source = source;
            taskData.delayMs = delayMs;
            taskData.fail = FAKE_SLOW_FAIL == behaviour;
            taskData.hang = FAKE_HANG == behaviour;
            CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( CORE::CTaskManager::DefaultThreadPoolName );
            if ( pool.IsNULL() )
                return false;
            m_taskSources.push_back( source );
            {
                MT::CScopeMutex lock( g_fakeLock );
                ++g_fakeRunningTasks;
            }
            CORE::CFutureResult queued = pool->QueueTask( CFakeLoginTaskConsumer::TaskType, &taskData );
            if ( !queued.HasAFuture() )
            {
                MT::CScopeMutex lock( g_fakeLock );
                --g_fakeRunningTasks;
                return false;
            }
            return true;
        }
        default :
            return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::TrySilentLogin( CORE::CFutureResult& future )
{GUCEF_TRACE;

    ++silentCount;
    if ( GUCEF_NULL != callLog )
        callLog->push_back( "silent:" + name );
    if ( 0 == ( capabilities & AUTH::AUTHCAP_SILENT ) )
    {
        future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        return false;
    }
    return Run( silentBehaviour, false, future );
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future )
{GUCEF_TRACE;

    ++beginCount;
    if ( GUCEF_NULL != callLog )
        callLog->push_back( "begin:" + name );
    lastOptions = options;
    return Run( loginBehaviour, true, future );
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::Refresh( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken, CORE::CFutureResult& future )
{GUCEF_TRACE;

    ++refreshCount;
    if ( GUCEF_NULL != callLog )
        callLog->push_back( "refresh:" + name );
    lastRefreshTokenSeen = refreshToken.Reveal();
    // The refreshed login is the one of the account that was asked for
    CORE::CString previousSubject = subject;
    CORE::CString previousNamespace = accountNamespace;
    subject = account.GetSubject();
    accountNamespace = account.GetProvider();
    bool started = Run( refreshBehaviour, false, future );
    if ( FAKE_SLOW != refreshBehaviour && FAKE_SLOW_FAIL != refreshBehaviour && FAKE_HANG != refreshBehaviour )
    {
        subject = previousSubject;
        accountNamespace = previousNamespace;
    }
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken )
{GUCEF_TRACE;

    ++logoutCount;
    lastLogoutAccount = account;
    lastLogoutToken = refreshToken.Reveal();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result )
{GUCEF_TRACE;

    ++verifyCount;
    if ( proof.data.Reveal().HasSubstr( SecretProof ) < 0 )
        return false;
    result = MakeResult();
    result.providerName = name;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::GetUnavailability( AUTH::CAuthUnavailableProvider& unavailable ) const
{GUCEF_TRACE;

    unavailable.Clear();
    if ( !explainsUnavailability || IsAvailable() )
        return false;
    unavailable = unavailability;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "clientId", configuredClientId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeAuthProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    configuredClientId = config.GetAttributeValueOrChildValueByName( "clientId" ).AsString( configuredClientId, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFakeAuthProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "CFakeAuthProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CFakeLoginTaskConsumer::CFakeLoginTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFakeLoginTaskConsumer::~CFakeLoginTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeLoginTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

void
CFakeLoginTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CFakeLoginTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    CFakeLoginTaskData* data = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CFakeLoginTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return true;

    UInt32 waited = 0;
    bool done = false;
    while ( !done && ( data->hang || waited < data->delayMs ) )
    {
        // ended from outside: the provider is going away
        if ( data->source.IsCompleted() || IsDeactivationRequested() )
        {
            done = true;
        }
        else
        if ( data->source.IsCancellationRequested() )
        {
            data->provider->Cancelled( data->source );
            done = true;
        }
        else
        {
            MT::ThreadDelay( 5 );
            waited += 5;
        }
    }
    if ( !done )
        data->provider->Finish( data->source, data->fail );

    MT::CScopeMutex lock( g_fakeLock );
    --g_fakeRunningTasks;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
RegisterFakeLoginTask( void )
{GUCEF_TRACE;

    if ( g_fakeLoginTaskRegistered )
        return true;
    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CFakeLoginTaskConsumer::TaskType, &g_fakeLoginTaskFactory );
    g_fakeLoginTaskRegistered = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
UnregisterFakeLoginTask( void )
{GUCEF_TRACE;

    if ( !g_fakeLoginTaskRegistered )
        return;
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CFakeLoginTaskConsumer::TaskType );
    g_fakeLoginTaskRegistered = false;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
