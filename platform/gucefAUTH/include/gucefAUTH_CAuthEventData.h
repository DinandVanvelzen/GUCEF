/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CAUTHEVENTDATA_H
#define GUCEF_AUTH_CAUTHEVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_AUTH_CAUTHOPTIONS_H
#include "gucefAUTH_CAuthOptions.h"
#define GUCEF_AUTH_CAUTHOPTIONS_H
#endif /* GUCEF_AUTH_CAUTHOPTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the application has to show or do for a login to go on */
enum EAuthActionKind
{
    AUTHACTION_OPEN_URL = 0,            /**< open the url in the system browser (a provider that opens it itself does not need the application to) */
    AUTHACTION_SHOW_DEVICE_CODE         /**< show the user code and the url of the page where it is entered, valid until expiresAtMs */
};

typedef enum EAuthActionKind TAuthActionKind;

/*-------------------------------------------------------------------------*/

/** Event data of CIAuthProvider::ActionRequiredEvent */
class GUCEF_AUTH_EXPORT_CPP CAuthActionRequired : public CORE::CICloneable
{
    public:

    CORE::CString providerName;
    TAuthActionKind kind;
    CORE::CString url;                  /**< to open, or for a device code the page where the code is entered */
    CORE::CString urlWithCode;          /**< device code only: the same page with the code in it, as a link or QR code; may be empty */
    CORE::CString userCode;             /**< device code only: what the user types or compares */
    UInt64 expiresAtMs;                 /**< milliseconds since 1970-01-01 UTC at which the action no longer works; 0 when not known */

    CAuthActionRequired( void );

    CAuthActionRequired( const CAuthActionRequired& src );

    virtual ~CAuthActionRequired();

    CAuthActionRequired& operator=( const CAuthActionRequired& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** Event data of CIAuthProvider::LoginFailedEvent */
class GUCEF_AUTH_EXPORT_CPP CAuthFailure : public CORE::CICloneable
{
    public:

    CORE::CString providerName;
    TAuthFailureReason reason;
    CORE::CString message;              /**< for people and logs, never holds a token */

    CAuthFailure( void );

    CAuthFailure( const CORE::CString& provider, TAuthFailureReason failureReason, const CORE::CString& failureMessage );

    CAuthFailure( const CAuthFailure& src );

    virtual ~CAuthFailure();

    CAuthFailure& operator=( const CAuthFailure& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHEVENTDATA_H ? */
