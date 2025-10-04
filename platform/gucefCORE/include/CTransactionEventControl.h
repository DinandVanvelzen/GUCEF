/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CTRANSACTIONEVENTCONTROL_H
#define GUCEF_CORE_CTRANSACTIONEVENTCONTROL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_CORE_CFORWARDINGNOTIFIER_H
#include "gucefCORE_CForwardingNotifier.h"
#define GUCEF_CORE_CFORWARDINGNOTIFIER_H
#endif /* GUCEF_CORE_CFORWARDINGNOTIFIER_H ? */

#include "gucefCORE_macros.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  This is a building block that can be used internally by classes that handle and/or
 *  emit events.
 *
 *  Sometimes a class performes actions on sub-system(s) that trigger events
 *  to which the class in question is actually subscribed. The intermediate events
 *  emitted might not be of any interest and/or should not be handled untill the
 *  transaction with the sub-system(s) is finished. This class can be your tool
 *  for solving those types of problems.
 *  Define your interactions with sub-system(s) as transactions and add filters
 *  to achieve the desired behaviour.
 */
class GUCEF_CORE_PUBLIC_CPP CTransactionEventControl : public CForwardingNotifier
{
    public:

    enum EEventFilterAction
    {
        EVENTFILTER_DELETE_EVENT ,   /**< mainly usefull if you listnen to all events of an object but want to exclude a few specific ones */
        EVENTFILTER_GROUP_EVENT      /**< this will group events of the same type as 1 event, updating to the latest occurance */
    };
    typedef enum EEventFilterAction TEventFilterAction;

    public:

    CTransactionEventControl( void );

    virtual ~CTransactionEventControl();

    void StartTransaction( void );

    void FinishTransaction( void );

    bool AddEventFilter( const CEvent& eventID                 ,
                         const TEventFilterAction filterAction ,
                         const TEventOriginFilter originFilter ,
                         const bool preserveEventData          ,
                         const CNotifier* sender = NULL        );

    bool RemoveEventFilter( const CEvent& eventID           ,
                            const CNotifier* sender = NULL  );

    bool IsEventFiltered( const CEvent& eventID   ,
                          const CNotifier* sender ) const;

    bool GetFilterInfo( const CEvent& eventID            ,
                        const CNotifier* sender          ,
                        TEventFilterAction& filterAction ,
                        TEventOriginFilter& originFilter ,
                        bool& preserveEventData          ) const;

    bool IsTransactionActive( void ) const;

    void ClearAllFilters( void );

    protected:

    virtual void OnNotify( CNotifier* notifier           ,
                           const CEvent& eventid         ,
                           CICloneable* eventdata = NULL );

    private:

    struct SFilterData
    {
        TEventFilterAction filterAction;
        TEventOriginFilter originFilter;
        bool preserveEventData;
    };
    typedef struct SFilterData TFilterData;

    typedef GUCEF::map< CEvent, TFilterData > TFilterMap;
    typedef GUCEF::map< const CNotifier*, TFilterMap > TSenderFilterMap;

    private:

    bool ApplyFilter( const TFilterMap& filterMap ,
                      CNotifier* notifier         ,
                      const CEvent& eventID       ,
                      CICloneable* eventData      );

    CTransactionEventControl( const CTransactionEventControl& src );             /**< not implemented */
    CTransactionEventControl& operator=( const CTransactionEventControl& src );  /**< not implemented */

    private:

    TFilterMap m_filterMap;
    TSenderFilterMap m_senderFilterMap;
    MT::CTMailBox< CEvent > m_mailbox;
    MT::CMutex m_mutex;
    bool m_transactionBusy;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTRANSACTIONEVENTCONTROL_H ? */
