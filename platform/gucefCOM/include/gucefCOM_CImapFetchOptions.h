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

#ifndef GUCEF_COM_CIMAPFETCHOPTIONS_H
#define GUCEF_COM_CIMAPFETCHOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

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

/** What a fetch of new mail does */
class GUCEF_COM_PUBLIC_CPP CImapFetchOptions
{
    public:

    CORE::CString mailbox;                      /**< "INBOX" */
    UInt32 lastUidValidity;                     /**< the UIDVALIDITY of the last fetch, 0 for none */
    UInt32 lastUid;                             /**< the highest UID that was handled by the last fetch, 0 for none */
    UInt32 maxMessages;                         /**< at most this many messages per fetch, 0: all */
    bool markSeen;                              /**< the messages that are fetched become \Seen */
    bool deleteAfterFetch;                      /**< the messages that were fetched are deleted and expunged */
    GUCEF::vector< CORE::CString > headerNames; /**< only messages that have these headers are fetched (the search is done by the server)... */
    GUCEF::vector< CORE::CString > headerTexts; /**< ...with this text in them, one text per name, empty: the header only has to be there */
    CMimeLimits mimeLimits;

    CImapFetchOptions( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPFETCHOPTIONS_H ? */
