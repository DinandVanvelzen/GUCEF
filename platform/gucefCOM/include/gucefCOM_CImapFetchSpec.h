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

#ifndef GUCEF_COM_CIMAPFETCHSPEC_H
#define GUCEF_COM_CIMAPFETCHSPEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_EMAILTYPES_H
#include "gucefCOM_EMailTypes.h"
#define GUCEF_COM_EMAILTYPES_H
#endif /* GUCEF_COM_EMAILTYPES_H ? */

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

/** What a FETCH brings */
class GUCEF_COM_PUBLIC_CPP CImapFetchSpec
{
    public:

    bool uid;                                   /**< always true for a UID FETCH, the server adds it */
    bool flags;
    bool internalDate;
    bool size;
    bool envelope;
    bool bodyStructure;
    GUCEF::vector< CORE::CString > sections;    /**< "" the whole message, "HEADER", "TEXT", "1", "1.MIME", "HEADER.FIELDS (FROM TO)": fetched with BODY.PEEK, the \Seen flag stays as it is */
    bool markSeen;                              /**< BODY instead of BODY.PEEK: the message becomes seen */
    UInt32 partialOffset;                       /**< with partialLength > 0 only that part of the first section is fetched */
    UInt32 partialLength;

    CImapFetchSpec( void );

    /** The items for the command, or false for an item that is not valid (a section with a character that could end the command) */
    bool Build( CORE::CString& items ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPFETCHSPEC_H ? */
