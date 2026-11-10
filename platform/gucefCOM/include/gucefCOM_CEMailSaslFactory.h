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

#ifndef GUCEF_COM_CEMAILSASLFACTORY_H
#define GUCEF_COM_CEMAILSASLFACTORY_H

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

/** Makes the mechanism of a name */
class GUCEF_COM_PUBLIC_CPP CEMailSaslFactory
{
    public:

    /** The mechanism of the name (any case) for a password or token, GUCEF_NULL for a name that is not known. 'isToken' selects XOAUTH2 behaviour for the secret */
    static CIEMailSaslMechanism* Create( const CORE::CString& name, const CORE::CString& userName, const CORE::CString& secret );

    /** The mechanisms that can be made, in the order of preference for a password */
    static void GetSupportedMechanisms( CORE::CString::StringVector& names );

    private:

    CEMailSaslFactory( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILSASLFACTORY_H ? */
