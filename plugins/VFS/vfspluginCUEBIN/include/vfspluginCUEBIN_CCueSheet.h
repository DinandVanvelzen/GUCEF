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

#ifndef GUCEF_VFSPLUGIN_CUEBIN_CCUESHEET_H
#define GUCEF_VFSPLUGIN_CUEBIN_CCUESHEET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  What a CUE sheet says about a CD image: the files that hold the sectors and the tracks in them.
 *  Positions are in sectors (frames of 1/75 second) from the start of the file the track is in, as the INDEX lines
 *  give them. PREGAP and POSTGAP describe silence that is not in the files, so they do not move anything.
 */
class CCueSheet
{
    public:

    enum ETrackMode
    {
        TRACKMODE_UNKNOWN = 0 ,
        TRACKMODE_AUDIO       ,     /**< CD audio: 2352 bytes of 44.1 kHz 16 bit stereo little endian PCM per sector */
        TRACKMODE_MODE1_2048  ,     /**< data, only the 2048 bytes of user data per sector are in the file */
        TRACKMODE_MODE1_2352  ,     /**< data, raw sectors: sync and header (16), user data (2048), error correction (288) */
        TRACKMODE_MODE2_2336  ,     /**< CD-ROM XA without sync and header: subheader (8), form 1 user data (2048), ... */
        TRACKMODE_MODE2_2352        /**< CD-ROM XA raw: sync and header (16), subheader (8), form 1 user data (2048), ... */
    };
    typedef enum ETrackMode TTrackMode;

    struct STrack
    {
        UInt32 number;
        TTrackMode mode;
        UInt32 fileIndex;           /**< into GetFiles() */
        Int64 index0;               /**< start of the pregap that is in the file, -1 when there is none */
        UInt64 index1;              /**< start of the track itself */

        STrack( void );
    };
    typedef GUCEF::vector< STrack > TTrackVector;

    struct SFile
    {
        CORE::CString name;         /**< as written in the sheet, usually relative to the sheet */
        CORE::CString type;         /**< BINARY, MOTOROLA, WAVE, ... upper case */
    };
    typedef GUCEF::vector< SFile > TFileVector;

    CCueSheet( void );

    /** Reads a sheet. False, with the reason, when it is not a CUE sheet this plugin can use */
    bool Parse( const CORE::CString& text, CORE::CString& error );

    const TFileVector& GetFiles( void ) const;

    const TTrackVector& GetTracks( void ) const;

    /** Bytes per sector in the file */
    static UInt32 GetSectorSize( TTrackMode mode );

    /** Where the payload starts in a sector: the 2048 bytes of user data, or all of an audio sector */
    static UInt32 GetPayloadOffset( TTrackMode mode );

    /** Bytes of payload per sector: 2048 for data, 2352 for audio */
    static UInt32 GetPayloadSize( TTrackMode mode );

    static bool IsDataTrack( TTrackMode mode );

    /** "mm:ss:ff" (minutes, seconds, frames of 1/75 s) as a number of sectors */
    static bool ParseMsf( const CORE::CString& msf, UInt64& sectors );

    private:

    TFileVector m_files;
    TTrackVector m_tracks;
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

#endif /* GUCEF_VFSPLUGIN_CUEBIN_CCUESHEET_H ? */
