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

#ifndef GUCEF_COM_CSTATICEMAILCREDENTIALPROVIDER_H
#define GUCEF_COM_CSTATICEMAILCREDENTIALPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIEMAILCREDENTIALPROVIDER_H
#include "gucefCOM_CIEMailCredentialProvider.h"
#define GUCEF_COM_CIEMAILCREDENTIALPROVIDER_H
#endif /* GUCEF_COM_CIEMAILCREDENTIALPROVIDER_H ? */

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

/** A fixed user name and password or token */
class GUCEF_COM_PUBLIC_CPP CStaticEMailCredentialProvider : public CIEMailCredentialProvider
{
    public:

    CStaticEMailCredentialProvider( void );

    CStaticEMailCredentialProvider( EMailCredentialType type, const CORE::CString& userName, const CORE::CString& secret );

    virtual ~CStaticEMailCredentialProvider() GUCEF_VIRTUAL_OVERRIDE;

    void SetCredential( EMailCredentialType type, const CORE::CString& userName, const CORE::CString& secret );

    virtual EResult GetCredential( const CEMailCredentialRequest& request, CEMailCredential& credential ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReportRejected( const CEMailCredentialRequest& request ) GUCEF_VIRTUAL_OVERRIDE;

    /** How often the server refused the credential */
    UInt32 GetRejectionCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CEMailCredential m_credential;
    UInt32 m_rejectionCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSTATICEMAILCREDENTIALPROVIDER_H ? */
