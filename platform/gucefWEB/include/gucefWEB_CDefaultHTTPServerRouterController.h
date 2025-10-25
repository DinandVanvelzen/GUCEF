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

#ifndef GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H
#define GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_WEB_CIHTTPSERVERROUTERCONTROLLER_H
#include "gucefWEB_CIHTTPServerRouterController.h"
#define GUCEF_WEB_CIHTTPSERVERROUTERCONTROLLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERROUTERCONTROLLER_H ? */

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

class CHTTPServer;

class GUCEF_WEB_PUBLIC_CPP CDefaultHTTPServerRouterController : public CIHTTPServerRouterController
{   
    public:
    
    CDefaultHTTPServerRouterController( void );
    
    virtual ~CDefaultHTTPServerRouterController();
    
    virtual CIHTTPServerRouter* GetHandler( const CString& uri );
    
    virtual CString GetUriAfterTheBaseAddress( CIHTTPServerRouter& router , 
                                               const CString& fullUri     );
    
    virtual CString MakeUriAbsolute( CIHTTPServerRouter& router      ,
                                     const CString& uriWithAuthority ,
                                     const CString& uriSuffix        );
    
    virtual void AddRouterMapping( CIHTTPServerRouter* router    ,
                                   const CString& uriBase        ,
                                   const CString& serviceRootUri );
    
    virtual void RemoveRouterMapping( const CString& uriBase );
    
    private:

    CDefaultHTTPServerRouterController( const CDefaultHTTPServerRouterController& src );            /**< not implemented */
    CDefaultHTTPServerRouterController& operator=( const CDefaultHTTPServerRouterController& src ); /**< not implemented */

    CString GetUriPathFromFullUri( const CString& uri ) const;
    
    private:
    
    typedef GUCEF::map< CString, CIHTTPServerRouter* > TRouterMap;
    
    TRouterMap m_routerMap;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 03-03-2007 :
        - Dinand: Added this section

---------------------------------------------------------------------------*/
