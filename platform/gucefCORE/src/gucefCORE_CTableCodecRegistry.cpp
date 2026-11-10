/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#include "gucefCORE_CTableCodecRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The extension in lower case without leading dots; extensions are ASCII */
static CString
NormalizeExtension( const CString& extension )
{GUCEF_TRACE;

    const UInt32 byteLength = extension.ByteSize() > 0 ? extension.ByteSize() - 1 : 0;
    const char* text = extension.C_String();
    UInt32 begin = 0;
    while ( begin < byteLength && '.' == text[ begin ] )
        ++begin;

    CString normalized;
    for ( UInt32 i = begin; i < byteLength; ++i )
    {
        char c = text[ i ];
        if ( c >= 'A' && c <= 'Z' )
            c = (char) ( c - 'A' + 'a' );
        normalized += c;
    }
    return normalized;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTableCodecRegistry::CTableCodecRegistry( void )
    : CTRegistry< CITableCodec, MT::CMutex >()
    , m_dataLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTableCodecRegistry::~CTableCodecRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CTableCodecRegistry::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTableCodecRegistry::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_dataLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTableCodecRegistry::Unlock( void ) const
{GUCEF_TRACE;

    return m_dataLock.Unlock();
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecRegistry::TryGetCodecForFileExtension( const CString& extension, TTableCodecPtr& codec ) const
{GUCEF_TRACE;

    const CString wanted = NormalizeExtension( extension );
    if ( wanted.IsNULLOrEmpty() )
    {
        codec.Unlink();
        return false;
    }

    // The lock is held while the codecs are inspected; the registered names are ordered, which makes the winner of a shared extension deterministic
    MT::CObjectScopeLock lock( this );

    TRegisteredObjPtrVector codecs;
    GetRegisteredObjs( codecs );
    CString winnerTypeName;
    TTableCodecPtr winner;
    for ( TRegisteredObjPtrVector::iterator i = codecs.begin(); i != codecs.end(); ++i )
    {
        if ( (*i).IsNULL() )
            continue;

        CString::StringSet extensions;
        (*i)->GetFileExtensions( extensions );
        bool listed = false;
        for ( CString::StringSet::iterator n = extensions.begin(); n != extensions.end() && !listed; ++n )
        {
            listed = ( NormalizeExtension( *n ) == wanted );
        }

        if ( listed )
        {
            CString typeName = (*i)->GetTypeName();
            if ( winner.IsNULL() || typeName < winnerTypeName )
            {
                winner = *i;
                winnerTypeName = typeName;
            }
        }
    }

    codec = winner;
    return !winner.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecRegistry::TryGetCsvCodec( TTableCodecPtr& codec ) const
{GUCEF_TRACE;

    return TryLookup( "csv", codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
