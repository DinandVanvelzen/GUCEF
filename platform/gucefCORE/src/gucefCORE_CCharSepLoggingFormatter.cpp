/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <sstream>
#include <iomanip>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#include "gucefCORE_CCharSepLoggingFormatter.h"

#ifndef GUCEF_CORE_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_ESSENTIALS_H
#endif /* GUCEF_CORE_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CCharSepLoggingFormatter::TypeName = "CharSepLoggingFormatter";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CCharSepLoggingFormatter::CCharSepLoggingFormatter( void )
    : CILoggingFormatter()
    , m_seperatorChar( '|' )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCharSepLoggingFormatter::~CCharSepLoggingFormatter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCharSepLoggingFormatter::CCharSepLoggingFormatter( const CCharSepLoggingFormatter& src )
    : CILoggingFormatter( src )
    , m_seperatorChar( src.m_seperatorChar )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCharSepLoggingFormatter&
CCharSepLoggingFormatter::operator=( const CCharSepLoggingFormatter& src )
{GUCEF_TRACE;
    
    if ( this != &src )
    {
        m_seperatorChar = src.m_seperatorChar;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString& 
CCharSepLoggingFormatter::GetType( void ) const
{GUCEF_TRACE;
    
    return TypeName;
}

/*-------------------------------------------------------------------------*/

CString
CCharSepLoggingFormatter::FormatLogMessage( const TLogMsgType logMsgType ,
                                            const Int32 logLevel         ,
                                            const CString& logMessage    ,
                                            const UInt32 threadId        ,
                                            const CTimestamp& timestamp  )
{GUCEF_TRACE;

    return timestamp.ToDateTime().ToIso8601DateTimeString( true, true ) + m_seperatorChar + 
           UInt32ToString( threadId ) + m_seperatorChar + 
           CLogManager::GetLogMsgTypeString( logMsgType ) + m_seperatorChar + 
           LogLevelToString( logLevel ) + m_seperatorChar + 
           logMessage;
}

/*-------------------------------------------------------------------------*/

CString
CCharSepLoggingFormatter::FormatLogMessage( const TLogMsgType logMsgType     ,
                                            const Int32 logLevel             ,
                                            const CVariantStream& logMessage ,
                                            const UInt32 threadId            ,
                                            const CTimestamp& timestamp      )
{GUCEF_TRACE;

    return FormatLogMessage( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CCharSepLoggingFormatter::SetSeperatorChar( char seperator )
{GUCEF_TRACE;

    m_seperatorChar = seperator;
}

/*-------------------------------------------------------------------------*/

char
CCharSepLoggingFormatter::GetSeperatorChar( void ) const
{GUCEF_TRACE;

    return m_seperatorChar;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
