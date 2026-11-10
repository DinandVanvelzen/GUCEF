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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H
#include "gucefCORE_CDataDrivenDStoreCodecFactory.h"         
#define GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H ? */

//#ifndef GUCEF_VFS_CVFSGLOBAL_H
//#include "gucefVFS_CVfsGlobal.h"
//#define GUCEF_VFS_CVFSGLOBAL_H
//#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_KAITAI_CGLOBALKAITAISCHEMAREGISTRY_H
#include "gucefKAITAI_CGlobalKaitaiSchemaRegistry.h"    
#define GUCEF_KAITAI_CGLOBALKAITAISCHEMAREGISTRY_H
#endif /* GUCEF_KAITAI_CGLOBALKAITAISCHEMAREGISTRY_H ? */

#ifndef GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H
#include "gucefKAITAI_CKaitaiDataDrivenDStoreCodecFactory.h"
#define GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H
#endif /* GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#include "gucefKAITAI_CKaitaiGlobal.h"  /* definition of the class implemented here */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CKaitaiGlobal::g_dataLock;
CKaitaiGlobal* CKaitaiGlobal::g_instance = GUCEF_NULL;

CKaitaiDataDrivenDStoreCodecFactory g_kaitaiDStoreCodecFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiGlobal::CKaitaiGlobal( void )
    : m_kaitaiSchemaRegistry( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CKaitaiGlobal::Initialize( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance();
    //VFS::CVfsGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefKAITAI Global systems initializing" );

    m_kaitaiSchemaRegistry = GUCEF_NEW CGlobalKaitaiSchemaRegistry();

    CORE::CCoreGlobal::Instance()->GetDataDrivenDStoreCodecFactory().RegisterConcreteFactory( CKaitaiDStoreCodec::CodecTypeName, &g_kaitaiDStoreCodecFactory );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefKAITAI" );
}

/*-------------------------------------------------------------------------*/

CKaitaiGlobal::~CKaitaiGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefKATAI Global systems shutting down" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefKAITAI" );

    CORE::CCoreGlobal::Instance()->GetDataDrivenDStoreCodecFactory().UnregisterConcreteFactory( CKaitaiDStoreCodec::CodecTypeName );
    
    GUCEF_DELETE static_cast< CGlobalKaitaiSchemaRegistry* >( m_kaitaiSchemaRegistry );
    m_kaitaiSchemaRegistry = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CKaitaiGlobal*
CKaitaiGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CKaitaiGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CKaitaiGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CKaitaiGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = NULL;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaRegistry& 
CKaitaiGlobal::GetKaitaiSchemaRegistry( void )
{GUCEF_TRACE;

    return *m_kaitaiSchemaRegistry;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
