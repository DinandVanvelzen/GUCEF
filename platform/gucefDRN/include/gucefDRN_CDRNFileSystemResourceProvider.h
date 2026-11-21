/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_DRN_CDRNFILESYSTEMRESOURCEPROVIDER_H
#define GUCEF_DRN_CDRNFILESYSTEMRESOURCEPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#include "gucefDRN_CIDRNResourceProvider.h"
#define GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#endif /* GUCEF_DRN_CIDRNRESOURCEPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Serves the regular files below a directory. A resource name is joined to the root; only a regular file resolves,
 *  anything else (a directory, a device, a FIFO, a missing file) is NOT_FOUND.
 *
 *  Symbolic links: without followSymlinks (the default) a name whose path below the root passes a symbolic link
 *  (Linux, Android) or a reparse point (Windows: links and junctions) is NOT_FOUND. With followSymlinks (Linux and
 *  Android only; Windows always refuses reparse points) links are followed as long as the real path stays inside the
 *  real root. The root itself may be a link: an administrator chose it.
 *
 *  The revision is "<size>-<modification time in ms>"; every read checks both and answers CHANGED when either moved.
 *  Each read opens the file, so a file replaced between reads is noticed and nothing stays open between reads.
 *  A file swapped for a link between the check and the open is a remaining race of the platforms without openat2.
 */
class GUCEF_DRN_EXPORT_CPP CDRNFileSystemResourceProvider : public CIDRNResourceProvider
{
    public:

    explicit CDRNFileSystemResourceProvider( const CORE::CString& rootDirectory, bool followSymlinks = false );

    virtual ~CDRNFileSystemResourceProvider() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetRootDirectory( void ) const;

    bool GetFollowSymlinks( void ) const;

    /** The path of a resource on the file system (no checks): for the application's own use */
    CORE::CString GetPathOf( const CORE::CString& name ) const;

    virtual TDRNResourceStatus Resolve( const CORE::CString& name, CDRNResourceInfo& info ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Read( const CORE::CString& name            ,
                                     const CDRNResourceInfo& resolved     ,
                                     UInt64 offset                        ,
                                     UInt32 size                          ,
                                     void* destination                    ,
                                     UInt32& bytesRead                    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNFileSystemResourceProvider( const CDRNFileSystemResourceProvider& src );            /**< not implemented */
    CDRNFileSystemResourceProvider& operator=( const CDRNFileSystemResourceProvider& src ); /**< not implemented */

    /** NOT_FOUND unless the path is a regular file inside the root under the link rules; size and modification time */
    TDRNResourceStatus Inspect( const CORE::CString& name, CORE::CString& path, UInt64& size, UInt64& modifiedUtcMs ) const;

    CORE::CString m_root;
    bool m_followSymlinks;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNFILESYSTEMRESOURCEPROVIDER_H ? */
