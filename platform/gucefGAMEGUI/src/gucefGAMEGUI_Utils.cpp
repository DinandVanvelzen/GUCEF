/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_NAMES_H
#include "gucefAUTH_Names.h"
#define GUCEF_AUTH_NAMES_H
#endif /* GUCEF_AUTH_NAMES_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

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
FormatTimeLeft( Int64 remainingInMs )
{GUCEF_TRACE;

    if ( remainingInMs <= 0 )
        return "Overdue";

    const Int64 seconds = ( remainingInMs + 999 ) / 1000;
    if ( seconds < 3600 )
    {
        CORE::CString secondsText = CORE::Int64ToString( seconds % 60 );
        if ( secondsText.Length() < 2 )
            secondsText = "0" + secondsText;
        return CORE::Int64ToString( seconds / 60 ) + ":" + secondsText;
    }
    if ( seconds < 86400 )
    {
        CORE::CString minutesText = CORE::Int64ToString( ( seconds % 3600 ) / 60 );
        if ( minutesText.Length() < 2 )
            minutesText = "0" + minutesText;
        return CORE::Int64ToString( seconds / 3600 ) + "h " + minutesText + "m";
    }
    return CORE::Int64ToString( seconds / 86400 ) + "d " + CORE::Int64ToString( ( seconds % 86400 ) / 3600 ) + "h";
}

/*-------------------------------------------------------------------------*/

namespace {

/** The width of text in a font; false when the context cannot measure it */
bool
MeasureWidth( GUI::CGUIContext& context, const CORE::CString& fontName, const CORE::CString& text, Int32& width )
{
    Int32 height = 0;
    width = 0;
    return context.MeasureText( fontName, text, 0, width, height );
}

}

/*-------------------------------------------------------------------------*/

CORE::CString
EllipsizeText( GUI::CGUIContext& context, const CORE::CString& fontName, const CORE::CString& text, Int32 maxWidth )
{GUCEF_TRACE;

    // control characters (a line break in a name from somewhere else) would break the line of the label; they are single bytes in UTF-8
    CORE::CString clean = text;
    const char* source = text.C_String();
    bool hasControl = false;
    for ( const char* p=source; 0 != *p && !hasControl; ++p )
        hasControl = (unsigned char) *p < 0x20 || 0x7F == (unsigned char) *p;
    if ( hasControl )
    {
        GUCEF::vector< char > bytes( source, source + strlen( source ) + 1 );
        for ( size_t i=0; i+1<bytes.size(); ++i )
            if ( (unsigned char) bytes[ i ] < 0x20 || 0x7F == (unsigned char) bytes[ i ] )
                bytes[ i ] = ' ';
        clean = CORE::CString( &bytes[ 0 ] );
    }
    const UInt32 length = clean.Length();

    Int32 width = 0;
    const bool measured = MeasureWidth( context, fontName, clean, width );
    if ( !measured )
        width = (Int32) length * 8;
    if ( width <= maxWidth )
        return clean;

    // the longest start that fits with the dots behind it: the width only grows with the length, so halve the range
    UInt32 low = 0;
    UInt32 high = length;
    while ( low < high )
    {
        const UInt32 middle = ( low + high + 1 ) / 2;
        const CORE::CString candidate = clean.SubstrToIndex( middle ) + "...";
        Int32 candidateWidth = 0;
        if ( !MeasureWidth( context, fontName, candidate, candidateWidth ) )
            candidateWidth = (Int32) ( middle + 3 ) * 8;
        if ( candidateWidth <= maxWidth )
            low = middle;
        else
            high = middle - 1;
    }
    CORE::CString start = clean.SubstrToIndex( low ).Trim( false );
    const CORE::CString result = start + "...";
    Int32 resultWidth = 0;
    if ( !MeasureWidth( context, fontName, result, resultWidth ) )
        resultWidth = (Int32) result.Length() * 8;
    return resultWidth <= maxWidth ? result : CORE::CString::Empty;
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
