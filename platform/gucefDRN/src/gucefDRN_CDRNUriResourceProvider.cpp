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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#include "gucefDRN_CDRNUriResourceProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNUriResourceProvider::CDRNUriResourceProvider( const CORE::CUri& baseUri )
    : CIDRNResourceProvider()
    , m_baseUri( baseUri )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNUriResourceProvider::~CDRNUriResourceProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CUri&
CDRNUriResourceProvider::GetBaseUri( void ) const
{GUCEF_TRACE;

    return m_baseUri;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNUriResourceProvider::GetDescription( void ) const
{GUCEF_TRACE;

    return "uri:" + m_baseUri.ToUriInStringForm();
}

/*-------------------------------------------------------------------------*/

CORE::CUri
CDRNUriResourceProvider::GetUriOf( const CORE::CString& name ) const
{GUCEF_TRACE;

    // the path is set as it is, not parsed: a name is taken literally, whatever characters it has
    CORE::CUri uri( m_baseUri );
    CORE::CString path = m_baseUri.GetPath();
    if ( !path.EndsWith( "/" ) )
        path += "/";
    uri.SetPath( path + name );
    return uri;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNUriResourceProvider::Inspect( const CORE::CUri& uri, UInt64& size, UInt64& modifiedUtcMs ) const
{GUCEF_TRACE;

    size = 0;
    modifiedUtcMs = 0;
    CORE::CUriResourceAccessorFactory::TProductPtr accessor = CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().CreateAccessor( uri );
    if ( accessor.IsNULL() )
        return DRN_RESSTATUS_NOT_FOUND;
    CORE::CResourceMetaData metaData;
    if ( !accessor->GetResourceMetaData( uri, metaData ) || !metaData.resourceExists )
        return DRN_RESSTATUS_NOT_FOUND;
    if ( accessor->IsACollectionResource( uri ) )
        return DRN_RESSTATUS_NOT_FOUND;
    if ( metaData.hasResourceSizeInBytes )
    {
        size = metaData.resourceSizeInBytes;
    }
    else
    {
        // an accessor whose metadata has no size (the "file" scheme's): the size of the resource as it is opened
        CORE::IOAccessPtr access;
        if ( !accessor->GetResourceAccess( uri, access, CORE::CUriResourceAccessor::URI_RESOURCEACCESS_MODE_READ ) || access.IsNULL() )
            return DRN_RESSTATUS_NOT_FOUND;
        size = access->GetSize();
        access->Close();
    }
    if ( metaData.hasModifiedDateTime )
        modifiedUtcMs = metaData.modifiedDateTime.ToUnixEpochBasedTicksInMillisecs();
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNUriResourceProvider::Resolve( const CORE::CString& name, CDRNResourceInfo& info )
{GUCEF_TRACE;

    info.Clear();
    if ( !CDRNResourceName::IsValidResourceName( name ) )
        return DRN_RESSTATUS_INVALID_NAME;
    UInt64 size = 0;
    UInt64 modifiedUtcMs = 0;
    TDRNResourceStatus status = Inspect( GetUriOf( name ), size, modifiedUtcMs );
    if ( DRN_RESSTATUS_OK != status )
        return status;
    info.name = name;
    info.size = size;
    info.modifiedUtcMs = modifiedUtcMs;
    info.revision = CDRNResourceInfo::MakeRevision( size, modifiedUtcMs );
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNUriResourceProvider::Read( const CORE::CString& name            ,
                               const CDRNResourceInfo& resolved     ,
                               UInt64 offset                        ,
                               UInt32 size                          ,
                               void* destination                    ,
                               UInt32& bytesRead                    )
{GUCEF_TRACE;

    bytesRead = 0;
    if ( !CDRNResourceName::IsValidResourceName( name ) )
        return DRN_RESSTATUS_INVALID_NAME;
    CORE::CUri uri = GetUriOf( name );
    UInt64 currentSize = 0;
    UInt64 modifiedUtcMs = 0;
    TDRNResourceStatus status = Inspect( uri, currentSize, modifiedUtcMs );
    if ( DRN_RESSTATUS_OK != status )
        return DRN_RESSTATUS_CHANGED;
    if ( currentSize != resolved.size || CDRNResourceInfo::MakeRevision( currentSize, modifiedUtcMs ) != resolved.revision )
        return DRN_RESSTATUS_CHANGED;
    return ReadFromUri( uri, currentSize, offset, size, destination, bytesRead );
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNUriResourceProvider::ReadFromUri( const CORE::CUri& uri, UInt64 resourceSize, UInt64 offset, UInt32 size, void* destination, UInt32& bytesRead )
{GUCEF_TRACE;

    bytesRead = 0;
    if ( offset > resourceSize )
        return DRN_RESSTATUS_OUT_OF_RANGE;
    if ( 0 == size || offset == resourceSize )
        return DRN_RESSTATUS_OK;

    CORE::CUriResourceAccessorFactory::TProductPtr accessor = CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().CreateAccessor( uri );
    if ( accessor.IsNULL() )
        return DRN_RESSTATUS_IO_ERROR;
    CORE::IOAccessPtr access;
    // an accessor may answer true without giving access: checked, not trusted
    if ( !accessor->GetResourceAccess( uri, access, CORE::CUriResourceAccessor::URI_RESOURCEACCESS_MODE_READ ) || access.IsNULL() )
        return DRN_RESSTATUS_IO_ERROR;
    if ( !access->Opened() )
        access->Open();
    if ( !access->Opened() || 0 != access->Setpos( offset ) )
    {
        access->Close();
        return DRN_RESSTATUS_IO_ERROR;
    }
    UInt64 left = resourceSize - offset;
    UInt32 toRead = left < size ? (UInt32) left : size;
    bytesRead = access->Read( destination, 1, toRead );
    access->Close();
    return bytesRead == toRead ? DRN_RESSTATUS_OK : DRN_RESSTATUS_IO_ERROR;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
