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

#ifndef GUCEF_AUTH_PLUGINSTESTAPP_TESTPLUGINSCOMMON_H
#define GUCEF_AUTH_PLUGINSTESTAPP_TESTPLUGINSCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The timers, notifiers and sockets of the providers run on the global pulse generator. Tests that wait for
 *  something pulse it from the test thread, so the providers run on that thread like they do in an application.
 */

/** Pulses the global pulse generator from the test thread for the given time */
inline void
PumpFor( GUCEF::UInt32 durationInMs )
{GUCEF_TRACE;

    for ( GUCEF::UInt32 waited=0; waited<durationInMs; ++waited )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

/** Pulses until the future has ended; true if it did in time */
inline bool
PumpUntilReady( const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

/** Loads a plugin from the directory of the module, debug or release build, into a group of its own */
inline bool
LoadTestPlugin( const GUCEF::CORE::CString& pluginName, const GUCEF::CORE::CString& group = "test" )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    GUCEF::CORE::CPluginControl& pluginControl = GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, group, true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, group, true );
}

/*-------------------------------------------------------------------------*/

/**
 *  Whether a HTTP client for https is there, loading the curl plugin as the shipped game has it. The plugin is in a group of its own
 *  which the tests unload at the end, after the HTTP client of the login engine let go of its backend
 */
inline bool
HttpsBackendAvailable( void )
{GUCEF_TRACE;

    if ( !GUCEF::WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().IsConstructible( "curl" ) )
        LoadTestPlugin( "httpclientpluginCURL", "https" );
    return GUCEF::WEB::CHTTPClient::IsSchemeSupported( "https" );
}

/*-------------------------------------------------------------------------*/

/** Whether the "json" data store codec is there, loading its plugin when it is not */
inline bool
JsonCodecAvailable( void )
{GUCEF_TRACE;

    GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
        LoadTestPlugin( "dstorepluginJSONPARSER" );
    return GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec );
}

/*-------------------------------------------------------------------------*/

/** Collects everything that is logged while it exists, to prove what does and does not end up in a log */
class CLogCapture
{
    public:

    CLogCapture( void )
        : m_buffer()
        , m_access( m_buffer )
        , m_logger( m_access )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_logger );
    }

    ~CLogCapture()
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &m_logger );
    }

    /** Whether the text was logged */
    bool Contains( const GUCEF::CORE::CString& text )
    {
        return Text().HasSubstr( text ) >= 0;
    }

    GUCEF::CORE::CString Text( void )
    {
        GUCEF::CORE::CLogStreamScope::FlushLogs();
        GUCEF::CORE::CString all;
        if ( m_buffer.GetDataSize() > 0 )
            all = GUCEF::CORE::CString( m_buffer.AsConstTypePtr< char >(), m_buffer.GetDataSize() );
        return all;
    }

    private:

    CLogCapture( const CLogCapture& src );
    CLogCapture& operator=( const CLogCapture& src );

    private:

    GUCEF::CORE::CDynamicBuffer m_buffer;
    GUCEF::CORE::CDynamicBufferAccess m_access;
    GUCEF::CORE::CStdLogger m_logger;
};

/*-------------------------------------------------------------------------*/

/** Records the events of a provider, in order, with the thread they were delivered on */
class CProviderRecorder : public GUCEF::CORE::CObserver
{
    public:

    enum EKind
    {
        KIND_SUCCEEDED, KIND_FAILED, KIND_ACTION
    };

    struct SRecord
    {
        EKind kind;
        GUCEF::UInt32 threadId;
        GUCEF::AUTH::CAuthResult result;
        GUCEF::AUTH::CAuthFailure failure;
        GUCEF::AUTH::CAuthActionRequired action;
    };

    GUCEF::vector< SRecord > records;

    CProviderRecorder( void ) : GUCEF::CORE::CObserver(), records() {}

    virtual ~CProviderRecorder() { UnsubscribeAllFromObserver(); }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CProviderRecorder";
        return classTypeName;
    }

    GUCEF::UInt32 Count( EKind kind ) const
    {
        GUCEF::UInt32 count = 0;
        for ( size_t i=0; i<records.size(); ++i )
            if ( records[ i ].kind == kind )
                ++count;
        return count;
    }

    /** The last record of the kind, GUCEF_NULL when there is none */
    const SRecord* Last( EKind kind ) const
    {
        for ( size_t i=records.size(); i>0; --i )
            if ( records[ i-1 ].kind == kind )
                return &records[ i-1 ];
        return GUCEF_NULL;
    }

    /** Pulses until the recorder has seen the kind the given number of times; true if it did in time */
    bool PumpUntil( EKind kind, GUCEF::UInt32 count, GUCEF::UInt32 timeoutInMs )
    {
        for ( GUCEF::UInt32 waited=0; waited<timeoutInMs && Count( kind ) < count; ++waited )
            PumpFor( 1 );
        return Count( kind ) >= count;
    }

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF;
        SRecord record;
        record.threadId = GUCEF::MT::GetCurrentTaskID();
        if ( AUTH::CIAuthProvider::LoginSucceededEvent == eventId )
        {
            record.kind = KIND_SUCCEEDED;
            record.result = *static_cast< AUTH::CAuthResult* >( eventData );
        }
        else
        if ( AUTH::CIAuthProvider::LoginFailedEvent == eventId )
        {
            record.kind = KIND_FAILED;
            record.failure = *static_cast< AUTH::CAuthFailure* >( eventData );
        }
        else
        if ( AUTH::CIAuthProvider::ActionRequiredEvent == eventId )
        {
            record.kind = KIND_ACTION;
            record.action = *static_cast< AUTH::CAuthActionRequired* >( eventData );
        }
        else
        {
            return;
        }
        records.push_back( record );
    }
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_PLUGINSTESTAPP_TESTPLUGINSCOMMON_H ? */
