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

#ifndef GUCEF_CORE_CSUBIOACCESS_H
#define GUCEF_CORE_CSUBIOACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"        /* base class for all media manipulators */
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

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
 *  Read-only window onto a byte range of another IO access object.
 *
 *  Where CSubFileAccess exposes a range of a file on disk, this class exposes a range of anything
 *  that has an IO access: a disc image, a memory block, a file descriptor, another window.
 *  This is what allows an archive inside an archive without extracting it first.
 *
 *  Every window keeps its own position, the parent is re-positioned before every read.
 *  As a result several windows can share one parent, but the windows must not be used
 *  concurrently from different threads as the parent itself is not synchronized.
 *  Offsets and sizes are 64 bit.
 */
class GUCEF_CORE_PUBLIC_CPP CSubIOAccess : public CIOAccess
{
    public:

    typedef CTSharedPtr< CSubIOAccess, MT::CMutex >     SubIOAccessPtr;

    CSubIOAccess( void );

    /** Same as default construction followed by Load() */
    CSubIOAccess( IOAccessPtr parent ,
                  const UInt64 offset ,
                  const UInt64 size   );

    virtual ~CSubIOAccess() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Defines the window. The window must lie within the parent, if the size of the parent
     *  is known and the window exceeds it this fails and nothing is changed.
     */
    bool Load( IOAccessPtr parent  ,
               const UInt64 offset ,
               const UInt64 size   );

    /** Offset of the start of the window in the parent */
    UInt64 GetOffset( void ) const;

    /** The IO access the window looks at */
    IOAccessPtr GetParent( void ) const;

    /**
     *  Rewinds the window to its start. A window cannot be reopened once closed.
     */
    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Releases the parent, the window is unusable afterwards
     */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Read( void *dest      ,
                         UInt32 esize    ,
                         UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    /** Windows are read only, always returns 0 */
    virtual UInt32 Write( const void* srcdata ,
                          UInt32 esize        ,
                          UInt32 elements     ) GUCEF_VIRTUAL_OVERRIDE;

    /** Position relative to the start of the window */
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  @return 0 on success, non-zero if the target lies outside of the window in which case the position is unchanged
     */
    virtual Int32 Seek( Int64 offset ,
                        Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;

    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Creates a new window on the same parent range with its own position */
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CSubIOAccess( const CSubIOAccess& src );
    CSubIOAccess& operator=( const CSubIOAccess& src );

    private:

    IOAccessPtr m_parent;
    UInt64 m_offset;
    UInt64 m_size;
    UInt64 m_pos;
};

typedef CSubIOAccess::SubIOAccessPtr    SubIOAccessPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSUBIOACCESS_H ? */
