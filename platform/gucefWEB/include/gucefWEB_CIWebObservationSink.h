/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CIWEBOBSERVATIONSINK_H
#define GUCEF_WEB_CIWEBOBSERVATIONSINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_COBSERVEDWEBREQUEST_H
#include "gucefWEB_CObservedWebRequest.h"
#define GUCEF_WEB_COBSERVEDWEBREQUEST_H
#endif /* GUCEF_WEB_COBSERVEDWEBREQUEST_H ? */

#ifndef GUCEF_WEB_COBSERVEDUSERACTION_H
#include "gucefWEB_CObservedUserAction.h"
#define GUCEF_WEB_COBSERVEDUSERACTION_H
#endif /* GUCEF_WEB_COBSERVEDUSERACTION_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

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
 *  Receives what site discovery observed: from CChromeNetworkRecorder while it records, and from
 *  CSiteDiscoveryJournal::Load() when a journal is read back. CSiteEndpointCatalog and CSiteDiscoveryJournal are sinks;
 *  a program adds a sink of its own to see the observations as they come in.
 */
class GUCEF_WEB_PUBLIC_CPP CIWebObservationSink
{
    public:

    CIWebObservationSink( void );

    virtual ~CIWebObservationSink();

    /** A request that finished, failed or was redirected */
    virtual void OnWebRequestObserved( const CObservedWebRequest& observation ) = 0;

    /** A user action, before the requests that follow it. Nothing by default */
    virtual void OnUserActionObserved( const CObservedUserAction& action );

    /**
     *  Asked before the response of a data request is fetched to describe its shape: fetching costs a command to the
     *  browser, so it is only done when a sink wants it, for example for the first response of an endpoint. false by default
     */
    virtual bool WantsResponseShape( const CObservedWebRequest& observation ) const;

    /**
     *  Asked of a sink that wants a shape, once the shape of the response is described: does it add something to what
     *  the sink knows of the endpoint (a path, a type of a path, an array that was empty before and has elements now)?
     *  Only then the shape goes with the observation to every sink and into the journal line. true by default
     */
    virtual bool IsNewResponseShape( const CObservedWebRequest& observation, const CWebBodyShape& shape ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIWEBOBSERVATIONSINK_H ? */
