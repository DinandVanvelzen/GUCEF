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

//-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------//

#include <malloc.h>     // memory management
#include <string.h>     // needed for memcpy()
#include <stdio.h>      // needed for dump to file and seek defines
#include "dvstrutils.h" // Needed for ASCII readers
#include "MFILE.h"      // header file with type and prototypes

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */  

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

//-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------//

void
mfinit( MFILE* file        , 
        const void *mchunk , 
        const UInt64 size  )
{
        /*
         *      Initialize values of existing MFILE struct
         */
        file->curpos = mchunk;
        file->mchunk = mchunk;
        file->size = size;
        file->offset = 0;
}

/*-------------------------------------------------------------------------*/

MFILE*
mfcreate( const void *mchunk , 
          UInt64 size        )
{
        /*
         *      Create the struct for the memfile
         */
        MFILE* mf;
        if ( !mchunk ) return NULL;

        mf = ( MFILE* ) malloc( sizeof( MFILE ) );
        mf->curpos = (const char*) mchunk;
        mf->mchunk = (const char*) mchunk;
        mf->size = size;
        mf->offset = 0;
        return( mf );
}

/*-------------------------------------------------------------------------*/

Int32 
mfdumptofile( MFILE *mfile         , 
              const char *filename , 
              const char *mode     )
{
        /*
         *      Dump the MFILE to a file on disk
         */
        FILE *fptr = fopen( filename, mode );
        if( !fptr ) return 0;
        fwrite( mfile->mchunk, (size_t) mfile->size, 1, fptr );
        fclose( fptr );
        return 1;
}

/*-------------------------------------------------------------------------*/

/*
 *	MFILE version of getc()
 */
char 
mfgetc( MFILE *mfile )
{
        if ( mfile->offset < mfile->size )
        {
	        char tmp = *mfile->curpos;
                ++mfile->offset;
                ++mfile->curpos;
                return tmp;
        }
        return '\0';                
}

/*-------------------------------------------------------------------------*/

/**
 *	Identical to mfgetc()
 */
char
mgetc( MFILE *mfile )
{
        return mfgetc( mfile );
}

/*-------------------------------------------------------------------------*/

UInt32
mfread( void *dest      ,
        UInt32 esize    , 
        UInt32 elements , 
        MFILE *mfile    )
{
        /*
         *      If possible copy the data of size specified into dest buffer
         *      It is assumed the dest buffer already has the needed space
         *      allocated.
         */
        size_t maxel;
        if ( mfile->offset + esize <= mfile->size )
        {
                /*
                 *      Set max posible amount of elements to read
                 */
                maxel = ( mfile->size - mfile->offset ) / esize;
                if ( elements > maxel )
                {
                        elements = maxel;
                }

                /*
                 *      Read the maximum possible amount of element
                 */
                memcpy( dest, mfile->curpos, esize*elements );

                /*
                 *      Update the 'file pointer'
                 */
                mfile->offset += ( esize * elements );
                mfile->curpos = mfile->mchunk + mfile->offset;

                /* Like fread() this is the number of elements read, not the number of bytes */
                return( (UInt32) elements );
        }
        return( 0 );
}

/*-------------------------------------------------------------------------*/

UInt32
mfreads( char **dest  , 
         MFILE *mfile )
{
        /*
         *      Read untill a whitespace and alloc and fill dest char buffer
         */
        int i = 0;
        UInt32 rbytes = 0, offset = 0;
        UInt32 stop[ 4 ];
        stop[ 0 ] = Find_Char( ' ', ( char* ) mfile->curpos, mfile->size-mfile->offset );
        stop[ 1 ] = Find_Char( '\n', ( char* ) mfile->curpos, mfile->size-mfile->offset );
        stop[ 2 ] = Find_Char( '\r', ( char* ) mfile->curpos, mfile->size-mfile->offset );
        stop[ 3 ] = Find_Char( '\t', ( char* ) mfile->curpos, mfile->size-mfile->offset );

        /*
         *      Determine where to stop
         */
        offset = 0;        
        for ( i=0; i<4; i++ )
        {
                if ( ( stop[ i ] >= 0 ) && ( stop[ i ] < offset  ) )
                {
                        offset = stop[ i ];
                }
        }

        /*
         *      Make sure it also works if there is no whitespace
         */
        if ( offset < 0 )
        {
                offset = 0;
        }

        /*
         *      Alloc and obtain string
         */
        *dest = ( char* ) calloc( sizeof( char ), offset+1 );
        *dest[ offset+1 ] = '\n';
        rbytes = mfread( *dest, sizeof( char ), offset, mfile );
        return( rbytes ); 
}

/*-------------------------------------------------------------------------*/

/**
 *      Free a block of memory that was allocated using mfreadl() or
 *      mfreads()
 */
void
mfree( void *mem )
{
        free( mem );
}

/*-------------------------------------------------------------------------*/

UInt32
mfreadl( char **dest  , 
         MFILE *mfile )
{
        /*
         *      Read untill eol and alloc and fill dest char buffer
         */
        UInt32 i, len, orgpos=mfile->offset;   
        for ( i=mfile->offset; i<mfile->size; ++i )
        {
                if ( ( mfile->mchunk[ i ] == '\n' ) ||
                     ( mfile->mchunk[ i ] == '\r' ) )
                {
                        mfile->offset = i+1;
                        if ( i+1 < mfile->size )
                        {
                                if ( mfile->mchunk[ i+1 ] == '\r' )
                                {
                                        ++mfile->offset;        
                                }
                                else
                                if ( mfile->mchunk[ i+1 ] == '\n' )
                                {
                                        ++mfile->offset;        
                                }                                        
                        }
                        len = i-orgpos;
                        *dest = malloc( len+1 );
                        memcpy( *dest, mfile->curpos, len );
                        (*dest)[ len ] = 0;
                        mfile->curpos = mfile->mchunk + mfile->offset;                        
                        return len;        
                }     
        }
        if ( i == mfile->size )
        {
                if ( i != mfile->offset )
                {
                        len = i-mfile->offset;
                        *dest = malloc( len+1 );
                        memcpy( *dest, mfile->curpos, len );
                        (*dest)[ len ] = 0;
                        
                        mfile->offset = i;
                        mfile->curpos = mfile->mchunk + mfile->offset;                        
                        return len;                         
                }
        }
        *dest = NULL;
        return 0; 
}

/*-------------------------------------------------------------------------*/

UInt32
mfwrite( void *source    , 
         UInt32 esize    , 
         UInt32 elements , 
         MFILE *mfile    )
{
        /*
         *      Write data to file in memory
         *
        unsigned maxel;

        /*
         *      prevent write beyond eof
         *
        if ( mfile->offset + esize <= mfile->size )
        {
                /*
                 *      Get max elements to write
                 *
                maxel = ( mfile->size - mfile->offset ) / esize;
                if ( maxel < elements )
                {
                        elements = maxel;
                }

                /*
                 *      Do the actual writing
                 *
                memcpy( mfile->curpos, source, ( esize * elements ) );

                /*
                 *      Update 'file pointer'
                 *
                mfile->offset += ( esize * elements );
                mfile->curpos = (void*)((UInt32)mfile->mchunk + (UInt32)mfile->offset);

                /*
                 *      Return bytes written
                 *
                return( esize * elements );
        }        */
        return 0;
}

/*-------------------------------------------------------------------------*/

Int32
mfsetpos( MFILE *mfile, UInt64 offset )
{
        /*
         *      The mfsetpos() function repositions a file position indicator
         *      for the MFILE.
         */
       if ( offset <= mfile->size )
       {
                /*
                 *      Valid range, set the filepointer
                 */
                mfile->curpos = mfile->mchunk + offset;
                mfile->offset = offset;
                return 0;
       }
       
       /*
        *       Set the current pos at the end of the file
        */
       mfile->curpos = mfile->mchunk + mfile->size;
       mfile->offset = mfile->size;       
       return 0;
}

/*-------------------------------------------------------------------------*/

Int32
mfseek( MFILE *mfile, Int64 offset, Int32 origin )
{
        /*
         *      Set the file pointer to the given offset from the origin
         *      provided, origin can be : SEEK_CUR, SEEK_END, SEEK_SET
         */
        switch( origin )
        {
                case SEEK_CUR :
                {
                        mfsetpos( mfile, (UInt64) mfile->offset+offset );                      
                        return 1;
                }
                case SEEK_END :
                {
                        if ( offset <= (Int32)mfile->size )
                        {
                                mfile->curpos = mfile->mchunk + mfile->size-offset;
                                mfile->offset = mfile->size-offset;
                                return 1;
                        }
                        mfile->curpos = mfile->mchunk;
                        mfile->offset = 0;                           
                        return 1;
                }
                case SEEK_SET :
                {
                        mfsetpos( mfile, offset );
                        return 1;
                }
                default :
                {
                        return 1;
                }
        }
}

/*-------------------------------------------------------------------------*/

UInt64
mftell( const MFILE *mfile )
{
        /*
         *      Return the current offset in bytes from the start of the file
         */
        return( mfile->offset );
}

/*-------------------------------------------------------------------------*/

Int32
mfeof( const MFILE *mfile )
{
        /*
         *      Return whether the end of the mem chunck representing the file
         *      has been reached.
         */
        return( mfile->offset >= mfile->size );
}

/*-------------------------------------------------------------------------*/

void
mfdestroy( MFILE *mfile )
{
        /*
         *      Free the memory chunck and the mfile structure
         */
       free( mfile );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
