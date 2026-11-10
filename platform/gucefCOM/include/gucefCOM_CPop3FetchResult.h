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

#ifndef GUCEF_COM_CPOP3FETCHRESULT_H
#define GUCEF_COM_CPOP3FETCHRESULT_H

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
class GUCEF_COM_PUBLIC_CPP CPop3FetchedMessage
{
    public:

    CORE::CString uid;
    UInt32 number;
    CEMailMessage message;
    bool parsed;                                /**< false: the message was larger than the limits or could not be read, 'message' is empty */
    bool deleted;

    CPop3FetchedMessage( void );
};

/*-------------------------------------------------------------------------*/

/** What a fetch of new mail found. 'allUids' is everything the server had, for the caller to prune its set of known ids */
class GUCEF_COM_PUBLIC_CPP CPop3FetchResult : public CORE::CICloneable
{
    public:

    bool success;
    EMailErrorClass errorClass;
    CORE::CString errorDescription;
    UInt32 messageCountOnServer;
    CORE::CString::StringVector allUids;
    GUCEF::vector< CPop3FetchedMessage > messages;

    CPop3FetchResult( void );
    CPop3FetchResult( const CPop3FetchResult& src );
    virtual ~CPop3FetchResult() GUCEF_VIRTUAL_OVERRIDE;
    CPop3FetchResult& operator=( const CPop3FetchResult& src );
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

#endif /* GUCEF_COM_CPOP3FETCHRESULT_H ? */
