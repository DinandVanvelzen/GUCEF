/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#include "gucefCORE_CVariantBinarySerializer.h"
#define GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CVARIANTBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H
#include "gucefPUBSUB_CPubSubClientFeatures.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H ? */

#include "gucefPUBSUB_CPubSubMsgBinarySerializer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubMsgBinarySerializerOptions::CPubSubMsgBinarySerializerOptions( void )
    : CORE::CIConfigurable()
    , msgIdIncluded( true )
    , msgIndexIncluded( true )
    , msgDateTimeIncluded( true )
    , msgDateTimeAsMsSinceUnixEpochInUtc( true )
    , msgPrimaryPayloadIncluded( true )
    , msgKeyValuePairsIncluded( true )
    , msgMetaDataKeyValuePairsIncluded( true )
    , topicNameIncluded( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubMsgBinarySerializerOptions::~CPubSubMsgBinarySerializerOptions()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void 
CPubSubMsgBinarySerializerOptions::Clear( void )
{GUCEF_TRACE;

    msgIdIncluded = false;
    msgIndexIncluded = false; 
    msgDateTimeIncluded = false; 
    msgDateTimeAsMsSinceUnixEpochInUtc = false; 
    msgPrimaryPayloadIncluded = false;
    msgKeyValuePairsIncluded = false;
    msgMetaDataKeyValuePairsIncluded = false; 
    topicNameIncluded = false;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgBinarySerializerOptions::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool success = true;
    
    success = config.SetAttribute( "msgDateTimeIncluded", msgDateTimeIncluded ) && success;
    success = config.SetAttribute( "msgDateTimeAsMsSinceUnixEpochInUtc", msgDateTimeAsMsSinceUnixEpochInUtc ) && success;
    success = config.SetAttribute( "msgIdIncluded", msgIdIncluded ) && success;
    success = config.SetAttribute( "msgIndexIncluded", msgIndexIncluded ) && success;
    success = config.SetAttribute( "msgPrimaryPayloadIncluded", msgPrimaryPayloadIncluded ) && success;
    success = config.SetAttribute( "msgKeyValuePairsIncluded", msgKeyValuePairsIncluded ) && success;
    success = config.SetAttribute( "msgMetaDataKeyValuePairsIncluded", msgMetaDataKeyValuePairsIncluded ) && success;
    success = config.SetAttribute( "topicNameIncluded", topicNameIncluded ) && success;
    
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgBinarySerializerOptions::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    msgDateTimeIncluded = config.GetAttributeValueOrChildValueByName( "msgDateTimeIncluded" ).AsBool( msgDateTimeIncluded, true );
    msgDateTimeAsMsSinceUnixEpochInUtc = config.GetAttributeValueOrChildValueByName( "msgDateTimeAsMsSinceUnixEpochInUtc" ).AsBool( msgDateTimeAsMsSinceUnixEpochInUtc, true );
    msgIdIncluded = config.GetAttributeValueOrChildValueByName( "msgIdIncluded" ).AsBool( msgIdIncluded, true );
    msgIndexIncluded = config.GetAttributeValueOrChildValueByName( "msgIndexIncluded" ).AsBool( msgIndexIncluded, true );
    msgPrimaryPayloadIncluded = config.GetAttributeValueOrChildValueByName( "msgPrimaryPayloadIncluded" ).AsBool( msgPrimaryPayloadIncluded, true );
    msgKeyValuePairsIncluded = config.GetAttributeValueOrChildValueByName( "msgKeyValuePairsIncluded" ).AsBool( msgKeyValuePairsIncluded, true );
    msgMetaDataKeyValuePairsIncluded = config.GetAttributeValueOrChildValueByName( "msgMetaDataKeyValuePairsIncluded" ).AsBool( msgMetaDataKeyValuePairsIncluded, true );
    topicNameIncluded = config.GetAttributeValueOrChildValueByName( "topicNameIncluded" ).AsBool( topicNameIncluded, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString& 
CPubSubMsgBinarySerializerOptions::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::PUBSUB::CPubSubMsgBinarySerializerOptions";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt32 
CPubSubMsgBinarySerializerOptions::ToOptionsBitMask( void ) const
{GUCEF_TRACE;

    UInt32 bitMask = 0;
    msgDateTimeIncluded ? GUCEF_SETBITXON( bitMask, 1 ) : GUCEF_SETBITXOFF( bitMask, 1 );
    msgDateTimeAsMsSinceUnixEpochInUtc ? GUCEF_SETBITXON( bitMask, 2 ) : GUCEF_SETBITXOFF( bitMask, 2 );
    msgIdIncluded ? GUCEF_SETBITXON( bitMask, 3 ) : GUCEF_SETBITXOFF( bitMask, 3 );
    msgIndexIncluded ? GUCEF_SETBITXON( bitMask, 4 ) : GUCEF_SETBITXOFF( bitMask, 4 );
    msgPrimaryPayloadIncluded ? GUCEF_SETBITXON( bitMask, 5 ) : GUCEF_SETBITXOFF( bitMask, 5 );
    msgKeyValuePairsIncluded ? GUCEF_SETBITXON( bitMask, 6 ) : GUCEF_SETBITXOFF( bitMask, 6 );
    msgMetaDataKeyValuePairsIncluded ? GUCEF_SETBITXON( bitMask, 7 ) : GUCEF_SETBITXOFF( bitMask, 7 );
    topicNameIncluded ? GUCEF_SETBITXON( bitMask, 8 ) : GUCEF_SETBITXOFF( bitMask, 8 );
    return bitMask;
}

/*-------------------------------------------------------------------------*/

void 
CPubSubMsgBinarySerializerOptions::FromOptionsBitMask( UInt32 bitMask )
{GUCEF_TRACE;

    msgDateTimeIncluded = ( 0 != GUCEF_GETBITX( bitMask, 1 ) );
    msgDateTimeAsMsSinceUnixEpochInUtc = ( 0 != GUCEF_GETBITX( bitMask, 2 ) );
    msgIdIncluded = ( 0 != GUCEF_GETBITX( bitMask, 3 ) );
    msgIndexIncluded = ( 0 != GUCEF_GETBITX( bitMask, 4 ) );
    msgPrimaryPayloadIncluded = ( 0 != GUCEF_GETBITX( bitMask, 5 ) );
    msgKeyValuePairsIncluded = ( 0 != GUCEF_GETBITX( bitMask, 6 ) );
    msgMetaDataKeyValuePairsIncluded = ( 0 != GUCEF_GETBITX( bitMask, 7 ) );
    topicNameIncluded = ( 0 != GUCEF_GETBITX( bitMask, 8 ) );
}

/*-------------------------------------------------------------------------*/

void 
CPubSubMsgBinarySerializerOptions::FromPubSubClientFeatures( const CPubSubClientFeatures& features )
{GUCEF_TRACE;

    msgDateTimeIncluded = features.supportsMsgDateTimeBasedBookmark;
    msgDateTimeAsMsSinceUnixEpochInUtc = true;
    msgIdIncluded = features.supportsPerMsgIds;
    msgIndexIncluded = features.supportsMsgIndex;
    msgPrimaryPayloadIncluded = features.supportsPrimaryPayloadPerMsg;
    msgKeyValuePairsIncluded = features.supportsKeyValueSetPerMsg;
    msgMetaDataKeyValuePairsIncluded = features.supportsMetaDataKeyValueSetPerMsg;
    topicNameIncluded = false; // just use the default setting for this which is 'false'
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgBinarySerializer::SerializeKvPairs( const CIPubSubMsg::TKeyValuePairs& kvPairs ,
                                              UInt32 currentTargetOffset                 ,
                                              CORE::CDynamicBuffer& target               , 
                                              UInt32& bytesWritten                       )
{GUCEF_TRACE;

    UInt32 nrOfKvPairs = (UInt32) kvPairs.size();
    UInt32 lastBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(nrOfKvPairs), &nrOfKvPairs );
    currentTargetOffset += lastBytesWritten;
    bytesWritten += lastBytesWritten;

    CIPubSubMsg::TKeyValuePairs::const_iterator i = kvPairs.begin();
    while ( i != kvPairs.end() )
    {                
        const CORE::CVariant& key = (*i).first;               
        const CORE::CVariant& value = (*i).second;

        UInt32 varBinSize = 0;
        if ( !CORE::CVariantBinarySerializer::Serialize( key, currentTargetOffset, target, varBinSize ) )
            return false;
        currentTargetOffset += varBinSize;
        bytesWritten += varBinSize;
                
        varBinSize = 0;
        if ( !CORE::CVariantBinarySerializer::Serialize( value, currentTargetOffset, target, varBinSize ) )
            return false;
        currentTargetOffset += varBinSize;
        bytesWritten += varBinSize;

        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgBinarySerializer::SerializeKvPairs( const CIPubSubMsg::TKeyValuePairs& kvPairs ,
                                              CORE::CIOAccess& access                    , 
                                              UInt32& bytesWritten                       )
{GUCEF_TRACE;

    UInt32 nrOfKvPairs = (UInt32) kvPairs.size();
    if ( !access.WriteValue( nrOfKvPairs ) )
        return false;

    CIPubSubMsg::TKeyValuePairs::const_iterator i = kvPairs.begin();
    while ( i != kvPairs.end() )
    {                
        const CORE::CVariant& key = (*i).first;               
        const CORE::CVariant& value = (*i).second;

        UInt32 varBinSize = 0;
        if ( !CORE::CVariantBinarySerializer::Serialize( key, access, varBinSize ) )
            return false;
        bytesWritten += varBinSize;
                
        varBinSize = 0;
        if ( !CORE::CVariantBinarySerializer::Serialize( value, access, varBinSize ) )
            return false;
        bytesWritten += varBinSize;

        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgBinarySerializer::Serialize( const CPubSubMsgBinarySerializerOptions& options ,
                                       const CIPubSubMsg& msg                           , 
                                       UInt32 currentTargetOffset                       , 
                                       CORE::CDynamicBuffer& target                     , 
                                       UInt32& bytesWritten                             )
{GUCEF_TRACE;
    
    try
    {
        if ( options.msgDateTimeIncluded )
        {
            if ( options.msgDateTimeAsMsSinceUnixEpochInUtc )
            {
                // Write the DateTime of the msg as milliseconds since Unix Epoch in UTC
                // In most applications Unix Epoch is not UTC and leap seconds are not taken into account
                // However since the source if of higher fidelity we can retain accuracy as long as the same method is
                // used to deserialize as well. There is a problem with any DateTime which predates Unix epoch
                // If you have to deal with those use string formatting instead
                UInt64 msSinceUnixEpochInUtc = msg.GetMsgDateTime().ToUnixEpochBasedTicksInMillisecs();
                UInt32 lastBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(msSinceUnixEpochInUtc), &msSinceUnixEpochInUtc );
                currentTargetOffset += lastBytesWritten;
                bytesWritten += lastBytesWritten;
            }
            else
            {
                // Write the DateTime of the msg which as a ISO 8601 spec string
                // Writing it in string form actually takes up less space vs writing all the binary components at full fidelity
                // Using Unix epoch uses less space but we are limited in the time range since historical dates would be a problem 
                Int32 dtStrSize = msg.GetMsgDateTime().ToIso8601DateTimeString( target, currentTargetOffset, false, true );
                if ( 0 >= dtStrSize )
                    return false;
                currentTargetOffset += (UInt32) dtStrSize;
                bytesWritten += (UInt32) dtStrSize;
            }
        }

        if ( options.msgIdIncluded )
        {
            // Write the ID using the variable length variant serializer
            UInt32 varBinSize = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( msg.GetMsgId(), currentTargetOffset, target, varBinSize ) )
                return false;
            currentTargetOffset += varBinSize;
            bytesWritten += varBinSize;
        }

        if ( options.msgIndexIncluded )
        {
            // Write the index using the variable length variant serializer
            UInt32 varBinSize = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( msg.GetMsgIndex(), currentTargetOffset, target, varBinSize ) )
                return false;
            currentTargetOffset += varBinSize;
            bytesWritten += varBinSize;
        }

        if ( options.msgPrimaryPayloadIncluded )
        {
            // Now write the primary payload if any exists
            const CORE::CVariant& primaryPayload = msg.GetPrimaryPayload();
            UInt32 varBinSize = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( primaryPayload, currentTargetOffset, target, varBinSize ) )
                return false;
            currentTargetOffset += varBinSize;
            bytesWritten += varBinSize;
        }

        if ( options.msgKeyValuePairsIncluded )
        {
            UInt32 kvPairsSize = 0;
            const CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetKeyValuePairs();            
            if ( !SerializeKvPairs( kvPairs, currentTargetOffset, target, kvPairsSize ) )
                return false;
            currentTargetOffset += kvPairsSize;
            bytesWritten += kvPairsSize;
        }

        if ( options.msgMetaDataKeyValuePairsIncluded )
        {
            UInt32 kvPairsSize = 0;
            const CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetMetaDataKeyValuePairs();            
            if ( !SerializeKvPairs( kvPairs, currentTargetOffset, target, kvPairsSize ) )
                return false;
            currentTargetOffset += kvPairsSize;
            bytesWritten += kvPairsSize;
        }

        if ( options.topicNameIncluded )
        {
            // We store the topic name as a variant to avoid having to deal with string encoding issues
            CORE::CVariant topicNameVariant;
            topicNameVariant.LinkTo( msg.GetOriginClientTopicName() );

            // Write the topic name using the variable length variant serializer
            UInt32 varBinSize = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( topicNameVariant, currentTargetOffset, target, varBinSize ) )
                return false;
            currentTargetOffset += varBinSize;
            bytesWritten += varBinSize;
        }

        return true;
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, CString( "PubSubMsgBinarySerializer:Serialize: caught exception: " ) + e.what()  );
        return false;
    } 
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubMsgBinarySerializer::Serialize( const CPubSubMsgBinarySerializerOptions& options ,
                                       const CIPubSubMsg& msg                           , 
                                       CORE::CIOAccess& target                          ,
                                       UInt32& bytesWritten                             )
{GUCEF_TRACE;
    
    try
    {
        if ( options.msgDateTimeIncluded )
        {
            if ( options.msgDateTimeAsMsSinceUnixEpochInUtc )
            {
                // Write the DateTime of the msg as milliseconds since Unix Epoch in UTC
                // In most applications Unix Epoch is not UTC and leap seconds are not taken into account
                // However since the source if of higher fidelity we can retain accuracy as long as the same method is
                // used to deserialize as well. There is a problem with any DateTime which predates Unix epoch
                // If you have to deal with those use string formatting instead
                UInt64 msSinceUnixEpochInUtc = msg.GetMsgDateTime().ToUnixEpochBasedTicksInMillisecs();
                if ( !target.WriteValue( msSinceUnixEpochInUtc ) )
                    return false;
                bytesWritten += sizeof( msSinceUnixEpochInUtc );
            }
            else
            {
                // Write the DateTime of the msg which as a ISO 8601 spec string
                // Writing it in string form actually takes up less space vs writing all the binary components at full fidelity
                // Using Unix epoch uses less space but we are limited in the time range since historical dates would be a problem 
                char targetBuffer[ 64 ];
                Int32 dtStrSize = msg.GetMsgDateTime().ToIso8601DateTimeString( targetBuffer, sizeof(targetBuffer), false, true );
                if ( 0 >= dtStrSize )
                    return false;
                if ( 1 != target.Write( targetBuffer, (UInt32) dtStrSize, 1 ) )
                    return false;
                bytesWritten += (UInt32) dtStrSize;
            }
        }

        if ( options.msgIdIncluded )
        {
            // Write the ID using the variable length variant serializer
            UInt32 varBinSize = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( msg.GetMsgId(), target, varBinSize ) )
                return false;
            bytesWritten += varBinSize;
        }

        if ( options.msgIndexIncluded )
        {
            // Write the index using the variable length variant serializer
            UInt32 varBinSize = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( msg.GetMsgIndex(), target, varBinSize ) )
                return false;
            bytesWritten += varBinSize;
        }

        if ( options.msgPrimaryPayloadIncluded )
        {
            // Now write the primary payload if any exists
            const CORE::CVariant& primaryPayload = msg.GetPrimaryPayload();
            UInt32 varBinSize = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( primaryPayload, target, varBinSize ) )
                return false;
            bytesWritten += varBinSize;
        }

        if ( options.msgKeyValuePairsIncluded )
        {
            UInt32 kvPairsSize = 0;
            const CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetKeyValuePairs();            
            if ( !SerializeKvPairs( kvPairs, target, kvPairsSize ) )
                return false;
            bytesWritten += kvPairsSize;
        }

        if ( options.msgMetaDataKeyValuePairsIncluded )
        {
            UInt32 kvPairsSize = 0;
            const CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetMetaDataKeyValuePairs();            
            if ( !SerializeKvPairs( kvPairs, target, kvPairsSize ) )
                return false;
            bytesWritten += kvPairsSize;
        }

        if ( options.topicNameIncluded )
        {
            // We store the topic name as a variant to avoid having to deal with string encoding issues
            CORE::CVariant topicNameVariant;
            topicNameVariant.LinkTo( msg.GetOriginClientTopicName() );

            // Write the topic name using the variable length variant serializer
            UInt32 varBinSize = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( topicNameVariant, target, varBinSize ) )
                return false;
            bytesWritten += varBinSize;
        }

        return true;
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, CString( "PubSubMsgBinarySerializer:Serialize: caught exception: " ) + e.what()  );
        return false;
    } 
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubMsgBinarySerializer::DeserializeKvPairs( bool linkWherePossible               ,
                                                CIPubSubMsg::TKeyValuePairs& kvPairs ,
                                                UInt32 currentSourceOffset           , 
                                                const CORE::CDynamicBuffer& source   , 
                                                UInt32& bytesRead                    )
{GUCEF_TRACE;

    // Now read the key-value pairs if any exists
            
    UInt32 nrOfKvPairs = source.ReadValue< UInt32 >( currentSourceOffset );
    currentSourceOffset += sizeof(nrOfKvPairs);
    bytesRead += sizeof(nrOfKvPairs);

    if ( nrOfKvPairs > 0 )
    {
        // Next is a basic sanity check
        // this is not full proof since the bytes needed for all kv pairs depends on the types of the variants used
        // however we do know that even if its a NIL variant or similar the smallest size is 1 byte hence we need at minimum nrOfKvPairs*2
        UInt32 bytesRemaining = source.GetDataSize() - currentSourceOffset;
        if ( nrOfKvPairs*2 > bytesRemaining )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgBinarySerializer:DeserializeKvPairs: nrOfKvPairs (" + CORE::ToString( nrOfKvPairs ) + ") cannot fit into " + 
                CORE::ToString( bytesRemaining ) + " bytes available, data is corrupt" );
            return false;
        }
        
        kvPairs.clear();
        kvPairs.resize( nrOfKvPairs );

        CIPubSubMsg::TKeyValuePairs::iterator i = kvPairs.begin();
        while ( i != kvPairs.end() )
        {                
            CORE::CVariant& key = (*i).first;               
            CORE::CVariant& value = (*i).second;
                
            UInt32 varByteSize = 0;
            if ( !CORE::CVariantBinarySerializer::Deserialize( key, currentSourceOffset, source, linkWherePossible, varByteSize ) )
                return false;
            currentSourceOffset += varByteSize;
            bytesRead += varByteSize;

            varByteSize = 0;
            if ( !CORE::CVariantBinarySerializer::Deserialize( value, currentSourceOffset, source, linkWherePossible, varByteSize ) )
                return false;
            currentSourceOffset += varByteSize;
            bytesRead += varByteSize;

            ++i;
        }
    }
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubMsgBinarySerializer::Deserialize( const CPubSubMsgBinarySerializerOptions& options ,
                                         bool linkWherePossible                           ,
                                         CIPubSubMsg& msg                                 , 
                                         UInt32 currentSourceOffset                       , 
                                         const CORE::CDynamicBuffer& source               , 
                                         UInt32& bytesRead                                )
{GUCEF_TRACE;

    try
    {
        if ( options.msgDateTimeIncluded )
        {
            if ( options.msgDateTimeAsMsSinceUnixEpochInUtc )
            {
                // Read the DateTime of the msg which is milliseconds since Unix epoch in UTC IF
                // this class's Serialize() was used. Commonly used Unix epoch variables do NOT have a  
                // timezone associated nor does the Unix epoch offset in a lot of implementations take
                // leap seconds into account. Hence choose carefully to ensure consistency in interpretation
                UInt64 msSinceUnixEpochInUtc = source.ReadValue< UInt64 >( currentSourceOffset );
                currentSourceOffset += sizeof(msSinceUnixEpochInUtc);
                bytesRead += sizeof(msSinceUnixEpochInUtc);

                msg.GetMsgDateTime().FromUnixEpochBasedTicksInMillisecs( msSinceUnixEpochInUtc );
            }
            else
            {
                // Read the DateTime of the msg which is a ISO 8601 format string
                Int32 dtStrSize = msg.GetMsgDateTime().FromIso8601DateTimeString( source, currentSourceOffset );
                if ( 0 >= dtStrSize )
                    return false;
                currentSourceOffset += (UInt32) dtStrSize;
                bytesRead += (UInt32) dtStrSize;
            }
        }

        if ( options.msgIdIncluded )
        {
            // Read the ID using the variable length variant serializer
            UInt32 varByteSize = 0;
            if ( !CORE::CVariantBinarySerializer::Deserialize( msg.GetMsgId(), currentSourceOffset, source, linkWherePossible, varByteSize ) )
                return false;
            currentSourceOffset += varByteSize;
            bytesRead += varByteSize;
        }

        if ( options.msgIndexIncluded )
        {
            // Read the msg index using the variable length variant serializer
            UInt32 varByteSize = 0;
            if ( !CORE::CVariantBinarySerializer::Deserialize( msg.GetMsgIndex(), currentSourceOffset, source, linkWherePossible, varByteSize ) )
                return false;
            currentSourceOffset += varByteSize;
            bytesRead += varByteSize;
        }

        if ( options.msgPrimaryPayloadIncluded )
        {
            // Now read the primary payload if any exists
            UInt32 varByteSize = 0;
            if ( !CORE::CVariantBinarySerializer::Deserialize( msg.GetPrimaryPayload(), currentSourceOffset, source, linkWherePossible, varByteSize ) )
                return false;
            currentSourceOffset += varByteSize;
            bytesRead += varByteSize;
        }

        if ( options.msgKeyValuePairsIncluded )
        {
            UInt32 kvPairsByteSize = 0;
            CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetKeyValuePairs();
            if ( !DeserializeKvPairs( linkWherePossible, kvPairs, currentSourceOffset, source, kvPairsByteSize ) )
                return false;
            currentSourceOffset += kvPairsByteSize;
            bytesRead += kvPairsByteSize;          
        }

        if ( options.msgMetaDataKeyValuePairsIncluded )
        {
            UInt32 kvPairsByteSize = 0;
            CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetMetaDataKeyValuePairs();
            if ( !DeserializeKvPairs( linkWherePossible, kvPairs, currentSourceOffset, source, kvPairsByteSize ) )
                return false;
            currentSourceOffset += kvPairsByteSize;
            bytesRead += kvPairsByteSize;          
        }

        if ( options.topicNameIncluded )
        {
            // the topic name is stored as a variant to avoid having to deal with string encoding issues
            CORE::CVariant topicNameVariant;

            UInt32 varByteSize = 0;
            if ( !CORE::CVariantBinarySerializer::Deserialize( topicNameVariant, currentSourceOffset, source, linkWherePossible, varByteSize ) )
                return false;
            currentSourceOffset += varByteSize;
            bytesRead += varByteSize;

            CORE::CString topicName = CORE::ToString( topicNameVariant );
            msg.SetOriginClientTopicName( topicName );
        }

        return true;
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, CString( "PubSubMsgBinarySerializer:Deserialize: caught exception: " ) + e.what()  );
        return false;
    } 
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
