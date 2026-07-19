/*
 *  gucefPRODMAN: Product management module
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_PRODMAN_CPRODUCTINFOLIST_H
#define GUCEF_PRODMAN_CPRODUCTINFOLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#include "ExceptionClassMacros.h"
#define GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONCLASSMACROS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_PRODMAN_CPRODUCTINFO_H
#include "gucefPRODMAN_CProductInfo.h"
#define GUCEF_PRODMAN_CPRODUCTINFO_H
#endif /* GUCEF_PRODMAN_CPRODUCTINFO_H ? */

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

class GUCEF_PRODMAN_PUBLIC_CPP CProductInfoList : public CORE::CIConfigurable
{
    public:
    
    typedef std::map< CString, CProductInfo > TProductMap;
    typedef CProductInfo::TDeploymentStatus   TDeploymentStatus;
    typedef CProductInfo::TProductType        TProductType;  

    CProductInfoList( void );
    
    CProductInfoList( const CProductInfoList& src );
    
    virtual ~CProductInfoList();

    CProductInfoList& operator=( const CProductInfoList& src );

    virtual bool SaveConfig( CORE::CDataNode& node ) const;
                                
    virtual bool LoadConfig( const CORE::CDataNode& node );
    
    TProductMap& GetList( const TDeploymentStatus deploymentStatus );
    
    const TProductMap& GetList( const TDeploymentStatus deploymentStatus ) const;

    void RetrieveListSubSet( const TProductType productType           ,
                             CProductInfoList& productList            ) const;
    
    void RetrieveListSubSet( const TDeploymentStatus deploymentStatus ,
                             const TProductType productType           ,
                             CProductInfoList& productList            ) const;
                             
    bool MergeProduct( const CProductInfo& productInfo );
    
    bool MergeProductList( const CProductInfoList& productInfoList );
    
    CProductInfo* RetrieveProductInfo( const CString& combinedProductString );
    
    const CProductInfo* RetrieveProductInfo( const CString& combinedProductString ) const;

    virtual const CString& GetClassTypeName( void ) const;
    
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_PRODMAN_PUBLIC_CPP, EInvalidDeploymentType );
    
    private:
    
    TProductMap m_availableProducts;
    TProductMap m_installedProducts;
    TProductMap m_updatingProducts;
    TProductMap m_downloadingProducts;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CProductInfoList, MT::CMutex >   CProductInfoListPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PRODMAN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PRODMAN_CPRODUCTINFOLIST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 11-02-2008 :
        - Initial implementation
          
---------------------------------------------------------------------------*/
