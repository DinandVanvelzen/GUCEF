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

#ifndef GUCEF_COM_CIMAPCOMMANDPARTS_H
#define GUCEF_COM_CIMAPCOMMANDPARTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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

/**
 *  What a command of the client says apart from its words: the arguments that come from the application are never
 *  pasted into a command as they are. A command is the text up to a literal, the literal bytes, the text up to the next
 *  literal, and so on; text that cannot be quoted (non ASCII, control characters) travels as a literal, and what has a
 *  CR, LF or NUL is refused.
 */
class GUCEF_COM_PUBLIC_CPP CImapCommandParts
{
    public:

    GUCEF::vector< CORE::CString > texts;               /**< texts.size() == literals.size() + 1; the literal i follows text i with its {size} */
    GUCEF::vector< CORE::CDynamicBuffer > literals;

    CImapCommandParts( void );

    void Clear( void );

    /** Adds words that are fixed text of the client */
    void AddWord( const char* text );

    /** Adds a string argument: quoted when it can be, a literal when it has to be. @return false for a CR, LF or NUL in it */
    bool AddString( const CORE::CString& utf8Text );

    /** Adds raw bytes as a literal (APPEND) */
    void AddLiteral( const void* data, UInt32 size );

    /** Adds a mailbox name, converted to the form of the wire. @return false for a name with a control character */
    bool AddMailbox( const CORE::CString& utf8Name );

    /** The text of all parts as it would be written in a log: literals as {n} */
    CORE::CString ToLogText( void ) const;

    /** Adds the parts of another command to the end of this one */
    void Append( const CImapCommandParts& other );

    /** Whether any literal is in the command */
    bool HasLiterals( void ) const;

    /** The text of the quoted string, or false when it has to be a literal */
    static bool Quote( const CORE::CString& text, CORE::CString& quoted );

    /** Whether the text is a valid sequence set of numbers, ranges, commas and *: "1:5,9,12:*" */
    static bool IsValidSequenceSet( const CORE::CString& set );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPCOMMANDPARTS_H ? */
