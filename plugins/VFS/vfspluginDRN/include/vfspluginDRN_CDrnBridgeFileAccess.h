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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#include "vfspluginDRN_CDrnBridgeHandlePool.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H ? */

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
 *  A file of the share opened for reading, as the archive hands it out: a read only CORE::CIOAccess pinned at the revision
 *  (size and modification time) the file had when it was opened. The bytes come through a DRN handle with the facility's chunk
 *  cache (DRN::CDRNResourceIOAccess), the one content cache of this version; the handle is taken from the pool of the archive
 *  when the file is read, and may be taken away from it again when the pool needs the room (CDrnBridgeHandlePool).
 *
 *  Position, size, Seek and Eof are answered by the file itself and cost no round trip. A read takes the position the file
 *  has to the handle, so a file whose handle was closed and opened again carries on where it was.
 *
 *  A file that changed on the serving side fails: the next read that needs bytes of another revision, or the opening of a
 *  handle for the revision the file was pinned at, answers "changed" (GetLastResult()), the file reads nothing from then on
 *  (IsValid() is false) and what the archive cached about the path is dropped. Opening the path again gives the new revision.
 *
 *  Writes do nothing (IsWriteable() is false). One thread reads a file at a time, as for the facility's handles.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeFileAccess : public CORE::CIOAccess
{
    public:

    /** Made by CDrnBridgeHandlePool::OpenFile() and Clone() */
    CDrnBridgeFileAccess( const CDrnBridgeHandlePoolPtr& pool, const CORE::CString& path );

    virtual ~CDrnBridgeFileAccess() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetPath( void ) const;

    /** What the file was when it was opened */
    const CORE::CString& GetPinnedRevision( void ) const;
    UInt64 GetPinnedModifiedUtcMs( void ) const;

    /** What the last failed operation came to, a success when none failed */
    const CDrnBridgeResult& GetLastResult( void ) const;

    /** Gives the handle of the file up (when it is not being read): the next read opens another one, at the revision the file is pinned at. For a read that failed in a way that leaves what the handle holds in doubt */
    void ReleaseHandle( void );

    /** Whether a handle is held for the file right now (it may have been given up to make room for another file) */
    bool HasHandle( void ) const;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    using CORE::CIOAccess::Read;
    using CORE::CIOAccess::Write;

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    /** Another access to the same file, at the same revision and position, with a handle of its own once it is read */
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDrnBridgeHandlePool;

    CDrnBridgeFileAccess( const CDrnBridgeFileAccess& src );                /**< used by Clone() only */
    CDrnBridgeFileAccess& operator=( const CDrnBridgeFileAccess& src );     /**< not implemented */

    /** Records why an operation failed; a change also ends the file */
    void Fail( const CDrnBridgeResult& result );

    CDrnBridgeHandlePoolPtr m_pool;
    CORE::CString m_path;
    CORE::CString m_pinnedRevision;
    CORE::CString m_expectedRevision;       /**< the revision the first open has to find, empty for any: set by the pool for a file that is opened for a revision that is known */
    bool m_reportsChange;                   /**< a change of the file is told to the listener of the pool: the file was opened by the archive itself, not for a block source whose user does that */
    UInt64 m_pinnedSize;
    UInt64 m_pinnedModifiedUtcMs;
    bool m_pinned;
    UInt64 m_position;
    bool m_closed;
    bool m_failed;
    CDrnBridgeResult m_lastResult;

    // the pool's: guarded by its lock
    DRN::CDRNResourceIOAccess* m_inner;
    bool m_busy;
    bool m_opening;
    UInt64 m_lastUsed;
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

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEFILEACCESS_H ? */
