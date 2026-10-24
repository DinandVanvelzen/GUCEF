/*
 *  gucefCOM: GUCEF module providing communication implementations 
 *  for standardized protocols
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

#ifndef GUCEF_COM_CCOMGLOBAL_H
#define GUCEF_COM_CCOMGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

#ifndef GUCEF_COM_CIIRCTRANSPORT_H
#include "gucefCOM_CIIrcTransport.h"
#define GUCEF_COM_CIIRCTRANSPORT_H
#endif /* GUCEF_COM_CIIRCTRANSPORT_H ? */

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

class CGlobalHttpCodecLinks;

/*-------------------------------------------------------------------------*/

class GUCEF_COM_PUBLIC_CPP CComGlobal
{
    public:

    static CComGlobal* Instance( void );

    /** Creates IRC transports by name: "tcp" here, "websocket" when gucefWEB registered it */
    CIrcTransportFactory& GetIrcTransportFactory( void );

    private:
    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    CComGlobal( void );    
    ~CComGlobal();

    CComGlobal( const CComGlobal& src );            /** <- not implemented */
    CComGlobal& operator=( const CComGlobal& src ); /** <- not implemented */

    private:

    CIrcTransportFactory* m_ircTransportFactory;

    static MT::CMutex g_dataLock;
    static CComGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

---------------------------------------------------------------------------*/
