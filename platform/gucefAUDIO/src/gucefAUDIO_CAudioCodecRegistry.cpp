/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUDIO_CAudioCodecRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioCodecRegistry::CAudioCodecRegistry( void )
    : CORE::CTGlobalONRegistry< CIAudioCodec, MT::CMutex >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioCodecRegistry::~CAudioCodecRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioCodecRegistry::TryFindCodec( const CString& codecType, TAudioCodecPtr& codec ) const
{GUCEF_TRACE;

    // a leading dot is accepted, as in ".wav"
    CString name = codecType;
    if ( 0 == name.HasChar( '.' ) )
        name = name.CutChars( 1, true );
    return TryLookup( name.Lowercase(), codec );
}

/*-------------------------------------------------------------------------*/

bool
CAudioCodecRegistry::TryFindCodecForFilename( const CString& filename, TAudioCodecPtr& codec ) const
{GUCEF_TRACE;

    // only the file name is considered, a dot in a directory name is not an extension
    const CString name = CORE::ExtractFilename( filename );
    const Int32 dot = name.HasChar( '.', false );
    if ( dot < 0 || (UInt32) dot + 1 >= name.Length() )
        return false;
    return TryFindCodec( name.CutChars( (UInt32) dot + 1, true ), codec );
}

bool
CAudioCodecRegistry::TryFindCodecForData( CORE::CIOAccess& encodedInput, TAudioCodecPtr& codec, UInt32* confidence ) const
{GUCEF_TRACE;

    CORE::CString::StringVector codecTypes;
    GetRegisteredObjNames( codecTypes );

    TAudioCodecPtr best;
    UInt32 bestConfidence = 0;
    for ( CORE::CString::StringVector::const_iterator i = codecTypes.begin(); i != codecTypes.end(); ++i )
    {
        TAudioCodecPtr candidate;
        if ( !TryLookup( (*i), candidate ) || candidate.IsNULL() )
            continue;

        UInt32 answer = 0;
        if ( candidate->Probe( encodedInput, answer ) && answer > bestConfidence )
        {
            best = candidate;
            bestConfidence = answer;
        }
    }

    if ( best.IsNULL() )
        return false;

    codec = best;
    if ( GUCEF_NULL != confidence )
        *confidence = bestConfidence;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
