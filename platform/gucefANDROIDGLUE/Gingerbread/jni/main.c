/*
 *  gucefANDROIDGLUE: GUCEF module providing glue code for Android
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>             /* POSIX utilities */
#include <limits.h>             /* Linux OS limits */
#include <string.h>
#include <dlfcn.h>              /* dynamic linking utilities */
#include <stdio.h>
#include <stdlib.h>             /* malloc, free */
#include <stdarg.h>             /* va_list */

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>

#include "gucefLOADER.h"

void
android_syslog(int level, const char *format, ...)
{
    va_list arglist;
    va_start( arglist, format );
    __android_log_vprint( level, "GalaxyUnlimitedPlatform", format, arglist );
    va_end( arglist );
    return;
}


#define FLOGI( format, ... ) ( (void) android_syslog( ANDROID_LOG_INFO, format, __VA_ARGS__) )
#define FLOGW( format, ... ) ( (void) android_syslog( ANDROID_LOG_WARN, format, __VA_ARGS__) )
#define FLOGF( format, ... ) ( (void) android_syslog( ANDROID_LOG_FATAL, format, __VA_ARGS__) )
#define FLOGE( format, ... ) ( (void) android_syslog( ANDROID_LOG_ERROR, format, __VA_ARGS__) )
#define FLOGD( format, ... ) ( (void) android_syslog( ANDROID_LOG_DEBUG, format, __VA_ARGS__) )

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "GalaxyUnlimitedPlatform", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "GalaxyUnlimitedPlatform", __VA_ARGS__))
#define LOGF(...) ((void)__android_log_print(ANDROID_LOG_FATAL, "GalaxyUnlimitedPlatform", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "GalaxyUnlimitedPlatform", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "GalaxyUnlimitedPlatform", __VA_ARGS__))

typedef int ( GUCEF_CALLSPEC_PREFIX *TGUCEFCORECINTERFACE_LoadAndRunGucefPlatformApp ) ( const char* appName, const char* rootDir, const char* resRootDir, const char* libRootDir, int platformArgc, char** platformArgv, int appArgc, char** appArgv );

#define NULLPTR ((void*)(0))

/*-------------------------------------------------------------------------*/

char*
AllocString( const char* str )
{
    int len = strlen( str )+1;
    char* buffer = (char*) malloc( len );
    memcpy( buffer, str, len );
    return buffer;
}

/*-------------------------------------------------------------------------*/

char*
Combine2Strings( const char* str1 ,
                 const char* str2 )
{
    int str1len = strlen( str1 );
    int str2len = strlen( str2 );
    char* strBuffer = (char*) malloc( str1len + str2len + 1 );
    memcpy( strBuffer, str1, str1len );
    memcpy( strBuffer+str1len, str2, str2len+1 );
    return strBuffer;
}

/*-------------------------------------------------------------------------*/

char*
Combine3Strings( const char* str1 ,
                 const char* str2 ,
                 const char* str3 )
{
    int str1len = strlen( str1 );
    int str2len = strlen( str2 );
    int str3len = strlen( str3 );
    char* strBuffer = (char*) malloc( str1len + str2len + str3len + 1 );
    memcpy( strBuffer, str1, str1len );
    memcpy( strBuffer+str1len, str2, str2len );
    memcpy( strBuffer+str1len+str2len, str3, str3len+1 );
    return strBuffer;
}

/*-------------------------------------------------------------------------*/

char*
PointerToString( void* ptr )
{
    char* ptrStrBuffer = malloc( 34 );
    sprintf( ptrStrBuffer, "%p", ptr );
    return ptrStrBuffer;
}

/*-------------------------------------------------------------------------*/

char*
GetAssetPath( const char* packageDir ,
              const char* fileName   )
{
    return Combine3Strings( packageDir, "/assets/", fileName );
}

/*-------------------------------------------------------------------------*/

char*
GetAssetLibRoot( const char* packageDir )
{
    return Combine2Strings( packageDir, "/assets/libs/armeabi/" );
}

/*-------------------------------------------------------------------------*/

char*
GetLibPath( const char* packageDir ,
            const char* moduleName )
{
    return Combine3Strings( packageDir, "/lib/", moduleName );
}

/*-------------------------------------------------------------------------*/

void
FreeStringMatrix( char** argv ,
                  int argc    )
{
    int i;
    for ( i=0; i<argc; ++i )
    {
        free( argv[ i ] );
    }
    free( argv );
}

/*-------------------------------------------------------------------------*/

void
CreatePlatformParams( char*** argv              ,
                      int* argc                 ,
                      struct android_app* state )
{
    // right now we have 1 param
    // Each key is an entry and each value is an entry
    *argv = (char**) malloc( 2 * sizeof(char**) );
    *argc = 2;

    // create the android_app param
    argv[ 0 ] = AllocString( "-android_app" );
    argv[ 1 ] = PointerToString( state );
}

/*-------------------------------------------------------------------------*/

int
InvokeLoadAndRunGucefPlatformApp( const char* appName ,
                                  const char* rootDir ,
                                  int platformArgc    ,
                                  char** platformArgv ,
                                  int appArgc         ,
                                  char** appArgv      )
{
    char* modulePath = GetLibPath( rootDir, "libgucefLOADER.so" );
    void* modulePtr = (void*) dlopen( modulePath, RTLD_NOW );
    if ( NULL == modulePtr )
    {
        free( modulePath );
        LOGF( "Unable to link gucefLOADER module" );
        return 0;
    }
    FLOGI( "Loading loader module from: %s", modulePath );
    free( modulePath );
    modulePath = NULL;

    TGUCEFCORECINTERFACE_LoadAndRunGucefPlatformApp loadAndRunGucefPlatformApp =
        (TGUCEFCORECINTERFACE_LoadAndRunGucefPlatformApp) dlsym( modulePtr, "LoadAndRunGucefPlatformApp" );

    if ( NULL == loadAndRunGucefPlatformApp )
    {
        LOGF( "Unable to link gucefLOADER function: LoadAndRunGucefPlatformApp" );
        dlclose( modulePtr );
        return 0;
    }

    char* libRootDir = GetAssetLibRoot( rootDir );
    char* assetRootDir = GetAssetPath( rootDir, "" );

    int returnValue = loadAndRunGucefPlatformApp( appName      ,
                                                  rootDir      ,
                                                  assetRootDir ,
                                                  libRootDir   ,
                                                  platformArgc ,
                                                  platformArgv ,
                                                  appArgc      ,
                                                  appArgv      );

    free( libRootDir );
    libRootDir = NULL;
    free( assetRootDir );
    assetRootDir = NULL;
    dlclose( modulePtr );
    modulePtr = NULL;

    FLOGI( "LoadAndRunGucefPlatformApp returned with code %i", returnValue );
    return returnValue;
}

/*-------------------------------------------------------------------------*/

void
StripLastPathComponent( char* path, int pathLength )
{
    int i=pathLength;
    while ( i > 0 )
    {
        if ( path[ i ] == '/' )
        {
            path[ i ] = '\0';
            break;
        }
        --i;
    }
}

/*-------------------------------------------------------------------------*/

void
GetPackageDir( struct android_app* state ,
               char* pathBuffer          ,
               int bufferSize            )
{
    ANativeActivity* activity = state->activity;
    int pathLength = strlen( activity->internalDataPath );
    memcpy( pathBuffer, activity->internalDataPath, pathLength+1 );
    StripLastPathComponent( pathBuffer, pathLength );
}

/*-------------------------------------------------------------------------*/

int
FileExists( const char* filename )
{
    struct stat buf;
    return stat( filename, &buf ) == 0;
}

/*-------------------------------------------------------------------------*/

static int
recursive_mkdir( const char* dir, int accessPerms )
{
    char tmp[ PATH_MAX ];
    char *p = NULL;
    size_t len;
    int retValue=0;

    snprintf( tmp, sizeof(tmp), "%s", dir );
    len = strlen( tmp );
    if( tmp[ len-1 ] == '/' )
    {
        tmp[ len-1 ] = 0;
    }
    for( p=tmp+1; *p; ++p )
    {
        if( *p == '/' )
        {
            *p = 0;
            retValue = mkdir( tmp, accessPerms );
            if ( 0 != retValue )
            {
                if ( EEXIST != errno )
                {
                    return retValue;
                }
            }
            *p = '/';
        }
    }
    return mkdir( tmp, accessPerms );
}

/*-------------------------------------------------------------------------*/

int
MakeDir( const char* path, int permissions )
{
    int retValue = recursive_mkdir( path, permissions );

    if ( retValue == 0 )
    {
        FLOGI( "created dir: %s", path );
    }
    else
    {
        if ( EEXIST == errno )
        {
            FLOGI( "found existing dir: %s", path );
            return 1;
        }
        else
        {
            FLOGI( "error %i creating dir: %s", errno, path );
        }
    }
    return retValue == 0 ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

int
MakeFileDir( const char* path, int permissions )
{
    // First get just the directories without the file
    int retValue, pathLength = strlen( path );
    char* dirs = (char*) malloc( pathLength+1 );
    memcpy( dirs, path, pathLength+1 );
    StripLastPathComponent( dirs, pathLength );

    // Now we make sure the directories exist
    retValue = MakeDir( dirs, permissions );

    free( dirs );
    return retValue;
}

/*-------------------------------------------------------------------------*/

int
ExtractAsset( AAssetManager* assetManager ,
              const char* assetPath       ,
              const char* destPath        )
{
    AAsset* asset = AAssetManager_open( assetManager, assetPath, AASSET_MODE_BUFFER );
    if ( NULL == asset )
    {
        FLOGE( "Unable to open asset for extraction: %s", assetPath );
        return 0;
    }
    FLOGI( "Extracting asset: %s", assetPath );

    const void* fileBuffer = AAsset_getBuffer( asset );
    if ( NULL == fileBuffer )
    {
        AAsset_close( asset );
        FLOGE( "Unable to get buffer to asset for extraction: %s", assetPath );
        return 0;
    }
    off_t bufferSize = AAsset_getLength( asset );

    // Make sure the directories exist to put the file in
    if ( 0 == MakeFileDir( destPath, 00777 ) )
    {
        AAsset_close( asset );
        FLOGE( "Unable to make directories for asset extraction: %s", destPath );
        return 0;
    }

    FILE* destFile = fopen( destPath, "wb" );
    if ( NULL == destFile )
    {
        AAsset_close( asset );
        FLOGE( "Unable to open destination file for asset: %s", destPath );
        return 0;
    }

    if ( 1 != fwrite( fileBuffer, bufferSize, 1, destFile ) )
    {
        FLOGE( "Error extracting asset from %s to %s", assetPath, destPath );
        fclose( destFile );
        AAsset_close( asset );
        return 0;
    }
    fclose( destFile );
    AAsset_close( asset );
    FLOGI( "Extracted asset from %s to %s", assetPath, destPath );
    return 1;
}

/*-------------------------------------------------------------------------*/

char*
ReadString( FILE* fptr, char* buffer, int bufferSize )
{
    char* strRead = fgets( buffer, bufferSize, fptr );
    if ( NULL != strRead )
    {
        // fgets() includes \n and \r, we dont want that
        // get rid of them...
        int i = strlen( strRead )-1;
        while ( i>0 )
        {
            if ( buffer[ i ] == '\n' || buffer[ i ] == '\r' )
            {
                buffer[ i ] = '\0';
            }
            else break;
            --i;
        }
    }
    return strRead;
}

/*-------------------------------------------------------------------------*/

int
ExtractAssets( struct android_app* state ,
               const char* packageDir    )
{
    AAssetManager* assetManager = state->activity->assetManager;
    LOGI( "Checking for assets to extract" );

    // First extract the index file which tells us which other files to extract
    char* extractionIndexfilePath = GetAssetPath( packageDir, "filestoextract.txt" );
    int retValue = ExtractAsset( assetManager, "filestoextract.txt", extractionIndexfilePath );

    if ( 0 != retValue )
    {
        LOGI( "Found index of assets to extract" );

        // Now go trough the list extraction all files listed.
        // each line in the index file is a file to extract
        char entryBuffer[ PATH_MAX ];
        FILE* listFptr = fopen( extractionIndexfilePath, "r" );
        char* bufferPtr = 0;
        if ( NULL == listFptr )
        {
            // Abort, unable to open the file
            FLOGE( "Error %i when opening file %s", errno, extractionIndexfilePath );
            free( extractionIndexfilePath );
            return 0;
        }
        do
        {
            // Read the entry
            bufferPtr = ReadString( listFptr, entryBuffer, PATH_MAX );
            if ( NULL != bufferPtr )
            {
                // Create destination path and extract the asset
                char* destPath = GetAssetPath( packageDir, bufferPtr );
                retValue = ExtractAsset( assetManager, entryBuffer, destPath );

                if ( 0 == retValue )
                {
                    // Failed to extract the current entry, abort
                    FLOGE( "Failed to extract asset to %s", destPath );
                    free( destPath );
                    fclose( listFptr );
                    free( extractionIndexfilePath );
                    return 0;
                }
                free( destPath );
            }
        }
        while ( NULL != bufferPtr );
        fclose( listFptr );
    }
    else
    {
        LOGI( "No extraction index is available, no assets will be extracted" );
    }

    free( extractionIndexfilePath );
    return 1;
}

/*-------------------------------------------------------------------------*/

int
IsFirstRun( const char* packageDir )
{
    // We know the gucefLOADER relies on a text file named firstrun.completed
    // we will use the same convention here to keep things consistent
    char* firstrunFile = GetAssetPath( packageDir, "firstrun.completed" );
    int existsBool = FileExists( firstrunFile );
    free( firstrunFile );
    return existsBool;
}

/*-------------------------------------------------------------------------*/

void
SetFirstRunCompleted( const char* packageDir )
{
    // We know the gucefLOADER relies on a text file named firstrun.completed
    // we will use the same convention here to keep things consistent
    char* firstrunFile = GetAssetPath( packageDir, "firstrun.completed" );
    int existsBool = FileExists( firstrunFile );
    if ( 0 == existsBool )
    {
        if ( 0 != MakeFileDir( firstrunFile, 00777 ) )
        {
            FILE* fptr = fopen( firstrunFile, "wb" );
            if ( NULL != fptr )
            {
                FLOGI( "Wrote flag to %s", firstrunFile );
                fclose( fptr );
                free( firstrunFile );
                return;
            }
        }
        FLOGE( "Unable to create flag at %s", firstrunFile );
    }
    free( firstrunFile );
}

/*-------------------------------------------------------------------------*/

void
UnSetFirstRunCompleted( const char* packageDir )
{
    // We know the gucefLOADER relies on a text file named firstrun.completed.txt
    // we will use the same convention here to keep things consistent
    char* firstrunFile = GetAssetPath( packageDir, "firstrun.completed" );
    remove( firstrunFile );
    free( firstrunFile );

    LOGI( "Unset first run flag" );
}

/*-------------------------------------------------------------------------*/

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void
android_main( struct android_app* state )
{
    // Make sure glue isn't stripped.
    app_dummy();

    char packageDir[ 512 ];
    GetPackageDir( state, packageDir, 512 );

    // Check if we need to perform first time initialization
    int firstRun = IsFirstRun( packageDir );
    if ( 0 == firstRun )
    {
        LOGI( "Performing first run initialization" );

        // Extract to our private storage as desired
        if ( 0 == ExtractAssets( state, packageDir ) )
        {
            return;
        }

        LOGI( "Completed first run initialization" );
    }
    else
    {
        LOGI( "Detected previous run, skipping first run initialization" );
    }

    // Create the platform specific params
    char** platformArgv = NULLPTR;
    int platformArgc = 0;
    CreatePlatformParams( &platformArgv, &platformArgc, state );

    // Start the process of invoking the launch of the platform using the loader
    int appStatus = InvokeLoadAndRunGucefPlatformApp( "gucefPRODMAN", packageDir, platformArgc, platformArgv, 0, NULLPTR );

    // clean up our platform param data
    FreeStringMatrix( platformArgv, platformArgc );

    // Check if we had a successfull run
    if ( 0 != firstRun )
    {
        if ( 0 == appStatus )
        {
            LOGI( "Successfull completed first run, setting first run flag to false" );

            // Set the flag that we completed the first run
            SetFirstRunCompleted( packageDir );
        }
        else
        {
            // If the flag is already set, unset it
            UnSetFirstRunCompleted( packageDir );
        }
    }
    FLOGI( "exit status code: %i", appStatus );

}

/*-------------------------------------------------------------------------*/

//END_INCLUDE(all)
