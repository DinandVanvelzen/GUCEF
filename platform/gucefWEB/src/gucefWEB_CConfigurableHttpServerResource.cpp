/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CConfigurableHttpServerResource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/
    
CConfigurableHttpServerResource::CConfigurableHttpServerResource( CORE::CIConfigurable* configurable  /* = GUCEF_NULL */ ,
                                                                  bool allowSerialize                 /* = true */       ,
                                                                  bool allowDeserialize               /* = true */       ,
                                                                  MT::CILockable* lockForConfigurable /* = GUCEF_NULL */ )
    : CCodecBasedHTTPServerResource()
    , m_configurable( configurable )
    , m_lockForConfigurable( lockForConfigurable )
{GUCEF_TRACE;

    m_allowDeserialize = allowDeserialize;
    m_allowSerialize = allowSerialize;
}

/*-------------------------------------------------------------------------*/
    
CConfigurableHttpServerResource::CConfigurableHttpServerResource( const CConfigurableHttpServerResource& src )
    : CCodecBasedHTTPServerResource( src )
    , m_configurable( src.m_configurable )
    , m_lockForConfigurable( src.m_lockForConfigurable )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CConfigurableHttpServerResource::~CConfigurableHttpServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CConfigurableHttpServerResource& 
CConfigurableHttpServerResource::operator=( const CConfigurableHttpServerResource& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CCodecBasedHTTPServerResource::operator=( src );
        m_configurable = src.m_configurable;
        m_lockForConfigurable = src.m_lockForConfigurable;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool 
CConfigurableHttpServerResource::Serialize( const CString& resourcePath         ,
                                            CORE::CDataNode& output             ,
                                            const CORE::CString& representation ,
                                            const CString& params               )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_configurable )
    {
        MT::CObjectScopeReadOnlyLock lock( m_lockForConfigurable );
        return m_configurable->SaveConfig( output );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CConfigurableHttpServerResource::TDeserializeState 
CConfigurableHttpServerResource::Deserialize( const CString& resourcePath   ,
                                              const CORE::CDataNode& input  ,
                                              const CString& representation ,
                                              bool isDeltaUpdateOnly        )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_configurable )
    {
        MT::CObjectScopeLock lock( m_lockForConfigurable );
        if ( m_configurable->LoadConfig( input ) )
            return CIHTTPServerResource::DESERIALIZESTATE_SUCCEEDED;
        else
            return CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;
    }
    return CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
