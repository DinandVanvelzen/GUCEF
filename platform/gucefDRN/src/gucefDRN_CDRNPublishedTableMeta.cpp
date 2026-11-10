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

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

#include "gucefDRN_CDRNPublishedTableMeta.h"

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

CDRNPublishedTableMeta::CDRNPublishedTableMeta( void )
    : pageRevision( 0 )
    , sourceVersion( 0 )
    , rowCount( 0 )
    , changedRowCount( 0 )
    , scalars()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNPublishedTableMeta::Encode( CORE::CVariantStream& stream ) const
{GUCEF_TRACE;

    UInt32 scalarCount = scalars.GetKeyCount();
    stream << EncodingVersion << pageRevision << sourceVersion << rowCount << changedRowCount << scalarCount;
    for ( UInt32 i=0; i<scalarCount; ++i )
    {
        const CORE::CValueList::TVariantVector& values = scalars.GetValueVector( i );
        stream << scalars.GetKey( i ).AsString() << ( values.empty() ? CORE::CVariant() : values.front() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNPublishedTableMeta::Decode( CORE::CVariantStream& stream, bool& unsupported )
{GUCEF_TRACE;

    unsupported = false;
    UInt8 version = 0;
    stream >> version;
    if ( !stream.IsValid() )
        return false;
    if ( version > EncodingVersion )
    {
        unsupported = true;
        return false;
    }
    UInt32 scalarCount = 0;
    stream >> pageRevision >> sourceVersion >> rowCount >> changedRowCount >> scalarCount;
    if ( !stream.IsValid() || scalarCount > 65536 )
        return false;
    scalars.DeleteAll();
    for ( UInt32 i=0; i<scalarCount; ++i )
    {
        CORE::CString name;
        CORE::CVariant value;
        stream >> name >> value;
        if ( !stream.IsValid() )
            return false;
        scalars.Set( name, value );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
