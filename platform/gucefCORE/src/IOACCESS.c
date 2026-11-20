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

#include <stdio.h>         /* needed for the fseek() enums */ 
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>     /* memory management */
#endif

#include "gucefCORE_c_ioaccess.h"   /* function prototypes and types we need */

#ifdef IOACCESS_MFILE_SUPPORT
#ifndef GUCEF_CORE_MFILE_H
#include "MFILE.h"      /* in-memory file handling */
#endif /* GUCEF_CORE_MFILE_H ? */
#endif /* IOACCESS_MFILE_SUPPORT ? */

#ifdef IOACCESS_PAFILE_SUPPORT
#ifndef GUCEF_CORE_PAFILE_H
#include "PAFILE.h"      /* On disk parallell access */
#endif /* GUCEF_CORE_PAFILE_H ? */
#endif /* IOACCESS_PAFILE_SUPPORT ? */

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

struct SIOAccessPrivateData
{
        void *source;            /* data used by the actual I/O functions */  
        TIOType type;            /* type of the I/O source */
};

typedef struct SIOAccessPrivateData TIOAccessPrivateData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef IOACCESS_PAFILE_SUPPORT
UInt32
ioa_pafreads( TIOAccess *access, char **dest )
{
        return pafreads( dest, (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_pafreadl( TIOAccess *access, char **dest )
{
        return pafreadl( dest, (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_pafread( TIOAccess *access, void *dest, UInt32 esize, UInt32 elements )
{
        return pafread( dest, esize, elements, (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_pafopen( TIOAccess *access )
{
        return pafsetpos( (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source, 0 );
}

/*-------------------------------------------------------------------------*/

void
ioa_pafclose( TIOAccess *access )
{
        pafsetpos( (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source, 0 );
}

/*-------------------------------------------------------------------------*/

UInt64
ioa_paftell( TIOAccess *access )
{
        return paftell( (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_pafseek( TIOAccess *access, Int64 offset, Int32 origin )
{
        return pafseek( (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source, offset, origin );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_pafsetpos( TIOAccess *access, UInt64 offset )
{
        return pafseek( (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source, offset, SEEK_SET );
}

/*-------------------------------------------------------------------------*/

Int32
ioa_pagetc( TIOAccess *access )
{
        return pafgetc( (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_pafeof( TIOAccess *access )
{
        return pafeof( (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_pafopened( TIOAccess *access )
{
        return (PAFILE*)((TIOAccessPrivateData*)access->privdata)->source > 0;
}

#endif /* IOACCESS_PAFILE_SUPPORT ? */

/*-------------------------------------------------------------------------*/

#ifdef IOACCESS_MFILE_SUPPORT

UInt32
ioa_mfopen( TIOAccess *access )
{
        mfsetpos( (MFILE*)((TIOAccessPrivateData*)access->privdata)->source, 0UL );
        return 1;
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_mfopened( TIOAccess *access )
{
        return (MFILE*)((TIOAccessPrivateData*)access->privdata)->source > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_mfclose( TIOAccess *access )
{
        mfsetpos( (MFILE*)((TIOAccessPrivateData*)access->privdata)->source, 0UL );
        return 1;
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_mfreads( TIOAccess *access, char **dest )
{
        return mfreads( dest, (MFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_mfreadl( TIOAccess *access, char **dest )
{
        return mfreadl( dest, (MFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_mfread( TIOAccess *access, void *dest, UInt32 esize, UInt32 elements )
{
        return mfread( dest, esize, elements, (MFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/
UInt64
ioa_mftell( TIOAccess *access )
{
        return mftell( (MFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_mfseek( TIOAccess *access, Int32 offset, Int32 origin )
{
        return mfseek( (MFILE*)((TIOAccessPrivateData*)access->privdata)->source, offset, origin );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_mfsetpos( TIOAccess *access, UInt32 offset )
{
        return mfseek( (MFILE*)((TIOAccessPrivateData*)access->privdata)->source, offset, SEEK_SET );
}

/*-------------------------------------------------------------------------*/

Int32
ioa_mgetc( TIOAccess *access )
{
        return mfgetc( (MFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_mfeof( TIOAccess *access )
{
        return mfeof( (MFILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

#endif /* IOACCESS_MFILE_SUPPORT ? */

/*-------------------------------------------------------------------------*/

#ifdef IOACCESS_FILE_SUPPORT

UInt32
ioa_fopen( TIOAccess *access )
{
        rewind( (FILE*)((TIOAccessPrivateData*)access->privdata)->source );
        return 1;
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_fopened( TIOAccess *access )
{
        return (FILE*)((TIOAccessPrivateData*)access->privdata)->source > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_fclose( TIOAccess *access )
{
        rewind( (FILE*)((TIOAccessPrivateData*)access->privdata)->source );
        return 1;
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_freads( TIOAccess *access, char **dest )
{
        /*
         *      Read untill a stop char and alloc and fill dest char buffer
         */
        int i = 0, c;
        FILE *fptr = (FILE*)((TIOAccessPrivateData*)access->privdata)->source;
        UInt32 rbytes, fpos = ftell( fptr );

        do
        {
                c = fgetc( fptr );
                ++i;
        } while ( ( c != ' ' ) && ( c != '\n' ) && ( c != '\r' ) && ( c != '\t' ) && ( c != EOF ) );

        /*
         *      Alloc and obtain string
         */
        *dest = ( char* ) calloc( sizeof( char ), i );
        *dest[ i-1 ] = '\0';
        fseek( fptr, fpos, SEEK_SET );
        rbytes = (UInt32)fread( *dest, sizeof( char ), i-1, fptr );
        return rbytes;
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_freadl( TIOAccess *access, char **dest )
{
        /*
         *      Read untill a stop char and alloc and fill dest char buffer
         */
        int i = 0, c;
        FILE *fptr = (FILE*)((TIOAccessPrivateData*)access->privdata)->source;
        UInt32 rbytes, fpos = ftell( fptr );

        do
        {
                c = fgetc( fptr );
                ++i;
        } while ( ( c != '\n' ) && ( c != '\r' ) && ( c != EOF ) );

        /*
         *      Alloc and obtain string
         */
        *dest = ( char* ) calloc( sizeof( char ), i );
        *dest[ i-1 ] = '\0';
        fseek( fptr, fpos, SEEK_SET );
        rbytes = (UInt32)fread( *dest, sizeof( char ), i-1, fptr );
        return rbytes;
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_fread( TIOAccess *access, void *dest, UInt32 esize, UInt32 elements )
{
        return (UInt32)fread( dest, esize, elements, (FILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/
UInt32
ioa_ftell( TIOAccess *access )
{
        return ftell( (FILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_fseek( TIOAccess *access, Int32 offset, Int32 origin )
{
        return fseek( (FILE*)((TIOAccessPrivateData*)access->privdata)->source, offset, origin );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_fsetpos( TIOAccess *access, UInt32 offset )
{
        return fseek( (FILE*)((TIOAccessPrivateData*)access->privdata)->source, offset, SEEK_SET );
}

/*-------------------------------------------------------------------------*/

Int32
ioa_fgetc( TIOAccess *access )
{
        return fgetc( (FILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

/*-------------------------------------------------------------------------*/

UInt32
ioa_feof( TIOAccess *access )
{
        return feof( (FILE*)((TIOAccessPrivateData*)access->privdata)->source );
}

#endif /* IOACCESS_FILE_SUPPORT ? */

/*-------------------------------------------------------------------------*/

/**
 *      Creates an I/O access point from the given source.
 *      source should be opened for reading. The type of source should be
 *      specified in type.
 */
TIOAccess*
IOA_Create_IOAccess( void *source, TIOType type )
{
        /*
         *      FILE* is not an acceptable input source type.
         */
        TIOAccess *access;
        #ifdef IOACCESS_FILE_SUPPORT
        if ( type == IOACCESS_FILE ) return NULL;

        /*
         *      Check if we are dealing with a filepath and if so attempt to
         *      get a FILE* In all other cases we simply copy the data.
         */
        if ( type == IOACCESS_FILEPATH )
        {
                FILE *fptr = fopen( ( const char* ) source, "rb" );
                if ( !fptr ) return NULL;

                access = ( TIOAccess* ) malloc( sizeof( TIOAccess ) );
                access->privdata = malloc( sizeof( TIOAccessPrivateData ) );
                ((TIOAccessPrivateData*)access->privdata)->source = fptr;
                ((TIOAccessPrivateData*)access->privdata)->type = IOACCESS_FILE;
        }
        else
        #endif /* IOACCESS_FILE_SUPPORT ? */
        {
                access = ( TIOAccess* ) malloc( sizeof( TIOAccess ) );
                access->privdata = malloc( sizeof( TIOAccessPrivateData ) );
                ((TIOAccessPrivateData*)access->privdata)->source = source;
                ((TIOAccessPrivateData*)access->privdata)->type = type;
        }

        /*
         *      Now we link the wrapper functions to the correct implementation.
         */
        switch ( type )
        {
                #ifdef IOACCESS_PAFILE_SUPPORT
                case IOACCESS_PAFILE :
                {
                        access->open    = (TIOAccessfunction_open)   &ioa_pafopen;
                        access->opened  = (TIOAccessfunction_opened) &ioa_pafopened;
                        access->close   = (TIOAccessfunction_open)   &ioa_pafclose;
                        access->readl   = (TIOAccessfunction_readl)  &ioa_pafreadl;
                        access->reads   = (TIOAccessfunction_reads)  &ioa_pafreads;
                        access->read    = (TIOAccessfunction_read)   &ioa_pafread;
                        access->tell    = (TIOAccessfunction_tell)   &ioa_paftell;
                        access->seek    = (TIOAccessfunction_seek)   &ioa_pafseek;
                        access->setpos  = (TIOAccessfunction_setpos) &ioa_pafsetpos;
                        access->getc    = (TIOAccessfunction_getc)   &ioa_pagetc;
                        access->eof     = (TIOAccessfunction_eof)    &ioa_pafeof;
                        access->memfree = (TIOAccessfunction_free)   &pafree;

                        return access;
                }
                #endif /* IOACCESS_PAFILE_SUPPORT ? */
                #ifdef IOACCESS_MFILE_SUPPORT
                case IOACCESS_MFILE :
                {
                        access->open    = (TIOAccessfunction_open)   &ioa_mfopen;
                        access->opened  = (TIOAccessfunction_opened) &ioa_mfopened;
                        access->close   = (TIOAccessfunction_open)   &ioa_mfclose;
                        access->readl   = (TIOAccessfunction_readl)  &ioa_mfclose;
                        access->reads   = (TIOAccessfunction_reads)  &ioa_mfreads;
                        access->read    = (TIOAccessfunction_read)   &ioa_mfread;
                        access->tell    = (TIOAccessfunction_tell)   &ioa_mftell;
                        access->seek    = (TIOAccessfunction_seek)   &ioa_mfseek;
                        access->setpos  = (TIOAccessfunction_setpos) &ioa_mfsetpos;
                        access->getc    = (TIOAccessfunction_getc)   &ioa_mgetc;
                        access->eof     = (TIOAccessfunction_eof)    &ioa_mfeof;
                        access->memfree = (TIOAccessfunction_free)   &mfree;

                        return access;
                }
                #endif /* IOACCESS_MFILE_SUPPORT ? */
                #ifdef IOACCESS_FILE_SUPPORT
                case IOACCESS_FILEPATH :
                {
                        access->open    = (TIOAccessfunction_open)   &ioa_fopen;
                        access->opened  = (TIOAccessfunction_opened) &ioa_fopened;
                        access->close   = (TIOAccessfunction_open)   &ioa_fclose;
                        access->readl   = (TIOAccessfunction_readl)  &ioa_fclose;
                        access->reads   = (TIOAccessfunction_reads)  &ioa_freads;
                        access->read    = (TIOAccessfunction_read)   &ioa_fread;
                        access->tell    = (TIOAccessfunction_tell)   &ioa_ftell;
                        access->seek    = (TIOAccessfunction_seek)   &ioa_fseek;
                        access->setpos  = (TIOAccessfunction_setpos) &ioa_fsetpos;
                        access->getc    = (TIOAccessfunction_getc)   &ioa_fgetc;
                        access->eof     = (TIOAccessfunction_eof)    &ioa_feof;
                        access->memfree = (TIOAccessfunction_free)   &free;

                        return access;                   
                }
                #endif /* IOACCESS_FILE_SUPPORT ? */
                default :
                {
                        free( access );
                        return NULL;
                }
        }
      /*  return NULL; */
}

/*-------------------------------------------------------------------------*/

/**
 *      De-allocate storage created with create_ioaccess().
 */
void
IOA_Destroy_IOAccess( TIOAccess *access )
{
        #ifdef IOACCESS_FILE_SUPPORT
        if ( ((TIOAccessPrivateData*)access->privdata)->type == IOACCESS_FILE )
        {
                fclose( (FILE*)(((TIOAccessPrivateData*)access->privdata)->source) );
        }
        #endif /* IOACCESS_FILE_SUPPORT ? */
        free( (TIOAccessPrivateData*)access->privdata );
        free( access );  
}

/*-------------------------------------------------------------------------*/

/**
 *      This function is meant to be used to access the original source
 *      pass to IOA_Create_IOAccess() for cleanup purposes before calling
 *      IOA_Destroy_IOAccess(). It should only be used at the same location
 *      as where the access point was originally created and only if you
 *      no longer have a pointer to whatever was provided as source in
 *      IOA_Create_IOAccess() yourself. This function should not be used in
 *      combination with the IOACCESS_FILEPATH source type because the IOA
 *      functions will manage the cleanup.
 */
void* 
IOA_Source( TIOAccess *access )
{
        return ( (TIOAccessPrivateData*)access->privdata )->source;        
}

/*-------------------------------------------------------------------------*/

/**
 *      Returns the type of recource this access point current wraps.
 *      The type returned may differ from that which was passed to
 *      IOA_Create_IOAccess() due to internal conversions. 
 *
 */
TIOType
IOA_Type( TIOAccess *access )
{
        return ( (TIOAccessPrivateData*)access->privdata )->type;
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
