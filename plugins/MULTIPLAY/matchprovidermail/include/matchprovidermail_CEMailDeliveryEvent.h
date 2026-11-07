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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H
#include "matchprovidermail_CEMailMatchCodec.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H ? */

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

/** Where a mail is on its way to the others */
enum EMailDeliveryState
{
    MAILDELIVERY_NONE      = 0 ,    /**< nothing is known of a mail for this */
    MAILDELIVERY_QUEUED    = 1 ,    /**< in the outbox, not yet accepted by the mail server (a temporary failure keeps it here) */
    MAILDELIVERY_SENT      = 2 ,    /**< the mail server of the player accepted it; no sign yet that it arrived */
    MAILDELIVERY_DELIVERED = 3 ,    /**< every recipient showed that it arrived (acknowledgement, or their next move) */
    MAILDELIVERY_FAILED    = 4      /**< the mail server refused it for good */
};
typedef enum EMailDeliveryState TMailDeliveryState;

/*-------------------------------------------------------------------------*/

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailDeliveryStateToString( TMailDeliveryState state );

/*-------------------------------------------------------------------------*/

/** The data of CEMailOutbox::DeliveryChangedEvent */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailDeliveryEvent : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    UInt32 revision;
    TMailMatchKind kind;
    TMailDeliveryState state;
    CORE::CString description;      /**< a phrase of ours: no addresses, no content */

    CEMailDeliveryEvent( void );
    CEMailDeliveryEvent( const CEMailDeliveryEvent& src );
    virtual ~CEMailDeliveryEvent() GUCEF_VIRTUAL_OVERRIDE;
    CEMailDeliveryEvent& operator=( const CEMailDeliveryEvent& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H ? */
