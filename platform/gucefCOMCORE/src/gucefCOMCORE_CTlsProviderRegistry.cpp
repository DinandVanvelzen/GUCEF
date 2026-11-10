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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefCOMCORE_CTlsProviderRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const Int32 CTlsProviderRegistry::PluginTlsProviderPriority = 25;
const Int32 CTlsProviderRegistry::SystemLibraryTlsProviderPriority = 50;
const Int32 CTlsProviderRegistry::OsTlsProviderPriority = 100;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTlsProviderRegistry::CTlsProviderRegistry( void )
    : m_registrations()
    , m_dataLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTlsProviderRegistry::~CTlsProviderRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CTlsProviderRegistry::RegisterProvider( const TITlsProviderPtr& provider ,
                                        Int32 priority                   )
{GUCEF_TRACE;

    if ( provider.IsNULL() )
        return false;

    MT::CScopeMutex lock( m_dataLock );

    for ( TRegistrationVector::const_iterator i = m_registrations.begin(); i != m_registrations.end(); ++i )
        if ( (*i).provider.GetPointerAlways() == provider.GetPointerAlways() )
            return false;

    // Behind everything with an equal or higher priority
    TRegistrationVector::iterator pos = m_registrations.begin();
    while ( pos != m_registrations.end() && (*pos).priority >= priority )
        ++pos;

    SRegistration registration;
    registration.provider = provider;
    registration.priority = priority;
    m_registrations.insert( pos, registration );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTlsProviderRegistry::UnregisterProvider( const TITlsProviderPtr& provider )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    for ( TRegistrationVector::iterator i = m_registrations.begin(); i != m_registrations.end(); ++i )
    {
        if ( (*i).provider.GetPointerAlways() == provider.GetPointerAlways() )
        {
            m_registrations.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTlsProviderRegistry::UnregisterAll( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_registrations.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CTlsProviderRegistry::GetProviderCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return (UInt32) m_registrations.size();
}

/*-------------------------------------------------------------------------*/

void
CTlsProviderRegistry::GetProviderStatus( TTlsProviderStatusVector& status ) const
{GUCEF_TRACE;

    // The providers are asked outside of the lock: probing can take a while and a provider may call back into the registry
    TRegistrationVector registrations;
    {
        MT::CScopeMutex lock( m_dataLock );
        registrations = m_registrations;
    }

    status.clear();
    status.reserve( registrations.size() );
    for ( TRegistrationVector::const_iterator i = registrations.begin(); i != registrations.end(); ++i )
    {
        STlsProviderStatus entry;
        entry.provider = (*i).provider;
        entry.priority = (*i).priority;
        entry.available = (*i).provider->IsAvailable( entry.reason );
        if ( entry.available )
            entry.reason.Clear();
        status.push_back( entry );
    }
}

/*-------------------------------------------------------------------------*/

TITlsProviderPtr
CTlsProviderRegistry::GetBestAvailable( UInt32 neededCapabilities ,
                                        CORE::CString& whyNot     ) const
{GUCEF_TRACE;

    whyNot.Clear();

    TTlsProviderStatusVector status;
    GetProviderStatus( status );

    for ( TTlsProviderStatusVector::const_iterator i = status.begin(); i != status.end(); ++i )
    {
        const STlsProviderStatus& entry = (*i);
        CORE::CString name = entry.provider->GetProviderName();
        if ( !entry.available )
        {
            if ( !whyNot.IsNULLOrEmpty() )
                whyNot += "; ";
            whyNot += name + ": " + entry.reason;
            continue;
        }

        if ( neededCapabilities != ( neededCapabilities & entry.provider->GetCapabilities() ) )
        {
            if ( !whyNot.IsNULLOrEmpty() )
                whyNot += "; ";
            whyNot += name + ": lacks a needed capability";
            continue;
        }

        whyNot.Clear();
        return entry.provider;
    }

    if ( status.empty() )
        whyNot = "no TLS provider is registered on this platform";
    return TITlsProviderPtr();
}

/*-------------------------------------------------------------------------*/

bool
CTlsProviderRegistry::IsAnyAvailable( CORE::CString& whyNot ) const
{GUCEF_TRACE;

    return !GetBestAvailable( TLSCAP_NONE, whyNot ).IsNULL();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
