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

#ifndef GUCEF_CORE_CSECUREBUFFER_H
#include "gucefCORE_CSecureBuffer.h"
#define GUCEF_CORE_CSECUREBUFFER_H
#endif /* GUCEF_CORE_CSECUREBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_KEYRINGTYPES_H
#include "gucefAUTH_KeyringTypes.h"
#define GUCEF_AUTH_KEYRINGTYPES_H
#endif /* GUCEF_AUTH_KEYRINGTYPES_H ? */

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

class CFileCredentialStoreTestAccess;

/*-------------------------------------------------------------------------*/

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
 *  External key file (SetKeyFile(), config keyFile): the key is kept apart from the data, for example in a Kubernetes
 *  Secret mounted read only, so the data volume, its backups and its copies never hold it. The store treats it as
 *  provisioned by someone else: it never makes, writes, restricts or deletes it (Wipe() deletes the data file only).
 *  It is read under the store lock through a trust check (SetKeyFileTrust()): owner-only (CORE::ReadOwnerOnlyFile())
 *  or mounted-secret (CORE::ReadMountedSecretFile(): links only within its directory, read only file system, owner
 *  root or the user, no write for group or others, no read for others). Its content is 32 bytes, or 64 hexadecimal
 *  characters with an optional line break (a Secret made from text). A key that is missing, refused by the trust
 *  check or not a key is a deployment fault, not tampering: GetKeyState() says which, IsTamperDetected() stays false,
 *  nothing is read and every change is refused, and the data file is left byte for byte, so the right key makes the
 *  store readable at the next operation. A credentials.key in the directory is never read then; a warning names it.
 *
 *  Keyring code (SetKeyMode( KEYMODE_KEYRING_CODE ), config keyMode "keyring-code"): the key is derived from a passphrase
 *  that the person types, the keyring code, instead of a key file, so a copy of the directory (a backup, another machine)
 *  is of no use without the code and opens with it. The data is in keyring.bin, next to (and apart from) the files of the
 *  key file mode, so one directory can have both: a key file store for what may be reused without the person (session
 *  cookies) and a keyring store for saved passwords. The file is format version 2 of the same magic: the header holds the
 *  derivation (Argon2id, the memory in KiB, the passes, the lanes), a random salt of 16 bytes and a check value, and is
 *  authenticated as additional data. Argon2id of the code gives 32 bytes; HKDF-SHA256 of those gives the sealing key
 *  (info "gucefAUTH keyring v2 seal") and the check value (info "gucefAUTH keyring v2 check"). A code whose check value
 *  does not match the file is a wrong code (KEYRING_WRONG_CODE), not tampering; a right code whose data fails
 *  authentication is tampering (KEYRING_TAMPERED, IsTamperDetected()). The parameters for the next seal are
 *  SetKeyringParameters(); a file keeps those it was sealed with, so they can be raised by a change of code.
 *
 *  The keyring is locked until UnlockKeyring() is given the right code. While it is locked nothing is read (Load() and
 *  ListAccounts() return false) and nothing is changed; the store keeps only the sealing key while unlocked, in a
 *  CORE::CSecureBuffer that is wiped when it locks (LockKeyring(), Wipe(), destruction, the file sealed with another code
 *  by another instance). Neither the code nor anything derived from it is logged or written; the file holds the salt and
 *  the check value only. ChangeKeyringCode() reseals with a new salt and the current parameters; Wipe() deletes
 *  keyring.bin (forgetting the code loses the saved credentials) and leaves the files of the key file mode alone. The
 *  derivation takes the time and memory of the parameters (64 MiB and 3 passes by default) and runs without the lock of
 *  the instance held, so reads on other threads answer (locked) meanwhile; CKeyringSession runs it on a pool thread and
 *  adds the idle lock, the wait between wrong codes and the events. External key file settings have no meaning in this mode.
 *
 *  Config: directory (empty: $APPDATADIR$/<appName>/Auth), appName, lockTimeoutInMs, keyFile (empty: credentials.key
 *  in the directory), keyFileTrust ("owner-only" or "mounted-secret"), keyMode ("key-file" or "keyring-code"),
 *  keyringMemoryInKiB, keyringPasses, keyringLanes; the key settings are saved when not the default
 */
class GUCEF_AUTH_EXPORT_CPP CFileCredentialStore : public ICredentialStore ,
                                                   public CORE::CIConfigurable
{
    public:

    static const CORE::CString DataFileName;        /**< "credentials.bin" */
    static const CORE::CString KeyFileName;         /**< "credentials.key" */
    static const CORE::CString LockFileName;        /**< "credentials.lock" */
    static const UInt32 DefaultLockTimeoutInMs;     /**< 10000 */
    static const CORE::CString KeyringFileName;     /**< "keyring.bin", the data of the keyring code mode */
    static const UInt32 DefaultKeyringMemoryInKiB;  /**< 65536 (64 MiB) */
    static const UInt32 DefaultKeyringPasses;       /**< 3 */
    static const UInt32 DefaultKeyringLanes;        /**< 1 */
    static const UInt32 MaxKeyringMemoryInKiB;      /**< 1048576 (1 GiB): a file asking for more is not a keyring file */
    static const UInt32 MaxKeyringPasses;           /**< 32 */
    static const UInt32 MaxKeyringLanes;            /**< 16 */
    static const UInt32 MaxKeyringCodeLength;       /**< 1024 bytes */

    /** Where the key of the store comes from */
    enum EKeyMode
    {
        KEYMODE_KEY_FILE        ,   /**< "key-file": credentials.key in the directory or the external key file, the default */
        KEYMODE_KEYRING_CODE        /**< "keyring-code": derived from a code the person types, data in keyring.bin */
    };

    /** How an external key file is checked before it is read */
    enum EKeyFileTrust
    {
        KEYFILETRUST_OWNER_ONLY     ,   /**< "owner-only": CORE::ReadOwnerOnlyFile(), the default */
        KEYFILETRUST_MOUNTED_SECRET     /**< "mounted-secret": CORE::ReadMountedSecretFile(), POSIX only */
    };

    /** What the key of the store is like, as last read */
    enum EKeyState
    {
        KEYSTATE_OK         ,   /**< a usable key */
        KEYSTATE_MISSING    ,   /**< there is no key file */
        KEYSTATE_UNTRUSTED  ,   /**< the trust check refused the external key file: owner, permissions, link, file type, mount */
        KEYSTATE_INVALID        /**< the key file is not a key: size or format */
    };

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

    /**
     *  An external key file, see the class comment; empty is credentials.key in the directory. Path variables are
     *  resolved. Forgets what was read
     */
    void SetKeyFile( const CORE::CString& keyFile );

    /** The key file in use with its path variables resolved: the external one, or credentials.key in the directory */
    CORE::CString GetKeyFile( void ) const;

    /** Whether an external key file is set */
    bool HasExternalKeyFile( void ) const;

    /** How an external key file is checked; forgets what was read */
    void SetKeyFileTrust( EKeyFileTrust trust );

    EKeyFileTrust GetKeyFileTrust( void ) const;

    /**
     *  The state of the key as the files are now: reads them first when the instance has not read them yet or they
     *  changed since, like IsTamperDetected()
     */
    EKeyState GetKeyState( void ) const;

    /** Why the key is not OK, for a log line or a status page; empty when it is */
    CORE::CString GetKeyProblem( void ) const;

    /** "owner-only" or "mounted-secret" */
    static const CORE::CString& KeyFileTrustToString( EKeyFileTrust trust );

    /** False for a text that is neither of the two */
    static bool KeyFileTrustFromString( const CORE::CString& text, EKeyFileTrust& trust );

    /**
     *  Makes a new key file of 32 random bytes that only the user of the process can read (CORE::CreateOwnerOnlyFile():
     *  refuses anything that exists at the path, a link included). For an operator tool that provisions an external
     *  key, and for tests. Fails closed when there is no random source.
     *  @param problem  why it failed, for a log line
     */
    static bool MakeKeyFile( const CORE::CString& path, CORE::CString& problem );

    /** Whether the AEAD codec the store needs is there */
    static bool IsAvailable( void );

    /** @name Keyring code mode */
    ///@{

    /** Switches the source of the key; locks the keyring and forgets what was read */
    void SetKeyMode( EKeyMode keyMode );
    EKeyMode GetKeyMode( void ) const;

    /** "key-file" or "keyring-code" */
    static const CORE::CString& KeyModeToString( EKeyMode keyMode );

    /** False for a text that is neither of the two */
    static bool KeyModeFromString( const CORE::CString& text, EKeyMode& keyMode );

    /** Whether the codecs of the keyring code mode are there: "argon2id" and "xchacha20-poly1305" */
    static bool IsKeyringAvailable( void );

    /**
     *  The Argon2id parameters of the next seal (SetUpKeyring(), ChangeKeyringCode()): the memory in KiB, the passes and the
     *  lanes, brought within 1 to MaxKeyringPasses, 1 to MaxKeyringLanes and 8 KiB per lane to MaxKeyringMemoryInKiB
     */
    void SetKeyringParameters( UInt32 memoryInKiB, UInt32 passes, UInt32 lanes );
    void GetKeyringParameters( UInt32& memoryInKiB, UInt32& passes, UInt32& lanes ) const;

    /** The parameters the keyring file was sealed with; false when there is no keyring file or it is not one */
    bool GetSealedKeyringParameters( UInt32& memoryInKiB, UInt32& passes, UInt32& lanes ) const;

    /** Whether the keyring file exists (a code was set up) */
    bool IsKeyringSetUp( void ) const;

    /** Whether the keyring is locked: no key in memory. False in the key file mode */
    bool IsKeyringLocked( void ) const;

    /** Why the keyring locked last, while it is locked */
    TKeyringLockReason GetKeyringLockReason( void ) const;

    /**
     *  Makes an empty keyring sealed with the code (a new salt, the parameters of SetKeyringParameters()) and leaves it unlocked.
     *  KEYRING_ALREADY_SET_UP when there is one, KEYRING_INVALID_CODE for an empty code or one longer than MaxKeyringCodeLength.
     *  Blocks the calling thread for the derivation
     */
    TKeyringResult SetUpKeyring( const CSecretString& code );

    /**
     *  Derives the key from the code and the salt and parameters of the file and keeps it when the check value matches.
     *  KEYRING_WRONG_CODE when it does not, KEYRING_TAMPERED when it does and the data fails authentication (the keyring
     *  stays locked then), KEYRING_NOT_SET_UP without a keyring file. Blocks the calling thread for the derivation
     */
    TKeyringResult UnlockKeyring( const CSecretString& code );

    /** Wipes the key and the slots read with it */
    void LockKeyring( void );

    /**
     *  Reseals the data with the new code, a new salt and the parameters of SetKeyringParameters(), under the exclusive lock of
     *  the store, and leaves the keyring unlocked with the new code. The current code is required also while unlocked:
     *  KEYRING_WRONG_CODE when it is not right. Blocks the calling thread for two derivations
     */
    TKeyringResult ChangeKeyringCode( const CSecretString& currentCode, const CSecretString& newCode );

    /** The monotonic time (GetMonotonicTimeInMs()) of the last operation that used the key; 0 while locked */
    UInt64 GetKeyringLastUseInMs( void ) const;

    ///@}

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** The test seam of the store, defined by the tests only */
    friend class CFileCredentialStoreTestAccess;

    /**
     *  For tests that cannot mount a file system read only: the mounted-secret check takes that rule as met. Only
     *  reachable through CFileCredentialStoreTestAccess, so production code cannot turn the rule off
     */
    void SetAssumeReadOnlyKeyMountForTesting( bool assumeReadOnly );

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
    void ReadKey( const CORE::CString& directory, SFileImage& image ) const;
    void NoteKeyState( const SFileImage& image ) const;
    bool HasExternalKeyFileWhileLocked( void ) const;
    void UpdateView( const SFileImage& image ) const;
    void ParseImage( const SFileImage& image ) const;
    bool RefreshView( void ) const;
    bool Change( TChangeKind kind, const CORE::CString& providerName, const CORE::CString& accountKey, const CSecretString* secret );
    bool WriteSlots( const CORE::CString& directory, const TSlotVector& slots, SFileImage& image );
    bool PrepareDirectory( void );
    bool IsKeyringModeWhileLocked( void ) const;
    const CORE::CString& GetDataFileNameWhileLocked( void ) const;
    TKeyringResult CheckKeyringUse( const CSecretString& code ) const;
    TKeyringResult ReadKeyringFile( CORE::CDynamicBuffer& file ) const;
    void ParseKeyringImage( const SFileImage& image ) const;
    void InstallKeyringKey( const UInt8* sealKey, const UInt8* check, const UInt8* header, UInt32 headerSize ) const;
    void LockKeyringWhileLocked( TKeyringLockReason reason ) const;
    bool WriteKeyringSlots( const CORE::CString& directory, const TSlotVector& slots, const UInt8* header, UInt32 headerSize,
                            const UInt8* sealKey, SFileImage& image ) const;
    static void SerializeSlots( const TSlotVector& slots, CORE::CDynamicBuffer& plain );
    static bool DeserializeSlots( const UInt8* data, UInt32 size, TSlotVector& slots );
    const SSlot* FindSlot( const CORE::CString& providerName, const CORE::CString& accountKey ) const;

    private:

    mutable MT::CMutex m_lock;
    CORE::CString m_appName;
    CORE::CString m_directory;
    UInt32 m_lockTimeoutInMs;
    CORE::CString m_keyFile;
    EKeyFileTrust m_keyFileTrust;
    bool m_assumeReadOnlyKeyMountForTesting;

    // the view of the files: what the instance last read or wrote, refreshed by every operation
    mutable TSlotVector m_slots;
    mutable bool m_loaded;
    mutable bool m_tamperDetected;
    mutable bool m_newerVersion;
    mutable bool m_keyRefused;              /**< an external key that is not OK: nothing is read, every change is refused */
    mutable EKeyState m_keyState;
    mutable CORE::CString m_keyProblem;
    mutable bool m_keyStateLogged;
    mutable EKeyState m_loggedKeyState;
    mutable bool m_strayKeyWarned;
    mutable UInt8 m_fileDigest[ CORE::CSha256::DigestSize ];
    EKeyMode m_keyMode;
    UInt32 m_keyringMemoryInKiB;
    UInt32 m_keyringPasses;
    UInt32 m_keyringLanes;
    mutable CORE::CSecureBuffer m_keyringKey;       /**< the sealing key while the keyring is unlocked, wiped when it locks */
    mutable UInt8 m_keyringCheck[ CORE::CSha256::DigestSize ];  /**< the check value of that key, as in the file */
    mutable CORE::CDynamicBuffer m_keyringHeader;   /**< the header of the file the key opens (no secret in it) */
    mutable bool m_keyringUnlocked;
    mutable UInt64 m_keyringLastUseMs;
    mutable TKeyringLockReason m_keyringLockReason;
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
