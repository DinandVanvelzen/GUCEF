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

#ifndef GUCEF_COM_CIMAPCOMMANDRESULT_H
#define GUCEF_COM_CIMAPCOMMANDRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMAPFETCHEDITEM_H
#include "gucefCOM_CImapFetchedItem.h"
#define GUCEF_COM_CIMAPFETCHEDITEM_H
#endif /* GUCEF_COM_CIMAPFETCHEDITEM_H ? */

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

enum EImapCommand
{
    IMAPCMD_CAPABILITY = 0,
    IMAPCMD_NOOP,
    IMAPCMD_LIST,
    IMAPCMD_SELECT,
    IMAPCMD_EXAMINE,
    IMAPCMD_STATUS,
    IMAPCMD_SEARCH,
    IMAPCMD_FETCH,
    IMAPCMD_STORE,
    IMAPCMD_EXPUNGE,
    IMAPCMD_CLOSE,
    IMAPCMD_CREATE,
    IMAPCMD_DELETE,
    IMAPCMD_APPEND,
    IMAPCMD_IDLE,
    IMAPCMD_COPY,
    IMAPCMD_MOVE
};

/*-------------------------------------------------------------------------*/

/** One line of the answer to LIST */
class GUCEF_COM_PUBLIC_CPP CImapMailboxEntry
{
    public:

    CORE::CString name;                         /**< UTF-8 */
    CORE::CString hierarchyDelimiter;
    CORE::CString::StringVector attributes;     /**< \HasNoChildren, \Sent, \Trash ... */

    CImapMailboxEntry( void );
};

/*-------------------------------------------------------------------------*/

/** What the server said about a mailbox that was selected or asked about with STATUS */
class GUCEF_COM_PUBLIC_CPP CImapMailboxStatus
{
    public:

    CORE::CString name;
    UInt32 exists;
    UInt32 recent;
    UInt32 unseen;
    UInt32 uidNext;
    UInt32 uidValidity;                         /**< the number that says whether the UIDs of the mailbox are still the ones that were known */
    bool readOnly;
    CORE::CString::StringVector flags;
    CORE::CString::StringVector permanentFlags;

    CImapMailboxStatus( void );
};

/*-------------------------------------------------------------------------*/

/** The outcome of one command */
class GUCEF_COM_PUBLIC_CPP CImapCommandResult : public CORE::CICloneable
{
    public:

    UInt32 requestId;
    EImapCommand command;
    bool ok;                                    /**< the server said OK */
    CORE::CString status;                       /**< OK, NO or BAD, empty when the command was not carried out */
    EMailErrorClass errorClass;
    CORE::CString text;                         /**< the text of the server, or of ours: for a log only after redaction */
    CORE::CString code;                         /**< response code of the completion: TRYCREATE, READ-ONLY, ... */
    GUCEF::vector< CImapMailboxEntry > mailboxes;       /**< LIST */
    CImapMailboxStatus mailbox;                         /**< SELECT, EXAMINE, STATUS */
    bool uidValidityChanged;                            /**< SELECT with an expected UIDVALIDITY that is not the one of the mailbox: the UIDs of before mean nothing */
    GUCEF::vector< UInt32 > numbers;                    /**< SEARCH: sequence numbers or UIDs */
    GUCEF::vector< CImapFetchedItem > fetched;          /**< FETCH, and the flags that STORE reports */
    UInt32 appendUid;                                   /**< APPEND with UIDPLUS */
    UInt32 copyUid;                                     /**< COPY and MOVE with UIDPLUS: the UID of the first message in the target mailbox, 0 when the server did not say */
    CORE::CString::StringVector capabilities;           /**< CAPABILITY */

    CImapCommandResult( void );
    CImapCommandResult( const CImapCommandResult& src );
    virtual ~CImapCommandResult() GUCEF_VIRTUAL_OVERRIDE;
    CImapCommandResult& operator=( const CImapCommandResult& src );
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

#endif /* GUCEF_COM_CIMAPCOMMANDRESULT_H ? */
