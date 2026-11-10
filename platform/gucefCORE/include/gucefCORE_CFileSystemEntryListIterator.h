/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_IFILESYSTEMITERATOR_H
#include "gucefCORE_IFileSystemIterator.h"
#define GUCEF_CORE_IFILESYSTEMITERATOR_H
#endif /* GUCEF_CORE_IFILESYSTEMITERATOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An iterator over entries that were read before: a directory listing held in memory. Used for listings read on another
 *  thread (IFileSystemAccess::TryGetFileSystemIteratorAsync), by file systems that list a directory in one go, and by tests.
 *  FindFirst() starts at the first of the entries; the directory it is given is not read again.
 */
class GUCEF_CORE_PUBLIC_CPP CFileSystemEntryListIterator : public IFileSystemIterator
{
    public:

    /** One entry of the listing */
    class GUCEF_CORE_PUBLIC_CPP CEntry
    {
        public:

        CString name;
        CString path;
        bool isDirectory;
        bool isFile;
        bool isSymlink;
        bool hasMetaData;
        CResourceMetaData metaData;

        CEntry( void );
    };
    typedef GUCEF::vector< CEntry > TEntryVector;

    CFileSystemEntryListIterator( void );

    CFileSystemEntryListIterator( const CString& dirPath, const TEntryVector& entries );

    virtual ~CFileSystemEntryListIterator() GUCEF_VIRTUAL_OVERRIDE;

    void SetEntries( const CString& dirPath, const TEntryVector& entries );

    void AddEntry( const CEntry& entry );

    const TEntryVector& GetEntries( void ) const;

    const CString& GetDirPath( void ) const;

    /**
     *  Reads every entry of a directory through another iterator, with its metadata. The iterator cannot tell an empty
     *  directory from one it cannot read: both give no entries, so check the directory first (IFileSystemAccess does).
     */
    static bool TryReadAll( IFileSystemIterator& source, const CString& dirPath, TEntryVector& entries );

    virtual bool FindFirst( const CString& dirPath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindNext( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindClose( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsADirectory( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAFile( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSymlink( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetResourceName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetResourcePath( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool TryReadMetaData( CResourceMetaData& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    const CEntry* GetCurrent( void ) const;

    CFileSystemEntryListIterator( const CFileSystemEntryListIterator& src );
    CFileSystemEntryListIterator& operator=( const CFileSystemEntryListIterator& src );

    CString m_dirPath;
    TEntryVector m_entries;
    size_t m_index;
    bool m_iterating;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */
