/*
 *  gucefCOM: GUCEF module providing communication 
 *  implementations for standardized protocols.
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
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#include "TestCode_CHTTPClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/ 

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

class CMyHTTPClient : public GUCEF::CORE::CObserver
{        
        public:
        
        CMyHTTPClient( void )
        {
            SubscribeTo( &m_client );
        }
        
        protected:
        
        virtual void OnConnected( WEB::CHTTPClient& client )
        {
                printf( "CMyHTTPClient::OnConnected()\n" );
        }
        
        virtual void OnDisconnected( WEB::CHTTPClient& client )
        {
                printf( "CMyHTTPClient::OnDisconnected()\n" );
        }
        
        virtual void OnHTTPError( WEB::CHTTPClient& client ,
                                  WEB::THTTPCODE errorcode )
        {
                printf( "CMyHTTPClient::OnHTTPError()\n" );
        }                                  
        
        virtual void OnError( WEB::CHTTPClient& client )
        {
                printf( "CMyHTTPClient::OnError()\n" );
        }
        
        virtual void OnRedirect( WEB::CHTTPClient& client             ,
                                 const GUCEF::CORE::CString& location )
        {
                printf( "CMyHTTPClient::OnRedirect()\n" );
                printf( "CMyHTTPClient: location: %s\n", location.C_String() );
        }
        
        virtual void OnDataRecieved( WEB::CHTTPClient& client ,
                                     const char *data         ,
                                     CORE::UInt32 length      )
        {
                printf( "CMyHTTPClient::OnDataRead()\n" );
                printf( "CMyHTTPClient: recieved %d bytes\n", length );
        }
        
        virtual void OnDataSend( WEB::CHTTPClient& client ,
                                 const char *data         ,
                                 CORE::UInt32 length      )
        {
                printf( "CMyHTTPClient::OnDataSend()\n" );
                printf( "CMyHTTPClient: sending %d bytes\n", length );
        }
        
        virtual void OnContent( WEB::CHTTPClient& client      ,
                                const CORE::UInt32 size       ,
                                const bool resumeable         ,
                                const WEB::THTTPCODE httpcode )
        {
                printf( "CMyHTTPClient::OnContent()\n" );
                printf( "CMyHTTPClient: size: %d\n", size );        
                printf( "CMyHTTPClient: resumeable: %d\n", resumeable );
                printf( "CMyHTTPClient: httpcode: %d\n", httpcode );
        }                                   
                                 
        virtual void OnFinish( WEB::CHTTPClient &client )
        {
                printf( "CMyHTTPClient::OnFinish()\n" );
        }                                              

        virtual void OnNotify( CORE::CNotifier* notifier           ,
                               const CORE::CEvent& eventid         ,
                               CORE::CICloneable* eventdata = GUCEF_NULL )
        {
            
        }

        public:
        WEB::CHTTPClient m_client;
};

/*-------------------------------------------------------------------------*/

class CMyPumpClient : public CORE::CObserver
{
    public:
    
    CMyPumpClient( void )
    {
            SubscribeTo( &GUCEF::CORE::CCoreGlobal::Instance()->GetApplication(), GUCEF::CORE::CGUCEFApplication::AppInitEvent );       
    }
    
    protected:
    
    virtual void OnUpdate( const CORE::UInt64 tickcount               ,
                           const CORE::Float64 updateDeltaInMilliSecs )
    {
    }

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ )
    {
        if ( eventid == GUCEF::CORE::CGUCEFApplication::AppInitEvent )
        {
                if ( client.m_client.Get( "www.google.com" ,
                                          80               ,
                                          ""               ) )
                {
                        printf( "Get succeeded\n" );
                }
                else                        
                {
                        printf( "Get failed\n" );
                }                                                  
        }                                                 
    }
        
        private:
        CMyHTTPClient client;   
                                              
};

/*-------------------------------------------------------------------------*/

void
SetupHTTPClientTest( void )
{
        CMyPumpClient* pclient = new CMyPumpClient();
}

/*-------------------------------------------------------------------------*/
