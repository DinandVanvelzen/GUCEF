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

#ifndef GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H
#define GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_CUriResourceAccessor.h"

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
 *  Class for direct representation of resource data encoded as a URI
 *  This class is threadsafe in the sense that it is stateless
 */
class GUCEF_CORE_PUBLIC_CPP CDataUriResourceAccessor : public CUriResourceAccessor
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

    enum EDataUriEncoding
    {
        DATAURI_ENCODING_BASE64,
        DATAURI_ENCODING_PLAIN,
        DATAURI_ENCODING_BASE16,  /**< base16 aka hex is non-standard but useful and we support it */
        
    };

    enum EDataUriPlainEncoding
    {
        DATAURI_PLAIN_ENCODING_NOT_APPLICABLE = 0,

        DATAURI_PLAIN_ENCODING_ASCII,
        DATAURI_PLAIN_ENCODING_UTF8,
        DATAURI_PLAIN_ENCODING_UTF16,
        DATAURI_PLAIN_ENCODING_UTF32
    };
    
    static bool CreateDataUri( CUri& uri                           ,
                               const CString& mimeType             ,
                               const CDynamicBuffer& payload       ,
                               EDataUriEncoding payloadEncoding    ,
                               EDataUriPlainEncoding plainEncoding ,
                               bool payloadIsAlreadyEncoded        );    

    static bool CreateDataUriFromText( CUri& uri                               ,
                                       const CString& mimeType                 ,
                                       const CUtf8String& textPayload          ,
                                       EDataUriEncoding payloadEncodingToApply );
    
    static bool CreateDataUriFromText( CUri& uri                               ,
                                       const CString& mimeType                 ,
                                       const CAsciiString& textPayload         ,
                                       EDataUriEncoding payloadEncodingToApply );

    static bool CreateDataUriFromBase64String( CUri& uri                    ,
                                               const CString& mimeType      ,
                                               const CString& base64Payload );
    
    static bool CreateDataUriFromBase16String( CUri& uri                    ,
                                               const CString& mimeType      ,
                                               const CString& base16Payload );

    static bool ParseDataUri( const CUri& uri                      ,
                              bool decodePayload                   ,
                              CString& mimeType                    ,
                              CDynamicBuffer& payload              ,
                              EDataUriEncoding& payloadEncoding    ,
                              EDataUriPlainEncoding& plainEncoding );
    
    
    CDataUriResourceAccessor( void ); 
    CDataUriResourceAccessor( const CDataUriResourceAccessor& src );
    virtual ~CDataUriResourceAccessor() GUCEF_VIRTUAL_OVERRIDE; 
    CDataUriResourceAccessor& operator=( const CDataUriResourceAccessor& src );
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CDataUriResourceAccessor, MT::CMutex > CDataUriResourceAccessorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H ? */
