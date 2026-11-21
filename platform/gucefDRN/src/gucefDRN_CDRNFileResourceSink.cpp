/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "gucefDRN_CDRNFileResourceSink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNFileResourceSink::CDRNFileResourceSink( const CORE::CString& targetPath )
    : CIDRNResourceSink()
    , m_targetPath( targetPath )
    , m_file()
    , m_windowOffset( 0 )
    , m_windowLength( 0 )
    , m_wholeResource( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNFileResourceSink::~CDRNFileResourceSink()
{GUCEF_TRACE;

    m_file.Close();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNFileResourceSink::GetTargetPath( void ) const
{GUCEF_TRACE;

    return m_targetPath;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNFileResourceSink::GetPartPath( void ) const
{GUCEF_TRACE;

    return m_targetPath + ".drnpart";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNFileResourceSink::GetStatePath( void ) const
{GUCEF_TRACE;

    return m_targetPath + ".drnstate";
}

/*-------------------------------------------------------------------------*/

bool
CDRNFileResourceSink::IsResumable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNFileResourceSink::PeekPartialState( CDRNResourcePartialState& state )
{GUCEF_TRACE;

    state.Clear();
    CORE::CString statePath = GetStatePath();
    if ( !CORE::FileExists( statePath ) || !CORE::FileExists( GetPartPath() ) )
        return false;
    CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( statePath ) || !state.Deserialize( content ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNFileResourceSink: The state \"" << statePath << "\" cannot be used, the transfer starts over" );
        state.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNFileResourceSink::RemovePartial( void )
{GUCEF_TRACE;

    m_file.Close();
    CORE::CString partPath = GetPartPath();
    CORE::CString statePath = GetStatePath();
    if ( CORE::FileExists( partPath ) )
        CORE::DeleteFile( partPath );
    if ( CORE::FileExists( statePath ) )
        CORE::DeleteFile( statePath );
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileResourceSink::Begin( const CORE::CString& provider       ,
                             const CDRNResourceInfo& info        ,
                             UInt64 windowOffset                 ,
                             UInt64 windowLength                 ,
                             bool resume                         ,
                             CDRNResourcePartialState& state     )
{GUCEF_TRACE;

    m_file.Close();
    m_windowOffset = windowOffset;
    m_windowLength = windowLength;
    m_wholeResource = 0 == windowOffset && windowLength == info.size;

    CORE::CString partPath = GetPartPath();
    if ( !CORE::CreatePathDirectories( partPath ) )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;

    bool kept = false;
    if ( resume && m_wholeResource )
    {
        CDRNResourcePartialState persisted;
        if ( PeekPartialState( persisted ) && persisted.IsOf( provider, info.name, info ) )
        {
            // what the part file does not reach was not written, whatever the state says
            UInt64 partSize = CORE::FileSize( partPath );
            for ( UInt64 i=persisted.FindFirstHeld( 0 ); i<persisted.GetChunkCount(); i=persisted.FindFirstHeld( i+1 ) )
            {
                if ( i * DRN_RESOURCE_CHUNK_SIZE + info.GetChunkBytes( i ) > partSize )
                    persisted.SetHeld( i, false );
            }
            if ( m_file.Open( partPath, "r+b" ) )
            {
                state = persisted;
                kept = true;
            }
        }
    }

    if ( !kept )
    {
        RemovePartial();
        if ( !state.Reset( provider, info.name, info ) )
            return DRN_RESSTATUS_TOO_LARGE;
        if ( !m_file.Open( partPath, "w+b" ) )
            return DRN_RESSTATUS_LOCAL_IO_ERROR;
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNFileResourceSink: Resuming \"" << m_targetPath << "\" with " << state.GetHeldCount() << " of " << state.GetChunkCount() << " chunks" );
    }
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileResourceSink::Write( UInt64 offset, const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( !m_file.Opened() || offset < m_windowOffset || offset - m_windowOffset + size > m_windowLength )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    if ( 0 != m_file.Setpos( offset - m_windowOffset ) || size != m_file.Write( data, 1, size ) )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CDRNFileResourceSink::SaveState( const CDRNResourcePartialState& state )
{GUCEF_TRACE;

    if ( !m_wholeResource || !state.IsValid() )
        return true;
    CORE::CDynamicBuffer content;
    state.Serialize( content );
    return CORE::WriteFileAtomically( GetStatePath(), content );
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileResourceSink::Commit( const CDRNResourcePartialState& state )
{GUCEF_TRACE;

    if ( !m_file.Opened() )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    m_file.Flush();
    return SaveState( state ) ? DRN_RESSTATUS_OK : DRN_RESSTATUS_LOCAL_IO_ERROR;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileResourceSink::ReadBack( UInt64 offset, UInt32 size, void* destination )
{GUCEF_TRACE;

    if ( !m_file.Opened() || offset < m_windowOffset )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    m_file.Flush();
    if ( 0 != m_file.Setpos( offset - m_windowOffset ) || size != m_file.Read( destination, 1, size ) )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNFileResourceSink::Finish( void )
{GUCEF_TRACE;

    if ( !m_file.Opened() )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    m_file.Flush();
    m_file.Close();
    if ( !CORE::MoveFileAtomically( GetPartPath(), m_targetPath, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNFileResourceSink: Cannot move the received file into place at \"" << m_targetPath << "\"" );
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    }
    CORE::CString statePath = GetStatePath();
    if ( CORE::FileExists( statePath ) )
        CORE::DeleteFile( statePath );
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDRNFileResourceSink::Abort( const CDRNResourcePartialState& state, bool discard )
{GUCEF_TRACE;

    if ( discard || !m_wholeResource )
    {
        RemovePartial();
        return;
    }
    if ( m_file.Opened() )
    {
        m_file.Flush();
        m_file.Close();
        // only while the part is there: another transfer of the same target may have finished it meanwhile
        if ( CORE::FileExists( GetPartPath() ) )
            SaveState( state );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
