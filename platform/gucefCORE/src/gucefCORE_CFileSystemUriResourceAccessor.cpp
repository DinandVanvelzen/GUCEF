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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "gucefCORE_CFileSystemUriResourceAccessor.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CFileSystemUriResourceAccessor::SchemeName = "file";

/*-------------------------------------------------------------------------*/

/**
 *  The path on the file system a file uri refers to. A uri without authority ("file:///dir/file") has an absolute path,
 *  one with an authority ("file://dir/file") is treated as a relative path as it always was.
 */
static CString
GetFileSystemPath( const CUri& uri )
{GUCEF_TRACE;

    CString fsPath = uri.GetAuthorityAndPath();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    // an absolute windows path in a uri is "file:///C:/dir/file", where the path starts with a slash before the drive
    if ( fsPath.Length() > 2 && '/' == fsPath[ 0 ] && ':' == fsPath[ 2 ] )
        fsPath = fsPath.SubstrToIndex( 1, false );
    #endif

    return fsPath;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFileSystemUriResourceAccessor::CFileSystemUriResourceAccessor( void )
    : CUriResourceAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemUriResourceAccessor::CFileSystemUriResourceAccessor( const CFileSystemUriResourceAccessor& src )
    : CUriResourceAccessor( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemUriResourceAccessor::~CFileSystemUriResourceAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemUriResourceAccessor& 
CFileSystemUriResourceAccessor::operator=( const CFileSystemUriResourceAccessor& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::CreateResource( const CUri& uri ,
                                                CIOAccess& src  )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:CreateResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    bool fileExists = FileExists( fsPath );

    if ( !fileExists )
    {
        CFileAccess fsFile;
        if ( fsFile.Open( fsPath, "wb" ) )
        {
            fsFile.Write( src );
            fsFile.Close();
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:CreateResource: Wrote file to " + fsPath );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:CreateResource: Cannot open file for writing at: " + fsPath );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:CreateResource: Cannot create resource since the file already exists: " + fsPath );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::GetResource( const CUri& uri        ,
                                             CIOAccess& destination )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    bool fileExists = FileExists( fsPath );

    if ( fileExists )
    {
        CFileAccess fsFile;
        if ( fsFile.Open( fsPath, "rb" ) )
        {
            destination.Write( fsFile );
            fsFile.Close();
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Wrote file to " + fsPath );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Cannot open file for reading at: " + fsPath );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: file does not exist: " + fsPath );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CFileSystemUriResourceAccessor::GetResourceAccess( const CUri& uri               ,
                                                   IOAccessPtr& accessToResource ,
                                                   TURI_RESOURCEACCESS_MODE mode )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResourceAccess: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    bool fileExists = FileExists( fsPath );

    if ( fileExists )
    {
        FileAccessPtr fsFile( GUCEF_NEW CFileAccess() );
        if ( !fsFile.IsNULL() )
        {
            const char* accessMode = ResourceAccessModeStr( mode );            
            if ( fsFile->Open( fsPath, accessMode ) )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResourceAccess: Obtained access to file " + fsPath );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResourceAccess: Cannot obtain access to file at: " + fsPath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResourceAccess: Cannot allocate file access object" );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResourceAccess: file does not exist: " + fsPath );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::GetPartialResource( const CUri& uri        ,
                                                    UInt64 byteOffset      ,
                                                    Int64 bytesToGet       ,
                                                    CIOAccess& destination )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetPartialResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    bool fileExists = FileExists( fsPath );

    if ( fileExists )
    {
        CFileAccess fsFile;
        if ( fsFile.Open( fsPath, "rb" ) )
        {
            if ( 0 == fsFile.Seek( byteOffset, SEEK_SET ) )
            {            
                destination.Write( fsFile, bytesToGet );
                fsFile.Close();
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetPartialResource: Wrote partial file to " + fsPath );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetPartialResource: Failed to seek to offset " + ToString( byteOffset ) + " in file : " + fsPath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetPartialResource: Cannot open file for reading at: " + fsPath );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetPartialResource: file does not exist: " + fsPath );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::GetResourceMetaData( const CUri& uri             ,
                                                     CResourceMetaData& metaData )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResourceMetaData: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    metaData.resourceExists = FileExists( fsPath );

    if ( metaData.resourceExists )
    {
        metaData.creationDateTime = GetFileCreationTime( fsPath );
        metaData.hasCreationDateTime = true;
        metaData.modifiedDateTime = GetFileModificationTime( fsPath );
        metaData.hasModifiedDateTime = true;
        metaData.lastAccessedDateTime = CDateTime::Empty;
        metaData.hasLastAccessedDateTime = false;
        metaData.version.Clear();
        metaData.hasVersion = false;
        metaData.name = fsPath;
        metaData.hasName = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::UpdateResource( const CUri& uri ,
                                                CIOAccess& src  )
{GUCEF_TRACE;

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    CFileAccess fsFile;
    if ( fsFile.Open( fsPath, "wb" ) )  // simply overwrite the file if it exists
    {
        fsFile.Write( src );
        fsFile.Close();
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdateResource: Wrote file to " + fsPath );
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdateResource: Cannot open file for writing at: " + fsPath );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::UpdatePartialResource( const CUri& uri    ,
                                                       UInt64 byteOffset  ,
                                                       Int64 bytesToWrite ,
                                                       CIOAccess& src     )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    bool fileExists = FileExists( fsPath );

    if ( fileExists )
    {
        CFileAccess fsFile;
        if ( fsFile.Open( fsPath, "rb+" ) )
        {
            if ( 0 == fsFile.Seek( byteOffset, SEEK_SET ) )
            {            
                fsFile.Write( src, bytesToWrite );
                fsFile.Close();
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Wrote partial file to " + fsPath );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Failed to seek to offset " + ToString( byteOffset ) + " in file : " + fsPath );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Cannot open file for alteration at: " + fsPath );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: file does not exist: " + fsPath );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::UpdatePartialResource( const CUri& uri    ,
                                                       CIOAccess& src     )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    bool fileExists = FileExists( fsPath );

    if ( fileExists )
    {
        CFileAccess fsFile;
        if ( fsFile.Open( fsPath, "wb+" ) ) // open the file in append mode
        {
            src.Write( fsFile, -1 );
            fsFile.Close();
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Appended to file " + fsPath );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Cannot open file for reading at: " + fsPath );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: file does not exist: " + fsPath );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::GetSupportedOperations( const CUri& uri                     ,
                                                        CUriResourceAccessorOperations& ops )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetSupportedOperations: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif
    
    bool fileExists = FileExists( fsPath );
    bool dirExists = DirExists( fsPath );

    ops.createResource = !fileExists;
    ops.deleteResource = fileExists || dirExists;
    ops.getPartialResource = fileExists;
    ops.getResource = fileExists;
    ops.getResourceMetaData = fileExists;
    ops.updatePartialResource = fileExists;
    ops.updateResource = fileExists;
    ops.collectionDetermnination = fileExists || dirExists;
    ops.collectionResolution = dirExists;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::DeleteResource( const CUri& uri )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:DeleteResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );
    
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif

    return DeleteFile( fsPath );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::IsACollectionResource( const CUri& uri ) const
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:IsACollectionResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    fsPath = fsPath.ReplaceChar( '/', '\\' );
    #endif
    
    // Within this context a directory is a collection
    // The directory needs to already exist for us to know if the URI is pointing at a directory
    return DirExists( fsPath );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemUriResourceAccessor::ResolveResourcesInCollection( const CUri& uri        ,
                                                              UriVector& resources   ,
                                                              bool recursive         ,
                                                              bool addCollectionUris )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:ResolveResourcesInCollection: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = GetFileSystemPath( uri );
    bool totalSuccess = true;

    if ( DirExists( fsPath ) )
    {
        CORE::CString filename;
        struct CORE::SDI_Data* did = DI_First_Dir_Entry( fsPath.C_String() );
        if ( did != GUCEF_NULL )
        {
            /*
             *      Iterate the dir content adding the file entries from
             *      the dir to the list
             */
            do
            {
                if ( DI_Is_It_A_File( did ) != 0 )
                {
                    filename = DI_Name( did );
                    if ( filename != '.' && filename != ".." )
                    {
                        CORE::CString filePath = CORE::CombinePath( fsPath, filename );
                        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
                        filePath = filePath.ReplaceChar( '\\', '/' );
                        #endif

                        CUri fileUri;
                        fileUri.SetScheme( SchemeName );
                        fileUri.SetPath( filePath );

                        resources.push_back( fileUri );
                    }
                }
                else
                {
                    CORE::CString dirName = DI_Name( did );
                    if ( dirName != '.' && dirName != ".." )
                    {
                        CORE::CString fsSubdir = CORE::CombinePath( fsPath, dirName );

                        CORE::CString dirPath = fsSubdir;
                        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
                        dirPath = dirPath.ReplaceChar( '\\', '/' );
                        #endif
                        dirPath += '/';

                        CUri subDirUri;
                        subDirUri.SetScheme( SchemeName );
                        subDirUri.SetPath( dirPath );

                        if ( recursive )
                        {
                            // Recursively process the sub-dir
                            totalSuccess = ResolveResourcesInCollection( subDirUri         ,
                                                                         resources         ,
                                                                         recursive         ,
                                                                         addCollectionUris ) && totalSuccess;
                        }

                        if ( addCollectionUris )
                            resources.push_back( subDirUri );
                    }
                }
            }
            while ( DI_Next_Dir_Entry( did ) );

            DI_Cleanup( did );
        }
        return totalSuccess;
    }
    else
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:ResolveResourcesInCollection: Cannot get resources in dir because the dir does not exist: " + fsPath );
        return false;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
