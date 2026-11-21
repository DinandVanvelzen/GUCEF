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

#ifndef GUCEF_DRN_CDRNRESOURCEPARTIALSTATE_H
#define GUCEF_DRN_CDRNRESOURCEPARTIALSTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEINFO_H
#include "gucefDRN_CDRNResourceInfo.h"
#define GUCEF_DRN_CDRNRESOURCEINFO_H
#endif /* GUCEF_DRN_CDRNRESOURCEINFO_H ? */

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
 *  What a resumable target holds of a resource: which resource (provider, name, the resolved info with its revision
 *  and digests) and which chunks were written, one bit per chunk.
 *
 *  Persisted form (Serialize): magic "GUDRNRS1", u8 format version 1, string provider, string name, varint64 size,
 *  varint64 modifiedUtcMs, string revision, varint chunkSize, u8 digest flags, the digests, varint64 chunk count,
 *  sized bytes bitmap, then the SHA-256 of all of it. A state that fails any of that is not taken.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourcePartialState
{
    public:

    static const UInt8 FormatVersion = 1;

    CDRNResourcePartialState( void );

    void Clear( void );

    /** A fresh state for this resource, no chunk held */
    bool Reset( const CORE::CString& provider, const CORE::CString& name, const CDRNResourceInfo& info );

    /** Same provider and name, and the same revision of the resource (CDRNResourceInfo::IsSameRevisionAs()) */
    bool IsOf( const CORE::CString& provider, const CORE::CString& name, const CDRNResourceInfo& info ) const;

    bool IsValid( void ) const;
    const CORE::CString& GetProvider( void ) const;
    const CORE::CString& GetName( void ) const;
    const CDRNResourceInfo& GetInfo( void ) const;

    UInt64 GetChunkCount( void ) const;
    UInt64 GetHeldCount( void ) const;
    bool IsHeld( UInt64 chunkIndex ) const;
    void SetHeld( UInt64 chunkIndex, bool held );

    /** Bytes of the chunks held */
    UInt64 GetHeldBytes( void ) const;

    /** The first chunk at or after 'from' that is not held, the chunk count when there is none */
    UInt64 FindFirstMissing( UInt64 from ) const;

    /** The first chunk at or after 'from' that is held, the chunk count when there is none */
    UInt64 FindFirstHeld( UInt64 from ) const;

    void Serialize( CORE::CDynamicBuffer& output ) const;

    bool Deserialize( const CORE::CDynamicBuffer& input );

    private:

    CORE::CString m_provider;
    CORE::CString m_name;
    CDRNResourceInfo m_info;
    CORE::CDynamicBuffer m_bitmap;
    UInt64 m_chunkCount;
    UInt64 m_heldCount;
    bool m_valid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCEPARTIALSTATE_H ? */
