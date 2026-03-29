/*
 *  gucefPRODMAN: Product management module
 *  Copyright (C) 2002 - 2012.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_PRODMAN_CPRODUCTMANAGER_H
#include "gucefPRODMAN_CProductManager.h"
#define GUCEF_PRODMAN_CPRODUCTMANAGER_H
#endif /* GUCEF_PRODMAN_CPRODUCTMANAGER_H ? */

#ifndef GUCEF_PRODMAN_CPRODUCTINFOLISTPROVIDERREGISTRY_H
#include "gucefPRODMAN_CProductInfoListProviderRegistry.h"
#define GUCEF_PRODMAN_CPRODUCTINFOLISTPROVIDERREGISTRY_H
#endif /* GUCEF_PRODMAN_CPRODUCTINFOLISTPROVIDERREGISTRY_H ? */

#ifndef GUCEF_PRODMAN_CCODECBASEDPRODUCTINFOLISTPROVIDER_H
#include "gucefPRODMAN_CCodecBasedProductInfoListProvider.h"
#define GUCEF_PRODMAN_CCODECBASEDPRODUCTINFOLISTPROVIDER_H
#endif /* GUCEF_PRODMAN_CCODECBASEDPRODUCTINFOLISTPROVIDER_H ? */

#include "gucefPRODMAN_CProdManGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PRODMAN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CProdManGlobal* CProdManGlobal::g_instance = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CProdManGlobal*
CProdManGlobal::Instance()
{GUCEF_TRACE;

    if ( NULL == g_instance )
    {
        g_instance = new CProdManGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CProdManGlobal::Deinstance( void )
{GUCEF_TRACE;

    delete g_instance;
    g_instance = NULL;
}

/*-------------------------------------------------------------------------*/

void
CProdManGlobal::Initialize( void )
{GUCEF_TRACE;

    // Ensure the systems we depend on are initialzed first
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Initializing gucefPRODMAN Global systems" );

    m_productInfoListProviderRegistry = new CProductInfoListProviderRegistry();
    m_productManager = new CProductManager();

    CProductManager::RegisterEvents();
    CProductInfoListProvider::RegisterEvents();

    m_productInfoListProviderRegistry->RegisterProvider( "CodecBased", CProductInfoListProviderPtr( new CCodecBasedProductInfoListProvider() ) );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefPRODMAN Global systems initialized" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefPRODMAN" );
}

/*-------------------------------------------------------------------------*/

CProdManGlobal::CProdManGlobal( void )
    : m_productManager( NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProdManGlobal::~CProdManGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefPRODMAN global systems" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefPRODMAN" );

    delete m_productManager;
    m_productManager = NULL;
    delete m_productInfoListProviderRegistry;
    m_productInfoListProviderRegistry = NULL;
}

/*-------------------------------------------------------------------------*/

CProductManager&
CProdManGlobal::GetProductManager( void )
{GUCEF_TRACE;

    return *m_productManager;
}

/*-------------------------------------------------------------------------*/

CProductInfoListProviderRegistry&
CProdManGlobal::GetProductInfoListProviderRegistry( void )
{GUCEF_TRACE;

    return *m_productInfoListProviderRegistry;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PRODMAN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
