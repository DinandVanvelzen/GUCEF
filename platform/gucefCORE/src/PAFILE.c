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

#include <malloc.h>     /* memory management */
#include <string.h>     /* standard c string utils */
#include <stdio.h>      /* standard I/O utils */

#ifndef GUCEF_CORE_MUTEX_H
#include "gucefMT_mutex.h"      /* mutex for threadsafety */
#endif /* GUCEF_CORE_MUTEX_H ? */

#ifndef GUCEF_CORE_PAFILE_H
#include "PAFILE.h"     /* paralell access file pointers */
#endif /* GUCEF_CORE_PAFILE_H ? */

#ifndef GUCEF_CORE_TSPRINTING_H
#include "tsprinting.h"		  /* threadsafe printing */ 
#endif /* GUCEF_CORE_TSPRINTING_H ? */

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      This is the control center structure. It keeps track of all the file
 *      refrences and all manipulations should use it's controllock mutex.
 *      This allows multiple threads to read from a file without having to
 *      worry about eachother. 
 */
struct SPAFILEControl
{
        struct SMutex *controllock;  /* mutex for threadsafety */
        FILE *fptr;                  /* the real file pointer */
        struct SPAFILE *refs;        /* pointer to our new file pointers */
        UInt32 refcount;             /* number of times that this file is refrenced */
        UInt64 offset;               /* current offset in file of file pointer */
        UInt64 fsize;                /* size of the physical file */
};

/*-------------------------------------------------------------------------*/

/**
 *      The parallell access equivalant of a FILE* structure.
 *      It's part of a linked list owned by the control structure.
 */
struct SPAFILE
{
        TPAFILEControl *control; /* access to the control mechanism */
        PAFILE *next;            /* next PAFILE* */
        PAFILE *prev;            /* previous PAFILE* */
        UInt64 offset;           /* current offset in our chunk */
        UInt64 csize;            /* size of the chunk */
        UInt64 coffset;          /* starting offset in the physical file */        
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      The following is a macro that checks wheter the offset between the
 *      PA file pointer and the actual position in the same.
 */
#define OFFSETCHANGED( pafile ) ( pafile->coffset + pafile->offset != pafile->control->offset )

/*
 *      This macro will move the 'physical' file pointer to the correct position
 *      if another process moved it. Thus making it seem as if the process has
 *      exclusive access.    FIXME - MACRO IS INVALID SOMEHOW
 */
#define ACTIVATE( pafile ) ( if ( OFFSETCHANGED( pafile ) ) { fseek( pafile->control->fptr, pafile->coffset + pafile->offset, SEEK_SET ); } )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      This opens a file on disk for parallell access reading. filename should
 *      be the filename and path for the file you want to create parallell access
 *      for offcourse. mode should be either "rb" or "r".
 *      If you no longer need the parallel file access you should call
 *      padestroy() to deallocate the data.
 *      This function is in itself not threadsafe.
 */
TPAFILEControl*
pacreate( const char *filename ,
          const char *mode     )
{
        FILE *fptr;
        TPAFILEControl *control;

        /*
         *      Check mode validity
         */
        if ( ( strcmp( "r", mode ) != 0 ) && ( strcmp( "rb", mode ) != 0 ) )
        {
                return NULL;
        }

        /*
         *      Open the file on disk.
         */
        fptr = fopen( filename, mode );
        if ( !fptr ) return NULL;

        /*
         *      Initialize the control center
         */
        control = malloc( sizeof( TPAFILEControl ) );
        if ( GUCEF_NULL == control )
        {
            fclose( fptr );
            return NULL;
        }
        control->fptr = fptr;
        control->refs = NULL;
        control->controllock = MutexCreate();
        control->refcount = 0;
        control->offset = 0;

        fseek( control->fptr, 0, SEEK_END );
        control->fsize = ftell( control->fptr );
        fseek( control->fptr, 0, SEEK_SET );

        return control;
}

/*-------------------------------------------------------------------------*/

/**
 *      Clean's up a TPAFILEControl structure which was allocated using
 *      pacreate() or pacreatechunk(). All links to the file are disabled.
 *      So make sure your code stops using them before calling this function.
 *      The file on disk will be closed.
 *      This function is in itself not threadsafe.
 */
void
padestroy( TPAFILEControl *control )
{
        /*
         *      Disable any refrences that are still active so that they won't
         *      attempt to access control evn though it no longer exists.
         */
        PAFILE *tmp;
        MutexLock( control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        tmp = control->refs;
        while ( tmp )
        {
                tmp->control = NULL;
                tmp = tmp->next;
        }

        /*
         *      Close the file and cleanup storage
         */
        fclose( control->fptr );
        free( control );
}

/*-------------------------------------------------------------------------*/

/**
 *      Creates an additional link to the file. After you are done using the
 *      file you should call paunlink().
 */
PAFILE*
palink( PAFILE *pafile )
{
        return palinktocontrol( pafile->control );        
}

/*-------------------------------------------------------------------------*/

/**
 *      Returns wheter the PAFILE is active. 1 equals true and 0 equals false
 *      in this context. If the PAFILE is no longer linked then the control
 *      has been destroyed and the PAFILE should be cleaned up using paunlink()
 */
UInt32
palinked( PAFILE *pafile )
{
        if ( pafile->control )
        {
                return 1;
        }
        return 0;
}

/*-------------------------------------------------------------------------*/

/**
 *      Basicly the same as palinktocontrol() with the exception that you
 *      can create a PAFILE* that will appear to be a complete file but will
 *      in reality only provide access to a specified section of the physical
 *      file. This allows you to treat a chunk of a physical file as a seperate
 *      file. A chunk size of 0 causes the remainder of the file from the given
 *      offset to be used as the file size.
 */
PAFILE*
palinkchunktocontrol( TPAFILEControl *control ,
                      UInt64 start_offset     ,
                      UInt64 chunk_size       )
{
        /*
         *      Get exclusive control access
         */
        PAFILE *tmp;
        if ( !control ) return NULL;
        MutexLock( control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        if ( control->fsize < start_offset+chunk_size )
        {
                MutexUnlock( control->controllock );
                return NULL;        
        }

        ++control->refcount;

        /*
         *      Prefix this link
         */
        tmp = control->refs;
        control->refs = ( PAFILE* ) malloc( sizeof( PAFILE ) );
        control->refs->next = tmp;
        if ( tmp )
        {
                tmp->prev = control->refs;
        }        
        tmp = control->refs;
        tmp->prev = NULL;
        tmp->offset = 0;
        tmp->control = control;
        tmp->coffset = start_offset;
        if ( !chunk_size )
        {
                tmp->csize = control->fsize - start_offset;
        }
        else
        {
                tmp->csize = chunk_size;
        }

        MutexUnlock( control->controllock );

        return tmp;        
}

/*-------------------------------------------------------------------------*/

/**
 *      Creates an additional link to the file. After you are done using the
 *      file you should call paunlink().
 */
PAFILE*
palinktocontrol( TPAFILEControl *control )
{
        return palinkchunktocontrol( control        ,
                                     0              ,
                                     control->fsize );
}

/*-------------------------------------------------------------------------*/

/**
 *      Clean's up a PAFILE structure which was created using palink() or
 *      using palinktocontrol().
 */
void
paunlink( PAFILE *pafile )
{
        TPAFILEControl *control;
        if ( !pafile->control )
        {
                free( pafile );
                return;
        }
        control = pafile->control;
        MutexLock( control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        --control->refcount;
        if ( pafile->next )
        {
                if ( pafile->prev )
                {
                        pafile->prev->next = pafile->next;
                        pafile->next->prev = pafile->prev;
                        free( pafile );
                        MutexUnlock( control->controllock );
                        return;
                }
                pafile->next->prev = NULL;
                free( pafile );
                MutexUnlock( control->controllock );
                return;
        }
        if ( pafile->prev )
        {
                pafile->prev->next = NULL;
        }
        free( pafile );
        MutexUnlock( control->controllock );
        return;
}

/*-------------------------------------------------------------------------*/

/**
 *      Returns the current file pointer, which is a void*
 */
UInt64
paftell( PAFILE *pafile )
{
    return pafile->offset;
}

/*-------------------------------------------------------------------------*/

UInt64
pafsize( PAFILE *pafile )
{
        return pafile->csize;
}

/*-------------------------------------------------------------------------*/

/**
 *      Set the file pointer to the given offset from the origin
 *      provided, origin can be : SEEK_CUR, SEEK_END, SEEK_SET
 */
Int32
pafseek( PAFILE *pafile ,
         Int64 offset   ,
         Int32 origin   )
{
        Int32 retval;
        if ( !pafile->control ) return 0;

        MutexLock( pafile->control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        switch( origin )
        {
                case SEEK_SET :
                {
                        if ( offset + pafile->coffset > pafile->coffset + pafile->csize )
                        {
                                /*
                                 *      We can't go beyond the end of the chunk
                                 */
                                offset = pafile->csize;
                        }
                        retval = fseek( pafile->control->fptr, (long)( pafile->coffset+offset ), SEEK_SET );
                        pafile->offset = ftell( pafile->control->fptr ) - pafile->coffset;
                        break;
                }
                case SEEK_CUR :
                {
                        if ( pafile->offset + offset + pafile->coffset > pafile->coffset + pafile->csize )
                        {
                                /*
                                 *      We can't go beyond the end of the chunk
                                 */
                                offset = pafile->csize - pafile->offset;
                        }
                        retval = fseek( pafile->control->fptr, pafile->offset+pafile->coffset+offset, SEEK_SET );
                        pafile->offset = ftell( pafile->control->fptr ) - pafile->coffset;
                        break;
                }
                case SEEK_END :
                {
                        if ( offset > (Int32)pafile->csize )
                        {
                                /*
                                 *      We can't go beyond the start of the chunk
                                 */
                                offset = pafile->csize;
                        }
                        retval = fseek( pafile->control->fptr, (long)( pafile->coffset+offset ), SEEK_SET );
                        pafile->offset = ftell( pafile->control->fptr ) - pafile->coffset;
                        break;
                }
                default :
                {
                        retval = 0;
                        break;
                }
        }
        MutexUnlock( pafile->control->controllock );

        return retval;
}

/*-------------------------------------------------------------------------*/

/**
 *      Set the file pointer to the given offset from the beginning of
 *      the file
 */
Int32
pafsetpos( PAFILE *pafile ,
           UInt64 offset  )
{
        Int32 retval;
        if ( !pafile->control ) return 0;

        MutexLock( pafile->control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        if ( offset > pafile->csize )
        {
                /*
                 *      We can't move beyond the end of the chunk
                 */
                offset = pafile->csize;
        }
        retval = fseek( pafile->control->fptr, (long)( pafile->coffset + offset ), SEEK_SET );
        pafile->offset = ftell( pafile->control->fptr ) - pafile->coffset;
        MutexUnlock( pafile->control->controllock );

        return retval;
}

/*-------------------------------------------------------------------------*/

/**
 *      PAFILE version of getc()
 */
Int32
pafgetc( PAFILE *pafile )
{
        Int32 retval;
        if ( !pafile->control ) return 0;

        MutexLock( pafile->control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        if ( OFFSETCHANGED( pafile ) ) { fseek( pafile->control->fptr, pafile->coffset + pafile->offset, SEEK_SET ); }
        retval = fgetc( pafile->control->fptr );
        MutexUnlock( pafile->control->controllock );

        return retval;
}

/*-------------------------------------------------------------------------*/

/**
 *      Identical to pafgetc()
 */
Int32
pagetc( PAFILE *pafile )
{
        return pafgetc( pafile );
}

/*-------------------------------------------------------------------------*/

/**
 *      Check if the filepointer has reached the end of the file
 *      if so returns true
 */
Int32
pafeof( PAFILE *pafile )
{
        Int32 retval;
        if ( !pafile->control ) return 0;

        MutexLock( pafile->control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        if ( OFFSETCHANGED( pafile ) ) { fseek( pafile->control->fptr, (long)( pafile->coffset + pafile->offset ), SEEK_SET ); }
        retval = feof( pafile->control->fptr );
        MutexUnlock( pafile->control->controllock );

        return retval;
}

/*-------------------------------------------------------------------------*/

/**
 *      Binary read the maximum number of elements of esize each
 *      from current position
 */
UInt32
pafread( void *dest      ,
         UInt32 esize    ,
         UInt32 elements ,
         PAFILE *pafile  )
{
        UInt32 retval;
        if ( !pafile->control ) return 0;

        MutexLock( pafile->control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        if ( ( pafile->coffset + pafile->offset != pafile->control->offset ) )
        {
                 fseek( pafile->control->fptr, (long)( pafile->coffset + pafile->offset ), SEEK_SET );
        }
    //    tsprintf( "pafile->coffset=%d  pafile->offset=%d  pafile->control->offset=%d\n", pafile->coffset, pafile->offset, pafile->control->offset );
//        if ( OFFSETCHANGED( pafile ) ) { fseek( pafile->control->fptr, pafile->coffset + pafile->offset, SEEK_SET ); }
        retval = (UInt32) fread( dest, esize, elements, pafile->control->fptr );
        MutexUnlock( pafile->control->controllock );

        return retval;        
}

/*-------------------------------------------------------------------------*/

/**
 *      Use ASCII : read a line. meaning read untill \n
 *      from current position. Note that this function allocates
 *      memory that should be freed using pafree()
 */
UInt32
pafreadl( char **dest    ,
          PAFILE *pafile )
{
        UInt32 retval;
        UInt64 len, pos;
        char c;
        if ( !pafile->control ) return 0;

        MutexLock( pafile->control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        if ( OFFSETCHANGED( pafile ) ) { fseek( pafile->control->fptr, (long)( pafile->coffset + pafile->offset ), SEEK_SET ); }

        c = 1;
        pos = pafile->offset;
        while ( ( !feof( pafile->control->fptr ) ) &&
                (  c != '\n' ) && ( c != '\0' ) && ( c != '\r' ) )
        {
                c = (char) fgetc( pafile->control->fptr );
        }
        len = ftell( pafile->control->fptr ) - pos + 1;
        if ( len > 1 )
        {
                fseek( pafile->control->fptr, pos, SEEK_SET );
                *dest = calloc( len, 1 );
                fread( *dest, len-1, 1, pafile->control->fptr );
                retval = len;
        }
        else
        {
                *dest = NULL;
                retval = 0;
        }

        MutexUnlock( pafile->control->controllock );

        return retval;
}

/*-------------------------------------------------------------------------*/

/**
 *      Use ASCII : read a string. meaning read untill
 *      a whitespace (blanks, TABs, CRs, LFs),
 *      works from current position. Note that this function allocates
 *      memory that should be freed using pafree()
 */
UInt32
pafreads( char **dest    ,
          PAFILE *pafile )
{
        UInt32 retval, len, pos;
        char c;
        
        if ( !pafile->control ) return 0;

        MutexLock( pafile->control->controllock, GUCEF_MUTEX_INFINITE_TIMEOUT );
        if ( OFFSETCHANGED( pafile ) ) { fseek( pafile->control->fptr, pafile->coffset + pafile->offset, SEEK_SET ); }

        c = 1;
        pos = pafile->offset;
        while ( ( !feof( pafile->control->fptr ) ) &&
                (  c != '\n' ) && ( c != '\0' ) && ( c != '\r' ) && ( c != '\t' ) && ( c != ' ' ) )
        {
                c = (char) fgetc( pafile->control->fptr );
        }
        len = ftell( pafile->control->fptr ) - pos + 1;
        if ( len > 1 )
        {
                fseek( pafile->control->fptr, pos, SEEK_SET );
                *dest = calloc( len, 1 );
                fread( *dest, len-1, 1, pafile->control->fptr );
                retval = len;
        }
        else
        {
                *dest = NULL;
                retval = 0;
        }

        MutexUnlock( pafile->control->controllock );

        return retval;         
}

/*-------------------------------------------------------------------------*/

/**
 *      Free a block of memory that was allocated using pafreadl() or
 *      pafreads()
 */
void
pafree( void *mem )
{
        free( mem );
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

/*-------------------------------------------------------------------------*/
