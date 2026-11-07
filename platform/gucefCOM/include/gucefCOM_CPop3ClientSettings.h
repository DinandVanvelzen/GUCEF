/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CPOP3CLIENTSETTINGS_H
#define GUCEF_COM_CPOP3CLIENTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMAILCLIENTSETTINGS_H
#include "gucefCOM_CMailClientSettings.h"
#define GUCEF_COM_CMAILCLIENTSETTINGS_H
#endif /* GUCEF_COM_CMAILCLIENTSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Settings of the POP3 client: POP3 (RFC 1939, 2449, 2595, 5034). Ports: 110 with STLS, 995 with implicit TLS. */
class GUCEF_COM_PUBLIC_CPP CPop3ClientSettings : public CMailClientSettings
{
    public:

    bool useApop;                                   /**< APOP (MD5 challenge) when the server offers it and no better mechanism is possible. Only with allowInsecureAuth */
    bool useTop;                                    /**< TOP is used to read headers when the server has it */
    UInt32 maxMessagesPerSession;                   /**< the most messages one fetch operation takes in, 0: no limit */

    CPop3ClientSettings( void );

    CPop3ClientSettings( const CPop3ClientSettings& src );

    virtual ~CPop3ClientSettings() GUCEF_VIRTUAL_OVERRIDE;

    CPop3ClientSettings& operator=( const CPop3ClientSettings& src );

    virtual UInt16 GetDefaultPort( EMailTlsMode mode ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CPOP3CLIENTSETTINGS_H ? */
