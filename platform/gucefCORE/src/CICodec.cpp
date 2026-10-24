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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDEFAULTCODECINSTANCE_H
#include "gucefCORE_CDefaultCodecInstance.h"
#define GUCEF_CORE_CDEFAULTCODECINSTANCE_H
#endif /* GUCEF_CORE_CDEFAULTCODECINSTANCE_H ? */

#include "CICodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CICodec::CICodec( void )
    : CICloneable()
    , CITypeNamed()
    , m_lastStatus()
    , m_lastStatusLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CICodec::CICodec( const CICodec& src )
    : CICloneable( src )
    , CITypeNamed( src )
    , m_lastStatus()
    , m_lastStatusLock()
{GUCEF_TRACE;

    // the outcome of calls belongs to the object they were made on, a copy starts without
}

/*-------------------------------------------------------------------------*/

CICodec&
CICodec::operator=( const CICodec& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

CICodec::~CICodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CICodec::Encode( CIOAccess& source        ,
                 CIOAccess& dest          ,
                 const CValueList& params )
{GUCEF_TRACE;

    bool success = Encode( source, dest );
    SetLastStatus( success ? GUCEF_CODEC_STATUS_OK : GUCEF_CODEC_STATUS_FAILED );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CICodec::Decode( CIOAccess& source        ,
                 CIOAccess& dest          ,
                 const CValueList& params )
{GUCEF_TRACE;

    bool success = Decode( source, dest );
    SetLastStatus( success ? GUCEF_CODEC_STATUS_OK : GUCEF_CODEC_STATUS_FAILED );
    return success;
}

/*-------------------------------------------------------------------------*/

void
CICodec::GetParameterInfo( CCodecParameterInfoList& info ) const
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CICodec::CreateInstance( const CValueList& params       ,
                         CICodecInstancePtr& instance   )
{GUCEF_TRACE;

    // The instance works on a copy of the codec of its own, so it does not depend on how this object is kept alive
    CICodec* copy = static_cast< CICodec* >( Clone() );
    if ( GUCEF_NULL == copy )
        return false;

    instance = CICodecInstancePtr( GUCEF_NEW CDefaultCodecInstance( CDefaultCodecInstance::TICodecPtr( copy ), params ) );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CICodec::GetCapabilities( void ) const
{GUCEF_TRACE;

    return GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
CICodec::GetLastStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lastStatusLock );
    TLastStatusMap::const_iterator i = m_lastStatus.find( MT::GetCurrentTaskID() );
    if ( i != m_lastStatus.end() )
        return (*i).second.status;
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CString
CICodec::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lastStatusLock );
    TLastStatusMap::const_iterator i = m_lastStatus.find( MT::GetCurrentTaskID() );
    if ( i != m_lastStatus.end() )
        return (*i).second.message;
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CICodec::SetLastStatus( TCodecStatus status   ,
                        const char* message   ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lastStatusLock );
    SLastStatus& lastStatus = m_lastStatus[ MT::GetCurrentTaskID() ];
    lastStatus.status = status;
    if ( GUCEF_NULL != message && '\0' != *message )
        lastStatus.message = message;
    else
        lastStatus.message.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
