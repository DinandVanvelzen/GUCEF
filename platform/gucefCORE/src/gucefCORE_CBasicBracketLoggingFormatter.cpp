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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#include "gucefCORE_CBasicBracketLoggingFormatter.h"

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

const CString CBasicBracketLoggingFormatter::TypeName = "BasicBracketLoggingFormatter";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CBasicBracketLoggingFormatter::CBasicBracketLoggingFormatter( void )
    : CILoggingFormatter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBasicBracketLoggingFormatter::~CBasicBracketLoggingFormatter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBasicBracketLoggingFormatter::CBasicBracketLoggingFormatter( const CBasicBracketLoggingFormatter& src )
    : CILoggingFormatter( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBasicBracketLoggingFormatter&
CBasicBracketLoggingFormatter::operator=( const CBasicBracketLoggingFormatter& src )
{GUCEF_TRACE;
    
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString& 
CBasicBracketLoggingFormatter::GetType( void ) const
{GUCEF_TRACE;
    
    return TypeName;
}

/*-------------------------------------------------------------------------*/

CString
CBasicBracketLoggingFormatter::FormatLogMessage( const TLogMsgType logMsgType ,
                                                 const Int32 logLevel         ,
                                                 const CString& logMessage    ,
                                                 const UInt32 threadId        ,
                                                 const CTimestamp& timestamp  )
{GUCEF_TRACE;

    return "[TIMESTAMP=" + timestamp.ToDateTime().ToIso8601DateTimeString( true, true ) +  
           "] [THREAD=" + UInt32ToString( threadId ) +
           "] [TYPE=" + CLogManager::GetLogMsgTypeString( logMsgType ) +
           "] [LVL=" + LogLevelToString( logLevel ) +
           "] [MSG=" + logMessage + "]";
}

/*-------------------------------------------------------------------------*/

CString
CBasicBracketLoggingFormatter::FormatLogMessage( const TLogMsgType logMsgType     ,
                                                 const Int32 logLevel             ,
                                                 const CVariantStream& logMessage ,
                                                 const UInt32 threadId            ,
                                                 const CTimestamp& timestamp      )
{GUCEF_TRACE;

    return FormatLogMessage( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
