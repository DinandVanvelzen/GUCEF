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

#ifndef GUCEF_MULTIPLAY_CIMATCHALERTSINK_H
#define GUCEF_MULTIPLAY_CIMATCHALERTSINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHALERT_H
#include "gucefMULTIPLAY_CMatchAlert.h"
#define GUCEF_MULTIPLAY_CMATCHALERT_H
#endif /* GUCEF_MULTIPLAY_CMATCHALERT_H ? */

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
 *  Delivers a match alert to the player: in the application (CInAppMatchAlertSink), in the desktop or on the
 *  phone (sinks of other modules and plugins), by mail (server side). The router (CMatchAlertRouter) decides
 *  what reaches which sink.
 *
 *  A sink that interrupts the player (a pop up of the desktop, a notification of the phone) is subject to
 *  quiet hours, coalescing, the minimum interval and the modes of the router. A sink that does not interrupt (the toasts
 *  and the counter inside the application) gets every match alert of a match that is not muted.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIMatchAlertSink
{
    public:

    CIMatchAlertSink( void );

    virtual ~CIMatchAlertSink();

    virtual void Deliver( const CMatchAlert& alert ) = 0;

    /** Takes back what was shown for the match (the match was opened, or played elsewhere) */
    virtual void Clear( const CORE::CString& matchId ) = 0;

    /** Whether the sink interrupts the player: see the class description */
    virtual bool IsInterrupting( void ) const = 0;

    /** The kind of sink, for example "in-app" or "desktop" */
    virtual const CORE::CString& GetSinkName( void ) const = 0;

    private:

    CIMatchAlertSink( const CIMatchAlertSink& src );
    CIMatchAlertSink& operator=( const CIMatchAlertSink& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIMatchAlertSink, MT::CMutex >    CMatchAlertSinkFactory;
typedef CMatchAlertSinkFactory::TProductPtr                                       CIMatchAlertSinkPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CIMATCHALERTSINK_H ? */
