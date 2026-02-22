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

#ifndef GUCEF_CORE_CCHARSEPLOGGINGFORMATTER_H
#define GUCEF_CORE_CCHARSEPLOGGINGFORMATTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CILOGGINGFORMATTER_H
#include "gucefCORE_CILoggingFormatter.h"
#define GUCEF_CORE_CILOGGINGFORMATTER_H
#endif /* GUCEF_CORE_CILOGGINGFORMATTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Basic log formatter implementation which envelops the various fields using a 
 *  simple special character seperator formatting
 */
class GUCEF_CORE_PUBLIC_CPP CCharSepLoggingFormatter : public CILoggingFormatter
{
    public:

    static const CString TypeName;
    
    virtual CString FormatLogMessage( const TLogMsgType logMsgType ,
                                      const Int32 logLevel         ,
                                      const CString& logMessage    ,
                                      const UInt32 threadId        ,
                                      const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString FormatLogMessage( const TLogMsgType logMsgType     ,
                                      const Int32 logLevel             ,
                                      const CVariantStream& logMessage ,
                                      const UInt32 threadId            ,
                                      const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Provide ability to get the textual name of the formatter
     */
    virtual const CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CCharSepLoggingFormatter( void );            
    virtual ~CCharSepLoggingFormatter();    
    CCharSepLoggingFormatter( const CCharSepLoggingFormatter& src ); 
    CCharSepLoggingFormatter& operator=( const CCharSepLoggingFormatter& src );

    void SetSeperatorChar( char seperator );

    char GetSeperatorChar( void ) const;

    private:

    char m_seperatorChar;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCHARSEPLOGGINGFORMATTER_H ? */

