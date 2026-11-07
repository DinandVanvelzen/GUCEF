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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/


#ifndef GUCEF_COM_CEMAILSASLPLAIN_H
#include "gucefCOM_CEMailSaslPlain.h"
#define GUCEF_COM_CEMAILSASLPLAIN_H
#endif /* GUCEF_COM_CEMAILSASLPLAIN_H ? */

#ifndef GUCEF_COM_CEMAILSASLLOGIN_H
#include "gucefCOM_CEMailSaslLogin.h"
#define GUCEF_COM_CEMAILSASLLOGIN_H
#endif /* GUCEF_COM_CEMAILSASLLOGIN_H ? */

#ifndef GUCEF_COM_CEMAILSASLXOAUTH2_H
#include "gucefCOM_CEMailSaslXOAuth2.h"
#define GUCEF_COM_CEMAILSASLXOAUTH2_H
#endif /* GUCEF_COM_CEMAILSASLXOAUTH2_H ? */

#ifndef GUCEF_COM_CEMAILSASLSCRAMSHA256_H
#include "gucefCOM_CEMailSaslScramSha256.h"
#define GUCEF_COM_CEMAILSASLSCRAMSHA256_H
#endif /* GUCEF_COM_CEMAILSASLSCRAMSHA256_H ? */

#ifndef GUCEF_COM_CIEMAILSASLMECHANISM_H
#include "gucefCOM_CIEMailSaslMechanism.h"
#define GUCEF_COM_CIEMAILSASLMECHANISM_H
#endif /* GUCEF_COM_CIEMAILSASLMECHANISM_H ? */

#include "gucefCOM_CEMailSaslFactory.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIEMailSaslMechanism*
CEMailSaslFactory::Create( const CORE::CString& name, const CORE::CString& userName, const CORE::CString& secret )
{GUCEF_TRACE;

    const CORE::CString upper = name.Uppercase();
    if ( upper == "PLAIN" ) return GUCEF_NEW CEMailSaslPlain( userName, secret );
    if ( upper == "LOGIN" ) return GUCEF_NEW CEMailSaslLogin( userName, secret );
    if ( upper == "XOAUTH2" ) return GUCEF_NEW CEMailSaslXOAuth2( userName, secret );
    if ( upper == "SCRAM-SHA-256" ) return GUCEF_NEW CEMailSaslScramSha256( userName, secret );
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CEMailSaslFactory::GetSupportedMechanisms( CORE::CString::StringVector& names )
{GUCEF_TRACE;

    names.clear();
    names.push_back( "SCRAM-SHA-256" );
    names.push_back( "PLAIN" );
    names.push_back( "LOGIN" );
    names.push_back( "XOAUTH2" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
