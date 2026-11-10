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

#ifndef GUCEF_COM_CIMAPFETCHRESULT_H
#define GUCEF_COM_CIMAPFETCHRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILCLIENTBASE_H
#include "gucefCOM_CEMailClientBase.h"
#define GUCEF_COM_CEMAILCLIENTBASE_H
#endif /* GUCEF_COM_CEMAILCLIENTBASE_H ? */

#ifndef GUCEF_COM_CEMAILMESSAGE_H
#include "gucefCOM_CEMailMessage.h"
#define GUCEF_COM_CEMAILMESSAGE_H
#endif /* GUCEF_COM_CEMAILMESSAGE_H ? */

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

/** A message that a fetch brought */
class GUCEF_COM_PUBLIC_CPP CImapFetchedMessage
{
    public:

    UInt32 uid;
    UInt32 size;
    CORE::CString::StringVector flags;
    CEMailMessage message;
    bool parsed;

    CImapFetchedMessage( void );
};

/*-------------------------------------------------------------------------*/

/** What a fetch of new mail found */
class GUCEF_COM_PUBLIC_CPP CImapFetchResult : public CORE::CICloneable
{
    public:

    bool success;
    EMailErrorClass errorClass;
    CORE::CString errorDescription;
    UInt32 uidValidity;
    bool uidValidityChanged;                    /**< the mailbox has other UIDs than the ones lastUid was about: all messages were looked at, none are skipped on the UID */
    UInt32 uidNext;
    UInt32 highestUid;                          /**< the highest UID handled, the lastUid for the next fetch */
    GUCEF::vector< CImapFetchedMessage > messages;

    CImapFetchResult( void );
    CImapFetchResult( const CImapFetchResult& src );
    virtual ~CImapFetchResult() GUCEF_VIRTUAL_OVERRIDE;
    CImapFetchResult& operator=( const CImapFetchResult& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPFETCHRESULT_H ? */
