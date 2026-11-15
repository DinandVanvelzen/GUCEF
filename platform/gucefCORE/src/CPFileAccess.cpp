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

#include "CPFileAccess.h"      /* definition of the class implemented here */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
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

UInt32 GUCEF_CALLSPEC_PREFIX 
pfa_open( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{
        ( (CPFileAccess*) access->privdata )->Open();
        return 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX 
pfa_opened( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{
        return ( (CPFileAccess*) access->privdata )->Opened();
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
pfa_close( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{
        ( (CPFileAccess*) access->privdata )->Close();
        return 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX 
pfa_readl( struct SIOAccess* access , 
           char **dest              ) GUCEF_CALLSPEC_SUFFIX
{
        CString str = ( (CPFileAccess*) access->privdata )->ReadLine();
        *dest = GUCEF_NEW char[ str.Length()+1 ];
        memcpy( *dest, str.C_String(), str.Length() );        
        return str.Length();
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX 
pfa_reads( struct SIOAccess* access , 
           char **dest              ) GUCEF_CALLSPEC_SUFFIX
{
        CString str = ( (CPFileAccess*) access->privdata )->ReadString();
        *dest = GUCEF_NEW char[ str.Length()+1 ];
        memcpy( *dest, str.C_String(), str.Length() );
        return str.Length();
}

/*-------------------------------------------------------------------------*/
           
UInt32 GUCEF_CALLSPEC_PREFIX
pfa_read( struct SIOAccess* access , 
          void *dest               , 
          UInt32 esize             , 
          UInt32 elements          ) GUCEF_CALLSPEC_SUFFIX
{
        return ( (CPFileAccess*) access->privdata )->Read( dest     ,
                                                           esize    ,
                                                           elements );
}

/*-------------------------------------------------------------------------*/
          
UInt64 GUCEF_CALLSPEC_PREFIX
pfa_tell( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{
        return ( (CPFileAccess*) access->privdata )->Tell();
}

/*-------------------------------------------------------------------------*/

Int32 GUCEF_CALLSPEC_PREFIX
pfa_seek( struct SIOAccess* access , 
          Int64 offset             ,
          Int32 origin             ) GUCEF_CALLSPEC_SUFFIX
{
        return ( (CPFileAccess*) access->privdata )->Seek( offset ,
                                                           origin );
}

/*-------------------------------------------------------------------------*/
          
UInt32 GUCEF_CALLSPEC_PREFIX 
pfa_setpos( struct SIOAccess* access , 
            UInt64 pos               ) GUCEF_CALLSPEC_SUFFIX
{
        return ( (CPFileAccess*) access->privdata )->Setpos( pos );
}

/*-------------------------------------------------------------------------*/
            
Int32 GUCEF_CALLSPEC_PREFIX 
pfa_getc( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{
        return ( (CPFileAccess*) access->privdata )->GetChar();
}

/*-------------------------------------------------------------------------*/

Int32 GUCEF_CALLSPEC_PREFIX
pfa_eof( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{
        return ( (CPFileAccess*) access->privdata )->Eof();
}

/*-------------------------------------------------------------------------*/

void GUCEF_CALLSPEC_PREFIX 
pfa_free( void* mem ) GUCEF_CALLSPEC_SUFFIX
{
        GUCEF_DELETE []((char*)mem);
}

/*-------------------------------------------------------------------------*/

CPFileAccess::CPFileAccess( const CString& file )
        : _filename( file )
{
        _pafilecontrol = pacreate( file.C_String() ,
                                   "rb"            );
        _pafile = palinktocontrol( _pafilecontrol );                                                   
                
        _access.open = pfa_open;
        _access.close = pfa_close;                       
        _access.opened = pfa_opened;
        _access.readl = pfa_readl;
        _access.reads = pfa_reads;
        _access.read = pfa_read;
        _access.tell = pfa_tell;
        _access.seek = pfa_seek;
        _access.setpos = pfa_setpos;
        _access.getc = pfa_getc;
        _access.eof = pfa_eof;
        _access.memfree = pfa_free;
        _access.privdata = this;
}

/*-------------------------------------------------------------------------*/

CPFileAccess::~CPFileAccess()
{
        paunlink( _pafile );
        padestroy( _pafilecontrol );
}

/*-------------------------------------------------------------------------*/

void 
CPFileAccess::Open( void )
{
        pafsetpos( _pafile ,
                   0       ); 
}

/*-------------------------------------------------------------------------*/

void 
CPFileAccess::Close( void )
{
        pafsetpos( _pafile ,
                   0       ); 
}

/*-------------------------------------------------------------------------*/

bool 
CPFileAccess::Opened( void ) const
{
        return ( _pafilecontrol != GUCEF_NULL ) && ( _pafile != GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

UInt32 
CPFileAccess::Read( void *dest      ,
                    UInt32 esize    ,
                    UInt32 elements )
{
        return pafread( dest     ,
                        esize    ,
                        elements ,
                        _pafile  );
}

/*-------------------------------------------------------------------------*/

UInt64 
CPFileAccess::Tell( void ) const
{GUCEF_TRACE;

    return paftell( _pafile );
}

/*-------------------------------------------------------------------------*/

Int32 
CPFileAccess::Seek( Int64 offset ,
                    Int32 origin )
{GUCEF_TRACE;

    return pafseek( _pafile ,
                    offset  ,
                    origin  );
}

/*-------------------------------------------------------------------------*/

UInt32 
CPFileAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    return pafsetpos( _pafile  , 
                      position ); 
}

/*-------------------------------------------------------------------------*/

UInt64
CPFileAccess::GetSize( void ) const
{GUCEF_TRACE;

    return pafsize( _pafile );
}

/*-------------------------------------------------------------------------*/

char 
CPFileAccess::GetChar( void )
{
        return (char) pafgetc( _pafile );
}

/*-------------------------------------------------------------------------*/

bool 
CPFileAccess::Eof( void ) const
{
        return pafeof( _pafile ) == 1;
}

/*-------------------------------------------------------------------------*/

TIOAccess* 
CPFileAccess::CStyleAccess( void )
{
        return &_access;        
}

/*-------------------------------------------------------------------------*/

/**
 *      Is the access to the resource a valid one or
 *      has something gone wrong ?
 */
bool 
CPFileAccess::IsValid( void )
{
        if ( _pafile && _pafilecontrol )
        {       
                return palinked( _pafile ) == 1;
        }
        return false;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPFileAccess::Clone( void ) const
{
        return GUCEF_NEW CPFileAccess( _filename );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
