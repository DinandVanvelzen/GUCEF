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

#ifndef GUCEF_ASSETSTORE_CWEBLOGINSCRIPTCONDITION_H
#define GUCEF_ASSETSTORE_CWEBLOGINSCRIPTCONDITION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

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
 *  What a page has to show for a condition of a login script (CWebLoginScript) to hold: an element (a CSS selector),
 *  an address (a pattern in which * stands for any text), a text in the page and a cookie of the store. Every part
 *  that is set has to hold, at least one is set. Conditions are data only: they are checked by CWebSessionSignIn with
 *  fixed, built-in queries of the page, never with code taken from a script.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CWebLoginScriptCondition
{
    public:

    CORE::CString selector;         /**< a CSS selector of an element that is shown */
    CORE::CString urlPattern;       /**< the address of the page, * for any text */
    CORE::CString text;             /**< a text that is in the page */
    CORE::CString cookie;           /**< the name of a cookie of the store's domains that has a value */

    CWebLoginScriptCondition( void );

    /** True when no part is set */
    bool IsEmpty( void ) const;

    void Clear( void );

    /** Reads the parts from the attributes of the node; problems (a part that is not text) are added with the prefix */
    void ReadFrom( const CORE::CDataNode& node, const CORE::CString& where, CORE::CString::StringVector& problems );

    /** Writes the parts that are set as attributes of the node */
    void WriteTo( CORE::CDataNode& node ) const;

    /** Checks the parts: at least one, lengths, no control characters; problems are added with the prefix */
    bool Validate( const CORE::CString& where, CORE::CString::StringVector& problems ) const;

    /** A description for a log line or a message: which parts, never a value of a cookie */
    CORE::CString Describe( void ) const;

    /** Whether the name is the name of a part: "selector", "urlPattern", "text" or "cookie" */
    static bool IsConditionKey( const CORE::CString& key );

    /** Whether the text matches the pattern, in which * stands for any text (also none) and every other character for itself */
    static bool MatchesPattern( const CORE::CString& text, const CORE::CString& pattern );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBLOGINSCRIPTCONDITION_H ? */
