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

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#define GUCEF_CORE_CLOGSTREAMSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_LOGTYPES_H
#include "gucefCORE_LogTypes.h"
#define GUCEF_CORE_LOGTYPES_H
#endif /* GUCEF_CORE_LOGTYPES_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

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
 *  RAII scope guard for log stream segments.
 *  On destruction, writes a VOID marker to end the segment.
 *  This allows natural C++ scoping to delineate log entries.
 */
class GUCEF_CORE_PUBLIC_CPP CLogStreamScope
{
    public:

    CLogStreamScope( const TLogMsgType logMsgType ,
                     const Int32 logLevel         );

    ~CLogStreamScope();

    CVariantStream* operator->( void );
    
    CVariantStream& operator*( void );

    CVariantStream& GetStream( void );

    /**
     *  Convenience streaming operator for the scope guard
     */
    template< typename T >
    CLogStreamScope& operator<<( const T& data );

    /**
     *  Static provided here to avoid including the full CLogManager header in the logging macros.
     *  It will forward the call provided the core system is available. If not, it will be a no-op.
     */
    static void FlushLogs( void );

    private:

    CLogStreamScope( const CLogStreamScope& src );              /**< not implemented, don't use */
    CLogStreamScope& operator=( const CLogStreamScope& src );   /**< not implemented, don't use */

    CVariantStreamPtr m_stream;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
CLogStreamScope&
CLogStreamScope::operator<<( const T& data )
{GUCEF_TRACE;

    if ( !m_stream.IsNULL() )
    {
        (*m_stream) << data;
    }
    return *this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */
