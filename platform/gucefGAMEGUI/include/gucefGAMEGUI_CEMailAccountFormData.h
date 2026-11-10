/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#ifndef GUCEF_GAMEGUI_CEMAILACCOUNTFORMDATA_H
#define GUCEF_GAMEGUI_CEMAILACCOUNTFORMDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of a mailbox as the form shows them and hands them on (the data of CEMailAccountForm::SaveRequestedEvent and TestRequestedEvent).
 *  The password is in it only as typed, for whoever keeps or uses it: never log this object. The password text of an object is overwritten
 *  when the object is destroyed or assigned to.
 */
class GUCEF_GAMEGUI_EXPORT_CPP CEMailAccountFormData : public CORE::CICloneable
{
    public:

    CORE::CString address;
    CORE::CString displayName;
    CORE::CString userName;             /**< empty: the address */
    CORE::CString password;
    bool useImap;                       /**< IMAP (true) or POP3 */
    CORE::CString incomingHost;
    UInt16 incomingPort;                /**< 0: the usual one */
    Int32 incomingSecurity;             /**< 0 SSL/TLS, 1 STARTTLS, 2 none */
    CORE::CString outgoingHost;
    UInt16 outgoingPort;
    Int32 outgoingSecurity;
    CORE::CString mailbox;
    UInt32 pollSeconds;
    bool useIdle;
    Int32 processedAction;              /**< what is done with a game mail once it was handled: 0 leave it in the inbox, 1 copy it to the folder, 2 move it to the folder (it is removed from the inbox) */
    CORE::CString processedFolder;      /**< the folder of processedAction, empty for none */
    bool pop3Delete;                    /**< POP3: a handled game mail is deleted from the server */
    bool autoAccept;                    /**< an invitation to a game the player agreed to be introduced to is accepted by itself */
    CORE::CString providerId;           /**< the id of the provider preset the person works with (CEMailProviderPresets), "custom" for none; empty: the provider the address says */
    Int32 authMode;                     /**< 0: the password above, 1: the account is signed in with the provider named below (OAuth 2.0); the password is not used then */
    CORE::CString oauthProvider;        /**< the gucefAUTH provider of authMode 1, empty otherwise */

    CEMailAccountFormData( void );
    CEMailAccountFormData( const CEMailAccountFormData& src );
    virtual ~CEMailAccountFormData();
    CEMailAccountFormData& operator=( const CEMailAccountFormData& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CEMAILACCOUNTFORMDATA_H ? */
