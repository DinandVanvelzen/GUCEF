/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"          /* data storage node class,.. this is what it's all about */
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSOR_H
#include "gucefCORE_CUriResourceAccessor.h"
#define GUCEF_CORE_CURIRESOURCEACCESSOR_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMA_H
#include "gucefKAITAI_CKaitaiSchema.h"
#define GUCEF_KAITAI_CKAITAISCHEMA_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMA_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#include "gucefKAITAI_CKaitaiSchemaRegistry.h"
#define GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H ? */

#ifndef GUCEF_KAITAI_CKAITAIGLOBAL_H
#include "gucefKAITAI_CKaitaiGlobal.h"
#define GUCEF_KAITAI_CKAITAIGLOBAL_H
#endif /* GUCEF_KAITAI_CKAITAIGLOBAL_H ? */

#include "gucefKAITAI_CKaitaiDataDrivenDStoreCodec.h"  /* definition of the class implemented here */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiDataDrivenDStoreCodec::CKaitaiDataDrivenDStoreCodec( void )
    : CORE::CDataDrivenDStoreCodec()    
    , _id( 0 )          
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiDataDrivenDStoreCodec::CKaitaiDataDrivenDStoreCodec( CORE::CDataDrivenDStoreCodecMetaPtr codecMeta )
    : CORE::CDataDrivenDStoreCodec( codecMeta )    
    , _id( 0 )
{GUCEF_TRACE;

    InitializeResources( m_codecMeta );
}

/*-------------------------------------------------------------------------*/

CKaitaiDataDrivenDStoreCodec::~CKaitaiDataDrivenDStoreCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiDataDrivenDStoreCodec::InitializeResources( CORE::CDataDrivenDStoreCodecMetaPtr codecMeta )
{GUCEF_TRACE;
    
    if GUCEF_PREDICT_FALSE( codecMeta.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiDataDrivenDStoreCodec:InitializeResources: Failed to initialize codec resources because the codec meta data is NULL" );
        return false;
    }
    m_codecMeta = codecMeta;

    CKaitaiSchemaRegistry& schemaRegistry = CKaitaiGlobal::Instance()->GetKaitaiSchemaRegistry();
    
    bool totalSuccess = true;
    
    const CORE::CStringMap& resources = m_codecMeta->GetDataMap();
    CORE::CStringMap::const_iterator i = resources.begin();
    while ( i != resources.end() )
    {
        const CORE::CString& resourceName = i->first;
        const CORE::CString& resourceValue = i->second;

        //if ( resourceName.EndsWith( CKaitaiSchema::SchemaFileExtension ) )
        {
            CORE::CUri resourceUri( resourceValue );
            if GUCEF_PREDICT_FALSE( !schemaRegistry.LoadSchema( resourceUri, m_codecMeta->GetDataDrivenCodecTypeName() ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiDataDrivenDStoreCodec:InitializeResources: Failed to load schema: " + resourceName );
                totalSuccess = false;
            }
        }
        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiDataDrivenDStoreCodec::StoreDataTree( const CORE::CDataNode* tree   ,
                                             CORE::CIOAccess* file         )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiDataDrivenDStoreCodec::BuildDataTree( CORE::CDataNode* treeroot ,
                                             CORE::CIOAccess* file     )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKaitaiDataDrivenDStoreCodec::GetName( void ) const
{GUCEF_TRACE;

    return "gucefKAITAI"; 
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKaitaiDataDrivenDStoreCodec::GetCopyright( void ) const
{GUCEF_TRACE;

    return "Licensed under the Apache License, Version 2.0 (the \"License\"). Copyright (C) 1998 - 2025.  Dinand Vanvelzen";
}

/*-------------------------------------------------------------------------*/

CORE::TVersion
CKaitaiDataDrivenDStoreCodec::GetVersion( void ) const
{GUCEF_TRACE;

    CORE::TVersion version = { GUCEF_KAITAI_MAJOR_VERSION, GUCEF_KAITAI_MINOR_VERSION, GUCEF_KAITAI_PATCH_VERSION, GUCEF_KAITAI_RELEASE_VERSION }; // major, minor, patch, release
    return version; 
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CKaitaiDataDrivenDStoreCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiDataDrivenDStoreCodec();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
