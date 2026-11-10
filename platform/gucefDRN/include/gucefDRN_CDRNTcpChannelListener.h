/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_DRN_CDRNTCPCHANNELLISTENER_H
#define GUCEF_DRN_CDRNTCPCHANNELLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNCHANNELLISTENER_H
#include "gucefDRN_CIDRNChannelListener.h"
#define GUCEF_DRN_CIDRNCHANNELLISTENER_H
#endif /* GUCEF_DRN_CIDRNCHANNELLISTENER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

namespace COMCORE {
class CTCPServerSocket;
}

namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Accepts "tcp" channels. Endpoint parameter "maxConnections" (default 256) limits the number of connections.
 *  The listener must outlive the channels it accepted to keep them connected.
 */
class GUCEF_DRN_EXPORT_CPP CDRNTcpChannelListener : public CIDRNChannelListener
{
    public:

    CDRNTcpChannelListener( void );

    virtual ~CDRNTcpChannelListener();

    virtual bool Listen( const CDRNEndpoint& local ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void StopListening( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsListening( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CDRNEndpoint GetListenEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNTcpChannelListener( const CDRNTcpChannelListener& src );
    CDRNTcpChannelListener& operator=( const CDRNTcpChannelListener& src );

    COMCORE::CTCPServerSocket* m_serverSocket;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CDRNEndpoint m_listenEndpoint;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNTCPCHANNELLISTENER_H ? */
