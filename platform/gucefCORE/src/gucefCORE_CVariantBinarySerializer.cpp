/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

/**
 *  Returns a pointer to the bytes that make up the variant's payload
 *  For memory address types AsVoidPtr() returns the address held by the variant, not the storage of said address.
 *  Since the payload of such types is the address itself we need its storage, which is also where deserialization puts it
 */
static const void*
GetPayloadBytes( const CVariant& var )
{GUCEF_TRACE;

    switch ( var.GetTypeId() )
    {
        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS:
            return var.CStyleAccess()->union_data.bsob_data;
        default:
            return var.AsVoidPtr();
    }
}

/*-------------------------------------------------------------------------*/

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
                    ToString( (UInt32) sizeof(payloadSize) ) + " bytes expected, " + ToString( (UInt32) sizeof(nrOfBytesWritten) ) + " bytes written" );
                return false;
            }
        }

        nrOfBytesWritten = target.CopyFrom( currentTargetOffset, payloadSize, GetPayloadBytes( var ) );
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

        if ( 1 != access.Write( GetPayloadBytes( var ), payloadSize, 1 ) )
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
        // the IO access write mimics fwrite: it returns the number of elements written, here 1 element of payloadSize bytes
        UInt32 payloadSize = var.ByteSize();
        if ( 0 == payloadSize )
            return true;
        UInt32 nrOfBytesWritten = 1 == access.Write( GetPayloadBytes( var ), payloadSize, 1 ) ? payloadSize : 0;

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

        UInt8 typeId = source.ReadValue< UInt8 >( currentSourceOffset );
        currentSourceOffset += sizeof( typeId );
        bytesRead += sizeof( typeId );

        // early exit for unknown type, think of this as a placeholder for a nil value
        if ( GUCEF_DATATYPE_UNKNOWN == typeId )
            return true;

        if ( CVariant::UsesDynamicMemory( typeId ) )
        {
            UInt32 payloadSize = source.ReadValue< UInt32 >( currentSourceOffset );
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
            UInt32 payloadSize = GucefByteSizeOfFixedSizeType( typeId );
            UInt32 availableBytesLeft = bufferDataSize - currentSourceOffset;
            if ( payloadSize > availableBytesLeft )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:Deserialize: Truncated payload of " << CVariant::TypeNameForTypeId( typeId ) <<
                    ", " << payloadSize << " bytes needed. Available data= " << availableBytesLeft );
                return false;
            }

            bool result = var.Set( source.GetConstBufferPtr( currentSourceOffset ), payloadSize, typeId, linkWherePossible );
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

        UInt32 bufferDataSize = source.GetDataSize();
        if ( currentSourceOffset > bufferDataSize || dataSize > bufferDataSize - currentSourceOffset )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "VariantBinarySerializer:DeserializeRaw: " << dataSize << " bytes of " << CVariant::TypeNameForTypeId( varType ) <<
                " needed at offset " << currentSourceOffset << " but the available data is " << bufferDataSize << " bytes" );
            return false;
        }

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
