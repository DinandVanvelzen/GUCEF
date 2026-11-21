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

#ifndef GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H
#define GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

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
 *  The only JavaScript a web session sign in has a page evaluate: fixed, built-in expressions, each a function that
 *  is called with one argument, a selector or a text of the login script written as a JavaScript string literal
 *  (JsStringLiteral()). A script can therefore only choose what is looked for, never what is run. Every expression
 *  catches what the page throws and answers with a plain value.
 *
 *  Values to type (credentials) never go into an expression: they are typed with Input.insertText into the field
 *  that FocusAndClearExpression() gave the focus.
 */
class GUCEF_WEB_PUBLIC_CPP CWebPageScriptHelpers
{
    public:

    /**
     *  The text as a JavaScript (and JSON) string literal in double quotes: backslash, quote, every control character,
     *  DEL, U+2028 and U+2029 escaped, so nothing in the text can end the literal or the line
     */
    static CORE::CString JsStringLiteral( const CORE::CString& text );

    /** Answers -1 for a selector the page refuses, 0 when there is no such element, 1 when it is there and 2 when it is shown and enabled */
    static CORE::CString ElementStateExpression( const CORE::CString& selector );

    /** Gives the element the focus and empties it (as typing would, with an input event); answers whether it has the focus */
    static CORE::CString FocusAndClearExpression( const CORE::CString& selector );

    /** Answers the number of characters in the value of the element, -1 when there is none */
    static CORE::CString ValueLengthExpression( const CORE::CString& selector );

    /** Clicks the element; answers whether it was there */
    static CORE::CString ClickExpression( const CORE::CString& selector );

    /** Submits the form (the element itself or the form it is in) as its submit button would; answers whether there was one */
    static CORE::CString SubmitExpression( const CORE::CString& selector );

    /** Answers whether the text of the page holds the text */
    static CORE::CString ContainsTextExpression( const CORE::CString& text );

    /** Answers the address of the page */
    static CORE::CString LocationExpression( void );

    private:

    CWebPageScriptHelpers( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H ? */
