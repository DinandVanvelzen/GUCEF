/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_CIRCMESSAGE_H
#define GUCEF_COM_CIRCMESSAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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
 *  One IRC line with IRCv3 message tags:
 *
 *      [@key=value;key2 ][:prefix ]COMMAND[ param ...][ :trailing]
 *
 *  The trailing part is the last parameter, Parse() puts it in the list like any other and ToString() writes
 *  the last parameter as trailing when it has to be: when it is empty, has a space or starts with a colon.
 *  Tag values are escaped on the wire (\: for ;, \s for space, \\, \r, \n) and plain in this class.
 */
class GUCEF_COM_PUBLIC_CPP CIrcMessage : public CORE::CICloneable
{
    public:

    struct STag
    {
        CORE::CString key;
        CORE::CString value;
    };
    typedef struct STag TTag;
    typedef GUCEF::vector< TTag > TTagVector;

    TTagVector tags;
    CORE::CString prefix;                   /**< the source, nick!user@host or a server name, without the colon */
    CORE::CString command;                  /**< for example PRIVMSG or 001 */
    CORE::CString::StringVector params;     /**< the parameters, the trailing one included */

    CIrcMessage( void );

    explicit CIrcMessage( const CORE::CString& command );

    CIrcMessage( const CIrcMessage& src );

    virtual ~CIrcMessage();

    CIrcMessage& operator=( const CIrcMessage& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

    /**
     *  Reads a line, with or without the CR LF at its end
     *
     *  @return false if there is no command in the line
     */
    bool Parse( const CORE::CString& line );

    /** The line in wire format, without CR LF */
    CORE::CString ToString( void ) const;

    /** Adds a parameter, handy for building messages */
    CIrcMessage& AddParam( const CORE::CString& param );

    /** The parameter at the index, empty when there is none */
    const CORE::CString& GetParam( UInt32 index ) const;

    UInt32 GetParamCount( void ) const;

    bool HasTag( const CORE::CString& key ) const;

    bool GetTag( const CORE::CString& key, CORE::CString& value ) const;

    void SetTag( const CORE::CString& key, const CORE::CString& value );

    /** The nick of the prefix: the part before the !, or the whole prefix when it is a server */
    CORE::CString GetSourceNick( void ) const;

    /** The user of the prefix: between ! and @ */
    CORE::CString GetSourceUser( void ) const;

    /** The host of the prefix: after the @ */
    CORE::CString GetSourceHost( void ) const;

    /** Whether the command is a numeric reply, for example 001 */
    bool IsNumeric( void ) const;

    static CORE::CString EscapeTagValue( const CORE::CString& value );

    static CORE::CString UnescapeTagValue( const CORE::CString& value );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIRCMESSAGE_H ? */
