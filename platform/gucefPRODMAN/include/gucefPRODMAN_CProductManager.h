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

#ifndef GUCEF_PRODMAN_CPRODUCTMANAGER_H
#define GUCEF_PRODMAN_CPRODUCTMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#include "gucefCORE_CAsyncUriResourceRetriever.h"
#define GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#endif /* GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_PRODMAN_CDOWNLOADSMANAGER_H
#include "gucefPRODMAN_CDownloadsManager.h"
#define GUCEF_PRODMAN_CDOWNLOADSMANAGER_H
#endif /* GUCEF_PRODMAN_CDOWNLOADSMANAGER_H ? */

#ifndef GUCEF_PRODMAN_CPRODUCTINFOLIST_H
#include "gucefPRODMAN_CProductInfoList.h"
#define GUCEF_PRODMAN_CPRODUCTINFOLIST_H
#endif /* GUCEF_PRODMAN_CPRODUCTINFOLIST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PRODMAN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_PRODMAN_PUBLIC_CPP CProductManager : public CORE::CObservingNotifier ,
                                                 public CORE::CGloballyConfigurable
{
    public:

    static const CORE::CEvent ProductInfoListRetrievalStartedEvent;
    static const CORE::CEvent ProductInfoListRetrievedEvent;
    static const CORE::CEvent ProductInfoListRetrievalErrorEvent;
    static const CORE::CEvent ProductInfoListRefreshStartedEvent;
    static const CORE::CEvent ProductInfoListRefreshedEvent;
    static const CORE::CEvent ProductInfoListRefreshFailedEvent;

    static void RegisterEvents( void );

    public:

    typedef CProductInfo::TDeploymentStatus  TDeploymentStatus;
    typedef CProductInfo::TProductType       TProductType;
    typedef std::set< CProductInfo >         TProductList;

    bool RetrieveProductInfo( const CString& combinedProductName ,
                              CProductInfo& productInfo          ) const;

    void RetrieveProductList( CProductInfoList& productList ) const;

    void RetrieveProductList( const TDeploymentStatus deploymentState ,
                              const TProductType productType          ,
                              CProductInfoList& productList           ) const;

    void RefreshProductInfoList( void );

    bool MergeProductList( const CProductInfoList& productList );

    bool MergeProduct( const CProductInfo& productInfo );

    virtual bool SaveConfig( GUCEF::CORE::CDataNode& node ) const;

    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& node );

    CString GetProductRoot( const CProductInfo& product ) const;

    void SetCommonProductRoot( const CString commonProductRoot );

    CString GetCommonProductRoot( void ) const;

    CDownloadsManager& GetDownloadsManager( void );

    virtual const CString& GetClassTypeName( void ) const;

    private:
    friend class CProdManGlobal;

    CProductManager( void );

    virtual ~CProductManager();

    private:
    
    CProductManager( const CProductManager& src );
    CProductManager& operator=( const CProductManager& src );

    void OnProductInfoListRetrievalEvent( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventid  ,
                                          CORE::CICloneable* eventdata );

    private:

    CProductInfoList m_productList;
    CString m_productListPath;
    CString m_productListCodec;
    CString m_productRoot;
    CDownloadsManager m_downloadsManager;
    CORE::CDynamicBuffer m_productListBuffer;               /**< destination of the product list retrieval, must outlive the retriever */
    CORE::CAsyncUriResourceRetriever m_urlDataRetriever;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PRODMAN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PRODMAN_CPRODUCTMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 11-02-2008 :
        - Initial implementation

---------------------------------------------------------------------------*/
