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

#ifndef GUCEF_CORE_CFILESYSTEMURIRESOURCEACCESSOR_H
#define GUCEF_CORE_CFILESYSTEMURIRESOURCEACCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CURIRESOURCEACCESSOR_H
#include "gucefCORE_CUriResourceAccessor.h"
#define GUCEF_CORE_CURIRESOURCEACCESSOR_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSOR_H ? */

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

/**
 *  Class for sync blocking CRUD access to a file system resource abstracted through a URI
 *  This class is threadsafe in the sense that it is stateless
 */
class GUCEF_CORE_PUBLIC_CPP CFileSystemUriResourceAccessor : public CUriResourceAccessor
{
    public:

    static const CString SchemeName;

    virtual bool CreateResource( const CUri& uri ,
                                 CIOAccess& src  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetResource( const CUri& uri        ,
                              CIOAccess& destination ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetPartialResource( const CUri& uri        ,
                                     UInt64 byteOffset      ,
                                     Int64 bytesToGet       ,
                                     CIOAccess& destination ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetResourceAccess( const CUri& uri               ,
                                    IOAccessPtr& accessToResource ,
                                    TURI_RESOURCEACCESS_MODE mode ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetResourceMetaData( const CUri& uri             ,
                                      CResourceMetaData& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UpdateResource( const CUri& uri ,
                                 CIOAccess& src  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UpdatePartialResource( const CUri& uri    ,
                                        UInt64 byteOffset  ,
                                        Int64 bytesToWrite ,
                                        CIOAccess& src     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UpdatePartialResource( const CUri& uri    ,
                                        CIOAccess& src     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetSupportedOperations( const CUri& uri                     ,
                                         CUriResourceAccessorOperations& ops ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeleteResource( const CUri& uri ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsACollectionResource( const CUri& uri ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ResolveResourcesInCollection( const CUri& uri        ,
                                               UriVector& resources   ,
                                               bool recursive         ,
                                               bool addCollectionUris ) GUCEF_VIRTUAL_OVERRIDE;

    CFileSystemUriResourceAccessor( void ); 
    CFileSystemUriResourceAccessor( const CFileSystemUriResourceAccessor& src );
    virtual ~CFileSystemUriResourceAccessor() GUCEF_VIRTUAL_OVERRIDE; 
    CFileSystemUriResourceAccessor& operator=( const CFileSystemUriResourceAccessor& src );
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CFileSystemUriResourceAccessor, MT::CMutex > CFileSystemUriResourceAccessorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CFILESYSTEMURIRESOURCEACCESSOR_H ? */
