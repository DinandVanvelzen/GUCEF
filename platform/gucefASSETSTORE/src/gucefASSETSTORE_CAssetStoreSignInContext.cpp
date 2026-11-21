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

#include "gucefASSETSTORE_CAssetStoreSignInContext.h"

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

CAssetStoreSignInContext::CAssetStoreSignInContext( void )
    : authManager( GUCEF_NULL )
    , sessionStore( GUCEF_NULL )
    , credentialStore( GUCEF_NULL )
    , keyringSession( GUCEF_NULL )
    , storeSettings( "storeSettings", GUCEF_DATATYPE_OBJECT )
    , browserAvailable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreSignInContext::~CAssetStoreSignInContext()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CAssetStoreSignInContext::GetStoreSettings( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    return storeSettings.FindChild( storeId );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
