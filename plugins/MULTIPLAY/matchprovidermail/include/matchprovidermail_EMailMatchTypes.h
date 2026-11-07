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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#include "matchprovidermail_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a mail of a match is, the value of the header X-Redux-Kind */
enum EMailMatchKind
{
    MAILKIND_INVITE   = 0 ,     /**< the signed match record to an invitee, with the first turn when there is one */
    MAILKIND_ACCEPT   = 1 ,     /**< a signed acceptance of the record */
    MAILKIND_DECLINE  = 2 ,
    MAILKIND_TURN     = 3 ,     /**< a turn: the container with the signed preview and the package */
    MAILKIND_ACK      = 4 ,     /**< a turn arrived */
    MAILKIND_NUDGE    = 5 ,     /**< a signed reminder to the participant whose turn it is */
    MAILKIND_RESIGN   = 6 ,     /**< a signed leave */
    MAILKIND_FINISH   = 7 ,     /**< the last turn of a match */
    MAILKIND_RESEND   = 8 ,     /**< a request to send the turn of a revision again */
    MAILKIND_PROPOSAL = 9 ,     /**< a signed proposal of a decision */
    MAILKIND_VOTE     = 10 ,    /**< a signed vote on a proposal */
    MAILKIND_INTRO_REQUEST = 11 ,   /**< the creator of a game asks an address to introduce itself: the signed request (design 18) */
    MAILKIND_INTRO_REPLY   = 12 ,   /**< the answer: a signed introduction or an unsigned decline */
    MAILKIND_CANCEL   = 13 ,    /**< the creator (or a boss) closes a match that has not started: a signed decision, the proposal of the kind CANCEL */
    MAILKIND_INCOMPATIBLE = 14 ,/**< a participant says, signed, that it watches instead of playing, and why (the first turn cannot be played here, or by choice): a signed proposal of the kind MOVE_TO_SPECTATOR */
    MAILKIND_ROSTER   = 15 ,    /**< the host of a game that has no record yet publishes the signed table of the draft (a CMatchRoster) to every player */
    MAILKIND_DRAFT_PROPOSAL = 16 ,  /**< a signed proposal of the draft stage (force start, kick, the own spectating), to the host and to every other player */
    MAILKIND_DRAFT_VOTE     = 17 ,  /**< a signed vote on such a proposal, to the host and to every other player */
    MAILKIND_UNKNOWN  = 18
};
typedef enum EMailMatchKind TMailMatchKind;

/*-------------------------------------------------------------------------*/

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailMatchKindToString( TMailMatchKind kind );

/*-------------------------------------------------------------------------*/

/** @return false (kind is MAILKIND_UNKNOWN) when the text is no kind */
GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP bool TryStringToMailMatchKind( const CORE::CString& text, TMailMatchKind& kind );

/*-------------------------------------------------------------------------*/

/** How the data of a mail travels, the match setting "mailTransfer" */
enum EMailTransfer
{
    MAILTRANSFER_ATTACHMENT = 0 ,
    MAILTRANSFER_BODY_TEXT  = 1
};
typedef enum EMailTransfer TMailTransfer;

/*-------------------------------------------------------------------------*/

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailTransferToString( TMailTransfer transfer );
GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP bool TryStringToMailTransfer( const CORE::CString& text, TMailTransfer& transfer );

/*-------------------------------------------------------------------------*/

/** To whom a turn is mailed, the match setting "mailDistribution" */
enum EMailDistribution
{
    MAILDISTRIBUTION_NEXT_ONLY = 0 ,
    MAILDISTRIBUTION_ALL       = 1
};
typedef enum EMailDistribution TMailDistribution;

/*-------------------------------------------------------------------------*/

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailDistributionToString( TMailDistribution distribution );
GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP bool TryStringToMailDistribution( const CORE::CString& text, TMailDistribution& distribution );

/*-------------------------------------------------------------------------*/

/** What was found when the data was taken out of a mail */
enum EMailPayloadResult
{
    MAILPAYLOAD_OK           = 0 ,
    MAILPAYLOAD_NONE         = 1 ,  /**< no attachment and no block of text for the match: the carrier may have removed it */
    MAILPAYLOAD_TOO_LARGE    = 2 ,
    MAILPAYLOAD_DAMAGED      = 3 ,  /**< the text was damaged on its way: ask for a new copy */
    MAILPAYLOAD_BAD_ENCODING = 4
};
typedef enum EMailPayloadResult TMailPayloadResult;

/*-------------------------------------------------------------------------*/

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailPayloadResultToString( TMailPayloadResult result );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H ? */
