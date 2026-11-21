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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H
#include "gucefWEB_CWebLoginScriptCondition.h"
#define GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H ? */
#ifndef GUCEF_WEB_CWEBLOGINSCRIPT_H
#include "gucefWEB_CWebLoginScript.h"
#define GUCEF_WEB_CWEBLOGINSCRIPT_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebLoginScriptCondition::CWebLoginScriptCondition( void )
    : selector()
    , urlPattern()
    , text()
    , cookie()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScriptCondition::IsEmpty( void ) const
{GUCEF_TRACE;

    return selector.IsNULLOrEmpty() && urlPattern.IsNULLOrEmpty() && text.IsNULLOrEmpty() && cookie.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CWebLoginScriptCondition::Clear( void )
{GUCEF_TRACE;

    selector.Clear();
    urlPattern.Clear();
    text.Clear();
    cookie.Clear();
}

/*-------------------------------------------------------------------------*/

void
CWebLoginScriptCondition::ReadFrom( const CORE::CDataNode& node, const CORE::CString& where, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    Clear();
    CWebLoginScript::ReadText( node, "selector", where, selector, problems );
    CWebLoginScript::ReadText( node, "urlPattern", where, urlPattern, problems );
    CWebLoginScript::ReadText( node, "text", where, text, problems );
    CWebLoginScript::ReadText( node, "cookie", where, cookie, problems );
}

/*-------------------------------------------------------------------------*/

void
CWebLoginScriptCondition::WriteTo( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    if ( !selector.IsNULLOrEmpty() )
        node.SetAttribute( "selector", selector );
    if ( !urlPattern.IsNULLOrEmpty() )
        node.SetAttribute( "urlPattern", urlPattern );
    if ( !text.IsNULLOrEmpty() )
        node.SetAttribute( "text", text );
    if ( !cookie.IsNULLOrEmpty() )
        node.SetAttribute( "cookie", cookie );
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScriptCondition::Validate( const CORE::CString& where, CORE::CString::StringVector& problems ) const
{GUCEF_TRACE;

    const size_t before = problems.size();
    if ( IsEmpty() )
    {
        problems.push_back( where + ": a condition needs at least one of 'selector', 'urlPattern', 'text' or 'cookie'" );
        return false;
    }
    if ( !selector.IsNULLOrEmpty() )
        CWebLoginScript::CheckText( selector, where + ": 'selector'", CWebLoginScript::MaxSelectorLength, problems );
    if ( !urlPattern.IsNULLOrEmpty() )
        CWebLoginScript::CheckText( urlPattern, where + ": 'urlPattern'", CWebLoginScript::MaxUrlLength, problems );
    if ( !text.IsNULLOrEmpty() )
        CWebLoginScript::CheckText( text, where + ": 'text'", CWebLoginScript::MaxTextLength, problems );
    if ( !cookie.IsNULLOrEmpty() && !CWebLoginScript::IsValidCookieName( cookie ) )
        problems.push_back( where + ": 'cookie' is not a valid cookie name (letters, digits and !#$%&'*+-.^_`|~, at most 128)" );
    return before == problems.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebLoginScriptCondition::Describe( void ) const
{GUCEF_TRACE;

    CORE::CString description;
    if ( !selector.IsNULLOrEmpty() )
        description += "element '" + selector + "'";
    if ( !urlPattern.IsNULLOrEmpty() )
        description += ( description.IsNULLOrEmpty() ? "" : ", " ) + CORE::CString( "address '" ) + urlPattern + "'";
    if ( !text.IsNULLOrEmpty() )
        description += ( description.IsNULLOrEmpty() ? "" : ", " ) + CORE::CString( "text '" ) + text + "'";
    if ( !cookie.IsNULLOrEmpty() )
        description += ( description.IsNULLOrEmpty() ? "" : ", " ) + CORE::CString( "cookie '" ) + cookie + "'";
    return description;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScriptCondition::IsConditionKey( const CORE::CString& key )
{GUCEF_TRACE;

    return key == "selector" || key == "urlPattern" || key == "text" || key == "cookie";
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScriptCondition::MatchesPattern( const CORE::CString& text, const CORE::CString& pattern )
{GUCEF_TRACE;

    // iterative glob with backtracking to the last star, linear in practice
    const char* t = text.C_String();
    const char* p = pattern.C_String();
    if ( GUCEF_NULL == t )
        t = "";
    if ( GUCEF_NULL == p )
        p = "";
    const char* star = GUCEF_NULL;
    const char* afterStar = GUCEF_NULL;
    while ( '\0' != *t )
    {
        if ( '*' == *p )
        {
            star = p++;
            afterStar = t;
        }
        else
        if ( *p == *t )
        {
            ++p;
            ++t;
        }
        else
        if ( GUCEF_NULL != star )
        {
            p = star + 1;
            t = ++afterStar;
        }
        else
            return false;
    }
    while ( '*' == *p )
        ++p;
    return '\0' == *p;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
