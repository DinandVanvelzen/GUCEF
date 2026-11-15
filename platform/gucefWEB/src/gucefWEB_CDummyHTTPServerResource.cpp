/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefWEB_CDummyHTTPServerResource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDummyHTTPServerResource::CDummyHTTPServerResource( void )
    : CDefaultHTTPServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CDummyHTTPServerResource::CDummyHTTPServerResource( const CDummyHTTPServerResource& src )
    : CDefaultHTTPServerResource( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CDummyHTTPServerResource::~CDummyHTTPServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CDummyHTTPServerResource&
CDummyHTTPServerResource::operator=( const CDummyHTTPServerResource& src )
{GUCEF_TRACE;

    CDefaultHTTPServerResource::operator=( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CString 
CDummyHTTPServerResource::GetBestMatchedSerializationRepresentation( const CORE::CString& resourcePath                              ,
                                                                     const CDummyHTTPServerResource::TStringVector& representations )
{GUCEF_TRACE;

    if ( !representations.empty() )
    {
        // For the dummy resource we always want to fake success
        // Same thing for the MIME types, we always match the first ask
        return *representations.begin();
    }
    return CString();
}

/*-------------------------------------------------------------------------*/

CString 
CDummyHTTPServerResource::GetBestSupportedDeserializationRepresentation( const CORE::CString& resourcePath    ,
                                                                         const TStringVector& representations )
{GUCEF_TRACE;

    if ( !representations.empty() )
    {
        // For the dummy resource we always want to fake success
        // Same thing for the MIME types, we always match the first ask
        return *representations.begin();
    }
    return CString();
}

/*-------------------------------------------------------------------------*/

CDummyHTTPServerResource::TCreateState 
CDummyHTTPServerResource::CreateResource( const CORE::CString& resourcePath             ,
                                          const CString& transactionID                  ,
                                          const CORE::CDynamicBuffer& inputBuffer       ,
                                          const CString& representation                 ,
                                          const CString& params                         ,
                                          THTTPServerResourcePtr& resourceOutput        ,
                                          TStringVector& supportedRepresentationsOutput )
{GUCEF_TRACE;

    // For the dummy resource everything always fake succeeds
    return CIHTTPServerResource::CREATESTATE_CREATED;
}

/*-------------------------------------------------------------------------*/
    
bool 
CDummyHTTPServerResource::DeleteResource( const CORE::CString& resourcePath )
{GUCEF_TRACE;

    // For the dummy resource everything always fake succeeds
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CDummyHTTPServerResource::Serialize( const CORE::CString& resourcePath  ,
                                     CORE::CDynamicBuffer& outputBuffer ,
                                     const CString& representation      ,
                                     const CString& params              )
{GUCEF_TRACE;

    // For the dummy resource everything always fake succeeds
    return true;
}

/*-------------------------------------------------------------------------*/

CDummyHTTPServerResource::TDeserializeState 
CDummyHTTPServerResource::Deserialize( const CORE::CString& resourcePath       ,
                                       const CORE::CDynamicBuffer& inputBuffer ,
                                       const CString& representation           ,
                                       bool isDeltaUpdateOnly                  )
{GUCEF_TRACE;

    // For the dummy resource everything always fake succeeds
    return CIHTTPServerResource::DESERIALIZESTATE_SUCCEEDED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
