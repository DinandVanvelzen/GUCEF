/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#include "AncestryBackup_CJsonShape.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CJsonShape::Describe( const CORE::CDataNode& root, SJsonShape& shape, UInt32 maxDepth, UInt32 maxLines )
{GUCEF_TRACE;

    WEB::CWebBodyShape::Describe( root, shape, maxDepth, maxLines, true );
}

/*-------------------------------------------------------------------------*/

bool
CJsonShape::IsPaginationKey( const CORE::CString& key )
{GUCEF_TRACE;

    return WEB::CWebBodyShape::IsPaginationKey( key );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CJsonShape::GetTypeName( const CORE::CVariant& value )
{GUCEF_TRACE;

    return WEB::CWebBodyShape::GetTypeName( value );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
