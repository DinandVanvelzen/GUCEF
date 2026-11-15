/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#include "gucefDRNUTILS_CDrnUtilsDigest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
AppendNode( const CORE::CDataNode& node, CORE::CString& canonical )
{GUCEF_TRACE;

    canonical += CORE::ToString( node.GetNodeType() ) + ":" + node.GetName() + "=" + node.GetValue().AsString() + "\n";
    CORE::CString::StringMap attributes;
    UInt32 attCount = node.GetAttCount();
    for ( UInt32 a=0; a<attCount; ++a )
    {
        const CORE::CDataNode::TKeyValuePair* att = node.GetAttribute( a );
        if ( GUCEF_NULL != att )
            attributes[ att->first ] = att->second.AsString();
    }
    CORE::CString::StringMap::const_iterator i = attributes.begin();
    while ( i != attributes.end() )
    {
        canonical += "@" + (*i).first + "=" + (*i).second + "\n";
        ++i;
    }
    canonical += "{\n";
    CORE::CDataNode::const_iterator c = node.ConstBegin();
    while ( c != node.ConstEnd() )
    {
        AppendNode( *(*c), canonical );
        ++c;
    }
    canonical += "}\n";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnUtilsDigest::Sha256Hex( const void* data, UInt32 size )
{GUCEF_TRACE;

    static const char* hexDigits = "0123456789abcdef";
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( data, size, digest );
    char text[ CORE::CSha256::DigestSize * 2 + 1 ];
    for ( UInt32 i=0; i<CORE::CSha256::DigestSize; ++i )
    {
        text[ i*2 ] = hexDigits[ ( digest[ i ] >> 4 ) & 0x0F ];
        text[ i*2+1 ] = hexDigits[ digest[ i ] & 0x0F ];
    }
    text[ CORE::CSha256::DigestSize * 2 ] = '\0';
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnUtilsDigest::ArgumentsDigest( const CORE::CString::StringMap& arguments )
{GUCEF_TRACE;

    if ( arguments.empty() )
        return CORE::CString::Empty;
    CORE::CString canonical;
    CORE::CString::StringMap::const_iterator i = arguments.begin();
    while ( i != arguments.end() )
    {
        canonical += (*i).first + "=" + (*i).second + "\n";
        ++i;
    }
    return Sha256Hex( canonical.C_String(), canonical.ByteSize() > 0 ? canonical.ByteSize()-1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsDigest::CanonicalDataNode( const CORE::CDataNode& node, CORE::CString& canonical )
{GUCEF_TRACE;

    canonical.Clear();
    AppendNode( node, canonical );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnUtilsDigest::DataNodeDigest( const CORE::CDataNode& node, UInt32& canonicalBytes )
{GUCEF_TRACE;

    CORE::CString canonical;
    CanonicalDataNode( node, canonical );
    canonicalBytes = canonical.ByteSize() > 0 ? canonical.ByteSize()-1 : 0;
    return Sha256Hex( canonical.C_String(), canonicalBytes );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
