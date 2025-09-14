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

#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <stdio.h>
#include <assert.h>

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"               /* needed for str to int */
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  /* Do not use WIN32_LEAN_AND_MEAN because it will remove timeBeginPeriod() etc. */
  #undef  WIN32_LEAN_AND_MEAN
  #include <windows.h>                /* Windows API */
  #include <psapi.h>
  #include <powerbase.h>              /* needed for CPU stats */
  #undef min
  #undef max
  #define MAX_DIR_LENGTH MAX_PATH

  #include <WinSock2.h>

  #ifndef GUCEF_CORE_MSWINUTILS_H
  #include "gucefCORE_mswinutils.h"
  #define GUCEF_CORE_MSWINUTILS_H
  #endif /* GUCEF_CORE_MSWINUTILS_H? */

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

  #include <sys/times.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <limits.h>                 /* Linux OS limits */
  #include <dlfcn.h>                  /* dynamic linking utilities */
  #include <dirent.h>
  #include <unistd.h>
  #define MAX_DIR_LENGTH PATH_MAX

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CString::StringMap           TStringMap;
typedef CString::StringVector        TStringVector;
typedef CString::StringPair          TStringPair;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CLogicModule : public CTSharedObjCreator< CLogicModule, MT::CNoLock >
{
    public:

    typedef CORE::CTSharedPtr< CLogicModule, MT::CNoLock >  CLogicModulePtr;
    typedef GUCEF::map< CString, TAnyPointer >              StringToAnyPointerMap;

    CString m_name;
    StringToAnyPointerMap m_functions;
    MT::CMutex m_lock;

    CLogicModule( void )
        : CTSharedObjCreator< CLogicModule, MT::CNoLock >( this )
        , m_name()
        , m_functions()
        , m_lock()
    {GUCEF_TRACE;
        
    }

    void
    RegisterFunction( const CString& name        ,
                      const TAnyPointer& funcPtr )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        m_functions[ name ] = funcPtr;    
    }

    TAnyPointer
    GetFunctionAddress( const CString& name )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );

        StringToAnyPointerMap::const_iterator i = m_functions.find( name );
        if ( i != m_functions.end() )
        {
            return (*i).second;
        }

        TAnyPointer ptr;
        ptr.funcPtr = 0;
        return ptr;
    }
};

typedef CLogicModule::CLogicModulePtr CLogicModulePtr;

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN CModuleRegistry
{
    private:

    typedef GUCEF::map< CString, CLogicModulePtr >    StringToLogicModulePtrMap;

    StringToLogicModulePtrMap m_modules;
    MT::CMutex m_lock;

    public:

    CModuleRegistry( void )
        : m_modules()
        , m_lock()
    {GUCEF_TRACE;
        
    }

    void*
    RegisterModule( const CString& name )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );

        StringToLogicModulePtrMap::iterator i = m_modules.find( name );
        if ( i != m_modules.end() )
        {
            // already registered
            CLogicModulePtr& module = (*i).second;
            return module.GetPointerAlways();
        }

        // no such module yet, create it
        CLogicModulePtr module = CLogicModule::CreateSharedObj();
        if ( !module.IsNULL() )
            m_modules[ name ] = module;
        return module.GetPointerAlways();
    }

    void*
    GetModuleAddress( const CString& name )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );

        StringToLogicModulePtrMap::iterator i = m_modules.find( name );
        if ( i != m_modules.end() )
        {
            CLogicModulePtr& module = (*i).second;
            return module.GetPointerAlways();
        }
        return GUCEF_NULL;
    }

    CLogicModulePtr
    GetModuleForAddress( const void* sohandle )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );

        StringToLogicModulePtrMap::const_iterator i = m_modules.begin();
        while ( i != m_modules.end() )
        {
            const CLogicModulePtr& module = (*i).second;
            if ( module.GetPointerAlways() == sohandle )
            {
                return module;
            }
            ++i;
        }

        return CLogicModulePtr();
    }

    bool
    RegisterFunction( const void* sohandle             ,
                      const CString& functionName      ,
                      const TAnyPointer& staticAddress )
    {GUCEF_TRACE;

        CLogicModulePtr module = GetModuleForAddress( sohandle );
        if ( module.IsNULL() )
        {
            return false;
        }

        module->RegisterFunction( functionName, staticAddress );
        return true;
    }

    bool
    RegisterFunction( const CString& moduleName        ,
                      const CString& functionName      ,
                      const TAnyPointer& staticAddress )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );

        StringToLogicModulePtrMap::iterator i = m_modules.find( moduleName );
        if ( i != m_modules.end() )
        {
            CLogicModulePtr& module = (*i).second;
            if ( !module.IsNULL() )
            {
                module->RegisterFunction( functionName, staticAddress );
                return true;
            }
        }

        return false;
    }

    TAnyPointer
    GetFunctionAddress( const void* sohandle         ,
                        const CString& functionName  )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        
        CLogicModulePtr module = GetModuleForAddress( sohandle );
        if ( module.IsNULL() )
        {
            TAnyPointer ptr;
            ptr.funcPtr = 0;
            return ptr;
        }

        return module->GetFunctionAddress( functionName );
    }

    TAnyPointer
    GetFunctionAddress( const CString& moduleName    ,
                        const CString& functionName  )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );

        StringToLogicModulePtrMap::iterator i = m_modules.find( moduleName );
        if ( i != m_modules.end() )
        {
            CLogicModulePtr& module = (*i).second;
            if ( module.IsNULL() )
            {
                TAnyPointer ptr;
                ptr.funcPtr = 0;
                return ptr;
            }

            return module->GetFunctionAddress( functionName );
        }

        TAnyPointer ptr;
        ptr.funcPtr = 0;
        return ptr;
    }

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex g_envOverridesLock;
CStringMap g_envOverrides;
CModuleRegistry g_moduleRegistry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
SetEnv( const CString& key   ,
        const CString& value )
{GUCEF_TRACE;

    return GUCEFSetEnv( key.C_String(), value.C_String() );
}

/*--------------------------------------------------------------------------*/

void
SetEnvOverride( const CString& key   ,
                const CString& value )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_envOverridesLock );
    g_envOverrides[ key ] = value;
}

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP void
SetEnvOverrides( const CString& allKeysAndValues )
{GUCEF_TRACE;

    TStringMap envOverrides;
    TStringVector elements = allKeysAndValues.ParseElements( '\n', false );
    TStringVector::iterator i = elements.begin();
    while ( i != elements.end() )
    {
        Int32 sepIndex = (*i).HasChar( '=', 0, true );
        if ( sepIndex >= 0 )
        {
            envOverrides.insert( TStringPair( (*i).SubstrToIndex( sepIndex, true ), (*i).SubstrToIndex( sepIndex, false ) ) );
        }
        ++i;
    }

    {
        MT::CScopeMutex scopeLock( g_envOverridesLock );
        TStringMap::iterator n = envOverrides.begin();
        while ( n != envOverrides.end() )
        {
            g_envOverrides[ (*n).first ] = (*n).second;
            ++n;
        }
    }
}

/*--------------------------------------------------------------------------*/

CString
GetEnv( const CString& key )
{GUCEF_TRACE;

    if ( key.IsNULLOrEmpty() ) return CString();

    {
        MT::CScopeMutex scopeLock( g_envOverridesLock );
        TStringMap::iterator i = g_envOverrides.find( key );
        if ( i != g_envOverrides.end() )
        {
            // we only allow this right now since there is no way to override
            return (*i).second;
        }
    }
    return GUCEFGetEnv( key.C_String() );
}

/*-------------------------------------------------------------------------*/

void*
LoadModuleDynamicly( const CString& filename )
{GUCEF_TRACE;

    // Check if the module is registered in our override registry
    void* modulePtr = g_moduleRegistry.GetModuleAddress( filename );
    if ( GUCEF_NULL != modulePtr )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Module address obtained from local registry for: " + filename );
        return modulePtr;
    }

    CString fName;
    CString fileExt = ExtractFileExtention( filename );

    /*
     *  If no module extension was given we will add the O/S default
     */
    if ( fileExt.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        fName = filename + ".dll\0";
        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        fName = filename + ".so\0";
        #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE )
        fName = filename + ".dylib\0";
        #endif
    }

    const CString& theFilename = fName.IsNULLOrEmpty() ? filename : fName; 

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    modulePtr = (void*) ::dlopen( theFilename.C_String(), RTLD_NOW );
    if ( GUCEF_NULL == modulePtr )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "dlopen() reports error: " + CString( dlerror() ) );
    }
    if ( GUCEF_NULL == modulePtr )
    {
        // It is possible the load failed due to missing "lib" prefix on linux/android.
        // Check for this and compensate as needed
        CString fileOnly = ExtractFilename( theFilename );
        if ( 0 != fileOnly.HasSubstr( "lib" ) )
        {
            // No module name previous "lib" prefix was found, we will add one and try to load again
            fileOnly = "lib" + fileOnly;
            CString newFilePath = CombinePath( StripFilename( theFilename ), fileOnly );
            modulePtr = (void*) ::dlopen( newFilePath.C_String(), RTLD_NOW );
            if ( GUCEF_NULL == modulePtr )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "dlopen() reports error: " + CString( dlerror() ) );
            }
        }
    }

    // It is possible that per Linux fashion the module is actually in a /lib/ dir
    // while the current dir is pointing at a /bin/ dir due to the pattern of allowing
    // $MODULEDIR$ variable based loading. We check for that here as well.
    if ( GUCEF_NULL == modulePtr )
    {
        CString pathOnly = StripFilename( theFilename );
        if ( "bin" == LastSubDir( pathOnly ) )
        {
            pathOnly = CombinePath( StripLastSubDir( pathOnly ), "lib" );
            CString fileOnly = ExtractFilename( fName );
            CString newFilePath = CombinePath( pathOnly, fileOnly );

            modulePtr = (void*) ::dlopen( newFilePath.C_String(), RTLD_NOW );

            if ( GUCEF_NULL == modulePtr )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "dlopen() reports error: " + CString( dlerror() ) );

                // It is possible the load failed due to missing "lib" prefix on linux/android.
                // Check for this and compensate as needed
                if ( 0 != fileOnly.HasSubstr( "lib" ) )
                {
                    // No module name previous "lib" prefix was found, we will add one and try to load again
                    fileOnly = "lib" + fileOnly;
                    newFilePath = CombinePath( pathOnly, fileOnly );

                    modulePtr = (void*) ::dlopen( newFilePath.C_String(), RTLD_NOW );
                    if ( GUCEF_NULL == modulePtr )
                    {
                        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "dlopen() reports error: " + CString( dlerror() ) );
                    }
                }
            }
        }
    }

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    std::wstring wFilename = ToWString( theFilename );
    modulePtr = (void*) ::LoadLibraryW( wFilename.c_str() );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == modulePtr )
    {
        DWORD lastErrorCode = ::GetLastError();
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "LoadLibrary() reports error code: " + ToString( (UInt32) lastErrorCode ) );
    }

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // Dynamic loading is not supported in Emscripten
    modulePtr = GUCEF_NULL;

    #endif

    return modulePtr;
}

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void*
GetModulePointer( const CString& moduleName )
{GUCEF_TRACE;

    if ( !moduleName.IsNULLOrEmpty() )
    {
        // check our overrides
        void* modulePtr = g_moduleRegistry.GetModuleAddress( moduleName );
        if ( GUCEF_NULL != modulePtr )
            return modulePtr;
    }

    // If no module name is passed we get the pointer to the main process module

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // On linux the reference count is always incremented so we must decrement again right away to get
    // the same behaviour as the windows version
    void* modulePtr = (void*) ::dlopen( moduleName.C_String(), RTLD_NOW );
    ::dlclose( modulePtr );
    return modulePtr;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    std::wstring wModuleName = ToWString( moduleName );
    return (void*) ::GetModuleHandleW( wModuleName.c_str() );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // Dynamic loading is not supported in Emscripten
    return nullptr;

    #else
    #error Unsupported target platform
    #endif
}

/*--------------------------------------------------------------------------*/

void
UnloadModuleDynamicly( void *sohandle )
{GUCEF_TRACE;

    if ( NULL == sohandle )
        return;

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    ::dlclose( sohandle );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::FreeLibrary( (HMODULE)sohandle );

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // Dynamic unloading is not supported in Emscripten

    #else
    #error Unsupported target platform
    #endif
}

/*--------------------------------------------------------------------------*/

TAnyPointer
GetFunctionAddress( void* sohandle              ,
                    const CString& functionname ,
                    UInt32 parambytes           )
{GUCEF_TRACE;

    /*
     *      Calling Convention      Internal*       MSVC DLL (w/ DEF)       MSVC DLL (dllexport)  	DMC DLL         MinGW DLL       BCC DLL
     *      __stdcall               _Function@n  	Function                _Function@n             _Function@n     Function@n      Function
     *      __cdecl                 _Function       Function                Function                Function        Function        _Function
     */
    TAnyPointer fptr;
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == sohandle || functionname.IsNULLOrEmpty() )
    {
        fptr.funcPtr = 0;
        return fptr;
    }

    // Check if the function is registered in our override registry
    fptr = g_moduleRegistry.GetFunctionAddress( sohandle, functionname );
    if ( GUCEF_NULL != fptr.funcPtr )
    {
        return fptr;
    }

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    fptr.objPtr = dlsym( sohandle                ,
                         functionname.C_String() );
    return fptr;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // In emscripten you will need dynamic linking enabled. Its not enabled by default
    // it becomes enabled by passing -s MAIN_MODULE when building
    // alas emscripten does not provide a good flag for us to key on so we create our own and
    // assume the build system will provide it as needed
    #ifdef GUCEF_EMSCRIPTEN_HAS_DYNAMIC_LINKING_SUPPORT

    fptr.objPtr = dlsym( sohandle                ,
                         functionname.C_String() );
    return fptr;

    #else

    fptr.objPtr = 0;
    return fptr;

    #endif

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    /*
     *      First we try a normal load using the given
     *      functionname.
     *
     *      Type: Function
     */
    fptr.funcPtr = (TDefaultFuncPtr) GetProcAddress( (HMODULE)sohandle       ,
                                                     functionname.C_String() );
    if ( fptr.funcPtr == NULL )
    {
        char buffer[ 1024 ];
        UInt32 len = functionname.Length();

        /*
         *      Maybe without the param bytes then :(
         *
         *      Type: _Function
         */
        strncpy( buffer+1, functionname.C_String(), len+1 );
        *buffer = '_';
        fptr.funcPtr = (TDefaultFuncPtr) GetProcAddress( (HMODULE)sohandle ,
                                                         buffer            );

        /*
         *      Try adding the param bytes value
         *      ... So much for naming conventions :/
         *
         *      Type: _Function@n
         */
        if ( fptr.funcPtr == NULL )
        {
            sprintf( buffer+len+1, "@%d", parambytes );
            fptr.funcPtr = (TDefaultFuncPtr) GetProcAddress( (HMODULE)sohandle ,
                                                             buffer            );

            /*
             *      Last but not least try..
             *
             *      Type: Function@n
             */
            if ( fptr.funcPtr == NULL )
            {
                fptr.funcPtr = (TDefaultFuncPtr) GetProcAddress( (HMODULE)sohandle ,
                                                                  buffer+1         );
            }
        }

    }
    return fptr;

    #else
    #error Unsupported target platform
    #endif
}

/*--------------------------------------------------------------------------*/

void*
RegisterStaticModule( const CString& name )
{GUCEF_TRACE;

    return g_moduleRegistry.RegisterModule( name );
}

/*--------------------------------------------------------------------------*/

void
RegisterStaticFunctionAddress( const void* sohandle             ,
                               const CString& functionName      ,
                               const TAnyPointer& staticAddress )
{GUCEF_TRACE;

    g_moduleRegistry.RegisterFunction( sohandle, functionName, staticAddress );
}

/*--------------------------------------------------------------------------*/

void
RegisterStaticFunctionAddress( const CString& moduleName        ,
                               const CString& functionName      ,
                               const TAnyPointer& staticAddress )
{GUCEF_TRACE;

    g_moduleRegistry.RegisterFunction( moduleName, functionName, staticAddress );
}

/*-------------------------------------------------------------------------*/

CString
GetHostname( void )
{GUCEF_TRACE;

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // Use the POSIX function gethostname()
    char nameBuffer[ 256 ];
    if ( 0 == gethostname( nameBuffer, sizeof(nameBuffer) ) )
    {
        nameBuffer[ 255 ] = 0;
        return nameBuffer;
    }

#else

    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "GetHostname: Platform has no supported implementation" );

#endif

    return CString();
}

/*-------------------------------------------------------------------------*/
#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

// Helper function to get the parent process ID (PPID) of a given process ID (PID)
pid_t GUCEF_HIDDEN
GetParentPID( pid_t pid )
{GUCEF_TRACE;

    CString statFile = "/proc/" + ToString( pid ) + "/stat";
    CString statContent;
    if ( !LoadTextFileAsString( statFile, statContent ) )
        return -1;

    CString::StringVector statParts = statContent.ParseElements( ' ', false );
    if ( statParts.size() < 4 )
    {
        return -1;
    }
    return StringToInt32( statParts[3], -1 );
}

#endif
/*-------------------------------------------------------------------------*/
#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

// Check if the process is running as a service under SysVinit
bool GUCEF_HIDDEN
IsRunningAsSysVinitService( pid_t pid )
{GUCEF_TRACE;

    CFileSystemIterator fsIterator;
    if ( fsIterator.FindFirst( "/etc/rc.d" ) )
    {
        do
        {
            CString path = "/etc/rc.d/" + fsIterator.GetResourceName();
            if ( FileExists( path ) )
            {
                std::ifstream file( path );
                if ( file.is_open() )
                {
                    std::string line;
                    while ( std::getline( file, line ) )
                    {
                        if ( line.find( "pidfile" ) != std::string::npos && line.find( ToString( pid ) ) != std::string::npos )
                        {
                            file.close();
                            return true;
                        }
                    }
                    file.close();
                }
            }
        }
        while ( fsIterator.FindNext() );
    }

    return false;
}

#endif
/*-------------------------------------------------------------------------*/
#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

// Check if the process is running as a service under Upstart
bool GUCEF_HIDDEN
IsRunningAsUpstartService( pid_t pid )
{GUCEF_TRACE;

    CFileSystemIterator fsIterator;
    if ( fsIterator.FindFirst( "/etc/init" ) )
    {
        do
        {
            CString path = "/etc/init/" + fsIterator.GetResourceName();
            if ( FileExists( path ) )
            {
                std::ifstream file( path );
                if ( file.is_open() )
                {
                    std::string line;
                    while ( std::getline( file, line ) )
                    {
                        if ( line.find( "exec" ) != std::string::npos && line.find( ToString( pid ) ) != std::string::npos )
                        {
                            file.close();
                            return true;
                        }
                    }
                    file.close();
                }
            }
        }
        while ( fsIterator.FindNext() );
    }

    return false;
}

#endif
/*-------------------------------------------------------------------------*/
#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

// Check if the process is running as a service under OpenRC
bool GUCEF_HIDDEN
IsRunningAsOpenRCService( pid_t pid )
{GUCEF_TRACE;

    CFileSystemIterator fsIterator;
    if ( fsIterator.FindFirst( "/etc/init.d" ) )
    {
        do
        {
            CString path = "/etc/init.d/" + fsIterator.GetResourceName();
            if ( FileExists( path ) )
            {
                std::ifstream file( path );
                if ( file.is_open() )
                {
                    std::string line;
                    while ( std::getline( file, line ) )
                    {
                        if ( line.find( "pidfile" ) != std::string::npos && line.find( ToString( pid ) ) != std::string::npos )
                        {
                            file.close();
                            return true;
                        }
                    }
                    file.close();
                }
            }
        }
        while ( fsIterator.FindNext() );
    }

    return false;
}

#endif
/*-------------------------------------------------------------------------*/
#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

// Check if the process is running as a service under systemd
bool GUCEF_HIDDEN
IsRunningAsSystemdService( pid_t pid )
{GUCEF_TRACE;

    pid_t ppid = GetParentPID( pid );
    while ( ppid > 1 )
    {
        CString cgroupFile = "/proc/" + ToString( ppid ) + "/cgroup";
        CString cgroupContent;
        if ( LoadTextFileAsString( cgroupFile, cgroupContent ) )
        {
            if ( cgroupContent.HasSubstr( "name=systemd" ) >  -1 )
            {
                return true;
            }
        }

        pid = ppid;
        ppid = GetParentPID( pid );
    }
    return false;
}

#endif
/*-------------------------------------------------------------------------*/
#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

// Check if the process is running as a service under supervisord
bool GUCEF_HIDDEN
IsRunningAsSupervisordService( pid_t pid )
{GUCEF_TRACE;

    // Construct the path to the process's status file
    std::ostringstream statusFilePath;
    statusFilePath << "/proc/" << pid << "/status";

    // Open the status file
    std::ifstream statusFile(statusFilePath.str().c_str());
    if (!statusFile.is_open()) {
        return false;
    }

    // Read the file line by line
    std::string line;
    while (std::getline(statusFile, line)) {
        // Check if the line contains "PPid:"
        if (line.find("PPid:") == 0) {
            // Extract the parent process ID
            pid_t ppid = atoi(line.substr(6).c_str());

            // Construct the path to the parent process's cmdline file
            std::ostringstream ppidCmdlineFilePath;
            ppidCmdlineFilePath << "/proc/" << ppid << "/cmdline";

            // Open the parent process's cmdline file
            std::ifstream ppidCmdlineFile(ppidCmdlineFilePath.str().c_str());
            if (!ppidCmdlineFile.is_open()) {
                return false;
            }

            // Read the entire file content
            std::string ppidCmdline;
            std::getline(ppidCmdlineFile, ppidCmdline, '\0');

            // Close the file
            ppidCmdlineFile.close();

            // Check if the parent process's command line contains "supervisord"
            if (ppidCmdline.find("supervisord") != std::string::npos) {
                return true;
            }
        }
    }

    // Close the status file
    statusFile.close();

    return false;
}

#endif

/*-------------------------------------------------------------------------*/

bool
IsProcessRunningAsService( TProcessId pid )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::SERVICE_STATUS_PROCESS serviceStatus;
    DWORD bytesNeeded = 0;

    ::SC_HANDLE scm = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE );
    if ( NULL == scm )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "IsRunningAsService: Failed to open Service Control Manager" );
        return false;
    }

    ::SC_HANDLE service;
    ::ENUM_SERVICE_STATUS_PROCESS enumStatus;
    DWORD bytesNeeded2 = 0;
    DWORD servicesReturned = 0;
    DWORD resumeHandle = 0;

    BOOL success = ::EnumServicesStatusExW(
        scm,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        (LPBYTE)&enumStatus,
        sizeof(enumStatus),
        &bytesNeeded2,
        &servicesReturned,
        &resumeHandle,
        NULL
    );

    if ( FALSE == success && ::GetLastError() != ERROR_MORE_DATA )
    {
        ::CloseServiceHandle( scm );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "IsRunningAsService: Failed to enumerate services" );
        return false;
    }

    BYTE* buffer = new BYTE[bytesNeeded2];
    success = ::EnumServicesStatusExW(
        scm,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        buffer,
        bytesNeeded2,
        &bytesNeeded2,
        &servicesReturned,
        &resumeHandle,
        NULL
    );

    if ( FALSE == success )
    {
        delete[] buffer;
        ::CloseServiceHandle( scm );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "IsRunningAsService: Failed to enumerate services  (2nd call)" );
        return false;
    }

    ::ENUM_SERVICE_STATUS_PROCESSW* services = (ENUM_SERVICE_STATUS_PROCESSW*) buffer;
    for ( DWORD i=0; i<servicesReturned; ++i )
    {
        if ( services[ i ].ServiceStatusProcess.dwProcessId == (DWORD) pid )
        {
            service = ::OpenServiceW( scm, services[ i ].lpServiceName, SERVICE_QUERY_STATUS );
            if ( NULL != service)
            {
                success = ::QueryServiceStatusEx( service, SC_STATUS_PROCESS_INFO, (LPBYTE)&serviceStatus, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded );
                ::CloseServiceHandle( service );
                if ( FALSE != success )
                {
                    bool result = (serviceStatus.dwCurrentState == SERVICE_RUNNING);
                    delete[] buffer;
                    ::CloseServiceHandle( scm );
                    return result;
                }
            }
        }
    }

    delete[] buffer;
    ::CloseServiceHandle( scm );
    return false;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // On Linux there are multiple service managers, so we need to check for each one
    // or at least some popular ones. This code would need updating as new service managers are introduced
    return IsRunningAsSystemdService( pid ) ||
           IsRunningAsSysVinitService( pid ) ||
           IsRunningAsUpstartService( pid ) ||
           IsRunningAsOpenRCService( pid ) ||
           IsRunningAsSupervisordService( pid );

    #else

    // platform not supported
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "IsRunningAsService: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
IsRunningAsService( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return IsProcessRunningAsService( ::GetCurrentProcessId() );
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    return IsProcessRunningAsService( ::getpid() );
    #else
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "IsRunningAsService: Platform has no supported implementation");
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

bool
IsRunningAsElevatedAdmin( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    static UInt8 g_isElevatedAdmin = 0; // tri-state: 0 = unknown, 1 = yes, 2 = no

    // Only check once
    // race condition is not a big deal, just slightly slower
    if ( 0 == g_isElevatedAdmin )
    {
        HANDLE hToken = GUCEF_NULL;
        BOOL openTokenSuccess = ::OpenProcessToken( ::GetCurrentProcess(), TOKEN_QUERY, &hToken );
        if ( 0 != openTokenSuccess ) 
        {
            TOKEN_ELEVATION elevation;
            DWORD dwSize = 0;
            BOOL bResult = ::GetTokenInformation( hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize );
            if ( 0 != bResult ) 
            {                
                if ( elevation.TokenIsElevated != 0 )
                {
                    g_isElevatedAdmin = 1; // Running as elevated admin
                }
                else
                {
                    g_isElevatedAdmin = 2; // Not running as elevated admin
                }
            }
            CloseHandle( hToken );
        }
    }

    return 1 == g_isElevatedAdmin;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "IsRunningAsElevatedAdmin: TODO" );
    return false;
    #else
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "IsRunningAsElevatedAdmin: Platform has no supported implementation" );
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

bool
GetExeImagePathForProcessId( TProcessId pid     ,
                             CString& imagePath )
{GUCEF_TRACE;

    if ( 0 == pid )
        return false;

    imagePath.Clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    HANDLE handle = ::OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION,
                                   FALSE,
                                   pid /* This is the PID, you can find one from windows task manager */
                                 );
    if ( GUCEF_NULL != handle )
    {
        std::wstring outNameBuffer;
        outNameBuffer.resize( MAX_PATH );
        DWORD buffSize = (DWORD) outNameBuffer.size();

        bool mayBeTruncated = false;
        bool hadError = false;
        do
        {
            BOOL result = ::QueryFullProcessImageNameW( handle, 0, &outNameBuffer[0], &buffSize );
            if ( result != FALSE )
            {
                if ( buffSize == (DWORD) outNameBuffer.size() )
                {
                    outNameBuffer.resize( outNameBuffer.size() * 2 );
                    buffSize = (DWORD) outNameBuffer.size();
                    mayBeTruncated = true;
                }
            }
        }
        while ( !hadError && mayBeTruncated );

        if ( !hadError )
        {
            outNameBuffer.shrink_to_fit();
            imagePath = ToString( outNameBuffer ).Trim( true );
            ::CloseHandle( handle );
            return true;
        }
        ::CloseHandle( handle );
    }
    return false;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // The following works on Linux 2.2 and later:
    // Intended to execute in user space

    char procInfoPath[ 64 ];
    sprintf( procInfoPath, "/proc/%d/exe", pid );

    CString procImagePath;
    UInt32 bufferSize = 1024;
    ssize_t bytesWritten = 0;
    while ( 0 == bytesWritten )
    {
        char* bufferPtr = procImagePath.Reserve( bufferSize );
        if ( GUCEF_NULL != bufferPtr )
        {
            bytesWritten = ::readlink( procInfoPath, bufferPtr, bufferSize );
            if ( bytesWritten <= 0 )
            {
                // Failed to read the link
                // this is expected for some OS level and zombie procs
                return false;
            }
            else
            if ( bytesWritten < bufferSize )
            {
                procImagePath.DetermineLength();
                imagePath = procImagePath;
                return true;
            }
            else
            {
                // May have suffered truncation. try again
                bufferSize += 1024;
                bytesWritten = 0;
            }
        }
        else
        {
            // bad alloc
            return false;
        }
    }

    return false;

    #else

    // platform not supported
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "GetExeImagePathForProcessId: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
GetExeNameForProcessId( TProcessId pid   ,
                        CString& exeName )
{GUCEF_TRACE;

    if ( 0 == pid )
        return false;

    exeName.Clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return GetExeImagePathForProcessId( pid, exeName );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    CString symLinkPath;
    if ( GetExeImagePathForProcessId( pid, symLinkPath ) )
    {
        // The symlink can have the path prefixed from where the executable is linked
        // We just want the name itself
        exeName = ExtractFilename( symLinkPath );
        return true;
    }
    return false;

    #else

    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "GetExeNameForProcessId: Platform has no supported implementation" );
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CommandLineExecute( const CString& command ,
                    CString& result        ,
                    bool waitForExit       )
{GUCEF_TRACE;

    result.Clear();
    if ( command.IsNULLOrEmpty() )
        return false;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    HANDLE hPipeRead = NULL;
    HANDLE hPipeWrite = NULL;

    SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
    saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to get results from child's stdout.
    if ( FALSE == ::CreatePipe( &hPipeRead, &hPipeWrite, &saAttr, 0 ) )
    {
        return false;
    }

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput  = hPipeWrite;
    si.hStdError   = hPipeWrite;
    si.wShowWindow = SW_HIDE; // <- Prevents cmd window from flashing, this requires STARTF_USESHOWWINDOW in dwFlags.

    std::wstring wCommand = ToWString( command );
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    BOOL fSuccess = ::CreateProcessW( NULL, (LPWSTR) wCommand.c_str(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi );
    if ( FALSE == fSuccess )
    {
        ::CloseHandle( hPipeWrite );
        ::CloseHandle( hPipeRead );
        return false;
    }

    if ( waitForExit )
    {
        bool bProcessEnded = false;
        while ( !bProcessEnded )
        {
            // Give some timeslice (50 ms), so we won't waste 100% CPU.
            bProcessEnded = WaitForSingleObject( pi.hProcess, 50) == WAIT_OBJECT_0;

            // Even if process exited - we continue reading, if
            // there is some data available over pipe.
            for (;;)
            {
                char buf[1024];
                DWORD dwRead = 0;
                DWORD dwAvail = 0;

                if ( !::PeekNamedPipe( hPipeRead, NULL, 0, NULL, &dwAvail, NULL ) )
                    break;

                if (!dwAvail) // No data available, return
                    break;

                if ( !::ReadFile( hPipeRead, buf, SMALLEST( sizeof(buf)-1, dwAvail ), &dwRead, NULL ) || !dwRead )
                {
                    // Error, the child process might have ended
                    break;
                }

                buf[ dwRead ] = 0;
                result += buf;
            }
        }
    }

    ::CloseHandle( hPipeWrite );
    ::CloseHandle( hPipeRead );
    ::CloseHandle( pi.hProcess );
    ::CloseHandle( pi.hThread );
    return true;

    #else

    char buffer[ 128 ];
    result = "";
    FILE* pipe = popen( command.C_String(), "r" );
    if ( NULL == pipe )
        return false;

    try
    {
        while ( fgets( buffer, sizeof buffer, pipe ) != NULL )
        {
            result += buffer;
        }
    }
    catch ( const std::exception& e )
    {
        pclose( pipe );
        throw e;
    }
    catch ( ... )
    {
        pclose( pipe );
        throw;
    }
    pclose( pipe );
    return true;

    #endif
}

/*--------------------------------------------------------------------------*/

bool
GetProcessList( TProcessIdVector& processList )
{GUCEF_TRACE;

    processList.clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    DWORD reservedListSizeInBytes = 1024 * sizeof(DWORD);
    DWORD usedListSizeInBytes = 0;
    DWORD* win32ProcessList = GUCEF_NULL;
    do
    {
        DWORD* newWin32ProcessList = (DWORD*) realloc( win32ProcessList, reservedListSizeInBytes );
        if ( GUCEF_NULL == newWin32ProcessList )
        {
            if ( GUCEF_NULL != win32ProcessList )
                free( win32ProcessList );
            return false;
        }
        win32ProcessList = newWin32ProcessList;

        if ( 0 == ::EnumProcesses( win32ProcessList, reservedListSizeInBytes, &usedListSizeInBytes ) )
        {
            return false;
        }

        if ( reservedListSizeInBytes != usedListSizeInBytes )
        {
            break;
        }
        else
        {
            // Guestimate: just double the storage
            reservedListSizeInBytes *= 2;
        }
    }
    while ( true );

    UInt32 processCount = usedListSizeInBytes / sizeof(DWORD);
    processList.reserve( processCount );
    for ( UInt32 i=0; i<processCount; ++i )
    {
        processList.push_back( win32ProcessList[ i ] );
    }

    free( win32ProcessList );
    win32ProcessList = GUCEF_NULL;

    return true;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    processList.reserve( 1024 );

    DIR* dp = ::opendir( "/proc/" );
    struct dirent* dirFilesys = NULL;
    while ( NULL != ( dirFilesys = ::readdir( dp ) ) )
    {
        if ( dirFilesys->d_type == DT_DIR )
        {
            if ( 1 == IsANumber( dirFilesys->d_name ) )
            {
                Int32 parsedId = StringToInt32( dirFilesys->d_name, -1 );
                if ( parsedId > 0 )
                {
                    processList.push_back( (pid_t) parsedId );
                }
            }
        }
    }
    ::closedir( dp );

    return true;

    #else

    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "GetProcessList: Platform has no supported implementation" );
    return false;

    #endif
}

/*--------------------------------------------------------------------------*/

bool
CheckOnProcessAliveStatus( TProcessId pid, bool& status )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // To get the alive status of another proc we need a handle to it
    HANDLE hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION,
                                    FALSE,
                                    pid );
    if ( GUCEF_NULL == hProcess )
    {
        DWORD errorCode = ::GetLastError();
        if ( ERROR_ACCESS_DENIED == errorCode )
        {
            // We are not allowed to open the process
            // this is expected for some OS level and zombie procs
            // We can use a more expensive way to check if the process is alive by going trough the process list and checking if a process with the given pid exists
            TProcessIdVector processList;
            if ( GetProcessList( processList ) )
            {
                TProcessIdVector::iterator i = processList.begin();
                while ( i != processList.end() )
                {
                    if ( (*i) == pid )
                    {
                        status = true;
                        return true;
                    }
                    ++i;
                }
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CheckOnProcessAliveStatus: Failed to open handle to process with PID " + ToString( pid ) + " ErrorCode=" + ToString( (UInt32) errorCode ) );
        }
        return false;
    }

    status = true;
    DWORD exitCode = 0;
    if ( ( ::GetExitCodeProcess( hProcess, &exitCode ) == FALSE ) || exitCode != STILL_ACTIVE )
    {
        status = false;
    }

    ::CloseHandle( hProcess );
    return true;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // If sig is 0, then no signal is sent, but existence and permission
    // checks are still performed; this can be used to check for the
    // existence of a process ID or process group ID that the caller is
    // permitted to signal.
    if ( ::kill( pid, 0 ) == 0 )
    {
        return true;
    }
    return false;

    #else

    // platform not supported
    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CheckOnProcessAliveStatus: Platform has no supported implementation" );
    return false;

    #endif

}

/*--------------------------------------------------------------------------*/

CProcessInformation::CProcessInformation( void )
    : m_commandLineArgs()
    , m_imagePath()
    , m_cStyleAccess()
{GUCEF_TRACE;

    memset( &m_cStyleAccess, 0, sizeof( m_cStyleAccess ) );
    m_cStyleAccess.commandLineArgs = m_commandLineArgs.C_String();
    m_cStyleAccess.commandLineArgsByteSize = 0;
}

/*--------------------------------------------------------------------------*/

CProcessInformation::~CProcessInformation()
{GUCEF_TRACE;

    Clear();
}

/*--------------------------------------------------------------------------*/

void
CProcessInformation::Clear( void )
{GUCEF_TRACE;

    memset( &m_cStyleAccess, 0, sizeof( m_cStyleAccess ) );
    m_commandLineArgs.Clear();
    m_imagePath.Clear();
}

/*--------------------------------------------------------------------------*/

void
CProcessInformation::SetImagePath( const CString& str )
{GUCEF_TRACE;

    m_imagePath = str;
    m_cStyleAccess.imagePath = m_imagePath.C_String();
    m_cStyleAccess.imagePathByteSize = m_imagePath.ByteSize();
}

/*--------------------------------------------------------------------------*/

CString&
CProcessInformation::GetImagePath( void )
{GUCEF_TRACE;

    return m_imagePath;
}

/*--------------------------------------------------------------------------*/

void
CProcessInformation::SetCommandLineArgs( const CString& str )
{GUCEF_TRACE;

    m_commandLineArgs = str;
    m_cStyleAccess.commandLineArgs = m_commandLineArgs.C_String();
    m_cStyleAccess.commandLineArgsByteSize = m_commandLineArgs.ByteSize();
}

/*--------------------------------------------------------------------------*/

CString&
CProcessInformation::GetCommandLineArgs( void )
{GUCEF_TRACE;

    return m_commandLineArgs;
}

/*--------------------------------------------------------------------------*/

const TProcessInformation&
CProcessInformation::GetCStyleAccess( void ) const
{GUCEF_TRACE;

    return m_cStyleAccess;
}

/*--------------------------------------------------------------------------*/

bool
CProcessInformation::TryGetProcessInformation( TProcessId pid            ,
                                               CProcessInformation& info )
{GUCEF_TRACE;

    if ( 0 != pid )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        // First open a handle to the other process

        CLIENT_ID clientId;
        clientId.UniqueThread = NULL;
        clientId.UniqueProcess = UlongToHandle( pid );

        OBJECT_ATTRIBUTES objAttribs;
        InitializeObjectAttributes( &objAttribs, NULL, 0, NULL, NULL );

        HANDLE hProcess = 0;
        ::NTSTATUS status = TryNtOpenProcess( &hProcess,
                                              (ACCESS_MASK) (PROCESS_VM_READ|PROCESS_QUERY_INFORMATION),
                                              &objAttribs,
                                              &clientId );
        if ( !WIN32_NT_SUCCESS( status ) || GUCEF_NULL == hProcess )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessInformation:TryGetProcessInformation: Failed to open handle to process" );
            return false;
        }

        // Now obtain basic starting off info about the process

        bool totalSuccess = true;

        PROCESS_BASIC_INFORMATION pbi;
        _RTL_USER_PROCESS_PARAMETERS* pv = NULL;
        _RTL_USER_PROCESS_PARAMETERS procParameters;
        memset( &procParameters, 0, sizeof( procParameters ) );

        ::NTSTATUS status2 = 0;
        PROCESSINFOCLASS procBasicInformation = PROCESSINFOCLASS::ProcessBasicInformation;
        status = TryNtQueryInformationProcess( hProcess, procBasicInformation, &pbi, sizeof(pbi), 0 );
        if ( WIN32_NT_SUCCESS( status ) )
        {
            // Now get the information we are after

            status = TryNtReadVirtualMemory( hProcess, (_PEB*)&pbi.PebBaseAddress->ProcessParameters, &pv, sizeof(pv), 0 );
            status2 = TryNtReadVirtualMemory( hProcess, pv, &procParameters, sizeof(procParameters), 0 );
            if ( WIN32_NT_SUCCESS( status ) && WIN32_NT_SUCCESS( status2 ) )
            {
                CDynamicBuffer buffer;
                if ( procParameters.CommandLine.Length > 0 )
                {
                    UInt32 strByteSize = ( procParameters.CommandLine.Length * sizeof(WCHAR) ) + sizeof(WCHAR);
                    if ( buffer.SetBufferSize( strByteSize, false, true ) )
                    {
                        buffer.SetDataSize( strByteSize );
                        wchar_t* cmdLineBuffer = buffer.AsTypePtr< wchar_t >();
                        status = TryNtReadVirtualMemory( hProcess, procParameters.CommandLine.Buffer, cmdLineBuffer, procParameters.CommandLine.Length, 0 );
                        if ( WIN32_NT_SUCCESS( status ) )
                        {
                            // set the null terminator
                            *(PWSTR)RtlOffsetToPointer( cmdLineBuffer, procParameters.CommandLine.Length ) = 0;

                            CVariant convertor;
                            convertor.LinkTo( cmdLineBuffer, buffer.GetDataSize(), GUCEF_DATATYPE_UTF16_STRING );
                            CString cmdLine = convertor.AsString();

                            // We only care about 'extra' params passed to a program not the default first argument on Windows which is automatically added
                            // this creates a asymmetrical relationship between setting and getting the command line which is platform specific, so we remove it
                            // we are already obtaining the image path anyway so that can be used to find the exe location
                            if ( !cmdLine.IsNULLOrEmpty() && cmdLine[ 0 ] == '"' )
                            {
                                cmdLine = cmdLine.CutEnvelopedSubstr( "\"", "\"", 0 );
                                if ( !cmdLine.IsNULLOrEmpty() && cmdLine[ 0 ] == ' ' )
                                    cmdLine = cmdLine.CutChars( 1, true, 0 );
                            }

                            info.SetCommandLineArgs( cmdLine );
                        }
                        else
                        {
                            totalSuccess = false;
                        }
                    }
                    else
                    {
                        totalSuccess = false;
                    }
                }
                if ( procParameters.ImagePathName.Length > 0 )
                {
                    UInt32 strByteSize = ( procParameters.ImagePathName.Length * sizeof(WCHAR) ) + sizeof(WCHAR);
                    if ( buffer.SetBufferSize( strByteSize, false, true ) )
                    {
                        buffer.SetDataSize( strByteSize );
                        wchar_t* imgPathBuffer = buffer.AsTypePtr< wchar_t >();
                        status = TryNtReadVirtualMemory( hProcess, procParameters.ImagePathName.Buffer, imgPathBuffer, procParameters.ImagePathName.Length, 0 );
                        if ( WIN32_NT_SUCCESS( status ) )
                        {
                            // set the null terminator
                            *(PWSTR)RtlOffsetToPointer( imgPathBuffer, procParameters.ImagePathName.Length ) = 0;

                            CVariant convertor;
                            convertor.LinkTo( imgPathBuffer, buffer.GetDataSize(), GUCEF_DATATYPE_UTF16_STRING );
                            info.SetImagePath( convertor.AsString() );
                        }
                        else
                        {
                            totalSuccess = false;
                        }
                    }
                    else
                    {
                        totalSuccess = false;
                    }
                }
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessInformation:TryGetProcessInformation: Failed TryNtReadVirtualMemory" );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ProcessInformation:TryGetProcessInformation: Failed TryNtQueryInformationProcess" );
        }

        ::CloseHandle( hProcess );

        return totalSuccess;

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

        bool totalSuccess = true;

        CString exeImagePath;
        if ( GetExeImagePathForProcessId( pid, exeImagePath ) )
            info.SetImagePath( exeImagePath );
        else
            totalSuccess = false;

        CString cmdLineArgsPath = "/proc/" + ToString( pid ) + "/cmdline";

        // the /proc/<prodId>/cmdline data is an array of null terminated strings
        CDynamicBuffer cmdLineArgData;
        if ( cmdLineArgData.LoadContentFromFile( cmdLineArgsPath ) )
        {
            // We need to make sure we read beyond the null terminators or we wont get all the args
            CString::StringVector args = cmdLineArgData.ParseUtf8StringElements( '\0', false );
            if ( args.size() > 1 )
            {
                // We only care about 'extra' params passed to a program not the default first argument on Windows which is automatically added
                // this creates a asymmetrical relationship between setting and getting the command line which is platform specific, so we remove it
                // we are already obtaining the image path anyway so that can be used to find the exe location
                args.erase( args.begin() );

                CString cmdLineArgs = ToString( args, ' ' );
                info.SetCommandLineArgs( cmdLineArgs );
            }
        }
        else
            totalSuccess = false;

        return totalSuccess;

        #else

        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "ProcessInformation:TryGetProcessInformation: Platform has no supported implementation" );
        return false;

        #endif
    }

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
