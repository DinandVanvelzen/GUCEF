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

#ifndef GUCEF_DRN_CDRNRESOURCEHASHER_H
#define GUCEF_DRN_CDRNRESOURCEHASHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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
 *  SHA-256 for resource transfers: the "sha256" codec of the CORE codec registry (the implementation the registry
 *  ranks first: operating system, plugin or built in), CORE::CSha256 when there is none or it fails.
 *  Also the leaf and node hashes of the resource hash tree (domain separated as RFC 9162: 0x00 leaf, 0x01 node).
 *  One object per thread at a time; objects are independent.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceHasher
{
    public:

    static const UInt32 DigestSize = CORE::CSha256::DigestSize;

    CDRNResourceHasher( void );
    ~CDRNResourceHasher();

    void Begin( void );
    void Update( const void* data, UInt32 size );
    void Final( UInt8* digest );

    /** SHA-256 of one buffer */
    void Hash( const void* data, UInt32 size, UInt8* digest );

    /** SHA-256( 0x00 || data ) */
    void HashLeaf( const void* data, UInt32 size, UInt8* digest );

    /** SHA-256( 0x01 || left || right ) */
    void HashNode( const UInt8* left, const UInt8* right, UInt8* digest );

    /** Whether the codec of the registry is in use (else CORE::CSha256) */
    bool IsUsingCodec( void ) const;

    private:

    CDRNResourceHasher( const CDRNResourceHasher& src );                /**< not implemented */
    CDRNResourceHasher& operator=( const CDRNResourceHasher& src );     /**< not implemented */

    void DropCodec( void );

    CORE::CICodecInstancePtr m_instance;
    CORE::CSha256 m_fallback;
    CORE::CDynamicBuffer m_output;
    bool m_useCodec;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCEHASHER_H ? */
