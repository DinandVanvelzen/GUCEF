/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H
#define GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CJOBCONTROLCOMMAND_H
#include "gucefCORE_CJobControlCommand.h"
#define GUCEF_CORE_CJOBCONTROLCOMMAND_H
#endif /* GUCEF_CORE_CJOBCONTROLCOMMAND_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H
#include "gucefDRNUTILS_CSchedulerActionInfo.h"
#define GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Custom actions of the service itself (not of a work source): declared through GetCustomActions() and applied by the
 *  host after its role, rate limit, idempotency and audit checks (CSchedulerHost::SetCustomActionHandler())
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CISchedulerCustomActionHandler
{
    public:

    CISchedulerCustomActionHandler( void );

    virtual ~CISchedulerCustomActionHandler();

    virtual void GetCustomActions( TSchedulerActionInfoVector& actions ) const = 0;

    /** On the caller's thread, no lock of the remote layer held */
    virtual CORE::TJobControlStatus ApplyCustomAction( const CORE::SJobControlCommand& command, CORE::CDataNode& detail, CORE::CString& message ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CISCHEDULERCUSTOMACTIONHANDLER_H ? */
