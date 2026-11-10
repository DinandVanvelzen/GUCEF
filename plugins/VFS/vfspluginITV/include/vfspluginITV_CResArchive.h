/*
 *  vfspluginZIP: Generic GUCEF VFS plugin for ZIP archives
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef VFSPLUGIN_ITV_CRESARCHIVE_H
#define VFSPLUGIN_ITV_CRESARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/


#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFSPLUGIN_ITV_MACROS_H
#include "vfspluginITV_macros.h"
#define GUCEF_VFSPLUGIN_ITV_MACROS_H
#endif /* GUCEF_VFSPLUGIN_ITV_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ITV {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CResArchive : public VFS::CArchive
{
    public:

    static const VFS::CString ResArchiveTypeName;
    
    CResArchive( void );

    virtual ~CResArchive();

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file          ,
                                               const char* mode = "rb"           ,
                                               const VFS::UInt32 memLoadSize = 0 ,
                                               const bool overwrite = false      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileList( TStringVector& outputList                                                   ,
                              const CORE::CString& mountLocation                                          , 
                              const CORE::CString& archiveLocation                                        ,
                              bool recursive = false                                                      ,
                              bool includePathInFilename = false                                          ,
                              const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                              UInt32 maxListEntries = GUCEF_UINT16MAX                                     ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                                   ,
                             const CORE::CString& mountLocation                                          , 
                             const CORE::CString& archiveLocation                                        ,
                             bool recursive = false                                                      ,
                             bool includeParentDirInName = false                                         ,
                             const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                             UInt32 maxListEntries = GUCEF_UINT16MAX                                     ) const GUCEF_VIRTUAL_OVERRIDE;

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

    /**
     *  Loads the archive from an already opened resource, which is either the index (IDX) or the resource data (RES) file.
     *  The other half of the pair is looked up next to it, in the VFS, using the path of the given resource.
     *  This is what allows the archive to live inside another archive, for example on a disc image.
     */
    virtual bool LoadArchive( const VFS::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Loads the archive from IO access objects for the index (IDX) and resource data (RES) files.
     *  The resource access is kept for the lifetime of the archive.
     */
    bool LoadArchive( const VFS::CString& archiveName ,
                      const VFS::CString& archivePath ,
                      CORE::IOAccessPtr idxAccess     ,
                      CORE::IOAccessPtr resAccess     );
    
    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const VFS::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    // IDX files seem to have a Int16 indicating the type of record followed by index records
    //
    // files:
    // SDEF: Star definitions
    // STR: misc string table
    // FDEF: Propulsion definitions
    // 16) SNDEF: Sensor definitions
    // 1) MATB: material (mineral/resource) gfx
    // 1) OTHERB: misc bitmaps?
    // 1) TUTB: tutorial gfx
    // 1) RBMP: ?
    // HDEF: Hull definitions
    // MOUSE: mouse mask?
    // 4) FONT: Font info/bitmap?
    // WDEF: Weapon definitions
    // 6) PDEF: Planet type definitions
    // 12) PPDEF: Propulsion definitions
    // 9) MDEF: Material (mineral/resource) definitions
    //
    // 10) Sound format:
    //  PCM stream. - Sampling freq: 22Khz
    //              - Sampling Depth: 8 bit unsigned
    //              - All mono (single channel)


    // seems to be 8 bytes per entry
    struct SIdxRecord
    {
        CORE::Int32 resourceNr; // number identifying the resource, this Id is global not per archive
        CORE::Int32 offset;     // offset of resource in res file
    };
    typedef struct SIdxRecord IdxRecord;
    typedef GUCEF::vector< IdxRecord > IndexVector;
    typedef GUCEF::map< CORE::Int32, size_t > IndexLookupMap;     // resource id -> position in the IndexVector

    struct SIdx
    {
        CORE::Int16 recordType;   // type of record stored in the archive
        IndexVector index;  // list of resources of the given type
    };
    typedef struct SIdx Idx;

    // Type 3 record as per IDX
    // text is followed by \r\n (0x0D 0x0A) followed by null terminator
    struct SStringRecord3
    {
        CORE::Int32 resourceNr;   // number identifying the resource, this Id is global not per archive
        CORE::Int8 a[ 51 ];       // don't know
        CORE::Int16 strLength;    // length of string to follow in chars/bytes
        VFS::CString text;  // text with the length of strLength
        char cr;            // carriage return char
        char lf;            // line feed char
        char nullTerm;      // null terminator
    };
    typedef struct SStringRecord3 StringRecord3;

    // Type 7 record as per IDX
    // caption is single line followed by null terminator
    // text is followed by \r\n (0x0D 0x0A) followed by null terminator
    struct SStringRecord7
    {
        CORE::Int32 resourceNr;   // number identifying the resource, this Id is global not per archive
        CORE::Int8 a[ 51 ];       // don't know
        CORE::UInt32 b;           // don't know
        CORE::Int32 captionTextLength;  // length of text block to follow in chars/bytes
        char* captionText;  // text with the length of strLength
        //char nullTerm;      // null terminator
        CORE::Int32 textLength;   // length of text block to follow in chars/bytes
        char* text;         // text with the length of strLength
        char cr;            // carriage return char
        char lf;            // line feed char
        char nullTerm;      // null terminator
    };
    typedef struct SStringRecord7 StringRecord7;

    CResArchive( const CResArchive& src );
    CResArchive& operator=( const CResArchive& src );

    CORE::IOAccessPtr
    LoadFile( const VFS::CString& file      ,
              const VFS::UInt32 memLoadSize ) const;

    bool LoadIndex( CORE::CIOAccess& idxAccess );

    /**
     *  Gives access to the resource data file that is safe to use independently of other users of the archive,
     *  if the underlying access cannot provide that the shared access is returned
     */
    CORE::IOAccessPtr GetResAccessForReading( void ) const;

    bool GetResourceInfo( CORE::Int32 resourceId ,
                          CORE::Int32& offset    ,
                          CORE::Int32& size      ) const;

    private:

    CORE::CString m_archiveName;
    CORE::CString m_archivePath;
    CORE::IOAccessPtr m_resAccess;
    VFS::TBasicVfsResourcePtr m_idxResource;
    VFS::TBasicVfsResourcePtr m_resResource;
    Idx m_index;
    IndexLookupMap m_indexLookup;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ITV */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* VFSPLUGIN_ITV_CZIPARCHIVE_H ? */
