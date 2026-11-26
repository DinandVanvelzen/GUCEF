/*
 *  DVPACKTOOL Application for handling DVP files
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

#include <stdlib.h>     /* standard utils */
#include <stdio.h>      /* standard I/O */
#include <string.h>     /* standard string utils */
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>     /* Memory management */
#endif
#include <math.h>       /* for key calcs */

#include "DVPACKSYS.h"  /* this files header */

#if !DVP_READ_ONLY_BUILD
  #include "dvstrutils.h" /* My own string utils */
  #include "dvfileutils.h"/* my own file utils */
  #include "tsprinting.h" /* threadsafe printing */
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACRO'D INCLUDES                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>		/* WIN32 API */
  #define MAX_DIR_LENGTH MAX_PATH
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )
  #include <limits.h>
  #include <unistd.h>             /* POSIX utilities */
  #define MAX_DIR_LENGTH PATH_MAX
#else
  #error Unsupported OS
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt32 DVP_Filesize( FILE* fptr );

/*---------------------------------------------------------------------------*/

FILE*
DVP_Open_File( const char *filename, const char *mode )
{
	/*
         *	Open file and return file pointer
         */
        return fopen( filename, mode );
}

/*---------------------------------------------------------------------------*/

void
DVP_Close_File( FILE *fptr )
{
	/*
         *	Close the file
         */
        fclose( fptr );
}

/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

/*
 *      Extract the index of the archive into an external file
 */
UInt32
DVP_IOA_Extract_Index_File( TIOAccess *access   ,
                            const char *hfout   ,
                            UInt32 index_offset ,
                            UInt32 num_entrys   )
{
        FILE   *out_file;
        char   tmp_data[ DVP_INDEX_ENTRY_SIZE  ];
        UInt32 i;
        UInt32 total = 0;

/*
        #if defined( DEBUG_MODE )
        tsprintf( "DVP_IOA_Extract_Index_File( %d, %s, %d, %d )\n", (UInt32)access, hfout, index_offset, num_entrys );
        #endif    */

        /*
         *      set output file
         */
        out_file = fopen( hfout , "wb" );
        if ( !out_file )
        {
                return 1;
        }

        /*
         *      Copy index from pack file into seperate file
         */
        access->seek( access , index_offset , SEEK_SET );
        for ( i=0; i < num_entrys; i++ )
        {
                total += access->read( access, tmp_data , 1 , DVP_INDEX_ENTRY_SIZE );
                fwrite( tmp_data , 1 , DVP_INDEX_ENTRY_SIZE, out_file );
        }
        fclose( out_file );

        if ( total != num_entrys*DVP_INDEX_ENTRY_SIZE ) return 1;

        return 0;
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Extract_Index_File( FILE *dfile         ,
                        const char *hfout   ,
                        UInt32 index_offset ,
                        UInt32 num_entrys   )
{
        FILE   *out_file;
        char   tmp_data[ DVP_INDEX_ENTRY_SIZE  ];
        UInt32 i;
        UInt32 total = 0;

     /*   #if defined( DEBUG_MODE )
        tsprintf( "DVP_Extract_Index_File( %d, %s, %d, %d )\n", (UInt32)dfile, hfout, index_offset, num_entrys );
        #endif */

        /*
         *      set output file
         */
        out_file = fopen( hfout , "wb" );
        if ( !out_file )
        {
                return 1;
        }

        /*
         *      Copy index from pack file into seperate file
         */
        fseek( dfile , index_offset , SEEK_SET );
        for ( i=0; i < num_entrys; i++ )
        {
                total += (UInt32)fread( tmp_data , 1 , DVP_INDEX_ENTRY_SIZE, dfile );
                fwrite( tmp_data , 1 , DVP_INDEX_ENTRY_SIZE, out_file );
        }
        fclose( out_file );

        if ( total != num_entrys*DVP_INDEX_ENTRY_SIZE ) return 1;

        return 0;
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

/**
 *      Verion of DVP_Extract_Index_File() for lazy users. obtains the 2 other
 *      parameters for DVP_Extract_Index_File() itself.
 */
UInt32
_DVP_IOA_Extract_Index_File( TIOAccess *access    ,
                             const char *filename )
{
        TDVP_File_Header header;

        /*
         *      Obtain header data
         */
        _DVP_IOA_Read_Header( access, &header );

        /*
         *      Call normal version
         */
        return DVP_IOA_Extract_Index_File( access, filename, header.index_offset, header.num_files+(header.num_dirs*2) );
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

/**
 *      Verion of DVP_Extract_Index_File() for lazy users. obtains the 2 other
 *      parameters for DVP_Extract_Index_File() itself.
 */
UInt32
_DVP_Extract_Index_File( FILE *dfile          ,
                         const char *filename )
{
        TDVP_File_Header header;

        /*
         *      Obtain header data
         */
        _DVP_Read_Header( dfile, &header );

        /*
         *      Call normal version
         */
        return DVP_Extract_Index_File( dfile, filename, header.index_offset, header.num_files+(header.num_dirs*2) );
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Strip_Index( const char *archive, const char *new_archive )
{
        /*
         *      Create new archive that does not have an index at the end.
         *      Might be usefull if you perhaps want to hardcode an index
         *      in memory preventing external reading from archive even if key
         *      is correct.
         */
        UInt32 index_offset;
        UInt32 num_files;
        UInt32 num_dirs;
        UInt32 nbytes;
        UInt32 lastchange;
        UInt32 version;
        void* data;
        char ID[ DVP_ID_LENGTH+1 ];
        FILE *fptr, *nfptr;

    /*    #if defined( DEBUG_MODE )
        tsprintf( "DVP_Strip_Index( %s, %s )\n", archive, new_archive );
        #endif */

        /*
         *      Attempt to open archive
         */
        fptr = fopen( archive, "rb" );
        if ( !fptr )
        {
                return 1;
        }

        /*
         *      Try to obtain header offset
         */
        ID[ DVP_ID_LENGTH ] = 0;
        if ( DVP_Read_Header(  fptr          ,
                               ID            ,
                               &version      ,
                               &index_offset ,
                               &num_files    ,
                               &num_dirs     ,
                               &lastchange   ) )
        {
                fclose( fptr );
                return 1;
        }

        /*
         *      Attempt to create new archive
         */
        nfptr = fopen( new_archive, "wb" );
        if ( !nfptr )
        {
                return 1;
        }

        /*
         *      Do the stripping,
         *      which is simply copying the data up to the index into a new file
         */
        data = malloc( index_offset );
        if ( !data ) return 1;
        nbytes = (UInt32)fread( data, 1, index_offset, fptr );
        fwrite( data, 1, index_offset, nfptr );
        fclose( fptr );
        fclose( nfptr );

        return !( nbytes == index_offset );
}

#endif
/*---------------------------------------------------------------------------*/

UInt32
DVP_Read_Header( FILE  *fp                ,
                 char  *ID                ,
                 UInt32   *version 	  ,
                 UInt32   *index_offset   ,
                 UInt32   *num_files	  ,
                 UInt32   *num_dirs	  ,
                 UInt32  *lastchange      )
{
        UInt32 checksum, pos;

     /*   #if defined( DEBUG_MODE )
        tsprintf( "DVP_Read_Header( %d, NA, NA, NA, NA, NA, NA )\n", (UInt32)fp );
        #endif */

        pos = ftell( fp );
        fseek( fp, 0, SEEK_SET );

        /*
         *      Initialize ID
         */
        memset( ID, 0, DVP_ID_LENGTH );

        /*
         *      Read header data
         */
        fread( ID , 1 , DVP_ID_LENGTH, fp );
        fread( version , 1 , 4,  fp );
        fread( index_offset , 1, 4 , fp );
        fread( num_files , 1 , 4, fp );
        fread( num_dirs , 1 , 4, fp );
        fread( lastchange , 1 , 4, fp );
        fread( &checksum , 1 , 4 , fp );

   /*     #if defined( DEBUG_MODE ) && defined( BUILD_DLL )
        tsprintf( "Header data: ID %s, version %d, index_offset %d, num_files %d, num_dirs %d, lastchange %d, checksum %d\n", ID, *version, *index_offset, *num_files, *num_dirs, *lastchange, checksum );
        #endif */

        /*
         *      Place file pointer back
         */
        fseek( fp, pos, SEEK_SET );

        /*
         *      Check checksum to make sure we have a valid pack file
         */
        if ( checksum == DVP_CHECKSUM )
        {
                return 0;
        }
        else
        {
                return 1;
        }
}

/*---------------------------------------------------------------------------*/

UInt32
DVP_IOA_Read_Header( TIOAccess *access      ,
                     char  *ID              ,
                     UInt32   *version      ,
                     UInt32   *index_offset ,
                     UInt32   *num_files    ,
                     UInt32   *num_dirs     ,
                     UInt32  *lastchange    )
{
        UInt32 checksum, pos;

    /*    #if defined( DEBUG_MODE )
        tsprintf( "DVP_IOA_Read_Header( %d, NA, NA, NA, NA, NA, NA )\n", (UInt32)access );
        #endif */

        pos = access->tell( access );
        access->seek( access, 0, SEEK_SET );

        /*
         *      Initialize ID
         */
        memset( ID, 0, DVP_ID_LENGTH );

        /*
         *      Read header data
         */
        access->read( access, ID , 1 , DVP_ID_LENGTH );
        access->read( access, version , 1 , 4 );
        access->read( access, index_offset , 1, 4 );
        access->read( access, num_files , 1 , 4 );
        access->read( access, num_dirs , 1 , 4 );
        access->read( access, lastchange , 1 , 4 );
        access->read( access, &checksum , 1 , 4 );

    /*    #if defined( DEBUG_MODE )
        tsprintf( "Header data:\n ID %s\n version %d\n index_offset %d\n num_files %d\n num_dirs %d\n lastchange %d\n checksum %d\n", ID, *version, *index_offset, *num_files, *num_dirs, *lastchange, checksum );
        #endif */

        /*
         *      Place file pointer back
         */
        access->seek( access, pos, SEEK_SET );

        /*
         *      Check checksum to make sure we have a valid pack file
         */
        if ( checksum == DVP_CHECKSUM )
        {
                return 0;
        }
        else
        {
                return 1;
        }
}

/*---------------------------------------------------------------------------*/

/**
 *      Read the archive header data
 *      The position of the file pointer will be reset to the original position
 *      after completion. The file fp should be open in rb mode.
 */
UInt32
_DVP_IOA_Read_Header( TIOAccess *access        ,
                      TDVP_File_Header *header )
{
      /*  #if defined( DEBUG_MODE )
        tsprintf( "_DVP_IOA_Read_Header( %d, %d )\n", (UInt32)access, (UInt32)header );
        #endif */

        return DVP_IOA_Read_Header( access, header->ID, &header->version, &header->index_offset, &header->num_files, &header->num_dirs, &header->lastchange );
}

/*---------------------------------------------------------------------------*/

/*
 *      Read the archive header data
 *      The position of the file pointer will be reset to the original position
 *      after completion. The file fp should be open in rb mode.
 */
UInt32
_DVP_Read_Header( FILE   *fp               ,
                  TDVP_File_Header *header )
{
      /*  #if defined( DEBUG_MODE )
        tsprintf( "_DVP_Read_Header( %d, %d )\n", (UInt32)fp, (UInt32)header );
        #endif */

        return DVP_Read_Header( fp, header->ID, &header->version, &header->index_offset, &header->num_files, &header->num_dirs, &header->lastchange );
}

/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Write_Header( FILE *fp_out 	        ,
	          const char *ID        ,
                  UInt32 index_offset   ,
	          UInt32 num_of_files   ,
                  UInt32 num_of_dirs    )
{
        UInt32  tmp, pos                        ;
        char id[ DVP_ID_LENGTH+1 ]        ;

/*	#if defined( DEBUG_MODE )
        tsprintf( "DVP_Write_Header( %d, %s, %d, %d, %d )\n", (UInt32) fp_out, ID, index_offset, num_of_files, num_of_dirs );
        #endif  */

	/*
	 *	Set filepointer for header position
	 */
        pos = ftell( fp_out );
	fseek(fp_out, 0, SEEK_SET)                                      ;

	/*
	 *	Write header data
	 */
        memset( id, DVP_ID_LENGTH, 0 )                                  ;
        strcpy( id, ID )                                                ;
        fwrite( id, 1, DVP_ID_LENGTH, fp_out )                            ;
        tmp = (DVP_MAJOR_VERSION*1000)+(DVP_MINOR_VERSION*100)+(DVP_RELEASE_VERSION*10)+DVP_PATCH_VERSION;
        fwrite( &tmp, 1, 4, fp_out );
        fwrite( &index_offset, 1, 4, fp_out );
        fwrite( &num_of_files, 1, 4, fp_out );
        fwrite( &num_of_dirs, 1, 4, fp_out );
        // TODO : write current datetime
        tmp = 0;
        fwrite( &tmp, 1, 4, fp_out );
        tmp = DVP_CHECKSUM;
        fwrite( &tmp, 1, 4, fp_out );

        fseek( fp_out, pos, SEEK_SET)                                      ;

	return 0;
}

#endif
/*---------------------------------------------------------------------------*/

/**
 *      Note that this reads an entire file into a buffer, so
 *      enough physical memory will
 *      have to be available for this operation.
 *
 *      fpt is a filepointer to the datafile
 *      offset the offset of the file in the datafile
 *      filesize the size of the file we are loading
 *      fbuffer is a pionter to a char buffer for the data out of the
 *      file rbytes returns the number of bytes read.
 */
UInt32
DVP_IOA_Load_File_Into_Mem( TIOAccess *access ,
                            UInt32 offset     ,
                            UInt32 filesize   ,
                            UInt32 *rbytes    ,
                            void *dest        )
{


     /*   #if defined( DEBUG_MODE )
        tsprintf( "DVP_IOA_Load_File_Into_Mem( %d, %d, %d, NA, NA )\n", (UInt32) access, offset, filesize );
        #endif */

        /*
         *      Initialize rbytes in case of error
         */
        *rbytes = 0;

        /*
         *      set datafile filepointer
         */
        access->seek( access, offset , SEEK_SET );

        /*
         *      Read the file data block into memory
         */
        *rbytes = access->read( access, dest, 1, filesize );

        return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *      Note that this reads an entire file into a buffer, so
 *      enough physical memory will
 *      have to be available for this operation.
 *
 *      fpt is a filepointer to the datafile
 *      offset the offset of the file in the datafile
 *      filesize the size of the file we are loading
 *      fbuffer is a pionter to a char buffer for the data out of the
 *      file rbytes returns the number of bytes read.
 */
UInt32
DVP_Load_File_Into_Mem(  FILE *fpt         ,
                         UInt32 offset     ,
                         UInt32 filesize   ,
                         UInt32 *rbytes    ,
                         void *dest        )
{
      /*  #if defined( DEBUG_MODE )
        tsprintf( "DVP_Load_File_Into_Mem( %d, %d, %d, NA, NA )\n", (UInt32) fpt, offset, filesize );
        #endif */

        /*
         *      Initialize rbytes in case of error
         */
        *rbytes = 0;

        /*
         *      set datafile filepointer
         */
        fseek( fpt, offset , SEEK_SET );

        /*
         *      Read the file data block into memory
         */
        *rbytes = (UInt32) fread( dest, 1, filesize , fpt );

        return 0;
}

/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Write_Index_Entry( UInt32 offset , UInt32 eoffset , UInt32 size , UInt32 tstamp , const char *istr , FILE *fpt )
{
	/*
	 *	Read entry data into variables passed down as parameter
	 *	Assumes the file to be open and specified by filepointer fpt
	 */

	/* #if defined( DEBUG_MODE )
        tsprintf( "DVP_Write_Index_Entry( %d, %d, %d, %d, %s, %d )\n", offset, eoffset, size, tstamp, istr, (UInt32) fpt );
        #endif */

	fseek( fpt , offset , SEEK_SET )			        ;
        /* offset  */
	fwrite( &eoffset , 1, 4, fpt )                     ;
        /* filesize */
	fwrite( &size , 1, 4, fpt )   			;
        /* name */
	fwrite( istr , 1, DVP_MAX_INAME_LENGTH, fpt )			;
        /* timestamp */
	fwrite( &tstamp , 1, 4, fpt )		        ;

	return 0							;
}

#endif
/*---------------------------------------------------------------------------*/

UInt32
DVP_IOA_Read_Index_Entry( UInt32 offset     ,
                          UInt32 *eoffset   ,
                          UInt32 *size      ,
                          UInt32 *tstamp    ,
                          char *istr        ,
                          TIOAccess *access )
{
        /*
         *      write entry data into variables passed down as parameter
         *      Assumes the file to be open in read mode and specified by
         *      filepointer fpt
         */

    /*   #if defined( DEBUG_MODE )
        tsprintf( "DVP_IOA_Read_Index_Entry( %d, NA, NA, NA, NA, %d )\n", offset, (UInt32) access );
        #endif */

        access->seek( access, offset, SEEK_SET );
        /* offset */
        if ( 4 != access->read( access, eoffset , 1, 4 ) ) return 1;
        /* filesize */
        if ( 4 != access->read( access, size , 1, 4 ) ) return 1;
        /* name */
        memset( istr, '\0', DVP_MAX_INAME_LENGTH );
        if ( DVP_MAX_INAME_LENGTH != access->read( access, istr , 1, DVP_MAX_INAME_LENGTH ) ) return 1;
        /* timestamp */
        if ( 4 != access->read( access, tstamp , 1, 4 ) ) return 1;

      /*  #if defined( DEBUG_MODE )
        tsprintf( "Index entry data:\n offset %d\n foffset %d\n fsize %d\n ftimestamp %d\n fname %s\n TIOAccess* %d\n", offset, *eoffset, *size, *tstamp, istr, (UInt32)access );
        #endif */

        return 0;
}

/*---------------------------------------------------------------------------*/

UInt32
DVP_Read_Index_Entry( UInt32 offset , UInt32 *eoffset , UInt32 *size , UInt32 *tstamp , char *istr , FILE *fpt )
{
        /*
         *      write entry data into variables passed down as parameter
         *      Assumes the file to be open in read mode and specified by
         *      filepointer fpt
         */

     /*   #if defined( DEBUG_MODE )
        tsprintf( "DVP_Read_Index_Entry( %d, NA, NA, NA, NA, %d )\n", offset, (UInt32) fpt );
        #endif */

        fseek( fpt , offset , SEEK_SET );
        /* offset */
        if ( 4 != fread( eoffset , 1, 4, fpt ) ) return 1;
        /* filesize */
        if ( 4 != fread( size , 1, 4, fpt ) ) return 1;
        /* name */
        memset( istr, '\0', DVP_MAX_INAME_LENGTH );
        if ( DVP_MAX_INAME_LENGTH != fread( istr , 1, DVP_MAX_INAME_LENGTH, fpt ) ) return 1;
        /* timestamp */
        if ( 4 != fread( tstamp , 1, 4, fpt ) ) return 1;

    /*    #if defined( DEBUG_MODE )
        tsprintf( "Index entry data: offset %d, foffset %d, fsize %d, ftimestamp %d, fname %s, FILE* %d\n", offset, *eoffset, *size, *tstamp, istr, (UInt32)fpt );
        #endif */

        return 0;
}

/*---------------------------------------------------------------------------*/

UInt32
_DVP_IOA_Read_Index_Entry( UInt32 offset           ,
                           TDVP_Index_Entry *entry ,
                           TIOAccess *access       )
{
        /*
         *      Read a single entry out of an archive index
         */

      /*  #ifdef DEBUG_MODE
        tsprintf( "_DVP_IOA_Read_Index_Entry( %d, %d, %d )\n", offset, (UInt32)entry, (UInt32) access );
        #endif /* DEBUG_MODE ? */

        return DVP_IOA_Read_Index_Entry( offset, &entry->offset, &entry->size, &entry->timestamp, entry->name, access );
}

/*---------------------------------------------------------------------------*/

UInt32
_DVP_Read_Index_Entry( UInt32 offset                  ,
                       TDVP_Index_Entry *entry 	      ,
                       FILE *fpt                      )
{
        /*
         *      Read a single entry out of an archive index
         */

     /*   #ifdef DEBUG_MODE
        tsprintf( "_DVP_Read_Index_Entry( %d, %d, %d )\n", offset, (UInt32)entry, (UInt32) fpt );
        #endif  */

        return DVP_Read_Index_Entry( offset, &entry->offset, &entry->size, &entry->timestamp, entry->name, fpt );
}

/*---------------------------------------------------------------------------*/

/**
 *      Get entry data by name of entry
 *      if there are multiple occurences of the name the first instance
 *      will be returned. if not found 1 is returned otherwise 0.
 */
UInt32
DVP_IOA_Find_Entry_Data( TIOAccess *access       ,
                         const char *name        ,
                         TDVP_Index_Entry *entry )
{
        /*
         *      Obtain entry data by searching for name in index
         */
        UInt32 timestamp;
        UInt32 tmp2;
        char ID[ DVP_ID_LENGTH+1 ];
        char ename[ DVP_MAX_INAME_LENGTH+1 ];
        UInt32 offset;
        UInt32 eoffset;
        UInt32 size;
        UInt32 i;
        UInt32 dirs = 0;
        UInt32 files = 0;

     /*   #ifdef DEBUG_MODE
        tsprintf( "DVP_Find_Entry_Data( %d, %s, %d )\n", (UInt32)access, name, (UInt32)entry );
        #endif  */

        /*
         *      Get index offset and size
         */
        ID[ DVP_ID_LENGTH ] = 0;
        if ( DVP_IOA_Read_Header( access     ,
                                  ID         ,
                                  &tmp2      ,
                                  &offset    ,
                                  &files     ,
                                  &dirs      ,
                                  &timestamp ) )
        {
                /*
                 *      Most likely checksum problem
                 */
                return 1;
        }

        /*
         *      Search for requested entry
         */
        ename[ DVP_MAX_INAME_LENGTH ] = 0;
        for ( i=0; i<(files+dirs); i++ )
        {
                /*
                 *      Read entry
                 */
                DVP_IOA_Read_Index_Entry( offset            ,
                                          &eoffset          ,
                                          &size             ,
                                          &timestamp        ,
                                          ename             ,
                                          access            );
                offset+= DVP_INDEX_ENTRY_SIZE;

                /*
                 *      Compare name
                 */
                if ( 0 == strcmp( ename, name ) )
                {
                        /*
                         *      Found entry with name requested
                         *      Create entry data struct
                         */
                        entry->offset = eoffset;
                        entry->size = size;
                        strcpy( entry->name, ename );
                        entry->timestamp = timestamp;

                        return 0;
                }
        }
        return 1;
}

/*---------------------------------------------------------------------------*/

/**
 *      Get entry data by name of entry
 *      if there are multiple occurences of the name the first instance
 *      will be returned. if not found 1 is returned otherwise 0.
 */
UInt32
DVP_Find_Entry_Data( FILE *fptr              ,
                     const char *name        ,
                     TDVP_Index_Entry *entry )
{
        /*
         *      Obtain entry data by searching for name in index
         */
        UInt32 timestamp;
        UInt32 tmp2;
        char ID[ DVP_ID_LENGTH+1 ];
        char ename[ DVP_MAX_INAME_LENGTH+1 ];
        UInt32 offset;
        UInt32 eoffset;
        UInt32 size;
        UInt32 i;
        UInt32 dirs = 0;
        UInt32 files = 0;

    /*    #if defined( DEBUG_MODE )
        tsprintf( "DVP_Find_Entry_Data( %d, %s, %d )\n", (UInt32)fptr, name, (UInt32)entry );
        #endif  */

        /*
         *      Get index offset and size
         */
        ID[ DVP_ID_LENGTH ] = 0;
        if ( DVP_Read_Header( fptr,
                              ID         ,
                              &tmp2      ,
                              &offset    ,
                              &files     ,
                              &dirs      ,
                              &timestamp ) )
        {
                /*
                 *      Most likely checksum problem
                 */
                return 1;
        }

        /*
         *      Search for requested entry
         */
        ename[ DVP_MAX_INAME_LENGTH ] = 0;
        for ( i=0; i<(files+dirs); i++ )
        {
                /*
                 *      Read entry
                 */
                DVP_Read_Index_Entry( offset            ,
                                      &eoffset          ,
                                      &size             ,
                                      &timestamp        ,
                                      ename             ,
                                      fptr              );
                offset+= DVP_INDEX_ENTRY_SIZE;

                /*
                 *      Compare name
                 */
                if ( !strcmp( ename, name ) )
                {
                        /*
                         *      Found entry with name requested
                         *      Create entry data struct
                         */
                        entry->offset = eoffset;
                        entry->size = size;
                        strcpy( entry->name, ename );
                        entry->timestamp = timestamp;

                        return 0;
                }
        }
        return 1;
}

/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

/**
 *      Extracts a block ( file ) from the archive into a external file
 *      specified by filename
 */
UInt32
DVP_IOA_Extract_File( TIOAccess *access       ,
                      UInt32 offset           ,
                      UInt32 filesize         ,
                      UInt32 blocksize        ,
                      const char *filename    )
{
        UInt32 nbytes;
        UInt32 nbytes_read = 0;
        char *tmp_data;
        FILE *out_file;

    /*    #if defined( DEBUG_MODE )
        tsprintf( "DVP_IOA_Extract_File( %d, %d, %d, %d, %s )\n", (UInt32)access, offset, filesize, blocksize, filename );
        #endif  */

        /*
         *      Allocate buffer
         */
        if ( filesize < blocksize )
        {
                blocksize = filesize;
        }
        tmp_data = ( char* ) malloc( blocksize );

        /*
         *      set output file
         */
        out_file = fopen( filename, "wb" );
        if ( !out_file )
        {
                return( 1 );
        }

        /*
         *      set datafile filepointer
         */
        access->seek( access , offset , SEEK_SET );

        /*
         *      read data from file and write to new file
         */
        do
        {
                nbytes = access->read( access, tmp_data, 1, blocksize );

                /*
                 *      Check then use actually read data to file
                 */
                if ( nbytes > 0 )
                {
                        nbytes_read += nbytes;

                        /*
                         *      Prevent writing to much data.
                         */
                        if ( nbytes_read > filesize )
                        {
                                nbytes -= nbytes_read-filesize;
                                nbytes_read = filesize;
                        }
                        fwrite( tmp_data, 1, nbytes, out_file );
                }
                else
                {
                        /*
                         *      Reached end of file
                         */
                        break;
                }

        } while( nbytes_read < filesize );

        /*
         *      close the newly created file
         */
        fclose(out_file);

        /*
         *      Dispose of buffer
         */
        free( tmp_data );

        /*
         *      Do a final file integrity check because the while loop could
         *      have exited because the end of the datafile was reached.
         */
        if ( nbytes_read != filesize )
        {
                /*
                 *      Not all data was read from file
                 */
                return( 1 );
        }
        return( 0 );
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Extract_File( FILE *fpt              ,
                  UInt32 offset          ,
                  UInt32 filesize        ,
                  UInt32 blocksize       ,
                  const char *filename   )
{
        UInt32 nbytes;
        UInt32 nbytes_read = 0;
        char *tmp_data;
        FILE *out_file;

    /*    #if defined( DEBUG_MODE )
        tsprintf( "DVP_Extract_File( %d, %d, %d, %d, %s )\n", (UInt32)fpt, offset, filesize, blocksize, filename );
        #endif  */

        /*
         *      Allocate buffer
         */
        if ( filesize < blocksize )
        {
                blocksize = filesize;
        }
        tmp_data = ( char* ) malloc( blocksize );

        /*
         *      set output file
         */
        out_file = fopen( filename, "wb" );
        if ( !out_file )
        {
                return( 1 );
        }

        /*
         *      set datafile filepointer
         */
        fseek( fpt , offset , SEEK_SET );

        /*
         *      read data from file and write to new file
         */
        do
        {
                nbytes = (UInt32)fread( tmp_data, 1, blocksize, fpt );

                /*
                 *      Check then use actually read data to file
                 */
                if ( nbytes > 0 )
                {
                        nbytes_read += nbytes;

                        /*
                         *      Prevent writing to much data.
                         */
                        if ( nbytes_read > filesize )
                        {
                                nbytes -= nbytes_read-filesize;
                                nbytes_read = filesize;
                        }
                        fwrite( tmp_data, 1, nbytes, out_file );
                }
                else
                {
                        /*
                         *      Reached end of file
                         */
                        break;
                }

        } while( nbytes_read < filesize );

        /*
         *      close the newly created file
         */
        fclose(out_file);

        /*
         *      Dispose of buffer
         */
        free( tmp_data );

        /*
         *      Do a final file integrity check because the while loop could
         *      have exited because the end of the datafile was reached.
         */
        if ( nbytes_read != filesize )
        {
                /*
                 *      Not all data was read from file
                 */
                return( 1 );
        }
        return( 0 );
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

/**
 *      This is more or less a version of DVP_Extract_File() for lazy users.
 *      it obtains the 3 other parameters that DVP_Extract_File() needs itself.
 */
UInt32
_DVP_IOA_Extract_File( TIOAccess *access ,
                   const char *filename  ,
                   const char *dest_file )
{
        TDVP_Index_Entry entry;
        if ( DVP_IOA_Find_Entry_Data( access, filename, &entry ) )
        {
                /*
                 *      File not found in index
                 */
                return 1;
        }

        /*
         *      Call the normal DVP_Extract_File() function
         */
        return DVP_IOA_Extract_File( access, entry.offset, entry.size, DVP_BLOCK_SIZE, dest_file );
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

/**
 *      This is more or less a version of DVP_Extract_File() for lazy users.
 *      it obtains the 3 other parameters that DVP_Extract_File() needs itself.
 */
UInt32
_DVP_Extract_File( FILE *fpt             ,
                   const char *filename  ,
                   const char *dest_file )
{
        TDVP_Index_Entry entry;
        if ( DVP_Find_Entry_Data( fpt, filename, &entry ) )
        {
                /*
                 *      File not found in index
                 */
                return 1;
        }

        /*
         *      Call the normal DVP_Extract_File() function
         */
        return DVP_Extract_File( fpt, entry.offset, entry.size, DVP_BLOCK_SIZE, dest_file );
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

/**
 *      Extracts all files stored within the archive with their respective
 *      paths starting in the specified dest_dir dir.
 */
UInt32
DVP_IOA_Extract_All_Files( TIOAccess *access      ,
                           const char *dest_dir   )
{
        /*
         *      Extract all files reconstructing original dir tree structure.
         */
        char curdir[ MAX_DIR_LENGTH+1 ];
        char filepath[ MAX_DIR_LENGTH+DVP_MAX_INAME_LENGTH+1 ];
        UInt32 entrys, i;
        TDVP_File_Header header;
        TDVP_Index_Entry entry;

     /*   #if defined( DEBUG_MODE )
        tsprintf( "DVP_Extract_All_Files( %d, %s )\n", (UInt32)access, dest_dir );
        #endif  */

        strcpy( curdir, dest_dir );

        /*
         *      Read header info
         */
        if ( _DVP_IOA_Read_Header( access, &header ) )
        {
                /*
                 *      Problem reading header
                 */
                return 1;
        }
        entrys = header.num_files + header.num_dirs*2;
        for ( i=0; i<entrys; i++)
        {
                /*
                 *      Read file info from index
                 */
                _DVP_IOA_Read_Index_Entry( header.index_offset, &entry, access );
                header.index_offset += DVP_INDEX_ENTRY_SIZE;

                /*
                 *      Is the entry a dir or a file ?
                 */
                if ( 0 != entry.size )
                {
                        /*
                         *      We have a file entry. We will extract the file
                         */
                        strcpy( filepath, curdir );
                        Append_To_Path( filepath, sizeof(filepath), entry.name );
                        if ( DVP_IOA_Extract_File( access, entry.offset, entry.size, DVP_BLOCK_SIZE, filepath ) )
                        {
                                /*
                                 *      Error extracting file
                                 */
                                return 1;
                        }
                }
                else
                {
                        /*
                         *      We have a directory entry
                         */
                        if ( 0 != strcmp( entry.name, ".." ) )
                        {
                                /*
                                 *      Go up one level by stripping last subdir
                                 */
                                Strip_Last_Subdir( curdir );
                        }
                        else
                        {
                                /*
                                 *      Go one level down by appending new
                                 *      subdir and create the dir. Creating the
                                 *      dir will fail if dir exists which is no
                                 *      problem but if the user does not have the
                                 *      acess rights then we will return at
                                 *      DVP_Extract_File() which will fail also.
                                 */
                                Append_To_Path( curdir, sizeof(curdir), entry.name );
                                Create_Directory( curdir );
                        }
                }
        }
        return( 0 );
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

/**
 *      Extracts all files stored within the archive with their respective
 *      paths starting in the specified dest_dir dir.
 */
UInt32
DVP_Extract_All_Files( FILE *fptr, const char *dest_dir )
{
        /*
         *      Extract all files reconstructing original dir tree structure.
         */
        char curdir[ MAX_DIR_LENGTH+1 ];
        char filepath[ MAX_DIR_LENGTH+DVP_MAX_INAME_LENGTH+1 ];
        UInt32 entrys, i;
        TDVP_File_Header header;
        TDVP_Index_Entry entry;

     /*   #if defined( DEBUG_MODE )
        tsprintf( "DVP_Extract_All_Files( %d, %s )\n", (UInt32)fptr, dest_dir );
        #endif  */

        strcpy( curdir, dest_dir );

        /*
         *      Read header info
         */
        if ( _DVP_Read_Header( fptr, &header ) )
        {
                /*
                 *      Problem reading header
                 */
                return 1;
        }
        entrys = header.num_files + header.num_dirs*2;
        for ( i=0; i<entrys; i++)
        {
                /*
                 *      Read file info from index
                 */
                _DVP_Read_Index_Entry( header.index_offset, &entry, fptr );
                header.index_offset += DVP_INDEX_ENTRY_SIZE;

                /*
                 *      Is the entry a dir or a file ?
                 */
                if ( entry.size )
                {
                        /*
                         *      We have a file entry. We will extract the file
                         */
                        strcpy( filepath, curdir );
                        Append_To_Path( filepath, sizeof(filepath), entry.name );
                        if ( DVP_Extract_File( fptr, entry.offset, entry.size, DVP_BLOCK_SIZE, filepath ) )
                        {
                                /*
                                 *      Error extracting file
                                 */
                                return 1;
                        }
                }
                else
                {
                        /*
                         *      We have a directory entry
                         */
                        if ( !strcmp( entry.name, ".." ) )
                        {
                                /*
                                 *      Go up one level by stripping last subdir
                                 */
                                Strip_Last_Subdir( curdir );
                        }
                        else
                        {
                                /*
                                 *      Go one level down by appending new
                                 *      subdir and create the dir. Creating the
                                 *      dir will fail if dir exists which is no
                                 *      problem but if the user does not have the
                                 *      acess rights then we will return at
                                 *      DVP_Extract_File() which will fail also.
                                 */
                                Append_To_Path( curdir, sizeof(curdir), entry.name );
                                Create_Directory( curdir );
                        }
                }
        }
        return( 0 );
}

#endif
/*---------------------------------------------------------------------------*/

static UInt32
DVP_Filesize( FILE* fptr )
{
        /*
         *      Portable ANSI C method of getting filesize,
         *      Filepointer is not moved after completion.
         */
        UInt32 size_of_file;
        UInt32 cur_pos;

      /*  #if defined( DEBUG_MODE )
        tsprintf( "DVP_Filesize( %d )\n", (UInt32)fptr );
        #endif  */
      //  fflush( fptr );

        /*
         *      Save the current position.
         */
        cur_pos = ftell( fptr );

        /*
         *      Jump to the end of the file.
         */
        fseek( fptr, 0L, SEEK_END );

        /*
         *      Get the end position.
         */
        size_of_file = ftell( fptr );

        /*
         *      Jump back to the original position.
         */
        fseek( fptr, cur_pos, SEEK_SET );

        return( size_of_file );
}

/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Append_Index( FILE *datafile , const char *indexfile , UInt32 num_entrys )
{
	/*
	 *	Write pack file index.
	 *	This is done by appending the hdr to the dvp file
	 *
	 *	d is the filepointer of the datafile
	 *	h is the filepointer of the index file
         *
         *      on error 1 is returned, otherwise 0
	 */

        UInt32  i                                                          ;
        FILE *idx                                                       ;
        char store[ DVP_INDEX_ENTRY_SIZE ];
        UInt32 entry_size;
        UInt32 pos = ftell( datafile );

	/*#if defined( DEBUG_MODE )
        tsprintf( "DVP_Append_Index( %d, %s, %d )\n", (UInt32)datafile, indexfile, num_entrys );
        #endif  */

        /*
         *	Move to end of data file
         */
        fseek( datafile, 0, SEEK_END );

        /*
         *      Open files
         */
        idx = fopen( indexfile, "rb" )                                  ;
        if ( !idx )
        {
                return 1;
        }

        /*
         *      Check parameter validity
         */
        if ( DVP_Filesize( idx ) != ( UInt32 )(num_entrys*DVP_INDEX_ENTRY_SIZE ))
        {
                /*
                 *      If we get here the num_entry's is incorrect OR
                 *      the index file was not created correctly
                 */
                fclose( idx );
                return 1;
        }

	/*
	 *	Loop to append index
	 */
	for ( i=0; i < num_entrys; i++ )
	{
		/*
		 *	Read then write 1 index entry
		 */
		entry_size = (UInt32)fread( store, 1, DVP_INDEX_ENTRY_SIZE, idx );
                if ( entry_size != (UInt32)(DVP_INDEX_ENTRY_SIZE) )
                {
                        /*
                         *      Problem reading index entry
                         */
                        fclose( idx );
                        return 1;
                }
		if ( entry_size != fwrite( store , 1 , DVP_INDEX_ENTRY_SIZE, datafile ) )
                {
                	/*
                         *	Problem writing index entry to file
                         */
                	fclose( idx );
                	return 1;
                }
	}

        /*
         *      Close the file
         */
        fclose( idx );

        /*
         *	Put file pointer back where we found it
         */
        fseek( datafile, pos, SEEK_SET );

        /*
         *      Everything worked correctly
         */
        return 0;
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Pack_File( FILE *dfpt       ,
               FILE *ifpt       ,
               const char *filespec   ,
               const char *filename   ,
               UInt32 *offset     ,
               UInt32 filesize    ,
               UInt32 time_write  )
{
        char path[1024];
        Ubyte *tmp_data;
        FILE *fp;
        UInt32  nbytes;
        UInt32 total = 0;
        UInt32 start;

      /*  #if defined( DEBUG_MODE )
        tsprintf( "DVP_Pack_File( %d, %d, %s, %s, %d, %d, %d )\n", (UInt32)dfpt, (UInt32)ifpt, filespec, filename, *offset, filesize, time_write );
        #endif  */

        /*
         *	Don't pack file that has filename length's we don't support
         */
        if ( strlen( filename ) > DVP_MAX_INAME_LENGTH )
        {

        	return 1;
        }

	/*
	 *	Don't pack yourself!!
	 */
	if ( strstr( filename, DVP_PACK_EXT ) )
	{
		return	1						;
	}

	/*
	 *	Don't pack yourself!!
	 */
	if ( strstr( filename, DVP_IDX_EXT ) )
	{
		return	1						;
	}

	/*
	 *	Don't pack 0 length files, screws up directory structure!
	 */
	if ( filesize == 0 )
	{
		return	1						;
	}

	/*
	 *	fill the path string with 0's.
	 *	this initializes the memory space.
	 */
	memset( path , 0 , 1024 )				;

	/*
	 *	filename is copyed into path
	 */
	strcpy( path, filename )			        	;

	sprintf( path, "%s\\%s", filespec, filename )			;

	fp = fopen( path, "rb" )					;
	if ( fp == NULL )
	{
		/*
		 *	Error opening path
		 */
		return( 1 )		   				;
	}

	/*
	 *	Add index entry
	 */
        start = ftell( ifpt );
	fwrite( offset , 1 , 4, ifpt )                 	;
	fwrite( &filesize , 1 , 4, ifpt )    		;
	fwrite( filename, 1 , DVP_MAX_INAME_LENGTH, ifpt );
	fwrite( &time_write , 1 , 4, ifpt )	        ;
        if ( ftell( ifpt )-start != DVP_INDEX_ENTRY_SIZE )
        {
        	/*
                 *	Incorrect index entry size
                 */
                return 1;
        }

        /*
         *	We dynamicly allocate the block to prevent a stack overflow
         *	if the block is big.
         */
 	tmp_data = ( Ubyte* ) malloc( DVP_BLOCK_SIZE ); /* 1 MB atm */

	/*
	 *	Read from file and add to pack
	 */
	do
        {
		nbytes = (UInt32)fread( tmp_data, 1, DVP_BLOCK_SIZE, fp );
                total += nbytes;
                fwrite( tmp_data, 1, nbytes, dfpt );
	}
        while( total < filesize );
        free( tmp_data );

	/*
	 *	Close File
	 */
	fclose( fp );

        /*
         *	Do a final check to see if correct amount of data was written
         */
        if ( ( total != filesize ) || ( (UInt32)ftell( dfpt ) != filesize+*offset ) ) return 1;

        return( 0 );
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Append_Directory( const char *path   ,
                      UInt32 total_size  ,
                      FILE *fp           )
{
	/*
	 *	This function adds a directory marker to the index file
	 */
        char direntry[ DVP_MAX_INAME_LENGTH ];
        const char *dirp;
	UInt32 	i = 0;
        UInt32 start;

       /* #if defined( DEBUG_MODE )
        tsprintf( "DVP_Append_Directory( %s, %d, %d )\n", path, total_size, (UInt32)fp );
        #endif  */

        /*
         *	Don't add dir that has name length we don't support
         */
        dirp = &path[ Last_Subdir( path ) ];
        if ( strlen( dirp ) >= DVP_MAX_INAME_LENGTH )
        {
        	return 1;
        }

	/*
	 *	Write first part of index directory entry
	 */
        start = ftell( fp );
	fwrite( &total_size , 1 , 4, fp );
	fwrite( &i , 1 , 4, fp );

	/*
	 *	Write second part of index directory entry
	 */
        memset( direntry, 0, DVP_MAX_INAME_LENGTH*sizeof(char) );
        strcpy( direntry, dirp );
	fwrite( direntry, 1 , DVP_MAX_INAME_LENGTH, fp );
	fwrite( &i , 1 , 4, fp );
        if ( ftell( fp )-start != DVP_INDEX_ENTRY_SIZE )
        {
        	return 1;
        }
        return 0;
}

#endif
/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Pack_Directory( FILE *dfpt, FILE *ifpt, UInt32 *total_size, const char *path, UInt32 *num_files, UInt32 *num_dirs )
{
	/*
	 *	Adds a directory and all files it contains to the archive,
	 *	in other words : the directory gets packed.
	 */
        struct SDI_Data *data;
	char tmp[ 512 ];
	char tmp1[ 512 ];

       /* #if defined( DEBUG_MODE )
        tsprintf( "DVP_Pack_Directory( %d, %d, %d, %s, %d, %d )\n", (UInt32)dfpt, (UInt32)ifpt, *total_size, path, *num_files, *num_dirs );
        #endif */

	/*
	 *	Setup dir string
	 */
	strcpy( tmp1, path );
        *num_dirs = *num_dirs+1;
	if ( DVP_Append_Directory( path, *total_size, ifpt ) )
        {
        	/*
                 *	Error adding directory index entry
                 */
        	return 1;
        }

        /*
         *	Get the first entry and make sure no error occured
         */
        data = DI_First_Dir_Entry( path );
        if ( !data ) return 1;

        while ( data )
        {
        	if ( DI_Is_It_A_File( data ) )
                {
                	/*
                         *	A file in the dir, we will attempt to add it
                         *	to the pack.
                         */
                        *num_files=*num_files+1;
                        if ( DVP_Pack_File( dfpt                 ,
                                            ifpt                 ,
                                            path                 ,
                                            DI_Name( data )      ,
                                            total_size           ,
                                            DI_Size( data )      ,
                                            DI_Timestamp( data ) ) )
                        {
                        	/*
                                 *	Error adding file to pack
                                 */
                                return 1;
                        }
                        *total_size+=DI_Size( data );
                }
                else
                {
                	/*
                         *	A subdirectory has been found.
                         *	If it is a real dir we will pack it otherwise
                         *	we will skip it.
                         */
			if ( ( strcmp( "..", DI_Name( data ) )!= 0 ) && ( strcmp( ".", DI_Name( data ) )!= 0 ) )
                        {
				strcpy( tmp, path );
				Append_To_Path( tmp, sizeof(tmp), DI_Name( data ) );
                                if ( DVP_Pack_Directory( dfpt        ,
                                                         ifpt        ,
                                                         total_size  ,
                                                         tmp         ,
                                                         num_files   ,
                                                         num_dirs    ) )
                                {
                                	/*
                                         *	There was an error packing a
                                         *	lower level dir.
                                         */
                                        return 1;
                                }
			}
                }
                if ( !DI_Next_Dir_Entry( data ) )
                {
                	/*
                         *	No more entry's in dir.
                         */
                        DI_Cleanup( data );
                        data = NULL;
                }
        }

        /*
         *	Now we add a .. entry to the index indicating we are leaving the
         *	dir and going to a dir higher in the tree.
         */
	if ( DVP_Append_Directory( "..", *total_size , ifpt ) )
        {
        	/*
                 *	Error adding directory
                 */
        	return 1;
        }
        return 0;
}

#endif
/*---------------------------------------------------------------------------*/

UInt32
DVP_Version( void )
{
        /*
         *	Return current module version, usefull for comparing the module
         *	version an application was built with to the actually linked module
         *	version.
         */
        return (DVP_MAJOR_VERSION*1000)+(DVP_MINOR_VERSION*100)+(DVP_RELEASE_VERSION*10)+DVP_PATCH_VERSION;
}

/*---------------------------------------------------------------------------*/

UInt32
DVP_Display_Index( FILE *fptr              ,
                   UInt32 index_offset     ,
                   UInt32 entrys           )
{
        /*
         *      Print archive contents to console
         */
        UInt32 offset      ;
        UInt32 size        ;
        UInt32 tstamp      ;
        UInt32 i            ;
        char name[ DVP_MAX_INAME_LENGTH+1 ];

      /*  #if defined( DEBUG_MODE )
        tsprintf( "DVP_Display_Index( %d, %d, %d )\n", (UInt32)fptr, index_offset, entrys );
        #endif */

        /*
         *      Jump to beginning of index
         */
        fseek( fptr, index_offset, SEEK_SET );

        /*
         *      Output data for each entry to console
         */
        for ( i=0; i<entrys; i++ )
        {
                /*
                 *      Read entry
                 */
                if ( DVP_Read_Index_Entry( index_offset      ,
                                           &offset           ,
                                           &size             ,
                                           &tstamp           ,
                                           name              ,
                                           fptr              ) )
                {
                        printf( "\nERROR: Cannot read entry data\n" );
                        return 1;
                }

                index_offset+= DVP_INDEX_ENTRY_SIZE                     ;

                /*
                 *      Display entry data
                 */
                printf( "\n# Entry data\n" )                            ;
                printf( "name : %s\n", name )                           ;
                if ( size == 0 )
                {
                        /*
                         *      display dir data
                         */
                        printf( "type : directory\n" )                  ;
                }
                else
                {
                        /*
                         *      display file data
                         */
                        printf( "type : file\n" )                       ;
                        printf( "size : %i\n", size )                   ;
                }
        }

        return 0                                                        ;
}

/*---------------------------------------------------------------------------*/
#if !DVP_READ_ONLY_BUILD

UInt32
DVP_Create_Archive( const char *archive_name , const char *src_dir , const char *ID )
{
	/*
	 *	Creates a packed file.
	 *
	 *	archive_name is the name used to make the names of the resulting
	 *	pack files.
	 *
	 *	src_dir is the dir where the packing should start
	 *
	 *	key is a int that can be used to change the header, to make it harder
	 *	to read the index and modify the file in any way without permission.
	 *
	 *	The return value is 1 in case of an error.
	 */

	FILE	*fp_out 		;
	FILE	*fp_out_hdr 		;
	char	*archive_dat		;
	char	*archive_hdr		;
        char    *p                      ;
        UInt32     num_of_files = 0        ;
        UInt32     num_of_dirs  = 0        ;
        UInt32    total_size = 0          ;

       /* #if defined( DEBUG_MODE )
        sprintf( "DVP_Create_Archive( %s, %s, %s )\n", archive_name, src_dir, ID );
        #endif  */

	/*
	 *	Make Filenames out of ArchiveName
	 */
	p = strchr( archive_name , '.' )				;
	if ( p )
	{
		*p = 0							;
	}

        archive_dat = ( char* ) calloc( strlen(archive_name)-File_Ext_Length(archive_name)+strlen(DVP_PACK_EXT)+2, sizeof( char ) );
	strcpy( archive_dat, archive_name )			    	;
        strcat( archive_dat, "." )                                      ;
	strcat( archive_dat, DVP_PACK_EXT )				;

        archive_hdr = ( char* ) calloc( strlen(archive_name)-File_Ext_Length(archive_name)+strlen(DVP_IDX_EXT)+2, sizeof( char ) ) ;
	strcpy( archive_hdr, archive_name )				;
        strcat( archive_hdr, "." )                                      ;
	strcat( archive_hdr, DVP_IDX_EXT )                              ;

	/*
	 *	Create the output files
	 */
	fp_out = fopen( archive_dat, "wb" )				;
	if ( !fp_out )
	{
		/*
		 *	Error creating file
		 */
		return 1			        		;
	}
        free( archive_dat );

	fp_out_hdr = fopen( archive_hdr, "wb" )				;
	if ( !fp_out_hdr )
	{
		/*
		 *	Error creating file
		 */
		return 1				        	;
	}

	/*
	 *	Write header to output file
	 */
	if ( DVP_Write_Header( fp_out, ID, DVP_HEADER_SIZE, num_of_files, num_of_dirs ) )
	{
		/*
		 *	Error writing header
		 */
		return 1				     	   	;
	}
        total_size = DVP_HEADER_SIZE;

	/*
	 *	Add data to pack file and and index entry's to index
	 */
        fseek( fp_out, DVP_HEADER_SIZE, SEEK_SET );
	if ( DVP_Pack_Directory( fp_out         ,
                                 fp_out_hdr     ,
                                 &total_size    ,
                                 src_dir        ,
                                 &num_of_files  ,
                                 &num_of_dirs   )  )
	{
		/*
		 *	Error packing directory
		 */
		return  1						;
	}

	/*
	 *	Write header to output file
	 */
	if ( DVP_Write_Header( fp_out, ID, total_size, num_of_files, num_of_dirs ) )
	{
		/*
		 *	Error writing header
		 */
		return 1				     	   	;
	}

	/*
	 *	Close output files
	 */
    	fclose( fp_out_hdr )				 		;

	/*
	 *	Add index to archive_dat
	 */
    	if ( DVP_Append_Index( fp_out , archive_hdr, num_of_files + (num_of_dirs*2) ) )
        {
                /*
		 *	error appending index
		 */
    	    	return 1			  		;
        }
        fclose( fp_out );
        free( archive_hdr );

	/*
	 *	All operations where successful, return written bytes
	 */
	return 0;
}

#endif

/*---------------------------------------------------------------------------*/
