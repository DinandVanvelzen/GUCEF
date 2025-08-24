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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#define GUCEF_CORE_DVCPPFILEUTILS_H

/*-------------------------------------------------------------------------*/

/*
 *      The utilities are meant to be cross-platform replacements for
 *      O/S dependent file utilities. Compile time build switches are used
 *      to direct these functions to the O/S native versions of the function.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include  <time.h>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"     /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"     /* simple types used */
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#undef MoveFile
#undef CopyFile
#undef DeleteFile

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Function which resolves an Operating System 'special' directory to a path
 *  The path is written into resolvedPath. If an error occurs false is returned.
 */
GUCEF_CORE_PUBLIC_CPP bool
TryResolveSpecialDir( TSpecialDirs dir, CString& resolvedPath );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the modification time of the indicated resource
 */
GUCEF_CORE_PUBLIC_CPP CDateTime
GetFileModificationTime( const CString& path );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the creation time of the indicated resource
 */
GUCEF_CORE_PUBLIC_CPP CDateTime
GetFileCreationTime( const CString& path );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to set various file meta-data using the file system
 *  The implementation will apply the meta-data 'best effort' based on the constraints of the actual underlying storage
 *  and return true if within the constraints of the file system the meta-data was applied successfully.
 */
GUCEF_CORE_PUBLIC_CPP bool
SetFileMetaData( const CString& filePath           ,
                 const CResourceMetaData& metaData );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to obtain various file meta-data from the file system
 */
GUCEF_CORE_PUBLIC_CPP bool
GetFileMetaData( const CString& filePath     ,
                 CResourceMetaData& metaData );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
FileExists( const CString& filename );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
DeleteFile( const CString& filename );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
MoveFile( const CString& oldPath ,
          const CString& newPath ,
          const bool overwrite   );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
DirExists( const CString& path );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
CreatePathDirectories( const CString& path );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP UInt64
FileSize( const CString& filename );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
PathExists( const CString& path );

/*-------------------------------------------------------------------------*/

class CStorageVolumeInfoOSDataImpl;

/**
 *  Class that holds the O/S specific data needed to repeatedly access the storage volume
 *  Intended to be kept thread-private 
 */
class GUCEF_CORE_PUBLIC_CPP CStorageVolumeInfoOSData
{
    public:
    
    CStorageVolumeInfoOSData( void );
    ~CStorageVolumeInfoOSData();

    CStorageVolumeInfoOSDataImpl* GetOsData( void ) const;

    void SetVolumeId( const CString& volumeId );
    void SetAnyPathForVolume( const CString& anyPathForVolume );

    const CString& GetVolumeId( void ) const;
    const CString& GetAnyPathForVolume( void ) const;

    private:
    CStorageVolumeInfoOSData( const CStorageVolumeInfoOSData& src );             /**< do not copy */
    CStorageVolumeInfoOSData& operator=( const CStorageVolumeInfoOSData& src );  /**< do not assign */
    
    private:

    class CStorageVolumeInfoOSDataImpl* m_impl;
    CString m_volumeId;         /**< the volume id of the volume this object is for */
    CString m_anyPathForVolume; /**< a path that is guaranteed to be backed by the volume */
};

typedef CTSharedPtr< CStorageVolumeInfoOSData, MT::CMutex > CStorageVolumeInfoOSDataPtr;

/*-------------------------------------------------------------------------*/

class CStorageDeviceInfoOSDataImpl;

/**
 *  Class that holds the O/S specific data needed to repeatedly access the storage device
 *  Intended to be kept thread-private 
 */
class GUCEF_CORE_PUBLIC_CPP CStorageDeviceInfoOSData
{
    public:
    CStorageDeviceInfoOSData( const CString& deviceId );
    ~CStorageDeviceInfoOSData();

    CStorageDeviceInfoOSDataImpl* GetOsData( void ) const;

    const CString& GetDeviceId( void ) const;

    private:
    CStorageDeviceInfoOSData( void );                                            /**< do not use */
    CStorageDeviceInfoOSData( const CStorageDeviceInfoOSData& src );             /**< do not copy */
    CStorageDeviceInfoOSData& operator=( const CStorageDeviceInfoOSData& src );  /**< do not assign */
    
    private:

    class CStorageDeviceInfoOSDataImpl* m_impl;
    CString m_deviceId;
};

typedef CTSharedPtr< CStorageDeviceInfoOSData, MT::CMutex > CStorageDeviceInfoOSDataPtr;

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CStorageDeviceGeometry
{
    public:

    bool hasCylinderCount;
    UInt64 cylinderCount;
    bool hasTracksPerCylinder;
    UInt64 tracksPerCylinder;
    bool hasSectorsPerTrack;
    UInt64 sectorsPerTrack;
    bool hasBytesPerSector;
    UInt64 bytesPerSector;
    bool hasIsRemovable;
    bool isRemovable;

    CStorageDeviceGeometry( void );
    void Clear( void );
};

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CStoragePerfStats
{
    public:

    bool hasBytesRead;
    UInt64 bytesRead;
    bool hasBytesReadPerSec;
    UInt64 bytesReadPerSec;
    bool hasBytesWritten;
    UInt64 bytesWritten;
    bool hasBytesWrittenPerSec;
    UInt64 bytesWrittenPerSec;
    bool hasReadTimeInMs;
    UInt64 readTimeInMs;
    bool hasWriteTimeInMs;
    UInt64 writeTimeInMs;
    bool hasIdleTimeInMs;
    UInt64 idleTimeInMs;
    bool hasAvgReadTimePerOperationInMs;
    Float32 avgReadTimePerOperationInMs;
    bool hasAvgWriteTimePerOperationInMs;
    Float32 avgWriteTimePerOperationInMs;
    bool hasRequestQueueDepth;
    UInt64 requestQueueDepth;
    bool hasRequestSplitCount;
    UInt64 requestSplitCount;
    bool hasRequestSplitCountPerSec;
    UInt64 requestSplitCountPerSec;

    CStoragePerfStats( void );
    void Clear( void );
};

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CStorageDeviceInformation
{
    public:

    bool hasDeviceId;
    CString deviceId;
    bool hasDeviceIndex;
    UInt32 deviceIndex;

    bool hasGeometry;
    CStorageDeviceGeometry geometry;
    bool hasPerfStats;
    CStoragePerfStats perfStats;

    CStorageDeviceInformation( void );
    void Clear( void );
};

typedef CTSharedPtr< CStorageDeviceInformation, MT::CMutex > CStorageDeviceInformationPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to obtain information about the storage device backing the given device id
 *  You can obtain the device id via the volume information since a volume maps to one or more devices
 */
GUCEF_CORE_PUBLIC_CPP bool
GetStorageDeviceInformationByDeviceId( CStorageDeviceInfoOSData& osData ,
                                       CStorageDeviceInformation& info  );

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CStorageVolumeInformation
{
    public:
    
    bool hasVolumeId;
    CString volumeId;

    bool hasFreeBytesAvailableToCaller;
    UInt64 freeBytesAvailableToCaller;
    bool hasTotalNumberOfBytes;
    UInt64 totalNumberOfBytes;
    bool hasTotalNumberOfFreeBytes;
    UInt64 totalNumberOfFreeBytes;

    bool hasIsReadOnly;
    bool isReadOnly;
    bool hasVolumeName;
    CString volumeName;
    bool hasPaths;
    CStringSet paths;
    bool hasPartitionId2deviceIdMapping;
    CStringMap partitionId2deviceId;     /**< if available maps 1-N partition ids to device ids */

    bool hasPerfStats;
    CStoragePerfStats perfStats;

    CStorageVolumeInformation( void );
    void Clear( void );
};

typedef CTSharedPtr< CStorageVolumeInformation, MT::CMutex > CStorageVolumeInformationPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Obtains the aggregate volume information for the entire system
 *  Note that this function is optimized to be called repeatedly for the same operation via volumeInfoOSData
 */
GUCEF_CORE_PUBLIC_CPP bool
GetFileSystemTotalStorageVolumeInformation( CStorageVolumeInfoOSData& volumeInfoOSData , 
                                            CStorageVolumeInformation& info            );

/*-------------------------------------------------------------------------*/

/**
 *  Obtains the volume information for the given volume id
 *  Note that this function is optimized to be called repeatedly for the same volume via volumeInfoOSData
 */
GUCEF_CORE_PUBLIC_CPP bool
GetFileSystemStorageVolumeInformation( CStorageVolumeInfoOSData& volumeInfoOSData , 
                                       CStorageVolumeInformation& info            );

/*-------------------------------------------------------------------------*/

/**
 *  Convenience function to obtain the volume information for a given volume id
 *  Use GetFileSystemStorageVolumeInformation() for repeated usage
 */
GUCEF_CORE_PUBLIC_CPP bool
GetFileSystemStorageVolumeInformationByVolumeId( CStorageVolumeInformation& info , 
                                                 const CString& volumeId         );

/*-------------------------------------------------------------------------*/

/**
 *  Convenience function to obtain the volume information for a given path
 *  Use GetFileSystemStorageVolumeInformation() for repeated usage
 */
GUCEF_CORE_PUBLIC_CPP bool
GetFileSystemStorageVolumeInformationByDirPath( CStorageVolumeInformation& info , 
                                                const CString& path             );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
GetFileSystemStorageVolumeIdByDirPath( CString& volumeId, const CString& path );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
GetAllFileSystemStorageVolumes( CString::StringSet& volumeIds );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to obtain a path specifically pointing directly to the volume with the id given
 *  This is the more low level path name which is used to access the volume directly
 *  Not to be confused with the more human friendly path names which are normally used for accessing files
 */
GUCEF_CORE_PUBLIC_CPP bool
GetVolumePathForVolumeId( const CString& volumeId ,
                          CString& volumePath     );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to obtain all filesystem path names for the given volume id
 *  These are the more human friendly path names that are normally used for accessing files
 *  Path names returned represent root paths which are garantueed to be backed by the given volume
 */
GUCEF_CORE_PUBLIC_CPP bool
GetAllFileSystemPathNamesForVolume( const CString& volumeId       ,
                                    CString::StringSet& pathNames );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to obtain all filesystem mount points for the given volume id
 *  These are alternate logical mappings which are in reality mapped to a different underlying volume
 */
GUCEF_CORE_PUBLIC_CPP bool
GetAllFileSystemMountPointsForVolume( const CString& volumeId         ,
                                      CString::StringSet& mountPoints );

/*-------------------------------------------------------------------------*/

/**
 *  When needing to iterate the file system this class can be used to do so
 *  It is a cross-platform implementation which provides meta-data for each
 *  resource found to the extent available
 *
 *  The class is not thread safe, keep the iterator object private to the thread
 *  Also note that the interface intentionally mimics the C API style for easy mapping
 */
class GUCEF_CORE_PUBLIC_CPP CFileSystemIterator
{
    public:

    CFileSystemIterator( void );
    ~CFileSystemIterator();

    bool FindFirst( const CString& path );
    bool FindNext( void );
    bool FindClose( void );

    bool IsADirectory( void ) const;
    bool IsAFile( void ) const;
    bool IsSymlink( void ) const;

    bool TryReadMetaData( CResourceMetaData& metaData );
    CString GetResourceName( void ) const;
    CString GetResourcePath( void ) const;

    bool TryReadSymlinkedMetaData( CResourceMetaData& metaData );
    CString GetSymlinkedResourceName( void ) const;
    CString GetSymlinkedResourcePath( void ) const;

    private:

    class CFileSystemIteratorOsData;

    class CFileSystemIteratorOsData* m_osData;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
