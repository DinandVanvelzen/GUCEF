/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBLOCKSOURCE_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBLOCKSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_VFS_CIVFSBLOCKSOURCE_H
#include "gucefVFS_CIVfsBlockSource.h"
#define GUCEF_VFS_CIVFSBLOCKSOURCE_H
#endif /* GUCEF_VFS_CIVFSBLOCKSOURCE_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#include "vfspluginDRN_CDrnBridgeHandlePool.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H
#include "vfspluginDRN_CDrnBridgeFileAccess.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The bytes of one file of a share at one revision, for the generic remote layer (VFS::CIVfsBlockSource): the file is a
 *  CDrnBridgeFileAccess of the pool of the archive, used raw (its own read, no block cache), so that the handle limits, the
 *  closing of the least recently used handle, the reopening at the pinned revision and the "changed" rule stay where they are.
 *
 *  A range is a position and a read. A read that comes short ends the call with the result of the file access made into a
 *  result of the generic layer (a file that changed gives REMOTE_RESOURCE_CHANGED, a link that was lost
 *  REMOTE_CONNECTION_PENDING, a limit of the serving side REMOTE_CAPLIMIT_RETRY_LATER), with the bytes that did arrive.
 *  The source is used by one thread at a time.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeBlockSource : public VFS::CIVfsBlockSource
{
    public:

    /**
     *  Opens the file at the revision: the handle is opened now (this is where the size of the file is learned) and the file is pinned
     *  to the revision, 'expectedRevision' empty pins it to whatever it is. An empty pointer when that fails; 'result' says why.
     *  'bridgeResult' (may be NULL) gets the outcome in the form of the bridge, 'fetchCalls' and 'bytesFetched' (both may be NULL) are counters that the source adds its calls and bytes to.
     */
    static VFS::CIVfsBlockSourcePtr Open( const CDrnBridgeHandlePoolPtr& pool            ,
                                          const CORE::CString& path                      ,
                                          const CORE::CString& expectedRevision          ,
                                          VFS::CVfsCallResult& result                    ,
                                          CDrnBridgeResult* bridgeResult = GUCEF_NULL     ,
                                          MT::CTAtomic< UInt64 >* fetchCalls = GUCEF_NULL ,
                                          MT::CTAtomic< UInt64 >* bytesFetched = GUCEF_NULL );

    virtual ~CDrnBridgeBlockSource() GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::CVfsCallResult FetchRange( UInt64 offset, UInt32 length, void* buffer, UInt32& bytesRead ) GUCEF_VIRTUAL_OVERRIDE;

    /** The size of the file at the revision, as the handle reported it */
    UInt64 GetFileSize( void ) const;

    const CORE::CString& GetPinnedRevision( void ) const;

    private:

    CDrnBridgeBlockSource( CDrnBridgeFileAccess* access, MT::CTAtomic< UInt64 >* fetchCalls, MT::CTAtomic< UInt64 >* bytesFetched );
    CDrnBridgeBlockSource( const CDrnBridgeBlockSource& src );                  /**< not implemented */
    CDrnBridgeBlockSource& operator=( const CDrnBridgeBlockSource& src );       /**< not implemented */

    CDrnBridgeFileAccess* m_access;
    MT::CTAtomic< UInt64 >* m_fetchCalls;
    MT::CTAtomic< UInt64 >* m_bytesFetched;
    UInt64 m_lastBegin;                         /**< the range asked for last: asked for again, what the handle kept of it is not trusted */
    UInt64 m_lastEnd;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBLOCKSOURCE_H ? */
