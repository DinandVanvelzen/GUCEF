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

#ifndef GUCEF_LOGSERVICELIB_CLOGSVCCLIENT_H
#define GUCEF_LOGSERVICELIB_CLOGSVCCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_LOGSERVICELIB_MACROS_H
#include "GucefLogServiceLib_macros.h"
#define GUCEF_LOGSERVICELIB_MACROS_H
#endif /* GUCEF_LOGSERVICELIB_MACROS_H ? */

#ifndef GUCEF_CORE_CLOGGINGTASK_H
#include "gucefCORE_CLoggingTask.h"
#define GUCEF_CORE_CLOGGINGTASK_H
#endif /* GUCEF_CORE_CLOGGINGTASK_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

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

class GUCEF_LOGSERVICELIB_EXPORT_CPP CLogSvcClient : public CORE::CLoggingTask                                                           
{
    public:

    static const CORE::CString ClientVersion;
    
    static const CORE::CEvent ConnectingEvent;
    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent DisconnectedEvent;
    static const CORE::CEvent SocketErrorEvent;
    static const CORE::CEvent ConnectionInitializedEvent;
    static const CORE::CEvent IncompatibleWithServerEvent;
    
    static void RegisterEvents( void );
    
    public:
        
    CLogSvcClient( void );
    
    CLogSvcClient( const CORE::PulseGeneratorPtr& pulseGenerator ); 
    
    virtual ~CLogSvcClient() GUCEF_VIRTUAL_OVERRIDE;
    
    bool ConnectTo( const CORE::CString& address , 
                    CORE::UInt16 port            ,
                    bool blocking = false        ); 

    bool ConnectTo( const COMCORE::CHostAddress& address ,
                    bool blocking = false                );

    /**
     *      Attempts to reconnect to the server provided with
     *      ConnectTo(). If ConnectTo() has not yet been called then this
     *      member function has no effect.
     */
    bool Reconnect( bool blocking = false );
    
    void Close( void );
    
    void SetApplicationName( const CORE::CString& applicationName );
    
    const CORE::CString& GetApplicationName( void ) const;

    protected:
    
    /**
     *  Event callback member function. Processes events form other
     *  entities
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnPumpedNotify( CORE::CNotifier* notifier           ,
                                 const CORE::CEvent& eventid         ,
                                 CORE::CICloneable* eventdata = NULL ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycleLog( const CORE::TLogMsgType logMsgType ,
                                 const CORE::Int32 logLevel         ,
                                 const CORE::CString& logMessage    ,
                                 const CORE::UInt32 threadId        ,
                                 const CORE::CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycleLogWithoutFormatting( const CORE::TLogMsgType logMsgType ,
                                                  const CORE::Int32 logLevel         ,
                                                  const CORE::CString& logMessage    ,
                                                  const CORE::UInt32 threadId        ,
                                                  const CORE::CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycleLogFlush( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsReadyToProcessCycle( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    private:
    
    struct SLogMessage
    {
        CORE::Int8 msgHeader[ 24 ];
        CORE::CString logMsg;
    };
    typedef struct SLogMessage TLogMessage;
    
    typedef std::vector< TLogMessage > TLogMsgStack;
    
    private:
    
    void SendAllQueuedItems( void );
    
    private:
    
    COMCORE::CTCPClientSocket m_tcpClient;
    CORE::CString m_appName;
    bool m_connectionInitialized;
    TLogMsgStack m_logQueue;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LOGSERVICELIB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LOGSERVICELIB_CLOGSVCCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

---------------------------------------------------------------------------*/
