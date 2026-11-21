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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_WEB_CWEBJSON_H
#include "gucefWEB_CWebJson.h"
#define GUCEF_WEB_CWEBJSON_H
#endif /* GUCEF_WEB_CWEBJSON_H ? */
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

const CORE::CString CWebLoginScript::FormatName = "gucef.webLoginScript";
const UInt32 CWebLoginScript::CurrentVersion = 1;
const UInt32 CWebLoginScript::MaxSelectorLength = 512;
const UInt32 CWebLoginScript::MaxTextLength = 256;
const UInt32 CWebLoginScript::MaxUrlLength = 2048;
const UInt32 CWebLoginScript::MaxPromptLength = 512;
const UInt32 CWebLoginScript::MaxSteps = 64;
const UInt32 CWebLoginScript::MaxDomains = 16;
const UInt32 CWebLoginScript::MaxCredentialSlots = 8;
const UInt32 CWebLoginScript::MaxTimeoutInMs = 600000;
const UInt32 CWebLoginScript::MaxDelayInMs = 10000;
const UInt32 CWebLoginScript::DefaultStepTimeoutInMs = 15000;
const UInt32 CWebLoginScript::MaxScriptBytes = 65536;

/*-------------------------------------------------------------------------*/

namespace {

bool
IsCodeKey( const CORE::CString& key )
{
    const CORE::CString lower = key.Lowercase();
    return lower == "script" || lower == "javascript" || lower == "js" || lower == "expression" || lower == "eval" ||
           lower == "function" || lower == "evaluate" || lower == "run" || lower == "code";
}

bool
IsTopLevelAttribute( const CORE::CString& key )
{
    return key == "format" || key == "version" || key == "name" || key == "comment" || key == "startUrl" || key == "stepTimeoutMs";
}

bool
IsTopLevelChild( const CORE::CString& key )
{
    return key == "cookieDomains" || key == "allowedHosts" || key == "credentials" || key == "signedIn" || key == "steps" || key == "sessionEnded";
}

CORE::CString
StepWhere( size_t index )
{
    return "step " + CORE::ToString( (UInt32) ( index + 1 ) );
}

}

/*-------------------------------------------------------------------------*/

CWebLoginScript::CWebLoginScript( void )
    : version( CurrentVersion )
    , name()
    , comment()
    , startUrl()
    , cookieDomains()
    , allowedHosts()
    , credentialSlots()
    , signedIn()
    , sessionEndedStatusCodes()
    , sessionEndedRedirectPattern()
    , stepTimeoutInMs( DefaultStepTimeoutInMs )
    , steps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebLoginScript::Clear( void )
{GUCEF_TRACE;

    *this = CWebLoginScript();
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::ReadText( const CORE::CDataNode& node, const CORE::CString& key, const CORE::CString& where, CORE::CString& value, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( key );
    if ( GUCEF_NULL == attribute )
        return true;
    if ( !(*attribute).second.IsString() )
    {
        problems.push_back( where + ": '" + key + "' must be text" );
        return false;
    }
    value = (*attribute).second.AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::ReadUInt32( const CORE::CDataNode& node, const CORE::CString& key, const CORE::CString& where, UInt32& value, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( key );
    if ( GUCEF_NULL == attribute )
        return true;
    const CORE::CVariant& variant = (*attribute).second;
    if ( !variant.IsInteger() || variant.AsInt64( -1 ) < 0 || variant.AsInt64( -1 ) > (Int64) GUCEF_MT_UINT32MAX )
    {
        problems.push_back( where + ": '" + key + "' must be a whole number from 0 up" );
        return false;
    }
    value = (UInt32) variant.AsInt64( 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::ReadBool( const CORE::CDataNode& node, const CORE::CString& key, const CORE::CString& where, bool& value, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( key );
    if ( GUCEF_NULL == attribute )
        return true;
    if ( !(*attribute).second.IsBoolean() )
    {
        problems.push_back( where + ": '" + key + "' must be true or false" );
        return false;
    }
    value = (*attribute).second.AsBool( false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::CheckText( const CORE::CString& text, const CORE::CString& what, UInt32 maxLength, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    const UInt32 byteCount = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    if ( byteCount > maxLength )
    {
        problems.push_back( what + " is longer than " + CORE::ToString( maxLength ) + " bytes" );
        return false;
    }
    const char* c = text.C_String();
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        const unsigned char b = (unsigned char) c[ i ];
        if ( b < 0x20 || 0x7F == b )
        {
            problems.push_back( what + " holds a control character" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::IsValidCookieName( const CORE::CString& name )
{GUCEF_TRACE;

    const UInt32 byteCount = name.IsNULLOrEmpty() ? 0 : name.ByteSize() - 1;
    if ( 0 == byteCount || byteCount > 128 )
        return false;
    static const char* allowed = "!#$%&'*+-.^_`|~";
    const char* c = name.C_String();
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        const char ch = c[ i ];
        const bool alphaNumeric = ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) || ( ch >= '0' && ch <= '9' );
        if ( !alphaNumeric && GUCEF_NULL == strchr( allowed, ch ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::IsValidCredentialSlotName( const CORE::CString& name )
{GUCEF_TRACE;

    const UInt32 byteCount = name.IsNULLOrEmpty() ? 0 : name.ByteSize() - 1;
    if ( 0 == byteCount || byteCount > 32 )
        return false;
    const char* c = name.C_String();
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        const char ch = c[ i ];
        const bool letter = ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' );
        const bool digit = ch >= '0' && ch <= '9';
        if ( 0 == i ? !letter : !( letter || digit || '_' == ch || '-' == ch ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::NormaliseDomain( const CORE::CString& domain, CORE::CString& normalised )
{GUCEF_TRACE;

    CORE::CString lower = domain.Lowercase();
    if ( lower.StartsWith( "." ) )
        lower = lower.CutChars( 1, true, 0 );
    const UInt32 byteCount = lower.IsNULLOrEmpty() ? 0 : lower.ByteSize() - 1;
    if ( 0 == byteCount || byteCount > 253 )
        return false;
    const char* c = lower.C_String();
    bool hasDot = false;
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        const char ch = c[ i ];
        if ( '.' == ch )
        {
            if ( 0 == i || i + 1 == byteCount || '.' == c[ i - 1 ] )
                return false;
            hasDot = true;
            continue;
        }
        if ( !( ( ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9' ) || '-' == ch ) )
            return false;
    }
    if ( !hasDot && lower != "localhost" )
        return false;
    normalised = lower;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebLoginScript::HostOfUrl( const CORE::CString& url )
{GUCEF_TRACE;

    const CORE::CString lower = url.Lowercase();
    UInt32 schemeLength = 0;
    if ( lower.StartsWith( "https://" ) )
        schemeLength = 8;
    else
    if ( lower.StartsWith( "http://" ) )
        schemeLength = 7;
    else
        return CORE::CString::Empty;

    CORE::CString rest = lower.CutChars( schemeLength, true, 0 );
    CORE::CString authority;
    const char* c = rest.C_String();
    const UInt32 byteCount = rest.IsNULLOrEmpty() ? 0 : rest.ByteSize() - 1;
    UInt32 end = 0;
    while ( end < byteCount && '/' != c[ end ] && '?' != c[ end ] && '#' != c[ end ] && '\\' != c[ end ] )
        ++end;
    authority.Set( c, end );

    // user information is how an address can look like one host and go to another
    if ( authority.HasChar( '@' ) >= 0 || authority.StartsWith( "[" ) )
        return CORE::CString::Empty;

    CORE::CString host = authority;
    const Int32 colon = authority.HasChar( ':' );
    if ( colon >= 0 )
    {
        host = authority.SubstrToIndex( (UInt32) colon, true );
        const CORE::CString port = authority.CutChars( (UInt32) colon + 1, true, 0 );
        const UInt32 portLength = port.IsNULLOrEmpty() ? 0 : port.ByteSize() - 1;
        if ( 0 == portLength || portLength > 5 )
            return CORE::CString::Empty;
        for ( UInt32 i=0; i<portLength; ++i )
            if ( port.C_String()[ i ] < '0' || port.C_String()[ i ] > '9' )
                return CORE::CString::Empty;
        const UInt32 portNumber = CORE::StringToUInt32( port, 0 );
        if ( 0 == portNumber || portNumber > 65535 )
            return CORE::CString::Empty;
    }
    CORE::CString normalised;
    if ( !NormaliseDomain( host, normalised ) || host.StartsWith( "." ) )
        return CORE::CString::Empty;
    return normalised;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::IsHostWithin( const CORE::CString& host, const CORE::CString& domain )
{GUCEF_TRACE;

    if ( host.IsNULLOrEmpty() || domain.IsNULLOrEmpty() )
        return false;
    return host == domain || host.EndsWith( "." + domain );
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::IsHostAllowed( const CORE::CString& host ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<cookieDomains.size(); ++i )
    {
        CORE::CString domain;
        if ( NormaliseDomain( cookieDomains[ i ], domain ) && IsHostWithin( host, domain ) )
            return true;
    }
    for ( size_t i=0; i<allowedHosts.size(); ++i )
    {
        CORE::CString domain;
        if ( NormaliseDomain( allowedHosts[ i ], domain ) && IsHostWithin( host, domain ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::IsUrlAllowed( const CORE::CString& url ) const
{GUCEF_TRACE;

    const CORE::CString host = HostOfUrl( url );
    return !host.IsNULLOrEmpty() && IsHostAllowed( host );
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::IsStoreCookieDomain( const CORE::CString& cookieDomain ) const
{GUCEF_TRACE;

    CORE::CString host;
    if ( !NormaliseDomain( cookieDomain, host ) )
        return false;
    for ( size_t i=0; i<cookieDomains.size(); ++i )
    {
        CORE::CString domain;
        if ( NormaliseDomain( cookieDomains[ i ], domain ) && IsHostWithin( host, domain ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::NeedsCredentials( void ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<steps.size(); ++i )
        if ( CWebLoginScriptStep::ACTION_FILL == steps[ i ].action )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebLoginScript::GetStepTimeout( const CWebLoginScriptStep& step ) const
{GUCEF_TRACE;

    if ( step.timeoutInMs > 0 )
        return step.timeoutInMs;
    return stepTimeoutInMs > 0 ? stepTimeoutInMs : DefaultStepTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebLoginScript::DescribeProblems( const CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<problems.size(); ++i )
    {
        if ( i > 0 )
            text += "\n";
        text += problems[ i ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::ReadTextList( const CORE::CDataNode& node, const CORE::CString& key, UInt32 maxCount, CORE::CString::StringVector& values, CORE::CString::StringVector& problems ) const
{GUCEF_TRACE;

    values.clear();
    if ( GUCEF_NULL != node.GetAttribute( key ) )
    {
        problems.push_back( "'" + key + "' must be a list of texts" );
        return false;
    }
    const CORE::CDataNode* list = node.FindChild( key );
    if ( GUCEF_NULL == list )
        return true;
    if ( GUCEF_DATATYPE_ARRAY != list->GetNodeType() )
    {
        problems.push_back( "'" + key + "' must be a list of texts" );
        return false;
    }
    bool ok = true;
    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
    {
        const CORE::CVariant& value = (*i)->GetValue();
        if ( !value.IsString() || (*i)->GetNrOfDirectChildNodes() > 0 )
        {
            problems.push_back( "'" + key + "' must be a list of texts" );
            return false;
        }
        values.push_back( value.AsString() );
    }
    if ( values.size() > maxCount )
    {
        problems.push_back( "'" + key + "' has more than " + CORE::ToString( maxCount ) + " entries" );
        ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::LoadFromDataNode( const CORE::CDataNode& node, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    Clear();
    const size_t before = problems.size();

    // keys first, so a script with a typo or an unexpected part is refused with a clear message
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin(); a != node.AttributeEnd(); ++a )
    {
        const CORE::CString& key = (*a).first;
        if ( IsTopLevelAttribute( key ) )
            continue;
        if ( IsTopLevelChild( key ) )
            problems.push_back( "'" + key + "' must be " + ( key == "signedIn" || key == "sessionEnded" ? "an object" : "a list" ) );
        else
        if ( IsCodeKey( key ) )
            problems.push_back( "'" + key + "' is not allowed: a login script holds no code, only selectors and the fixed steps" );
        else
            problems.push_back( "'" + key + "' is not a key of a login script" );
    }
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
    {
        const CORE::CString& key = (*i)->GetName();
        if ( IsTopLevelChild( key ) )
            continue;
        if ( IsTopLevelAttribute( key ) )
            problems.push_back( "'" + key + "' must not be an object or a list" );
        else
        if ( IsCodeKey( key ) )
            problems.push_back( "'" + key + "' is not allowed: a login script holds no code, only selectors and the fixed steps" );
        else
            problems.push_back( "'" + key + "' is not a key of a login script" );
    }

    CORE::CString format;
    ReadText( node, "format", "script", format, problems );
    if ( format != FormatName )
        problems.push_back( "'format' must be \"" + FormatName + "\"" );

    if ( GUCEF_NULL == node.GetAttribute( "version" ) )
        problems.push_back( "'version' is missing" );
    version = 0;
    ReadUInt32( node, "version", "script", version, problems );
    if ( 0 != version && CurrentVersion != version )
    {
        problems.push_back( "version " + CORE::ToString( version ) + " is not supported, this build reads version " + CORE::ToString( CurrentVersion ) );
        return false;
    }

    ReadText( node, "name", "script", name, problems );
    ReadText( node, "comment", "script", comment, problems );
    ReadText( node, "startUrl", "script", startUrl, problems );
    ReadUInt32( node, "stepTimeoutMs", "script", stepTimeoutInMs, problems );
    ReadTextList( node, "cookieDomains", MaxDomains, cookieDomains, problems );
    ReadTextList( node, "allowedHosts", MaxDomains, allowedHosts, problems );
    ReadTextList( node, "credentials", MaxCredentialSlots, credentialSlots, problems );

    const CORE::CDataNode* signedInNode = node.FindChild( "signedIn" );
    if ( GUCEF_NULL != signedInNode )
    {
        if ( GUCEF_DATATYPE_OBJECT != signedInNode->GetNodeType() || signedInNode->GetNrOfDirectChildNodes() > 0 )
            problems.push_back( "'signedIn' must be an object of texts" );
        for ( CORE::CDataNode::TAttributeMap::const_iterator a = signedInNode->AttributeBegin(); a != signedInNode->AttributeEnd(); ++a )
            if ( !CWebLoginScriptCondition::IsConditionKey( (*a).first ) )
                problems.push_back( "signedIn: '" + (*a).first + "' is not a key of a condition: selector, urlPattern, text or cookie" );
        signedIn.ReadFrom( *signedInNode, "signedIn", problems );
    }

    const CORE::CDataNode* sessionEndedNode = node.FindChild( "sessionEnded" );
    if ( GUCEF_NULL != sessionEndedNode )
    {
        for ( CORE::CDataNode::TAttributeMap::const_iterator a = sessionEndedNode->AttributeBegin(); a != sessionEndedNode->AttributeEnd(); ++a )
            if ( (*a).first != "redirectPattern" )
                problems.push_back( "sessionEnded: '" + (*a).first + "' is not a key: statusCodes or redirectPattern" );
        ReadText( *sessionEndedNode, "redirectPattern", "sessionEnded", sessionEndedRedirectPattern, problems );
        for ( CORE::CDataNode::const_iterator i = sessionEndedNode->ConstBegin(); i != sessionEndedNode->ConstEnd(); ++i )
        {
            if ( (*i)->GetName() != "statusCodes" || GUCEF_DATATYPE_ARRAY != (*i)->GetNodeType() )
            {
                problems.push_back( "sessionEnded: '" + (*i)->GetName() + "' is not a list of status codes" );
                continue;
            }
            for ( CORE::CDataNode::const_iterator c = (*i)->ConstBegin(); c != (*i)->ConstEnd(); ++c )
            {
                const CORE::CVariant& code = (*c)->GetValue();
                if ( !code.IsInteger() )
                {
                    problems.push_back( "sessionEnded: 'statusCodes' must be whole numbers" );
                    break;
                }
                sessionEndedStatusCodes.push_back( (UInt32) code.AsInt64( 0 ) );
            }
        }
    }

    const CORE::CDataNode* stepsNode = node.FindChild( "steps" );
    if ( GUCEF_NULL != stepsNode )
    {
        if ( GUCEF_DATATYPE_ARRAY != stepsNode->GetNodeType() )
            problems.push_back( "'steps' must be a list of objects" );
        size_t index = 0;
        for ( CORE::CDataNode::const_iterator i = stepsNode->ConstBegin(); i != stepsNode->ConstEnd(); ++i, ++index )
        {
            if ( index >= MaxSteps )
            {
                problems.push_back( "'steps' has more than " + CORE::ToString( MaxSteps ) + " steps" );
                break;
            }
            CWebLoginScriptStep step;
            step.ReadFrom( *(*i), StepWhere( index ), problems );
            steps.push_back( step );
        }
    }

    if ( before != problems.size() )
        return false;
    return Validate( problems );
}

/*-------------------------------------------------------------------------*/

void
CWebLoginScript::SaveToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "format", FormatName );
    node.SetAttribute( "version", version );
    node.SetAttribute( "name", name );
    if ( !comment.IsNULLOrEmpty() )
        node.SetAttribute( "comment", comment );
    node.SetAttribute( "startUrl", startUrl );
    node.SetAttribute( "stepTimeoutMs", stepTimeoutInMs );
    node.SetValuesOfChildByName( "cookieDomains", cookieDomains );
    if ( !allowedHosts.empty() )
        node.SetValuesOfChildByName( "allowedHosts", allowedHosts );
    if ( !credentialSlots.empty() )
        node.SetValuesOfChildByName( "credentials", credentialSlots );
    CORE::CDataNode* signedInNode = node.AddChild( "signedIn", GUCEF_DATATYPE_OBJECT );
    signedIn.WriteTo( *signedInNode );
    if ( !sessionEndedStatusCodes.empty() || !sessionEndedRedirectPattern.IsNULLOrEmpty() )
    {
        CORE::CDataNode* sessionEndedNode = node.AddChild( "sessionEnded", GUCEF_DATATYPE_OBJECT );
        if ( !sessionEndedRedirectPattern.IsNULLOrEmpty() )
            sessionEndedNode->SetAttribute( "redirectPattern", sessionEndedRedirectPattern );
        if ( !sessionEndedStatusCodes.empty() )
        {
            CORE::CDataNode::TVariantVector codes;
            for ( size_t i=0; i<sessionEndedStatusCodes.size(); ++i )
                codes.push_back( CORE::CVariant( (Int64) sessionEndedStatusCodes[ i ] ) );
            sessionEndedNode->SetValuesOfChildByName( "statusCodes", codes );
        }
    }
    CORE::CDataNode* stepsNode = node.AddChild( "steps", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<steps.size(); ++i )
    {
        CORE::CDataNode* stepNode = stepsNode->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        steps[ i ].WriteTo( *stepNode );
    }
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::LoadFromText( const CORE::CString& jsonText, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    Clear();
    const UInt32 byteCount = jsonText.IsNULLOrEmpty() ? 0 : jsonText.ByteSize() - 1;
    if ( byteCount > MaxScriptBytes )
    {
        problems.push_back( "the script is larger than " + CORE::ToString( MaxScriptBytes ) + " bytes" );
        return false;
    }
    CORE::CDataNode tree;
    if ( 0 == byteCount || !CWebJson::FromText( jsonText, tree ) )
    {
        problems.push_back( "the script is not JSON (or the json codec, dstorepluginJSONPARSER, is not loaded)" );
        return false;
    }

    // the codec may put the object under a root node of its own
    const CORE::CDataNode* root = &tree;
    if ( 0 == tree.GetAttCount() && 1 == tree.GetNrOfDirectChildNodes() && GUCEF_NULL == tree.FindChild( "steps" ) )
        root = *tree.ConstBegin();
    if ( GUCEF_DATATYPE_OBJECT != root->GetNodeType() )
    {
        problems.push_back( "the script must be a JSON object" );
        return false;
    }
    return LoadFromDataNode( *root, problems );
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::SaveToText( CORE::CString& jsonText ) const
{GUCEF_TRACE;

    CORE::CDataNode tree;
    SaveToDataNode( tree );
    return CWebJson::ToText( tree, jsonText );
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::LoadFromFile( const CORE::CString& path, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    Clear();
    const CORE::CString resolved = CORE::RelativePath( path );
    if ( !CORE::FileExists( resolved ) )
    {
        problems.push_back( "the script file " + resolved + " does not exist" );
        return false;
    }
    if ( CORE::FileSize( resolved ) > MaxScriptBytes )
    {
        problems.push_back( "the script file " + resolved + " is larger than " + CORE::ToString( MaxScriptBytes ) + " bytes" );
        return false;
    }
    CORE::CString text;
    if ( !CORE::LoadTextFileAsString( resolved, text ) )
    {
        problems.push_back( "the script file " + resolved + " cannot be read" );
        return false;
    }
    return LoadFromText( text, problems );
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::SaveToFile( const CORE::CString& path ) const
{GUCEF_TRACE;

    CORE::CString text;
    if ( !SaveToText( text ) )
        return false;
    const CORE::CString resolved = CORE::RelativePath( path );
    return CORE::WriteFileAtomically( resolved, text.C_String(), text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1 );
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::ValidateUrl( const CORE::CString& url, const CORE::CString& where, CORE::CString::StringVector& problems ) const
{GUCEF_TRACE;

    if ( url.IsNULLOrEmpty() )
    {
        problems.push_back( where + " is missing" );
        return false;
    }
    if ( !CheckText( url, where, MaxUrlLength, problems ) )
        return false;
    const CORE::CString host = HostOfUrl( url );
    if ( host.IsNULLOrEmpty() )
    {
        problems.push_back( where + " must be an http or https address with a host name or IPv4 address and no user information" );
        return false;
    }
    if ( !IsHostAllowed( host ) )
    {
        problems.push_back( where + " goes to " + host + ", which is not one of the cookie domains or allowed hosts" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::ValidateStep( const CWebLoginScriptStep& step, const CORE::CString& where, CORE::CString::StringVector& problems ) const
{GUCEF_TRACE;

    const size_t before = problems.size();
    const CORE::CString stepWhere = where + " (" + CWebLoginScriptStep::ActionToString( step.action ) + ")";
    if ( !step.comment.IsNULLOrEmpty() )
        CheckText( step.comment, stepWhere + ": 'comment'", MaxPromptLength, problems );
    if ( step.timeoutInMs > MaxTimeoutInMs )
        problems.push_back( stepWhere + ": 'timeoutMs' is more than " + CORE::ToString( MaxTimeoutInMs ) );

    switch ( step.action )
    {
        case CWebLoginScriptStep::ACTION_OPEN:
        {
            ValidateUrl( step.url, stepWhere + ": 'url'", problems );
            break;
        }
        case CWebLoginScriptStep::ACTION_WAIT_FOR:
        {
            step.condition.Validate( stepWhere, problems );
            break;
        }
        case CWebLoginScriptStep::ACTION_FILL:
        case CWebLoginScriptStep::ACTION_CLICK:
        case CWebLoginScriptStep::ACTION_SUBMIT:
        {
            if ( step.selector.IsNULLOrEmpty() )
                problems.push_back( stepWhere + ": 'selector' is missing" );
            else
                CheckText( step.selector, stepWhere + ": 'selector'", MaxSelectorLength, problems );
            if ( CWebLoginScriptStep::ACTION_FILL == step.action )
            {
                if ( step.credential.IsNULLOrEmpty() )
                    problems.push_back( stepWhere + ": 'credential' is missing: name the credential slot whose value is typed" );
                else
                if ( !IsValidCredentialSlotName( step.credential ) )
                    problems.push_back( stepWhere + ": 'credential' is not a valid slot name" );
                else
                {
                    bool declared = false;
                    for ( size_t i=0; i<credentialSlots.size(); ++i )
                        declared = declared || credentialSlots[ i ] == step.credential;
                    if ( !declared )
                        problems.push_back( stepWhere + ": the credential slot '" + step.credential + "' is not declared in 'credentials'" );
                }
            }
            break;
        }
        case CWebLoginScriptStep::ACTION_PERSON_CHECK:
        {
            step.condition.Validate( stepWhere, problems );
            if ( !CWebLoginScriptStep::IsValidPersonCheckKind( step.kind ) )
                problems.push_back( stepWhere + ": 'kind' must be captcha, code, secondFactor or other" );
            if ( step.prompt.IsNULLOrEmpty() )
                problems.push_back( stepWhere + ": 'prompt' is missing: say what the owner has to do" );
            else
                CheckText( step.prompt, stepWhere + ": 'prompt'", MaxPromptLength, problems );
            break;
        }
        case CWebLoginScriptStep::ACTION_FAIL_CHECK:
        {
            step.condition.Validate( stepWhere, problems );
            if ( step.reason.IsNULLOrEmpty() )
                problems.push_back( stepWhere + ": 'reason' is missing" );
            else
                CheckText( step.reason, stepWhere + ": 'reason'", MaxPromptLength, problems );
            break;
        }
        case CWebLoginScriptStep::ACTION_DELAY:
        {
            if ( 0 == step.delayInMs || step.delayInMs > MaxDelayInMs )
                problems.push_back( stepWhere + ": 'ms' must be from 1 to " + CORE::ToString( MaxDelayInMs ) );
            break;
        }
        case CWebLoginScriptStep::ACTION_WAIT_FOR_SIGNED_IN:
            break;
        default:
        {
            problems.push_back( stepWhere + ": not an action" );
            break;
        }
    }
    return before == problems.size();
}

/*-------------------------------------------------------------------------*/

bool
CWebLoginScript::Validate( CORE::CString::StringVector& problems ) const
{GUCEF_TRACE;

    const size_t before = problems.size();
    if ( CurrentVersion != version )
        problems.push_back( "version " + CORE::ToString( version ) + " is not supported, this build reads version " + CORE::ToString( CurrentVersion ) );
    if ( name.IsNULLOrEmpty() )
        problems.push_back( "'name' is missing" );
    else
        CheckText( name, "'name'", MaxTextLength, problems );
    if ( !comment.IsNULLOrEmpty() )
        CheckText( comment, "'comment'", MaxPromptLength, problems );

    if ( cookieDomains.empty() )
        problems.push_back( "'cookieDomains' is missing: name the domains whose cookies are the session" );
    if ( cookieDomains.size() > MaxDomains || allowedHosts.size() > MaxDomains )
        problems.push_back( "at most " + CORE::ToString( MaxDomains ) + " cookie domains and " + CORE::ToString( MaxDomains ) + " allowed hosts" );
    for ( size_t i=0; i<cookieDomains.size(); ++i )
    {
        CORE::CString normalised;
        if ( !NormaliseDomain( cookieDomains[ i ], normalised ) )
            problems.push_back( "cookie domain '" + cookieDomains[ i ] + "' is not a host name or IPv4 address (a name needs a dot, no wildcards)" );
    }
    for ( size_t i=0; i<allowedHosts.size(); ++i )
    {
        CORE::CString normalised;
        if ( !NormaliseDomain( allowedHosts[ i ], normalised ) )
            problems.push_back( "allowed host '" + allowedHosts[ i ] + "' is not a host name or IPv4 address (a name needs a dot, no wildcards)" );
    }

    if ( credentialSlots.size() > MaxCredentialSlots )
        problems.push_back( "at most " + CORE::ToString( MaxCredentialSlots ) + " credential slots" );
    for ( size_t i=0; i<credentialSlots.size(); ++i )
    {
        if ( !IsValidCredentialSlotName( credentialSlots[ i ] ) )
            problems.push_back( "credential slot '" + credentialSlots[ i ] + "' is not a valid name (a letter, then letters, digits, _ or -, at most 32)" );
        for ( size_t j=0; j<i; ++j )
            if ( credentialSlots[ j ] == credentialSlots[ i ] )
                problems.push_back( "credential slot '" + credentialSlots[ i ] + "' is declared twice" );
    }

    ValidateUrl( startUrl, "'startUrl'", problems );
    if ( signedIn.IsEmpty() )
        problems.push_back( "'signedIn' is missing: say what the page shows once signed in" );
    else
        signedIn.Validate( "signedIn", problems );
    if ( 0 == stepTimeoutInMs || stepTimeoutInMs > MaxTimeoutInMs )
        problems.push_back( "'stepTimeoutMs' must be from 1 to " + CORE::ToString( MaxTimeoutInMs ) );

    for ( size_t i=0; i<sessionEndedStatusCodes.size(); ++i )
        if ( sessionEndedStatusCodes[ i ] < 100 || sessionEndedStatusCodes[ i ] > 599 )
            problems.push_back( "sessionEnded: " + CORE::ToString( sessionEndedStatusCodes[ i ] ) + " is not an HTTP status code" );
    if ( !sessionEndedRedirectPattern.IsNULLOrEmpty() )
        CheckText( sessionEndedRedirectPattern, "sessionEnded: 'redirectPattern'", MaxUrlLength, problems );

    if ( steps.size() > MaxSteps )
        problems.push_back( "more than " + CORE::ToString( MaxSteps ) + " steps" );
    for ( size_t i=0; i<steps.size() && i<MaxSteps; ++i )
        ValidateStep( steps[ i ], StepWhere( i ), problems );

    return before == problems.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
