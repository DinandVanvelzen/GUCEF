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

#include <string.h>            /* needed for memcpy() */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"       /* cross-platform file utils */
#include "dvcppfileutils.h"    /* cross-platform C++ file utils */
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#include "gucefCORE_CSubFileAccess.h"      /* definition of the class implemented here */

#ifndef GUCEF_CORE_UCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */ 

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

CSubFileAccess::CSubFileAccess( void )
    : CIOAccess()      ,
      m_file( NULL )   ,
      m_filename()     ,
      m_size( 0 )      ,
      m_totalSize( 0 ) ,
      m_offset( 0 )
{GUCEF_TRACE;
                     
}

/*-------------------------------------------------------------------------*/

bool
CSubFileAccess::Load( const CString& file ,
                      const UInt64 offset ,
                      const UInt64 size   )
{GUCEF_TRACE;
    
    Close();
    
    m_offset = offset;
    m_size = size;
    m_filename = file;
    m_file = fopen( m_filename.C_String() ,
                    "rb"                  );

    if ( NULL != m_file )
    {
        fseek( m_file, (long) m_offset, SEEK_SET );
        m_totalSize = FileSize( m_filename );
        return true;
    }                
    return false;
}

/*-------------------------------------------------------------------------*/

CSubFileAccess::~CSubFileAccess()
{GUCEF_TRACE;

    Close();    
}

/*-------------------------------------------------------------------------*/

void 
CSubFileAccess::Open( void )
{GUCEF_TRACE;

    Close();
    
    m_file = fopen( m_filename.C_String() ,
                    "rb"                  );
    fseek( m_file, (long) m_offset, SEEK_SET );
    m_totalSize = FileSize( m_filename );
}

/*-------------------------------------------------------------------------*/

void 
CSubFileAccess::Close( void )
{GUCEF_TRACE;

    if ( NULL != m_file )
    { 
        fclose( m_file );
        m_file = NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CSubFileAccess::Opened( void ) const
{GUCEF_TRACE;

    return m_file != NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 
CSubFileAccess::Read( void *dest      ,
                      UInt32 esize    ,
                      UInt32 elements )
{GUCEF_TRACE;

    Int64 pos = ftell( m_file );
    UInt64 readBytes = esize * elements;
    
    if ( pos + readBytes > m_offset + m_size )
    {        
        UInt64 remainder = m_size - ( pos - m_offset );
        elements = (UInt32) ( remainder / esize ); // intentional cutoff using int division
        if ( 0 == elements ) 
            return 0;
    }
    
    return (UInt32) fread( dest     ,
                           esize    ,
                           elements ,
                           m_file    );
}

/*-------------------------------------------------------------------------*/

UInt64 
CSubFileAccess::Tell( void ) const
{GUCEF_TRACE;

    return ftell( m_file ) - m_offset;
}

/*-------------------------------------------------------------------------*/

Int32 
CSubFileAccess::Seek( Int64 offset ,
                      Int32 origin )
{GUCEF_TRACE;
    
    if ( origin == SEEK_END )
    {
        return fseek( m_file            , 
                      (long) ( m_offset + m_size ), 
                      SEEK_SET          );
    }
    if ( origin == SEEK_CUR )
    {
        Int32 pos = ftell( m_file );
        if ( ( pos + offset < (Int32) m_size )   && 
             ( pos + offset > (Int32) m_offset )  )
        {    
            return fseek( m_file       ,
                          (long) ( pos + offset ) ,
                          SEEK_SET     );
        }
        return 1;
    }
    if ( origin == SEEK_SET )
    {
        if ( ( m_offset + offset < m_totalSize ) &&
             ( offset < (Int32) m_size )          )
        {
            return fseek( m_file            ,
                          (long) ( m_offset + offset ),
                          SEEK_SET          );
        }
        return 1;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 
CSubFileAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    return Seek( position, SEEK_SET );
}

/*-------------------------------------------------------------------------*/

char 
CSubFileAccess::GetChar( void )
{GUCEF_TRACE;

    return (char) fgetc( m_file );
}

/*-------------------------------------------------------------------------*/

bool 
CSubFileAccess::Eof( void ) const
{GUCEF_TRACE;

    return (UInt32)ftell( m_file ) >= ( m_offset + m_size );
}

/*-------------------------------------------------------------------------*/

bool 
CSubFileAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CSubFileAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/
       
UInt32 
CSubFileAccess::Write( const void* srcdata ,
                       UInt32 esize        ,
                       UInt32 elements     )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CSubFileAccess::Write( CIOAccess& sourceData )
{GUCEF_TRACE;
    
    return 0;
}

/*-------------------------------------------------------------------------*/

bool 
CSubFileAccess::IsValid( void )
{GUCEF_TRACE;

    return m_file != NULL;
}

/*-------------------------------------------------------------------------*/

UInt64 
CSubFileAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

void
CSubFileAccess::Flush( void )
{GUCEF_TRACE;

    if ( NULL != m_file )
    {
        fflush( m_file );
    }
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CSubFileAccess::Clone( void ) const
{GUCEF_TRACE;

    CSubFileAccess* access = GUCEF_NEW CSubFileAccess();
    if( access->Load( m_filename ,
                      m_offset   ,
                      m_size     ) )
    {
        return access;
    }
    
    GUCEF_DELETE access;
    return NULL;
}

/*-------------------------------------------------------------------------*/

const CString&
CSubFileAccess::GetFilename( void ) const
{GUCEF_TRACE;

    return m_filename; 
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
