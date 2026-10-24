/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "CCodecRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const Int32 CCodecRegistry::SoftwareCodecPriority = 25;
const Int32 CCodecRegistry::PluginCodecPriority = 50;
const Int32 CCodecRegistry::OsCodecPriority = 100;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CCodecRegistry::CCodecRegistry( void )
    : CTONRegistry< CTONRegistry< CICodec, MT::CMutex >, MT::CMutex >()
    , m_dataLock()
    , m_registrations()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCodecRegistry::~CCodecRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CCodecRegistry::AsLockable( void ) const
{GUCEF_TRACE;

    return CTONRegistry< CTONRegistry< CICodec, MT::CMutex >, MT::CMutex >::AsLockable();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CCodecRegistry::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_dataLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CCodecRegistry::Unlock( void ) const
{GUCEF_TRACE;

    return m_dataLock.Unlock();
}

/*-------------------------------------------------------------------------*/

CString
CCodecRegistry::GetRegistrationKey( const CString& codecFamily, const CString& codecType )
{GUCEF_TRACE;

    // family and type are looked up without regard to case, as TryGetCodec() always did
    return codecFamily.Lowercase() + "\n" + codecType.Lowercase();
}

/*-------------------------------------------------------------------------*/

CCodecRegistry::TICodecPtr 
CCodecRegistry::TryGetCodec( const CString& codecFamily , 
                             const CString& codecName   ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    TCodecTypeRegistrationsMap::const_iterator i = m_registrations.find( GetRegistrationKey( codecFamily, codecName ) );
    if ( i != m_registrations.end() && !(*i).second.registrations.empty() )
        return (*i).second.registrations.front().codec;

    // A codec that was put into a family registry directly
    TCodecFamilyRegistryPtr codecFamilyRegistry;
    if ( TryLookup( codecFamily, codecFamilyRegistry, false ) && codecFamilyRegistry )
    {
        TICodecPtr codec;
        if ( codecFamilyRegistry->TryLookup( codecName, codec, false ) && codec )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CodecRegistry::TryGetCodec: Obtained codec " << codecName << " from codec family " << codecFamily );
            return codec;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CodecRegistry::TryGetCodec: Failed to obtain codec " << codecName << " from codec family " << codecFamily );
    return TICodecPtr();
}

/*-------------------------------------------------------------------------*/

CCodecRegistry::TCodecFamilyRegistryPtr
CCodecRegistry::GetOrCreateFamilyRegistry( const CString& codecFamily )
{GUCEF_TRACE;

    TCodecFamilyRegistryPtr familyRegistry;
    if ( TryLookup( codecFamily, familyRegistry, true ) && familyRegistry )
        return familyRegistry;

    familyRegistry = TCodecFamilyRegistryTypedPtr( GUCEF_NEW TCodecFamilyRegistry() );
    if ( TryRegister( codecFamily, familyRegistry ) )
        return familyRegistry;

    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CodecRegistry:GetOrCreateFamilyRegistry: Unable to register codec family \"" << codecFamily << "\"" );
    return TCodecFamilyRegistryPtr();
}

/*-------------------------------------------------------------------------*/

void
CCodecRegistry::UpdateFamilyRegistry( const SCodecTypeRegistrations& typeRegistrations )
{GUCEF_TRACE;

    const CString& codecType = typeRegistrations.codecType;
    if ( typeRegistrations.registrations.empty() )
    {
        TCodecFamilyRegistryPtr familyRegistry;
        if ( TryLookup( typeRegistrations.codecFamily, familyRegistry, true ) && familyRegistry )
            familyRegistry->TryUnregister( codecType );
        return;
    }

    TCodecFamilyRegistryPtr familyRegistry = GetOrCreateFamilyRegistry( typeRegistrations.codecFamily );
    if ( !familyRegistry )
        return;

    const TICodecPtr& top = typeRegistrations.registrations.front().codec;
    TICodecPtr current;
    if ( familyRegistry->TryLookup( codecType, current, true ) )
    {
        if ( current == top )
            return;
        familyRegistry->TryUnregister( codecType );
    }
    if ( !familyRegistry->TryRegister( codecType, top ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CodecRegistry:UpdateFamilyRegistry: Unable to register codec \"" << codecType <<
            "\" in codec family \"" << typeRegistrations.codecFamily << "\"" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CCodecRegistry::RegisterCodec( const TICodecPtr& codec ,
                               Int32 priority          )
{GUCEF_TRACE;

    if ( codec.IsNULL() )
        return false;
    return RegisterCodec( codec->GetFamilyName(), codec->GetType(), codec, priority );
}

/*-------------------------------------------------------------------------*/

bool
CCodecRegistry::RegisterCodec( const CString& codecFamily ,
                               const CString& codecType   ,
                               const TICodecPtr& codec    ,
                               Int32 priority             )
{GUCEF_TRACE;

    if ( codec.IsNULL() || codecFamily.IsNULLOrEmpty() || codecType.IsNULLOrEmpty() )
        return false;

    MT::CScopeMutex lock( m_dataLock );

    SCodecTypeRegistrations& typeRegistrations = m_registrations[ GetRegistrationKey( codecFamily, codecType ) ];
    TCodecRegistrationVector& registrations = typeRegistrations.registrations;
    if ( registrations.empty() )
    {
        typeRegistrations.codecFamily = codecFamily;
        typeRegistrations.codecType = codecType;

        // A codec that was put into the family registry directly becomes a registration of its own
        TCodecFamilyRegistryPtr familyRegistry;
        TICodecPtr existing;
        if ( TryLookup( codecFamily, familyRegistry, true ) && familyRegistry &&
             familyRegistry->TryLookup( codecType, existing, true ) && !existing.IsNULL() && existing != codec )
        {
            SCodecRegistration adopted;
            adopted.codec = existing;
            adopted.priority = PluginCodecPriority;
            registrations.push_back( adopted );
        }
    }

    TCodecRegistrationVector::iterator i = registrations.begin();
    while ( i != registrations.end() )
    {
        if ( (*i).codec == codec )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CodecRegistry:RegisterCodec: The codec is already registered as \"" << codecType <<
                "\" in codec family \"" << codecFamily << "\"" );
            return false;
        }
        ++i;
    }

    // after all registrations of the same or a higher priority
    i = registrations.begin();
    while ( i != registrations.end() && (*i).priority >= priority )
        ++i;
    SCodecRegistration registration;
    registration.codec = codec;
    registration.priority = priority;
    registrations.insert( i, registration );

    UpdateFamilyRegistry( typeRegistrations );

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "CodecRegistry:RegisterCodec: Registered codec \"" << codecType << "\" in codec family \"" <<
        codecFamily << "\" with priority " << priority << ", " << registrations.size() << " provider(s) of it now" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCodecRegistry::UnregisterCodec( const TICodecPtr& codec )
{GUCEF_TRACE;

    if ( codec.IsNULL() )
        return false;

    MT::CScopeMutex lock( m_dataLock );

    bool found = false;
    TCodecTypeRegistrationsMap::iterator i = m_registrations.begin();
    while ( i != m_registrations.end() )
    {
        SCodecTypeRegistrations& typeRegistrations = (*i).second;
        TCodecRegistrationVector& registrations = typeRegistrations.registrations;
        bool changed = false;
        TCodecRegistrationVector::iterator n = registrations.begin();
        while ( n != registrations.end() )
        {
            if ( (*n).codec == codec )
            {
                n = registrations.erase( n );
                changed = true;
            }
            else
                ++n;
        }

        if ( changed )
        {
            found = true;
            UpdateFamilyRegistry( typeRegistrations );
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "CodecRegistry:UnregisterCodec: Unregistered a provider of codec \"" << typeRegistrations.codecType <<
                "\" in codec family \"" << typeRegistrations.codecFamily << "\", " << registrations.size() << " provider(s) of it left" );
        }

        if ( registrations.empty() )
            m_registrations.erase( i++ );
        else
            ++i;
    }

    if ( !found )
    {
        // A codec that was put into the family registry directly
        TCodecFamilyRegistryPtr familyRegistry;
        TICodecPtr existing;
        CString codecType = codec->GetType();
        if ( TryLookup( codec->GetFamilyName(), familyRegistry, true ) && familyRegistry &&
             familyRegistry->TryLookup( codecType, existing, true ) && existing == codec )
        {
            found = familyRegistry->TryUnregister( codecType );
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

bool
CCodecRegistry::GetCodecs( const CString& codecFamily ,
                           const CString& codecType   ,
                           TICodecPtrVector& codecs   ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    TCodecTypeRegistrationsMap::const_iterator i = m_registrations.find( GetRegistrationKey( codecFamily, codecType ) );
    if ( i != m_registrations.end() && !(*i).second.registrations.empty() )
    {
        const TCodecRegistrationVector& registrations = (*i).second.registrations;
        TCodecRegistrationVector::const_iterator n = registrations.begin();
        while ( n != registrations.end() )
        {
            codecs.push_back( (*n).codec );
            ++n;
        }
        return true;
    }

    // A codec that was put into a family registry directly
    TCodecFamilyRegistryPtr familyRegistry;
    TICodecPtr codec;
    if ( TryLookup( codecFamily, familyRegistry, false ) && familyRegistry &&
         familyRegistry->TryLookup( codecType, codec, false ) && !codec.IsNULL() )
    {
        codecs.push_back( codec );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCodecRegistry::TryGetCodecPriority( const CString& codecFamily ,
                                     const CString& codecType   ,
                                     const CICodec* codec       ,
                                     Int32& priority            ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    TCodecTypeRegistrationsMap::const_iterator i = m_registrations.find( GetRegistrationKey( codecFamily, codecType ) );
    if ( i != m_registrations.end() )
    {
        const TCodecRegistrationVector& registrations = (*i).second.registrations;
        TCodecRegistrationVector::const_iterator n = registrations.begin();
        while ( n != registrations.end() )
        {
            if ( (*n).codec.GetPointerAlways() == codec )
            {
                priority = (*n).priority;
                return true;
            }
            ++n;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CCodecRegistry::GetHeldReferenceCount( const CICodec* codec ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == codec )
        return 0;

    MT::CScopeMutex lock( m_dataLock );

    UInt32 count = 0;
    TCodecTypeRegistrationsMap::const_iterator i = m_registrations.begin();
    while ( i != m_registrations.end() )
    {
        const TCodecRegistrationVector& registrations = (*i).second.registrations;
        TCodecRegistrationVector::const_iterator n = registrations.begin();
        while ( n != registrations.end() )
        {
            if ( (*n).codec.GetPointerAlways() == codec )
                ++count;
            ++n;
        }

        // the one with the highest priority is held by the family registry as well
        if ( !registrations.empty() && registrations.front().codec.GetPointerAlways() == codec )
            ++count;
        ++i;
    }

    if ( 0 == count )
    {
        // A codec that was put into the family registry directly
        TCodecFamilyRegistryPtr familyRegistry;
        TICodecPtr existing;
        if ( TryLookup( codec->GetFamilyName(), familyRegistry, true ) && familyRegistry &&
             familyRegistry->TryLookup( codec->GetType(), existing, true ) && existing.GetPointerAlways() == codec )
        {
            count = 1;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------*/

void
CCodecRegistry::UnregisterAll( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );

    m_registrations.clear();
    CTONRegistry< CTONRegistry< CICodec, MT::CMutex >, MT::CMutex >::UnregisterAll();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/