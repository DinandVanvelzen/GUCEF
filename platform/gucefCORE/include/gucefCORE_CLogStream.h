/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CLOGSTREAM_H
#define GUCEF_CORE_CLOGSTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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
 *  A log-aware wrapper around CVariantStream that composites the stream and
 *  adds single-threaded app support via synchronous flushing.
 *
 *  The key difference from CVariantStream is in WriteSegmentEnd(): after
 *  writing the end marker it checks m_isPulseGeneratorThread and, if true,
 *  immediately flushes all thread stream buffers to the logging backends
 *  within the same call.  This ensures single-threaded applications (where
 *  no separate logging task is running the pulse) still see log output.
 */
class GUCEF_CORE_PUBLIC_CPP CLogStream : public CTSharedObjCreator< CLogStream, MT::CMutex >
{
    public:

    typedef CTSharedObjCreator< CLogStream, MT::CMutex >::TBasicSharedPtrType   CLogStreamPtr;
    typedef CTSharedObjCreator< CLogStream, MT::CMutex >::TSharedPtrType      CLogStreamTypedPtr;

    CLogStream( void );

    explicit CLogStream( bool isPulseGeneratorThread );

    virtual ~CLogStream() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Direct access to the composited variant stream.
     */
    CVariantStream& GetStream( void );

    const CVariantStream& GetStream( void ) const;

    /**
     *  Forwarding stream insertion operator - writes to the composited CVariantStream.
     */
    template< typename T >
    CLogStream& operator<<( const T& data );

    /**
     *  Writes a VOID segment-end marker to the composited stream and, when
     *  m_isPulseGeneratorThread is true, synchronously flushes all thread
     *  stream buffers to the registered logging backends.
     *
     *  This guarantees that single-threaded applications without a dedicated
     *  logging task still emit log output on every completed log entry.
     */
    void WriteSegmentEnd( void );

    bool GetIsPulseGeneratorThread( void ) const;

    void SetIsPulseGeneratorThread( bool isPulseGeneratorThread );

    private:

    CVariantStream m_variantStream;
    bool m_isPulseGeneratorThread;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CLogStream::CLogStreamPtr        CLogStreamPtr;
typedef CLogStream::CLogStreamTypedPtr   CLogStreamTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
CLogStream&
CLogStream::operator<<( const T& data )
{GUCEF_TRACE;

    m_variantStream << data;
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

#endif /* GUCEF_CORE_CLOGSTREAM_H ? */
