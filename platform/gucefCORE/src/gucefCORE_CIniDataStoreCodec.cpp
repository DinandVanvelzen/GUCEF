/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODEC_H
#include "CDStoreCodec.h"
#define GUCEF_CORE_CDSTORECODEC_H
#endif /* GUCEF_CORE_CDSTORECODEC_H ? */

#ifndef GUCEF_CORE_CINIPARSER_H
#include "gucefCORE_CIniParser.h"
#define GUCEF_CORE_CINIPARSER_H
#endif /* GUCEF_CORE_CINIPARSER_H ? */

#include "gucefCORE_CIniDataStoreCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CIniDataStoreCodec::CIniDataStoreCodec( void )
    : CDStoreCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIniDataStoreCodec::~CIniDataStoreCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIniDataStoreCodec::IsValid( void ) const
{GUCEF_TRACE;
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIniDataStoreCodec::StoreDataTree( const CDataNode* tree   ,
                                   const CString& filename )
{GUCEF_TRACE;

    CFileAccess fileAccess( filename, "wb" );
    return StoreDataTree( tree, &fileAccess );
}

/*-------------------------------------------------------------------------*/

bool
CIniDataStoreCodec::StoreDataTree( const CDataNode* tree   ,
                                   CIOAccess* file         )
{GUCEF_TRACE;

    CIniParser parser;
    if ( parser.LoadFrom( *tree ) )
    {
        return parser.SaveTo( *file );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIniDataStoreCodec::BuildDataTree( CDataNode* treeroot     ,
                                   const CString& filename )
{GUCEF_TRACE;

    CFileAccess fileAccess( filename, "rb" );
    return BuildDataTree( treeroot, &fileAccess );
}

/*-------------------------------------------------------------------------*/

bool
CIniDataStoreCodec::BuildDataTree( CDataNode* treeroot ,
                                   CIOAccess* file     )
{GUCEF_TRACE;

    CIniParser parser;
    if ( parser.LoadFrom( *file ) )
    {
        return parser.SaveTo( *treeroot );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CString
CIniDataStoreCodec::GetName( void ) const
{GUCEF_TRACE;

    return "Build-in GUCEF data store codec for the \'ini\' format";
}

/*-------------------------------------------------------------------------*/

CString
CIniDataStoreCodec::GetTypeName( void ) const
{GUCEF_TRACE;
    
    return "ini";
}

/*-------------------------------------------------------------------------*/

bool 
CIniDataStoreCodec::IsCodecTypeDataDriven( void ) const
{GUCEF_TRACE;

    return false; 
}

/*-------------------------------------------------------------------------*/

CString
CIniDataStoreCodec::GetCopyright( void ) const
{GUCEF_TRACE;

    return "Prevailing GUCEF license at " __TIMESTAMP__;
}

/*-------------------------------------------------------------------------*/

TVersion
CIniDataStoreCodec::GetVersion( void ) const
{GUCEF_TRACE;

    TVersion version;
    version.major = 1;
    version.minor = 0;
    version.patch = 0;
    version.release = 0;
    return version;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CIniDataStoreCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CIniDataStoreCodec();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
