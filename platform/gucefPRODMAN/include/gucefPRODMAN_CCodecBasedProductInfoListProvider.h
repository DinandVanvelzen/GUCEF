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

#ifndef GUCEF_PRODMAN_CCODECBASEDPRODUCTINFOLISTPROVIDER_H
#define GUCEF_PRODMAN_CCODECBASEDPRODUCTINFOLISTPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CURLDATARETRIEVER_H
#include "gucefCORE_CAsyncUriResourceRetriever.h"
#define GUCEF_CORE_CURLDATARETRIEVER_H
#endif /* GUCEF_CORE_CURLDATARETRIEVER_H ? */

#include "gucefPRODMAN_CProductInfoListProvider.h"

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
                  
class GUCEF_PRODMAN_PUBLIC_CPP CCodecBasedProductInfoListProvider : public CProductInfoListProvider
{
    public:

    CCodecBasedProductInfoListProvider( void );

    virtual ~CCodecBasedProductInfoListProvider();

    virtual bool RetrieveList( const CORE::CString& listLocation );

    virtual const CORE::CString& GetClassTypeName( void ) const;

    protected:

    /**
     *  Event callback member function.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL );

    private:

    CCodecBasedProductInfoListProvider( const CCodecBasedProductInfoListProvider& src );
    CCodecBasedProductInfoListProvider& operator=( const CCodecBasedProductInfoListProvider& other );

    void OnAllDataRetrieved( void );

    private:

    CORE::CString m_codecToUse;
    CORE::CString m_listUrl;
    CORE::CDynamicBuffer m_listBuffer;                      /**< destination of the list retrieval, must outlive the retriever */
    CORE::CAsyncUriResourceRetriever m_dataRetriever;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PRODMAN */
}; /* namespace GUCEF*/

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PRODMAN_CIPRODUCTINFOLISTPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 11-02-2008 :
        - Initial implementation
          
---------------------------------------------------------------------------*/

