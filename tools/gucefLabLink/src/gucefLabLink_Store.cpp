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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_Store.h"

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

/** The lines of the text, without empty ones */
static CORE::CString::StringVector
Lines( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString::StringVector lines;
    CORE::CString::StringVector all = text.ParseElements( '\n', false );
    for ( size_t i=0; i<all.size(); ++i )
    {
        CORE::CString line = all[ i ];
        while ( line.EndsWith( "\r" ) )
            line = line.CutChars( 1, false );
        if ( !line.IsNULLOrEmpty() )
            lines.push_back( line );
    }
    return lines;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStore::CStore( const CORE::CString& stateDir )
    : m_stateDir( stateDir )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStore::GetStateDir( void ) const
{GUCEF_TRACE;

    return m_stateDir;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStore::GetPath( const char* fileName ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_stateDir, fileName );
}

/*-------------------------------------------------------------------------*/

bool
CStore::ReadFile( const char* fileName, CORE::CString& content, bool& missing, CORE::CString& problem ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    missing = false;
    if ( !CORE::ReadOwnerOnlyFile( GetPath( fileName ), buffer, problem, &missing ) )
        return false;
    content = ( 0 == buffer.GetDataSize() ? CORE::CString::Empty : CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStore::WriteFile( const char* fileName, const CORE::CString& content, CORE::CString& problem ) const
{GUCEF_TRACE;

    const CORE::CString path = GetPath( fileName );
    const UInt32 size = content.Length();
    if ( !CORE::FileExists( path ) )
    {
        if ( !CORE::CreateDirs( m_stateDir ) && !CORE::DirExists( m_stateDir ) )
        {
            problem = "cannot make " + m_stateDir;
            return false;
        }
        return CORE::CreateOwnerOnlyFile( path, content.C_String(), size, problem );
    }
    // keeps the permissions of the file it replaces
    if ( !CORE::WriteFileAtomically( path, content.C_String(), size ) )
    {
        problem = "cannot write " + path;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStore::ReadSecret( CORE::CString& secret, CORE::CString& problem ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    bool missing = false;
    CORE::CString content;
    if ( !ReadFile( "secret", content, missing, problem ) )
    {
        if ( missing )
            problem = "there is no lab secret (" + GetPath( "secret" ) + ")";
        return false;
    }
    secret = StripText( content );
    if ( secret.IsNULLOrEmpty() )
    {
        problem = "the lab secret is empty";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStore::WriteSecret( const CORE::CString& secret, CORE::CString& problem ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return WriteFile( "secret", StripText( secret ) + "\n", problem );
}

/*-------------------------------------------------------------------------*/

bool
CStore::LoadIdentity( SIdentity& identity, CORE::CString& problem ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    identity = SIdentity();
    bool missing = false;
    CORE::CString content;
    if ( !ReadFile( "identity", content, missing, problem ) )
    {
        if ( !missing )
            return false;
        identity.machineId = MakeMachineId();
        if ( identity.machineId.IsNULLOrEmpty() )
        {
            problem = "no randomness for a machine id";
            return false;
        }
        problem.Clear();
        return WriteFile( "identity", "machineId=" + identity.machineId + "\n", problem );
    }

    CORE::CString::StringVector lines = Lines( content );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const Int32 equals = lines[ i ].HasChar( '=' );
        if ( equals <= 0 )
            continue;
        const CORE::CString key = lines[ i ].SubstrToIndex( (UInt32) equals, true );
        const CORE::CString value = StripText( lines[ i ].CutChars( (UInt32) equals + 1, true ) );
        if ( key == "machineId" )
            identity.machineId = value;
        else if ( key == "token" )
            identity.token = value;
        else if ( key == "clusterKey" )
            identity.clusterKey = value;
        else if ( key == "cluster" )
            identity.cluster = value;
    }
    if ( identity.machineId.IsNULLOrEmpty() )
    {
        problem = GetPath( "identity" ) + " has no machine id";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStore::SaveIdentity( const SIdentity& identity, CORE::CString& problem ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString content = "machineId=" + identity.machineId + "\n";
    if ( !identity.token.IsNULLOrEmpty() )
        content += "token=" + identity.token + "\n";
    if ( !identity.clusterKey.IsNULLOrEmpty() )
        content += "clusterKey=" + identity.clusterKey + "\n";
    if ( !identity.cluster.IsNULLOrEmpty() )
        content += "cluster=" + identity.cluster + "\n";
    return WriteFile( "identity", content, problem );
}

/*-------------------------------------------------------------------------*/

bool
CStore::LoadApprovals( GUCEF::vector< SApproval >& approvals, CORE::CString& problem ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    approvals.clear();
    bool missing = false;
    CORE::CString content;
    if ( !ReadFile( "approvals", content, missing, problem ) )
    {
        if ( missing )
        {
            problem.Clear();
            return true;
        }
        return false;
    }
    CORE::CString::StringVector lines = Lines( content );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        CORE::CString::StringVector fields = lines[ i ].ParseElements( '\t', true );
        if ( fields.size() < 5 )
            continue;
        SApproval approval;
        approval.state = fields[ 0 ];
        approval.machineId = fields[ 1 ];
        approval.name = fields[ 2 ];
        approval.address = fields[ 3 ];
        approval.os = fields[ 4 ];
        if ( fields.size() > 5 )
            approval.token = fields[ 5 ];
        approvals.push_back( approval );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** A field of a line of the approvals file: no tabs or line ends in it */
static CORE::CString
Field( const CORE::CString& value )
{GUCEF_TRACE;

    return value.ReplaceChar( '\t', ' ' ).ReplaceChar( '\n', ' ' ).ReplaceChar( '\r', ' ' );
}

/*-------------------------------------------------------------------------*/

bool
CStore::SaveApprovals( const GUCEF::vector< SApproval >& approvals, CORE::CString& problem ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString content;
    for ( size_t i=0; i<approvals.size(); ++i )
    {
        const SApproval& a = approvals[ i ];
        content += Field( a.state ) + "\t" + Field( a.machineId ) + "\t" + Field( a.name ) + "\t" + Field( a.address ) + "\t" +
                   Field( a.os ) + "\t" + Field( a.token ) + "\n";
    }
    return WriteFile( "approvals", content, problem );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
