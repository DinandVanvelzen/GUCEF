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

#ifndef GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H
#define GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRNUTILS_CDRNUTILSAUDITRECORD_H
#include "gucefDRNUTILS_CDrnUtilsAuditRecord.h"
#define GUCEF_DRNUTILS_CDRNUTILSAUDITRECORD_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSAUDITRECORD_H ? */

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
 *  Receives one record per remote command, on the host's notification thread, outside every lock of the remote layer
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CIDrnUtilsAuditSink
{
    public:

    CIDrnUtilsAuditSink( void );

    virtual ~CIDrnUtilsAuditSink();

    virtual void OnAuditRecord( const SDrnUtilsAuditRecord& record ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H ? */
