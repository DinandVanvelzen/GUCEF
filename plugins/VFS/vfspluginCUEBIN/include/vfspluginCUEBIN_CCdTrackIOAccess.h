/*
 *  vfspluginCUEBIN: Generic GUCEF VFS plugin for CD images described by a CUE sheet (BIN/CUE)
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

#ifndef GUCEF_VFSPLUGIN_CUEBIN_CCDTRACKIOACCESS_H
#define GUCEF_VFSPLUGIN_CUEBIN_CCDTRACKIOACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFSPLUGIN_CUEBIN_MACROS_H
#include "vfspluginCUEBIN_macros.h"
#define GUCEF_VFSPLUGIN_CUEBIN_MACROS_H
#endif /* GUCEF_VFSPLUGIN_CUEBIN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace CUEBIN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Read only access to the payload of a run of CD sectors in an image file, as one contiguous stream: the 2048 bytes of
 *  user data of each data sector (what a CD-ROM drive returns), or the PCM of audio sectors. A header (a WAVE header
 *  for an audio track, for instance) can be put in front. Each access opens the image file itself, so accesses can be
 *  used independently. Positioning follows fseek() and fsetpos().
 */
class CCdTrackIOAccess : public CORE::CIOAccess
{
    public:

    struct SLayout
    {
        UInt64 fileOffset;          /**< byte offset of the first sector in the file */
        UInt64 sectorCount;
        UInt32 sectorSize;          /**< bytes per sector in the file */
        UInt32 payloadOffset;       /**< where the payload is in a sector */
        UInt32 payloadSize;         /**< payload bytes per sector */

        SLayout( void );
    };

    CCdTrackIOAccess( const CORE::CString& imagePath, const SLayout& layout, const CORE::CDynamicBuffer& header = CORE::CDynamicBuffer() );

    virtual ~CCdTrackIOAccess() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;

    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CCdTrackIOAccess( const CCdTrackIOAccess& src );            /**< not implemented, use Clone() */
    CCdTrackIOAccess& operator=( const CCdTrackIOAccess& src ); /**< not implemented */

    private:

    CORE::CString m_imagePath;
    SLayout m_layout;
    CORE::CDynamicBuffer m_header;
    CORE::IOAccessPtr m_file;
    UInt64 m_size;
    UInt64 m_pos;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CUEBIN */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_CUEBIN_CCDTRACKIOACCESS_H ? */
