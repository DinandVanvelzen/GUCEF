/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CMessageSerializer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CMessageSerializerOptions::ClassTypeName = "GUCEF::COMCORE::CMessageSerializerOptions";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMessageSerializerOptions::CMessageSerializerOptions( void )
    : CDataNodeSerializableSettings()
    , msgIdIncluded( true )
    , msgIdTypeIncluded( true )
    , msgIndexIncluded( true )
    , msgIndexTypeIncluded( true )
    , msgDateTimeIncluded( true )
    , msgDateTimeAsMsSinceUnixEpochInUtc( true )
    , msgPrimaryPayloadIncluded( true )
    , msgPrimaryPayloadTypeIncluded( true )
    , msgKeyValuePairsIncluded( true )
    , msgMetaDataKeyValuePairsIncluded( true )
    , includeUndefinedValues( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMessageSerializerOptions::CMessageSerializerOptions( const CORE::CDataNodeSerializableSettings& basicNodeOptions )
    : CDataNodeSerializableSettings( basicNodeOptions )
    , msgIdIncluded( true )
    , msgIdTypeIncluded( true )
    , msgIndexIncluded( true )
    , msgIndexTypeIncluded( true )
    , msgDateTimeIncluded( true )
    , msgDateTimeAsMsSinceUnixEpochInUtc( true )
    , msgPrimaryPayloadIncluded( true )
    , msgPrimaryPayloadTypeIncluded( true )
    , msgKeyValuePairsIncluded( true )
    , msgMetaDataKeyValuePairsIncluded( true )
    , includeUndefinedValues( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMessageSerializerOptions::~CMessageSerializerOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMessageSerializerOptions::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool success = true;

    if ( CDataNodeSerializableSettings::SaveConfig( config ) )
    {
        success = config.SetAttribute( "msgDateTimeIncluded", msgDateTimeIncluded ) && success;
        success = config.SetAttribute( "msgDateTimeAsMsSinceUnixEpochInUtc", msgDateTimeAsMsSinceUnixEpochInUtc ) && success;
        success = config.SetAttribute( "msgIdIncluded", msgIdIncluded ) && success;
        success = config.SetAttribute( "msgIdTypeIncluded", msgIdTypeIncluded ) && success;
        success = config.SetAttribute( "msgIndexIncluded", msgIndexIncluded ) && success;
        success = config.SetAttribute( "msgIndexTypeIncluded", msgIndexTypeIncluded ) && success;
        success = config.SetAttribute( "msgPrimaryPayloadIncluded", msgPrimaryPayloadIncluded ) && success;
        success = config.SetAttribute( "msgPrimaryPayloadTypeIncluded", msgPrimaryPayloadTypeIncluded ) && success;
        success = config.SetAttribute( "msgKeyValuePairsIncluded", msgKeyValuePairsIncluded ) && success;
        success = config.SetAttribute( "msgMetaDataKeyValuePairsIncluded", msgMetaDataKeyValuePairsIncluded ) && success;

        success = config.SetAttribute( "includeUndefinedValues", includeUndefinedValues ) && success;        
    }
    
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CMessageSerializerOptions::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( CDataNodeSerializableSettings::LoadConfig( config ) )
    {
        // We initialize initial settings based on the Level-Of-Detail (LOD) per the base class
        // This negates the need to config all fields as desired. You only need to config those where you wish to differ from the LOD level defaults
        if ( levelOfDetail >= DataNodeSerializableLod_MaximumDetails )
        {
            msgDateTimeIncluded = true;
            msgDateTimeAsMsSinceUnixEpochInUtc = true;
            msgIdIncluded = true;
            msgIdTypeIncluded = true;
            msgIndexIncluded = true;
            msgIndexTypeIncluded = true;
            msgPrimaryPayloadIncluded = true;
            msgPrimaryPayloadTypeIncluded = true;
            msgKeyValuePairsIncluded = true;
            msgMetaDataKeyValuePairsIncluded = true;
            includeUndefinedValues = true;
        }
        else
        if ( levelOfDetail == DataNodeSerializableLod_KeyOnly )
        {
            msgDateTimeIncluded = false;
            msgDateTimeAsMsSinceUnixEpochInUtc = false;
            msgIdIncluded = true;
            msgIdTypeIncluded = true;
            msgIndexIncluded = true;
            msgIndexTypeIncluded = true;
            msgPrimaryPayloadIncluded = false;
            msgPrimaryPayloadTypeIncluded = false;
            msgKeyValuePairsIncluded = false;
            msgMetaDataKeyValuePairsIncluded = false;
            includeUndefinedValues = false;
        }
        else
        {
            // We only support 3 default levels here since the data is already pretty basic
            // the only thing dropped is meta-data
            
            msgDateTimeIncluded = true;
            msgDateTimeAsMsSinceUnixEpochInUtc = true;
            msgIdIncluded = true;
            msgIdTypeIncluded = true;
            msgIndexIncluded = true;
            msgIndexTypeIncluded = true;
            msgPrimaryPayloadIncluded = true;
            msgPrimaryPayloadTypeIncluded = true;
            msgKeyValuePairsIncluded = true;
            msgMetaDataKeyValuePairsIncluded = false;
            includeUndefinedValues = false;
        }

        msgDateTimeIncluded = config.GetAttributeValueOrChildValueByName( "msgDateTimeIncluded" ).AsBool( msgDateTimeIncluded, true );
        msgDateTimeAsMsSinceUnixEpochInUtc = config.GetAttributeValueOrChildValueByName( "msgDateTimeAsMsSinceUnixEpochInUtc" ).AsBool( msgDateTimeAsMsSinceUnixEpochInUtc, true );
        msgIdIncluded = config.GetAttributeValueOrChildValueByName( "msgIdIncluded" ).AsBool( msgIdIncluded, true );
        msgIdTypeIncluded = config.GetAttributeValueOrChildValueByName( "msgIdTypeIncluded" ).AsBool( msgIdTypeIncluded, true );
        msgIndexIncluded = config.GetAttributeValueOrChildValueByName( "msgIndexIncluded" ).AsBool( msgIndexIncluded, true );
        msgIndexTypeIncluded = config.GetAttributeValueOrChildValueByName( "msgIndexTypeIncluded" ).AsBool( msgIndexTypeIncluded, true );
        msgPrimaryPayloadIncluded = config.GetAttributeValueOrChildValueByName( "msgPrimaryPayloadIncluded" ).AsBool( msgPrimaryPayloadIncluded, true );
        msgPrimaryPayloadTypeIncluded = config.GetAttributeValueOrChildValueByName( "msgPrimaryPayloadTypeIncluded" ).AsBool( msgPrimaryPayloadTypeIncluded, true );
        msgKeyValuePairsIncluded = config.GetAttributeValueOrChildValueByName( "msgKeyValuePairsIncluded" ).AsBool( msgKeyValuePairsIncluded, true );
        msgMetaDataKeyValuePairsIncluded = config.GetAttributeValueOrChildValueByName( "msgMetaDataKeyValuePairsIncluded" ).AsBool( msgMetaDataKeyValuePairsIncluded, true );

        includeUndefinedValues = config.GetAttributeValueOrChildValueByName( "includeUndefinedValues" ).AsBool( includeUndefinedValues, true );
    
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

const CString& 
CMessageSerializerOptions::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CMessageSerializerOptions::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMessageSerializerOptions( *this );
}

/*-------------------------------------------------------------------------*/

UInt32 
CMessageSerializerOptions::ToOptionsBitMask( void ) const
{GUCEF_TRACE;

    // First set of flags we order the same as for the binary serializer
    UInt32 bitMask = 0;
    msgDateTimeIncluded ? GUCEF_SETBITXON( bitMask, 1 ) : GUCEF_SETBITXOFF( bitMask, 1 );
    msgDateTimeAsMsSinceUnixEpochInUtc ? GUCEF_SETBITXON( bitMask, 2 ) : GUCEF_SETBITXOFF( bitMask, 2 );
    msgIdIncluded ? GUCEF_SETBITXON( bitMask, 3 ) : GUCEF_SETBITXOFF( bitMask, 3 );    
    msgIndexIncluded ? GUCEF_SETBITXON( bitMask, 4 ) : GUCEF_SETBITXOFF( bitMask, 4 );    
    msgPrimaryPayloadIncluded ? GUCEF_SETBITXON( bitMask, 5 ) : GUCEF_SETBITXOFF( bitMask, 5 );
    msgKeyValuePairsIncluded ? GUCEF_SETBITXON( bitMask, 6 ) : GUCEF_SETBITXOFF( bitMask, 6 );
    msgMetaDataKeyValuePairsIncluded ? GUCEF_SETBITXON( bitMask, 7 ) : GUCEF_SETBITXOFF( bitMask, 7 );
    
    // This serializer may also need to explicitly communicate type information which may be lost otherwise
    // As such it has additional flags
    includeUndefinedValues ? GUCEF_SETBITXON( bitMask, 8 ) : GUCEF_SETBITXOFF( bitMask, 8 );
    msgIdTypeIncluded ? GUCEF_SETBITXON( bitMask, 9 ) : GUCEF_SETBITXOFF( bitMask, 9 );
    msgIndexTypeIncluded ? GUCEF_SETBITXON( bitMask, 10 ) : GUCEF_SETBITXOFF( bitMask, 10 );
    msgPrimaryPayloadTypeIncluded ? GUCEF_SETBITXON( bitMask, 11 ) : GUCEF_SETBITXOFF( bitMask, 11 );
    return bitMask;
}

/*-------------------------------------------------------------------------*/

void 
CMessageSerializerOptions::FromOptionsBitMask( UInt32 bitMask )
{GUCEF_TRACE;

    // First set of flags we order the same as for the binary serializer
    msgDateTimeIncluded = ( 0 != GUCEF_GETBITX( bitMask, 1 ) );
    msgDateTimeAsMsSinceUnixEpochInUtc = ( 0 != GUCEF_GETBITX( bitMask, 2 ) );
    msgIdIncluded = ( 0 != GUCEF_GETBITX( bitMask, 3 ) );
    msgIndexIncluded = ( 0 != GUCEF_GETBITX( bitMask, 4 ) );
    msgPrimaryPayloadIncluded = ( 0 != GUCEF_GETBITX( bitMask, 5 ) );
    msgKeyValuePairsIncluded = ( 0 != GUCEF_GETBITX( bitMask, 6 ) );
    msgMetaDataKeyValuePairsIncluded = ( 0 != GUCEF_GETBITX( bitMask, 7 ) );

    // This serializer may also need to explicitly communicate type information which may be lost otherwise
    // As such it has additional flags
    includeUndefinedValues = ( 0 != GUCEF_GETBITX( bitMask, 8 ) );
    msgIdTypeIncluded = ( 0 != GUCEF_GETBITX( bitMask, 9 ) );
    msgIndexTypeIncluded = ( 0 != GUCEF_GETBITX( bitMask, 10 ) );
    msgPrimaryPayloadTypeIncluded = ( 0 != GUCEF_GETBITX( bitMask, 11 ) );
}

/*-------------------------------------------------------------------------*/

bool
CMessageSerializer::SerializeKvPairs( const CIMessage::TKeyValuePairs& kvPairs ,
                                      CORE::CDataNode& output                  )
{GUCEF_TRACE;

    bool success = true;
    CIMessage::TKeyValuePairs::const_iterator i = kvPairs.begin();
    while ( i != kvPairs.end() )
    {
        success = output.AddChildWithValue( (*i).first.AsString(), (*i).second ) && success;
        ++i;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CMessageSerializer::Serialize( const CMessageSerializerOptions& options ,
                               const CIMessage& msg                     ,
                               CORE::CDataNode& output                  )
{GUCEF_TRACE;

    bool success = true;
    try
    {
        output.Clear();
        output.SetNodeType( GUCEF_DATATYPE_OBJECT );
        output.SetName( "Message" );

        if ( options.msgDateTimeIncluded )
        {
            if ( options.msgDateTimeAsMsSinceUnixEpochInUtc )
            {
                // Write the DateTime of the msg as milliseconds since Unix Epoch in UTC
                // In most applications Unix Epoch is not UTC and leap seconds are not taken into account
                // However since the source is of higher fidelity we can retain accuracy as long as the same method is
                // used to deserialize as well. There is a problem with any DateTime which predates Unix epoch
                // If you have to deal with those use string formatting instead
                UInt64 msSinceUnixEpochInUtc = msg.GetMsgDateTime().ToUnixEpochBasedTicksInMillisecs();
                success = output.SetAttribute( "msgDateTimeAsMsSinceUnixEpochInUtc", msSinceUnixEpochInUtc ) && success;
            }
            else
            {
                // Write the DateTime of the msg which as a ISO 8601 spec string
                // Writing it in string form actually takes up less space vs writing all the binary components at full fidelity
                // Using Unix epoch uses less space but we are limited in the time range since historical dates would be a problem 
                CORE::CString dtStr = msg.GetMsgDateTime().ToIso8601DateTimeString( false, true );
                success = output.SetAttribute( "msgDateTimeAsIso8601", dtStr ) && success;
            }
        }

        if ( GUCEF_DATATYPE_UNKNOWN != msg.GetMsgId().GetTypeId() || options.includeUndefinedValues )
        {
            if ( options.msgIdIncluded )
            {
                success = output.SetAttribute( "msgId", msg.GetMsgId() ) && success;
            }
            if ( options.msgIdTypeIncluded )
            {
                success = output.SetAttribute( "msgIdType", msg.GetMsgId().GetTypeId() ) && success;
            }
        }

        if ( GUCEF_DATATYPE_UNKNOWN != msg.GetMsgIndex().GetTypeId() || options.includeUndefinedValues )
        {
            if ( options.msgIndexIncluded )
            {
                success = output.SetAttribute( "msgIndex", msg.GetMsgIndex() ) && success;
            }
            if ( options.msgIndexTypeIncluded )
            {
                success = output.SetAttribute( "msgIndexType", msg.GetMsgIndex().GetTypeId() ) && success;
            }
        }

        if ( GUCEF_DATATYPE_UNKNOWN != msg.GetPrimaryPayload().GetTypeId() || options.includeUndefinedValues )
        {
            if ( options.msgPrimaryPayloadIncluded )
            {
                success = output.AddChildWithValue( "primaryPayload", msg.GetPrimaryPayload() ) && success;
            }
            if ( options.msgPrimaryPayloadTypeIncluded )
            {
                success = output.AddChildWithValue( "primaryPayloadType", CORE::CVariant( msg.GetPrimaryPayload().GetTypeId() ) ) && success;
            }
        }

        if ( options.msgKeyValuePairsIncluded )
        {
            const CIMessage::TKeyValuePairs& kvPairs = msg.GetKeyValuePairs();            
            CORE::CDataNode* kvPairsNode = output.AddChild( "keyValuePairs", GUCEF_DATATYPE_ARRAY );
            success = SerializeKvPairs( kvPairs, *kvPairsNode ) && success;
        }

        if ( options.msgMetaDataKeyValuePairsIncluded )
        {
            const CIMessage::TKeyValuePairs& kvPairs = msg.GetKeyValuePairs();            
            CORE::CDataNode* kvPairsNode = output.AddChild( "metaDataKeyValuePairs", GUCEF_DATATYPE_ARRAY );
            success = SerializeKvPairs( kvPairs, *kvPairsNode ) && success;
        }

    }   
    catch ( const std::exception& e )    
    {
        success = false;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, CString( "MessageSerializer:Serialize: caught exception: " ) + e.what()  );        
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CMessageSerializer::Serialize( const CMessageSerializerOptions& options ,
                               const CIMessage& msg                     ,
                               CORE::CDynamicBuffer& outputBuffer       ,
                               const CORE::CString& dstoreCodecToUse    )
{GUCEF_TRACE;
    
    try
    {
        CORE::CDataNode nodes;
        if ( Serialize( options, msg, nodes ) )
        {
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( dstoreCodecToUse, codec, false ) && !codec.IsNULL() )
            {
                CORE::CDynamicBufferAccess bufferIoAccess( outputBuffer );
                if ( codec->StoreDataTree( &nodes, &bufferIoAccess ) )
                    return true;
            }
        }
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, CString( "MessageSerializer:Serialize: caught exception: " ) + e.what()  );        
    } 
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMessageSerializer::Deserialize( const CMessageSerializerOptions& options ,
                                 CIMessage& msg                           ,
                                 const CORE::CDataNode& input             )
{GUCEF_TRACE;

    bool success = true;
    try
    {
        if ( options.msgDateTimeIncluded )
        {
            const CORE::CVariant& dtValue = input.GetAttributeValueOrChildValueByName( "msgDateTimeAsMsSinceUnixEpochInUtc" );
            if ( !dtValue.IsNULLOrEmpty() )
            {
                msg.GetMsgDateTime().FromUnixEpochBasedTicksInMillisecs( dtValue.AsUInt64( 0, true ) );
            }
            else
            {
                const CORE::CVariant& iso8601 = input.GetAttributeValueOrChildValueByName( "msgDateTimeAsIso8601" );
                if ( !iso8601.IsNULLOrEmpty() )
                {
                    CORE::CString iso8601Str = iso8601.AsString();
                    msg.GetMsgDateTime().FromIso8601DateTimeString( iso8601Str.C_String(), iso8601Str.Length() );
                }
            }
        }

        if ( options.msgIdIncluded )
        {
            const CORE::CVariant& msgId = input.GetAttributeValueOrChildValueByName( "msgId" );
            if ( !msgId.IsNULLOrEmpty() )
            {
                msg.GetMsgId() = msgId;
            }
        }

        if ( options.msgIndexIncluded )
        {
            const CORE::CVariant& msgIndex = input.GetAttributeValueOrChildValueByName( "msgIndex" );
            if ( !msgIndex.IsNULLOrEmpty() )
            {
                msg.GetMsgIndex() = msgIndex;
            }
        }

        if ( options.msgPrimaryPayloadIncluded )
        {
            const CORE::CDataNode* payloadNode = input.FindChild( "primaryPayload" );
            if ( GUCEF_NULL != payloadNode )
            {
                msg.GetPrimaryPayload() = payloadNode->GetValue();
            }
        }

        if ( options.msgKeyValuePairsIncluded )
        {
            const CORE::CDataNode* kvPairsNode = input.FindChild( "keyValuePairs" );
            if ( GUCEF_NULL != kvPairsNode )
            {
                CORE::CDataNode::const_iterator i = kvPairsNode->ConstBegin();
                while ( i != kvPairsNode->ConstEnd() )
                {
                    msg.GetKeyValuePairs().push_back( CIMessage::TKeyValuePair(
                        CORE::CVariant( (*i)->GetName() ),
                        (*i)->GetValue() ) );
                    ++i;
                }
            }
        }

        if ( options.msgMetaDataKeyValuePairsIncluded )
        {
            const CORE::CDataNode* kvPairsNode = input.FindChild( "metaDataKeyValuePairs" );
            if ( GUCEF_NULL != kvPairsNode )
            {
                CORE::CDataNode::const_iterator i = kvPairsNode->ConstBegin();
                while ( i != kvPairsNode->ConstEnd() )
                {
                    msg.GetMetaDataKeyValuePairs().push_back( CIMessage::TKeyValuePair(
                        CORE::CVariant( (*i)->GetName() ),
                        (*i)->GetValue() ) );
                    ++i;
                }
            }
        }
    }
    catch ( const std::exception& e )
    {
        success = false;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, CString( "MessageSerializer:Deserialize: caught exception: " ) + e.what() );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool 
CMessageSerializer::Deserialize( const CMessageSerializerOptions& options ,
                                 CIMessage& msg                           ,
                                 const CORE::CDynamicBuffer& inputBuffer  ,
                                 const CORE::CString& dstoreCodecToUse    )
{GUCEF_TRACE;

    try
    {
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
        if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( dstoreCodecToUse, codec, false ) && !codec.IsNULL() )
        {
            CORE::CDataNode nodes;
            CORE::CDynamicBufferAccess bufferIoAccess( inputBuffer );
            if ( codec->BuildDataTree( &nodes, &bufferIoAccess ) )
            {
                if ( Deserialize( options, msg, nodes ) )
                {
                    return true;
                }
            }
        }
    }   
    catch ( const std::exception& e )    
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, CString( "MessageSerializer:Serialize: caught exception: " ) + e.what()  );        
    } 
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
