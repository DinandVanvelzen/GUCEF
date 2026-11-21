/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#include "gucefCORE_macros.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

#include <stdio.h>
#include <io.h>
#include <windows.h>
#include <aclapi.h>
#undef min
#undef max

#ifndef GUCEF_CORE_CUTF16STRING_H
#include "gucefCORE_CUtf16String.h"
#define GUCEF_CORE_CUTF16STRING_H
#endif /* GUCEF_CORE_CUTF16STRING_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "gucefLabLink_WindowsHost.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The security of a state file or directory: owner Administrators; full control for SYSTEM and Administrators only */
class CStateSecurity
{
    public:

    explicit CStateSecurity( bool forDirectory )
        : m_acl( GUCEF_NULL )
        , m_valid( false )
    {GUCEF_TRACE;

        DWORD systemSize = sizeof( m_systemSid );
        DWORD adminsSize = sizeof( m_adminsSid );
        if ( FALSE == ::CreateWellKnownSid( WinLocalSystemSid, GUCEF_NULL, m_systemSid, &systemSize ) ||
             FALSE == ::CreateWellKnownSid( WinBuiltinAdministratorsSid, GUCEF_NULL, m_adminsSid, &adminsSize ) )
            return;

        EXPLICIT_ACCESSW access[ 2 ];
        ::ZeroMemory( access, sizeof( access ) );
        for ( int i=0; i<2; ++i )
        {
            access[ i ].grfAccessPermissions = FILE_ALL_ACCESS;
            access[ i ].grfAccessMode = SET_ACCESS;
            access[ i ].grfInheritance = forDirectory ? SUB_CONTAINERS_AND_OBJECTS_INHERIT : NO_INHERITANCE;
            access[ i ].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        }
        access[ 0 ].Trustee.TrusteeType = TRUSTEE_IS_USER;
        access[ 0 ].Trustee.ptstrName = (LPWSTR) (PSID) m_systemSid;
        access[ 1 ].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        access[ 1 ].Trustee.ptstrName = (LPWSTR) (PSID) m_adminsSid;
        if ( ERROR_SUCCESS != ::SetEntriesInAclW( 2, access, GUCEF_NULL, &m_acl ) )
        {
            m_acl = GUCEF_NULL;
            return;
        }

        // the descriptor of a new file: the owner is set at the creation, which needs the Administrators group in
        // the token of the process (an elevated administrator, or SYSTEM)
        if ( FALSE == ::InitializeSecurityDescriptor( &m_descriptor, SECURITY_DESCRIPTOR_REVISION ) ||
             FALSE == ::SetSecurityDescriptorOwner( &m_descriptor, (PSID) m_adminsSid, FALSE ) ||
             FALSE == ::SetSecurityDescriptorDacl( &m_descriptor, TRUE, m_acl, FALSE ) ||
             FALSE == ::SetSecurityDescriptorControl( &m_descriptor, SE_DACL_PROTECTED, SE_DACL_PROTECTED ) )
            return;
        m_valid = true;
    }

    ~CStateSecurity()
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_acl )
            ::LocalFree( m_acl );
    }

    bool IsValid( void ) const { return m_valid; }
    PACL GetAcl( void ) const { return m_acl; }
    PSID GetAdministrators( void ) { return (PSID) m_adminsSid; }
    SECURITY_DESCRIPTOR* GetDescriptor( void ) { return &m_descriptor; }

    private:

    CStateSecurity( const CStateSecurity& src );                /**< not implemented */
    CStateSecurity& operator=( const CStateSecurity& src );     /**< not implemented */

    BYTE m_systemSid[ SECURITY_MAX_SID_SIZE ];
    BYTE m_adminsSid[ SECURITY_MAX_SID_SIZE ];
    PACL m_acl;
    SECURITY_DESCRIPTOR m_descriptor;
    bool m_valid;
};

/*-------------------------------------------------------------------------*/

/** The text of a Windows error code, for a problem */
static CORE::CString
ErrorText( DWORD error )
{GUCEF_TRACE;

    return "Windows error " + CORE::ToString( (UInt32) error );
}

/*-------------------------------------------------------------------------*/

bool
ProtectStateDirectory( const CORE::CString& directory, CORE::CString& problem )
{GUCEF_TRACE;

    const CORE::CString path = CORE::RelativePath( directory );
    if ( !CORE::DirExists( path ) && !CORE::CreateDirs( path ) && !CORE::DirExists( path ) )
    {
        problem = "cannot make " + path;
        return false;
    }
    CStateSecurity security( true );
    if ( !security.IsValid() )
    {
        problem = "the access list of " + path + " cannot be made";
        return false;
    }
    CORE::CUtf16String widePath( path );
    const DWORD result = ::SetNamedSecurityInfoW( (LPWSTR) widePath.C_String(), SE_FILE_OBJECT,
                                                  OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                                                  security.GetAdministrators(), GUCEF_NULL, security.GetAcl(), GUCEF_NULL );
    if ( ERROR_SUCCESS != result )
    {
        problem = "the access of " + path + " cannot be limited to SYSTEM and the Administrators (" + ErrorText( result ) + "; run as administrator)";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
WriteProtectedStateFile( const CORE::CString& stateDir    ,
                         const CORE::CString& path        ,
                         const void* data                 ,
                         UInt32 size                      ,
                         CORE::CString& problem           )
{GUCEF_TRACE;

    problem.Clear();
    if ( GUCEF_NULL == data && size > 0 )
    {
        problem = "no data";
        return false;
    }
    if ( !CORE::DirExists( CORE::RelativePath( stateDir ) ) && !ProtectStateDirectory( stateDir, problem ) )
        return false;

    CStateSecurity security( false );
    if ( !security.IsValid() )
    {
        problem = "the access list of " + path + " cannot be made";
        return false;
    }
    SECURITY_ATTRIBUTES attributes;
    attributes.nLength = sizeof( attributes );
    attributes.lpSecurityDescriptor = security.GetDescriptor();
    attributes.bInheritHandle = FALSE;

    const CORE::CString actualPath = CORE::RelativePath( path );
    CORE::CString tempPath = actualPath + ".tmp";
    UInt8 randomBytes[ 4 ];
    if ( CORE::CRandom::GetRandomBytes( randomBytes, sizeof( randomBytes ) ) )
    {
        static const char* hexDigits = "0123456789abcdef";
        for ( UInt32 i=0; i<sizeof( randomBytes ); ++i )
        {
            tempPath += CORE::CString( hexDigits + ( randomBytes[ i ] >> 4 ), 1 );
            tempPath += CORE::CString( hexDigits + ( randomBytes[ i ] & 0xF ), 1 );
        }
    }
    CORE::CUtf16String wideTemp( tempPath );
    CORE::CUtf16String widePath( actualPath );
    const wchar_t* tempName = (const wchar_t*) wideTemp.C_String();
    const wchar_t* fileName = (const wchar_t*) widePath.C_String();

    // CREATE_NEW: nothing that exists at the temporary path is used, a planted file or link included
    HANDLE file = ::CreateFileW( tempName, GENERIC_WRITE, 0, &attributes, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, GUCEF_NULL );
    if ( INVALID_HANDLE_VALUE == file )
    {
        const DWORD error = ::GetLastError();
        problem = "cannot make " + tempPath + " (" + ErrorText( error ) + ( ERROR_INVALID_OWNER == error ? "; run as administrator)" : ")" );
        return false;
    }
    const char* bytes = static_cast< const char* >( data );
    UInt32 written = 0;
    bool success = true;
    while ( written < size )
    {
        DWORD chunk = 0;
        if ( FALSE == ::WriteFile( file, bytes + written, size - written, &chunk, GUCEF_NULL ) || 0 == chunk )
        {
            success = false;
            break;
        }
        written += (UInt32) chunk;
    }
    success = ( FALSE != ::FlushFileBuffers( file ) ) && success;
    ::CloseHandle( file );

    // a rename on the same volume: the file is the old one or the new one, whole, with the access of the new one
    if ( success && FALSE == ::MoveFileExW( tempName, fileName, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) )
    {
        problem = "cannot replace " + actualPath + " (" + ErrorText( ::GetLastError() ) + ")";
        ::DeleteFileW( tempName );
        return false;
    }
    if ( !success )
    {
        problem = "cannot write " + tempPath;
        ::DeleteFileW( tempName );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether a standard handle goes to a file or a pipe, which a console must not take over */
static bool
IsRedirected( DWORD standardHandle )
{GUCEF_TRACE;

    HANDLE handle = ::GetStdHandle( standardHandle );
    if ( GUCEF_NULL == handle || INVALID_HANDLE_VALUE == handle )
        return false;
    const DWORD type = ::GetFileType( handle );
    return FILE_TYPE_DISK == type || FILE_TYPE_PIPE == type;
}

/*-------------------------------------------------------------------------*/

/** Opens the console device for a C runtime stream and makes it the standard handle of Windows as well */
static void
ReopenOnConsole( FILE* stream, const char* device, const char* mode, DWORD standardHandle )
{GUCEF_TRACE;

    #ifdef _MSC_VER
    FILE* reopened = GUCEF_NULL;
    if ( 0 != ::freopen_s( &reopened, device, mode, stream ) || GUCEF_NULL == reopened )
        return;
    #else
    if ( GUCEF_NULL == ::freopen( device, mode, stream ) )
        return;
    #endif
    const intptr_t osHandle = ::_get_osfhandle( ::_fileno( stream ) );
    if ( -1 != osHandle )
        ::SetStdHandle( standardHandle, (HANDLE) osHandle );
}

/*-------------------------------------------------------------------------*/

void
AttachToConsole( void )
{GUCEF_TRACE;

    const bool outRedirected = IsRedirected( STD_OUTPUT_HANDLE );
    const bool errRedirected = IsRedirected( STD_ERROR_HANDLE );
    const bool inRedirected = IsRedirected( STD_INPUT_HANDLE );
    if ( GUCEF_NULL == ::GetConsoleWindow() )
    {
        // the console of the command prompt; a console of its own when it was not started from one
        if ( FALSE == ::AttachConsole( ATTACH_PARENT_PROCESS ) && ERROR_ACCESS_DENIED != ::GetLastError() )
            ::AllocConsole();
    }
    if ( !outRedirected )
        ReopenOnConsole( stdout, "CONOUT$", "w", STD_OUTPUT_HANDLE );
    if ( !errRedirected )
        ReopenOnConsole( stderr, "CONOUT$", "w", STD_ERROR_HANDLE );
    if ( !inRedirected )
        ReopenOnConsole( stdin, "CONIN$", "r", STD_INPUT_HANDLE );
}

/*-------------------------------------------------------------------------*/

UInt32
NativeProcessorArchitecture( void )
{GUCEF_TRACE;

    SYSTEM_INFO info;
    ::ZeroMemory( &info, sizeof( info ) );
    ::GetNativeSystemInfo( &info );
    return (UInt32) info.wProcessorArchitecture;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */
