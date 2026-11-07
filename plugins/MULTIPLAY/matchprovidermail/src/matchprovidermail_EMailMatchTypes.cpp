/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#include "matchprovidermail_EMailMatchTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const g_kindNames[] = { "INVITE", "ACCEPT", "DECLINE", "TURN", "ACK", "NUDGE", "RESIGN", "FINISH", "RESEND", "PROPOSAL", "VOTE", "INTRO_REQUEST", "INTRO_REPLY", "CANCEL", "INCOMPATIBLE", "ROSTER", "DRAFT_PROPOSAL", "DRAFT_VOTE" };
static const UInt32 g_kindCount = sizeof( g_kindNames ) / sizeof( g_kindNames[ 0 ] );
static const UInt32 g_maxNestingDepth = 16;
static const UInt32 g_maxRevisionDigits = 9;
static const UInt32 g_maxBodyTextBytes = 8 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
MailMatchKindToString( TMailMatchKind kind )
{GUCEF_TRACE;

    return (UInt32) kind < g_kindCount ? g_kindNames[ (UInt32) kind ] : "UNKNOWN";
}

/*-------------------------------------------------------------------------*/

bool
TryStringToMailMatchKind( const CORE::CString& text, TMailMatchKind& kind )
{GUCEF_TRACE;

    const CORE::CString upper = text.Uppercase();
    for ( UInt32 i=0; i<g_kindCount; ++i )
        if ( upper == g_kindNames[ i ] )
        {
            kind = (TMailMatchKind) i;
            return true;
        }
    kind = MAILKIND_UNKNOWN;
    return false;
}

/*-------------------------------------------------------------------------*/

const char*
MailTransferToString( TMailTransfer transfer )
{GUCEF_TRACE;

    return MAILTRANSFER_BODY_TEXT == transfer ? "BODY_TEXT" : "ATTACHMENT";
}

/*-------------------------------------------------------------------------*/

bool
TryStringToMailTransfer( const CORE::CString& text, TMailTransfer& transfer )
{GUCEF_TRACE;

    const CORE::CString upper = text.Uppercase();
    if ( upper == "ATTACHMENT" ) { transfer = MAILTRANSFER_ATTACHMENT; return true; }
    if ( upper == "BODY_TEXT" ) { transfer = MAILTRANSFER_BODY_TEXT; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

const char*
MailDistributionToString( TMailDistribution distribution )
{GUCEF_TRACE;

    return MAILDISTRIBUTION_NEXT_ONLY == distribution ? "NEXT_ONLY" : "ALL";
}

/*-------------------------------------------------------------------------*/

bool
TryStringToMailDistribution( const CORE::CString& text, TMailDistribution& distribution )
{GUCEF_TRACE;

    const CORE::CString upper = text.Uppercase();
    if ( upper == "ALL" ) { distribution = MAILDISTRIBUTION_ALL; return true; }
    if ( upper == "NEXT_ONLY" ) { distribution = MAILDISTRIBUTION_NEXT_ONLY; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

const char*
MailPayloadResultToString( TMailPayloadResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case MAILPAYLOAD_OK:            return "OK";
        case MAILPAYLOAD_NONE:          return "NONE";
        case MAILPAYLOAD_TOO_LARGE:     return "TOO_LARGE";
        case MAILPAYLOAD_DAMAGED:       return "DAMAGED";
        case MAILPAYLOAD_BAD_ENCODING:  return "BAD_ENCODING";
        default:                        return "NONE";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
