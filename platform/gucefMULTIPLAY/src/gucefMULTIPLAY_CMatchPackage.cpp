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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#include "gucefMULTIPLAY_CMatchPackage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchPackage::CMatchPackage( void )
    : CORE::CIDataNodeSerializable()
    , payload()
    , sha256()
    , revision( 0 )
    , baseRevision( 0 )
    , baseSha256()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchPackage::CMatchPackage( const CMatchPackage& src )
    : CORE::CIDataNodeSerializable( src )
    , payload( src.payload )
    , sha256( src.sha256 )
    , revision( src.revision )
    , baseRevision( src.baseRevision )
    , baseSha256( src.baseSha256 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchPackage::~CMatchPackage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchPackage&
CMatchPackage::operator=( const CMatchPackage& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        payload = src.payload;
        sha256 = src.sha256;
        revision = src.revision;
        baseRevision = src.baseRevision;
        baseSha256 = src.baseSha256;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMatchPackage::SetPayload( const void* data, UInt32 size )
{GUCEF_TRACE;

    payload.Clear();
    if ( GUCEF_NULL != data && size > 0 )
        payload.Append( data, size );
    sha256 = CMatchSigning::Sha256Hex( payload.GetConstBufferPtr(), payload.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CMatchPackage::IsConsistent( void ) const
{GUCEF_TRACE;

    return sha256 == CMatchSigning::Sha256Hex( payload.GetConstBufferPtr(), payload.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CMatchPackage::Serialize( CORE::CDataNode& domRootNode                        ,
                          const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "sha256", sha256 );
    domRootNode.SetAttribute( "revision", revision );
    domRootNode.SetAttribute( "baseRevision", baseRevision );
    domRootNode.SetAttribute( "baseSha256", baseSha256 );
    domRootNode.SetAttribute( "payload", CORE::Base64Encode( payload.GetConstBufferPtr(), payload.GetDataSize() ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchPackage::Deserialize( const CORE::CDataNode& domRootNode                  ,
                            const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    sha256 = domRootNode.GetAttributeValueOrChildValueByName( "sha256" ).AsString( CORE::CString::Empty, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt32( 0, true );
    baseRevision = domRootNode.GetAttributeValueOrChildValueByName( "baseRevision" ).AsUInt32( 0, true );
    baseSha256 = domRootNode.GetAttributeValueOrChildValueByName( "baseSha256" ).AsString( CORE::CString::Empty, true );
    payload.Clear();
    CORE::CString encoded = domRootNode.GetAttributeValueOrChildValueByName( "payload" ).AsString( CORE::CString::Empty, true );

    // the decoded size is never more than three quarters of the text: refused before anything is made
    if ( encoded.Length() / 4 * 3 > MaxPayloadSize )
        return false;
    if ( encoded.Length() > 0 )
    {
        payload.SetBufferSize( encoded.Length() );
        UInt32 used = 0;
        if ( !CORE::Base64Decode( encoded, payload.GetBufferPtr(), encoded.Length(), used ) )
        {
            payload.Clear();
            return false;
        }
        payload.SetDataSize( used );
    }
    return IsConsistent();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchPackage::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchPackage( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchPackage::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchPackage";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchPackage::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchPackage::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchPackage";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
