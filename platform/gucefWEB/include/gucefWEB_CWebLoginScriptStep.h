/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H
#define GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H
#include "gucefWEB_CWebLoginScriptCondition.h"
#define GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H ? */

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

/**
 *  One step of a login script (CWebLoginScript). The vocabulary is fixed; a step is data and never code:
 *
 *  | action            | keys                                                  | what it does                                          |
 *  |-------------------|-------------------------------------------------------|-------------------------------------------------------|
 *  | `open`            | `url`                                                 | shows the address (http or https, an allowed host)    |
 *  | `waitFor`         | condition keys, `timeoutMs`, `optional`               | waits until the condition holds                       |
 *  | `fill`            | `selector`, `credential`, `timeoutMs`, `optional`     | types the value of the named credential slot          |
 *  | `click`           | `selector`, `timeoutMs`, `optional`                   | clicks the element                                    |
 *  | `submit`          | `selector`, `timeoutMs`                               | submits the form (the element or the form it is in)   |
 *  | `personCheck`     | condition keys, `kind`, `prompt`                      | from here on: when it holds, only a person can go on  |
 *  | `failCheck`       | condition keys, `reason`                              | from here on: when it holds, the sign in failed       |
 *  | `delay`           | `ms` (at most 10000)                                  | waits                                                 |
 *  | `waitForSignedIn` | `timeoutMs`                                           | waits for the signed in condition of the script       |
 *
 *  The condition keys are those of CWebLoginScriptCondition: `selector`, `urlPattern`, `text` and `cookie`. Every step
 *  may have a `comment`. A value to type is never part of a script: `fill` names a credential slot that the script
 *  declares in its `credentials`. The kinds of `personCheck` are "captcha", "code", "secondFactor" and "other".
 */
class GUCEF_WEB_PUBLIC_CPP CWebLoginScriptStep
{
    public:

    enum EAction
    {
        ACTION_UNKNOWN = 0          ,
        ACTION_OPEN                 ,
        ACTION_WAIT_FOR             ,
        ACTION_FILL                 ,
        ACTION_CLICK                ,
        ACTION_SUBMIT               ,
        ACTION_PERSON_CHECK         ,
        ACTION_FAIL_CHECK           ,
        ACTION_DELAY                ,
        ACTION_WAIT_FOR_SIGNED_IN
    };
    typedef enum EAction TAction;

    TAction action;
    CORE::CString url;                      /**< open */
    CORE::CString selector;                 /**< the element of fill, click and submit */
    CORE::CString credential;               /**< fill: the name of the credential slot */
    CWebLoginScriptCondition condition;     /**< waitFor, personCheck, failCheck */
    CORE::CString kind;                     /**< personCheck: "captcha", "code", "secondFactor" or "other" */
    CORE::CString prompt;                   /**< personCheck: what the owner is asked to do */
    CORE::CString reason;                   /**< failCheck: why the sign in failed, for the owner */
    CORE::CString comment;
    UInt32 timeoutInMs;                     /**< 0: the step timeout of the script */
    UInt32 delayInMs;                       /**< delay */
    bool optional;                          /**< waitFor, fill, click: the step is skipped when its element does not appear in time */

    CWebLoginScriptStep( void );

    void Clear( void );

    /**
     *  Reads the step strictly: an unknown action, a key the action does not have, a value of the wrong type or a
     *  value that is inline where a credential slot belongs are problems, added with the prefix
     */
    bool ReadFrom( const CORE::CDataNode& node, const CORE::CString& where, CORE::CString::StringVector& problems );

    void WriteTo( CORE::CDataNode& node ) const;

    /** Whether the condition of the step (waitFor, personCheck, failCheck) or its element (fill, click, submit) is used */
    bool UsesCondition( void ) const;
    bool UsesSelector( void ) const;

    /** "open", "waitFor", ... ; "unknown" for ACTION_UNKNOWN */
    static const char* ActionToString( TAction action );
    static TAction ActionFromString( const CORE::CString& text );

    /** Whether the text is a kind of personCheck */
    static bool IsValidPersonCheckKind( const CORE::CString& kind );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CWebLoginScriptStep > TWebLoginScriptStepVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H ? */
