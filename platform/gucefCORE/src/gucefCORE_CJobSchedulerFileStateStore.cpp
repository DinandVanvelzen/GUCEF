/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CJobSchedulerFileStateStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CJobSchedulerFileStateStore::DefaultCodecName = "json";

/*-------------------------------------------------------------------------*/

CJobSchedulerFileStateStore::CJobSchedulerFileStateStore( const CString& filePath, const CString& codecName )
    : CIJobSchedulerStateStore()
    , m_lock()
    , m_filePath( filePath )
    , m_codecName( codecName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobSchedulerFileStateStore::~CJobSchedulerFileStateStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void CJobSchedulerFileStateStore::SetFilePath( const CString& filePath ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_filePath = filePath; }
CString CJobSchedulerFileStateStore::GetFilePath( void ) const {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); return m_filePath; }
void CJobSchedulerFileStateStore::SetCodecName( const CString& codecName ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_codecName = codecName; }
CString CJobSchedulerFileStateStore::GetCodecName( void ) const {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); return m_codecName; }

/*-------------------------------------------------------------------------*/

bool
CJobSchedulerFileStateStore::LoadState( CDataNode& state, CString& error )
{GUCEF_TRACE;

    CString filePath = GetFilePath();
    CString codecName = GetCodecName();
    state.Clear();

    if ( filePath.IsNULLOrEmpty() )
    {
        error = "the state store has no file";
        return false;
    }
    if ( !FileExists( filePath ) )
        return true;    // a first start: nothing kept yet

    CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        error = "the data tree codec \"" + codecName + "\" is not available (is its plugin loaded?)";
        return false;
    }

    CDynamicBuffer buffer;
    bool parsed = false;
    if ( buffer.LoadContentFromFile( filePath ) )
    {
        CDynamicBufferAccess access( buffer );
        parsed = codec->BuildDataTree( &state, &access );
    }
    if ( parsed )
        return true;

    // kept for whoever wants to look at it; the scheduler starts from its defaults
    state.Clear();
    CString badPath = filePath + ".bad";
    if ( MoveFile( filePath, badPath, true ) )
        error = "the state file " + filePath + " cannot be read, it is kept as " + badPath;
    else
        error = "the state file " + filePath + " cannot be read and cannot be moved aside";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CJobSchedulerFileStateStore::SaveState( const CDataNode& state, CString& error )
{GUCEF_TRACE;

    CString filePath = GetFilePath();
    CString codecName = GetCodecName();
    if ( filePath.IsNULLOrEmpty() )
    {
        error = "the state store has no file";
        return false;
    }

    CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        error = "the data tree codec \"" + codecName + "\" is not available (is its plugin loaded?)";
        return false;
    }

    CDynamicBuffer buffer;
    {
        CDynamicBufferAccess access( buffer );
        if ( !codec->StoreDataTree( &state, &access ) )
        {
            error = "the state cannot be encoded with the codec \"" + codecName + "\"";
            return false;
        }
    }
    if ( !WriteFileAtomically( filePath, buffer ) )
    {
        error = "the state file " + filePath + " cannot be written";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
