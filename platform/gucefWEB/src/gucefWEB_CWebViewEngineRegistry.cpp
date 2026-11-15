/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CWebViewEngineRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

const CORE::CEvent CWebViewEngineRegistry::EngineRegisteredEvent = "GUCEF::WEB::CWebViewEngineRegistry::EngineRegisteredEvent";
const CORE::CEvent CWebViewEngineRegistry::EngineUnregisteredEvent = "GUCEF::WEB::CWebViewEngineRegistry::EngineUnregisteredEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWebViewEngineRegistry::RegisterEvents( void )
{GUCEF_TRACE;

    EngineRegisteredEvent.Initialize();
    EngineUnregisteredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWebViewEngineRegistry::CWebViewEngineRegistry( void )
    : CORE::CNotifier()
    , m_lock()
    , m_engines()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWebViewEngineRegistry::~CWebViewEngineRegistry()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_engines.clear();
}

/*-------------------------------------------------------------------------*/

Int32
CWebViewEngineRegistry::Find( const CString& engineName ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_engines.size(); ++i )
        if ( m_engines[ i ]->GetEngineName().Equals( engineName, false ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineRegistry::Register( const TWebViewEnginePtr& engine )
{GUCEF_TRACE;

    if ( engine.IsNULL() )
        return false;
    const CString name = engine->GetEngineName();
    if ( IWebViewEngine::InterfaceVersion != engine->GetInterfaceVersion() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngineRegistry: The engine \"" << name << "\" was built for interface version " << engine->GetInterfaceVersion()
                         << ", this module has version " << IWebViewEngine::InterfaceVersion << ": it is not registered" );
        return false;
    }
    {
        MT::CScopeMutex lock( m_lock );
        if ( name.IsNULLOrEmpty() || Find( name ) >= 0 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebViewEngineRegistry: An engine named \"" << name << "\" is registered already" );
            return false;
        }
        m_engines.push_back( engine );
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "WebViewEngineRegistry: Registered the engine \"" << name << "\"" );
    CORE::TCloneableString eventData( name );
    NotifyObservers( EngineRegisteredEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineRegistry::Unregister( const CString& engineName )
{GUCEF_TRACE;

    TWebViewEnginePtr engine;
    {
        MT::CScopeMutex lock( m_lock );
        const Int32 index = Find( engineName );
        if ( index < 0 )
            return false;
        engine = m_engines[ index ];
        m_engines.erase( m_engines.begin() + index );
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "WebViewEngineRegistry: Unregistered the engine \"" << engineName << "\"" );
    CORE::TCloneableString eventData( engineName );
    NotifyObservers( EngineUnregisteredEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineRegistry::IsRegistered( const CString& engineName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return Find( engineName ) >= 0;
}

/*-------------------------------------------------------------------------*/

TWebViewEnginePtr
CWebViewEngineRegistry::GetEngine( const CString& engineName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const Int32 index = Find( engineName );
    return index >= 0 ? m_engines[ index ] : TWebViewEnginePtr();
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CWebViewEngineRegistry::GetEngineNames( void ) const
{GUCEF_TRACE;

    CString::StringVector names;
    MT::CScopeMutex lock( m_lock );
    for ( TEngineVector::const_iterator i=m_engines.begin(); i!=m_engines.end(); ++i )
        names.push_back( (*i)->GetEngineName() );
    return names;
}

/*-------------------------------------------------------------------------*/

CWebViewEngineRegistry::TEngineVector
CWebViewEngineRegistry::GetEngines( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_engines;
}

/*-------------------------------------------------------------------------*/

CWebViewEngineRegistry::TEngineVector
CWebViewEngineRegistry::GetEnginesInOrder( const CString::StringVector& preferredOrder ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEngineVector ordered;
    GUCEF::vector< UInt8 > taken( m_engines.size(), 0 );
    for ( CString::StringVector::const_iterator n=preferredOrder.begin(); n!=preferredOrder.end(); ++n )
    {
        const Int32 index = Find( (*n) );
        if ( index >= 0 && !taken[ index ] )
        {
            ordered.push_back( m_engines[ index ] );
            taken[ index ] = 1;
        }
    }
    for ( UInt32 i=0; i<(UInt32)m_engines.size(); ++i )
        if ( !taken[ i ] )
            ordered.push_back( m_engines[ i ] );
    return ordered;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewEngineRegistry::GetEngineCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_engines.size();
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewEngineRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewEngineRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
