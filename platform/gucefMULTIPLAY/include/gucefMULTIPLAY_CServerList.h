/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_MULTIPLAY_CSERVERLIST_H
#define GUCEF_MULTIPLAY_CSERVERLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CEVENT_H
#include "CEvent.h"
#define GUCEF_CORE_CEVENT_H
#endif /* GUCEF_CORE_CEVENT_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_COMCORE_CPING_H
#include "CPing.h"
#define GUCEF_COMCORE_CPING_H
#endif /* GUCEF_COMCORE_CPING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MULTIPLAY_CSERVERLISTPROVIDER_H
#include "gucefMULTIPLAY_CServerListProvider.h"
#define GUCEF_MULTIPLAY_CSERVERLISTPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CSERVERLISTPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

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

class GUCEF_MULTIPLAY_EXPORT_CPP CServerList : public CORE::CObservingNotifier
{
    public:
    
    typedef CServerListProvider::TServerInfo TServerInfo;
    struct SServerListEntry
    {
        TServerInfo serverInfo;            /**< server information */
        Int32 ping;                        /**< ping to the server in milliseconds */
    };
    typedef struct SServerListEntry TServerListEntry;
    typedef CORE::CTSharedPtr< TServerListEntry, MT::CMutex >   TServerListEntryPtr;
    typedef std::vector< TServerListEntryPtr >                  TServerInfoList;
    typedef std::vector< CORE::CString >                        TStringList;
        
    static const CORE::CEvent ServerListChangedEvent;    /**< event indicating that the list has changed */  
   
    static void RegisterEvents( void );

    public:
    
    CServerList( void );
    
    CServerList( const CServerList& src );
    
    virtual ~CServerList() GUCEF_VIRTUAL_OVERRIDE;
    
    CServerList& operator=( const CServerList& src );
    
    void SetList( const TServerInfoList& newList );

    const TServerInfoList& GetList( void ) const;
    
    void GetServerHostnameList( TStringList& list ) const;
    
    void GetServerAddressList( TStringList& list ) const;
    
    void GetServerNameList( TStringList& list ) const;
    
    void Clear( void );
    
    void RefreshPings( void );
    
    void StopRefreshingPings( void );
    
    bool SetParallelPingMax( const UInt32 max );
    
    UInt32 GetParallelPingMax( void ) const;
    
    bool IsBusyWithPing( void ) const;
    
    protected:
    
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL ) GUCEF_VIRTUAL_OVERRIDE;    
    
    private:
    typedef std::vector< COMCORE::CPing* > TPingers;
    
    TServerInfoList m_serverList;
    TServerInfoList m_toBePingedList;
    TPingers m_pingers;    
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CSERVERLIST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 07-04-2007 :
       - Initial implementation.

---------------------------------------------------------------------------*/
