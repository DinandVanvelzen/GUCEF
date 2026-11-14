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

#ifndef GUCEF_CORE_CTIOACCESSTOSTLSTREAMBUFF_H
#define GUCEF_CORE_CTIOACCESSTOSTLSTREAMBUFF_H

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
 *  Class template for deriving from STL stream buffer types while redirecting I/O to the IOAccess API
 */
template< typename base_streambuf_type >
class CTIOAccessToStlStreambuff : public base_streambuf_type
{   
    public:
 
    typedef base_streambuf_type                 Base;
    typedef typename Base::int_type             int_type;
    typedef typename Base::char_type            char_type;
    typedef typename Base::traits_type          traits_type;
    typedef typename traits_type::pos_type      pos_type;
    typedef typename traits_type::off_type      off_type;
    typedef std::streamsize                     streamsize;

    /**
     *  Default constructor. 
     *  Requires calling Init() at a later stage before actual utilization of an instance of this class
     */
    CTIOAccessToStlStreambuff( void );

    CTIOAccessToStlStreambuff( CIOAccess& access );

    virtual ~CTIOAccessToStlStreambuff() GUCEF_VIRTUAL_OVERRIDE;

    bool Init( CIOAccess& access );

    protected:
    
    virtual int_type overflow( int_type c ) GUCEF_VIRTUAL_OVERRIDE;

    virtual int_type underflow() GUCEF_VIRTUAL_OVERRIDE;

    virtual int sync() GUCEF_VIRTUAL_OVERRIDE;

    virtual streamsize showmanyc() GUCEF_VIRTUAL_OVERRIDE;

    virtual pos_type seekoff( off_type offset, std::ios_base::seekdir origin, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out ) GUCEF_VIRTUAL_OVERRIDE;

    virtual pos_type seekpos( pos_type pos, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out ) GUCEF_VIRTUAL_OVERRIDE;

    virtual streamsize xsputn( const char_type* inputBuffer, streamsize charsToWrite ) GUCEF_VIRTUAL_OVERRIDE;

    virtual streamsize xsgetn( char_type* outBuffer, streamsize charsToRead ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIOAccess* m_access;
};

/*-------------------------------------------------------------------------*/

typedef std::basic_streambuf< char, std::char_traits< char > >          basic_char_streambuf;
typedef std::basic_stringbuf< char, std::char_traits< char > >          basic_char_stringbuf;

typedef CTIOAccessToStlStreambuff< basic_char_streambuf >               TIOAccessToStlBasicCharStreambuff;
typedef CTIOAccessToStlStreambuff< basic_char_stringbuf >               TIOAccessToStlBasicCharStringbuff;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
CTIOAccessToStlStreambuff< base_streambuf_type >::CTIOAccessToStlStreambuff( void )
    : Base()
    , m_access( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
CTIOAccessToStlStreambuff< base_streambuf_type >::CTIOAccessToStlStreambuff( CIOAccess& access )
    : Base()
    , m_access( &access )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
CTIOAccessToStlStreambuff< base_streambuf_type >::~CTIOAccessToStlStreambuff()
{GUCEF_TRACE;

    m_access = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
bool
CTIOAccessToStlStreambuff< base_streambuf_type >::Init( CIOAccess& access )
{GUCEF_TRACE;

    if ( GUCEF_NULL != &access )
    {
        m_access = &access;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
typename CTIOAccessToStlStreambuff< base_streambuf_type >::int_type
CTIOAccessToStlStreambuff< base_streambuf_type >::overflow( int_type c ) 
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_access )
    {
        char_type chars[2];
        chars[0] = traits_type::to_char_type( c );
        chars[1] = '\0';
        if ( 2 == m_access->Write( &c, sizeof(char_type), 2 ) )
            return traits_type::not_eof( c );    
    }
    return traits_type::eof();
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
typename CTIOAccessToStlStreambuff< base_streambuf_type >::int_type
CTIOAccessToStlStreambuff< base_streambuf_type >::underflow()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_access )
    {
        char_type c;
        if ( 1 == m_access->Read( &c, sizeof(c), 1 ) )
            return traits_type::to_int_type( c );    
    }
    return traits_type::eof();
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
int 
CTIOAccessToStlStreambuff< base_streambuf_type >::sync()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_access )
    {
        m_access->Flush(); 
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
typename CTIOAccessToStlStreambuff< base_streambuf_type >::streamsize 
CTIOAccessToStlStreambuff< base_streambuf_type >::showmanyc()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_access )
    {
        return static_cast< streamsize >( m_access->GetSize() - m_access->Tell() );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
typename CTIOAccessToStlStreambuff< base_streambuf_type >::pos_type
CTIOAccessToStlStreambuff< base_streambuf_type >::seekoff( off_type offset, std::ios_base::seekdir origin, std::ios_base::openmode mode )
{GUCEF_TRACE;

    // change position by offset, according to way and mode
    if ( GUCEF_NULL != m_access )
    {
        Int32 fOrigin = -1;
        switch ( origin )
        {
            case std::ios_base::beg: fOrigin = SEEK_SET; break;
            case std::ios_base::cur: fOrigin = SEEK_CUR; break;
            case std::ios_base::end: fOrigin = SEEK_END; break;
            default: return pos_type(off_type(-1));
        }

        if ( 0 == m_access->Seek( (Int32) offset, fOrigin ) )
            return (pos_type) m_access->Tell(); 
    }
    return pos_type(off_type(-1));
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
typename CTIOAccessToStlStreambuff< base_streambuf_type >::pos_type
CTIOAccessToStlStreambuff< base_streambuf_type >::seekpos( pos_type pos, std::ios_base::openmode mode )
{GUCEF_TRACE;

    // change to specified position, according to mode
    if ( GUCEF_NULL != m_access )
    {
        if ( 0 == m_access->Setpos( (UInt32) pos ) )
            return (pos_type) m_access->Tell(); 
    }
    return pos_type(off_type(-1));
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
typename CTIOAccessToStlStreambuff< base_streambuf_type >::streamsize 
CTIOAccessToStlStreambuff< base_streambuf_type >::xsputn( const char_type* inputBuffer, streamsize charsToWrite )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_access )
    {
        UInt32 elementsWritten = m_access->Write( inputBuffer, sizeof(char_type), (UInt32)charsToWrite );
        return (streamsize) elementsWritten;    
    }
    return traits_type::eof();
}

/*-------------------------------------------------------------------------*/

template< typename base_streambuf_type >
typename CTIOAccessToStlStreambuff< base_streambuf_type >::streamsize 
CTIOAccessToStlStreambuff< base_streambuf_type >::xsgetn( char_type* outBuffer, streamsize charsToRead )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_access )
    {
        UInt32 elementsRead = m_access->Read( outBuffer, sizeof(char_type), (UInt32)charsToRead );
        return (streamsize) elementsRead;    
    }
    return traits_type::eof();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTIOACCESSTOSTLSTREAMBUFF_H ? */
