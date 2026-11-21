/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CIREMOTEFILESOURCE_H
#define GUCEF_ASSETSTORE_CIREMOTEFILESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

class CRemoteFileTransfer;
typedef CORE::CTSharedPtr< CRemoteFileTransfer, MT::CMutex > TRemoteFileTransferPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a transfer of files of a remote asset store service (CRemoteFileTransfer) and the discoveries that it asks (CIRemoteFileDiscovery)
 *  need of the files of the service: sizes and kinds, the names in a folder, whole files into the cache of this computer (and kept
 *  there), ranges of bytes for a download, and the mapping between the paths of the service and the paths of this computer. Every
 *  path that is given or returned is a path of the service unless the name says client path. All calls are for threads that may
 *  wait for the service (the thread of a transfer).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIRemoteFileSource
{
    public:

    /** The kind and size of a file or folder */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SFileInfo
    {
        bool exists;
        bool isDirectory;
        UInt64 size;
        CORE::CString digest;               /**< the SHA-256 (lower case hexadecimal) of the file when it is known, else empty */
        SFileInfo( void );
    };

    virtual ~CIRemoteFileSource() {}

    /** A path of the service as the files have it: '/' separators, no '/' first and none at the end */
    static CORE::CString Normalize( const CORE::CString& path );

    /** Whether a path of this computer is below a mount of the files of the service; 'servicePath' is the path as the service has it */
    virtual bool ToServicePath( const CORE::CString& clientPath, CORE::CString& servicePath ) const = 0;

    /** The path of this computer for a path of the service */
    virtual CORE::CString ToClientPath( const CORE::CString& servicePath ) const = 0;

    /** Size and kind of a path. False for a path that is not there */
    virtual bool GetFileInfo( const CORE::CString& servicePath, SFileInfo& info ) = 0;

    /** The names in a folder; false when the folder cannot be listed or is not there. 'truncated' is set when the list was cut */
    virtual bool GetFolderEntries( const CORE::CString& serviceFolder, CORE::CString::StringVector& dirs, CORE::CString::StringVector& files, bool* truncated = GUCEF_NULL ) = 0;

    /** Remembers the SHA-256 that the catalogue gave for a file */
    virtual void NoteDigest( const CORE::CString& servicePath, const CORE::CString& digest ) = 0;

    /** Whether the service could not be reached at the last call */
    virtual bool IsOffline( void ) const = 0;

    /** Whether every block of the file is in the cache (and its size is known) */
    virtual bool IsFileCached( const CORE::CString& servicePath, UInt64 size ) const = 0;

    /** Lets the cache remove the file again when it needs the room */
    virtual void UnpinFile( const CORE::CString& servicePath ) = 0;

    /**
     *  Makes sure every block of a file is in the cache, checks the digest when it is known (a mismatch empties the entry and is tried once
     *  more) and keeps the file when 'pin' is set. 'cancel' is looked at between two calls; 'done' counts the bytes that were added to what
     *  the cache held, as they arrive. OK, or what went wrong in 'message'
     */
    virtual TAssetStoreStatus FetchWholeFile( const CORE::CString& servicePath, bool pin, const MT::CTAtomic< bool >* cancel, MT::CTAtomic< UInt64 >* done, CORE::CString& message ) = 0;

    /** Reads up to 'length' bytes at an offset (fewer at the end of the file); OK, or what went wrong in 'message' */
    virtual TAssetStoreStatus ReadFileRange( const CORE::CString& servicePath, UInt64 offset, UInt32 length, CORE::CDynamicBuffer& data, CORE::CString& message ) = 0;

    /** Called by the task of a transfer when it ends */
    virtual void TransferEnded( const TRemoteFileTransferPtr& transfer ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIREMOTEFILESOURCE_H ? */
