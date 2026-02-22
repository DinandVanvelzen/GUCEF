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

#include "gucefCORE_CJsonLoggingFormatter.h"

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

const CString CJsonLoggingFormatter::TypeName = "JsonLoggingFormatter";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CJsonLoggingFormatter::CJsonLoggingFormatter( void )
    : CILoggingFormatter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJsonLoggingFormatter::~CJsonLoggingFormatter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJsonLoggingFormatter::CJsonLoggingFormatter( const CJsonLoggingFormatter& src )
    : CILoggingFormatter( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJsonLoggingFormatter&
CJsonLoggingFormatter::operator=( const CJsonLoggingFormatter& src )
{GUCEF_TRACE;
    
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString& 
CJsonLoggingFormatter::GetType( void ) const
{GUCEF_TRACE;
    
    return TypeName;
}

/*-------------------------------------------------------------------------*/

CString 
CJsonLoggingFormatter::EscapeJson( const CString& s ) 
{
    std::ostringstream o;
    for ( UInt32 i=0; i<s.Length(); ++i ) 
    {
        char c = s[ i ];
        switch ( c ) 
        {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
            {
                if ('\x00' <= c && c <= '\x1f') 
                {
                    o << "\\u"
                    << std::hex << std::setw(4) << std::setfill('0') << (int) c;
                } 
                else 
                {
                    o << c;
                }
            }
        }
    }
    return o.str();
}

/*-------------------------------------------------------------------------*/

CString
CJsonLoggingFormatter::FormatLogMessage( const TLogMsgType logMsgType ,
                                         const Int32 logLevel         ,
                                         const CString& logMessage    ,
                                         const UInt32 threadId        ,
                                         const CTimestamp& timestamp  )
{GUCEF_TRACE;

    return "{\"timestamp\"=\"" + timestamp.ToDateTime().ToIso8601DateTimeString( true, true ) + "\"" 
           ", \"thread\"=" + UInt32ToString( threadId ) +
           ", \"type\"=\"" + CLogManager::GetLogMsgTypeString( logMsgType ) + "\""
           ", \"lvl\"=\"" + LogLevelToString( logLevel ) + "\""
           ", \"msg\"=\"" + EscapeJson( logMessage ) + "\"}";
}

/*-------------------------------------------------------------------------*/

CString
CJsonLoggingFormatter::FormatLogMessage( const TLogMsgType logMsgType     ,
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
