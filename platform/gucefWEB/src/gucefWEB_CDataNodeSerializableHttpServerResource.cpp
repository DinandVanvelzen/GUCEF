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

#include <map>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */


#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

#ifndef GUCEF_CORE_CTDATANODESERIALIZABLEOBJ_H
#include "gucefCORE_CTDataNodeSerializableObj.h"
#define GUCEF_CORE_CTDATANODESERIALIZABLEOBJ_H
#endif /* GUCEF_CORE_CTDATANODESERIALIZABLEOBJ_H ? */

#ifndef GUCEF_CORE_DATANODE_SERIALIZABLES_H
#include "gucefCORE_datanode_serializables.h"
#define GUCEF_CORE_DATANODE_SERIALIZABLES_H
#endif /* GUCEF_CORE_DATANODE_SERIALIZABLES_H ? */

#ifndef GUCEF_CORE_CGENERICVALUETODATANODESERIALIZER_H
#include "gucefCORE_CGenericValueToDataNodeSerializer.h"
#define GUCEF_CORE_CGENERICVALUETODATANODESERIALIZER_H
#endif /* GUCEF_CORE_CGENERICVALUETODATANODESERIALIZER_H ? */

#ifndef GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#include "gucefWEB_CCodecBasedHTTPServerResource.h"
#define GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CSTDPARAMPARSER_H
#include "gucefWEB_CStdParamParser.h"
#define GUCEF_WEB_CSTDPARAMPARSER_H
#endif /* GUCEF_WEB_CSTDPARAMPARSER_H ? */

#include "gucefWEB_CDataNodeSerializableHttpServerResource.h"

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

CDataNodeSerializableHttpServerResource::CDataNodeSerializableHttpServerResource( void )
    : CCodecBasedHTTPServerResource()
    , m_serializerOptions( GUCEF_NULL )
    , m_serializable( GUCEF_NULL )
    , m_serializableLock( GUCEF_NULL )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = true;
    m_allowCreate = false;
}

/*-------------------------------------------------------------------------*/

CDataNodeSerializableHttpServerResource::CDataNodeSerializableHttpServerResource( CORE::CIDataNodeSerializable* serializable                   ,
                                                                                  const CORE::CDataNodeSerializableSettings* serializerOptions ,
                                                                                  MT::CILockable* serializableLock                             )
    : CCodecBasedHTTPServerResource()
    , m_serializerOptions( serializerOptions )
    , m_serializable( serializable )
    , m_serializableLock( serializableLock )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = true;
    m_allowCreate = false;
}

/*-------------------------------------------------------------------------*/

CDataNodeSerializableHttpServerResource::~CDataNodeSerializableHttpServerResource()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

void
CDataNodeSerializableHttpServerResource::LinkTo( const CORE::CDataNodeSerializableSettings* serializerOptions ,
                                                 CORE::CIDataNodeSerializable* serializable                   ,
                                                 MT::CILockable* serializableLock                             )
{GUCEF_TRACE;

    m_serializerOptions = serializerOptions;
    m_serializable = serializable;
    m_serializableLock = serializableLock;
}

/*-------------------------------------------------------------------------*/

void 
CDataNodeSerializableHttpServerResource::Unlink( void )
{GUCEF_TRACE;

    m_serializerOptions = GUCEF_NULL;
    m_serializable = GUCEF_NULL;
    m_serializableLock = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDataNodeSerializableHttpServerResource::Serialize( const CString& resourcePath         ,
                                                    CORE::CDataNode& output             ,
                                                    const CORE::CString& representation ,
                                                    const CString& params               )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_serializable )
        return false;

    // Make sure we always have some degree of serializer options
    const CORE::CDataNodeSerializableSettings* serializerOptions = m_serializerOptions;
    if ( GUCEF_NULL == serializerOptions )
    {
        static const CORE::CDataNodeSerializableSettings defaultDummy;
        serializerOptions = &defaultDummy;
    }

    bool foundSupportedParams = false;
    CStdParams stdParams( *serializerOptions );
    CORE::CTSharedPtr< const CORE::CDataNodeSerializableSettings, MT::CNoLock > requestSpecificSerializerOptions;
    if ( CStdParamParser::ParseStdParams( params, stdParams, foundSupportedParams ) && foundSupportedParams && stdParams.levelOfDetailWasProvided )
    {
        // We place the clone in a shared ptr so that when we leave this scope the clone is properly disposed of
        requestSpecificSerializerOptions = CORE::CTSharedPtr< CORE::CDataNodeSerializableSettings, MT::CNoLock >( static_cast< CORE::CDataNodeSerializableSettings* >( serializerOptions->Clone() ) );
        const_cast< CORE::CDataNodeSerializableSettings* >( requestSpecificSerializerOptions.GetPointerAlways() )->levelOfDetail = stdParams.levelOfDetail;
        serializerOptions = requestSpecificSerializerOptions.GetPointerAlways();
    }

    MT::CObjectScopeReadOnlyLock lock( m_serializableLock );

    if ( GUCEF_NULL != m_serializable )
        return m_serializable->Serialize( output, *serializerOptions );
    return false;
}

/*-------------------------------------------------------------------------*/

CDataNodeSerializableHttpServerResource::TDeserializeState 
CDataNodeSerializableHttpServerResource::Deserialize( const CString& resourcePath   ,
                                                      const CORE::CDataNode& input  ,
                                                      const CString& representation ,
                                                      bool isDeltaUpdateOnly        )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_serializable )
        return CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;

    // Make sure we always have some degree of serializer options
    const CORE::CDataNodeSerializableSettings* serializerOptions = m_serializerOptions;
    if ( GUCEF_NULL == serializerOptions )
    {
        static const CORE::CDataNodeSerializableSettings defaultDummy;
        serializerOptions = &defaultDummy;
    }

    bool foundSupportedParams = false;
    CStdParams stdParams( *serializerOptions );
    CORE::CTSharedPtr< const CORE::CDataNodeSerializableSettings, MT::CNoLock > requestSpecificSerializerOptions;
    //if ( CStdParamParser::ParseStdParams( params, stdParams, foundSupportedParams ) && foundSupportedParams && stdParams.levelOfDetailWasProvided )
    //{
    //    // We place the clone in a shared ptr so that when we leave this scope the clone is properly disposed of
    //    requestSpecificSerializerOptions = CORE::CTSharedPtr< CORE::CDataNodeSerializableSettings, MT::CNoLock >( static_cast< CORE::CDataNodeSerializableSettings* >( serializerOptions->Clone() ) );
    //    const_cast< CORE::CDataNodeSerializableSettings* >( requestSpecificSerializerOptions.GetPointerAlways() )->levelOfDetail = stdParams.levelOfDetail;
    //    serializerOptions = requestSpecificSerializerOptions.GetPointerAlways();
    //}

    MT::CObjectScopeReadOnlyLock lock( m_serializableLock );

    if ( GUCEF_NULL != m_serializable )
        return m_serializable->Deserialize( input, *serializerOptions ) ? CIHTTPServerResource::DESERIALIZESTATE_SUCCEEDED : CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;
    return CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;    
}

/*-------------------------------------------------------------------------*/

CORE::CIDataNodeSerializable* 
CDataNodeSerializableHttpServerResource::GetSerializable( void )
{GUCEF_TRACE;
    
    return m_serializable;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
