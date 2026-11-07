/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include "gucefVFS_CArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CArchive::CArchive( void )
    : CORE::CObservingNotifier()
    , CORE::CIDirectoryWatcher()
    , CORE::CTDynamicDestructorBase< CVFSHandle >() 
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CArchive::CArchive( const CArchive& src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CArchive::~CArchive()
{GUCEF_TRACE;

}
    
/*-------------------------------------------------------------------------*/
    
CArchive&
CArchive::operator=( const CArchive& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

TBasicVfsResourcePtr 
CArchive::GetFileAs( const CString& file                     ,
                     const CORE::CResourceMetaData& metaData ,
                     const char* mode                        ,
                     const UInt32 memLoadSize                ,
                     const bool overwrite                    )
{GUCEF_TRACE;

    return TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

bool 
CArchive::AddDirToWatch( const CString& dirToWatch       ,
                         const CDirWatchOptions& options ,
                         bool tryToCreatePathIfNotExists )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CArchive::RemoveDirToWatch( const CString& dirToWatch )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CArchive::RemoveAllWatches( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CArchive::GetAllWatchedDirs( CString::StringSet& dirs ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CArchive::IsDirectoryWatchingSupported( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/
 
bool
CArchive::IsHealthy( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CArchive::IsConnected( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CArchive::SetFileMetaData( const CString& filePath                 ,
                           const CORE::CResourceMetaData& metaData )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CArchive::MoveFileAtomically( const CString& oldFilePath ,
                              const CString& newFilePath ,
                              const bool overwrite       )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CArchive::TryResolveSpecialDir( CORE::TSpecialDirs dir, CString& resolvedPath ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CArchive::CreateDir( const CString& dirPath )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CArchive::DeleteDir( const CString& dirPath, bool deleteContents )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CArchive::MoveDir( const CString& oldDirPath, const CString& newDirPath )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
