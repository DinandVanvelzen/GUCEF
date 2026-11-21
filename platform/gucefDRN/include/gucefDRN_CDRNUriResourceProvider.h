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

#ifndef GUCEF_DRN_CDRNURIRESOURCEPROVIDER_H
#define GUCEF_DRN_CDRNURIRESOURCEPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#include "gucefDRN_CIDRNResourceProvider.h"
#define GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#endif /* GUCEF_DRN_CIDRNRESOURCEPROVIDER_H ? */

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
 *  Serves the resources below a base URI through the URI resource accessor of its scheme (the accessor factory of
 *  gucefCORE): "vfs://<mount>/<dir>" serves a subtree of the virtual file system once gucefVFS registered its "vfs"
 *  scheme, without a dependency of gucefDRN on gucefVFS; other registered schemes likewise.
 *
 *  A resource name is appended to the path of the base URI. Resolve asks the accessor for the metadata (size,
 *  modification time); reads take resource access in read mode and position with Setpos (f-function semantics, 0 is
 *  success). Every read asks the metadata again and answers CHANGED when size or modification time moved. What the
 *  scheme resolves (mounts, archives, links) is the accessor's: the name checks of the facility keep "." and ".."
 *  segments out of the path.
 */
class GUCEF_DRN_EXPORT_CPP CDRNUriResourceProvider : public CIDRNResourceProvider
{
    public:

    explicit CDRNUriResourceProvider( const CORE::CUri& baseUri );

    virtual ~CDRNUriResourceProvider() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CUri& GetBaseUri( void ) const;

    /** The URI of a resource: the base with the name appended to its path */
    CORE::CUri GetUriOf( const CORE::CString& name ) const;

    virtual TDRNResourceStatus Resolve( const CORE::CString& name, CDRNResourceInfo& info ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Read( const CORE::CString& name            ,
                                     const CDRNResourceInfo& resolved     ,
                                     UInt64 offset                        ,
                                     UInt32 size                          ,
                                     void* destination                    ,
                                     UInt32& bytesRead                    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads [offset, offset + size) of the resource at 'uri', which is 'resourceSize' bytes, through the accessor of its
     *  scheme (GetResourceAccess() in read mode, Setpos(), Read()): what Read() does once the revision matches, for other
     *  providers that know the bytes by URI (CDRNFileSystemAccessResourceProvider).
     */
    static TDRNResourceStatus ReadFromUri( const CORE::CUri& uri, UInt64 resourceSize, UInt64 offset, UInt32 size, void* destination, UInt32& bytesRead );

    private:

    CDRNUriResourceProvider( const CDRNUriResourceProvider& src );              /**< not implemented */
    CDRNUriResourceProvider& operator=( const CDRNUriResourceProvider& src );   /**< not implemented */

    TDRNResourceStatus Inspect( const CORE::CUri& uri, UInt64& size, UInt64& modifiedUtcMs ) const;

    CORE::CUri m_baseUri;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNURIRESOURCEPROVIDER_H ? */
