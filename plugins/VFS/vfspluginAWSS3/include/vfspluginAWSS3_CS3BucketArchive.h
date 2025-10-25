/*
 *  vfspluginAWSS3: Generic GUCEF VFS plugin for dealing with S3 storage in AWS
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef VFSPLUGIN_AWSS3_CS3BUCKETARCHIVE_H
#define VFSPLUGIN_AWSS3_CS3BUCKETARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#undef GetObject
#include <aws/core/Aws.h>
#include <aws/s3/model/Object.h>
#include <aws/s3/model/GetObjectResult.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/GetObjectResult.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/PutObjectResult.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectResult.h>
#include <aws/s3/S3Client.h>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFSPLUGIN_AWSS3_MACROS_H
#include "vfspluginAWSS3_macros.h"
#define GUCEF_VFSPLUGIN_AWSS3_MACROS_H
#endif /* GUCEF_VFSPLUGIN_AWSS3_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace AWSS3 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CS3BucketArchive : public VFS::CArchive
{
    public:

    CS3BucketArchive( void );

    virtual ~CS3BucketArchive();

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file          ,
                                               const char* mode = "rb"           ,
                                               const VFS::UInt32 memLoadSize = 0 ,
                                               const bool overwrite = false      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::TBasicVfsResourcePtr GetFileAs( const VFS::CString& file                ,
                                                 const CORE::CResourceMetaData& metaData ,
                                                 const char* mode = "wb"                 ,
                                                 const UInt32 memLoadSize = 0            ,
                                                 const bool overwrite = false            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileList( TStringVector& outputList                                                 ,
                              const VFS::CString& mountLocation                                         , 
                              const VFS::CString& archiveLocation                                       ,
                              bool recursive = false                                                    ,
                              bool includePathInFilename = false                                        ,
                              const VFS::CString::StringSet& nameFilters = VFS::CString::EmptyStringSet ,
                              UInt32 maxListEntries = GUCEF_UINT16MAX                                   ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                                 ,
                             const VFS::CString& mountLocation                                         , 
                             const VFS::CString& archiveLocation                                       ,
                             bool recursive = false                                                    ,
                             bool includePathInFilename = false                                        ,
                             const VFS::CString::StringSet& nameFilters = VFS::CString::EmptyStringSet ,
                             UInt32 maxListEntries = GUCEF_UINT16MAX                                   ) const GUCEF_VIRTUAL_OVERRIDE;

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

    private:

    typedef GUCEF::map< CORE::CString, Aws::S3::Model::Object >   TObjectMap;
    typedef CORE::CTEventHandlerFunctor< CS3BucketArchive >     TEventCallback;
    
    CS3BucketArchive( const CS3BucketArchive& src );
    CS3BucketArchive& operator=( const CS3BucketArchive& src );

    void RegisterEventHandlers( void );
    
    void
    OnAwsS3Initialized( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    bool LoadBucketObjectIndex( void );

    private:
    
    TObjectMap m_objects;
    VFS::CString m_archiveName;
    bool m_autoMountBuckets;
    bool m_writeableRequest;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSS3 */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* VFSPLUGIN_AWSS3_CS3BUCKETARCHIVE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

-----------------------------------------------------------------------------*/
