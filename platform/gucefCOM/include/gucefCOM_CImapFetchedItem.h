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

#ifndef GUCEF_COM_CIMAPFETCHEDITEM_H
#define GUCEF_COM_CIMAPFETCHEDITEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMAPRESPONSE_H
#include "gucefCOM_CImapResponse.h"
#define GUCEF_COM_CIMAPRESPONSE_H
#endif /* GUCEF_COM_CIMAPRESPONSE_H ? */

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

/** What a FETCH response said about one message */
class GUCEF_COM_PUBLIC_CPP CImapFetchedItem
{
    public:

    UInt32 sequence;
    bool hasUid;
    UInt32 uid;
    bool hasFlags;
    CORE::CString::StringVector flags;
    bool hasInternalDate;
    CORE::CString internalDate;         /**< as the server wrote it, dd-Mon-yyyy hh:mm:ss +zzzz */
    bool hasSize;
    UInt32 size;                        /**< RFC822.SIZE */
    bool hasEnvelope;
    CImapEnvelope envelope;
    bool hasBodyStructure;
    CImapBodyStructure bodyStructure;
    GUCEF::vector< CImapFetchSection > sections;

    CImapFetchedItem( void );

    /** The section of the name ("" for the whole message), GUCEF_NULL when it was not fetched */
    const CImapFetchSection* FindSection( const CORE::CString& name ) const;

    /** Whether the flag is set, \Seen for example, without regard for case */
    bool HasFlag( const CORE::CString& flag ) const;

    /** Reads the items of an untagged FETCH response. @return false when the response is not a valid FETCH */
    static bool FromResponse( const CImapResponse& response, CImapFetchedItem& item );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPFETCHEDITEM_H ? */
