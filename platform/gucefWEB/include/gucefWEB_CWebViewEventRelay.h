/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWEVENTRELAY_H
#define GUCEF_WEB_CWEBVIEWEVENTRELAY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Brings the events of a view to the thread of its owner. The view notifies on the engine thread; the relay takes
 *  those events (a copy of their data) into a mailbox, whose lock is the only one it takes there, and Relay() notifies
 *  them again from this object on the thread that calls it: a GUI calls it once per frame (on the FrameStartedEvent of
 *  its context), a task thread when it looks for news. Observers subscribe to this object with the events of IWebView
 *  and find the view with GetView(). Events keep their order.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewEventRelay : public CORE::CNotifier
{
    public:

    CWebViewEventRelay( void );

    virtual ~CWebViewEventRelay();

    /** Takes the events of the view from now on; the relay keeps a reference to it. A view relayed before is let go */
    bool Attach( const TWebViewPtr& view );

    /** Lets go of the view and drops the events not relayed yet */
    void Detach( void );

    TWebViewPtr GetView( void ) const;

    /** Notifies the events that waited, on the calling thread; at most 'maxEvents' (0 all). The number relayed */
    UInt32 Relay( UInt32 maxEvents = 0 );

    bool HasEvents( void ) const;

    UInt32 GetQueuedCount( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** Subscribed to the view: puts what arrives (on the engine thread) in the mailbox, nothing else */
    class CMailObserver : public CORE::CObserver
    {
        public:

        explicit CMailObserver( MT::CTMailBox< CORE::CEvent >& mail );

        virtual ~CMailObserver();

        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

        virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

        private:

        CMailObserver( const CMailObserver& src );              /**< not implemented */
        CMailObserver& operator=( const CMailObserver& src );   /**< not implemented */

        MT::CTMailBox< CORE::CEvent >& m_mail;
    };

    CWebViewEventRelay( const CWebViewEventRelay& src );            /**< not implemented */
    CWebViewEventRelay& operator=( const CWebViewEventRelay& src ); /**< not implemented */

    private:

    MT::CTMailBox< CORE::CEvent > m_mail;
    CMailObserver m_observer;
    TWebViewPtr m_view;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWEVENTRELAY_H ? */
