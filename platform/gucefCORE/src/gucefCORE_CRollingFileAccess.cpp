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

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"        
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h" 
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CRollingFileAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define DEFAULT_MAXFILESIZE_IN_BYTES    10485760 // 10 MB max file size threshold for rollover
#define DEFAULT_MAX_ROLLED_FILES        -1

/*-------------------------------------------------------------------------*/

CRollingFileAccess::CRollingFileAccess( const CString& file ,
                                        const char* mode    )
    : CIOAccess()
    , m_maxFileSize( DEFAULT_MAXFILESIZE_IN_BYTES )
    , m_currentFileIndex( 0 )
    , m_offset( 0 )    
    , m_baseFilename( file )
    , m_fileMode()
    , m_currentFile()
    , m_currentFileName()
    , m_maxRolloverFilesBeforeDeletion( DEFAULT_MAX_ROLLED_FILES )
{GUCEF_TRACE;

    if ( Open( file, mode ) )
    {
        PerformRolledoverFilesCleanup();
    }
}

/*-------------------------------------------------------------------------*/

CRollingFileAccess::CRollingFileAccess( void )
    : CIOAccess()
    , m_maxFileSize( DEFAULT_MAXFILESIZE_IN_BYTES )
    , m_currentFileIndex( 0 )
    , m_offset( 0 )    
    , m_baseFilename()
    , m_fileMode()
    , m_currentFile()
    , m_currentFileName()
    , m_maxRolloverFilesBeforeDeletion( DEFAULT_MAX_ROLLED_FILES )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRollingFileAccess::CRollingFileAccess( const CRollingFileAccess& src )
    : CIOAccess( src )
    , m_maxFileSize( src.m_maxFileSize )
    , m_currentFileIndex( src.m_currentFileIndex )
    , m_offset( 0 )    
    , m_baseFilename( src.m_baseFilename )
    , m_fileMode( src.m_fileMode )
    , m_currentFile()
    , m_currentFileName()
    , m_maxRolloverFilesBeforeDeletion( src.m_maxRolloverFilesBeforeDeletion )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRollingFileAccess::~CRollingFileAccess()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void 
CRollingFileAccess::SetRolloverSizeThreshold( UInt64 sizeThreshold )
{GUCEF_TRACE;

    m_maxFileSize = sizeThreshold;
}

/*-------------------------------------------------------------------------*/

UInt64 
CRollingFileAccess::GetRolloverSizeThreshold( void ) const
{GUCEF_TRACE;

    return m_maxFileSize;
}

/*-------------------------------------------------------------------------*/

void
CRollingFileAccess::GetExistingMatchingFiles( UInt64ToStringMap& files ) const
{GUCEF_TRACE;

    UInt32 currentFileIndex = m_currentFileIndex;
    CString filter = GenerateCurrentFilename( m_baseFilename, currentFileIndex, true );
    CString dirPath = StripFilename( m_baseFilename );

    CFileSystemIterator fsIterator;
    if ( fsIterator.FindFirst( filter ) )
    {
        do
        {
            if ( fsIterator.IsAFile() )
            {
                // The file being written is never a candidate, whatever its time says: after the wall clock was set
                // back it is the "oldest" one. File times have a resolution of a second here (st_ctime), so files of
                // the same second get keys of their own instead of replacing each other, which hid them from the count
                CResourceMetaData metaData;
                if ( fsIterator.TryReadMetaData( metaData ) && metaData.hasName && metaData.hasCreationDateTime &&
                     metaData.name != m_currentFileName )
                {
                    UInt64 key = metaData.creationDateTime.ToUnixEpochBasedTicksInMillisecs();
                    while ( files.find( key ) != files.end() )
                        ++key;
                    files[ key ] = CombinePath( dirPath, metaData.name );
                }
            }
        }
        while ( fsIterator.FindNext() );
    }
}

/*-------------------------------------------------------------------------*/

CString
CRollingFileAccess::GenerateCurrentFilename( const CString& baseName, UInt32& fileIndex, bool asWildcardFilter ) const
{GUCEF_TRACE;

    CString actualFilename = ResolveVars( baseName );
    CString fileExt = actualFilename.SubstrToChar( '.', false );
    if ( !fileExt.IsNULLOrEmpty() )
    {
        actualFilename = actualFilename.CutChars( fileExt.Length()+1, false );
        fileExt = '.' + fileExt;
    }
    
    CString testFilename;
    if ( !asWildcardFilter )
    {
        testFilename = actualFilename + '.' + UInt32ToString( fileIndex ) + fileExt;    
        while ( FileExists( testFilename ) )
        {           
            ++fileIndex;
            testFilename = actualFilename + '.' + UInt32ToString( fileIndex ) + fileExt;
        }
    }
    else
    {
        testFilename = actualFilename + ".*" + fileExt;
    }

    return testFilename;
}

/*-------------------------------------------------------------------------*/

bool 
CRollingFileAccess::Open( const CString& file ,
                          const char* mode    )
{GUCEF_TRACE;

    if ( !m_currentFile.Opened() )
    {    
        m_baseFilename = file;
        m_fileMode = mode;

        CString actualFilename = GenerateCurrentFilename( file, m_currentFileIndex, false );
        if ( m_currentFile.Open( actualFilename, mode ) )
        {
            m_currentFileName = ExtractFilename( actualFilename );
            m_offset = m_currentFile.Tell();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void 
CRollingFileAccess::Open( void )
{GUCEF_TRACE;

    m_currentFile.Open();
    m_offset = m_currentFile.Tell();
}

/*-------------------------------------------------------------------------*/

void
CRollingFileAccess::Close( void )
{GUCEF_TRACE;

    m_currentFile.Close();
}

/*-------------------------------------------------------------------------*/

bool
CRollingFileAccess::Opened( void ) const
{GUCEF_TRACE;

    return m_currentFile.Opened();
}

/*-------------------------------------------------------------------------*/

CString 
CRollingFileAccess::ReadLine( void )
{GUCEF_TRACE;

    CString result = m_currentFile.ReadLine();
    m_offset = m_currentFile.Tell();
    return result;
}

/*-------------------------------------------------------------------------*/


CString
CRollingFileAccess::ReadString( void )
{GUCEF_TRACE;

    CString result = m_currentFile.ReadString();
    m_offset = m_currentFile.Tell();
    return result;
}

/*-------------------------------------------------------------------------*/

UInt32
CRollingFileAccess::Read( void *dest      ,
                          UInt32 esize    ,
                          UInt32 elements )
{GUCEF_TRACE;

    UInt32 result = m_currentFile.Read( dest, esize, elements );
    m_offset += result * esize;
    return result;
}

/*-------------------------------------------------------------------------*/

void
CRollingFileAccess::PerformRolledoverFilesCleanup( void )
{GUCEF_TRACE;

    if ( m_maxRolloverFilesBeforeDeletion < 1 )
        return;

    // the files other than the one being written, oldest first: with it at most the maximum remain
    UInt64ToStringMap files;
    GetExistingMatchingFiles( files );
    const size_t othersToKeep = (size_t) m_maxRolloverFilesBeforeDeletion - ( m_currentFile.Opened() ? 1 : 0 );

    if ( files.size() > othersToKeep )
    {    
        CString::StringVector failedFiles;
        CString::StringVector deletedFiles;
        deletedFiles.reserve( files.size() - othersToKeep );

        while ( files.size() > othersToKeep )
        {
            const CString& fileToDelete = (*files.begin()).second;
            if ( DeleteFile( fileToDelete ) )
            {
                deletedFiles.push_back( fileToDelete );
            }
            else
            {
                failedFiles.push_back( fileToDelete );
            }
            files.erase( files.begin() );
        }

        CString::StringVector::iterator i = failedFiles.begin();
        while ( i != failedFiles.end() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "RollingFileAccess: Failed to GUCEF_DELETE rolled over file: " + (*i) );
            ++i;
        }

        i = deletedFiles.begin();
        while ( i != deletedFiles.end() )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "RollingFileAccess: Deleted rolled over file: " + (*i) );
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRollingFileAccess::EnforceFileSizeThreshold( void )
{GUCEF_TRACE;

    if ( m_offset >= m_maxFileSize )
    {
        Close();
        if ( Open( m_baseFilename, m_fileMode.C_String() ) )
        {
            PerformRolledoverFilesCleanup();
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CRollingFileAccess::Write( const void* srcdata ,
                           UInt32 esize        ,
                           UInt32 elements     )
{GUCEF_TRACE;

    UInt32 elementsWritten = m_currentFile.Write( srcdata, esize, elements );
    m_offset += elementsWritten * esize;
    EnforceFileSizeThreshold();
    return elementsWritten;
}

/*-------------------------------------------------------------------------*/

UInt64
CRollingFileAccess::Write( CIOAccess& sourceData, Int64 bytesToWrite )
{GUCEF_TRACE;

    UInt64 bytesWritten = m_currentFile.Write( sourceData, bytesToWrite );
    m_offset += bytesWritten;
    EnforceFileSizeThreshold();
    return bytesWritten;
}

/*-------------------------------------------------------------------------*/
        
UInt32 
CRollingFileAccess::Write( const CString& string ,
                           bool addEol           )
{GUCEF_TRACE;

    UInt32 bytesWritten = m_currentFile.Write( string, addEol );
    m_offset += bytesWritten;
    EnforceFileSizeThreshold();
    return bytesWritten;
}

/*-------------------------------------------------------------------------*/

UInt64
CRollingFileAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_offset = m_currentFile.Tell();
}

/*-------------------------------------------------------------------------*/

UInt64
CRollingFileAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_currentFile.GetSize();
}

/*-------------------------------------------------------------------------*/

Int32
CRollingFileAccess::Seek( Int64 offset ,
                          Int32 origin )
{GUCEF_TRACE;

    UInt32 retVal = m_currentFile.Seek( offset, origin );
    if ( 0 == retVal ) // 0 indicates success
    {
        m_offset = m_currentFile.Tell();
    }
    return retVal;
}

/*-------------------------------------------------------------------------*/

UInt32
CRollingFileAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    UInt32 retVal = m_currentFile.Setpos( position );
    if ( 0 == retVal ) // 0 indicates success
    {
        m_offset = position;
    }
    return retVal;
}

/*-------------------------------------------------------------------------*/

char
CRollingFileAccess::GetChar( void )
{GUCEF_TRACE;

    return m_currentFile.GetChar();
}

/*-------------------------------------------------------------------------*/

bool
CRollingFileAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_currentFile.Eof();
}

/*-------------------------------------------------------------------------*/

bool
CRollingFileAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return m_currentFile.IsReadable();
}

/*-------------------------------------------------------------------------*/

bool
CRollingFileAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return m_currentFile.IsWriteable();
}

/*-------------------------------------------------------------------------*/

bool
CRollingFileAccess::IsValid( void )
{GUCEF_TRACE;

    return m_currentFile.IsValid();
}

/*-------------------------------------------------------------------------*/

void
CRollingFileAccess::Flush( void )
{GUCEF_TRACE;

    m_currentFile.Flush();
}

/*-------------------------------------------------------------------------*/

CICloneable*
CRollingFileAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRollingFileAccess( *this );
}

/*-------------------------------------------------------------------------*/

bool
CRollingFileAccess::SetFileToUse( const CString& filename  ,
                                  const char* mode         ,
                                  bool moveIfCurrentlyOpen )
{GUCEF_TRACE;

    m_baseFilename = filename;
    UInt32 index = m_currentFileIndex;
    CString actualFilename = GenerateCurrentFilename( filename, index, false );
    bool result = m_currentFile.SetFileToUse( actualFilename, mode, moveIfCurrentlyOpen );
    if ( result && moveIfCurrentlyOpen )
    {
        m_currentFileIndex = index;
        if ( m_currentFile.Opened() )
        {
            m_offset = m_currentFile.Tell();
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/
    
const CString& 
CRollingFileAccess::GetFilename( void ) const
{GUCEF_TRACE;

    return m_currentFile.GetFilename();
}

/*-------------------------------------------------------------------------*/

void
CRollingFileAccess::SetMaxRolloverFilesBeforeDeletion( Int32 deleteThreshold )
{GUCEF_TRACE;
    
    m_maxRolloverFilesBeforeDeletion = deleteThreshold;
    if ( m_currentFile.Opened() )
        PerformRolledoverFilesCleanup();
}

/*-------------------------------------------------------------------------*/

Int32
CRollingFileAccess::GetMaxRolloverFilesBeforeDeletion( void ) const
{GUCEF_TRACE;
    
    return m_maxRolloverFilesBeforeDeletion;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
