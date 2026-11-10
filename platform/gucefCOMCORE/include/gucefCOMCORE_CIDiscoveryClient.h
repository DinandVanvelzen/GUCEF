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

#ifndef GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#define GUCEF_COMCORE_CIDISCOVERYCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CITYPENAMED_H
#include "CITypeNamed.h"
#define GUCEF_CORE_CITYPENAMED_H
#endif /* GUCEF_CORE_CITYPENAMED_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERY_H
#include "gucefCOMCORE_CIDiscovery.h"
#define GUCEF_COMCORE_CIDISCOVERY_H
#endif /* GUCEF_COMCORE_CIDISCOVERY_H ? */

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
 *  The data of the entry events: the entry
 */
typedef CIDiscoveryEntryPtr CDiscoveryEntryEventData;

/*-------------------------------------------------------------------------*/

/**
 *  Interface class which serves as the base class for all discovery clients
 *
 *  A client browses for service types and advertises entries. What it finds it reports with events.
 *  An entry is lost when its TTL runs out without being heard again, or when its advertiser withdraws it.
 */
class GUCEF_COMCORE_EXPORT_CPP CIDiscoveryClient : public CORE::CObservingNotifier     ,
                                                   public virtual CORE::CIConfigurable ,
                                                   public virtual CIDiscovery
{
    public:

    static const CORE::CEvent EntryDiscoveredEvent;     /**< CDiscoveryEntryEventData: an entry was found */
    static const CORE::CEvent EntryUpdatedEvent;        /**< CDiscoveryEntryEventData: a known entry was heard with changed content */
    static const CORE::CEvent EntryLostEvent;           /**< CDiscoveryEntryEventData: the TTL ran out or the advertiser withdrew it */
    static const CORE::CEvent BrowseStartedEvent;       /**< TBrowseEventData: the service type */
    static const CORE::CEvent BrowseStoppedEvent;       /**< TBrowseEventData: the service type */
    static const CORE::CEvent DiscoveryErrorEvent;      /**< TDiscoveryErrorEventData: a description of the problem */

    typedef CORE::TCloneableString  TBrowseEventData;
    typedef CORE::TCloneableString  TDiscoveryErrorEventData;

    /** Browsing for this service type accepts every service type */
    static const CORE::CString AnyServiceType;

    static void RegisterEvents( void );

    CIDiscoveryClient( void );

    CIDiscoveryClient( const CIDiscoveryClient& src );

    virtual ~CIDiscoveryClient();

    CIDiscoveryClient& operator=( const CIDiscoveryClient& src );

    virtual const CORE::CString& GetDiscoveryClientName( void ) const = 0;

    /**
     *  Starts looking for entries of the given service type. Found entries are reported with events
     */
    virtual bool Browse( const CORE::CString& serviceType ) = 0;

    virtual bool StopBrowse( const CORE::CString& serviceType ) = 0;

    /**
     *  @param serviceType the service type to ask about, empty asks whether any browse is active
     */
    virtual bool IsBrowsing( const CORE::CString& serviceType = CORE::CString::Empty ) const = 0;

    /**
     *  Starts advertising the given entry until it is withdrawn
     */
    virtual bool Advertise( CIDiscoveryEntryPtr entry ) = 0;

    /**
     *  Stops advertising the given entry, telling the browsers it is gone where the protocol allows it
     */
    virtual bool Withdraw( CIDiscoveryEntryPtr entry ) = 0;

    /**
     *  Adds the entries currently known for the given service type to the given set
     */
    virtual bool GetDiscoveredEntries( const CORE::CString& serviceType ,
                                       TIDiscoveryEntrySet& entries     ) const = 0;

    /** The same operation as Advertise() */
    virtual bool Register( CIDiscoveryEntryPtr entry ) GUCEF_VIRTUAL_OVERRIDE;

    /** The same operation as Withdraw() */
    virtual bool Unregister( CIDiscoveryEntryPtr entry ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTBasicSharedPtr< CIDiscoveryClient, MT::CMutex > CIDiscoveryClientPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIDISCOVERYCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

---------------------------------------------------------------------------*/
