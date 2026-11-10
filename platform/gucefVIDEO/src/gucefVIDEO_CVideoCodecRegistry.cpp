/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#include "gucefVIDEO_CVideoCodecRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVideoCodecRegistry::CVideoCodecRegistry( void )
    : CORE::CTGlobalONRegistry< CIVideoCodec, MT::CMutex >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoCodecRegistry::~CVideoCodecRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecRegistry::TryFindCodec( const CString& codecType, TVideoCodecPtr& codec ) const
{GUCEF_TRACE;

    // a leading dot is accepted, as in ".smk"
    CString name = codecType;
    if ( 0 == name.HasChar( '.' ) )
        name = name.CutChars( 1, true );
    return TryLookup( name.Lowercase(), codec );
}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecRegistry::TryFindCodecForFilename( const CString& filename, TVideoCodecPtr& codec ) const
{GUCEF_TRACE;

    // only the file name is considered, a dot in a directory name is not an extension
    const CString name = CORE::ExtractFilename( filename );
    const Int32 dot = name.HasChar( '.', false );
    if ( dot < 0 || (UInt32) dot + 1 >= name.Length() )
        return false;
    return TryFindCodec( name.CutChars( (UInt32) dot + 1, true ), codec );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
