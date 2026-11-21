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

#ifndef GUCEF_WEB_ETYPES_H
#define GUCEF_WEB_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Maximum and minimal values for the simple types which we are about to
 *      define.
 */
#define GUCEFWEB_INT8MAX     GUCEFCORE_INT8MAX
#define GUCEFWEB_INT8MIN     GUCEFCORE_INT8MIN
#define GUCEFWEB_UINT8MAX    GUCEFCORE_UINT8MAX
#define GUCEFWEB_UINT8MIN    GUCEFCORE_UINT8MIN
#define GUCEFWEB_INT16MAX    GUCEFCORE_INT16MAX
#define GUCEFWEB_INT16MIN    GUCEFCORE_INT16MIN
#define GUCEFWEB_UINT16MAX   GUCEFCORE_UINT16MAX
#define GUCEFWEB_UINT16MIN   GUCEFCORE_UINT16MIN
#define GUCEFWEB_INT32MAX    GUCEFCORE_INT32MAX
#define GUCEFWEB_INT32MIN    GUCEFCORE_INT32MIN
#define GUCEFWEB_UINT32MAX   GUCEFCORE_UINT32MAX
#define GUCEFWEB_UINT32MIN   GUCEFCORE_UINT32MIN
#define GUCEFWEB_FLOAT32MAX  GUCEFCORE_FLOAT32MAX
#define GUCEFWEB_FLOAT32MIN  GUCEFCORE_FLOAT32MIN

/*--------------------------------------------------------------------------*/

/*
 *  We only have to define types when using C++ due to namespacing 
 *  For C the gucefMT versions are automaticly used in the global namespace
 */
#ifdef __cplusplus
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::UInt8 UInt8;    /* 1 byte, unsigned */
typedef CORE::Int8 Int8;      /* 1 byte, signed */

typedef CORE::UInt16 UInt16;   /* 2 bytes, unsigned */
typedef CORE::Int16 Int16;     /* 2 bytes, signed */

typedef CORE::UInt32 UInt32;   /* 4 bytes, unsigned */
typedef CORE::Int32  Int32;    /* 4 bytes, signed */

typedef CORE::Int64 Int64;     /* 8 bytes, signed */
typedef CORE::UInt64 UInt64;   /* 8 bytes, unsigned */

typedef CORE::Float32 Float32;  /* 4 bytes, signed, decimal */
typedef CORE::Float64 Float64;  /* 8 bytes, signed, decimal */

typedef CORE::CString CString;  /* import the string class, we consider it a basic type */

/** How a web session sign in runs */
enum EWebSignInMode
{
    WEBSIGNIN_MODE_INTERACTIVE = 0  ,   /**< the owner started it: a browser window, saved credentials filled in, steps of a person handed over */
    WEBSIGNIN_MODE_UNATTENDED           /**< nobody watches: a browser without a window, saved credentials only, a step of a person ends it */
};
typedef enum EWebSignInMode TWebSignInMode;

/** What a page shows of the element a selector names */
enum EWebElementState
{
    WEBELEMENT_STATE_NO_ANSWER = 0      ,   /**< the page could not be asked: no connection, or the page was being replaced */
    WEBELEMENT_STATE_INVALID_SELECTOR   ,   /**< the page refused the selector */
    WEBELEMENT_STATE_ABSENT             ,
    WEBELEMENT_STATE_PRESENT            ,   /**< in the page, not shown or disabled */
    WEBELEMENT_STATE_VISIBLE                /**< in the page, shown and enabled */
};
typedef enum EWebElementState TWebElementState;

/**
 *  The outcome of a web session sign in (CWebSessionSignIn of gucefAUTH). The values and their names are those of the
 *  same outcomes of an asset store operation (TAssetStoreStatus of gucefASSETSTORE): never renumbered, so a value can
 *  be handed on as one of those and texts made from either read the same.
 */
enum EWebSignInStatus
{
    WEBSIGNIN_STATUS_OK                     = 0     ,   /**< done */
    WEBSIGNIN_STATUS_FAILED                 = 10    ,   /**< another failure */
    WEBSIGNIN_STATUS_INVALID_ARGUMENT       = 11    ,   /**< a bad request or setting */
    WEBSIGNIN_STATUS_NOT_SIGNED_IN          = 13    ,   /**< no session: the owner has to sign in */
    WEBSIGNIN_STATUS_NETWORK_ERROR          = 17    ,   /**< no response: name lookup, connection, time out */
    WEBSIGNIN_STATUS_CANCELLED              = 24    ,   /**< cancelled */
    WEBSIGNIN_STATUS_ALREADY_EXISTS         = 30    ,   /**< a sign in of the same store runs already */
    WEBSIGNIN_STATUS_TIMED_OUT              = 40    ,   /**< a wait ran out of time, such as a sign in that was not finished in time */
    WEBSIGNIN_STATUS_OWNER_ACTION_NEEDED    = 41    ,   /**< only a person can go on (a captcha, a code sent by e-mail, a second factor) and none is there to do it */
    WEBSIGNIN_STATUS_BROWSER_UNAVAILABLE    = 42        /**< no browser to sign in with: none was found, or it did not start */
};
typedef enum EWebSignInStatus TWebSignInStatus;

/*
 *  The texts of the types above. They are inline: this header is included by gucefWEB_macros.h before the macros that
 *  export a function of the module are defined.
 */

/** "interactive", "unattended" */
inline const char*
WebSignInModeToString( TWebSignInMode mode )
{
    return WEBSIGNIN_MODE_UNATTENDED == mode ? "unattended" : "interactive";
}

/** The mode a text names, case insensitive; the default for a text that names none */
inline TWebSignInMode
WebSignInModeFromString( const CORE::CString& text, TWebSignInMode defaultMode = WEBSIGNIN_MODE_INTERACTIVE )
{
    if ( text.Equals( "interactive", false ) )
        return WEBSIGNIN_MODE_INTERACTIVE;
    if ( text.Equals( "unattended", false ) )
        return WEBSIGNIN_MODE_UNATTENDED;
    return defaultMode;
}

/** "ok", "failed", "invalidArgument", "notSignedIn", "networkError", "cancelled", "alreadyExists", "timedOut", "ownerActionNeeded", "browserUnavailable" */
inline const char*
WebSignInStatusToString( TWebSignInStatus status )
{
    switch ( status )
    {
        case WEBSIGNIN_STATUS_OK:                   return "ok";
        case WEBSIGNIN_STATUS_FAILED:               return "failed";
        case WEBSIGNIN_STATUS_INVALID_ARGUMENT:     return "invalidArgument";
        case WEBSIGNIN_STATUS_NOT_SIGNED_IN:        return "notSignedIn";
        case WEBSIGNIN_STATUS_NETWORK_ERROR:        return "networkError";
        case WEBSIGNIN_STATUS_CANCELLED:            return "cancelled";
        case WEBSIGNIN_STATUS_ALREADY_EXISTS:       return "alreadyExists";
        case WEBSIGNIN_STATUS_TIMED_OUT:            return "timedOut";
        case WEBSIGNIN_STATUS_OWNER_ACTION_NEEDED:  return "ownerActionNeeded";
        case WEBSIGNIN_STATUS_BROWSER_UNAVAILABLE:  return "browserUnavailable";
        default:                                    return "failed";
    }
}

/** The status a text names (WebSignInStatusToString()), case insensitive; WEBSIGNIN_STATUS_FAILED for a text that names none */
inline TWebSignInStatus
WebSignInStatusFromString( const CORE::CString& text )
{
    static const TWebSignInStatus statuses[] = { WEBSIGNIN_STATUS_OK, WEBSIGNIN_STATUS_FAILED, WEBSIGNIN_STATUS_INVALID_ARGUMENT,
                                                 WEBSIGNIN_STATUS_NOT_SIGNED_IN, WEBSIGNIN_STATUS_NETWORK_ERROR, WEBSIGNIN_STATUS_CANCELLED,
                                                 WEBSIGNIN_STATUS_ALREADY_EXISTS, WEBSIGNIN_STATUS_TIMED_OUT, WEBSIGNIN_STATUS_OWNER_ACTION_NEEDED,
                                                 WEBSIGNIN_STATUS_BROWSER_UNAVAILABLE };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( statuses ) / sizeof( statuses[ 0 ] ) ); ++i )
        if ( text.Equals( WebSignInStatusToString( statuses[ i ] ), false ) )
            return statuses[ i ];
    return WEBSIGNIN_STATUS_FAILED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_WEB_ETYPES_H ? */





