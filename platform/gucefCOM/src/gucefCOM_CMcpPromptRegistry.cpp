/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOM_CMcpPromptRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CMcpPromptRegistry::PromptRegisteredEvent = "GUCEF::COM::CMcpPromptRegistry::PromptRegisteredEvent";
const CORE::CEvent CMcpPromptRegistry::PromptUnregisteredEvent = "GUCEF::COM::CMcpPromptRegistry::PromptUnregisteredEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpPromptEntry::CMcpPromptEntry( void )
    : prompt()
    , descriptor()
    , descriptorNode()
    , generation( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMcpPromptRegistry::RegisterEvents( void )
{GUCEF_TRACE;

    CMcpRegistryBase::RegisterEvents();
    PromptRegisteredEvent.Initialize();
    PromptUnregisteredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMcpPromptRegistry::CMcpPromptRegistry( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CMcpRegistryBase( pulseGenerator )
    , m_entries()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMcpPromptRegistry::~CMcpPromptRegistry()
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwLock );
    m_entries.clear();
}

/*-------------------------------------------------------------------------*/

bool
CMcpPromptRegistry::Register( const CIMcpPromptPtr& prompt, CORE::CString& error )
{GUCEF_TRACE;

    error.Clear();
    if ( prompt.IsNULL() )
    {
        error = "no prompt";
        return false;
    }
    CMcpPromptEntryPtr entry( GUCEF_NEW CMcpPromptEntry() );
    entry->prompt = prompt;
    if ( !prompt->GetDescriptor( entry->descriptor ) )
        error = "the prompt gave no descriptor";
    else
    if ( entry->descriptor.name.IsNULLOrEmpty() )
        error = "a prompt needs a name";
    else
    {
        const CMcpPromptDescriptor::TArgumentVector& arguments = entry->descriptor.arguments;
        for ( size_t i=0; i<arguments.size() && error.IsNULLOrEmpty(); ++i )
        {
            if ( arguments[ i ].name.IsNULLOrEmpty() )
                error = "an argument of the prompt \"" + entry->descriptor.name + "\" has no name";
            for ( size_t n=0; n<i && error.IsNULLOrEmpty(); ++n )
                if ( arguments[ n ].name == arguments[ i ].name )
                    error = "the prompt \"" + entry->descriptor.name + "\" names the argument \"" + arguments[ i ].name + "\" twice";
        }
    }
    if ( !error.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpPromptRegistry: A prompt was refused: " << error );
        return false;
    }
    entry->descriptor.ToDataNode( entry->descriptorNode );

    const CORE::CString name = entry->descriptor.name;
    {
        MT::CScopeWriterLock lock( m_rwLock );
        TPromptEntryMap::iterator existing = m_entries.find( name );
        if ( existing != m_entries.end() && (*existing).second->prompt != prompt )
        {
            error = "another prompt is registered with the name \"" + name + "\"";
            lock.EarlyUnlock();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpPromptRegistry: A prompt was refused: " << error );
            return false;
        }
        entry->generation = OnChangedWhileLocked();
        m_entries[ name ] = entry;
    }

    RaiseNameEvent( PromptRegisteredEvent, name );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpPromptRegistry::Unregister( const CORE::CString& name )
{GUCEF_TRACE;

    {
        MT::CScopeWriterLock lock( m_rwLock );
        TPromptEntryMap::iterator i = m_entries.find( name );
        if ( i == m_entries.end() )
            return false;
        m_entries.erase( i );
        OnChangedWhileLocked();
    }

    RaiseNameEvent( PromptUnregisteredEvent, name );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpPromptRegistry::UnregisterAll( void )
{GUCEF_TRACE;

    TPromptEntryMap removed;
    {
        MT::CScopeWriterLock lock( m_rwLock );
        if ( m_entries.empty() )
            return;
        removed.swap( m_entries );
        OnChangedWhileLocked();
    }

    for ( TPromptEntryMap::iterator i = removed.begin(); i != removed.end(); ++i )
        RaiseNameEvent( PromptUnregisteredEvent, (*i).first );
}

/*-------------------------------------------------------------------------*/

bool
CMcpPromptRegistry::TryGet( const CORE::CString& name, CMcpPromptEntryPtr& entry ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwLock );
    TPromptEntryMap::const_iterator i = m_entries.find( name );
    if ( i == m_entries.end() )
        return false;
    entry = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpPromptRegistry::GetEntries( TPromptEntryVector& entries ) const
{GUCEF_TRACE;

    entries.clear();
    MT::CScopeReaderLock lock( m_rwLock );
    entries.reserve( m_entries.size() );
    for ( TPromptEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
        entries.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpPromptRegistry::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwLock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpPromptRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpPromptRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
