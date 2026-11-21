/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#define GUCEF_VFS_CVFSGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKCACHE_H
#include "gucefVFS_CVfsBlockCache.h"
#define GUCEF_VFS_CVFSBLOCKCACHE_H
#endif /* GUCEF_VFS_CVFSBLOCKCACHE_H ? */

#ifndef GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#include "gucefVFS_CVfsThreadPolicyRegistry.h"
#define GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSFETCHSCHEDULER_H
#include "gucefVFS_CVfsFetchScheduler.h"
#define GUCEF_VFS_CVFSFETCHSCHEDULER_H
#endif /* GUCEF_VFS_CVFSFETCHSCHEDULER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CVFS;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global Vfs systems
 */
class GUCEF_VFS_PUBLIC_CPP CVfsGlobal
{
    public:

    static CVfsGlobal* Instance( void );

    CVFS& GetVfs( void );

    /**
     *  The block cache of a folder: one cache object per folder in the process, so that every archive that wants
     *  the folder shares one size limit and a folder is never open twice. The same folder gives the same cache; the folder
     *  is closed when the last pointer goes. The parameters are those of the first caller: a later caller that
     *  asks for other values gets the cache as it is (and a warning).
     *  The block cache is off until an application opens one: the library has no default folder and no default size.
     *  An empty pointer when the folder cannot be opened (see CVfsBlockCache::Open()).
     */
    CVfsBlockCachePtr AcquireBlockCache( const CORE::CString& folder                                   ,
                                         UInt64 maxBytes                                               ,
                                         UInt32 blockBytes = CVfsBlockCache::DefaultBlockBytes         ,
                                         UInt32 maxOpenFiles = CVfsBlockCache::DefaultMaxOpenFiles     ,
                                         bool breakStaleLock = false                                   );

    /** The cache that the application made the default one, which archives without a folder of their own use; empty when there is none */
    CVfsBlockCachePtr GetDefaultBlockCache( void ) const;

    /** The VFS configuration ("BlockCache" below "VFS") and applications set it; an empty pointer clears it */
    void SetDefaultBlockCache( const CVfsBlockCachePtr& cache );

    /**
     *  The rules of the threads for a miss on a remote archive (see CVfsThreadPolicy): a thread that must never wait for a
     *  remote place registers itself here, or uses a CVfsThreadPolicyScope for one call
     */
    CVfsThreadPolicyRegistry& GetThreadPolicyRegistry( void );

    /**
     *  Runs the background fetches of the remote archives on a pool of its own (VfsRemoteOps, see CVfsFetchScheduler), with
     *  the bounds of that class
     */
    CVfsFetchScheduler& GetFetchScheduler( void );

    private:
    friend class CModule;

    static void Deinstance( void );

    private:

    CVfsGlobal( void );

    typedef CORE::CTWeakPtr< CVfsBlockCache, MT::CMutex >   TBlockCacheWeakPtr;
    typedef GUCEF::map< CORE::CString, TBlockCacheWeakPtr > TBlockCacheMap;

    ~CVfsGlobal();

    void Initialize( void );

    private:

    CVFS* m_vfs;
    mutable MT::CMutex m_blockCacheLock;
    TBlockCacheMap m_blockCaches;                           /**< by folder: weak, so that the folder is closed when the last user lets go */
    CVfsBlockCachePtr m_defaultBlockCache;
    CVfsThreadPolicyRegistry m_threadPolicyRegistry;
    CVfsFetchScheduler* m_fetchScheduler;

    static MT::CMutex g_datalock;
    static CVfsGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-02-2007 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
