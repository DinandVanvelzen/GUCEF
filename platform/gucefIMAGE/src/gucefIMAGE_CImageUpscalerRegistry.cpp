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

#include "gucefIMAGE_CImageUpscalerRegistry.h"

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

CImageUpscalerRegistry::CImageUpscalerRegistry( void )
    : m_upscalers()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageUpscalerRegistry::~CImageUpscalerRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImageUpscalerRegistry::Register( CImageUpscaler* upscaler )
{GUCEF_TRACE;

    if ( GUCEF_NULL == upscaler )
        return false;
    CORE::CString name = upscaler->GetName();
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32)m_upscalers.size(); ++i )
    {
        if ( m_upscalers[ i ]->GetName() == name )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageUpscalerRegistry: An image upscaler named \"" << name << "\" is already registered" );
            return false;
        }
    }
    m_upscalers.push_back( upscaler );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ImageUpscalerRegistry: Registered image upscaler \"" << name << "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImageUpscalerRegistry::Unregister( const CORE::CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TUpscalerVector::iterator i=m_upscalers.begin(); i!=m_upscalers.end(); ++i )
    {
        if ( (*i)->GetName() == name )
        {
            m_upscalers.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CImageUpscaler*
CImageUpscalerRegistry::Get( const CORE::CString& name ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32)m_upscalers.size(); ++i )
        if ( m_upscalers[ i ]->GetName() == name )
            return m_upscalers[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CImageUpscalerRegistry::TNameVector
CImageUpscalerRegistry::GetNames( void ) const
{GUCEF_TRACE;

    TNameVector names;
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32)m_upscalers.size(); ++i )
        names.push_back( m_upscalers[ i ]->GetName() );
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
