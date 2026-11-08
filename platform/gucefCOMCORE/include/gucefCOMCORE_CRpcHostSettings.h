/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCHOSTSETTINGS_H
#define GUCEF_COMCORE_CRPCHOSTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H
#include "gucefCOMCORE_CRpcEndpointSettings.h"
#define GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H
#endif /* GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H ? */

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
 *  Settings of an RPC host (gucefCOMCORE RPC design 10.5): the listeners (URI plus listener configuration), the
 *  settings of the endpoints it creates, and session limits.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcHostSettings : public CORE::CIConfigurable
{
    public:

    struct GUCEF_COMCORE_EXPORT_CPP SListener
    {
        CORE::CUri uri;                 /**< "tcp://127.0.0.1:47300", "loopback://notes", "drn://session" */
        CORE::CDataNode config;         /**< handed to the listener's LoadConfig() */

        SListener( void );
    };
    typedef GUCEF::vector< SListener > TListenerVector;

    TListenerVector listeners;
    CRpcEndpointSettings endpointSettings;
    UInt32 maxSessions;                 /**< 0 = no maximum */
    UInt32 maxSessionsPerSubject;       /**< 0 = no maximum */
    UInt32 helloTimeoutMs;              /**< an accepted transport must send HELLO within this, 5000 */

    CRpcHostSettings( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCHOSTSETTINGS_H ? */
