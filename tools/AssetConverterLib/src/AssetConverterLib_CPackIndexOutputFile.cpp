/*
 *  AssetConverterLib: one converted output file of a resource of the pack index
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

#include "AssetConverterLib_CPackIndexOutputFile.h"

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

CPackIndexOutputFile::CPackIndexOutputFile( void )
    : path()
    , sha256()
    , size( 0 )
    , profile()
    , kind()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexOutputFile::IsEmpty( void ) const
{GUCEF_TRACE;

    return path.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexOutputFile::operator==( const CPackIndexOutputFile& other ) const
{GUCEF_TRACE;

    return path == other.path && sha256 == other.sha256 && size == other.size && profile == other.profile && kind == other.kind;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexOutputFile::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "path", path );
    CPackIndexSerial::SetText( node, "sha256", sha256 );
    CPackIndexSerial::SetCount( node, "size", size );
    CPackIndexSerial::SetText( node, "profile", profile );
    CPackIndexSerial::SetText( node, "kind", kind );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexOutputFile::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    path = CPackIndexSerial::GetText( node, "path" );
    sha256 = CPackIndexSerial::GetText( node, "sha256" );
    size = CPackIndexSerial::GetCount( node, "size" );
    profile = CPackIndexSerial::GetText( node, "profile" );
    kind = CPackIndexSerial::GetText( node, "kind" );
    return !path.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
