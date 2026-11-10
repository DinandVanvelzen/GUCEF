/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTPUMP_H
#define GUCEF_AUTH_TESTAPP_TESTPUMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The sockets, timers and notifiers the tests use run on the global pulse generator. Tests that wait for
 *  something pulse it from the test thread, so everything runs on one thread.
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

/** Pulses until the flag, which an observer sets, is true; true if it became true in time */
inline bool
PumpUntilTrue( const bool& flag, GUCEF::UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs && !flag; ++waited )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
    return flag;
}

/*-------------------------------------------------------------------------*/

/** Pulses until the counter, which an observer increments, reaches the target; true if it did in time */
inline bool
PumpUntilAtLeast( const GUCEF::UInt32& counter, GUCEF::UInt32 target, GUCEF::UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs && counter < target; ++waited )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
    return counter >= target;
}

/*-------------------------------------------------------------------------*/

/** Loads a plugin from the directory of the module, debug or release build */
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
 *  Whether a HTTP client for https is there, loading the curl plugin as the shipped game has it (the plugin goes first when it is loaded).
 *  The plugin is in a group of its own which the tests unload at the end, after the HTTP client of the login engine let go of its backend
 */
inline bool
HttpsBackendAvailable( void )
{GUCEF_TRACE;

    if ( !GUCEF::WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().IsConstructible( "curl" ) )
        LoadTestPlugin( "httpclientpluginCURL", "https" );
    return GUCEF::WEB::CHTTPClient::IsSchemeSupported( "https" );
}

/*-------------------------------------------------------------------------*/

/** A system without a TLS library, for as long as the object lives: the TLS providers of the process are taken out of the registry and put back */
class CScopedNoTlsProviders
{
    public:

    CScopedNoTlsProviders( void )
        : m_providers()
    {
        GUCEF::COMCORE::CTlsProviderRegistry& registry = GUCEF::COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry();
        registry.GetProviderStatus( m_providers );
        registry.UnregisterAll();
    }

    ~CScopedNoTlsProviders()
    {
        GUCEF::COMCORE::CTlsProviderRegistry& registry = GUCEF::COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry();
        for ( size_t i=0; i<m_providers.size(); ++i )
            registry.RegisterProvider( m_providers[ i ].provider, m_providers[ i ].priority );
    }

    private:

    CScopedNoTlsProviders( const CScopedNoTlsProviders& src );
    CScopedNoTlsProviders& operator=( const CScopedNoTlsProviders& src );

    GUCEF::COMCORE::CTlsProviderRegistry::TTlsProviderStatusVector m_providers;
};

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

/** Whether the "xml" data store codec is there, loading its plugin when it is not */
inline bool
XmlCodecIsAvailable( void )
{GUCEF_TRACE;

    GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetXmlCodec( codec ) )
        LoadTestPlugin( "dstorepluginPARSIFALXML" );
    return GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetXmlCodec( codec );
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTPUMP_H ? */
