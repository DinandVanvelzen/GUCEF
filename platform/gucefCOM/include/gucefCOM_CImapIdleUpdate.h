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

#ifndef GUCEF_COM_CIMAPIDLEUPDATE_H
#define GUCEF_COM_CIMAPIDLEUPDATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILCLIENTBASE_H
#include "gucefCOM_CEMailClientBase.h"
#define GUCEF_COM_CEMAILCLIENTBASE_H
#endif /* GUCEF_COM_CEMAILCLIENTBASE_H ? */

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

/** What happened in the mailbox while the client was idle */
class GUCEF_COM_PUBLIC_CPP CImapIdleUpdate : public CORE::CICloneable
{
    public:

    enum EKind
    {
        KIND_EXISTS = 0,        /**< 'number' messages are in the mailbox now */
        KIND_EXPUNGE,           /**< the message with sequence number 'number' is gone */
        KIND_RECENT,
        KIND_FLAGS              /**< the flags of the message with this sequence number changed: 'flags', and the uid when the server says */
    };

    EKind kind;
    UInt32 number;
    bool hasUid;
    UInt32 uid;
    CORE::CString::StringVector flags;

    CImapIdleUpdate( void );
    CImapIdleUpdate( const CImapIdleUpdate& src );
    virtual ~CImapIdleUpdate() GUCEF_VIRTUAL_OVERRIDE;
    CImapIdleUpdate& operator=( const CImapIdleUpdate& src );
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

#endif /* GUCEF_COM_CIMAPIDLEUPDATE_H ? */
