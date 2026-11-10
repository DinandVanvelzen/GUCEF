/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CIRPCSTREAM_H
#define GUCEF_COMCORE_CIRPCSTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The caller's side of a streaming call (gucefCOMCORE RPC design 10.4). Events are raised on the endpoint's
 *  notification pulse generator thread.
 */
class GUCEF_COMCORE_EXPORT_CPP CIRpcStream : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent MessageEvent;     /**< CRpcStreamMessageEventData: the message, its stream index */
    static const CORE::CEvent StatusEvent;      /**< CRpcStatus: the call ended */
    static const CORE::CEvent WritableEvent;    /**< credit or window available again */

    static void RegisterEvents( void );

    CIRpcStream( const CORE::PulseGeneratorPtr& pulseGenerator );
    virtual ~CIRpcStream() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Write( const CRpcMessage& message ) = 0;    /**< false without credit or after the end */
    virtual bool WritesDone( void ) = 0;                     /**< HALF_CLOSE */
    virtual void Cancel( const CRpcStatus& status ) = 0;

    /**
     *  Blocking form (design 10.9): the next message, or false when the call ended (status) or the time ran out
     *  (status DEADLINE_EXCEEDED). On the protocol thread it pumps that thread's pulse generator.
     */
    virtual bool ReadNext( CRpcMessage& message, CRpcStatus& status, UInt32 timeoutMs ) = 0;

    virtual void SetManualCredit( bool manual ) = 0;
    virtual void GrantCredit( UInt32 messages ) = 0;
    virtual bool IsDone( void ) const = 0;
    virtual CRpcStatus GetStatus( void ) const = 0;

    /** Client streams: the response message that ended the call */
    virtual CRpcMessage GetResponse( void ) const = 0;

    /** The REQUEST's seq: the call reference in this side's numbering */
    virtual UInt64 GetCallRef( void ) const = 0;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIRpcStream, MT::CMutex > CIRpcStreamPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIRPCSTREAM_H ? */
