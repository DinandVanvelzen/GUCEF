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

#ifndef GUCEF_CORE_TESTAPP_TESTX11CLIPBOARDPEER_H
#define GUCEF_CORE_TESTAPP_TESTX11CLIPBOARDPEER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifdef GUCEF_LINUX_BUILD

#include <X11/Xlib.h>

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The other X client of the clipboard tests: a second connection to the X server, made by the test itself, that asks
 *  the owner of CLIPBOARD for its content (with INCR for large data) and that can own CLIPBOARD itself, serving the
 *  requests of other clients from its own thread (Activate() / Deactivate() of the active object).
 *  Every use of the connection is done with the lock of the peer held.
 */
class CTestX11ClipboardPeer : public GUCEF::MT::CActiveObject
{
    public:

    enum EConvertResult
    {
        CONVERTRESULT_OK = 0,
        CONVERTRESULT_REFUSED,
        CONVERTRESULT_TIMEOUT,
        CONVERTRESULT_FAILED
    };

    CTestX11ClipboardPeer( void );

    virtual ~CTestX11ClipboardPeer();

    bool Open( void );

    void Close( void );

    ::Atom GetAtom( const char* name );

    /**
     *  Asks the owner of CLIPBOARD for the target and reads the answer, also when it comes with INCR.
     *  Data of format 32 comes as the longs that Xlib returns.
     */
    EConvertResult Convert( const char* targetName       ,
                            GUCEF::CORE::CDynamicBuffer& data ,
                            ::Atom& type                 ,
                            int& format                  ,
                            bool& wasIncr                ,
                            GUCEF::UInt32 timeoutInMs = 2000 );

    /**
     *  Asks the owner of CLIPBOARD for the target and reads (deletes) only the first answer: an INCR transfer is started
     *  and left unfinished, as a requestor that goes away in the middle of it would
     */
    EConvertResult BeginConvert( const char* targetName, bool& wasIncr );

    /**
     *  Asks the owner of CLIPBOARD for the target on behalf of a second window and destroys that window right away, so
     *  that the owner answers to a window that no longer exists (an X error on the connection of the owner)
     */
    bool RequestForVanishingWindow( const char* targetName );

    /** Asks for MULTIPLE with two targets; tells which of them the owner converted, and the data of the first */
    bool ConvertMultiple( const char* firstTarget              ,
                          const char* secondTarget             ,
                          GUCEF::CORE::CDynamicBuffer& firstData ,
                          bool& firstConverted                 ,
                          bool& secondConverted                );

    /**
     *  Makes this connection the owner of CLIPBOARD with the given text, served as UTF8_STRING (or as STRING only when
     *  asLatin1String), in parts of incrChunkSize bytes with INCR when it is larger than that
     */
    bool TakeClipboard( const GUCEF::CORE::CDynamicBuffer& text, bool asLatin1String, GUCEF::UInt32 incrChunkSize );

    /** Whether this connection received the SelectionClear of a new owner */
    bool HasLostClipboard( void ) const;

    /** Whether this connection sent the text with INCR */
    bool HasSentIncr( void ) const;

    /** The number of requests this connection answered as owner */
    GUCEF::UInt32 GetServedRequestCount( void ) const;

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool WaitForEvent( int eventType, ::Atom atom, ::XEvent& event, GUCEF::UInt32 timeoutInMs );

    void ProcessOwnerEvent( ::XEvent& event );

    void OnSelectionRequest( const ::XSelectionRequestEvent& request );

    CTestX11ClipboardPeer( const CTestX11ClipboardPeer& src );
    CTestX11ClipboardPeer& operator=( const CTestX11ClipboardPeer& src );

    private:

    GUCEF::MT::CMutex m_peerLock;
    ::Display* m_display;
    ::Window m_window;
    ::Atom m_atomClipboard;
    ::Atom m_atomProperty;

    GUCEF::CORE::CDynamicBuffer m_ownText;
    bool m_ownTextAsLatin1;
    GUCEF::UInt32 m_incrChunkSize;
    bool m_lostClipboard;
    bool m_sentIncr;
    GUCEF::UInt32 m_servedRequests;

    ::Window m_incrRequestor;
    ::Atom m_incrProperty;
    ::Atom m_incrType;
    GUCEF::UInt32 m_incrOffset;
    bool m_incrActive;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LINUX_BUILD ? */

#endif /* GUCEF_CORE_TESTAPP_TESTX11CLIPBOARDPEER_H ? */
