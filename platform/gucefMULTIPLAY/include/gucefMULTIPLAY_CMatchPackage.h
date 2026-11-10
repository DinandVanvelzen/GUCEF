/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#define GUCEF_MULTIPLAY_CMATCHPACKAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The opaque payload that goes from machine to machine for a revision of a match, with its integrity: the
 *  SHA-256 of the payload, the revision and the revision and SHA-256 of the package it follows (the chain
 *  that receivers check). The generic layer never opens the payload.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchPackage : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 MaxPayloadSize = 16 * 1024 * 1024;      /**< of the raw payload; the container is capped at 2 MB */

    CORE::CDynamicBuffer payload;
    CORE::CString sha256;               /**< hex, of the payload */
    UInt32 revision;
    UInt32 baseRevision;                /**< 0 for the first package */
    CORE::CString baseSha256;           /**< hex, empty for the first package */

    CMatchPackage( void );

    CMatchPackage( const CMatchPackage& src );

    virtual ~CMatchPackage();

    CMatchPackage& operator=( const CMatchPackage& src );

    /** Sets the payload and the sha256 that goes with it */
    void SetPayload( const void* data, UInt32 size );

    /** Whether sha256 is the SHA-256 of the payload */
    bool IsConsistent( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHPACKAGE_H ? */
