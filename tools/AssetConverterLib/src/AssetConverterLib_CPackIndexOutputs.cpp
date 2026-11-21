/*
 *  AssetConverterLib: the converted outputs of a resource of the pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#include "AssetConverterLib_CPackIndexOutputs.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackIndexOutputs::CPackIndexOutputs( void )
    : intermediate()
    , cacheKey()
    , sidecar()
    , cooked()
    , lossy( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexOutputs::IsEmpty( void ) const
{GUCEF_TRACE;

    return intermediate.IsEmpty() && cacheKey.IsNULLOrEmpty() && sidecar.IsEmpty() && cooked.empty() && 0 == lossy;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexOutputs::operator==( const CPackIndexOutputs& other ) const
{GUCEF_TRACE;

    return intermediate == other.intermediate && cacheKey == other.cacheKey && sidecar == other.sidecar && cooked == other.cooked && lossy == other.lossy;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexOutputs::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( !intermediate.IsEmpty() || !cacheKey.IsNULLOrEmpty() )
    {
        CORE::CDataNode* im = node.AddChild( "intermediate", GUCEF_DATATYPE_OBJECT );
        intermediate.Serialize( *im );
        CPackIndexSerial::SetText( *im, "cacheKey", cacheKey );
        if ( !sidecar.IsEmpty() )
        {
            CORE::CDataNode* sc = im->AddChild( "sidecar", GUCEF_DATATYPE_OBJECT );
            CPackIndexSerial::SetTextAlways( *sc, "path", sidecar.path );
            CPackIndexSerial::SetText( *sc, "sha256", sidecar.sha256 );
        }
    }
    if ( !cooked.empty() )
    {
        CORE::CDataNode* list = node.AddChild( "cooked", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<cooked.size(); ++i )
            cooked[ i ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }
    CPackIndexSerial::SetCount( node, "lossy", lossy );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexOutputs::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CPackIndexOutputs();
    const CORE::CDataNode* im = node.FindChild( "intermediate" );
    if ( GUCEF_NULL != im )
    {
        intermediate.Deserialize( *im );
        cacheKey = CPackIndexSerial::GetText( *im, "cacheKey" );
        const CORE::CDataNode* sc = im->FindChild( "sidecar" );
        if ( GUCEF_NULL != sc )
            sidecar.Deserialize( *sc );
    }
    const CORE::CDataNode* list = node.FindChild( "cooked" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            CPackIndexOutputFile file;
            if ( file.Deserialize( *(*i) ) )
                cooked.push_back( file );
            ++i;
        }
    }
    lossy = (UInt32) CPackIndexSerial::GetCount( node, "lossy" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
