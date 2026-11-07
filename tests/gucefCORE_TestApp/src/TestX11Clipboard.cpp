/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifdef GUCEF_LINUX_BUILD

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifndef GUCEF_CORE_CX11CLIPBOARD_H
#include "gucefCORE_CX11Clipboard.h"
#define GUCEF_CORE_CX11CLIPBOARD_H
#endif /* GUCEF_CORE_CX11CLIPBOARD_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "TestX11ClipboardPeer.h"

#endif /* GUCEF_LINUX_BUILD ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestX11Clipboard.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#ifdef GUCEF_LINUX_BUILD

namespace {

const char* NoDisplayReason = "DISPLAY is not set, the X11 clipboard cases need an X server";

/*-------------------------------------------------------------------------*/

bool
HasTestDisplay( void )
{GUCEF_TRACE;

    const char* display = ::getenv( "DISPLAY" );
    return GUCEF_NULL != display && '\0' != *display;
}

/*-------------------------------------------------------------------------*/

void
AppendText( CORE::CDynamicBuffer& buffer, const char* text )
{GUCEF_TRACE;

    buffer.Append( text, (UInt32) strlen( text ) );
}

/*-------------------------------------------------------------------------*/

/** A text of at least minimumSize bytes with characters of 1, 2, 3 and 4 bytes in UTF-8 */
void
BuildUtf8Text( CORE::CDynamicBuffer& text, UInt32 minimumSize )
{GUCEF_TRACE;

    text.Clear( false );
    UInt32 lineNr = 0;
    while ( text.GetDataSize() < minimumSize )
    {
        // "Gruesse" with u-umlaut and sharp s, the euro sign, three CJK ideographs and the G clef of U+1D11E
        AppendText( text, "Gr\xC3\xBC\xC3\x9F" "e \xE2\x82\xAC 12,34 \xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E \xF0\x9D\x84\x9E line " );
        CORE::CString number = CORE::ToString( lineNr++ );
        AppendText( text, number.C_String() );
        AppendText( text, "\n" );
    }
}

/*-------------------------------------------------------------------------*/

bool
IsSameData( const CORE::CDynamicBuffer& a, const CORE::CDynamicBuffer& b )
{GUCEF_TRACE;

    if ( a.GetDataSize() != b.GetDataSize() )
        return false;
    if ( 0 == a.GetDataSize() )
        return true;
    return 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
IsSameData( const CORE::CDynamicBuffer& a, const char* b )
{GUCEF_TRACE;

    CORE::CDynamicBuffer other;
    AppendText( other, b );
    return IsSameData( a, other );
}

/*-------------------------------------------------------------------------*/

/** StringToClipboard() of the text, which gets a terminating null for that */
bool
PutOnClipboard( const CORE::CDynamicBuffer& text )
{GUCEF_TRACE;

    CORE::CDynamicBuffer terminated;
    if ( text.GetDataSize() > 0 )
        terminated.Append( text );
    char terminator = '\0';
    terminated.Append( &terminator, 1 );
    return OSWRAP_TRUE == CORE::StringToClipboard( static_cast< const char* >( terminated.GetConstBufferPtr() ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Reads the whole clipboard with StringFromClipboard() in parts of at most partSize-1 bytes, checking the terminator
 *  after every part. Returns whether the first call succeeded
 */
bool
ReadWithCApi( CORE::CDynamicBuffer& text, UInt32 partSize, bool& terminatorsOk )
{GUCEF_TRACE;

    text.Clear( false );
    terminatorsOk = true;

    CORE::CDynamicBuffer part;
    part.SetBufferSize( partSize, false );
    char* partBuffer = static_cast< char* >( part.GetBufferPtr() );

    UInt32 wbytes = 0;
    UInt32 calls = 0;
    while ( calls < 100000 )
    {
        UInt32 before = wbytes;
        memset( partBuffer, 'X', partSize );
        if ( OSWRAP_TRUE != CORE::StringFromClipboard( partBuffer, partSize, &wbytes ) )
            break;
        ++calls;
        UInt32 copied = wbytes - before;
        if ( copied > partSize-1 || '\0' != partBuffer[ copied ] )
            terminatorsOk = false;
        text.Append( partBuffer, copied );
    }
    return calls > 0;
}

/*-------------------------------------------------------------------------*/

/** Whether a TARGETS answer (format 32, longs) lists the target */
bool
TargetsContain( const CORE::CDynamicBuffer& targets, ::Atom target )
{GUCEF_TRACE;

    UInt32 count = targets.GetDataSize() / sizeof( long );
    const long* list = static_cast< const long* >( targets.GetConstBufferPtr() );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( (::Atom) list[ i ] == target )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
SleepMs( UInt32 ms )
{GUCEF_TRACE;

    struct timespec pause;
    pause.tv_sec = ms / 1000;
    pause.tv_nsec = (long) ( ms % 1000 ) * 1000000L;
    ::nanosleep( &pause, GUCEF_NULL );
}

}

#endif /* GUCEF_LINUX_BUILD ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformX11ClipboardTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.X11Clipboard" );

    #ifdef GUCEF_LINUX_BUILD

    // This case has to be the first use of the clipboard in the process: a connection, once made, stays
    GUCEF_TESTFW_TESTCASE( "X11Clipboard: Without DISPLAY StringToClipboard() and StringFromClipboard() answer false" )
    {
        const char* display = ::getenv( "DISPLAY" );
        bool hadDisplay = GUCEF_NULL != display;
        CORE::CString savedDisplay( hadDisplay ? display : "" );
        ::unsetenv( "DISPLAY" );
        try
        {
            ASSERT_FALSE( OSWRAP_TRUE == CORE::StringToClipboard( "no display" ) );

            char buffer[ 16 ];
            UInt32 wbytes = 0;
            ASSERT_FALSE( OSWRAP_TRUE == CORE::StringFromClipboard( buffer, sizeof( buffer ), &wbytes ) );
            ASSERT_TRUE( 0 == wbytes );

            ASSERT_FALSE( CORE::CX11Clipboard::Instance()->HasDisplay() );
            ASSERT_FALSE( CORE::CX11Clipboard::Instance()->IsClipboardOwner() );
            CORE::CString text;
            ASSERT_FALSE( CORE::CX11Clipboard::Instance()->GetText( text ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        if ( hadDisplay )
            ::setenv( "DISPLAY", savedDisplay.C_String(), 1 );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: The C API refuses null pointers and a buffer without room for a byte and the terminator" )
    try
    {
        char buffer[ 4 ];
        UInt32 wbytes = 0;
        ASSERT_FALSE( OSWRAP_TRUE == CORE::StringToClipboard( GUCEF_NULL ) );
        ASSERT_FALSE( OSWRAP_TRUE == CORE::StringFromClipboard( GUCEF_NULL, 4, &wbytes ) );
        ASSERT_FALSE( OSWRAP_TRUE == CORE::StringFromClipboard( buffer, 4, GUCEF_NULL ) );
        ASSERT_FALSE( OSWRAP_TRUE == CORE::StringFromClipboard( buffer, 1, &wbytes ) );
        ASSERT_TRUE( 0 == wbytes );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: Another X client gets the copied UTF-8 text byte for byte, and the targets that go with it" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CORE::CDynamicBuffer text;
        BuildUtf8Text( text, 4096 );
        ASSERT_TRUE( PutOnClipboard( text ) );
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->IsClipboardOwner() );

        CTestX11ClipboardPeer peer;
        ASSERT_TRUE( peer.Open() );

        CORE::CDynamicBuffer data;
        ::Atom type = None;
        int format = 0;
        bool wasIncr = false;
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "UTF8_STRING", data, type, format, wasIncr ) );
        ASSERT_TRUE( IsSameData( data, text ) );
        ASSERT_TRUE( type == peer.GetAtom( "UTF8_STRING" ) );
        ASSERT_TRUE( 8 == format );
        ASSERT_FALSE( wasIncr );

        // The same bytes under the MIME name, and TEXT as UTF8_STRING because the text is not ASCII
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "text/plain;charset=utf-8", data, type, format, wasIncr ) );
        ASSERT_TRUE( IsSameData( data, text ) );
        ASSERT_TRUE( type == peer.GetAtom( "text/plain;charset=utf-8" ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "TEXT", data, type, format, wasIncr ) );
        ASSERT_TRUE( IsSameData( data, text ) );
        ASSERT_TRUE( type == peer.GetAtom( "UTF8_STRING" ) );

        CORE::CDynamicBuffer targets;
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "TARGETS", targets, type, format, wasIncr ) );
        ASSERT_TRUE( XA_ATOM == type );
        ASSERT_TRUE( 32 == format );
        ASSERT_TRUE( TargetsContain( targets, peer.GetAtom( "UTF8_STRING" ) ) );
        ASSERT_TRUE( TargetsContain( targets, peer.GetAtom( "TARGETS" ) ) );
        ASSERT_TRUE( TargetsContain( targets, peer.GetAtom( "TIMESTAMP" ) ) );
        ASSERT_TRUE( TargetsContain( targets, peer.GetAtom( "MULTIPLE" ) ) );
        ASSERT_TRUE( TargetsContain( targets, peer.GetAtom( "TEXT" ) ) );
        ASSERT_TRUE( TargetsContain( targets, peer.GetAtom( "text/plain;charset=utf-8" ) ) );

        // Not representable in Latin-1: STRING is neither listed nor served, rather than sent with characters lost
        ASSERT_FALSE( TargetsContain( targets, XA_STRING ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_REFUSED == peer.Convert( "STRING", data, type, format, wasIncr ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_REFUSED == peer.Convert( "text/plain", data, type, format, wasIncr ) );

        // The time the clipboard was taken
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "TIMESTAMP", data, type, format, wasIncr ) );
        ASSERT_TRUE( XA_INTEGER == type );
        ASSERT_TRUE( 32 == format );
        ASSERT_TRUE( sizeof( long ) == data.GetDataSize() );
        if ( sizeof( long ) == data.GetDataSize() )
            ASSERT_TRUE( 0 != *static_cast< const long* >( data.GetConstBufferPtr() ) );

        // Anything else is refused
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_REFUSED == peer.Convert( "image/png", data, type, format, wasIncr ) );

        peer.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: A Latin-1 text is also served as STRING in ISO 8859-1 and an ASCII text as text/plain and TEXT in STRING" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CORE::CDynamicBuffer latin1Text;
        AppendText( latin1Text, "Caf\xC3\xA9 cr\xC3\xA8me \xC2\xBD" );    // "Cafe creme 1/2" with e-acute, e-grave and the one half sign
        ASSERT_TRUE( PutOnClipboard( latin1Text ) );

        CTestX11ClipboardPeer peer;
        ASSERT_TRUE( peer.Open() );

        CORE::CDynamicBuffer data;
        ::Atom type = None;
        int format = 0;
        bool wasIncr = false;
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "TARGETS", data, type, format, wasIncr ) );
        ASSERT_TRUE( TargetsContain( data, XA_STRING ) );
        ASSERT_FALSE( TargetsContain( data, peer.GetAtom( "text/plain" ) ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "STRING", data, type, format, wasIncr ) );
        ASSERT_TRUE( XA_STRING == type );
        ASSERT_TRUE( IsSameData( data, "Caf\xE9 cr\xE8me \xBD" ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "UTF8_STRING", data, type, format, wasIncr ) );
        ASSERT_TRUE( IsSameData( data, latin1Text ) );

        CORE::CDynamicBuffer asciiText;
        AppendText( asciiText, "plain ASCII 123" );
        ASSERT_TRUE( PutOnClipboard( asciiText ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "TARGETS", data, type, format, wasIncr ) );
        ASSERT_TRUE( TargetsContain( data, XA_STRING ) );
        ASSERT_TRUE( TargetsContain( data, peer.GetAtom( "text/plain" ) ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "text/plain", data, type, format, wasIncr ) );
        ASSERT_TRUE( IsSameData( data, asciiText ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "TEXT", data, type, format, wasIncr ) );
        ASSERT_TRUE( IsSameData( data, asciiText ) );
        ASSERT_TRUE( XA_STRING == type );

        // An empty text is content as well
        CORE::CDynamicBuffer emptyText;
        ASSERT_TRUE( PutOnClipboard( emptyText ) );
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "UTF8_STRING", data, type, format, wasIncr ) );
        ASSERT_TRUE( 0 == data.GetDataSize() );

        peer.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: StringFromClipboard() returns the text this process copied, in parts with wbytes as the offset" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CORE::CDynamicBuffer text;
        BuildUtf8Text( text, 3000 );
        ASSERT_TRUE( PutOnClipboard( text ) );

        // In one go
        CORE::CDynamicBuffer read;
        bool terminatorsOk = false;
        ASSERT_TRUE( ReadWithCApi( read, text.GetDataSize() + 1, terminatorsOk ) );
        ASSERT_TRUE( terminatorsOk );
        ASSERT_TRUE( IsSameData( read, text ) );

        // In parts of 99 bytes: every call continues where wbytes says, the last one answers false
        ASSERT_TRUE( ReadWithCApi( read, 100, terminatorsOk ) );
        ASSERT_TRUE( terminatorsOk );
        ASSERT_TRUE( IsSameData( read, text ) );

        // Past the end there is nothing more
        char buffer[ 8 ];
        UInt32 wbytes = text.GetDataSize();
        ASSERT_FALSE( OSWRAP_TRUE == CORE::StringFromClipboard( buffer, sizeof( buffer ), &wbytes ) );
        ASSERT_TRUE( text.GetDataSize() == wbytes );

        // The C++ API
        CORE::CString asString;
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->GetText( asString ) );
        CORE::CUtf8String asUtf8 = CORE::ToUtf8String( asString );
        ASSERT_TRUE( asUtf8.ByteSize() == text.GetDataSize() + 1 );
        ASSERT_TRUE( 0 == memcmp( asUtf8.C_String(), text.GetConstBufferPtr(), text.GetDataSize() ) );

        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->SetText( CORE::CString( "set as CString \xE2\x82\xAC" ) ) );
        ASSERT_TRUE( ReadWithCApi( read, 64, terminatorsOk ) );
        ASSERT_TRUE( IsSameData( read, "set as CString \xE2\x82\xAC" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: When another client takes the clipboard StringFromClipboard() gets its text, and copying takes it back" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CORE::CDynamicBuffer ourText;
        AppendText( ourText, "copied by gucefCORE" );
        ASSERT_TRUE( PutOnClipboard( ourText ) );
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->IsClipboardOwner() );

        CTestX11ClipboardPeer peer;
        ASSERT_TRUE( peer.Open() );

        CORE::CDynamicBuffer foreignText;
        AppendText( foreignText, "copied by another client: \xC3\xBC\xE2\x82\xAC\xF0\x9D\x84\x9E" );
        ASSERT_TRUE( peer.TakeClipboard( foreignText, false, 65536 ) );
        ASSERT_TRUE( peer.Activate( GUCEF_NULL, 0, 0 ) );

        CORE::CDynamicBuffer read;
        bool terminatorsOk = false;
        ASSERT_TRUE( ReadWithCApi( read, 4096, terminatorsOk ) );
        ASSERT_TRUE( terminatorsOk );
        ASSERT_TRUE( IsSameData( read, foreignText ) );
        ASSERT_TRUE( peer.GetServedRequestCount() > 0 );

        // gucefCORE processed the SelectionClear of the new owner
        UInt32 waited = 0;
        while ( CORE::CX11Clipboard::Instance()->IsClipboardOwner() && waited < 2000 )
        {
            SleepMs( 10 );
            waited += 10;
        }
        ASSERT_FALSE( CORE::CX11Clipboard::Instance()->IsClipboardOwner() );

        // Copying again takes the clipboard back from the other client
        CORE::CDynamicBuffer backText;
        AppendText( backText, "copied again by gucefCORE" );
        ASSERT_TRUE( PutOnClipboard( backText ) );
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->IsClipboardOwner() );
        waited = 0;
        while ( !peer.HasLostClipboard() && waited < 2000 )
        {
            SleepMs( 10 );
            waited += 10;
        }
        ASSERT_TRUE( peer.HasLostClipboard() );
        peer.Deactivate( false, true );

        ::Atom type = None;
        int format = 0;
        bool wasIncr = false;
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "UTF8_STRING", read, type, format, wasIncr ) );
        ASSERT_TRUE( IsSameData( read, backText ) );
        ASSERT_TRUE( ReadWithCApi( read, 4096, terminatorsOk ) );
        ASSERT_TRUE( IsSameData( read, backText ) );

        // An owner that only knows STRING: the Latin-1 text arrives as UTF-8
        CORE::CDynamicBuffer stringText;
        AppendText( stringText, "na\xEFve r\xE9sum\xE9" );     // "naive resume" with i-diaeresis and e-acute, in ISO 8859-1
        ASSERT_TRUE( peer.TakeClipboard( stringText, true, 65536 ) );
        ASSERT_TRUE( peer.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( ReadWithCApi( read, 4096, terminatorsOk ) );
        ASSERT_TRUE( IsSameData( read, "na\xC3\xAFve r\xC3\xA9sum\xC3\xA9" ) );
        peer.Deactivate( false, true );

        peer.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: When the owner is gone the clipboard is empty and reading answers false" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CTestX11ClipboardPeer peer;
        ASSERT_TRUE( peer.Open() );
        CORE::CDynamicBuffer peerText;
        AppendText( peerText, "owner that goes away" );
        ASSERT_TRUE( peer.TakeClipboard( peerText, false, 65536 ) );
        peer.Close();    // the server forgets the owner with its window

        CORE::CDynamicBuffer read;
        bool terminatorsOk = false;
        ASSERT_FALSE( ReadWithCApi( read, 64, terminatorsOk ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: A text of several hundred KB goes both ways with INCR" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CORE::CDynamicBuffer largeText;
        BuildUtf8Text( largeText, 600 * 1024 );
        ASSERT_TRUE( PutOnClipboard( largeText ) );

        CTestX11ClipboardPeer peer;
        ASSERT_TRUE( peer.Open() );

        // Sent by gucefCORE in parts
        CORE::CDynamicBuffer data;
        ::Atom type = None;
        int format = 0;
        bool wasIncr = false;
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "UTF8_STRING", data, type, format, wasIncr, 5000 ) );
        ASSERT_TRUE( wasIncr );
        ASSERT_TRUE( type == peer.GetAtom( "UTF8_STRING" ) );
        ASSERT_TRUE( IsSameData( data, largeText ) );

        // Again, for a second transfer after the first one ended
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "text/plain;charset=utf-8", data, type, format, wasIncr, 5000 ) );
        ASSERT_TRUE( wasIncr );
        ASSERT_TRUE( IsSameData( data, largeText ) );

        // From memory, this process being the owner
        CORE::CDynamicBuffer read;
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->GetUtf8Text( read ) );
        ASSERT_TRUE( IsSameData( read, largeText ) );

        // Received by gucefCORE in parts of 64 KB from the other client
        CORE::CDynamicBuffer otherLargeText;
        BuildUtf8Text( otherLargeText, 700 * 1024 );
        AppendText( otherLargeText, "end of the other text" );
        ASSERT_TRUE( peer.TakeClipboard( otherLargeText, false, 65536 ) );
        ASSERT_TRUE( peer.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->GetUtf8Text( read ) );
        ASSERT_TRUE( IsSameData( read, otherLargeText ) );
        ASSERT_TRUE( peer.HasSentIncr() );

        bool terminatorsOk = false;
        ASSERT_TRUE( ReadWithCApi( read, otherLargeText.GetDataSize() + 1, terminatorsOk ) );
        ASSERT_TRUE( terminatorsOk );
        ASSERT_TRUE( IsSameData( read, otherLargeText ) );
        peer.Deactivate( false, true );

        peer.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: A requestor that goes away, also in the middle of an INCR transfer, does not disturb the owner" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CORE::CDynamicBuffer largeText;
        BuildUtf8Text( largeText, 900 * 1024 );
        ASSERT_TRUE( PutOnClipboard( largeText ) );

        // Starts a transfer and closes its connection, which destroys the window the owner writes the parts to
        bool wasIncr = false;
        for ( UInt32 i=0; i<3; ++i )
        {
            CTestX11ClipboardPeer leavingPeer;
            ASSERT_TRUE( leavingPeer.Open() );
            ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == leavingPeer.BeginConvert( "UTF8_STRING", wasIncr ) );
            ASSERT_TRUE( wasIncr );
            leavingPeer.Close();
        }

        // Requests whose requestor window is gone before the answer: X errors on the connection of the owner, which
        // the default handler of Xlib would end the process for
        CTestX11ClipboardPeer peer;
        ASSERT_TRUE( peer.Open() );
        ASSERT_TRUE( peer.RequestForVanishingWindow( "UTF8_STRING" ) );
        ASSERT_TRUE( peer.RequestForVanishingWindow( "TARGETS" ) );
        SleepMs( 100 );

        // The owner still serves the next requestor, and this process still owns the clipboard
        CORE::CDynamicBuffer data;
        ::Atom type = None;
        int format = 0;
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "UTF8_STRING", data, type, format, wasIncr, 5000 ) );
        ASSERT_TRUE( wasIncr );
        ASSERT_TRUE( IsSameData( data, largeText ) );
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->IsClipboardOwner() );
        peer.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: A text above the maximum size is refused and the previous text stays on the clipboard" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CORE::CDynamicBuffer previousText;
        AppendText( previousText, "the text before the one that is too large" );
        ASSERT_TRUE( PutOnClipboard( previousText ) );

        CORE::CDynamicBuffer tooLarge;
        UInt32 tooLargeSize = CORE::CX11Clipboard::MaxTextSizeInBytes + 1;
        ASSERT_TRUE( tooLarge.SetBufferSize( tooLargeSize + 1, false ) );
        memset( tooLarge.GetBufferPtr(), 'a', tooLargeSize );
        static_cast< char* >( tooLarge.GetBufferPtr() )[ tooLargeSize ] = '\0';
        tooLarge.SetDataSize( tooLargeSize + 1 );

        ASSERT_FALSE( OSWRAP_TRUE == CORE::StringToClipboard( static_cast< const char* >( tooLarge.GetConstBufferPtr() ) ) );
        ASSERT_FALSE( CORE::CX11Clipboard::Instance()->SetUtf8Text( static_cast< const char* >( tooLarge.GetConstBufferPtr() ), tooLargeSize ) );
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->IsClipboardOwner() );

        CTestX11ClipboardPeer peer;
        ASSERT_TRUE( peer.Open() );
        CORE::CDynamicBuffer data;
        ::Atom type = None;
        int format = 0;
        bool wasIncr = false;
        ASSERT_TRUE( CTestX11ClipboardPeer::CONVERTRESULT_OK == peer.Convert( "UTF8_STRING", data, type, format, wasIncr ) );
        ASSERT_TRUE( IsSameData( data, previousText ) );

        // Exactly the maximum is accepted
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->SetUtf8Text( static_cast< const char* >( tooLarge.GetConstBufferPtr() ), tooLargeSize - 1 ) );
        CORE::CDynamicBuffer read;
        ASSERT_TRUE( CORE::CX11Clipboard::Instance()->GetUtf8Text( read ) );
        ASSERT_TRUE( CORE::CX11Clipboard::MaxTextSizeInBytes == read.GetDataSize() );

        ASSERT_TRUE( PutOnClipboard( previousText ) );
        peer.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "X11Clipboard: MULTIPLE converts the targets it can and marks the others with None" )
        GUCEF_TESTFW_SKIP_IF( !HasTestDisplay(), NoDisplayReason );
    try
    {
        CORE::CDynamicBuffer text;
        AppendText( text, "multiple \xE2\x82\xAC" );
        ASSERT_TRUE( PutOnClipboard( text ) );

        CTestX11ClipboardPeer peer;
        ASSERT_TRUE( peer.Open() );
        CORE::CDynamicBuffer firstData;
        bool firstConverted = false;
        bool secondConverted = true;
        ASSERT_TRUE( peer.ConvertMultiple( "UTF8_STRING", "image/png", firstData, firstConverted, secondConverted ) );
        ASSERT_TRUE( firstConverted );
        ASSERT_FALSE( secondConverted );
        ASSERT_TRUE( IsSameData( firstData, text ) );
        peer.Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    #endif /* GUCEF_LINUX_BUILD ? */
}

/*-------------------------------------------------------------------------*/
