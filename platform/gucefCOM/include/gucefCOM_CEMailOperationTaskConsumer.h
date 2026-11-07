/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CEMAILOPERATIONTASKCONSUMER_H
#define GUCEF_COM_CEMAILOPERATIONTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COM_CSMTPCLIENT_H
#include "gucefCOM_CSmtpClient.h"
#define GUCEF_COM_CSMTPCLIENT_H
#endif /* GUCEF_COM_CSMTPCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The task that carries out a mail operation on a thread of a pool: it calls the same blocking function that the
 *  caller can call itself (CSmtpClient::SendMessageSync() and the like), so the work is the same and only the thread
 *  that does it differs.
 */
class GUCEF_COM_PUBLIC_CPP CEMailOperationTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CEMailOperationTaskConsumer( void );

    virtual ~CEMailOperationTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /** The task data stays: it has the result */
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CEMailOperationTaskConsumer( const CEMailOperationTaskConsumer& src );
    CEMailOperationTaskConsumer& operator=( const CEMailOperationTaskConsumer& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CEMailOperationTaskConsumer, MT::CMutex > TEMailOperationTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILOPERATIONTASKCONSUMER_H ? */
