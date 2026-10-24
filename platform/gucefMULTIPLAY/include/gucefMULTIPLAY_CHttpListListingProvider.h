/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H
#define GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H
#include "gucefMULTIPLAY_CPassiveURLServerListProvider.h"
#define GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CIGameListingProvider.h"
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Lists the servers of a list a URL gives (XML or JSON, see CPassiveURLServerListProvider).
 *
 *  Every server entry becomes a listing with the id "http:<host>:<port>" and the endpoint
 *  "<endpointScheme>://<host>:<port>". The attributes of the entry become the extra values; those named like the
 *  LAN attributes (kind, game, ver, map, mode, pl, max, spec, pw) also fill the listing fields.
 *  A refresh drops the listings the list no longer has.
 *
 *  Config: url, codec (default "xml"), endpointScheme (default "udp"), refreshIntervalInSecs (0 for none)
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CHttpListListingProvider : public CIGameListingProvider
{
    public:

    static const CORE::CString ProviderName;        /**< "http-list" */

    CHttpListListingProvider( void );

    virtual ~CHttpListListingProvider();

    virtual bool Start( const CGameListingFilter& filter ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetUrl( const CORE::CString& url );

    const CORE::CString& GetUrl( void ) const;

    void SetCodec( const CORE::CString& codec );

    void SetEndpointScheme( const CORE::CString& scheme );

    void SetRefreshIntervalInSecs( UInt32 seconds );

    /** Makes the listing of a server list entry */
    static void ServerInfoToListing( const CServerListProvider::TServerInfo& info, const CORE::CString& endpointScheme, CGameListingEntry& listing );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CHttpListListingProvider( const CHttpListListingProvider& src );
    CHttpListListingProvider& operator=( const CHttpListListingProvider& src );

    bool StartRetrieval( void );
    void ScheduleRefresh( void );

    private:

    CPassiveURLServerListProvider m_list;
    CORE::CTimer m_refreshTimer;
    CORE::CString m_url;
    CORE::CString m_codec;
    CORE::CString m_endpointScheme;
    UInt32 m_refreshIntervalInSecs;
    bool m_active;
    CORE::CString::StringSet m_seenThisRound;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CIGameListingProvider, CHttpListListingProvider, MT::CMutex > THttpListListingProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H ? */
