/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#define GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_ITLSPROVIDER_H
#include "gucefCOMCORE_ITlsProvider.h"
#define GUCEF_COMCORE_ITLSPROVIDER_H
#endif /* GUCEF_COMCORE_ITLSPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The TLS providers of the process, in priority order like the codec registry: the operating system
 *  first (100), then a library of the system that is loaded when asked for (50), then plugins (25).
 *  Registration is cheap and probes nothing: a provider checks whether it can run when it is first asked
 *  (ITlsProvider::IsAvailable()), so nothing loads in a process that never uses TLS.
 *
 *  Held by CComCoreGlobal. The application decides what it means when no provider is available: the layers
 *  above only report why.
 */
class GUCEF_COMCORE_EXPORT_CPP CTlsProviderRegistry
{
    public:

    static const Int32 PluginTlsProviderPriority;           /**< 25: a plugin that brings its own TLS library */
    static const Int32 SystemLibraryTlsProviderPriority;    /**< 50: the TLS library of the system, loaded when asked for */
    static const Int32 OsTlsProviderPriority;               /**< 100: the TLS of the operating system (SChannel, SecureTransport) */

    /** A provider with what is known about it */
    struct STlsProviderStatus
    {
        TITlsProviderPtr provider;
        Int32 priority;
        bool available;             /**< ITlsProvider::IsAvailable() */
        CORE::CString reason;       /**< why not, when it is not available */
    };
    typedef struct STlsProviderStatus TTlsProviderStatus;
    typedef GUCEF::vector< STlsProviderStatus > TTlsProviderStatusVector;

    CTlsProviderRegistry( void );

    ~CTlsProviderRegistry();

    /**
     *  Adds a provider. Equal priorities keep the order of registration.
     *  @return false when this provider object is registered already
     */
    bool RegisterProvider( const TITlsProviderPtr& provider ,
                           Int32 priority                   );

    /** Takes the provider out, @return false when it was not registered */
    bool UnregisterProvider( const TITlsProviderPtr& provider );

    void UnregisterAll( void );

    UInt32 GetProviderCount( void ) const;

    /**
     *  The available provider with the highest priority that has all the needed capabilities (ETlsCapability bits).
     *  When there is none a NULL pointer is returned and whyNot says why for each provider, for example
     *  "openssl-dynamic: libssl.so.3: cannot open shared object file: No such file or directory".
     */
    TITlsProviderPtr GetBestAvailable( UInt32 neededCapabilities ,
                                       CORE::CString& whyNot     ) const;

    /** Whether there is an available provider at this moment, with the reasons when there is not */
    bool IsAnyAvailable( CORE::CString& whyNot ) const;

    /** All providers, highest priority first, each with its availability. Probes the ones that were not asked before */
    void GetProviderStatus( TTlsProviderStatusVector& status ) const;

    private:

    CTlsProviderRegistry( const CTlsProviderRegistry& src );
    CTlsProviderRegistry& operator=( const CTlsProviderRegistry& src );

    struct SRegistration
    {
        TITlsProviderPtr provider;
        Int32 priority;
    };
    typedef GUCEF::vector< SRegistration > TRegistrationVector;

    private:

    TRegistrationVector m_registrations;    /**< highest priority first */
    MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H ? */
