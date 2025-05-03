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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "CDStoreCodec.h"       /* definition of the class implemented here */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CDStoreCodec::CDStoreCodec( const CDStoreCodec& src )
    : CICloneable( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDStoreCodec::CDStoreCodec( void )
    : CICloneable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDStoreCodec::~CDStoreCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDStoreCodec&
CDStoreCodec::operator=( const CDStoreCodec& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodec::StoreDataTree( const CDataNode* tree   ,
                             const CString& filename )
{GUCEF_TRACE;

    CreatePathDirectories( filename );

    CFileAccess access( filename, "wb" );
    if ( access.IsValid() )
    {
        return StoreDataTree( tree, &access );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodec::BuildDataTree( CDataNode* treeroot     ,
                             const CString& filename )

{GUCEF_TRACE;

    CFileAccess access( filename );
    if ( access.IsValid() )
    {
        return BuildDataTree( treeroot, &access );
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
