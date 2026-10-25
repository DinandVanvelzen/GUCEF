/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H
#include "pluginglueEOS_CEosPlatformSettings.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H
#include "pluginglueEOS_CEosP2PRouter.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Who is logged in at the platform: the Epic account and the product user of EOS Connect */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosLoginState : public CORE::CICloneable
{
    public:

    CORE::CString epicAccountId;
    CORE::CString productUserId;

    CEosLoginState( void );

    CEosLoginState( const CORE::CString& epicAccountId, const CORE::CString& productUserId );

    /** The lobby and P2P interfaces need the product user id */
    bool IsLoggedIn( void ) const;

    bool operator==( const CEosLoginState& other ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  One EOS platform (EOS_HPlatform) that everyone who needs it shares: made by the first CEosGlue::AcquirePlatform(),
 *  released with the last CEosGlue::ReleasePlatform(). It holds
 *    - the tick: the platform is ticked by a timer on the pulse generator of the application, as long as somebody
 *      has a tick demand (a login under way, a lobby, a P2P connection). There is no thread of its own and nothing
 *      pumps another pulse generator.
 *    - who is logged in (GetLoginState()), set by whoever logs in (the auth plugin) with an event when it changes.
 *    - the P2P router, when a table for the P2P interface is attached.
 *
 *  Events (on the thread that pumps the pulse generator):
 *    TickEvent                 after every tick of the platform, for those who poll the interfaces of the SDK
 *    LoginStateChangedEvent    CEosLoginState: the state it is now
 */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosPlatform : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent TickEvent;
    static const CORE::CEvent LoginStateChangedEvent;

    static const UInt32 DefaultTickIntervalInMs = 50;

    static void RegisterEvents( void );

    /** A monotonic clock in milliseconds, for the deadlines of everyone who works with the platform */
    static UInt64 GetMonotonicTimeInMs( void );

    const SEosPlatformShim& GetPlatformShim( void ) const;

    const CEosPlatformSettings& GetSettings( void ) const;

    bool IsReady( void ) const;

    /** The number of holders: every Acquire needs a Release */
    UInt32 GetRefCount( void ) const;

    /** Whether the platform was made by the glue (and is released with it), not handed in ready */
    bool WasCreatedHere( void ) const;

    /** The EOS_HPlatform behind the tables, NULL for a fake */
    void* GetPlatformHandle( void ) const;

    CEosLoginState GetLoginState( void ) const;

    /**
     *  Sets who is logged in; LoginStateChangedEvent follows when it is not what it was. Call it on the thread that
     *  pumps the pulse generator. The auth plugin does it when a login ends and when the account logs out
     */
    void SetLoginState( const CORE::CString& epicAccountId, const CORE::CString& productUserId );

    void ClearLoginState( void );

    /** Asks for the platform to be ticked at least this often until the demand is removed. The id of the demand, never 0 */
    UInt32 AddTickDemand( UInt32 intervalInMs );

    void SetTickDemandInterval( UInt32 demandId, UInt32 intervalInMs );

    void RemoveTickDemand( UInt32 demandId );

    UInt32 GetTickDemandCount( void ) const;

    /** The interval the timer runs at: the shortest of the demands, 0 when it does not run */
    UInt32 GetTickInterval( void ) const;

    /** Ticks the platform now: EOS_Platform_Tick, the P2P router, then TickEvent */
    void Tick( void );

    /**
     *  Attaches the table of the P2P interface; the router of the platform works on it. The first table counts: the
     *  tables of a platform do not change. False without a usable table or when another one is attached already
     */
    bool AttachP2P( const SEosP2PShim& shim );

    /** The router of the P2P interface, NULL without an attached table */
    CEosP2PRouter* GetP2PRouter( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CEosGlue;

    CEosPlatform( const SEosPlatformShim& shim, const CEosPlatformSettings& settings );

    virtual ~CEosPlatform();

    CEosPlatform( const CEosPlatform& src );
    CEosPlatform& operator=( const CEosPlatform& src );

    bool Create( CORE::CString& reason );

    void Shutdown( void );

    void UpdateTimer( void );

    private:

    struct SDemand
    {
        UInt32 id;
        UInt32 intervalInMs;
    };
    typedef GUCEF::vector< SDemand > TDemandVector;

    mutable MT::CMutex m_lock;
    SEosPlatformShim m_shim;
    CEosPlatformSettings m_settings;
    bool m_createdHere;
    UInt32 m_refCount;
    CEosLoginState m_loginState;
    TDemandVector m_demands;
    UInt32 m_nextDemandId;
    CORE::CTimer m_timer;
    CEosP2PRouter* m_router;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H ? */
