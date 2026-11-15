/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefIMAGE_CImageCompressorRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageCompressorRegistry::CImageCompressorRegistry( void )
    : m_compressors()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageCompressorRegistry::~CImageCompressorRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImageCompressorRegistry::Register( CImageCompressor* compressor )
{GUCEF_TRACE;

    if ( GUCEF_NULL == compressor )
        return false;
    CORE::CString name = compressor->GetName();
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32)m_compressors.size(); ++i )
    {
        if ( m_compressors[ i ]->GetName() == name )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageCompressorRegistry: An image compressor named \"" << name << "\" is already registered" );
            return false;
        }
    }
    m_compressors.push_back( compressor );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ImageCompressorRegistry: Registered image compressor \"" << name << "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImageCompressorRegistry::Unregister( const CORE::CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TCompressorVector::iterator i=m_compressors.begin(); i!=m_compressors.end(); ++i )
    {
        if ( (*i)->GetName() == name )
        {
            m_compressors.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CImageCompressor*
CImageCompressorRegistry::Get( const CORE::CString& name ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32)m_compressors.size(); ++i )
        if ( m_compressors[ i ]->GetName() == name )
            return m_compressors[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CImageCompressor*
CImageCompressorRegistry::FindByFormat( const CORE::CString& format ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32)m_compressors.size(); ++i )
    {
        CImageCompressor::TFormatVector formats = m_compressors[ i ]->GetFormats();
        for ( UInt32 n=0; n<(UInt32)formats.size(); ++n )
            if ( formats[ n ] == format )
                return m_compressors[ i ];
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CImageCompressorRegistry::TNameVector
CImageCompressorRegistry::GetNames( void ) const
{GUCEF_TRACE;

    TNameVector names;
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32)m_compressors.size(); ++i )
        names.push_back( m_compressors[ i ]->GetName() );
    return names;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
