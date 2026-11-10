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
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#include "gucefPRODMAN_CProductManager.h"

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

const GUCEF::CORE::CEvent CProductManager::ProductInfoListRetrievalStartedEvent = "GUCEF::PRODMAN::CProductManager::ProductInfoListRetrievalStartedEvent";
const GUCEF::CORE::CEvent CProductManager::ProductInfoListRetrievedEvent = "GUCEF::PRODMAN::CProductManager::ProductInfoListRetrievedEvent";
const GUCEF::CORE::CEvent CProductManager::ProductInfoListRetrievalErrorEvent = "GUCEF::PRODMAN::CProductManager::ProductInfoListRetrievalErrorEvent";
const GUCEF::CORE::CEvent CProductManager::ProductInfoListRefreshStartedEvent = "GUCEF::PRODMAN::CProductManager::ProductInfoListRefreshStartedEvent";
const GUCEF::CORE::CEvent CProductManager::ProductInfoListRefreshedEvent = "GUCEF::PRODMAN::CProductManager::ProductInfoListRefreshedEvent";
const GUCEF::CORE::CEvent CProductManager::ProductInfoListRefreshFailedEvent = "GUCEF::PRODMAN::CProductManager::ProductInfoListRefreshFailedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CProductManager::RegisterEvents( void )
{GUCEF_TRACE;

    ProductInfoListRetrievalStartedEvent.Initialize();
    ProductInfoListRetrievedEvent.Initialize();
    ProductInfoListRetrievalErrorEvent.Initialize();
    ProductInfoListRefreshStartedEvent.Initialize();
    ProductInfoListRefreshedEvent.Initialize();
    ProductInfoListRefreshFailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CProductManager::CProductManager( void )
    : CObservingNotifier()        ,
      CGloballyConfigurable()     ,
      m_productList()             ,
      m_productListPath()         ,
      m_productListCodec( "xml" ) ,
      m_productRoot()             ,
      m_downloadsManager()        ,
      m_productListBuffer( true ) ,
      m_urlDataRetriever()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProductManager::~CProductManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDownloadsManager&
CProductManager::GetDownloadsManager( void )
{GUCEF_TRACE;

    return m_downloadsManager;
}

/*-------------------------------------------------------------------------*/

void
CProductManager::RetrieveProductList( CProductInfoList& productList ) const
{GUCEF_TRACE;

    RetrieveProductList( CProductInfo::DEPLOYMENTSTATE_UNKNOWN ,
                         CProductInfo::PRODUCTTYPE_UNKNOWN     ,
                         productList                           );
}

/*-------------------------------------------------------------------------*/

void
CProductManager::RetrieveProductList( const TDeploymentStatus deploymentState ,
                                      const TProductType productType          ,
                                      CProductInfoList& productList           ) const
{GUCEF_TRACE;

    m_productList.RetrieveListSubSet( deploymentState ,
                                      productType     ,
                                      productList     );
}

/*-------------------------------------------------------------------------*/

bool
CProductManager::RetrieveProductInfo( const CString& combinedProductName ,
                                      CProductInfo& productInfo          ) const

{GUCEF_TRACE;

    const CProductInfo* foundProductInfo = m_productList.RetrieveProductInfo( combinedProductName );
    if ( NULL != foundProductInfo )
    {
        productInfo = *foundProductInfo;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CProductManager::MergeProduct( const CProductInfo& productInfo )
{GUCEF_TRACE;

    return m_productList.MergeProduct( productInfo );
}

/*-------------------------------------------------------------------------*/

bool
CProductManager::MergeProductList( const CProductInfoList& productList )
{GUCEF_TRACE;

    return m_productList.MergeProductList( productList );
}

/*-------------------------------------------------------------------------*/

bool
CProductManager::SaveConfig( GUCEF::CORE::CDataNode& node ) const
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( 0, "Saving the product manager configuration" );

    GUCEF::CORE::CDataNode* n = node.Structure( "GU%CORE%CProductManager", '%' );
    n->SetAttribute( "ProductListPath", m_productListPath );
    n->SetAttribute( "ProductListCodec", m_productListCodec );
    n->SetAttribute( "CommonProductRoot", m_productRoot );
    m_downloadsManager.SaveConfig( node );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProductManager::LoadConfig( const GUCEF::CORE::CDataNode& node )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "CProductManager: Loading the configuration" );

    GUCEF::CORE::CDataNode* n = node.Search( "GUCEF%PRODMAN%CProductManager", '%', false );
    if ( NULL != n )
    {
        GUCEF::CORE::CDataNode::TKeyValuePair* att = NULL;
        att = n->GetAttribute( "ProductListPath" );
        if ( NULL != att )
        {
            m_productListPath = att->second;
        }
        att = n->GetAttribute( "ProductListCodec" );
        if ( NULL != att )
        {
            m_productListCodec = att->second;
        }

        att = n->GetAttribute( "CommonProductRoot" );
        if ( NULL != att )
        {
            m_productRoot = att->second;
        }

        //GUCEF::CORE::CDataNode productList;
        //if ( GUCE::CORE::VFSLoadDataTree( m_productListPath  ,
        //                                  m_productListCodec ,
        //                                  &productList       ) )
        //{
        //    m_productList.LoadConfig( productList );
        //}

        m_downloadsManager.LoadConfig( node );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CProductManager::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PRODMAN::CProductManager";
    return classTypeName; 
}

/*-------------------------------------------------------------------------*/

void
CProductManager::RefreshProductInfoList( void )
{
    NotifyObservers( ProductInfoListRefreshStartedEvent );

    // Load the already installed product list

    // Download the available products which are to be merged into our existing list
    NotifyObservers( ProductInfoListRetrievalStartedEvent );
}


/*-------------------------------------------------------------------------*/

void
CProductManager::SetCommonProductRoot( const CString commonProductRoot )
{GUCEF_TRACE;

    m_productRoot = commonProductRoot;
}

/*-------------------------------------------------------------------------*/

CString
CProductManager::GetCommonProductRoot( void ) const
{GUCEF_TRACE;

    return m_productRoot;
}

/*-------------------------------------------------------------------------*/

CString
CProductManager::GetProductRoot( const CProductInfo& product ) const
{GUCEF_TRACE;

    CString path;
    if ( m_productRoot.Length() > 0 )
    {
         path = m_productRoot + '/';
    }
    path += CString( "Products" ) + '/' + product.GetCombinedProductString();
    return path;
}

/*-------------------------------------------------------------------------*/

void
CProductManager::OnProductInfoListRetrievalEvent( CORE::CNotifier* notifier    ,
                                                  const CORE::CEvent& eventid  ,
                                                  CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent == eventid )
    {
        CORE::CDynamicBufferAccess bufferAccess( m_productListBuffer );
        CORE::CIOAccess* access = &bufferAccess;

        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
        CORE::CDStoreCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
        if ( codecRegistry.TryLookup( m_productListCodec, codec, false ) )
        {
            // we found a codec of the given type, use it to deserialize the data received
            CORE::CDataNode dataRootNode;
            if ( codec->BuildDataTree( &dataRootNode, access ) )
            {
                CORE::CDataNode::TDataNodeSet productLists = dataRootNode.FindChildrenOfType( "ProductList", true );
                CORE::CDataNode::TDataNodeSet::iterator i = productLists.begin();
                while ( i != productLists.end() )
                {
                    m_productList.LoadConfig( *(*i) );
                    ++i;
                }
            }
        }
    }
    else
    if ( GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent == eventid )
    {
        NotifyObservers( ProductInfoListRetrievalErrorEvent );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PRODMAN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
