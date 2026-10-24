/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#define GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H

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

#ifndef GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#include "gucefCOMCORE_CIDiscoveryClient.h"
#define GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#endif /* GUCEF_COMCORE_CIDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYENTRY_H
#include "gucefCOMCORE_CDiscoveryEntry.h"
#define GUCEF_COMCORE_CDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CDISCOVERYENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The bookkeeping every discovery client needs, so a protocol implementation only has to deal with its wire format:
 *  the browsed service types, the advertised entries, the discovered entries with their TTL, the events and the
 *  Find* queries. The derived class sends what the hooks ask for and hands what it hears to ProcessHeardEntry() and
 *  ProcessGoodbye().
 *
 *  The work runs on the pulse generator given at construction, by default the global one.
 */
class GUCEF_COMCORE_EXPORT_CPP CDiscoveryClientBase : public CIDiscoveryClient
{
    public:

    typedef GUCEF::vector< CIDiscoveryEntryPtr >    TIDiscoveryEntryPtrVector;

    explicit CDiscoveryClientBase( const CORE::CString& defaultClientName );

    CDiscoveryClientBase( const CORE::CString& defaultClientName      ,
                          const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CDiscoveryClientBase();

    virtual const CORE::CString& GetDiscoveryClientName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetDiscoveryClientName( const CORE::CString& clientName );

    virtual bool Browse( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StopBrowse( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsBrowsing( const CORE::CString& serviceType = CORE::CString::Empty ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Advertise( CIDiscoveryEntryPtr entry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Withdraw( CIDiscoveryEntryPtr entry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetDiscoveredEntries( const CORE::CString& serviceType ,
                                       TIDiscoveryEntrySet& entries     ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindCapability( const CORE::CString& capabilityId ,
                                 TIDiscoveryEntrySet& entries      ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindApplication( const CORE::CString& applicationId ,
                                  TIDiscoveryEntrySet& entries       ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindHost( const COMCORE::CHostAddress& host ,
                           TIDiscoveryEntrySet& entries      ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Withdraws every advertised entry and stops every browse */
    virtual void StopAll( void );

    /**
     *  Reads the settings common to all clients: name, housekeepingIntervalInMs
     *  Derived classes call this from their own LoadConfig()
     */
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    const CORE::PulseGeneratorPtr& GetPulseGenerator( void ) const;

    /** A monotonic clock in milliseconds */
    static UInt64 GetTimeInMs( void );

    protected:

    /** A browse for the service type started: send a query */
    virtual bool OnBrowseStarted( const CORE::CString& serviceType ) = 0;

    /** The browse for the service type stopped */
    virtual void OnBrowseStopped( const CORE::CString& serviceType );

    /** An entry is to be advertised: announce it */
    virtual bool OnAdvertised( const CIDiscoveryEntryPtr& entry ) = 0;

    /** An entry is withdrawn: say goodbye for it */
    virtual bool OnWithdrawn( const CIDiscoveryEntryPtr& entry ) = 0;

    /** Called on every housekeeping cycle, for example to announce again */
    virtual void OnHousekeeping( UInt64 nowInMs );

    /**
     *  Records an entry that was heard on the network: new entries are reported as discovered, changed ones as
     *  updated, unchanged ones only get their TTL renewed. Entries of service types that are not browsed and
     *  entries this client advertises itself are ignored.
     *
     *  @return false if this object was destroyed by an observer of the events, the caller must return right away
     */
    bool ProcessHeardEntry( const CIDiscoveryEntry& heard );

    /**
     *  An advertiser withdrew the given entry: it is reported as lost
     *
     *  @return false if this object was destroyed by an observer of the events, the caller must return right away
     */
    bool ProcessGoodbye( const CORE::CString& serviceType, const CORE::CString& instanceId );

    /** @return false if this object was destroyed by an observer of the event */
    bool ReportDiscoveryError( const CORE::CString& description );

    /** Whether entries of the given service type are being looked for */
    bool IsServiceTypeBrowsed( const CORE::CString& serviceType ) const;

    /** Whether the given key belongs to an entry this client advertises */
    bool IsOwnEntry( const CORE::CString& entryKey ) const;

    /** Copies of the pointers of the advertised entries, safe to use while advertising changes */
    void GetAdvertisedEntries( TIDiscoveryEntryPtrVector& entries ) const;

    /** The browsed service types */
    void GetBrowsedServiceTypes( CORE::CString::StringSet& serviceTypes ) const;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SDiscoveredEntry
    {
        CDiscoveryEntryPtr entry;
        UInt64 expiresAtInMs;
    };
    typedef struct SDiscoveredEntry TDiscoveredEntry;
    typedef GUCEF::map< CORE::CString, TDiscoveredEntry >       TDiscoveredEntryMap;
    typedef GUCEF::map< CORE::CString, CIDiscoveryEntryPtr >    TAdvertisedEntryMap;

    CDiscoveryClientBase( void );
    CDiscoveryClientBase( const CDiscoveryClientBase& src );
    CDiscoveryClientBase& operator=( const CDiscoveryClientBase& src );

    void UpdateHousekeepingTimer( void );
    bool ExpireEntries( UInt64 nowInMs );

    private:

    CORE::CString m_clientName;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CORE::CTimer m_housekeepingTimer;
    CORE::CString::StringSet m_browsedServiceTypes;
    TAdvertisedEntryMap m_advertised;
    TDiscoveredEntryMap m_discovered;
    mutable MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H ? */
