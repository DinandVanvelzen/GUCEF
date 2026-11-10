/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
 *  Copyright (C) 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEENTRYIOACCESS_H
#define GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEENTRYIOACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#include "vfspluginLIBARCHIVE_macros.h"
#define GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H ? */

struct archive;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Read only access to the data of a single archive entry, streamed from the archive.
 *  Seeking forward skips data, seeking backward restarts from the beginning of the entry.
 */
class CLibArchiveEntryIOAccess : public CORE::CIOAccess
{
    public:

    /**
     *  Positions a new reader of the archive at the given path at the entry with the given index
     *  Check IsValid() afterwards
     */
    CLibArchiveEntryIOAccess( const CORE::CString& archivePath ,
                              UInt32 entryIndex                ,
                              UInt64 entrySize                 );

    virtual ~CLibArchiveEntryIOAccess() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CLibArchiveEntryIOAccess( const CLibArchiveEntryIOAccess& src );            /**< not supported */
    CLibArchiveEntryIOAccess& operator=( const CLibArchiveEntryIOAccess& src ); /**< not supported */

    bool PositionAtEntry( void );
    void Release( void );
    bool SkipForward( UInt64 bytes );

    CORE::CString m_archivePath;
    UInt32 m_entryIndex;
    UInt64 m_size;
    UInt64 m_pos;
    struct archive* m_archive;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEENTRYIOACCESS_H ? */
