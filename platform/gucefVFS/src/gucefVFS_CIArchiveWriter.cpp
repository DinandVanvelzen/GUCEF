/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#include "gucefVFS_CIArchiveWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CArchiveWriterEntrySettings::CArchiveWriterEntrySettings( void )
    : compression( COMPRESSION_STORE )
    , compressionLevel( -1 )
    , modificationTime( CORE::CDateTime::NowLocalDateTime() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIArchiveWriter::CIArchiveWriter( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIArchiveWriter::~CIArchiveWriter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIArchiveWriter::AddFileFromPath( const CORE::CString& nameInArchive           ,
                                  const CORE::CString& filePath                ,
                                  const CArchiveWriterEntrySettings& settings )
{GUCEF_TRACE;

    CORE::CFileAccess file( filePath, "rb" );
    if ( !file.IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ArchiveWriter: Unable to open file \"" + filePath + "\" to add it to the archive as \"" + nameInArchive + "\"" );
        return false;
    }

    CArchiveWriterEntrySettings fileSettings( settings );
    fileSettings.modificationTime = CORE::GetFileModificationTime( filePath );
    return AddFile( nameInArchive, file, fileSettings );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
