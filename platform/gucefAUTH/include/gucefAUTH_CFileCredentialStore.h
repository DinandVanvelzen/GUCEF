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
 *  The header (magic and version) is authenticated as additional data. A file that fails authentication was changed
 *  or belongs to another key: IsTamperDetected() says so, the slots read as empty and nothing is overwritten until
 *  the next Store(). A file of a newer format version is left alone: nothing is read from it and Store() refuses.
 *  Without the AEAD codec (codecspluginMONOCYPHER) nothing can be stored and nothing is ever written in the clear.
 *
 *  The key file protects against a file that travels alone (a backup or a copy of the data file, a log); it does not
 *  protect against other code that runs as the same user. Stores of the operating system are the answer to that.
 *
 *  Config: directory (empty: $APPDATADIR$/<appName>/Auth), appName
 */
class GUCEF_AUTH_EXPORT_CPP CFileCredentialStore : public ICredentialStore ,
                                                   public CORE::CIConfigurable
{
    public:

    static const CORE::CString DataFileName;        /**< "credentials.bin" */
    static const CORE::CString KeyFileName;         /**< "credentials.key" */

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

    /** Whether the data file failed authentication when it was read */
    bool IsTamperDetected( void ) const;

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

    CFileCredentialStore( const CFileCredentialStore& src );
    CFileCredentialStore& operator=( const CFileCredentialStore& src );

    void EnsureLoaded( void );
    bool ReadKey( UInt8* derivedKey );
    bool EnsureKey( UInt8* derivedKey );
    bool Save( void );
    bool PrepareDirectory( void );
    SSlot* FindSlot( const CORE::CString& providerName, const CORE::CString& accountKey );

    private:

    mutable MT::CMutex m_lock;
    CORE::CString m_appName;
    CORE::CString m_directory;
    TSlotVector m_slots;
    bool m_loaded;
    bool m_tamperDetected;
    bool m_newerVersion;
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
