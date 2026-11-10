/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CINAPPMATCHALERTSINK_H
#define GUCEF_MULTIPLAY_CINAPPMATCHALERTSINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_MULTIPLAY_CIMATCHALERTSINK_H
#include "gucefMULTIPLAY_CIMatchAlertSink.h"
#define GUCEF_MULTIPLAY_CIMATCHALERTSINK_H
#endif /* GUCEF_MULTIPLAY_CIMATCHALERTSINK_H ? */

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
 *  The match alerts inside the application: the toast and the counter of the user interface. It keeps the
 *  pending match alerts (one per match, the newest revision) and tells the interface by events when one is added
 *  or taken away and when the count changes. It does not interrupt: it is always on.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CInAppMatchAlertSink : public CORE::CObservingNotifier ,
                                                          public CIMatchAlertSink
{
    public:

    static const CORE::CEvent MatchAlertAddedEvent;       /**< TMatchAlertAddedEventData: show a toast */
    static const CORE::CEvent MatchAlertClearedEvent;     /**< TMatchAlertClearedEventData: the match id of a match alert that was taken away */
    static const CORE::CEvent PendingCountChangedEvent;     /**< TPendingCountEventData: the number of pending match alerts, for the counter */

    typedef CMatchAlert                   TMatchAlertAddedEventData;
    typedef CORE::TCloneableString          TMatchAlertClearedEventData;
    typedef CORE::TCloneableUInt32          TPendingCountEventData;

    static const UInt32 MaxPending = 256;

    static void RegisterEvents( void );

    CInAppMatchAlertSink( void );

    virtual ~CInAppMatchAlertSink();

    virtual void Deliver( const CMatchAlert& alert ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Clear( const CORE::CString& matchId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsInterrupting( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetSinkName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The pending match alerts, the oldest first */
    void GetPending( TMatchAlertVector& pending ) const;

    UInt32 GetPendingCount( void ) const;

    /** Takes all pending match alerts away (the player looked at the list) */
    void ClearAll( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CInAppMatchAlertSink( const CInAppMatchAlertSink& src );
    CInAppMatchAlertSink& operator=( const CInAppMatchAlertSink& src );

    void RaiseCount( void );

    private:

    TMatchAlertVector m_pending;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CINAPPMATCHALERTSINK_H ? */
