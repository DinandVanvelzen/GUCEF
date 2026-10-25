/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEHTTP_H
#define GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEHTTP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A web server of the test that answers every request with the scripted answer, on 127.0.0.1 and a free port, with a
 *  thread of its own: the code under test blocks the test thread in a request (the server role of a provider), so the
 *  server cannot run on it. It keeps the requests it got.
 */
class CFakeHttpServer : public GUCEF::CORE::CObserver
{
    public:

    CFakeHttpServer( void );

    virtual ~CFakeHttpServer();

    /** Starts on 127.0.0.1 and a free port; false when it cannot */
    bool Start( void );

    void Stop( void );

    GUCEF::UInt16 GetPort( void ) const;

    /** "http://127.0.0.1:port" */
    GUCEF::CORE::CString GetBaseUrl( void ) const;

    /** What the next requests are answered with */
    void SetResponse( int status, const GUCEF::CORE::CString& body, const GUCEF::CORE::CString& contentType = "application/json" );

    /** Answers with this many bytes of body instead of the scripted one, to test the size limit */
    void SetLargeResponse( GUCEF::UInt32 bodySize );

    GUCEF::UInt32 GetRequestCount( void ) const;

    /** The first line of the last request, for example "GET /path?x=1 HTTP/1.1" */
    GUCEF::CORE::CString GetLastRequestLine( void ) const;

    /** The body of the last request */
    GUCEF::CORE::CString GetLastRequestBody( void ) const;

    /** The head and body of everything that was received, for a check that a secret was only sent where it should be */
    GUCEF::CORE::CString GetLastRequestText( void ) const;

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::CORE::CTEventHandlerFunctor< CFakeHttpServer > TEventCallback;

    void OnClientConnected( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void OnClientDisconnected( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void OnDataReceived( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );

    class CThread : public GUCEF::MT::CActiveObject
    {
        public:
        explicit CThread( CFakeHttpServer& owner );
        virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;
        protected:
        virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
        virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
        private:
        CFakeHttpServer& m_owner;
    };
    friend class CThread;

    struct SConnection
    {
        GUCEF::COMCORE::CTCPServerConnection* connection;
        GUCEF::CORE::CString received;
        bool answered;
        SConnection( void ) : connection( GUCEF_NULL ), received(), answered( false ) {}
    };

    void Pulse( void );
    void TryAnswer( SConnection& state );

    private:

    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::COMCORE::CTCPServerSocket m_socket;
    CThread m_thread;
    bool m_running;
    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::map< void*, SConnection > m_connections;
    int m_status;
    GUCEF::CORE::CString m_body;
    GUCEF::CORE::CString m_contentType;
    GUCEF::UInt32 m_largeBodySize;
    GUCEF::UInt32 m_requestCount;
    GUCEF::CORE::CString m_lastRequestLine;
    GUCEF::CORE::CString m_lastRequestBody;
    GUCEF::CORE::CString m_lastRequestText;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEHTTP_H ? */
