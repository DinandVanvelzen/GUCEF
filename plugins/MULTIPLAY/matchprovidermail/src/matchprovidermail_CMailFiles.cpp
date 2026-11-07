/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H
#include "matchprovidermail_CMailMatchCodec.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H ? */

#include "matchprovidermail_CMailFiles.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CMailFiles::WriteAtomic( const CORE::CString& path, const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return false;
    // the directory of the file
    Int32 slash = path.HasChar( '/', false );
    const Int32 backslash = path.HasChar( '\\', false );
    if ( backslash > slash )
        slash = backslash;
    if ( slash > 0 )
    {
        const CORE::CString directory = path.SubstrToIndex( (UInt32) slash, true );
        if ( !CORE::DirExists( directory ) && !CORE::CreateDirs( directory ) )
            return false;
    }
    static const char nothing[ 1 ] = { 0 };
    return CORE::WriteFileAtomically( path, GUCEF_NULL == data ? static_cast< const void* >( nothing ) : data, size );
}

/*-------------------------------------------------------------------------*/

bool
CMailFiles::ReadBounded( const CORE::CString& path, UInt32 maxBytes, CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    bytes.Clear();
    if ( path.IsNULLOrEmpty() || !CORE::FileExists( path ) )
        return false;
    const UInt64 size = CORE::FileSize( path );
    if ( size > maxBytes )
        return false;
    if ( 0 == size )
        return true;
    CORE::CFileAccess file( path, "rb" );
    if ( !file.IsValid() )
        return false;
    bytes.SetDataSize( (UInt32) size );
    if ( file.Read( bytes.GetBufferPtr(), 1, (UInt32) size ) != (UInt32) size )
    {
        bytes.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailFiles::WriteRecord( const CORE::CString& path, const CORE::CIDataNodeSerializable& record )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    if ( !CMailMatchCodec::SerializeToBytes( record, bytes, 64 * 1024 * 1024 ) )
        return false;
    return WriteAtomic( path, bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CMailFiles::ReadRecord( const CORE::CString& path, CORE::CIDataNodeSerializable& record, UInt32 maxBytes )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    if ( !ReadBounded( path, maxBytes, bytes ) || 0 == bytes.GetDataSize() )
        return false;
    return CMailMatchCodec::DeserializeFromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize(), record, maxBytes );
}

/*-------------------------------------------------------------------------*/

void
CMailFiles::ListFiles( const CORE::CString& directory, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    names.clear();
    if ( directory.IsNULLOrEmpty() || !CORE::DirExists( directory ) )
        return;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( directory ) )
    {
        do
        {
            if ( iterator.IsAFile() )
                names.push_back( iterator.GetResourceName() );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
}

/*-------------------------------------------------------------------------*/

void
CMailFiles::ListDirectories( const CORE::CString& directory, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    names.clear();
    if ( directory.IsNULLOrEmpty() || !CORE::DirExists( directory ) )
        return;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( directory ) )
    {
        do
        {
            if ( iterator.IsADirectory() )
            {
                const CORE::CString name = iterator.GetResourceName();
                if ( name != "." && name != ".." )
                    names.push_back( name );
            }
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
