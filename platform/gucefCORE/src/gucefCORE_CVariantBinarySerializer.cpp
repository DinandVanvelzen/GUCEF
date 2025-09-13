/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#include "gucefCORE_CVariantBinarySerializer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CVariantBinarySerializer::Serialize( const CVariant& var, UInt32 currentTargetOffset, CDynamicBuffer& target, UInt32& bytesWritten )
{GUCEF_TRACE;
    
    try
    {
        UInt32 nrOfBytesWritten = 0;
    
        UInt8 typeId = var.GetTypeId();
        nrOfBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(typeId), &typeId );
        currentTargetOffset += nrOfBytesWritten;
        bytesWritten += nrOfBytesWritten;

        // early exit for unknown type, think of this as a placeholder for a nil value
        if ( GUCEF_DATATYPE_UNKNOWN == typeId )
            return true;

        // if a variable length is possible we need to prefix the size
        UInt32 payloadSize = var.ByteSize();
        if ( var.UsesDynamicMemory() )
        {        
            nrOfBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(payloadSize), &payloadSize );
            currentTargetOffset += nrOfBytesWritten;
            bytesWritten += nrOfBytesWritten;

            if ( nrOfBytesWritten != sizeof(payloadSize) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:Serialize: payloadSize (" + ToString( payloadSize ) + ") was not written correctly. " + 
                    ToString( (UInt32) sizeof(payloadSize) ) + " bytes expected, " + ToString( sizeof(nrOfBytesWritten) ) + " bytes written" );
                return false;
            }
        }

        nrOfBytesWritten = target.CopyFrom( currentTargetOffset, payloadSize, var.AsVoidPtr() );
        currentTargetOffset += nrOfBytesWritten;
        bytesWritten += nrOfBytesWritten;

        if ( nrOfBytesWritten != payloadSize )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:Serialize: payload of payloadSize " + ToString( payloadSize ) + " bytes was not written correctly. " + 
                ToString( (UInt32) sizeof(nrOfBytesWritten) ) + " bytes were written" );
            return false;
        }

        return true;
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "VariantBinarySerializer:Serialize: caught exception: " ) + e.what()  );
        return false;
    } 
}

/*-------------------------------------------------------------------------*/

bool 
CVariantBinarySerializer::Serialize( const CVariant& var, CIOAccess& access, UInt32& bytesWritten )
{GUCEF_TRACE;
    
    try
    {
        UInt8 typeId = var.GetTypeId();
        if ( !access.WriteValue( typeId ) )
            return false;
        bytesWritten += sizeof( typeId );

        // early exit for unknown type, think of this as a placeholder for a nil value
        if ( GUCEF_DATATYPE_UNKNOWN == typeId )
            return true;

        // if a variable length is possible we need to prefix the size
        UInt32 payloadSize = var.ByteSize();
        if ( var.UsesDynamicMemory() )
        {        
            if ( !access.WriteValue( payloadSize ) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:Serialize: payloadSize (" + ToString( payloadSize ) + ") was not written correctly" );
                return false;
            }
            bytesWritten += sizeof( payloadSize );
        }

        if ( 1 != access.Write( var.AsVoidPtr(), payloadSize, 1 ) )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:Serialize: payload of payloadSize " + ToString( payloadSize ) + " bytes was not written correctly" );
            return false;
        }
        bytesWritten += payloadSize;
        return true;
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "VariantBinarySerializer:Serialize: caught exception: " ) + e.what() );
        return false;
    } 
}

/*-------------------------------------------------------------------------*/

bool 
CVariantBinarySerializer::SerializeRaw( const CVariant& var, UInt32 currentTargetOffset, CDynamicBuffer& target, UInt32& bytesWritten )
{GUCEF_TRACE;

    try
    {
        UInt32 payloadSize = var.ByteSize();
        UInt32 nrOfBytesWritten = target.CopyFrom( var, currentTargetOffset );

        bytesWritten += nrOfBytesWritten;

        if ( nrOfBytesWritten != payloadSize )
        {
            GUCEF_WARNING_LOG(LOGLEVEL_NORMAL, "VariantBinarySerializer:SerializeRaw: payload of payloadSize " + ToString( payloadSize ) + " bytes was not written correctly. " +
                ToString( nrOfBytesWritten ) + " bytes were written" );
            return false;
        }

        return true;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "VariantBinarySerializer:SerializeRaw: caught exception: " ) + e.what() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CVariantBinarySerializer::SerializeRaw( const CVariant& var, CIOAccess& access, UInt32& bytesWritten )
{GUCEF_TRACE;

    try
    {
        UInt32 payloadSize = var.ByteSize();
        UInt32 nrOfBytesWritten = access.Write( var, false );

        bytesWritten += nrOfBytesWritten;

        if ( nrOfBytesWritten != payloadSize )
        {
            GUCEF_WARNING_LOG(LOGLEVEL_NORMAL, "VariantBinarySerializer:SerializeRaw: payload of payloadSize " + ToString( payloadSize ) + " bytes was not written correctly. " +
                ToString( nrOfBytesWritten ) + " bytes were written" );
            return false;
        }

        return true;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "VariantBinarySerializer:SerializeRaw: caught exception: " ) + e.what() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CVariantBinarySerializer::Deserialize( CVariant& var                , 
                                       UInt32 currentSourceOffset   , 
                                       const CDynamicBuffer& source , 
                                       bool linkWherePossible       , 
                                       UInt32& bytesRead            )
{GUCEF_TRACE;

    try
    {
        var.Clear();

        UInt32 bufferDataSize = source.GetDataSize();
        if ( currentSourceOffset+1 > bufferDataSize )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:Deserialize: Invalid starting offset given of " + 
                ToString( currentSourceOffset ) + " bytes. Available data= " + ToString( bufferDataSize ) );
            return false;
        }

        UInt8 typeId = source.AsConstType< UInt8 >( currentSourceOffset );
        currentSourceOffset += sizeof( typeId );
        bytesRead += sizeof( typeId );

        // early exit for unknown type, think of this as a placeholder for a nil value
        if ( GUCEF_DATATYPE_UNKNOWN == typeId )
            return true;

        if ( CVariant::UsesDynamicMemory( typeId ) )
        {
            UInt32 payloadSize = source.AsConstType< UInt32 >( currentSourceOffset );
            currentSourceOffset += sizeof( payloadSize );
            bytesRead += sizeof( payloadSize );

            UInt32 availableBytesLeft = bufferDataSize - currentSourceOffset;
            if ( payloadSize > availableBytesLeft )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:Deserialize: Invalid payloadSize of " + 
                    ToString( payloadSize ) + " bytes. Available data= " + ToString( availableBytesLeft ) );
                return false;
            }

            if ( var.Set( source.GetConstBufferPtr( currentSourceOffset ), payloadSize, typeId, linkWherePossible ) )
            {
                bytesRead += payloadSize;
                return true;
            }
            else
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:Deserialize: Failed to set payloadSize " + 
                    ToString( payloadSize ) + " bytes on variant as " + CVariant::TypeNameForTypeId( typeId ) );
                bytesRead += var.ByteSize();
                return false;
            }
        }
        else
        {
            bool result = var.Set( source.GetConstBufferPtr( currentSourceOffset ), GucefByteSizeOfFixedSizeType( typeId ), typeId, linkWherePossible );
            bytesRead += var.ByteSize();
            return result;            
        }
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "VariantBinarySerializer:Deserialize: caught exception: " ) + e.what()  );
        return false;
    } 
}

/*-------------------------------------------------------------------------*/

bool 
CVariantBinarySerializer::DeserializeRaw( CVariant& var                , 
                                          UInt8 varType                , 
                                          UInt32 currentSourceOffset   , 
                                          const CDynamicBuffer& source , 
                                          bool linkWherePossible       , 
                                          UInt32& bytesRead            ,
                                          UInt32 dataSizeIfDynamic     )
{GUCEF_TRACE;

    try
    {
        var.Clear();

        // early exit for unknown type, think of this as a placeholder for a nil value
        if ( GUCEF_DATATYPE_UNKNOWN == varType )
            return true;

        UInt32 dataSize = dataSizeIfDynamic;
        if ( !CVariant::UsesDynamicMemory( varType ) )
            dataSize = GucefByteSizeOfFixedSizeType( varType );
        
        if ( var.Set( source.GetConstBufferPtr( currentSourceOffset ), dataSize, varType, linkWherePossible ) )
        {
            bytesRead += dataSize;
            return true;
        }
        else
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:DeserializeRaw: Failed to set payloadSize " + 
                ToString( dataSize ) + " bytes on variant as " + CVariant::TypeNameForTypeId( varType ) );
            bytesRead += var.ByteSize();
            return false;
        }
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "VariantBinarySerializer:DeserializeRaw: caught exception: " ) + e.what()  );
        return false;
    } 
}

/*-------------------------------------------------------------------------*/

bool 
CVariantBinarySerializer::DeserializeRaw( CVariant& var            , 
                                          UInt8 varType            , 
                                          CIOAccess& source        ,  
                                          UInt32& bytesRead        ,
                                          UInt32 dataSizeIfDynamic )
{GUCEF_TRACE;

    try
    {
        var.Clear();

        // early exit for unknown type, think of this as a placeholder for a nil value
        if ( GUCEF_DATATYPE_UNKNOWN == varType )
            return true;

        UInt32 dataSize = dataSizeIfDynamic;
        if ( CVariant::UsesDynamicMemory( varType ) )
        {
            void* varBuffer = var.PreAllocateDynamicMemory( dataSize, varType );
            if ( GUCEF_NULL != varBuffer )
            {
                UInt32 srcBytesRead = source.Read( varBuffer, 1, dataSize );
                bytesRead += srcBytesRead; 
                if ( srcBytesRead == dataSize )
                {
                    return true;
                }
                else
                {
                    GUCEF_WARNING_LOG(LOGLEVEL_NORMAL, "VariantBinarySerializer:DeserializeRaw: Failed to read " +
                        ToString( dataSize ) + " bytes for dynamic data type " + CVariant::TypeNameForTypeId( varType ) +
                        ". Only " + ToString( srcBytesRead ) + " bytes were read" );
                    return false;
                }
            }
            else
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:DeserializeRaw: Failed to pre-allocate " +
                    ToString( dataSize ) + " bytes for dynamic data type " + CVariant::TypeNameForTypeId( varType ) );
                return false;
            }
        }
        else
        {
            // only use the type info for the data size
            dataSize = GucefByteSizeOfFixedSizeType( varType );
            TVariantData varData;
            varData.containedType = varType;
            UInt32 srcBytesRead = ( source.Read( varData.union_data.bsob_data, dataSize, 1 ) * dataSize );
            bytesRead += srcBytesRead; 
            if ( srcBytesRead == dataSize )
            {
                var = varData;
                return true;
            }
            else
            {
                GUCEF_WARNING_LOG(LOGLEVEL_NORMAL, "VariantBinarySerializer:DeserializeRaw: Failed to read " +
                    ToString( dataSize ) + " bytes for dynamic data type " + CVariant::TypeNameForTypeId( varType ) +
                    ". Only " + ToString( srcBytesRead ) + " bytes were read" );
                return false;
            }   
        }
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "VariantBinarySerializer:DeserializeRaw: caught exception: " ) + e.what()  );
        return false;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
