/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTBOXITEM_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTBOXITEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H
#include "matchprovidermail_CEMailDeliveryEvent.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** One mail waiting to be sent or to be confirmed; stored as one file in the outbox directory */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailOutboxItem
{
    public:

    UInt32 id;
    TMailMatchKind kind;
    CORE::CString matchId;
    UInt32 revision;
    CORE::CString messageId;
    CORE::CString envelopeFrom;
    CORE::CString::StringVector recipients;         /**< addresses of the envelope */
    CORE::CString::StringVector awaiting;           /**< recipients (lower case) that have not shown that the mail arrived */
    CORE::CString packageSha256;
    bool needsConfirmation;                         /**< the item stays until the recipients confirm; else it ends when the server accepted it */
    TMailDeliveryState state;
    UInt32 attempts;
    Int64 createdAt;
    Int64 nextAttemptAt;
    Int64 lastSentAt;
    CORE::CDynamicBuffer message;                   /**< the mail as it goes over the wire */

    CEMailOutboxItem( void );

    /** The bytes of the file, a fixed layout */
    bool ToBytes( CORE::CDynamicBuffer& bytes ) const;

    /** Strict reader of the bytes of a file */
    bool FromBytes( const void* data, UInt32 size );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTBOXITEM_H ? */
