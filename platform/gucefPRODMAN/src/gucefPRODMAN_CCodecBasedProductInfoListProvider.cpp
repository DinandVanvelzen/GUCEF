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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_PRODMAN_CPRODUCTINFOLIST_H
#include "gucefPRODMAN_CProductInfoList.h"
#define GUCEF_PRODMAN_CPRODUCTINFOLIST_H
#endif /* GUCEF_PRODMAN_CPRODUCTINFOLIST_H ? */

#ifndef GUCEF_PRODMAN_CPRODUCTINFOLISTRETRIEVEDEVENTDATA_H
#include "gucefPRODMAN_CProductInfoListRetrievedEventData.h"
#define GUCEF_PRODMAN_CPRODUCTINFOLISTRETRIEVEDEVENTDATA_H
#endif /* GUCEF_PRODMAN_CPRODUCTINFOLISTRETRIEVEDEVENTDATA_H ? */

#include "gucefPRODMAN_CCodecBasedProductInfoListProvider.h"

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
                  
CCodecBasedProductInfoListProvider::CCodecBasedProductInfoListProvider()
    : CProductInfoListProvider(),
      m_codecToUse( "xml" ),
      m_listUrl(),
      m_listBuffer( true ),
      m_dataRetriever()
{GUCEF_TRACE;

    SubscribeTo( &m_dataRetriever );
}

/*-------------------------------------------------------------------------*/

CCodecBasedProductInfoListProvider::~CCodecBasedProductInfoListProvider()
{GUCEF_TRACE;

    // The retriever stops a retrieval that is still busy when it is destroyed
}

/*-------------------------------------------------------------------------*/

void
CCodecBasedProductInfoListProvider::OnAllDataRetrieved( void )
{
    CORE::CDStoreCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( codecRegistry.TryLookup( m_codecToUse ,
                                  codec        ,
                                  false        ) )
    {
        // we have our codec, parse the data into data nodes
        CORE::CDataNode rootNode;
        CORE::CDynamicBufferAccess listAccess( m_listBuffer );
        if ( codec->BuildDataTree( &rootNode, &listAccess ) )
        {
            // Now try to load the product info list data
            CProductInfoListPtr productInfoList( new CProductInfoList() );
            if ( productInfoList->LoadConfig( rootNode ) )
            {
                rootNode.Clear();
                
                CProductInfoListRetrievedEventData eventData( m_listUrl, productInfoList );
                NotifyObservers( ProductInfoListRetrievedEvent, &eventData );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CCodecBasedProductInfoListProvider::RetrieveList( const CORE::CString& listLocation )
{GUCEF_TRACE;
   
    if ( !m_dataRetriever.IsBusy() )
    {
        // Start with a fresh buffer as storage
        m_listBuffer.Clear();

        m_listUrl = listLocation;
        if ( m_dataRetriever.Retrieve( CORE::CUri( m_listUrl ), m_listBuffer ).HasAFuture() )
        {
            NotifyObservers( ProductInfoListRetrievalStartedEvent );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CCodecBasedProductInfoListProvider::OnNotify( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventid  ,
                                              CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent == eventid )
    {
        OnAllDataRetrieved();
    }
    else
    if ( CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent == eventid )
    {
        NotifyObservers( ProductInfoListRetrievalErrorEvent );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CCodecBasedProductInfoListProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PRODMAN::CCodecBasedProductInfoListProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PRODMAN */
}; /* namespace GUCEF*/

/*-------------------------------------------------------------------------*/
