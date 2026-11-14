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

#ifndef GUCEF_CORE_CURIRESOURCEACCESSOR_H
#define GUCEF_CORE_CURIRESOURCEACCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

// remove defines defensively, mainly an issue on MS Windows
#undef DeleteResource
#undef CreateResource
#undef UpdateResource

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
 *  Base class for sync blocking CRUD access to a resource abstracted through a URI
 *  This class is threadsafe in the sense that it is stateless
 */
class GUCEF_CORE_PUBLIC_CPP CUriResourceAccessor
{
    public:

    typedef CUri::UriVector     UriVector;
    typedef CUri::UriSet        UriSet;

    enum URI_RESOURCEACCESS_MODE GUCEF_ENUM_BASE( UInt8 )
    {
        URI_RESOURCEACCESS_MODE_UNDEFINED = 0,

        URI_RESOURCEACCESS_MODE_READ,
        URI_RESOURCEACCESS_MODE_WRITE,
        URI_RESOURCEACCESS_MODE_APPEND
    };
    typedef enum URI_RESOURCEACCESS_MODE  TURI_RESOURCEACCESS_MODE;

    class GUCEF_CORE_PUBLIC_CPP CUriResourceAccessorOperations
    {
        public:
        bool createResource;
        bool getResource;
        bool getPartialResource;
        bool getResourceMetaData;
        bool getResourceWriteAccess;
        bool getResourceAppendAccess;
        bool getResourceReadAccess;
        bool updateResource;
        bool updatePartialResource;
        bool deleteResource;
        bool collectionDetermnination;
        bool collectionResolution;

        CUriResourceAccessorOperations( void );
        CUriResourceAccessorOperations( const CUriResourceAccessorOperations& src );
    };

    virtual bool CreateResource( const CUri& uri                 ,
                                 const CDynamicBuffer& srcBuffer );

    virtual bool CreateResource( const CUri& uri ,
                                 CIOAccess& src  ) = 0;

    virtual bool GetResource( const CUri& uri                   ,
                              CDynamicBuffer& destinationBuffer );

    virtual bool GetResource( const CUri& uri        ,
                              CIOAccess& destination ) = 0;

    virtual bool GetPartialResource( const CUri& uri                   ,
                                     UInt64 byteOffset                 ,
                                     Int64 bytesToGet                  ,
                                     CDynamicBuffer& destinationBuffer );

    virtual bool GetPartialResource( const CUri& uri        ,
                                     UInt64 byteOffset      ,
                                     Int64 bytesToGet       ,
                                     CIOAccess& destination ) = 0;

    virtual bool GetResourceAccess( const CUri& uri               ,
                                    IOAccessPtr& accessToResource ,
                                    TURI_RESOURCEACCESS_MODE mode ) = 0;

    virtual bool GetResourceMetaData( const CUri& uri             ,
                                      CResourceMetaData& metaData ) = 0;

    virtual bool UpdateResource( const CUri& uri           ,
                                 CDynamicBuffer& srcBuffer );

    virtual bool UpdateResource( const CUri& uri ,
                                 CIOAccess& src  ) = 0;

    virtual bool UpdatePartialResource( const CUri& uri           ,
                                        UInt64 byteOffset         ,
                                        Int64 bytesToWrite        ,
                                        CDynamicBuffer& srcBuffer );

    virtual bool UpdatePartialResource( const CUri& uri    ,
                                        UInt64 byteOffset  ,
                                        Int64 bytesToWrite ,
                                        CIOAccess& src     ) = 0;

    virtual bool UpdatePartialResource( const CUri& uri           ,
                                        CDynamicBuffer& srcBuffer );

    virtual bool UpdatePartialResource( const CUri& uri    ,
                                        CIOAccess& src     ) = 0;

    virtual bool GetSupportedOperations( const CUri& uri                     ,
                                         CUriResourceAccessorOperations& ops ) = 0;

    virtual bool DeleteResource( const CUri& uri ) = 0;

    virtual bool IsACollectionResource( const CUri& uri ) const = 0;

    virtual bool ResolveResourcesInCollection( const CUri& uri        ,
                                               UriVector& resources   ,
                                               bool recursive         ,
                                               bool addCollectionUris ) = 0;

    static const char* ResourceAccessModeStr( TURI_RESOURCEACCESS_MODE mode );

    static bool IsReadOnlyResourceAccessMode( TURI_RESOURCEACCESS_MODE mode );
    static bool IsOverwriteResourceAccessMode( TURI_RESOURCEACCESS_MODE mode );

    CUriResourceAccessor( void ); /**< Doesn't do anything special atm. */
    CUriResourceAccessor( const CUriResourceAccessor& src ); /**< Doesn't do anything special atm. */
    virtual ~CUriResourceAccessor(); /**< Doesn't do anything special atm. */
    CUriResourceAccessor& operator=( const CUriResourceAccessor& src );
};

/*-------------------------------------------------------------------------*/

typedef CTBasicSharedPtr< CUriResourceAccessor, MT::CMutex >        CUriResourceAccessorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CURIRESOURCEACCESSOR_H ? */
