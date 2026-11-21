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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "gucefVFS_CVfsUriResourceAccessor.h"

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
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CVfsUriResourceAccessor::SchemeName = "vfs";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsUriResourceAccessor::CVfsUriResourceAccessor( void )
    : CUriResourceAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsUriResourceAccessor::CVfsUriResourceAccessor( const CVfsUriResourceAccessor& src )
    : CUriResourceAccessor( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsUriResourceAccessor::~CVfsUriResourceAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsUriResourceAccessor& 
CVfsUriResourceAccessor::operator=( const CVfsUriResourceAccessor& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool 
CVfsUriResourceAccessor::CreateResource( const CORE::CUri& uri                 ,
                                         const CORE::CDynamicBuffer& srcBuffer )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:CreateResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    return CVfsGlobal::Instance()->GetVfs().StoreAsFile( fsPath, srcBuffer, 0, false );
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::CreateResource( const CORE::CUri& uri ,
                                         CORE::CIOAccess& src  )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:CreateResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    TBasicVfsResourcePtr file = CVfsGlobal::Instance()->GetVfs().GetFile( fsPath, "wb", true );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( !fileAccess.IsNULL() ) 
        {
            fileAccess->Open();
            fileAccess->Write( src );
            fileAccess->Close();
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:CreateResource: Wrote file to " + fsPath );
            return true;         
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:CreateResource: Cannot obtain access to resource: " + fsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CVfsUriResourceAccessor::GetResource( const CORE::CUri& uri                   ,
                                      CORE::CDynamicBuffer& destinationBuffer )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    if ( CVfsGlobal::Instance()->GetVfs().LoadFile( destinationBuffer, fsPath, "rb" ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Loaded file " + fsPath );
        return true;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Cannot load resource: " + fsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::GetResource( const CORE::CUri& uri        ,
                                      CORE::CIOAccess& destination )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    TBasicVfsResourcePtr file = CVfsGlobal::Instance()->GetVfs().GetFile( fsPath, "rb", false );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( !fileAccess.IsNULL() )  
        {
            fileAccess->Open();
            destination.Open();
            destination.Write( *fileAccess );
            fileAccess->Close();
            destination.Close();
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Wrote file to " + fsPath );
            return true;         
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Cannot obtain access to resource: " + fsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::GetPartialResource( const CORE::CUri& uri        ,
                                             UInt64 byteOffset            ,
                                             Int64 bytesToGet             ,
                                             CORE::CIOAccess& destination )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    TBasicVfsResourcePtr file = CVfsGlobal::Instance()->GetVfs().GetFile( fsPath, "rb", false );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( !fileAccess.IsNULL() ) 
        {
            fileAccess->Open();
            // Seek follows fseek(): zero is success
            if ( 0 == fileAccess->Seek( byteOffset, SEEK_SET ) )
            {            
                destination.Open();
                destination.Write( *fileAccess, bytesToGet );            
                destination.Close();
                fileAccess->Close();
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetPartialResource: Wrote partial file to " + fsPath );
                return true;
            }
            fileAccess->Close();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:GetPartialResource: Failed to seek to offset " << byteOffset << " in file : " << fsPath );
            return false;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetResource: Cannot obtain access to resource: " + fsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CVfsUriResourceAccessor::GetResourceAccess( const CORE::CUri& uri               ,
                                            CORE::IOAccessPtr& accessToResource ,
                                            TURI_RESOURCEACCESS_MODE mode       )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:GetResourceAccess: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    const char* accessMode = ResourceAccessModeStr( mode );
    bool overwriteResource = IsOverwriteResourceAccessMode( mode );

    TBasicVfsResourcePtr file = CVfsGlobal::Instance()->GetVfs().GetFile( fsPath, accessMode, overwriteResource );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( !fileAccess.IsNULL() ) 
        {
            fileAccess->Open();

            accessToResource = fileAccess;
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:GetResourceAccess: Obtained access to file " + fsPath );
            return true;         
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:GetResource: Cannot obtain access to resource: " + fsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::GetResourceMetaData( const CORE::CUri& uri             ,
                                              CORE::CResourceMetaData& metaData )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:GetSupportedOperations: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    return CVfsGlobal::Instance()->GetVfs().GetFileMetaData( fsPath, metaData );
}

/*-------------------------------------------------------------------------*/

bool 
CVfsUriResourceAccessor::UpdateResource( const CORE::CUri& uri           ,
                                         CORE::CDynamicBuffer& srcBuffer )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdateResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    return CVfsGlobal::Instance()->GetVfs().StoreAsFile( fsPath, srcBuffer, 0, true );
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::UpdateResource( const CORE::CUri& uri ,
                                         CORE::CIOAccess& src  )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdateResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    TBasicVfsResourcePtr file = CVfsGlobal::Instance()->GetVfs().GetFile( fsPath, "wb", true );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( !fileAccess.IsNULL() )  
        {
            fileAccess->Open();
            fileAccess->Write( src );
            fileAccess->Close();
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdateResource: Appended partial file to " + fsPath );
            return true;         
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdateResource: Cannot obtain access to resource: " + fsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::UpdatePartialResource( const CORE::CUri& uri ,
                                                UInt64 byteOffset     ,
                                                Int64 bytesToWrite    ,
                                                CORE::CIOAccess& src  )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    TBasicVfsResourcePtr file = CVfsGlobal::Instance()->GetVfs().GetFile( fsPath, "rb+", false );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( !fileAccess.IsNULL() ) 
        {
            fileAccess->Open();
            // Seek follows fseek(): zero is success
            if ( 0 == fileAccess->Seek( byteOffset, SEEK_SET ) )
            {            
                src.Open();
                fileAccess->Write( src, bytesToWrite );            
                src.Close();
                fileAccess->Close();
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Wrote file to " + fsPath );
                return true;
            }
            fileAccess->Close();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:UpdatePartialResource: Failed to seek to offset " << byteOffset << " in file : " << fsPath );
            return false;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Cannot obtain access to resource: " + fsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::UpdatePartialResource( const CORE::CUri& uri    ,
                                                CORE::CIOAccess& src     )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    TBasicVfsResourcePtr file = CVfsGlobal::Instance()->GetVfs().GetFile( fsPath, "wb+", true ); // we will use append mode
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( !fileAccess.IsNULL() ) 
        {
            fileAccess->Open();
            fileAccess->Write( src );
            fileAccess->Close();
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Appended partial file to " + fsPath );
            return true;         
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemUriResourceAccessor:UpdatePartialResource: Cannot obtain access to resource: " + fsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::GetSupportedOperations( const CORE::CUri& uri               ,
                                                 CUriResourceAccessorOperations& ops )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:GetSupportedOperations: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    bool fileExists = CVfsGlobal::Instance()->GetVfs().FileExists( fsPath );

    ops.createResource = !fileExists;
    ops.deleteResource = fileExists;
    ops.getPartialResource = fileExists;
    ops.getResource = fileExists;
    ops.getResourceMetaData = fileExists;
    ops.updatePartialResource = fileExists;
    ops.updateResource = fileExists;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::DeleteResource( const CORE::CUri& uri )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:DeleteResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    return CVfsGlobal::Instance()->GetVfs().DeleteFile( uri.GetAuthorityAndPath(), false );
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::IsACollectionResource( const CORE::CUri& uri ) const
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:IsACollectionResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    
    // Within this context a VFS directory is a collection
    // The directory needs to already exist for us to know if the URI is pointing at a directory
    return CVfsGlobal::Instance()->GetVfs().DirExists( fsPath );
}

/*-------------------------------------------------------------------------*/

bool
CVfsUriResourceAccessor::ResolveResourcesInCollection( const CORE::CUri& uri  ,
                                                       UriVector& resources   ,
                                                       bool recursive         ,
                                                       bool addCollectionUris )
{GUCEF_TRACE;

    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsUriResourceAccessor:ResolveResourcesInCollection: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString fsPath = uri.GetAuthorityAndPath();
    bool totalSuccess = true;

    if ( recursive && addCollectionUris )
    {
        CVFS::TStringVector dirList;    
        totalSuccess = CVfsGlobal::Instance()->GetVfs().GetDirList( dirList, fsPath, recursive, true, CString::EmptyStringSet ) && totalSuccess;
        resources.reserve( dirList.size() );

        CVFS::TStringVector::iterator i = dirList.begin();
        while ( i != dirList.end() )
        {
            CORE::CUri subDirUri;
            subDirUri.SetScheme( SchemeName );
            subDirUri.SetPath( (*i) );

            resources.push_back( subDirUri );
        
            ++i;
        }
    }

    CVFS::TStringVector fileList;    
    totalSuccess = CVfsGlobal::Instance()->GetVfs().GetFileList( fileList, fsPath, recursive, true, CString::EmptyStringSet ) && totalSuccess;
    resources.reserve( resources.capacity() + fileList.size() );

    CVFS::TStringVector::iterator n = fileList.begin();
    while ( n != fileList.end() )
    {
        CORE::CUri fileUri;
        fileUri.SetScheme( SchemeName );
        fileUri.SetPath( (*n) );

        resources.push_back( fileUri );
        
        ++n;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
