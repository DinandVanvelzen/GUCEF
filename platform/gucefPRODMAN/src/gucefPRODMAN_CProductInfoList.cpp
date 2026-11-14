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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefPRODMAN_CProductInfoList.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PRODMAN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_IMPLEMENT_MSGEXCEPTION( CProductInfoList, EInvalidDeploymentType );

/*-------------------------------------------------------------------------*/

CProductInfoList::CProductInfoList( void )
    : m_availableProducts()   ,
      m_installedProducts()   ,
      m_updatingProducts()    ,
      m_downloadingProducts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CProductInfoList::CProductInfoList( const CProductInfoList& src )
    : m_availableProducts( src.m_availableProducts )     ,
      m_installedProducts( src.m_installedProducts )     ,
      m_updatingProducts( src.m_updatingProducts )       ,
      m_downloadingProducts( src.m_downloadingProducts )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CProductInfoList::~CProductInfoList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CProductInfoList&
CProductInfoList::operator=( const CProductInfoList& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_availableProducts = src.m_availableProducts;
        m_installedProducts = src.m_installedProducts;
        m_updatingProducts = src.m_updatingProducts;
        m_downloadingProducts = src.m_downloadingProducts;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CProductInfoList::SaveConfig( GUCEF::CORE::CDataNode& node ) const
{GUCEF_TRACE;

    // @TODO @MAKEME
    return false;
}

/*-------------------------------------------------------------------------*/
                                
bool
CProductInfoList::LoadConfig( const GUCEF::CORE::CDataNode& node )
{GUCEF_TRACE;

    const GUCEF::CORE::CDataNode* n = node.Find( "ProductList" );
    if ( GUCEF_NULL != n )
    {
        GUCEF::CORE::CDataNode::const_iterator i = n->ConstBegin();
        while ( i != n->ConstEnd() )
        {
            const GUCEF::CORE::CDataNode* m = (*i);
            if ( m->GetName() == "ProductInfo" )
            {
                CProductInfo productInfo;
                if ( productInfo.LoadConfig( *m ) )
                {
                    MergeProduct( productInfo );
                }
            }
            ++i;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CProductInfoList::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PRODMAN::CProductInfoList";
    return classTypeName; 
}

/*-------------------------------------------------------------------------*/
    
CProductInfoList::TProductMap&
CProductInfoList::GetList( const TDeploymentStatus deploymentStatus )
{GUCEF_TRACE;

    switch ( deploymentStatus )
    {
        case CProductInfo::DEPLOYMENTSTATE_AVAILABLEFORDOWNLOAD :
        {
            return m_availableProducts;
        }
        case CProductInfo::DEPLOYMENTSTATE_INSTALLED :
        {
            return m_installedProducts;
        }
        case CProductInfo::DEPLOYMENTSTATE_UPDATING :
        {
            return m_updatingProducts;
        }
        case CProductInfo::DEPLOYMENTSTATE_DOWNLOADING :
        {
            return m_downloadingProducts;
        }                        
    }
    
    GUCEF_EMSGTHROW( EInvalidDeploymentType, "The given deployment type is invalid for this operation" );
}

/*-------------------------------------------------------------------------*/

CProductInfo*
CProductInfoList::RetrieveProductInfo( const CString& combinedProductString )
{GUCEF_TRACE;

    TProductMap::iterator i = m_installedProducts.find( combinedProductString );
    if ( i == m_installedProducts.end() )
    {
        i = m_updatingProducts.find( combinedProductString );
        if ( i == m_updatingProducts.end() )
        {
            i = m_availableProducts.find( combinedProductString );
            if ( i == m_availableProducts.end() )
            {
                i = m_downloadingProducts.find( combinedProductString );
                if ( i == m_downloadingProducts.end() )
                {
                    return GUCEF_NULL;
                }            
            }            
        }
    }
    
    return &(*i).second;
}

/*-------------------------------------------------------------------------*/

const CProductInfo*
CProductInfoList::RetrieveProductInfo( const CString& combinedProductString ) const
{GUCEF_TRACE;

    TProductMap::const_iterator i = m_installedProducts.find( combinedProductString );
    if ( i == m_installedProducts.end() )
    {
        i = m_updatingProducts.find( combinedProductString );
        if ( i == m_updatingProducts.end() )
        {
            i = m_availableProducts.find( combinedProductString );
            if ( i == m_availableProducts.end() )
            {
                i = m_downloadingProducts.find( combinedProductString );
                if ( i == m_downloadingProducts.end() )
                {
                    return GUCEF_NULL;
                }            
            }            
        }
    }
    
    return &(*i).second;
}

/*-------------------------------------------------------------------------*/

void
CProductInfoList::RetrieveListSubSet( const TDeploymentStatus deploymentStatus ,
                                      const TProductType productType           ,
                                      CProductInfoList& productMap             ) const
{GUCEF_TRACE;

    if ( ( CProductInfo::DEPLOYMENTSTATE_UNKNOWN != deploymentStatus ) &&
         ( CProductInfo::PRODUCTTYPE_UNKNOWN != productType )           )
    {
        const TProductMap& fullProductMap = GetList( deploymentStatus );
        TProductMap::const_iterator i = fullProductMap.begin();
        while ( i != fullProductMap.end() )
        {
            const CProductInfo& productInfo = (*i).second;
            if ( productType == productInfo.GetProductType() )
            {
                productMap.MergeProduct( (*i).second );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CProductInfoList::RetrieveListSubSet( const TProductType productType           ,
                                      CProductInfoList& productList            ) const
{GUCEF_TRACE;

    RetrieveListSubSet( CProductInfo::DEPLOYMENTSTATE_AVAILABLEFORDOWNLOAD ,
                        productType                                        ,
                        productList                                        );
    RetrieveListSubSet( CProductInfo::DEPLOYMENTSTATE_DOWNLOADING          ,
                        productType                                        ,
                        productList                                        );                        
    RetrieveListSubSet( CProductInfo::DEPLOYMENTSTATE_INSTALLED            ,
                        productType                                        ,
                        productList                                        );
    RetrieveListSubSet( CProductInfo::DEPLOYMENTSTATE_UPDATING             ,
                        productType                                        ,
                        productList                                        );
}

/*-------------------------------------------------------------------------*/
    
const CProductInfoList::TProductMap&
CProductInfoList::GetList( const TDeploymentStatus deploymentStatus ) const
{GUCEF_TRACE;

    switch ( deploymentStatus )
    {
        case CProductInfo::DEPLOYMENTSTATE_AVAILABLEFORDOWNLOAD :
        {
            return m_availableProducts;
        }
        case CProductInfo::DEPLOYMENTSTATE_INSTALLED :
        {
            return m_installedProducts;
        }
        case CProductInfo::DEPLOYMENTSTATE_UPDATING :
        {
            return m_updatingProducts;
        }
        case CProductInfo::DEPLOYMENTSTATE_DOWNLOADING :
        {
            return m_downloadingProducts;
        }                        
    }
    
    GUCEF_EMSGTHROW( EInvalidDeploymentType, "The given deployment type is invalid for this operation" );
}

/*-------------------------------------------------------------------------*/

bool
CProductInfoList::MergeProduct( const CProductInfo& productInfo )
{GUCEF_TRACE;

    CProductInfoList::TProductMap& productMap = GetList( productInfo.GetDeploymentState() );    
    productMap[ productInfo.GetCombinedProductString() ] = productInfo;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CProductInfoList::MergeProductList( const CProductInfoList& productInfoList )
{GUCEF_TRACE;

    const TProductMap& aProductMap = productInfoList.GetList( CProductInfo::DEPLOYMENTSTATE_AVAILABLEFORDOWNLOAD );
    TProductMap::const_iterator i = aProductMap.begin();
    while ( i != aProductMap.end() )
    {
        MergeProduct( (*i).second );
        ++i;
    }
    const TProductMap& iProductMap = productInfoList.GetList( CProductInfo::DEPLOYMENTSTATE_INSTALLED );
    i = iProductMap.begin();
    while ( i != iProductMap.end() )
    {
        MergeProduct( (*i).second );
        ++i;
    }
    const TProductMap& uProductMap = productInfoList.GetList( CProductInfo::DEPLOYMENTSTATE_UPDATING );
    i = uProductMap.begin();
    while ( i != uProductMap.end() )
    {
        MergeProduct( (*i).second );
        ++i;
    }
    const TProductMap& dProductMap = productInfoList.GetList( CProductInfo::DEPLOYMENTSTATE_DOWNLOADING );
    i = dProductMap.begin();
    while ( i != dProductMap.end() )
    {
        MergeProduct( (*i).second );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PRODMAN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
