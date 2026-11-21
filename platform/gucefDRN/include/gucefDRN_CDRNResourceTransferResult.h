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

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H
#define GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How a resource transfer ended, or why a call failed: a status and words for a log or a person */
class GUCEF_DRN_EXPORT_CPP CDRNResourceTransferResult
{
    public:

    TDRNResourceStatus status;
    CORE::CString message;

    CDRNResourceTransferResult( void );
    CDRNResourceTransferResult( TDRNResourceStatus status, const CORE::CString& message );

    bool IsOk( void ) const;

    /** "OK", "NOT_FOUND", ... */
    static const char* StatusToString( TDRNResourceStatus status );

    /** A status as it came over the wire: values this node does not know are IO_ERROR */
    static TDRNResourceStatus StatusFromWire( UInt32 value );

    /** Whether trying again later can succeed: BUSY, LINK_LOST, TIMEOUT, CHANGED (with a new open) */
    static bool IsRetryable( TDRNResourceStatus status );
};

typedef CORE::CTCloneableObj< CDRNResourceTransferResult > CDRNResourceTransferResultEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H ? */
