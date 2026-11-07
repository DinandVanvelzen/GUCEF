/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#define GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_CFILELOCK_H
#include "gucefAUTH_CFileLock.h"
#define GUCEF_AUTH_CFILELOCK_H
#endif /* GUCEF_AUTH_CFILELOCK_H ? */

#ifndef GUCEF_AUTH_IIDENTITYLINKSTORE_H
#include "gucefAUTH_IIdentityLinkStore.h"
#define GUCEF_AUTH_IIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_IIDENTITYLINKSTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The links in a data tree file, json by default (the codec that the extension of the path names), replaced
 *  atomically on every change. Without a path the links are kept in memory only.
 *
 *  Every record is verified when the file is read: a record that was changed after it was signed, or that is not well
 *  formed, is refused and counted (GetRefusedCount()); it is dropped when the file is written again. A record of a
 *  version this code does not know is not used and not touched: it is written back as it was, so a file shared with a
 *  newer version of the module keeps its content; a file of a newer format version, or one that cannot be read, is not
 *  changed. When a signer is set, records of the signer's own profile must carry the public key of the signer.
 *
 *  Several instances on one file, in one process or in several, never undo each other's changes: every operation
 *  reads the file under an advisory lock on the lock file next to it (the path with ".lock" appended; CFileLock:
 *  exclusive for a change, shared for a read). A change applies its one change (one link made or replaced, one link
 *  removed, the links of one profile merged) to what is in the file at that moment and writes before the lock is let
 *  go. The file is parsed again only when it differs from what the instance last read or wrote (a SHA-256 digest of
 *  the file); a record that is in the view already is not verified again. A change that does not get the lock within
 *  the lock timeout (default 10 s, 0 waits without a limit) fails and changes nothing (LINKRESULT_STORE_ERROR, or
 *  false); so does a read (GetLinks(), Resolve()). The lock file is never deleted.
 *
 *  The file is the cache of a service that is the authority later; it is restricted to the user.
 *
 *  Config: path, lockTimeoutInMs
 */
class GUCEF_AUTH_EXPORT_CPP CLocalIdentityLinkStore : public IIdentityLinkStore ,
                                                      public CORE::CIConfigurable
{
    public:

    static const UInt32 FileVersion = 1;
    static const CORE::CString LockFileSuffix;      /**< ".lock", appended to the path of the file */
    static const UInt32 DefaultLockTimeoutInMs;     /**< 10000 */

    CLocalIdentityLinkStore( void );

    /** @param path The file; empty for a store in memory only */
    explicit CLocalIdentityLinkStore( const CORE::CString& path );

    virtual ~CLocalIdentityLinkStore();

    virtual TLinkResult Link( const CLinkRecord& record ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Unlink( const CAccountRef& account, const CORE::CString& profileKeyId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetLinks( const CORE::CString& profileKeyId, TLinkRecordVector& links ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Resolve( const CAccountRef& account, CORE::CString& profileKeyId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TLinkResult Merge( const CORE::CString& fromProfileKeyId ,
                               ILinkSigner& intoSigner               ,
                               UInt32* mergedCount = GUCEF_NULL      ) GUCEF_VIRTUAL_OVERRIDE;

    /** Sets the file and forgets what was read from the previous one */
    void SetPath( const CORE::CString& path );

    /** The path with its variables resolved, empty for a store in memory only */
    CORE::CString GetPath( void ) const;

    /** The path of the lock file: the path with LockFileSuffix appended, empty for a store in memory only */
    CORE::CString GetLockPath( void ) const;

    /** How long an operation waits for the lock of the file, 0 waits without a limit */
    void SetLockTimeoutInMs( UInt32 lockTimeoutInMs );

    UInt32 GetLockTimeoutInMs( void ) const;

    /** The signer is not owned and has to outlive the store; GUCEF_NULL for none */
    void SetSigner( ILinkSigner* signer );

    /**
     *  How many records were refused when the file was read: reads the file first when it changed since the last read,
     *  answers from the last read when the lock is not had in time
     */
    UInt32 GetRefusedCount( void ) const;

    /** How many records of an unknown version are carried along, read like GetRefusedCount() */
    UInt32 GetForeignCount( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< CORE::CDataNode > TForeignNodeVector;

    /** What was read of the file while the lock was held, defined in the source */
    struct SFileImage;

    CLocalIdentityLinkStore( const CLocalIdentityLinkStore& src );
    CLocalIdentityLinkStore& operator=( const CLocalIdentityLinkStore& src );

    void ForgetView( void ) const;
    void ReadFile( const CORE::CString& path, SFileImage& image ) const;
    void UpdateView( const SFileImage& image ) const;
    void ParseImage( const SFileImage& image ) const;
    bool RefreshView( void ) const;
    bool PrepareChange( CFileLock& fileLock, bool makeDirectory );
    bool IsAcceptable( const CLinkRecord& record, const TLinkRecordVector* verified = GUCEF_NULL ) const;
    bool Save( void );
    size_t FindIndex( const CAccountRef& account ) const;

    private:

    mutable MT::CMutex m_lock;
    CORE::CString m_path;
    ILinkSigner* m_signer;
    UInt32 m_lockTimeoutInMs;

    // the view of the file: what the instance last read or wrote, refreshed by every operation
    mutable TLinkRecordVector m_links;
    mutable TForeignNodeVector m_foreign;
    mutable UInt32 m_refusedCount;
    mutable bool m_loaded;
    mutable bool m_readOnly;
    mutable UInt8 m_fileDigest[ CORE::CSha256::DigestSize ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H ? */
