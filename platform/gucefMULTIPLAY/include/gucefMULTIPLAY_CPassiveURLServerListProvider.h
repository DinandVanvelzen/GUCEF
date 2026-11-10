/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H
#define GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#include "gucefCORE_CAsyncUriResourceRetriever.h"
#define GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#endif /* GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAY_CSERVERLISTPROVIDER_H
#include "gucefMULTIPLAY_CServerListProvider.h"
#define GUCEF_MULTIPLAY_CSERVERLISTPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CSERVERLISTPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Implementation of a server list provider that obtains a list using a URL
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CPassiveURLServerListProvider : public CServerListProvider
{
    public:
    
    CPassiveURLServerListProvider( void );
    
    virtual ~CPassiveURLServerListProvider();
    
    virtual bool Start( const CORE::CValueList& params );
    
    virtual bool IsBusy( void );
    
    virtual void Stop( void );
    
    bool SetListCodecType( const CORE::CString& codecType );
    
    const CORE::CString& GetListCodecType( void ) const; 

    bool SetURL( const CORE::CString& url );
    
    const CORE::CString& GetURL( void ) const;

    protected:
    
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL );    
    
    private:
    
    CPassiveURLServerListProvider( const CPassiveURLServerListProvider& src );  /**< not implemented */
    CPassiveURLServerListProvider& operator=( const CPassiveURLServerListProvider& src ); /**< not implemented */
    
    void ProcessRetrievedList( void );
    
    void ParseList( const CORE::CDataNode& listRoot );
    
    private:
    
    CORE::CDynamicBuffer m_buffer;                      /**< destination of the retrieval, must outlive the retriever */
    CORE::CString m_codecType;
    CORE::CString m_url;
    CORE::CAsyncUriResourceRetriever m_retriever;
    bool m_isBusy;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 07-06-2007 :
       - Initial implementation.

---------------------------------------------------------------------------*/
