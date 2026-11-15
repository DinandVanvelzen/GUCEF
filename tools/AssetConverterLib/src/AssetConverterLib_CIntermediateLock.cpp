/*
 *  AssetConverterLib: the lock of an intermediate folder, so that two runs never write it at the same time
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CIntermediateLock.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CIntermediateLock::LockFileName = ".lock";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
MakeToken( void )
{GUCEF_TRACE;

    UInt8 bytes[ 16 ];
    CORE::CString token;
    if ( !CORE::CRandom::GetRandomBytes( bytes, sizeof( bytes ) ) || !CORE::Base64UrlEncode( bytes, sizeof( bytes ), token, false ) )
        token = CORE::ToString( MT::GetProcessID() ) + "-" + CORE::ToString( MT::PrecisionTickCount() );
    return token;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIntermediateLock::CIntermediateLock( const CORE::CString& intermediateDir )
    : m_intermediateDir( intermediateDir )
    , m_vfsLockPath( CVfsPaths::Join( intermediateDir, LockFileName ) )
    , m_nativeLockPath()
    , m_token( MakeToken() )
    , m_holder()
    , m_held( false )
{GUCEF_TRACE;

    m_held = Acquire();
}

/*-------------------------------------------------------------------------*/

CIntermediateLock::~CIntermediateLock()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CIntermediateLock::IsHeld( void ) const
{GUCEF_TRACE;

    return m_held;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIntermediateLock::GetHolder( void ) const
{GUCEF_TRACE;

    return m_holder;
}

/*-------------------------------------------------------------------------*/

bool
CIntermediateLock::ResolveNativeLockPath( CORE::CString& nativeLockPath ) const
{GUCEF_TRACE;

    // the VFS gives the disk path of files that exist only: a probe file of this lock finds the folder
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString probe = CVfsPaths::Join( m_intermediateDir, LockFileName + ".probe." + m_token );
    CORE::CDynamicBuffer probeContent( 1, true );
    probeContent.Append( "p", 1 );
    if ( !vfs.StoreAsFile( probe, probeContent, 0, true ) )
        return false;
    CORE::CString nativeProbe;
    const bool resolved = vfs.GetActualFilePath( probe, nativeProbe ) && CORE::FileExists( nativeProbe );
    vfs.DeleteFile( probe, true );
    if ( !resolved )
        return false;
    nativeLockPath = CORE::StripFilename( nativeProbe );
    CORE::AppendToPath( nativeLockPath, LockFileName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIntermediateLock::ParseLock( const CORE::CString& text, SLockInfo& info )
{GUCEF_TRACE;

    info.processId = 0;
    info.host.Clear();
    info.started.Clear();
    info.token.Clear();
    CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        CORE::CString line = lines[ i ].RemoveChar( '\r' );
        Int32 equals = line.HasChar( '=', true );
        if ( equals <= 0 )
            continue;
        CORE::CString name = line.SubstrFromRange( 0, (UInt32) equals );
        CORE::CString value = line.CutChars( (UInt32) equals + 1, true );
        if ( "pid" == name )
            info.processId = CORE::StringToUInt32( value, 0 );
        else if ( "host" == name )
            info.host = value;
        else if ( "started" == name )
            info.started = value;
        else if ( "token" == name )
            info.token = value;
    }
    return 0 != info.processId && !info.token.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CIntermediateLock::ReadLock( SLockInfo& info ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    if ( !m_nativeLockPath.IsNULLOrEmpty() )
    {
        if ( !content.LoadContentFromFile( m_nativeLockPath ) )
            return false;
    }
    else
    {
        if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( content, m_vfsLockPath, "rb" ) )
            return false;
    }
    CORE::CString text( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
    return ParseLock( text, info );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIntermediateLock::DescribeHolder( const SLockInfo& info )
{GUCEF_TRACE;

    return "process " + CORE::ToString( info.processId ) + " on host " + info.host + " since " + info.started;
}

/*-------------------------------------------------------------------------*/

bool
CIntermediateLock::IsStale( const SLockInfo& info ) const
{GUCEF_TRACE;

    if ( info.host != CORE::GetHostname() || info.processId == MT::GetProcessID() )
        return false;
    bool alive = true;
    if ( CORE::CheckOnProcessAliveStatus( (CORE::TProcessId) info.processId, alive ) )
        return !alive;
    // the platform could not tell: the process list can
    CORE::TProcessIdVector processes;
    if ( !CORE::GetProcessList( processes ) || processes.empty() )
        return false;
    for ( size_t i=0; i<processes.size(); ++i )
        if ( (UInt32) processes[ i ] == info.processId )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIntermediateLock::RemoveLockFile( void )
{GUCEF_TRACE;

    if ( !m_nativeLockPath.IsNULLOrEmpty() )
        return CORE::DeleteFile( m_nativeLockPath );
    return VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( m_vfsLockPath, true );
}

/*-------------------------------------------------------------------------*/

bool
CIntermediateLock::Acquire( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( m_intermediateDir ) )
        vfs.CreateDir( m_intermediateDir );

    const bool native = ResolveNativeLockPath( m_nativeLockPath );
    if ( !native )
    {
        m_nativeLockPath.Clear();
        if ( !vfs.IsWriteable( m_intermediateDir ) )
        {
            m_holder = "the intermediate folder " + m_intermediateDir + " cannot be written";
            return false;
        }
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "IntermediateLock: The folder " << m_intermediateDir << " is not on a file system: its lock is taken through the VFS, which does not exclude runs that start at the same moment" );
    }

    CORE::CString content = "pid=" + CORE::ToString( MT::GetProcessID() ) + "\nhost=" + CORE::GetHostname() +
                            "\nstarted=" + CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( true, true ) +
                            "\ntoken=" + m_token + "\n";
    const UInt32 contentBytes = content.ByteSize() > 0 ? content.ByteSize() - 1 : 0;

    for ( UInt32 attempt=0; attempt<2; ++attempt )
    {
        if ( native )
        {
            CORE::CString problem;
            if ( CORE::CreateOwnerOnlyFile( m_nativeLockPath, content.C_String(), contentBytes, problem ) )
            {
                m_holder.Clear();
                return true;
            }
            if ( !CORE::FileExists( m_nativeLockPath ) )
            {
                m_holder = "the lock file " + m_nativeLockPath + " cannot be made: " + problem;
                return false;
            }
        }
        else
        if ( !vfs.FileExists( m_vfsLockPath ) )
        {
            CORE::CDynamicBuffer buffer( content.C_String(), contentBytes, true );
            if ( vfs.StoreAsFile( m_vfsLockPath, buffer, 0, true ) )
            {
                m_holder.Clear();
                return true;
            }
            m_holder = "the lock file " + m_vfsLockPath + " cannot be written";
            return false;
        }

        // there is a lock: whose, and is it stale?
        SLockInfo info;
        if ( !ReadLock( info ) )
        {
            m_holder = "an unreadable lock (" + ( native ? m_nativeLockPath : m_vfsLockPath ) + ")";
            return false;
        }
        m_holder = DescribeHolder( info );
        if ( 0 != attempt || !IsStale( info ) )
            return false;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "IntermediateLock: The lock of " << m_intermediateDir << " held by " << m_holder << " is stale (the process is gone): it is taken over" );
        if ( !RemoveLockFile() )
            return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CIntermediateLock::Release( void )
{GUCEF_TRACE;

    if ( !m_held )
        return;
    m_held = false;
    SLockInfo info;
    if ( ReadLock( info ) && info.token == m_token )
        RemoveLockFile();
    else
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "IntermediateLock: The lock of " << m_intermediateDir << " is no longer this run's: it is left as it is" );
}

/*-------------------------------------------------------------------------*/

bool
CIntermediateLock::GetCurrentHolder( const CORE::CString& intermediateDir, CORE::CString& holder )
{GUCEF_TRACE;

    holder.Clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString vfsLockPath = CVfsPaths::Join( intermediateDir, LockFileName );
    if ( !vfs.FileExists( vfsLockPath ) )
        return false;
    CORE::CDynamicBuffer content;
    if ( !vfs.LoadFile( content, vfsLockPath, "rb" ) )
    {
        holder = "an unreadable lock";
        return true;
    }
    SLockInfo info;
    if ( !ParseLock( CORE::CString( (const char*) content.GetConstBufferPtr(), content.GetDataSize() ), info ) )
        holder = "an unreadable lock";
    else
        holder = DescribeHolder( info );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
