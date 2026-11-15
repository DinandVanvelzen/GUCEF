/*
 *  guidriverImGui: the gucefGUI driver library of Dear ImGui, shared by its renderer drivers
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

#include "imgui.h"
#include "imgui_gucef.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXT_H
#include "guidriverImGui_CImGuiContext.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXT_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXT_H ? */

#include "guidriverImGui_CImGuiGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiGlobal* CImGuiGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiGlobal*
CImGuiGlobal::Instance( void )
{GUCEF_TRACE;

    // made and destroyed on the ImGui thread: by the drivers and contexts, which live there
    if ( GUCEF_NULL == g_instance )
    {
        g_instance = GUCEF_NEW CImGuiGlobal();
        g_instance->InstallHooks();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGlobal::HasInstance( void )
{GUCEF_TRACE;

    return GUCEF_NULL != g_instance;
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::DeinstanceIfUnused( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
        return;
    {
        MT::CScopeMutex lock( g_instance->m_lock );
        if ( 0 != g_instance->m_drivers || !g_instance->m_contexts.empty() )
            return;
    }
    g_instance->RemoveAllocator();
    g_instance->RemoveHooks();
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CImGuiGlobal::CImGuiGlobal( void )
    : m_contexts()
    , m_drivers( 0 )
    , m_imguiThread( 0 )
    , m_allocatorInstalled( false )
    , m_previousAlloc( GUCEF_NULL )
    , m_previousFree( GUCEF_NULL )
    , m_previousUserData( GUCEF_NULL )
    , m_allocations( 0 )
    , m_frees( 0 )
    #ifdef GUCEF_DEBUG_MODE
    , m_stopOnAssertion( true )
    #else
    , m_stopOnAssertion( false )
    #endif
    , m_assertions( 0 )
    , m_lastAssertion()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiGlobal::~CImGuiGlobal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::InstallHooks( void )
{GUCEF_TRACE;

    GucefImGui_SetAssertHandler( &CImGuiGlobal::OnAssertionFailed );
    GucefImGui_SetDebugPrintHandler( &CImGuiGlobal::OnDebugPrint );
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::RemoveHooks( void )
{GUCEF_TRACE;

    if ( GucefImGui_GetAssertHandler() == &CImGuiGlobal::OnAssertionFailed )
        GucefImGui_SetAssertHandler( GUCEF_NULL );
    if ( GucefImGui_GetDebugPrintHandler() == &CImGuiGlobal::OnDebugPrint )
        GucefImGui_SetDebugPrintHandler( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::InstallAllocator( void )
{GUCEF_TRACE;

    // only while no context exists: what Dear ImGui allocated before is freed by the functions it was allocated with
    if ( m_allocatorInstalled )
        return;
    ImGuiMemAllocFunc allocFunc = GUCEF_NULL;
    ImGuiMemFreeFunc freeFunc = GUCEF_NULL;
    void* userData = GUCEF_NULL;
    ImGui::GetAllocatorFunctions( &allocFunc, &freeFunc, &userData );
    m_previousAlloc = allocFunc;
    m_previousFree = freeFunc;
    m_previousUserData = userData;
    ImGui::SetAllocatorFunctions( &CImGuiGlobal::Allocate, &CImGuiGlobal::Free, this );
    m_allocatorInstalled = true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::RemoveAllocator( void )
{GUCEF_TRACE;

    if ( !m_allocatorInstalled )
        return;
    ImGui::SetAllocatorFunctions( m_previousAlloc, m_previousFree, m_previousUserData );
    m_allocatorInstalled = false;
}

/*-------------------------------------------------------------------------*/

void*
CImGuiGlobal::Allocate( size_t size, void* userData )
{
    CImGuiGlobal* global = static_cast< CImGuiGlobal* >( userData );
    void* memory = GUCEF_MALLOC( size );
    if ( GUCEF_NULL != global && GUCEF_NULL != memory )
        ++global->m_allocations;
    return memory;
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::Free( void* pointer, void* userData )
{
    if ( GUCEF_NULL == pointer )
        return;
    CImGuiGlobal* global = static_cast< CImGuiGlobal* >( userData );
    if ( GUCEF_NULL != global )
        ++global->m_frees;
    GUCEF_FREE( pointer );
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::OnAssertionFailed( const char* expression, const char* file, int line )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "ImGui: Assertion failed: " << ( GUCEF_NULL != expression ? expression : "" ) << " ("
                     << ( GUCEF_NULL != file ? file : "" ) << ":" << line << ")" );

    bool stop = false;
    CImGuiGlobal* global = g_instance;
    if ( GUCEF_NULL != global )
    {
        MT::CScopeMutex lock( global->m_lock );
        ++global->m_assertions;
        global->m_lastAssertion = CString( GUCEF_NULL != expression ? expression : "" ) + " (" + ( GUCEF_NULL != file ? file : "" ) + ":"
                                  + CORE::Int32ToString( (Int32) line ) + ")";
        stop = global->m_stopOnAssertion;
    }
    if ( stop )
    {
        GUCEF_ASSERT( false );
    }
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::OnDebugPrint( const char* text )
{GUCEF_TRACE;

    if ( GUCEF_NULL == text )
        return;
    CString line( text );
    while ( line.Length() > 0 && ( '\n' == line[ line.Length() - 1 ] || '\r' == line[ line.Length() - 1 ] ) )
        line = line.CutChars( 1, false, 0 );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ImGui: " << line );
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::RegisterDriver( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ++m_drivers;
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::UnregisterDriver( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_drivers > 0 )
            --m_drivers;
    }
    DeinstanceIfUnused();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGlobal::RegisterContext( CImGuiContext* context )
{GUCEF_TRACE;

    if ( GUCEF_NULL == context )
        return false;

    MT::CScopeMutex lock( m_lock );
    TThreadId thread = MT::GetCurrentThreadID();
    if ( !m_contexts.empty() && thread != m_imguiThread )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ImGui: A context cannot be made on thread " << thread << ": the ImGui contexts live on thread "
                         << m_imguiThread << ", Dear ImGui has one current context per process" );
        return false;
    }
    if ( m_contexts.empty() )
    {
        m_imguiThread = thread;
        InstallAllocator();
    }
    SContextEntry entry;
    entry.context = context;
    entry.imguiContext = GUCEF_NULL;
    m_contexts.push_back( entry );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::SetImGuiContext( CImGuiContext* context, ::ImGuiContext* imguiContext )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TContextEntries::iterator i=m_contexts.begin(); i!=m_contexts.end(); ++i )
    {
        if ( (*i).context == context )
        {
            (*i).imguiContext = imguiContext;
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::UnregisterContext( CImGuiContext* context )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        for ( TContextEntries::iterator i=m_contexts.begin(); i!=m_contexts.end(); ++i )
        {
            if ( (*i).context == context )
            {
                m_contexts.erase( i );
                break;
            }
        }
    }
    DeinstanceIfUnused();
}

/*-------------------------------------------------------------------------*/

CImGuiContext*
CImGuiGlobal::FindContext( const GUI::CIGUIContext* context ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == context )
        return GUCEF_NULL;
    MT::CScopeMutex lock( m_lock );
    for ( TContextEntries::const_iterator i=m_contexts.begin(); i!=m_contexts.end(); ++i )
        if ( static_cast< const GUI::CIGUIContext* >( (*i).context ) == context )
            return (*i).context;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CImGuiContext*
CImGuiGlobal::FindContext( const ::ImGuiContext* imguiContext ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == imguiContext )
        return GUCEF_NULL;
    MT::CScopeMutex lock( m_lock );
    for ( TContextEntries::const_iterator i=m_contexts.begin(); i!=m_contexts.end(); ++i )
        if ( (*i).imguiContext == imguiContext )
            return (*i).context;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiGlobal::GetContextCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_contexts.size();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGlobal::IsImGuiThread( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_contexts.empty() || MT::GetCurrentThreadID() == m_imguiThread;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGlobal::CheckImGuiThread( const char* action ) const
{GUCEF_TRACE;

    if ( IsImGuiThread() )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ImGui: Refused on thread " << MT::GetCurrentThreadID() << ", the ImGui contexts live on thread "
                     << m_imguiThread << ": " << ( GUCEF_NULL != action ? action : "" ) );
    return false;
}

/*-------------------------------------------------------------------------*/

UInt64
CImGuiGlobal::GetAllocationCount( void ) const
{GUCEF_TRACE;

    return m_allocations;
}

/*-------------------------------------------------------------------------*/

UInt64
CImGuiGlobal::GetFreeCount( void ) const
{GUCEF_TRACE;

    return m_frees;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGlobal::IsAllocatorInstalled( void ) const
{GUCEF_TRACE;

    ImGuiMemAllocFunc allocFunc = GUCEF_NULL;
    ImGuiMemFreeFunc freeFunc = GUCEF_NULL;
    void* userData = GUCEF_NULL;
    ImGui::GetAllocatorFunctions( &allocFunc, &freeFunc, &userData );
    return m_allocatorInstalled && allocFunc == &CImGuiGlobal::Allocate && freeFunc == &CImGuiGlobal::Free;
}

/*-------------------------------------------------------------------------*/

void
CImGuiGlobal::SetStopOnAssertion( bool stop )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_stopOnAssertion = stop;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiGlobal::GetStopOnAssertion( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_stopOnAssertion;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiGlobal::GetAssertionCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_assertions;
}

/*-------------------------------------------------------------------------*/

CString
CImGuiGlobal::GetLastAssertion( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastAssertion;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
