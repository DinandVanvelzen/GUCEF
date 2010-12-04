/*
 *  GucefLogServiceLib: Library containing the main logic for the Logging service
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

#ifndef GUCEF_LOGSERVICELIB_CLOGSVCSERVER_H
#define GUCEF_LOGSERVICELIB_CLOGSVCSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_LOGSERVICELIB_MACROS_H
#include "GucefLogServiceLib_macros.h"
#define GUCEF_LOGSERVICELIB_MACROS_H
#endif /* GUCEF_LOGSERVICELIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace LOGSERVICELIB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_LOGSERVICELIB_EXPORT_CPP CLogSvcServer : public CORE::CObservingNotifier
{
    public:
    
    static const CORE::CEvent ConnectingEvent;
    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent DisconnectedEvent;
    static const CORE::CEvent SocketErrorEvent;
    
    static void RegisterEvents( void );
    
    public:
    
    struct SClientInfo
    {
        CORE::CString appName;
        CORE::CString processName;
        CORE::CString processId;
        CORE::CString logClientVersion;
        bool initialized;
        bool connected;
    };
    typedef struct SClientInfo TClientInfo;
        
    CLogSvcServer( void );
    
    CLogSvcServer( CORE::CPulseGenerator& pulseGenerator ); 
    
    virtual ~CLogSvcServer();
    
    bool ListnenOnPort( const CORE::CString& address , 
                        CORE::UInt16 port            ); 

    virtual const CORE::CString& GetClassTypeName( void ) const;

    private:
    
    typedef CORE::CTEventHandlerFunctor< CLogSvcServer > TEventCallback;
    
    void RegisterServerSocketEventHandlers( void );
    
    void OnServerSocketClientConnected( CORE::CNotifier* notifier           ,
                                        const CORE::CEvent& eventid         ,
                                        CORE::CICloneable* eventdata = NULL );

    void OnServerSocketClientDisconnected( CORE::CNotifier* notifier           ,
                                           const CORE::CEvent& eventid         ,
                                           CORE::CICloneable* eventdata = NULL );

    void OnServerSocketClientError( CORE::CNotifier* notifier           ,
                                    const CORE::CEvent& eventid         ,
                                    CORE::CICloneable* eventdata = NULL );
                                           
    void OnServerSocketServerSocketClosed( CORE::CNotifier* notifier           ,
                                           const CORE::CEvent& eventid         ,
                                           CORE::CICloneable* eventdata = NULL );
                                           
    void OnServerSocketServerSocketError( CORE::CNotifier* notifier           ,
                                          const CORE::CEvent& eventid         ,
                                          CORE::CICloneable* eventdata = NULL );
                                           
    void OnServerSocketServerSocketClientError( CORE::CNotifier* notifier           ,
                                                const CORE::CEvent& eventid         ,
                                                CORE::CICloneable* eventdata = NULL );    
    private:
    
    typedef std::map< CORE::UInt32, TClientInfo > TClientInfoMap;
    
    COMCORE::CTCPServerSocket m_tcpServer;
    TClientInfoMap m_clientInfoMap;
        
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LOGSERVICELIB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LOGSERVICELIB_CLOGSVCSERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

---------------------------------------------------------------------------*/