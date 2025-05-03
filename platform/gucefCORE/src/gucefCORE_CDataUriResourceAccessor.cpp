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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CDataUriResourceAccessor.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CDataUriResourceAccessor::SchemeName = "data";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataUriResourceAccessor::CDataUriResourceAccessor( void )
    : CUriResourceAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataUriResourceAccessor::CDataUriResourceAccessor( const CDataUriResourceAccessor& src )
    : CUriResourceAccessor( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataUriResourceAccessor::~CDataUriResourceAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataUriResourceAccessor& 
CDataUriResourceAccessor::operator=( const CDataUriResourceAccessor& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::CreateResource( const CUri& uri ,
                                          CIOAccess& src  )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:CreateResource: Not supported" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::GetResource( const CUri& uri        ,
                                       CIOAccess& destination )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceAccess: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString mimeType;    
    CDynamicBuffer payload;
    EDataUriEncoding payloadEncoding = DATAURI_ENCODING_BASE64;
    EDataUriPlainEncoding plainEncoding = DATAURI_PLAIN_ENCODING_NOT_APPLICABLE;    
    if ( ParseDataUri( uri             ,
                       true            ,
                       mimeType        ,
                       payload         ,
                       payloadEncoding ,
                       plainEncoding   ) )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceAccess: Obtained access to data in data uri: mimeType=\""
            + mimeType + "\" decoded payload size=" + ToString( payload.GetDataSize() ) );

        UInt32 bytesWritten = destination.Write( payload );
        if GUCEF_PREDICT_FALSE( bytesWritten != payload.GetDataSize() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceAccess: Failed to write all data to destination media" );
            return false;
        }
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceAccess: Failed to parse data uri: " + ToString( uri ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CDataUriResourceAccessor::GetResourceAccess( const CUri& uri               ,
                                             IOAccessPtr& accessToResource ,
                                             TURI_RESOURCEACCESS_MODE mode )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceAccess: Unsupported scheme " + uri.GetScheme() );
        return false;
    }
    if GUCEF_PREDICT_FALSE( mode != URI_RESOURCEACCESS_MODE_READ )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceAccess: Unsupported access mode " + ToString( mode ) );
        return false;
    }
    DynamicBufferAccessPtr bufferAccess = CDynamicBufferAccess::CreateSharedObj();
    if GUCEF_PREDICT_FALSE( bufferAccess.IsNULL() )
    {
        return false;
    }
    
    // Create a dynamic buffer to hold the payload and give lifetime ownership to the buffer access object
    CDynamicBuffer* payload = GUCEF_NEW CDynamicBuffer();    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == payload )
        return false;
    bufferAccess->LinkTo( payload, true );
    
    CString mimeType;    
    EDataUriEncoding payloadEncoding = DATAURI_ENCODING_BASE64;
    EDataUriPlainEncoding plainEncoding = DATAURI_PLAIN_ENCODING_NOT_APPLICABLE;    
    if ( ParseDataUri( uri             ,
                       true            ,
                       mimeType        ,
                       *payload        ,
                       payloadEncoding ,
                       plainEncoding   ) )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceAccess: Obtained access to data in data uri: mimeType=\""
            + mimeType + "\" decoded payload size=" + ToString( payload->GetDataSize() ) );
        
        accessToResource = bufferAccess;
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceAccess: Failed to parse data uri: " + ToString( uri ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::GetPartialResource( const CUri& uri        ,
                                              UInt64 byteOffset      ,
                                              Int64 bytesToGet       ,
                                              CIOAccess& destination )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetPartialResource: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    CString mimeType;    
    CDynamicBuffer payload;
    EDataUriEncoding payloadEncoding = DATAURI_ENCODING_BASE64;
    EDataUriPlainEncoding plainEncoding = DATAURI_PLAIN_ENCODING_NOT_APPLICABLE;    
    if ( ParseDataUri( uri             ,
                       true            ,
                       mimeType        ,
                       payload         ,
                       payloadEncoding ,
                       plainEncoding   ) )
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetPartialResource: Obtained access to data in data uri: mimeType=\""
            + mimeType + "\" decoded payload size=" + ToString( payload.GetDataSize() ) );

        if ( bytesToGet < 0 && byteOffset < payload.GetDataSize() )
        {
            bytesToGet = payload.GetDataSize() - byteOffset;
        }        
        if ( byteOffset + bytesToGet <= payload.GetDataSize() )
        {        
            CDynamicBuffer subSection;
            subSection.LinkTo( payload.GetConstBufferPtr( static_cast< UInt32 >( byteOffset ) ), static_cast< UInt32 >( bytesToGet ) );

            UInt32 bytesWritten = destination.Write( subSection );
            if GUCEF_PREDICT_FALSE( bytesWritten != subSection.GetDataSize() )
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetPartialResource: Failed to write all data to destination media" );
                return false;
            }
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetPartialResource: Requested byte range exceeds payload size " + 
                ToString( byteOffset ) + " + " + ToString( bytesToGet ) + " for buffer of " + ToString( payload.GetDataSize() ) + " bytes" );
            return false;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetPartialResource: Failed to parse data uri: " + ToString( uri ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::GetResourceMetaData( const CUri& uri             ,
                                               CResourceMetaData& metaData )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceMetaData: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    metaData.Clear();
    metaData.isReadOnly = true;
    metaData.hasIsReadOnly = true;
 
    CDynamicBuffer payload;
    EDataUriEncoding payloadEncoding = DATAURI_ENCODING_BASE64;
    EDataUriPlainEncoding plainEncoding = DATAURI_PLAIN_ENCODING_NOT_APPLICABLE;    
    if ( ParseDataUri( uri               ,
                       false             ,
                       metaData.mimeType ,
                       payload           ,
                       payloadEncoding   ,
                       plainEncoding     ) )
    {
        metaData.resourceSizeInBytes = payload.GetDataSize();
        metaData.hasResourceSizeInBytes = true;
        metaData.isEncoded = true;
        metaData.hasIsEncoded = true;
        metaData.hasMimeType = !metaData.mimeType.IsNULLOrEmpty();
        metaData.resourceExists = payload.GetDataSize() > 0;

        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetResourceMetaData: Failed to parse data uri: " + ToString( uri ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::UpdateResource( const CUri& uri ,
                                                CIOAccess& src  )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:UpdateResource: Not supported" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::UpdatePartialResource( const CUri& uri    ,
                                                       UInt64 byteOffset  ,
                                                       Int64 bytesToWrite ,
                                                       CIOAccess& src     )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:UpdatePartialResource: Not supported" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::UpdatePartialResource( const CUri& uri    ,
                                                       CIOAccess& src     )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:UpdatePartialResource: Not supported" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::GetSupportedOperations( const CUri& uri                     ,
                                                  CUriResourceAccessorOperations& ops )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:GetSupportedOperations: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    ops.createResource = false;
    ops.deleteResource = false;
    ops.getPartialResource = false;
    ops.getResource = true;
    ops.getResourceMetaData = true;
    ops.updatePartialResource = false;
    ops.updateResource = false;
    ops.collectionDetermnination = false;
    ops.collectionResolution = false;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::DeleteResource( const CUri& uri )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:DeleteResource: Not supported" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::IsACollectionResource( const CUri& uri ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::ResolveResourcesInCollection( const CUri& uri        ,
                                                        UriVector& resources   ,
                                                        bool recursive         ,
                                                        bool addCollectionUris )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataUriResourceAccessor::CreateDataUri( CUri& uri                           ,
                                         const CString& mimeType             ,
                                         const CDynamicBuffer& payload       ,
                                         EDataUriEncoding payloadEncoding    ,
                                         EDataUriPlainEncoding plainEncoding ,
                                         bool payloadIsAlreadyEncoded        )
{GUCEF_TRACE;

    uri.Clear();
    
    // Check prerequisites
    if GUCEF_PREDICT_FALSE( mimeType.IsNULLOrEmpty()   || 
                            0 == payload.GetDataSize()  )
        return false;
    
    CString payloadAsStr;
    CString payloadEncodingPrefix;
    
    switch ( payloadEncoding )
    {
        case DATAURI_ENCODING_BASE64:
        {
            payloadEncodingPrefix = "base64";
            if ( payloadIsAlreadyEncoded )
                payloadAsStr = ToString( payload );    
            else
                payloadAsStr = Base64Encode( payload.GetConstBufferPtr(), payload.GetDataSize() );
            break;
        }
        case DATAURI_ENCODING_BASE16:
        {
            payloadEncodingPrefix = "base16";
            if ( payloadIsAlreadyEncoded )
                payloadAsStr = ToString( payload );    
            else
                payloadAsStr = Base16Encode( payload.GetConstBufferPtr(), payload.GetDataSize() );
            break;
        }
        default:
        {
            break;
        }
    }
    
    CString charsetStr;
    switch ( plainEncoding )
    {
        case DATAURI_PLAIN_ENCODING_ASCII:
        {
            charsetStr = "us-ascii";
            break;
        }
        case DATAURI_PLAIN_ENCODING_UTF8:
        {
            charsetStr = "utf-8";
            break;
        }
        case DATAURI_PLAIN_ENCODING_UTF16:
        {
            charsetStr = "utf-16";
            break;
        }
        case DATAURI_PLAIN_ENCODING_UTF32:
        {
            charsetStr = "utf-32";
            break;
        }
        default:
        {
            break;
        }
    }

    CString uriPath = mimeType + ';';
    if ( !charsetStr.IsNULLOrEmpty() )
        uriPath += "charset=" + charsetStr + ';';
    if ( !payloadEncodingPrefix.IsNULLOrEmpty() )
        uriPath += payloadEncodingPrefix;
    uriPath += ',' + payloadAsStr;

    // Data URIs do not have a host part aka authority
    uri.SetScheme( SchemeName );
    uri.SetPath( uriPath );

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CDataUriResourceAccessor::CreateDataUriFromText( CUri& uri                               ,
                                                 const CString& mimeType                 ,
                                                 const CUtf8String& textPayload          ,
                                                 EDataUriEncoding payloadEncodingToApply )
{GUCEF_TRACE;

    CDynamicBuffer payloadAsBuffer;
    payloadAsBuffer.LinkTo( textPayload, false );
    return CreateDataUri( uri, mimeType, payloadAsBuffer, payloadEncodingToApply, DATAURI_PLAIN_ENCODING_UTF8, false );
}

/*-------------------------------------------------------------------------*/
    
bool 
CDataUriResourceAccessor::CreateDataUriFromText( CUri& uri                               ,
                                                 const CString& mimeType                 ,
                                                 const CAsciiString& textPayload         ,
                                                 EDataUriEncoding payloadEncodingToApply )
{GUCEF_TRACE;

    CDynamicBuffer payloadAsBuffer;
    payloadAsBuffer.LinkTo( textPayload, false );
    return CreateDataUri( uri, mimeType, payloadAsBuffer, payloadEncodingToApply, DATAURI_PLAIN_ENCODING_ASCII, false );
}

/*-------------------------------------------------------------------------*/

bool 
CDataUriResourceAccessor::CreateDataUriFromBase64String( CUri& uri                    ,
                                                         const CString& mimeType      ,
                                                         const CString& base64Payload )
{GUCEF_TRACE;

    CDynamicBuffer payloadAsBuffer;
    payloadAsBuffer.LinkTo( base64Payload, false );
    return CreateDataUri( uri, mimeType, payloadAsBuffer, DATAURI_ENCODING_BASE64, DATAURI_PLAIN_ENCODING_NOT_APPLICABLE, true );
}

/*-------------------------------------------------------------------------*/

bool 
CDataUriResourceAccessor::CreateDataUriFromBase16String( CUri& uri                    ,
                                                         const CString& mimeType      ,
                                                         const CString& base16Payload )
{GUCEF_TRACE;

    CDynamicBuffer payloadAsBuffer;
    payloadAsBuffer.LinkTo( base16Payload, false );
    return CreateDataUri( uri, mimeType, payloadAsBuffer, DATAURI_ENCODING_BASE16, DATAURI_PLAIN_ENCODING_NOT_APPLICABLE, true );
}

/*-------------------------------------------------------------------------*/

bool 
CDataUriResourceAccessor::ParseDataUri( const CUri& uri                      ,
                                        bool decodePayload                   ,
                                        CString& mimeType                    ,
                                        CDynamicBuffer& payload              ,
                                        EDataUriEncoding& payloadEncoding    ,
                                        EDataUriPlainEncoding& plainEncoding )
{GUCEF_TRACE;

    mimeType.Clear();
    payload.Clear();
    payloadEncoding = DATAURI_ENCODING_PLAIN;
    plainEncoding = DATAURI_PLAIN_ENCODING_NOT_APPLICABLE;
    
    if ( SchemeName != uri.GetScheme() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:ParseDataUri: Unsupported scheme " + uri.GetScheme() );
        return false;
    }

    const CString& uriPath = uri.GetPath();

    // First mandatory section is the MIME type
    Int32 commaPos = uriPath.HasChar( ',' );
    if ( commaPos < 1 )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:ParseDataUri: No payload found in URI" );
        return false;
    }

    Int32 mimeEndPos = uriPath.HasChar( ';' );
    if ( mimeEndPos < 1 )
    {
        // its allowed to not have optional charset and encoding info
        // however that does mean the payload should be the very next thing
        mimeEndPos = commaPos;
    }
    mimeType = uriPath.SubstrToIndex( static_cast< UInt32 >( mimeEndPos ) );

    // Check if there are more properties to parse
    if ( mimeEndPos != commaPos )
    {
        // Parse the properties
        CString propertySegment = uriPath.SubstrFromRange( static_cast<UInt32>( mimeEndPos + 1), static_cast<UInt32>( commaPos ) );
        CString::StringVector properties = propertySegment.ParseElements( ';', false );
        for ( UInt32 i=0; i<properties.size(); ++i )
        {
            CString property = properties[ i ].Lowercase();
            if ( property.StartsWith( "charset=" ) )
            {
                CString charset = property.CutChars( 8, true, 0 );
                if ( charset == "us-ascii" )
                    plainEncoding = DATAURI_PLAIN_ENCODING_ASCII;
                else 
                if ( charset == "utf-8" )
                    plainEncoding = DATAURI_PLAIN_ENCODING_UTF8;
                else 
                if ( charset == "utf-16" )
                    plainEncoding = DATAURI_PLAIN_ENCODING_UTF16;
                else 
                if ( charset == "utf-32" )
                    plainEncoding = DATAURI_PLAIN_ENCODING_UTF32;
            }
            else 
            if ( property == "base64" )
            {
                payloadEncoding = DATAURI_ENCODING_BASE64;
            }
            else 
            if ( property == "base16" )
            {
                payloadEncoding = DATAURI_ENCODING_BASE16;
            }
        }
    }
    
    CString payloadStr = uriPath.SubstrToIndex( static_cast<UInt32>( commaPos + 1 ), false );
    
    if ( decodePayload && payloadEncoding != DATAURI_ENCODING_PLAIN )
    {        
        switch ( payloadEncoding )
        {
            case DATAURI_ENCODING_BASE64:
            {
                if ( 0 == payload.CopyAndDecodeBase64From( payloadStr, 0 ) )
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:ParseDataUri: Failed to decode base64 payload" );
                    return false;
                }
                break;
            }
            case DATAURI_ENCODING_BASE16:
            {
                if ( 0 == payload.CopyAndDecodeBase16From( payloadStr, 0 ) )
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DataUriResourceAccessor:ParseDataUri: Failed to decode base16 payload" );
                    return false;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        payload = payloadStr;
    }
    
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
