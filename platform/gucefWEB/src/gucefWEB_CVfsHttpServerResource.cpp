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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h" 
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "gucefWEB_CVfsHttpServerResource.h"

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

CVfsHttpServerResource::CVfsHttpServerResource( void )
    : CDefaultHTTPServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CVfsHttpServerResource::CVfsHttpServerResource( const CVfsHttpServerResource& src )
    : CDefaultHTTPServerResource( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CVfsHttpServerResource::~CVfsHttpServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CVfsHttpServerResource&
CVfsHttpServerResource::operator=( const CVfsHttpServerResource& src )
{GUCEF_TRACE;

    CDefaultHTTPServerResource::operator=( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CString 
CVfsHttpServerResource::GetBestMatchedSerializationRepresentation( const CORE::CString& resourcePath                            ,
                                                                   const CVfsHttpServerResource::TStringVector& representations )
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
CVfsHttpServerResource::GetBestSupportedDeserializationRepresentation( const CORE::CString& resourcePath    ,
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

CVfsHttpServerResource::TCreateState 
CVfsHttpServerResource::CreateResource( const CORE::CString& resourcePath             ,
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
CVfsHttpServerResource::DeleteResource( const CORE::CString& resourcePath )
{GUCEF_TRACE;

    // For the dummy resource everything always fake succeeds
    return VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( resourcePath, true );
}

/*-------------------------------------------------------------------------*/

bool 
CVfsHttpServerResource::Serialize( const CORE::CString& resourcePath  ,
                                   CORE::CDynamicBuffer& outputBuffer ,
                                   const CString& representation      ,
                                   const CString& params              )
{GUCEF_TRACE;

    bool success = VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( outputBuffer, resourcePath, "rb" );
    return true;
}

/*-------------------------------------------------------------------------*/

CVfsHttpServerResource::TDeserializeState 
CVfsHttpServerResource::Deserialize( const CORE::CString& resourcePath       ,
                                     const CORE::CDynamicBuffer& inputBuffer ,
                                     const CString& representation           ,
                                     bool isDeltaUpdateOnly                  )
{GUCEF_TRACE;

    if ( isDeltaUpdateOnly )
        return CIHTTPServerResource::DESERIALIZESTATE_NOTSUPPORTED;
    
    bool success = VFS::CVfsGlobal::Instance()->GetVfs().StoreAsFile( resourcePath, inputBuffer, 0, true );
    return CIHTTPServerResource::DESERIALIZESTATE_SUCCEEDED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace WEB */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
