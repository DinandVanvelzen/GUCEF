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

#ifndef GUCEF_VFSPLUGIN_CUEBIN_CCUEBINARCHIVE_H
#define GUCEF_VFSPLUGIN_CUEBIN_CCUEBINARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CARCHIVE_H
#include "gucefVFS_CArchive.h"
#define GUCEF_VFS_CARCHIVE_H
#endif /* GUCEF_VFS_CARCHIVE_H ? */

#ifndef GUCEF_VFSPLUGIN_CUEBIN_CCUESHEET_H
#include "vfspluginCUEBIN_CCueSheet.h"
#define GUCEF_VFSPLUGIN_CUEBIN_CCUESHEET_H
#endif /* GUCEF_VFSPLUGIN_CUEBIN_CCUESHEET_H ? */

#ifndef GUCEF_VFSPLUGIN_CUEBIN_CCDTRACKIOACCESS_H
#include "vfspluginCUEBIN_CCdTrackIOAccess.h"
#define GUCEF_VFSPLUGIN_CUEBIN_CCDTRACKIOACCESS_H
#endif /* GUCEF_VFSPLUGIN_CUEBIN_CCDTRACKIOACCESS_H ? */

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
 *  Read only archive for a CD image described by a CUE sheet (archive type "cue"), mounted from the .cue file.
 *
 *  The files the sheet names are looked for next to the sheet, ignoring case. When one is not there, a file with the
 *  name of the sheet and the extension .bin is used, as images are often renamed after the sheet was made. A plain .bin
 *  is never mounted by itself: that extension is used for all kinds of binary dumps.
 *
 *  Contents: the file system of the first data track, read through an archive of type "iso9660" (vfspluginISO9660)
 *  over the 2048 byte user data of its sectors, whatever the sector format in the image; and every audio track as a
 *  WAVE file (44.1 kHz, 16 bit, stereo) in the directory CDDA, named TRACKnn.WAV after the track number. Lookups
 *  ignore case.
 */
class CCueBinArchive : public VFS::CArchive
{
    public:

    static const VFS::CString CueArchiveTypeName;

    /** The directory with the audio tracks */
    static const VFS::CString AudioTrackDirName;

    CCueBinArchive( void );

    virtual ~CCueBinArchive() GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file          ,
                                               const char* mode = "rb"           ,
                                               const VFS::UInt32 memLoadSize = 0 ,
                                               const bool overwrite = false      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileList( TStringVector& outputList                                                    ,
                              const VFS::CString& mountLocation                                            ,
                              const VFS::CString& archiveLocation                                          ,
                              bool recursive = false                                                       ,
                              bool includePathInFilename = false                                           ,
                              const VFS::CString::StringSet& nameFilters = VFS::CString::EmptyStringSet    ,
                              VFS::UInt32 maxListEntries = GUCEF_UINT16MAX                                 ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                                    ,
                             const VFS::CString& mountLocation                                            ,
                             const VFS::CString& archiveLocation                                          ,
                             bool recursive = false                                                       ,
                             bool includeParentDirInName = false                                          ,
                             const VFS::CString::StringSet& nameFilters = VFS::CString::EmptyStringSet    ,
                             VFS::UInt32 maxListEntries = GUCEF_UINT16MAX                                 ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeleteFile( const VFS::CString& filePath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MoveFile( const VFS::CString& oldFilePath ,
                           const VFS::CString& newFilePath ,
                           const bool overwrite            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetFileMetaData( const VFS::CString& filePath      ,
                                  CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FileExists( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DirExists( const VFS::CString& dirPath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::UInt64 GetFileSize( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::CString GetFileHash( const VFS::CString& file ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CDateTime GetFileModificationTime( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const VFS::CString& GetArchiveName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadArchive( const VFS::CArchiveSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadArchive( const VFS::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const VFS::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The sheet as read, for those who want to know about the tracks */
    const CCueSheet& GetCueSheet( void ) const;

    private:

    struct SAudioTrack
    {
        VFS::CString name;                      /**< TRACKnn.WAV */
        VFS::CString imagePath;
        CCdTrackIOAccess::SLayout layout;
        CORE::CDynamicBuffer header;            /**< the WAVE header */
        VFS::UInt64 size;                       /**< header plus PCM */
    };
    typedef GUCEF::map< VFS::CString, SAudioTrack > TAudioTrackMap;    /**< by lower case name */

    bool Load( const VFS::CString& archiveName, const VFS::CString& archivePath, const VFS::CString& sheetText, const VFS::CString& sheetPath );

    VFS::CString ResolveImageFile( const VFS::CString& sheetPath, const VFS::CString& fileName ) const;

    static VFS::CString NormalizePath( const VFS::CString& path );

    /** For a path in the audio directory: the track, else NULL */
    const SAudioTrack* FindAudioTrack( const VFS::CString& path ) const;

    /** True for paths the data track archive answers */
    bool IsDataPath( const VFS::CString& normalizedPath ) const;

    void ListAudioTracks( TStringVector& outputList, const VFS::CString& mountLocation, bool includePath,
                          const VFS::CString::StringSet& nameFilters, VFS::UInt32 maxListEntries ) const;

    CCueBinArchive( const CCueBinArchive& src );              /**< not implemented */
    CCueBinArchive& operator=( const CCueBinArchive& src );   /**< not implemented */

    private:

    VFS::CString m_archiveName;
    VFS::CString m_archivePath;
    CCueSheet m_sheet;
    TAudioTrackMap m_audioTracks;
    VFS::CVFS::TArchivePtr m_dataArchive;
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

#endif /* GUCEF_VFSPLUGIN_CUEBIN_CCUEBINARCHIVE_H ? */
