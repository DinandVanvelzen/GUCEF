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

#ifndef GUCEF_CORE_CILOGGINGFORMATTER_H
#define GUCEF_CORE_CILOGGINGFORMATTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_CORE_LOGLEVELS_H
#include "gucefCORE_LogLevels.h"
#define GUCEF_CORE_LOGLEVELS_H
#endif /* GUCEF_CORE_LOGLEVELS_H ? */

#ifndef GUCEF_CORE_LOGTYPES_H
#include "gucefCORE_LogTypes.h"
#define GUCEF_CORE_LOGTYPES_H
#endif /* GUCEF_CORE_LOGTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      FORWARD DECLARATIONS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

class CVariantStream;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Interface class for implementations of log formatters
 */
class GUCEF_CORE_PUBLIC_CPP CILoggingFormatter
{
    public:

    virtual CString FormatLogMessage( const TLogMsgType logMsgType ,
                                      const Int32 logLevel         ,
                                      const CString& logMessage    ,
                                      const UInt32 threadId        ,
                                      const CTimestamp& timestamp  ) = 0;

    virtual CString FormatLogMessage( const TLogMsgType logMsgType     ,
                                      const Int32 logLevel             ,
                                      const CVariantStream& logMessage ,
                                      const UInt32 threadId            ,
                                      const CTimestamp& timestamp      ) = 0;

    virtual const CString& GetType( void ) const = 0;
    
    CILoggingFormatter( void );                                       /**< interface class: no-op */
    virtual ~CILoggingFormatter();                                    /**< interface class: no-op */
    CILoggingFormatter( const CILoggingFormatter& src );              /**< interface class: no-op */
    CILoggingFormatter& operator=( const CILoggingFormatter& src );   /**< interface class: no-op */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CILOGGINGFORMATTER_H ? */

