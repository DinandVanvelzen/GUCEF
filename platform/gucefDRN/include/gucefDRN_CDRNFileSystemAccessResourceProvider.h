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

#ifndef GUCEF_DRN_CDRNFILESYSTEMACCESSRESOURCEPROVIDER_H
#define GUCEF_DRN_CDRNFILESYSTEMACCESSRESOURCEPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#include "gucefDRN_CIDRNResourceProvider.h"
#define GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#endif /* GUCEF_DRN_CIDRNRESOURCEPROVIDER_H ? */

#ifndef GUCEF_DRN_CDRNURIRESOURCEPROVIDER_H
#include "gucefDRN_CDRNUriResourceProvider.h"
#define GUCEF_DRN_CDRNURIRESOURCEPROVIDER_H
#endif /* GUCEF_DRN_CDRNURIRESOURCEPROVIDER_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACESHARESETTINGS_H
#include "gucefDRN_CDRNNamespaceShareSettings.h"
#define GUCEF_DRN_CDRNNAMESPACESHARESETTINGS_H
#endif /* GUCEF_DRN_CDRNNAMESPACESHARESETTINGS_H ? */

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
 *  A share: a subtree of a CORE::IFileSystemAccess (the file system of the operating system, the VFS, any other) that
 *  members can browse and read. The names, sizes, times and listings come from the file system access; the bytes from the
 *  URI accessor of the base URI (CDRNUriResourceProvider::ReadFromUri(), the route CDRNUriResourceProvider takes), so
 *  gucefDRN depends on neither the VFS nor any file system layer: "vfs://<mount>/<dir>" over a CVfsFileSystemAccess
 *  rooted at "/<mount>/<dir>", or "file://<dir>" over a CNativeFileSystemAccess rooted at "<dir>".
 *
 *  The share is registered like any provider (CDRNResourceSessionComponent::RegisterProvider(), with the share rules that
 *  say which roles see which subtrees). The component confines every request to it: no name with "..", an absolute
 *  or drive form, a backslash or a control character reaches this class (and it checks again), names the resource rules
 *  refuse are left out of listings and counted, the share is read-only, roles and the access policy decide what a member sees.
 *
 *  One revision for a file in all three places: Resolve() (what OPEN checks), Stat() and the entries of a listing derive
 *  it from the same size and modification time, "<size>-<modifiedUtcMs>", so a revision of STAT or LIST is the one OPEN
 *  will accept. A directory's revision is "0-<modifiedUtcMs>" of the directory, empty where the file system has no time
 *  for directories (nothing then answers UNCHANGED). Digests are never computed here: a subclass that knows one overrides
 *  GetKnownSha256().
 *
 *  Symbolic links: entries that are links are left out of listings unless followSymlinks is set. A path that is
 *  asked for by name goes through whatever the file system access resolves: a file system access has no way to ask
 *  whether a part of a path is a link, so this class cannot refuse such a path (CDRNFileSystemResourceProvider can, for a
 *  directory of the operating system).
 *
 *  Thread safe as the provider interface asks: the file system access is called from several pool threads at once.
 */
class GUCEF_DRN_EXPORT_CPP CDRNFileSystemAccessResourceProvider : public CIDRNResourceProvider
{
    public:

    /**
     *  @param fileSystem the file system to serve from
     *  @param rootPath   the path of the share's root in that file system
     *  @param bytesBase  where the same files are as a URI, to read their bytes from
     */
    CDRNFileSystemAccessResourceProvider( const CORE::TFileSystemAccessPtr& fileSystem            ,
                                          const CORE::CString& rootPath                          ,
                                          const CORE::CUri& bytesBase                            ,
                                          const CDRNNamespaceShareSettings& settings = CDRNNamespaceShareSettings() );

    virtual ~CDRNFileSystemAccessResourceProvider() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetRootPath( void ) const;
    const CDRNNamespaceShareSettings& GetSettings( void ) const;

    virtual TDRNResourceStatus Resolve( const CORE::CString& name, CDRNResourceInfo& info ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Read( const CORE::CString& name            ,
                                     const CDRNResourceInfo& resolved     ,
                                     UInt64 offset                        ,
                                     UInt32 size                          ,
                                     void* destination                    ,
                                     UInt32& bytesRead                    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetNamespaceCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetNamespaceFlags( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetNamespaceLimits( UInt32& maxStatPaths, UInt32& maxPageEntries ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TDRNResourceStatus Stat( const CORE::CString& name, CDRNNamespaceEntry& entry ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TDRNResourceStatus OpenEnumerator( const CORE::CString& dirName, CIDRNNamespaceEnumeratorPtr& enumerator ) GUCEF_VIRTUAL_OVERRIDE;

    /** Entries left out of listings because they are links, or hidden, since this provider was made */
    UInt64 GetSkippedSymlinkCount( void ) const;
    UInt64 GetSkippedHiddenCount( void ) const;

    protected:

    /**
     *  The SHA-256 of a file if it is known already (a catalogue's), without reading the file. Called for the files STAT, LIST
     *  and Resolve() describe. Default none.
     */
    virtual bool GetKnownSha256( const CORE::CString& name, const CDRNNamespaceEntry& entry, UInt8* digest ) const;

    private:

    friend class CDRNFileSystemAccessEnumerator;

    CDRNFileSystemAccessResourceProvider( const CDRNFileSystemAccessResourceProvider& src );                /**< not implemented */
    CDRNFileSystemAccessResourceProvider& operator=( const CDRNFileSystemAccessResourceProvider& src );     /**< not implemented */

    CORE::CString PathOf( const CORE::CString& name ) const;

    /** Asks the file system access: kind, size, time, revision, flags, digest; the one source of every revision */
    TDRNResourceStatus Inspect( const CORE::CString& name, CDRNNamespaceEntry& entry ) const;

    /** What the entry is from metadata already read: the same rules Inspect() applies */
    void Describe( const CORE::CString& name, bool isDirectory, const CORE::CResourceMetaData& metaData, CDRNNamespaceEntry& entry ) const;

    static TDRNResourceStatus StatusOf( CORE::TFileSystemAccessResult result );

    void CountSkipped( bool symlink ) const;

    CORE::TFileSystemAccessPtr m_fileSystem;
    CORE::CString m_rootPath;
    CDRNUriResourceProvider m_bytes;
    CDRNNamespaceShareSettings m_settings;
    mutable MT::CMutex m_lock;
    mutable UInt64 m_skippedSymlinks;
    mutable UInt64 m_skippedHidden;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNFILESYSTEMACCESSRESOURCEPROVIDER_H ? */
