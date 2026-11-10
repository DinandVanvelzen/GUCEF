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

#ifndef GUCEF_COM_CPOP3COMMANDRESULT_H
#define GUCEF_COM_CPOP3COMMANDRESULT_H

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

enum EPop3Command
{
    POP3CMD_STAT = 0,
    POP3CMD_LIST,
    POP3CMD_UIDL,
    POP3CMD_RETR,
    POP3CMD_TOP,
    POP3CMD_DELE,
    POP3CMD_RSET,
    POP3CMD_NOOP
};

/*-------------------------------------------------------------------------*/

/** One line of an answer to LIST or UIDL */
class GUCEF_COM_PUBLIC_CPP CPop3ListEntry
{
    public:

    UInt32 number;              /**< the number of the message in this session */
    UInt32 size;                /**< octets, LIST */
    CORE::CString uid;          /**< UIDL: stays the same for the message from session to session */

    CPop3ListEntry( void );
};

/*-------------------------------------------------------------------------*/

/** The outcome of one command */
class GUCEF_COM_PUBLIC_CPP CPop3CommandResult : public CORE::CICloneable
{
    public:

    UInt32 requestId;
    EPop3Command command;
    bool ok;                                    /**< +OK */
    EMailErrorClass errorClass;                 /**< why not, when not */
    CORE::CString text;                         /**< the text of the status line, or of the error: for the log only after redaction */
    UInt32 number;                              /**< the message number the command was about */
    UInt32 messageCount;                        /**< STAT */
    UInt32 totalSize;                           /**< STAT */
    GUCEF::vector< CPop3ListEntry > entries;    /**< LIST and UIDL without a number */
    CORE::CDynamicBuffer data;                  /**< RETR and TOP: the message as it came, dot stuffing undone, lines end in CR LF */

    CPop3CommandResult( void );
    CPop3CommandResult( const CPop3CommandResult& src );
    virtual ~CPop3CommandResult() GUCEF_VIRTUAL_OVERRIDE;
    CPop3CommandResult& operator=( const CPop3CommandResult& src );
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

#endif /* GUCEF_COM_CPOP3COMMANDRESULT_H ? */
