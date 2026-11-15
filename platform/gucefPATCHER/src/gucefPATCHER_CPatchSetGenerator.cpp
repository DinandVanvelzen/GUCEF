/*
 *  gucefPATCHER: GUCEF RAD module providing a patch delivery system
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

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#include "dvstrutils.h"

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#include "gucefPATCHER_CPatchSetGenerator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPatchSetGenerator::CPatchSetGenerator( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPatchSetGenerator::~CPatchSetGenerator()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetGenerator::GeneratePatchSet( const CORE::CString& localRoot      ,
                                      const CORE::CString& URLRoot        ,
                                      TPatchSet& patchSet                 ,
                                      const TStringSet* dirsToIgnore      ,
                                      const TStringSet* fileTypesToIgnore ) const
{GUCEF_TRACE;

    CORE::CString subDir = CORE::LastSubDir( localRoot );

    if ( GUCEF_NULL != dirsToIgnore )
    {
        if ( dirsToIgnore->find( subDir ) != dirsToIgnore->end() )
        {
            // the current subDir is a dir we are instructed to ignore
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Ignoring dir as instructed: " + subDir );
            return false;
        }
    }

    CORE::CString URLRootDir = URLRoot + '/' + subDir.ReplaceSubstr( " ", "%20" );

    TDirEntry dirEntry;
    dirEntry.name = subDir;
    if ( GeneratePatchSet( localRoot         ,
                           URLRootDir        ,
                           dirEntry          ,
                           dirsToIgnore      ,
                           fileTypesToIgnore ) )
    {
        patchSet.push_back( dirEntry );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetGenerator::GeneratePatchSet( const CORE::CString& localRoot      ,
                                      const CORE::CString& URLRoot        ,
                                      TDirEntry& currentDir               ,
                                      const TStringSet* dirsToIgnore      ,
                                      const TStringSet* fileTypesToIgnore ) const
{GUCEF_TRACE;

    currentDir.sizeInBytes = 0;
    currentDir.hash = currentDir.name;

    struct CORE::SDI_Data* dirEntry = CORE::DI_First_Dir_Entry( localRoot.C_String() );
    if ( dirEntry != GUCEF_NULL )
    {
        do
        {
            CORE::CString entryName = CORE::DI_Name( dirEntry );
            if ( ( entryName != "." )  &&
                 ( entryName != ".." )  )
            {
                if ( 0 == CORE::DI_Is_It_A_File( dirEntry ) )
                {
                    // We found a sub-dir
                    if ( GUCEF_NULL != dirsToIgnore )
                    {
                        if ( dirsToIgnore->find( entryName ) != dirsToIgnore->end() )
                        {
                            // the current subDir is a dir we are instructed to ignore
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Ignoring dir as instructed: " + entryName );
                            continue;
                        }
                    }

                    // We can process this sub-dir
                    TDirEntry subDirs;
                    subDirs.name = entryName;

                    CORE::CString subDirPath = localRoot;
                    CORE::AppendToPath( subDirPath, subDirs.name );
                    CORE::CString URLRootPlusDir = URLRoot + '/' + subDirs.name.ReplaceSubstr( " ", "%20" );

                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Found sub-dir to generate patch set entries from: " + subDirPath );

                    if ( GeneratePatchSet( subDirPath        ,
                                           URLRootPlusDir    ,
                                           subDirs           ,
                                           dirsToIgnore      ,
                                           fileTypesToIgnore ) )
                    {
                        currentDir.sizeInBytes += subDirs.sizeInBytes;
                        currentDir.hash += subDirs.hash;
                        currentDir.subDirs.push_back( subDirs );
                    }
                }
                else
                {
                    // We found a file in this dir, check if we should ignore the file based on file type
                    if ( GUCEF_NULL != fileTypesToIgnore )
                    {
                        CORE::CString fileExtention = CORE::ExtractFileExtention( entryName );
                        if ( fileTypesToIgnore->find( fileExtention ) != fileTypesToIgnore->end() )
                        {
                            // the current file is of a type we are instructed to ignore
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Ignoring file based on it's type as instructed: " + entryName );
                            continue;
                        }
                    }

                    TFileEntry fileEntry;
                    fileEntry.name = entryName;
                    fileEntry.sizeInBytes = (UInt32) CORE::DI_Size( dirEntry );
                    CORE::CString filePath = localRoot;
                    CORE::AppendToPath( filePath, fileEntry.name );

                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Found file to generate patch set entries from: " + filePath );

                    CORE::CFileAccess fileAccess( filePath, "rb" );
                    fileAccess.Open();

                    if ( fileAccess.IsValid() )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Opened file and commencing MD5 generation from: " + filePath );

                        UInt8 md5Digest[ 16 ];
                        if ( 0 == CORE::md5fromfile( fileAccess.CStyleAccess() ,
                                                     md5Digest                 ) )
                        {
                            // clean up our toys
                            CORE::DI_Cleanup( dirEntry );
                            return false;
                        }

                        fileEntry.hash = CORE::MD5ToString( md5Digest );
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Finished MD5 generation, producing " + fileEntry.hash + " from: " + filePath );
                        currentDir.hash += fileEntry.hash;
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Failed to open file for MD5 generation: " + filePath );
                    }
                    currentDir.sizeInBytes += fileEntry.sizeInBytes;

                    TFileLocation location;
                    location.URL = URLRoot + '/' + fileEntry.name.ReplaceSubstr( " ", "%20" );
                    fileEntry.fileLocations.push_back( location );
                    currentDir.files.push_back( fileEntry );
                }
            }
        }
        while ( CORE::DI_Next_Dir_Entry( dirEntry ) != 0 );

        // clean up our toys
        CORE::DI_Cleanup( dirEntry );

        currentDir.hash = CORE::StringToMD5String( currentDir.hash );

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetGenerator::GeneratePatchSet( const CORE::CString& localRoot      ,
                                      const CORE::CString& URLRoot        ,
                                      CORE::CDataNode& patchSet           ,
                                      const TStringSet* dirsToIgnore      ,
                                      const TStringSet* fileTypesToIgnore ) const
{GUCEF_TRACE;

    TPatchSet patchSetData;
    if ( GeneratePatchSet( localRoot         ,
                           URLRoot           ,
                           patchSetData      ,
                           dirsToIgnore      ,
                           fileTypesToIgnore ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Successfully completed patch set generation, generating data tree from raw data" );

        CPatchSetParser parser;
        return parser.ParsePatchSet( patchSetData ,
                                     patchSet     );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetGenerator::GeneratePatchSet( const CORE::CString& localRoot      ,
                                      const CORE::CString& URLRoot        ,
                                      const CORE::CString& storageCodec   ,
                                      CORE::CIOAccess& patchSetStorage    ,
                                      const TStringSet* dirsToIgnore      ,
                                      const TStringSet* fileTypesToIgnore ) const
{GUCEF_TRACE;

    // Get the required codec for the patch set storage conversion
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Attempting to find storage codec \"" + storageCodec + "\"" );
    CORE::CDStoreCodecRegistry* codecRegistry = &CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
    if ( codecRegistry->IsRegistered( storageCodec ) )
    {
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr = codecRegistry->Lookup( storageCodec );
        if ( codecPtr )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Found storage codec \"" + storageCodec + "\", generating patch set" );

            CORE::CDataNode patchSet;
            if ( GeneratePatchSet( localRoot         ,
                                   URLRoot           ,
                                   patchSet          ,
                                   dirsToIgnore      ,
                                   fileTypesToIgnore ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Successfully completed patch set generation, serializing patch set" );

                // decode the data in our buffer into a data tree
                return codecPtr->StoreDataTree( &patchSet        ,
                                                &patchSetStorage );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Failed to complete patch set generation" );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Cannot find storage codec \"" + storageCodec + "\" to use for serializing the patch set" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PatchSetGenerator: Found storage codec \"" + storageCodec + "\", serializing patch set" );
    }

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
