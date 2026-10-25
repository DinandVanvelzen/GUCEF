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

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  newer version of the module keeps its content; a file of a newer format version, or one that cannot be read, is not changed. When a signer is set, records of the signer's own profile must carry
 *  the public key of the signer.
 *
 *  The file is the cache of a service that is the authority later; it is restricted to the user.
 *
 *  Config: path
 */
class GUCEF_AUTH_EXPORT_CPP CLocalIdentityLinkStore : public IIdentityLinkStore ,
                                                      public CORE::CIConfigurable
{
    public:

    static const UInt32 FileVersion = 1;

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

    /** The signer is not owned and has to outlive the store; GUCEF_NULL for none */
    void SetSigner( ILinkSigner* signer );

    /** How many records were refused while reading the file */
    UInt32 GetRefusedCount( void ) const;

    /** How many records of an unknown version are carried along */
    UInt32 GetForeignCount( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< CORE::CDataNode > TForeignNodeVector;

    CLocalIdentityLinkStore( const CLocalIdentityLinkStore& src );
    CLocalIdentityLinkStore& operator=( const CLocalIdentityLinkStore& src );

    void EnsureLoaded( void );
    bool IsAcceptable( const CLinkRecord& record ) const;
    bool Save( void );
    size_t FindIndex( const CAccountRef& account ) const;

    private:

    mutable MT::CMutex m_lock;
    CORE::CString m_path;
    ILinkSigner* m_signer;
    TLinkRecordVector m_links;
    TForeignNodeVector m_foreign;
    UInt32 m_refusedCount;
    bool m_loaded;
    bool m_readOnly;
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
