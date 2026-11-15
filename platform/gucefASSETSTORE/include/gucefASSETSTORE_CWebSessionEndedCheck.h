/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONENDEDCHECK_H
#define GUCEF_ASSETSTORE_CWEBSESSIONENDEDCHECK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIWEBSESSIONENDEDCHECK_H
#include "gucefASSETSTORE_CIWebSessionEndedCheck.h"
#define GUCEF_ASSETSTORE_CIWEBSESSIONENDEDCHECK_H
#endif /* GUCEF_ASSETSTORE_CIWEBSESSIONENDEDCHECK_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H
#include "gucefASSETSTORE_CWebLoginScript.h"
#define GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H
#endif /* GUCEF_ASSETSTORE_CWEBLOGINSCRIPT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The default CIWebSessionEndedCheck: the session ended when the store answers with one of the status codes (401 by
 *  default), or redirects (a 3xx with a Location) to an address that matches the redirect pattern, such as its login
 *  page. Configure() takes both from the `sessionEnded` part of a login script.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CWebSessionEndedCheck : public CIWebSessionEndedCheck
{
    public:

    CWebSessionEndedCheck( void );
    virtual ~CWebSessionEndedCheck() GUCEF_VIRTUAL_OVERRIDE;

    /** The status codes and the redirect pattern of the script; 401 when it gives no status codes */
    void Configure( const CWebLoginScript& script );

    void SetStatusCodes( const CWebLoginScript::TUInt32Vector& statusCodes );
    const CWebLoginScript::TUInt32Vector& GetStatusCodes( void ) const;

    /** The pattern (* for any text) of an address a redirect goes to when the session ended; empty for none */
    void SetRedirectPattern( const CORE::CString& pattern );
    const CORE::CString& GetRedirectPattern( void ) const;

    virtual bool IsSessionEnded( const WEB::CHttpResponseData& response ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWebLoginScript::TUInt32Vector m_statusCodes;
    CORE::CString m_redirectPattern;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBSESSIONENDEDCHECK_H ? */
