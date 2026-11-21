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

#ifndef GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H
#include "gucefWEB_CWebLoginScriptStep.h"
#define GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H ? */
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

namespace {

struct SActionName
{
    CWebLoginScriptStep::TAction action;
    const char* name;
};

const SActionName g_actionNames[] =
{
    { CWebLoginScriptStep::ACTION_OPEN, "open" },
    { CWebLoginScriptStep::ACTION_WAIT_FOR, "waitFor" },
    { CWebLoginScriptStep::ACTION_FILL, "fill" },
    { CWebLoginScriptStep::ACTION_CLICK, "click" },
    { CWebLoginScriptStep::ACTION_SUBMIT, "submit" },
    { CWebLoginScriptStep::ACTION_PERSON_CHECK, "personCheck" },
    { CWebLoginScriptStep::ACTION_FAIL_CHECK, "failCheck" },
    { CWebLoginScriptStep::ACTION_DELAY, "delay" },
    { CWebLoginScriptStep::ACTION_WAIT_FOR_SIGNED_IN, "waitForSignedIn" }
};

const UInt32 g_actionCount = sizeof( g_actionNames ) / sizeof( g_actionNames[ 0 ] );

bool
IsKeyOf( CWebLoginScriptStep::TAction action, const CORE::CString& key )
{
    if ( key == "action" || key == "comment" )
        return true;
    switch ( action )
    {
        case CWebLoginScriptStep::ACTION_OPEN:
            return key == "url";
        case CWebLoginScriptStep::ACTION_WAIT_FOR:
            return CWebLoginScriptCondition::IsConditionKey( key ) || key == "timeoutMs" || key == "optional";
        case CWebLoginScriptStep::ACTION_FILL:
            return key == "selector" || key == "credential" || key == "timeoutMs" || key == "optional";
        case CWebLoginScriptStep::ACTION_CLICK:
            return key == "selector" || key == "timeoutMs" || key == "optional";
        case CWebLoginScriptStep::ACTION_SUBMIT:
            return key == "selector" || key == "timeoutMs";
        case CWebLoginScriptStep::ACTION_PERSON_CHECK:
            return CWebLoginScriptCondition::IsConditionKey( key ) || key == "kind" || key == "prompt";
        case CWebLoginScriptStep::ACTION_FAIL_CHECK:
            return CWebLoginScriptCondition::IsConditionKey( key ) || key == "reason";
        case CWebLoginScriptStep::ACTION_DELAY:
            return key == "ms";
        case CWebLoginScriptStep::ACTION_WAIT_FOR_SIGNED_IN:
            return key == "timeoutMs";
        default:
            return false;
    }
}

bool
IsInlineValueKey( const CORE::CString& key )
{
    const CORE::CString lower = key.Lowercase();
    return lower == "value" || lower == "password" || lower == "username" || lower == "user" || lower == "email" ||
           lower == "secret" || lower == "token" || lower == "code" || lower == "otp";
}

bool
IsCodeKey( const CORE::CString& key )
{
    const CORE::CString lower = key.Lowercase();
    return lower == "script" || lower == "javascript" || lower == "js" || lower == "expression" || lower == "eval" ||
           lower == "function" || lower == "evaluate" || lower == "run";
}

}

/*-------------------------------------------------------------------------*/

CWebLoginScriptStep::CWebLoginScriptStep( void )
    : action( ACTION_UNKNOWN )
    , url()
    , selector()
    , credential()
    , condition()
    , kind()
    , prompt()
    , reason()
    , comment()
    , timeoutInMs( 0 )
    , delayInMs( 0 )
    , optional( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebLoginScriptStep::Clear( void )
{GUCEF_TRACE;

    *this = CWebLoginScriptStep();
}

/*-------------------------------------------------------------------------*/

const char*
CWebLoginScriptStep::ActionToString( TAction action )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<g_actionCount; ++i )
        if ( g_actionNames[ i ].action == action )
            return g_actionNames[ i ].name;
    return "unknown";
}

/*-------------------------------------------------------------------------*/

CWebLoginScriptStep::TAction
CWebLoginScriptStep::ActionFromString( const CORE::CString& text )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<g_actionCount; ++i )
        if ( text == g_actionNames[ i ].name )
            return g_actionNames[ i ].action;
    return ACTION_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScriptStep::IsValidPersonCheckKind( const CORE::CString& kind )
{GUCEF_TRACE;

    return kind == "captcha" || kind == "code" || kind == "secondFactor" || kind == "other";
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScriptStep::UsesCondition( void ) const
{GUCEF_TRACE;

    return ACTION_WAIT_FOR == action || ACTION_PERSON_CHECK == action || ACTION_FAIL_CHECK == action;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScriptStep::UsesSelector( void ) const
{GUCEF_TRACE;

    return ACTION_FILL == action || ACTION_CLICK == action || ACTION_SUBMIT == action;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScriptStep::ReadFrom( const CORE::CDataNode& node, const CORE::CString& where, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    Clear();
    const size_t before = problems.size();
    if ( GUCEF_DATATYPE_OBJECT != node.GetNodeType() )
    {
        problems.push_back( where + ": a step must be an object" );
        return false;
    }

    // a step holds text, numbers and switches only
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
        problems.push_back( where + ": '" + (*i)->GetName() + "' must not be an object or a list" );

    const CORE::CVariant& actionValue = node.GetAttributeValue( "action" );
    if ( !actionValue.IsString() )
    {
        problems.push_back( where + ": 'action' is missing or not text" );
        return false;
    }
    action = ActionFromString( actionValue.AsString() );
    if ( ACTION_UNKNOWN == action )
    {
        CWebLoginScript::CheckText( actionValue.AsString(), where + ": 'action'", CWebLoginScript::MaxTextLength, problems );
        problems.push_back( where + ": '" + actionValue.AsString() + "' is not an action: open, waitFor, fill, click, submit, personCheck, failCheck, delay or waitForSignedIn" );
        return false;
    }

    const CORE::CString stepWhere = where + " (" + ActionToString( action ) + ")";
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin(); a != node.AttributeEnd(); ++a )
    {
        const CORE::CString& key = (*a).first;
        if ( IsKeyOf( action, key ) )
            continue;
        if ( IsInlineValueKey( key ) )
            problems.push_back( stepWhere + ": '" + key + "' is not allowed: a script holds no values to type, name a credential slot with 'credential'" );
        else
        if ( IsCodeKey( key ) )
            problems.push_back( stepWhere + ": '" + key + "' is not allowed: a script holds no code, only selectors and the fixed steps" );
        else
            problems.push_back( stepWhere + ": '" + key + "' is not a key of this action" );
    }

    CWebLoginScript::ReadText( node, "comment", stepWhere, comment, problems );
    switch ( action )
    {
        case ACTION_OPEN:
        {
            CWebLoginScript::ReadText( node, "url", stepWhere, url, problems );
            break;
        }
        case ACTION_WAIT_FOR:
        {
            condition.ReadFrom( node, stepWhere, problems );
            CWebLoginScript::ReadUInt32( node, "timeoutMs", stepWhere, timeoutInMs, problems );
            CWebLoginScript::ReadBool( node, "optional", stepWhere, optional, problems );
            break;
        }
        case ACTION_FILL:
        {
            CWebLoginScript::ReadText( node, "selector", stepWhere, selector, problems );
            CWebLoginScript::ReadText( node, "credential", stepWhere, credential, problems );
            CWebLoginScript::ReadUInt32( node, "timeoutMs", stepWhere, timeoutInMs, problems );
            CWebLoginScript::ReadBool( node, "optional", stepWhere, optional, problems );
            break;
        }
        case ACTION_CLICK:
        {
            CWebLoginScript::ReadText( node, "selector", stepWhere, selector, problems );
            CWebLoginScript::ReadUInt32( node, "timeoutMs", stepWhere, timeoutInMs, problems );
            CWebLoginScript::ReadBool( node, "optional", stepWhere, optional, problems );
            break;
        }
        case ACTION_SUBMIT:
        {
            CWebLoginScript::ReadText( node, "selector", stepWhere, selector, problems );
            CWebLoginScript::ReadUInt32( node, "timeoutMs", stepWhere, timeoutInMs, problems );
            break;
        }
        case ACTION_PERSON_CHECK:
        {
            condition.ReadFrom( node, stepWhere, problems );
            CWebLoginScript::ReadText( node, "kind", stepWhere, kind, problems );
            CWebLoginScript::ReadText( node, "prompt", stepWhere, prompt, problems );
            break;
        }
        case ACTION_FAIL_CHECK:
        {
            condition.ReadFrom( node, stepWhere, problems );
            CWebLoginScript::ReadText( node, "reason", stepWhere, reason, problems );
            break;
        }
        case ACTION_DELAY:
        {
            CWebLoginScript::ReadUInt32( node, "ms", stepWhere, delayInMs, problems );
            break;
        }
        case ACTION_WAIT_FOR_SIGNED_IN:
        {
            CWebLoginScript::ReadUInt32( node, "timeoutMs", stepWhere, timeoutInMs, problems );
            break;
        }
        default:
            break;
    }
    return before == problems.size();
}

/*-------------------------------------------------------------------------*/

void
CWebLoginScriptStep::WriteTo( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "action", ActionToString( action ) );
    if ( !comment.IsNULLOrEmpty() )
        node.SetAttribute( "comment", comment );
    if ( ACTION_OPEN == action )
        node.SetAttribute( "url", url );
    if ( UsesCondition() )
        condition.WriteTo( node );
    if ( UsesSelector() )
        node.SetAttribute( "selector", selector );
    if ( ACTION_FILL == action )
        node.SetAttribute( "credential", credential );
    if ( ACTION_PERSON_CHECK == action )
    {
        node.SetAttribute( "kind", kind );
        node.SetAttribute( "prompt", prompt );
    }
    if ( ACTION_FAIL_CHECK == action )
        node.SetAttribute( "reason", reason );
    if ( ACTION_DELAY == action )
        node.SetAttribute( "ms", delayInMs );
    if ( timeoutInMs > 0 && ( ACTION_WAIT_FOR == action || UsesSelector() || ACTION_WAIT_FOR_SIGNED_IN == action ) )
        node.SetAttribute( "timeoutMs", timeoutInMs );
    if ( optional && ( ACTION_WAIT_FOR == action || ACTION_FILL == action || ACTION_CLICK == action ) )
        node.SetAttribute( "optional", true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
