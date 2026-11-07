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

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H

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

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

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
 *  The secrets of the module in an authenticated and encrypted file: all slots in credentials.bin, sealed with
 *  XChaCha20-Poly1305 (the "xchacha20-poly1305" codec) under a key derived (HKDF-SHA256) from the 32 random bytes of
 *  credentials.key, which is made on the first Store() next to the data. The directory is restricted to the user
 *  (SetOwnerOnlyAccess) before anything is written to it, and so are both files. Every write has a new random nonce
 *  and replaces the file atomically (WriteFileAtomically), a crash leaves the old or the new content.
 *
 *  The header (magic and version) is authenticated as additional data. A file that fails authentication was changed,
 *  is cut short, lost its key or belongs to another key: IsTamperDetected() says so, nothing is read from it (the
 *  slots read as empty) and nothing is written to it: Store(), Remove() and RemoveProvider() return false and leave
 *  both files as they are. Wipe() is the only way to start over. A file of a newer format version is left alone in
 *  the same way. Without the AEAD codec (codecspluginMONOCYPHER) nothing can be stored and nothing is ever written
 *  in the clear.
 *
 *  Several instances on one directory, in one process or in several, never undo each other's changes: every
 *  operation reads the files under an advisory lock on credentials.lock (flock() on POSIX, LockFileEx() on Windows;
 *  exclusive for a change, shared for a read). A change applies its one change to what is in the file at that
 *  moment and writes before the lock is let go. The data is decrypted again only when the files differ from what the
 *  instance last read or wrote (a SHA-256 digest over both files; every write has a new nonce). IsTamperDetected()
 *  reads the files as well, so it is meaningful right after construction. A change that does not get the lock within
 *  the lock timeout (default 10 s, 0 waits without a limit) returns false and changes nothing; so does a read
 *  (Load(), ListAccounts()). The lock file stays when the store is wiped.
 *
 *  The key file protects against a file that travels alone (a backup or a copy of the data file, a log); it does not
 *  protect against other code that runs as the same user. Stores of the operating system are the answer to that.
 *
 *  Config: directory (empty: $APPDATADIR$/<appName>/Auth), appName, lockTimeoutInMs
 */
class GUCEF_AUTH_EXPORT_CPP CFileCredentialStore : public ICredentialStore ,
                                                   public CORE::CIConfigurable
{
    public:

    static const CORE::CString DataFileName;        /**< "credentials.bin" */
    static const CORE::CString KeyFileName;         /**< "credentials.key" */
    static const CORE::CString LockFileName;        /**< "credentials.lock" */
    static const UInt32 DefaultLockTimeoutInMs;     /**< 10000 */

    CFileCredentialStore( void );

    explicit CFileCredentialStore( const CORE::CString& directory );

    virtual ~CFileCredentialStore();

    virtual bool Store( const CORE::CString& providerName ,
                        const CORE::CString& accountKey   ,
                        const CSecretString& secret       ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Load( const CORE::CString& providerName ,
                       const CORE::CString& accountKey   ,
                       CSecretString& secret             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Remove( const CORE::CString& providerName ,
                         const CORE::CString& accountKey   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ListAccounts( const CORE::CString& providerName          ,
                               CORE::CString::StringVector& accountKeys   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RemoveProvider( const CORE::CString& providerName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Wipe( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Sets the directory and forgets what was read from the previous one */
    void SetDirectory( const CORE::CString& directory );

    /** The directory with its path variables resolved */
    CORE::CString GetDirectory( void ) const;

    /**
     *  Whether the data file fails its integrity check: reads the files first when the instance has not read them yet
     *  or they changed since, so the answer is about the files as they are now, also right after construction.
     *  When the lock is not had in time the answer is the one of the last read
     */
    bool IsTamperDetected( void ) const;

    /** How long an operation waits for the lock of the store file, 0 waits without a limit */
    void SetLockTimeoutInMs( UInt32 lockTimeoutInMs );

    UInt32 GetLockTimeoutInMs( void ) const;

    /** Whether the AEAD codec the store needs is there */
    static bool IsAvailable( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SSlot
    {
        CORE::CString providerName;
        CORE::CString accountKey;
        CSecretString secret;
        UInt64 storedAtMs;
    };

    typedef GUCEF::vector< SSlot > TSlotVector;

    /** What was read of the files while the lock was held, defined in the source */
    struct SFileImage;

    enum TChangeKind
    {
        CHANGE_STORE ,
        CHANGE_REMOVE ,
        CHANGE_REMOVE_PROVIDER
    };

    CFileCredentialStore( const CFileCredentialStore& src );
    CFileCredentialStore& operator=( const CFileCredentialStore& src );

    void ForgetView( void ) const;
    void ReadFiles( const CORE::CString& directory, SFileImage& image ) const;
    void UpdateView( const SFileImage& image ) const;
    void ParseImage( const SFileImage& image ) const;
    bool RefreshView( void ) const;
    bool Change( TChangeKind kind, const CORE::CString& providerName, const CORE::CString& accountKey, const CSecretString* secret );
    bool WriteSlots( const CORE::CString& directory, const TSlotVector& slots, SFileImage& image );
    bool PrepareDirectory( void );
    const SSlot* FindSlot( const CORE::CString& providerName, const CORE::CString& accountKey ) const;

    private:

    mutable MT::CMutex m_lock;
    CORE::CString m_appName;
    CORE::CString m_directory;
    UInt32 m_lockTimeoutInMs;

    // the view of the files: what the instance last read or wrote, refreshed by every operation
    mutable TSlotVector m_slots;
    mutable bool m_loaded;
    mutable bool m_tamperDetected;
    mutable bool m_newerVersion;
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

#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */
