/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include <string>
#include <vector>

#include "AncestryBackup_CZipExtractor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const RootName = "AncestryBackupZipRoot";
static const char* const ArchiveName = "AncestryBackupZip";
static const char* const MountPoint = "AncestryBackupZipContents";

/*-------------------------------------------------------------------------*/

bool
CZipExtractor::IsAvailable( void )
{GUCEF_TRACE;

    VFS::CVFS::TAbstractArchiveFactory::TKeySet types;
    VFS::CVfsGlobal::Instance()->GetVfs().GetListOfSupportedArchiveTypes( types );
    return types.find( "zip" ) != types.end();
}

/*-------------------------------------------------------------------------*/

static bool
EndsWithNoCase( const std::string& text, const std::string& suffix )
{
    if ( text.size() < suffix.size() )
        return false;
    for ( size_t i=0; i<suffix.size(); ++i )
        if ( tolower( text[ text.size() - suffix.size() + i ] ) != tolower( suffix[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CZipExtractor::ExtractFirst( const CORE::CString& zipPath        ,
                             const CORE::CString& extension      ,
                             const CORE::CString& destinationDir ,
                             CORE::CString& extractedPath        )
{GUCEF_TRACE;

    extractedPath = CORE::CString::Empty;

    if ( !IsAvailable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Zip files cannot be opened, the archive plugin (vfspluginLIBARCHIVE) is not loaded" );
        return false;
    }
    if ( !CORE::FileExists( zipPath ) )
        return false;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.UnmountArchiveByName( ArchiveName );
    vfs.UnmountArchiveByName( RootName );

    // the directory the zip is in becomes a root of the virtual file system, the zip in it an archive
    const CORE::CString zipDir = CORE::StripFilename( zipPath );
    const CORE::CString zipName = CORE::ExtractFilename( zipPath );
    bool ok = vfs.AddRoot( zipDir, RootName, false, false, RootName ) &&
              vfs.MountArchive( ArchiveName, CORE::CString( RootName ) + "/" + zipName, "zip", MountPoint, false, false );

    CORE::CString entry;
    if ( ok )
    {
        VFS::CVFS::TStringVector files;
        const bool listed = vfs.GetFileList( files, MountPoint, true, false, CORE::CString::EmptyStringSet );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Listing the zip returned " + CORE::CString( listed ? "true" : "false" ) + " with " + CORE::UInt32ToString( (UInt32) files.size() ) + " entries" );
        for ( size_t i=0; i<files.size() && i<5; ++i )
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Zip entry: " + files[ i ] );
        for ( size_t i=0; i<files.size() && entry.IsNULLOrEmpty(); ++i )
            if ( EndsWithNoCase( files[ i ].STL_String(), extension.STL_String() ) )
                entry = CORE::CString( MountPoint ) + "/" + files[ i ];
        ok = !entry.IsNULLOrEmpty();
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: " + zipPath + " holds no file that ends with " + extension );
    }
    else
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Unable to open " + zipPath + " as a zip file" );

    if ( ok )
    {
        VFS::TBasicVfsResourcePtr resource = vfs.GetFile( entry, "rb" );
        CORE::IOAccessPtr access = resource.IsNULL() ? CORE::IOAccessPtr() : resource->GetAccess();
        ok = !access.IsNULL();
        if ( ok )
        {
            // the file keeps the name it has in the zip, without the directories
            std::string name = entry.STL_String();
            const size_t slash = name.find_last_of( "/\\" );
            if ( slash != std::string::npos )
                name = name.substr( slash + 1 );
            extractedPath = CORE::CombinePath( destinationDir, name.c_str() );

            ok = CORE::CreateDirs( destinationDir );
            const CORE::CString temp = extractedPath + ".new";
            {
                CORE::CFileAccess out( temp, "wb" );
                ok = ok && out.Opened();
                std::vector< char > buffer( 1024 * 1024 );
                while ( ok )
                {
                    const UInt32 got = access->Read( &buffer[ 0 ], 1, (UInt32) buffer.size() );
                    if ( got == 0 )
                        break;
                    ok = out.Write( &buffer[ 0 ], 1, got ) == got;
                }
                out.Flush();
                out.Close();
            }
            ok = ok && CORE::MoveFile( temp, extractedPath, true );
            if ( !ok )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Failed to extract " + entry + " from " + zipPath );
                CORE::DeleteFile( temp );
            }
        }
    }

    vfs.UnmountArchiveByName( ArchiveName );
    vfs.UnmountArchiveByName( RootName );
    if ( !ok )
        extractedPath = CORE::CString::Empty;
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
