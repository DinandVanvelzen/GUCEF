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

#ifndef GUCEF_COMCORE_CDISCOVERYMANAGER_H
#define GUCEF_COMCORE_CDISCOVERYMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERYENTRY_H
#include "gucefCOMCORE_CIDiscoveryEntry.h"
#define GUCEF_COMCORE_CIDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CIDISCOVERYENTRY_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#include "gucefCOMCORE_CIDiscoveryClient.h"
#define GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#endif /* GUCEF_COMCORE_CIDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERY_H
#include "gucefCOMCORE_CIDiscovery.h"
#define GUCEF_COMCORE_CIDISCOVERY_H
#endif /* GUCEF_COMCORE_CIDISCOVERY_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* often used gucefCOMCORE macros */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

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
 *  Combines the registered discovery clients into one: browses and advertisements go to every client, and what
 *  the clients find is published again with the same events as CIDiscoveryClient uses, with the entries merged
 *  by their key (service type, instance id). An entry is lost once no client knows it anymore.
 *
 *  LoadConfig() creates the clients its configuration names through the discovery client factory of
 *  CComCoreGlobal, for example:
 *      <DiscoveryManager>
 *          <DiscoveryClient type="mdns" name="lan-mdns" />
 *      </DiscoveryManager>
 *  Every attribute and child of a DiscoveryClient node is handed to the LoadConfig() of the client it creates.
 */
class GUCEF_COMCORE_EXPORT_CPP CDiscoveryManager : public CORE::CObservingNotifier            ,
                                                   public virtual CORE::CGloballyConfigurable ,
                                                   public virtual CIDiscovery
{
    public:

    typedef GUCEF::set< CORE::CString > TStringSet;
    typedef GUCEF::set< CIDiscoveryClientPtr > TDiscoveryClientSet;
    typedef GUCEF::vector< CIDiscoveryClientPtr > TDiscoveryClientVector;

    /**
     *  Adds the client: it gets the active browses and advertisements and its findings are merged in
     */
    bool RegisterDiscoveryClient( CIDiscoveryClientPtr discoveryClient );

    bool UnregisterDiscoveryClient( const CORE::CString& discoveryClientName );

    /**
     *  Unregisters the clients that were created through the factory with the given type, for example when the
     *  module that provides the type goes away
     *
     *  @return the number of clients unregistered
     */
    UInt32 UnregisterDiscoveryClientsOfType( const CORE::CString& clientType );

    CIDiscoveryClientPtr GetDiscoveryClient( const CORE::CString& discoveryClientName ) const;

    void GetDiscoveryClients( TDiscoveryClientVector& clients ) const;

    /**
     *  Creates a client of the given type through the discovery client factory, configures it with the given
     *  config when there is one and registers it
     */
    CIDiscoveryClientPtr CreateDiscoveryClient( const CORE::CString& clientType         ,
                                                const CORE::CDataNode* config = GUCEF_NULL );

    bool Browse( const CORE::CString& serviceType );

    bool StopBrowse( const CORE::CString& serviceType );

    bool IsBrowsing( const CORE::CString& serviceType = CORE::CString::Empty ) const;

    bool Advertise( CIDiscoveryEntryPtr entry );

    bool Withdraw( CIDiscoveryEntryPtr entry );

    /** The merged entries currently known for the given service type, empty for all */
    bool GetDiscoveredEntries( const CORE::CString& serviceType ,
                               TIDiscoveryEntrySet& entries     ) const;

    virtual bool FindCapability( const CORE::CString& capabilityId ,
                                 TIDiscoveryEntrySet& entries      ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindApplication( const CORE::CString& applicationId ,
                                 TIDiscoveryEntrySet& entries        ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindHost( const COMCORE::CHostAddress& host ,
                           TIDiscoveryEntrySet& entries      ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The same operation as Advertise() */
    virtual bool Register( CIDiscoveryEntryPtr entry ) GUCEF_VIRTUAL_OVERRIDE;

    /** The same operation as Withdraw() */
    virtual bool Unregister( CIDiscoveryEntryPtr entry ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Saves a DiscoveryManager node with a DiscoveryClient node per client
     */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Looks for the DiscoveryManager node, which is either the given node or a child of it, and creates and
     *  configures the clients its DiscoveryClient nodes name. A client that is already registered under the
     *  same name is configured again instead.
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDiscoveryManager( const CDiscoveryManager& src );            /** not implemented */
    CDiscoveryManager& operator=( const CDiscoveryManager& src ); /** not implemented */

    private:
    friend class CComCoreGlobal;

    CDiscoveryManager( void );
    ~CDiscoveryManager();

    bool OnClientEntryFound( const CORE::CString& clientName, const CIDiscoveryEntryPtr& entry );
    bool OnClientEntryLost( const CORE::CString& clientName, const CIDiscoveryEntryPtr& entry );
    bool ForgetClientEntries( const CORE::CString& clientName );

    private:

    struct SMergedEntry
    {
        CIDiscoveryEntryPtr entry;
        TStringSet clientNames;     /**< the clients that currently know the entry */
    };
    typedef struct SMergedEntry TMergedEntry;
    typedef GUCEF::map< CORE::CString, TMergedEntry >           TMergedEntryMap;
    typedef GUCEF::map< CORE::CString, CIDiscoveryEntryPtr >    TAdvertisedEntryMap;
    typedef GUCEF::map< CORE::CString, CORE::CString >          TStringMap;
    typedef GUCEF::vector< CIDiscoveryEntryPtr >                TEntryVector;

    TDiscoveryClientSet m_clients;
    TStringMap m_clientTypes;               /**< client name -> the factory type it was created with */
    TStringSet m_browsedServiceTypes;
    TAdvertisedEntryMap m_advertised;
    TMergedEntryMap m_merged;
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

#endif /* GUCEF_COMCORE_CDISCOVERYMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

---------------------------------------------------------------------------*/
