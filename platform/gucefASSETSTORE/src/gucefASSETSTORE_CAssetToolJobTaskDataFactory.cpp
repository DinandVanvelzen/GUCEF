/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include "gucefASSETSTORE_CAssetToolJobTaskDataFactory.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolJobTaskDataFactory::CAssetToolJobTaskDataFactory( const CORE::CString& taskTypeName )
    : TBaseFactory()
    , m_taskTypeName( taskTypeName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobTaskDataFactory::~CAssetToolJobTaskDataFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobTaskDataFactory::TProductPtr
CAssetToolJobTaskDataFactory::Create( void )
{GUCEF_TRACE;

    CAssetToolJobTaskData* data = GUCEF_NEW CAssetToolJobTaskData( m_taskTypeName );
    CORE::CIDataNodeSerializableTaskData* base = static_cast< CORE::CIDataNodeSerializableTaskData* >( data );
    return TProductPtr( base, this, data );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetToolJobTaskDataFactory::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetToolJobTaskDataFactory( m_taskTypeName );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
