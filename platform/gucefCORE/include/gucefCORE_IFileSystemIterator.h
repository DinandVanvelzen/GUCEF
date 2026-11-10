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

#ifndef GUCEF_CORE_IFILESYSTEMITERATOR_H
#define GUCEF_CORE_IFILESYSTEMITERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
 *  The interface level of an iterator over the entries of a directory, whatever file system it belongs to.
 *  FindFirst() starts at the first entry of the directory (false: none, or the directory cannot be read), FindNext() moves to the
 *  next one (false: no more). While positioned on an entry the other functions describe it. The entries "." and ".." are not
 *  given.
 */
class GUCEF_CORE_PUBLIC_CPP IFileSystemIterator
{
    public:

    IFileSystemIterator( void );

    virtual ~IFileSystemIterator();

    virtual bool FindFirst( const CString& dirPath ) = 0;

    virtual bool FindNext( void ) = 0;

    virtual bool FindClose( void ) = 0;

    virtual bool IsADirectory( void ) const = 0;

    virtual bool IsAFile( void ) const = 0;

    virtual bool IsSymlink( void ) const = 0;

    /** The name of the entry, without its directory */
    virtual CString GetResourceName( void ) const = 0;

    /** The full path of the entry, in the paths of its file system */
    virtual CString GetResourcePath( void ) const = 0;

    /** What is known about the entry: size, times, flags. False when nothing could be read */
    virtual bool TryReadMetaData( CResourceMetaData& metaData ) = 0;

    private:

    IFileSystemIterator( const IFileSystemIterator& src );
    IFileSystemIterator& operator=( const IFileSystemIterator& src );
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< IFileSystemIterator, MT::CMutex > TFileSystemIteratorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_IFILESYSTEMITERATOR_H ? */
