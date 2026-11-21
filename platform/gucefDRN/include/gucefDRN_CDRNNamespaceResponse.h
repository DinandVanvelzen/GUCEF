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

#ifndef GUCEF_DRN_CDRNNAMESPACERESPONSE_H
#define GUCEF_DRN_CDRNNAMESPACERESPONSE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACERESULT_H
#include "gucefDRN_CDRNNamespaceResult.h"
#define GUCEF_DRN_CDRNNAMESPACERESULT_H
#endif /* GUCEF_DRN_CDRNNAMESPACERESULT_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEINFO_H
#include "gucefDRN_CDRNNamespaceInfo.h"
#define GUCEF_DRN_CDRNNAMESPACEINFO_H
#endif /* GUCEF_DRN_CDRNNAMESPACEINFO_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACESHAREINFO_H
#include "gucefDRN_CDRNNamespaceShareInfo.h"
#define GUCEF_DRN_CDRNNAMESPACESHAREINFO_H
#endif /* GUCEF_DRN_CDRNNAMESPACESHAREINFO_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACELISTPAGE_H
#include "gucefDRN_CDRNNamespaceListPage.h"
#define GUCEF_DRN_CDRNNAMESPACELISTPAGE_H
#endif /* GUCEF_DRN_CDRNNAMESPACELISTPAGE_H ? */

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

/**
 *  What an asynchronous call of CDRNNamespaceClient delivers: made by the call, filled on a pool thread, and read once the
 *  CORE::CFutureResult of the call is ready (CFutureResult::IsReady() or Await()). Which members hold something depends
 *  on the call: Negotiate: info; Shares: shares; Stat: entries (in the order of the paths, each with its own status);
 *  List: page; ListAll: entries and dirRevision. 'result' is the outcome of the call as a whole.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceResponse
{
    public:

    CDRNNamespaceResult result;
    CDRNNamespaceInfo info;
    GUCEF::vector< CDRNNamespaceShareInfo > shares;
    GUCEF::vector< CDRNNamespaceEntry > entries;
    CDRNNamespaceListPage page;
    CORE::CString dirRevision;
    bool changedDuringListing;      /**< ListAll: the directory's revision differed between its pages */

    CDRNNamespaceResponse( void );
};

typedef CORE::CTSharedPtr< CDRNNamespaceResponse, MT::CMutex > CDRNNamespaceResponsePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACERESPONSE_H ? */
