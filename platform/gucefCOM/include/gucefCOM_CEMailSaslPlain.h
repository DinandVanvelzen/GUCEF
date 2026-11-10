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

#ifndef GUCEF_COM_CEMAILSASLPLAIN_H
#define GUCEF_COM_CEMAILSASLPLAIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

#ifndef GUCEF_COM_CIEMAILSASLMECHANISM_H
#include "gucefCOM_CIEMailSaslMechanism.h"
#define GUCEF_COM_CIEMAILSASLMECHANISM_H
#endif /* GUCEF_COM_CIEMAILSASLMECHANISM_H ? */

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

/** PLAIN, RFC 4616: NUL user NUL password */
class GUCEF_COM_PUBLIC_CPP CEMailSaslPlain : public CIEMailSaslMechanism
{
    public:

    CEMailSaslPlain( const CORE::CString& userName, const CORE::CString& password );
    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool HasInitialResponse( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RevealsSecret( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Start( CORE::CDynamicBuffer& initialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EMailSaslState Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_userName;
    CORE::CString m_password;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILSASLPLAIN_H ? */
