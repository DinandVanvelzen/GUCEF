/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_NAMES_H
#include "gucefAUTH_Names.h"
#define GUCEF_AUTH_NAMES_H
#endif /* GUCEF_AUTH_NAMES_H ? */

#include "gucefGAMEGUI_Utils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
FormatPing( Int32 pingMs )
{GUCEF_TRACE;

    if ( pingMs < 0 )
        return CORE::CString::Empty;
    return CORE::Int32ToString( pingMs ) + " ms";
}

/*-------------------------------------------------------------------------*/

CORE::CString
FormatPlayers( UInt32 players, UInt32 maxPlayers )
{GUCEF_TRACE;

    if ( 0 == maxPlayers )
        return CORE::UInt32ToString( players );
    return CORE::UInt32ToString( players ) + "/" + CORE::UInt32ToString( maxPlayers );
}

/*-------------------------------------------------------------------------*/

CORE::CString
TrimText( const CORE::CString& text )
{GUCEF_TRACE;

    return text.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
FormatClock( const CORE::CDateTime& time )
{GUCEF_TRACE;

    CORE::CString hours = CORE::Int32ToString( (Int32) time.GetHours() );
    CORE::CString minutes = CORE::Int32ToString( (Int32) time.GetMinutes() );
    if ( hours.Length() < 2 )
        hours = "0" + hours;
    if ( minutes.Length() < 2 )
        minutes = "0" + minutes;
    return hours + ":" + minutes;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetMemberName( const MULTIPLAY::CLobbyState& state, const CORE::CString& memberId )
{GUCEF_TRACE;

    const MULTIPLAY::CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL != member && !member->name.IsNULLOrEmpty() )
        return member->name;
    return memberId;
}

/*-------------------------------------------------------------------------*/

Int32
GetSecondsUntil( const CORE::CDateTime& deadline )
{GUCEF_TRACE;

    // a deadline that was never set is the default date time
    if ( deadline.GetYear() < 1971 )
        return -1;

    Int64 msLeft = CORE::CDateTime::NowUTCDateTime().GetTimeDifferenceInMillisecondsTowards( deadline );
    if ( msLeft <= 0 )
        return 0;
    return (Int32) ( ( msLeft + 999 ) / 1000 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
FormatCountdown( Int64 timeInMs )
{GUCEF_TRACE;

    Int64 seconds = timeInMs <= 0 ? 0 : ( timeInMs + 999 ) / 1000;
    CORE::CString secondsText = CORE::Int64ToString( seconds % 60 );
    if ( secondsText.Length() < 2 )
        secondsText = "0" + secondsText;
    return CORE::Int64ToString( seconds / 60 ) + ":" + secondsText;
}

/*-------------------------------------------------------------------------*/

CORE::CString
JoinLines( const CORE::CString::StringVector& lines )
{GUCEF_TRACE;

    CORE::CString text;
    for ( UInt32 i=0; i<(UInt32)lines.size(); ++i )
    {
        if ( i > 0 )
            text += '\n';
        text += lines[ i ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
GetAccountProviderNames( const MULTIPLAY::CPlayerIdentity& identity, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    names.clear();
    const UInt32 keyCount = identity.accounts.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        const CORE::CString name = AUTH::GetAccountProviderDisplayName( identity.accounts.GetKey( i ).AsString() );
        bool known = false;
        for ( size_t n=0; n<names.size(); ++n )
            known = known || names[ n ] == name;
        if ( !known )
            names.push_back( name );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeAccountsSummary( const MULTIPLAY::CPlayerIdentity& identity, UInt32 maxNames )
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    GetAccountProviderNames( identity, names );
    if ( names.empty() )
        return "Guest";
    CORE::CString summary;
    for ( size_t n=0; n<names.size() && n<maxNames; ++n )
        summary += ( 0 == n ? "" : ", " ) + names[ n ];
    if ( names.size() > maxNames )
        summary += " +" + CORE::UInt32ToString( (UInt32)( names.size() - maxNames ) );
    return summary;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
