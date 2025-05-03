/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"            /* OS wrapping functionality */
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"          /* data storage node class,.. this is what it's all about */
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"        /* file recource access */
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H
#include "gucefCORE_CPluginFactoryAdapterForDataDrivenDStoreCodec.h"
#define GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H
#endif /* GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H ? */

#include "gucefKAITAI_CKaitaiDStoreCodec.h"  /* definition of the class implemented here */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CKaitaiDStoreCodec::CodecTypeName = "kaitai";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiDStoreCodec::CKaitaiDStoreCodec( void )
    : CORE::CDStoreCodec()    
    , _id( 0 )          
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiDStoreCodec::~CKaitaiDStoreCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CKaitaiDStoreCodec::StoreDataTree( const CORE::CDataNode* tree   ,
                                   CORE::CIOAccess* file         )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiDStoreCodec::BuildDataTree( CORE::CDataNode* treeroot ,
                                   CORE::CIOAccess* file     )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKaitaiDStoreCodec::GetTypeName( void ) const
{GUCEF_TRACE;

    return CodecTypeName;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiDStoreCodec::IsCodecTypeDataDriven( void ) const
{GUCEF_TRACE;

    return true; 
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKaitaiDStoreCodec::GetName( void ) const
{GUCEF_TRACE;

    return "gucefKAITAI"; 
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKaitaiDStoreCodec::GetCopyright( void ) const
{GUCEF_TRACE;

    return "Licensed under the Apache License, Version 2.0 (the \"License\"). Copyright (C) 1998 - 2025.  Dinand Vanvelzen";
}

/*-------------------------------------------------------------------------*/

CORE::TVersion
CKaitaiDStoreCodec::GetVersion( void ) const
{GUCEF_TRACE;

    CORE::TVersion version = { GUCEF_KAITAI_MAJOR_VERSION, GUCEF_KAITAI_MINOR_VERSION, GUCEF_KAITAI_PATCH_VERSION, GUCEF_KAITAI_RELEASE_VERSION }; // major, minor, patch, release
    return version; 
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CKaitaiDStoreCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiDStoreCodec();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
