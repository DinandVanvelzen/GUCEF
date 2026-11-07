/*
 *  gucefWEB: GUCEF module providing Web application functionality 
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

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#define GUCEF_WEB_CWEBGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"      /* often used gucefCOM macros */
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#include "gucefWEB_CIHttpClientBackend.h"
#define GUCEF_WEB_CIHTTPCLIENTBACKEND_H
#endif /* GUCEF_WEB_CIHTTPCLIENTBACKEND_H ? */

#ifndef GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H
#include "gucefWEB_CIMcpHttpAuthenticator.h"
#define GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H
#endif /* GUCEF_WEB_CIMCPHTTPAUTHENTICATOR_H ? */

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

class CGlobalHttpCodecLinks;

/*-------------------------------------------------------------------------*/

class GUCEF_WEB_PUBLIC_CPP CWebGlobal
{
    public:

    static CWebGlobal* Instance( void );

    CGlobalHttpCodecLinks& GetGlobalHttpCodecLinks( void ) const;

    /**
     *  The factory for HTTP client backends, keyed by backend name. Plugins register their backends here,
     *  the platform registers the "native" backend.
     */
    THttpClientBackendFactory& GetHttpClientBackendFactory( void ) const;

    /**
     *  The factory for the authenticators of the Streamable HTTP transport of MCP, keyed by name. Built in are
     *  "static-bearer" and "none"; other modules register theirs here.
     */
    TMcpHttpAuthenticatorFactory& GetMcpHttpAuthenticatorFactory( void ) const;

    private:
    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    CWebGlobal( void );    
    ~CWebGlobal();

    CWebGlobal( const CWebGlobal& src );            /** <- not implemented */
    CWebGlobal& operator=( const CWebGlobal& src ); /** <- not implemented */

    private:

    static MT::CMutex g_dataLock;
    static CWebGlobal* g_instance;
    CGlobalHttpCodecLinks* m_globalHttpCodecLinks;
    THttpClientBackendFactory* m_httpClientBackendFactory;
    TMcpHttpAuthenticatorFactory* m_mcpHttpAuthenticatorFactory;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

-----------------------------------------------------------------------------*/
