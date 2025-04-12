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

#ifndef GUCEF_CORE_C_IOACCESS_H
#define GUCEF_CORE_C_IOACCESS_H

/*
 *      (@file) ioaccess.h
 *
 *      The following is a toolkit to allow users to access a resource and
 *      preform I/O on it without actually knowing where the resource is stored.
 *      It allows your code to use a single set of functions instead of making
 *      versions for each type of I/O operation. All the utility's are provided
 *      to keep memory management in scope. It is safe to pass a TIOAccess*
 *      across DLL boundries and use it inside the DLL.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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
//      BUILD CONFIG                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Build switches.
 *      Used to disable/enable support for specific source types thus
 *      controlling dependencies.
 */
#define IOACCESS_PAFILE_SUPPORT
#define IOACCESS_MFILE_SUPPORT
#define IOACCESS_FILE_SUPPORT

#if !defined( IOACCESS_PAFILE_SUPPORT ) && !defined( IOACCESS_MFILE_SUPPORT ) && !defined( IOACCESS_FILE_SUPPORT )
#error You have to select at least one supported source type
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      enumeration values identifying possible types of I/O sources.
 */
typedef enum
{
        IOACCESS_UNSUPPORTED = 0,
        #ifdef IOACCESS_MFILE_SUPPORT
        IOACCESS_MFILE,         /* MFILE* of a file stored in memory */
        #endif /* IOACCESS_MFILE_SUPPORT ? */
        #ifdef IOACCESS_PAFILE_SUPPORT
        IOACCESS_PAFILE,        /* PAFILE* of a file stored on disk */
        #endif /* IOACCESS_PAFILE_SUPPORT ? */
        #ifdef IOACCESS_FILE_SUPPORT
        IOACCESS_FILEPATH,      /* char* filename & path to a file stored on disk */
        IOACCESS_FILE           /* FILE* this becomes the type if IOACCESS_FILEPATH was the source */
        #endif /* IOACCESS_FILE_SUPPORT ? */
} TIOType;

/*-------------------------------------------------------------------------*/

/**
 *      Forward declaration required of the I/O access data structure to
 *      avoid mutual calling.
 */
struct SIOAccess;

/*-------------------------------------------------------------------------*/

typedef UInt32 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_open ) ( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_opened ) ( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_close ) ( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_write ) ( struct SIOAccess* access, const void* srcData, UInt32 esize, UInt32 elements ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_readl ) ( struct SIOAccess* access, char **dest ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_reads ) ( struct SIOAccess* access, char **dest ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_read ) ( struct SIOAccess* access, void *dest, UInt32 esize, UInt32 elements ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt64 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_tell ) ( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX;
typedef Int32  ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_seek ) ( struct SIOAccess* access, Int64 offset, Int32 origin ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_setpos ) ( struct SIOAccess* access, UInt64 pos ) GUCEF_CALLSPEC_SUFFIX;
typedef Int32  ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_getc ) ( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX;
typedef Int32  ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_eof ) ( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX;
typedef UInt64 ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_size ) ( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX;
typedef void   ( GUCEF_CALLSPEC_PREFIX *TIOAccessfunction_free ) ( void* mem ) GUCEF_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/**
 *      Storage for our recource access point. Most of the functions operate
 *      like the standard f functions. for example read() will operate like
 *      fread() even though it may be reading from an entirely different
 *      source then a file on disk. Users should treat this data strucure in
 *      a read-only fashion at all times. Do NOT copy the data stored in the
 *      structure in any way !!!
 */
struct SIOAccess
{
    TIOAccessfunction_open   open;    /* open the resource for I/O */
    TIOAccessfunction_close  close;   /* close the recource */
    TIOAccessfunction_write  write;   /* writes to to the output medium */
    TIOAccessfunction_opened opened;  /* is the recource opened for reading ? */
    TIOAccessfunction_readl  readl;   /* read a line of text (allocates mem)*/
    TIOAccessfunction_reads  reads;   /* read a string (allocates mem)*/
    TIOAccessfunction_read   read;    /* read a number of bytes */
    TIOAccessfunction_tell   tell;    /* get the current offset in bytes */
    TIOAccessfunction_seek   seek;    /* move to a different part */
    TIOAccessfunction_setpos setpos;  /* move to a different part */
    TIOAccessfunction_getc   getc;    /* get a character */
    TIOAccessfunction_eof    eof;     /* are we at the end of the recource */
    TIOAccessfunction_size   size;    /* attempts to obtain the overall size of the resource */
    TIOAccessfunction_free   memfree; /* free memory allocated with one of the other functions here */

    void *privdata;                   /* internally used data, Don't touch !!! */
};

typedef struct SIOAccess TIOAccess;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/**
 *      Creates an I/O access point from the given source.
 *      source access should be thread-safe. The type of source should be
 *      specified in type. Should be used in combination with
 *      IOA_Destroy_IOAccess()
 */
GUCEF_CORE_PUBLIC_C TIOAccess*
IOA_Create_IOAccess( void *source, TIOType type );

/*-------------------------------------------------------------------------*/

/**
 *      De-allocate storage created with IOA_Create_IOAccess().
 */
GUCEF_CORE_PUBLIC_C void
IOA_Destroy_IOAccess( TIOAccess *access );

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
GUCEF_CORE_PUBLIC_C void*
IOA_Source( TIOAccess *access );

/*-------------------------------------------------------------------------*/

/**
 *      Returns the type of recource this access point current wraps.
 *      The type returned may differ from that which was passed to
 *      IOA_Create_IOAccess() due to internal conversions.
 *
 */
GUCEF_CORE_PUBLIC_C TIOType
IOA_Type( TIOAccess *access );

/*-------------------------------------------------------------------------*/

/*
 *      End the C++ name mangling protection
 */
#ifdef __cplusplus
};
#endif

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

#endif /* GUCEF_CORE_C_IOACCESS_H ? */
