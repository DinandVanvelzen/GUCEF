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

#include "gucefASSETSTORE_CAssetToolJobTaskConsumerFactory.h"

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

CAssetToolJobTaskConsumerFactory::CAssetToolJobTaskConsumerFactory( CAssetToolContext* context, const CORE::CString& taskTypeName )
    : TBaseFactory()
    , m_context( context )
    , m_taskTypeName( taskTypeName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobTaskConsumerFactory::~CAssetToolJobTaskConsumerFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobTaskConsumerFactory::TProductPtr
CAssetToolJobTaskConsumerFactory::Create( void )
{GUCEF_TRACE;

    CAssetToolJobTaskConsumer* task = GUCEF_NEW CAssetToolJobTaskConsumer( m_context, m_taskTypeName );
    CORE::CTaskConsumer* base = static_cast< CORE::CTaskConsumer* >( task );
    return TProductPtr( base, this, task );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetToolJobTaskConsumerFactory::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetToolJobTaskConsumerFactory( m_context, m_taskTypeName );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
