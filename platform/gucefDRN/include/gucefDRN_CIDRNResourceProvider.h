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

#ifndef GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#define GUCEF_DRN_CIDRNRESOURCEPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEINFO_H
#include "gucefDRN_CDRNResourceInfo.h"
#define GUCEF_DRN_CDRNRESOURCEINFO_H
#endif /* GUCEF_DRN_CDRNRESOURCEINFO_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHASHTREE_H
#include "gucefDRN_CDRNResourceHashTree.h"
#define GUCEF_DRN_CDRNRESOURCEHASHTREE_H
#endif /* GUCEF_DRN_CDRNRESOURCEHASHTREE_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEENTRY_H
#include "gucefDRN_CDRNNamespaceEntry.h"
#define GUCEF_DRN_CDRNNAMESPACEENTRY_H
#endif /* GUCEF_DRN_CDRNNAMESPACEENTRY_H ? */

#ifndef GUCEF_DRN_CIDRNNAMESPACEENUMERATOR_H
#include "gucefDRN_CIDRNNamespaceEnumerator.h"
#define GUCEF_DRN_CIDRNNAMESPACEENUMERATOR_H
#endif /* GUCEF_DRN_CIDRNNAMESPACEENUMERATOR_H ? */

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
 *  What a session serves under a provider name (CDRNResourceSessionComponent::RegisterProvider()): the application
 *  decides what is shared by the providers it registers; the facility never reaches anything else.
 *
 *  The names a provider gets are checked already (CDRNResourceName::IsValidResourceName()): relative, '/' separated,
 *  without "." or ".." segments. The provider maps them into its own root and keeps them there (symbolic links,
 *  mounts). A name it will not expose is NOT_FOUND, so a requester learns nothing about what lies outside.
 *
 *  Every member function may be called from several threads of the pool at once.
 *
 *  Namespace operations (protocol 2.3): a provider that can also tell what it holds overrides GetNamespaceCapabilities()
 *  (DRN_NS_CAP_LIST, DRN_NS_CAP_STAT), Stat() and OpenEnumerator(); the member functions of the namespace default to
 *  "no namespace" (UNSUPPORTED) and a provider that does not override them stays a provider of bytes. The provider is
 *  the share: one registration, one set of share rules, one name. The serving side (CDRNResourceSessionComponent)
 *  does everything that is not about the content of the provider itself: names, roles, the access policy, limits, paging,
 *  the answer sizes and what is left out of listings.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNResourceProvider
{
    public:

    CIDRNResourceProvider( void );

    virtual ~CIDRNResourceProvider();

    /** What the resource is now: size, modification time, revision; digests when the provider knows them */
    virtual TDRNResourceStatus Resolve( const CORE::CString& name, CDRNResourceInfo& info ) = 0;

    /**
     *  Reads [offset, offset + size) of the resource as it was resolved into 'destination' (room for 'size' bytes).
     *  CHANGED when the resource is no longer that revision; bytesRead is less than size only at its end.
     */
    virtual TDRNResourceStatus Read( const CORE::CString& name            ,
                                     const CDRNResourceInfo& resolved     ,
                                     UInt64 offset                        ,
                                     UInt32 size                          ,
                                     void* destination                    ,
                                     UInt32& bytesRead                    ) = 0;

    /** A hash tree the provider keeps for the resource as resolved (a catalogue's), so none has to be built. Default none */
    virtual bool GetHashTree( const CORE::CString& name, const CDRNResourceInfo& resolved, CDRNResourceHashTree& tree );

    /** For logs: "filesystem:/srv/packs", "uri:vfs://packs" */
    virtual CORE::CString GetDescription( void ) const = 0;

    /** What the provider offers beyond bytes: DRN_NS_CAP_LIST and DRN_NS_CAP_STAT when it overrides Stat() and OpenEnumerator(). Default 0 */
    virtual UInt32 GetNamespaceCapabilities( void ) const;

    /** DRN_NS_SHAREFLAG_*: whether the share can be changed (never, in this version) and whether its revisions hold no times. Default read-only */
    virtual UInt32 GetNamespaceFlags( void ) const;

    /** Limits of this share that are stricter than those of the serving side, 0 = those of the serving side */
    virtual void GetNamespaceLimits( UInt32& maxStatPaths, UInt32& maxPageEntries ) const;

    /**
     *  Describes a file or directory: kind, size, modification time, revision (what Resolve() gives for a file, so a
     *  revision of STAT or LIST is the one OPEN checks), flags and the SHA-256 only if the provider already knows it.
     *  The name is validated and relative; the empty name is the root of the provider. NOT_FOUND for anything the
     *  provider does not expose. Default UNSUPPORTED.
     */
    virtual TDRNResourceStatus Stat( const CORE::CString& name, CDRNNamespaceEntry& entry );

    /**
     *  Starts the listing of a directory ("" is the root), without reading it all. NOT_FOUND when there is no such
     *  directory, NOT_A_DIRECTORY for a file. Default UNSUPPORTED.
     */
    virtual TDRNResourceStatus OpenEnumerator( const CORE::CString& dirName, CIDRNNamespaceEnumeratorPtr& enumerator );

    private:

    CIDRNResourceProvider( const CIDRNResourceProvider& src );              /**< not implemented */
    CIDRNResourceProvider& operator=( const CIDRNResourceProvider& src );   /**< not implemented */
};

typedef CORE::CTSharedPtr< CIDRNResourceProvider, MT::CMutex > CIDRNResourceProviderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNRESOURCEPROVIDER_H ? */
