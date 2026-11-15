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

#ifndef GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#define GUIDRIVERIMGUI_CIMGUIGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

struct ImGuiContext;

namespace GUCEF {
namespace GUI { class CIGUIContext; }
namespace GUIDRIVERIMGUI {

class CImGuiContext;

/**
 *  What the Dear ImGui drivers share in a process:
 *
 *  - the allocator: Dear ImGui allocates through GUCEF_MALLOC and GUCEF_FREE, installed before the first context is made
 *    and taken out again only when no context exists;
 *  - the hooks of imgui_gucef.cpp: failed assertions and the debug log of Dear ImGui go to the GUCEF log;
 *  - the ImGui thread: Dear ImGui has one current context per process, so all contexts live on the thread on which the
 *    first of them was made; a context made or input given on another thread is refused with an error in the log;
 *  - the contexts that exist, so that a gucefGUI context can be told to be an ImGui context without RTTI.
 *
 *  It exists while a driver or a context uses it: made at the first use, gone with the last user.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiGlobal
{
    public:

    /** The instance, made at the first call */
    static CImGuiGlobal* Instance( void );

    /** Whether the instance exists */
    static bool HasInstance( void );

    /** A driver uses the global for as long as it exists */
    void RegisterDriver( void );
    void UnregisterDriver( void );

    /**
     *  A context begins to exist: false when it is refused because other contexts exist on another thread. The first
     *  context sets the ImGui thread and installs the allocator before Dear ImGui makes anything
     */
    bool RegisterContext( CImGuiContext* context );

    /** The Dear ImGui context of a registered context was made */
    void SetImGuiContext( CImGuiContext* context, ::ImGuiContext* imguiContext );

    void UnregisterContext( CImGuiContext* context );

    /** The ImGui context of a gucefGUI context, NULL when it is no ImGui context */
    CImGuiContext* FindContext( const GUI::CIGUIContext* context ) const;

    /** The context that owns a Dear ImGui context, NULL when none of the registered ones does */
    CImGuiContext* FindContext( const ::ImGuiContext* imguiContext ) const;

    UInt32 GetContextCount( void ) const;

    /** Whether the caller may use Dear ImGui: no context exists, or the caller is on the thread of the contexts */
    bool IsImGuiThread( void ) const;

    /** Logs an error and answers false when the caller is not on the ImGui thread; 'action' says what was refused */
    bool CheckImGuiThread( const char* action ) const;

    /** How many allocations and frees of Dear ImGui went through the GUCEF allocator */
    UInt64 GetAllocationCount( void ) const;
    UInt64 GetFreeCount( void ) const;

    /** Whether the GUCEF allocator is the one Dear ImGui uses now */
    bool IsAllocatorInstalled( void ) const;

    /**
     *  Whether a failed assertion of Dear ImGui stops the program after it is logged: true in debug builds, false in
     *  release builds, where execution continues. Tests set it to false and count the assertions instead
     */
    void SetStopOnAssertion( bool stop );
    bool GetStopOnAssertion( void ) const;

    /** How many assertions of Dear ImGui failed, and the last of them as "expression (file:line)" */
    UInt32 GetAssertionCount( void ) const;
    CString GetLastAssertion( void ) const;

    private:

    CImGuiGlobal( void );
    ~CImGuiGlobal();

    CImGuiGlobal( const CImGuiGlobal& src );
    CImGuiGlobal& operator=( const CImGuiGlobal& src );

    void InstallHooks( void );
    void RemoveHooks( void );
    void InstallAllocator( void );
    void RemoveAllocator( void );

    /** Destroys the instance when nothing uses it any more */
    static void DeinstanceIfUnused( void );

    static void* Allocate( size_t size, void* userData );
    static void Free( void* pointer, void* userData );
    static void OnAssertionFailed( const char* expression, const char* file, int line );
    static void OnDebugPrint( const char* text );

    private:

    struct SContextEntry
    {
        CImGuiContext* context;
        ::ImGuiContext* imguiContext;
    };
    typedef GUCEF::vector< SContextEntry > TContextEntries;

    typedef void* (*TAllocFunc)( size_t size, void* userData );
    typedef void (*TFreeFunc)( void* pointer, void* userData );

    TContextEntries m_contexts;
    UInt32 m_drivers;
    TThreadId m_imguiThread;
    bool m_allocatorInstalled;
    TAllocFunc m_previousAlloc;
    TFreeFunc m_previousFree;
    void* m_previousUserData;
    UInt64 m_allocations;
    UInt64 m_frees;
    bool m_stopOnAssertion;
    UInt32 m_assertions;
    CString m_lastAssertion;
    mutable MT::CMutex m_lock;

    static CImGuiGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUIGLOBAL_H ? */
