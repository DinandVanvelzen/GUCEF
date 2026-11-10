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

#ifndef GUCEF_CORE_CTIOACCESSTOSTLSTREAM_H
#define GUCEF_CORE_CTIOACCESSTOSTLSTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>
#include <iostream>
#include <sstream>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"         
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"        /* base class for all media manipulators */
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CTIOACCESSTOSTLSTREAMBUFF_H
#include "gucefCORE_CTIOAccessToStlStreambuff.h"
#define GUCEF_CORE_CTIOACCESSTOSTLSTREAMBUFF_H
#endif /* GUCEF_CORE_CTIOACCESSTOSTLSTREAMBUFF_H ? */

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
 *  Provides the argument for constructing the STL stream base class without a buffer
 *  std::iostream takes a streambuf pointer but std::stringstream has no such constructor: passing NULL to it
 *  would select the std::string constructor which is undefined behaviour (newer standard libraries throw)
 */
template< typename stl_stream_type >
struct CTStlStreamNoBufferCtorArg
{
    static std::streambuf* Get( void ) { return GUCEF_NULL; }
};

template<>
struct CTStlStreamNoBufferCtorArg< std::stringstream >
{
    static std::ios_base::openmode Get( void ) { return std::ios_base::in | std::ios_base::out; }
};

/*-------------------------------------------------------------------------*/

/**
 *  Class template for redirecting STL stream operations to IOAccess operations
 */
template< typename stl_stream_type, typename io_streambuff_adapter_type >
class CTIOAccessToStlStream : public stl_stream_type
{
    public:
 
    using Base = stl_stream_type;

    CTIOAccessToStlStream( CIOAccess& access );

    virtual ~CTIOAccessToStlStream() GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  Default constructor. 
     *  Requires calling InitIOStreamAccess() at a later stage before actual utilization of an instance of this class
     */
    CTIOAccessToStlStream( void );

    bool InitIOStreamAccess( CIOAccess& access );

    private:

    CTIOAccessToStlStream( const CTIOAccessToStlStream& src );             /**< not implemented */
    CTIOAccessToStlStream& operator=( const CTIOAccessToStlStream& src );  /**< not implemented */

    private:

    io_streambuff_adapter_type m_stlAdapter;
};

/*-------------------------------------------------------------------------*/

typedef CTIOAccessToStlStream< std::iostream, TIOAccessToStlBasicCharStreambuff >           CIOAccessToStlCharStream;
typedef CTIOAccessToStlStream< std::stringstream, TIOAccessToStlBasicCharStringbuff >       CIOAccessToStlStringStream;

typedef CIOAccessToStlCharStream                                                            CIOAccessToIOStream;  // legacy name remap - deprecated

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename stl_stream_type, typename io_streambuff_adapter_type >
CTIOAccessToStlStream< stl_stream_type, io_streambuff_adapter_type >::CTIOAccessToStlStream( void )
    : stl_stream_type( CTStlStreamNoBufferCtorArg< stl_stream_type >::Get() )
    , m_stlAdapter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename stl_stream_type, typename io_streambuff_adapter_type >
CTIOAccessToStlStream< stl_stream_type, io_streambuff_adapter_type >::CTIOAccessToStlStream( CIOAccess& access )
    : stl_stream_type( CTStlStreamNoBufferCtorArg< stl_stream_type >::Get() )
    , m_stlAdapter()
{GUCEF_TRACE;

    InitIOStreamAccess( access );
}

/*-------------------------------------------------------------------------*/

template< typename stl_stream_type, typename io_streambuff_adapter_type >
CTIOAccessToStlStream< stl_stream_type, io_streambuff_adapter_type >::~CTIOAccessToStlStream()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename stl_stream_type, typename io_streambuff_adapter_type >
bool 
CTIOAccessToStlStream< stl_stream_type, io_streambuff_adapter_type >::InitIOStreamAccess( CIOAccess& access )
{GUCEF_TRACE;

    if ( m_stlAdapter.Init( access ) )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        std::iostream::init( &m_stlAdapter, false );
        #else
        std::iostream::init( &m_stlAdapter );
        #endif

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTIOACCESSTOSTLSTREAM_H ? */
