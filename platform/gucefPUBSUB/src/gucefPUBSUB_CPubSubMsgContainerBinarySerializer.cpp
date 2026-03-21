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

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#include "gucefPUBSUB_CPubSubMsgContainerBinarySerializer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPubSubMsgContainerBinarySerializer::MagicText = "PUBSUBMSGS";
const CORE::UInt8   CPubSubMsgContainerBinarySerializer::CurrentFormatVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::SerializeHeader( const CPubSubMsgBinarySerializerOptions& options ,
                                                      CORE::CIOAccess& dest                            ,
                                                      UInt32& bytesWritten                             )
{GUCEF_TRACE;

    if ( !dest.IsWriteable() || !dest.IsValid() )
    {
        // We consider this an error because the source is not in a state to be read from which means upstream some assumption failed
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeHeader: Source is not writeable or invalid" );
        return false;
    }
    if ( !dest.Opened() ) 
    {
        dest.Open();
    }
    UInt64 pos = dest.Tell();
    if ( 0 != pos )
    {
        if ( 0 != dest.Setpos( 0 ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeHeader: Failed to seek to start of resource" );
            return false;
        }
    }

    // magic text bytes to help identify the blob as being a container of pub sub messages
    if ( 1 != dest.Write( MagicText.C_String(), MagicText.ByteSize()-1, 1 ) )
        return false;
    bytesWritten += MagicText.ByteSize()-1;

    // Reserved for the future version field
    if ( 1 != dest.Write( &CurrentFormatVersion, sizeof( CurrentFormatVersion ), 1 ) )
        return false;
    bytesWritten += sizeof( CurrentFormatVersion );

    // Since the serializer settings used have to match to ensure compatibility we will
    // store those in the header so that we know the format of each message
    UInt32 msgSerializerOptions = options.ToOptionsBitMask();
    if ( 1 != dest.Write( &msgSerializerOptions, sizeof( msgSerializerOptions ), 1 ) )
        return false;
    bytesWritten += sizeof( msgSerializerOptions );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::SerializeHeader( const CPubSubMsgBinarySerializerOptions& options ,
                                                      UInt32 currentTargetOffset                       ,
                                                      CORE::CDynamicBuffer& target                     ,
                                                      UInt32& bytesWritten                             )
{GUCEF_TRACE;

    // magic text bytes to help identify the blob as being a container of pub sub messages
    UInt32 newBytesWritten = target.CopyFrom( MagicText, currentTargetOffset, false );
    bytesWritten += newBytesWritten;
    currentTargetOffset += newBytesWritten;
    if ( newBytesWritten != MagicText.ByteSize()-1 )
        return false;

    // Reserved for the future version field
    UInt8 formatVersion = CurrentFormatVersion;
    newBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(formatVersion), &formatVersion );
    bytesWritten += newBytesWritten;
    currentTargetOffset += newBytesWritten;
    if ( newBytesWritten != sizeof(formatVersion) )
        return false;

    // Since the serializer settings used have to match to ensure compatibility we will
    // store those in the header so that we know the format of each message
    UInt32 msgSerializerOptions = options.ToOptionsBitMask();
    newBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(msgSerializerOptions), &msgSerializerOptions );
    bytesWritten += newBytesWritten;
    currentTargetOffset += newBytesWritten;
    if ( newBytesWritten != sizeof(msgSerializerOptions) )
        return false;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeHeader( CPubSubMsgBinarySerializerOptions& options ,
                                                        CORE::CIOAccess& source                    ,
                                                        UInt32& bytesRead                          )
{GUCEF_TRACE;

    try
    {
        options.Clear();
        bytesRead = 0;

        if ( !source.IsReadable() || !source.IsValid() )
        {
            // We consider this an error because the source is not in a state to be read from which means upstream some assumption failed
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeHeader: Source is not readable or invalid" );
            return false;
        }

        if ( !source.Opened() ) 
        {
            source.Open();
        }
        else
        {
            if ( source.Setpos( 0 ) != 0 )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeHeader: Failed to seek to start of resource" );
                return false;
            }
        }
        
        // magic text bytes to help identify the blob as being a container of pub sub messages
        CORE::CString testStr;
        if ( GUCEF_NULL == testStr.Reserve( MagicText.ByteSize(), (Int32) MagicText.Length() ) )
            return false;
        if ( 1 != source.Read( testStr.C_String(), MagicText.ByteSize()-1, 1 ) || testStr != MagicText )
        {
            // We dont want to log errors or warnings for this because header reading magic is a typical way to brute force
            // check if the resource is of a certain type
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeHeader: Magic text test failed" );
            bytesRead = (UInt32) source.Tell();
            return false;
        }

        // Next read the format version
        UInt8 formatVersion = 0;
        if ( !source.ReadValue< UInt8 >( formatVersion ) || ( CurrentFormatVersion != formatVersion ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeHeader: Incompatible format version" );
            bytesRead = (UInt32) source.Tell();
            return false;
        }

        // Next read the option flags
        UInt32 optionsBitMask = 0;
        if ( !source.ReadValue< UInt32 >( optionsBitMask ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeHeader: Incompatible format" );
            bytesRead = (UInt32) source.Tell();
            return false;
        }

        options.FromOptionsBitMask( optionsBitMask );
        bytesRead = (UInt32) source.Tell();

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeHeader: Successfully read header" );
        return true;
    }
    catch ( const std::exception& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeHeader( CPubSubMsgBinarySerializerOptions& options ,
                                                        UInt32 currentSourceOffset                 ,
                                                        const CORE::CDynamicBuffer& source         ,
                                                        UInt32& bytesRead                          )
{GUCEF_TRACE;

    try
    {
        // magic text bytes to help identify the blob as being a container of pub sub messages
        CORE::CString testStr;
        UInt32 newBytesRead = source.CopyTo( currentSourceOffset, MagicText.ByteSize()-1, testStr.Reserve( MagicText.ByteSize(), (Int32) MagicText.Length() ) );
        bytesRead += newBytesRead;
        currentSourceOffset += newBytesRead;
        if ( newBytesRead != MagicText.ByteSize()-1 || testStr != MagicText )
        {
            // We dont want to log errors or warnings for this because header reading magic is a typical way to brute force
            // check if the resource is of a certain type
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeHeader: Magic text test failed" );
            return false;
        }

        UInt8 formatVersion = source.AsConstType< UInt8 >( currentSourceOffset );
        bytesRead += sizeof(formatVersion);
        currentSourceOffset += sizeof(formatVersion);
        if ( CurrentFormatVersion != formatVersion )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeHeader: Incompatible format version" );
            return false;
        }

        UInt32 optionsBitMask = source.AsConstType< UInt32 >( currentSourceOffset );
        bytesRead += sizeof(optionsBitMask);
        currentSourceOffset += sizeof(optionsBitMask);
        options.FromOptionsBitMask( optionsBitMask );

        return true;
    }
    catch ( const std::exception& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeHeader( CPubSubMsgBinarySerializerOptions& options ,
                                                        const CORE::CDynamicBuffer& source         ,
                                                        UInt32& bytesRead                          )
{GUCEF_TRACE;

    return DeserializeHeader( options, 0, source, bytesRead );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::SerializeFooter( const TMsgOffsetIndex& index ,
                                                      CORE::CIOAccess& target      ,
                                                      UInt32& bytesWritten         )
{GUCEF_TRACE;

    UInt32 footerSize = ((UInt32)index.size()*4)+4+(MagicText.ByteSize()-1);

    // Write the msg offset index
    // The purpose of the index is quick access to messages by index even though the
    // messages have variable length
    // Use the fact that vector is stored as a single block of memory underneath
    if ( !index.empty() )
    {
        UInt32 bytesToWrite = (UInt32) index.size() * sizeof(UInt32);
        if ( 1 != target.Write( &index[0], bytesToWrite, 1 ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeFooter: Failed to write footer index entry field" );
            return false;
        }
        bytesWritten += bytesToWrite;
    }

    // Write the msg offset index size
    // The point of this is fast loading of the footer when reading from the end of the container backwards
    UInt32 indexSize = (UInt32) index.size();
    if ( !target.WriteValue( indexSize ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeFooter: Failed to write footer index size field" );
        return false;
    }
    bytesWritten += sizeof(indexSize);

    // Now write magic text bytes to help identify the blob as being a complete container of pub sub messages
    // If the magic text was at the start of the container but not at the end you know that the container is potentially
    // only a partial file. At that point the footer/index could potentially be rebuild by scanning the entire container from the start
    UInt32 newBytesWritten = target.Write( MagicText, false );
    bytesWritten += newBytesWritten;
    if ( newBytesWritten != MagicText.ByteSize()-1 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeFooter: Failed to write write magic text" );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::SerializeFooter( const TMsgOffsetIndex& index ,
                                                      UInt32 currentTargetOffset   ,
                                                      CORE::CDynamicBuffer& target ,
                                                      UInt32& bytesWritten         )
{GUCEF_TRACE;

    UInt32 footerSize = ((UInt32)index.size()*4)+4+(MagicText.ByteSize()-1);
    if ( target.GetUnusedBufferSize() < footerSize )
        target.SetBufferSize( target.GetBufferSize() + footerSize, false );

    #ifdef GUCEF_DEBUG_MODE
    // Extra sanity check
    if ( index.size() >= 2 )
    {
        for ( UInt64 m=0; m<index.size()-2; ++m )
        {
            if ( index[ m ] > index[ m+1 ] )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeFooter: Non-Ascending footer entry detected" )
            }
        }
    }
    #endif

    // Write the msg offset index
    // The purpose of the index is quick access to messages by index even though the
    // messages have variable length
    // Use the fact that vector is stored as a single block of memory underneath
    if ( !index.empty() )
    {
        UInt32 bytesToWrite = (UInt32) index.size() * sizeof(UInt32);
        UInt32 newBytesWritten = target.CopyFrom( currentTargetOffset, bytesToWrite, &index[0] );
        bytesWritten += newBytesWritten;
        currentTargetOffset += newBytesWritten;

        if ( newBytesWritten != bytesToWrite )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeFooter: Failed to write footer index entry field" );
            return false;
        }
    }

    // Write the msg offset index size
    // The point of this is fast loading of the footer when reading from the end of the container backwards
    UInt32 indexSize = (UInt32) index.size();
    UInt32 newBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(indexSize), &indexSize );
    bytesWritten += newBytesWritten;
    currentTargetOffset += newBytesWritten;
    if ( newBytesWritten != sizeof(indexSize) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeFooter: Failed to write footer index size field" );
        return false;
    }

    // Now write magic text bytes to help identify the blob as being a complete container of pub sub messages
    // If the magic text was at the start of the container but not at the end you know that the container is potentially
    // only a partial file. At that point the footer/index could potentially be rebuild by scanning the entire container from the start
    newBytesWritten = target.CopyFrom( MagicText, currentTargetOffset, false );
    bytesWritten += newBytesWritten;
    currentTargetOffset += newBytesWritten;
    if ( newBytesWritten != MagicText.ByteSize()-1 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeFooter: Failed to write write magic text" );
        return false;
    }

    if ( currentTargetOffset == target.GetDataSize() )
    {
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:SerializeFooter: Buffer data size sanity check failed" );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeFooter( TMsgOffsetIndex& index             ,
                                                        UInt32 currentSourceOffset         ,
                                                        const CORE::CDynamicBuffer& source ,
                                                        UInt32& bytesRead                  ,
                                                        bool hdrToFtrOrderedIndex          )
{GUCEF_TRACE;

    try
    {
        // Note that footer reading occurs backwards from the end of the container towards the front

        if ( source.GetDataSize() < ( MagicText.ByteSize() + 4 ) || currentSourceOffset < ( MagicText.ByteSize() + 4 ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeFooter: Not enough data for minimal footer" );
            return false;
        }

        CORE::CString testStr;
        UInt32 newBytesRead = source.CopyTo( currentSourceOffset - MagicText.ByteSize()+1, MagicText.ByteSize()-1, testStr.Reserve( MagicText.ByteSize(), (Int32) MagicText.Length() ) );
        if ( newBytesRead != MagicText.ByteSize()-1 || testStr != MagicText )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeFooter: Failed to read container footer magic text" );
            return false;
        }
        currentSourceOffset -= MagicText.ByteSize()-1;
        bytesRead += MagicText.ByteSize()-1;

        UInt32 indexSize = 0;
        indexSize = source.AsConstType< UInt32 >( currentSourceOffset - sizeof(indexSize) );
        currentSourceOffset -= sizeof(indexSize);
        bytesRead += sizeof(indexSize);

        // read the msg offset index
        // The purpose of the index is quick access to messages by index even though the
        // messages have variable length
        index.reserve( indexSize );
        if ( hdrToFtrOrderedIndex )
        {
            // We want to switch to forward reading in order to order the index entries as such
            currentSourceOffset = currentSourceOffset - ( indexSize * sizeof(UInt32) );
            for ( UInt32 i=0; i<indexSize; ++i )
            {
                UInt32 msgOffset = 0;
                msgOffset = source.AsConstType< UInt32 >( currentSourceOffset );
                currentSourceOffset += sizeof(msgOffset);
                bytesRead += sizeof(msgOffset);

                if ( !index.empty() )
                {
                    if ( msgOffset < index[ index.size()-1 ] )
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeFooter: non-ascending footer entry detected while reading hdr to ftr" );
                    }
                }

                index.push_back( msgOffset );
            }
        }
        else
        {
            for ( UInt32 i=0; i<indexSize; ++i )
            {
                UInt32 msgOffset = 0;
                currentSourceOffset -= sizeof(UInt32);
                msgOffset = source.AsConstType< UInt32 >( currentSourceOffset );
                bytesRead += sizeof(UInt32);

                if ( !index.empty() )
                {
                    if ( msgOffset > index[ index.size()-1 ] )
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeFooter: non-decending footer entry detected while reading ftr to hdr" );
                    }
                }

                index.push_back( msgOffset );
            }
        }

        return true;
    }
    catch ( const std::exception& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeFooter( TMsgOffsetIndex& index             ,
                                                        const CORE::CDynamicBuffer& source ,
                                                        UInt32& bytesRead                  ,
                                                        bool hdrToFtrOrderedIndex          )
{GUCEF_TRACE;

    return DeserializeFooter( index, source.GetDataSize(), source, bytesRead, hdrToFtrOrderedIndex );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeFirstAndLastMsgOffsetFromFooter( const CORE::CDynamicBuffer& source ,
                                                                                 UInt32 currentSourceOffset         ,
                                                                                 UInt32& indexSize                  ,
                                                                                 UInt32& firstMsgOffset             ,
                                                                                 UInt32& lastMsgOffset              )
{GUCEF_TRACE;

    try
    {
        // Note that footer reading occurs backwards from the end of the container towards the front

        indexSize = 0;
        firstMsgOffset = 0;
        lastMsgOffset = 0;

        CORE::CString testStr;
        UInt32 newBytesRead = source.CopyTo( currentSourceOffset - MagicText.ByteSize()+1, MagicText.ByteSize()-1, testStr.Reserve( MagicText.ByteSize(), (Int32) MagicText.Length() ) );
        if ( newBytesRead != MagicText.ByteSize()-1 || testStr != MagicText )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeFirstAndLastMsgOffsetFromFooter: footer magic text verification failed" );
            return false;
        }
        currentSourceOffset -= MagicText.ByteSize()-1;

        indexSize = source.AsConstType< UInt32 >( currentSourceOffset - sizeof(indexSize) );
        currentSourceOffset -= sizeof(indexSize);

        if ( 0 == indexSize )
            return true;

        currentSourceOffset = currentSourceOffset - ( indexSize * sizeof(UInt32) );
        firstMsgOffset = source.AsConstType< UInt32 >( currentSourceOffset );

        currentSourceOffset = currentSourceOffset + ( (indexSize-1) * sizeof(UInt32) );
        lastMsgOffset = source.AsConstType< UInt32 >( currentSourceOffset );
        return true;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "PubSubMsgContainerBinarySerializer:DeserializeFirstAndLastMsgOffsetFromFooter: caught exception: " ) + e.what()  );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

template < class MsgCollectionType >
bool
CPubSubMsgContainerBinarySerializer::SerializeMsgCollection( const CPubSubMsgBinarySerializerOptions& options ,
                                                             const MsgCollectionType& msgs                    ,
                                                             UInt32 currentTargetOffset                       ,
                                                             CORE::CDynamicBuffer& target                     ,
                                                             UInt32& bytesWritten                             )
{GUCEF_TRACE;

    // Write the header
    UInt32 newBytesWritten = 0;
    bool wroteHeader = SerializeHeader( options, currentTargetOffset, target, newBytesWritten );
    currentTargetOffset += newBytesWritten;
    bytesWritten += newBytesWritten;
    if ( !wroteHeader )
        return false;

    // Write all the messages
    // We do not accept partial success
    TMsgOffsetIndex index( msgs.size(), 0 );
    TMsgOffsetIndex::iterator n = index.begin();
    typename MsgCollectionType::const_iterator i = msgs.begin();
    while ( i != msgs.end() )
    {
        const CIPubSubMsg* msg = AsIPubSubMsgPointer( (*i) );
        if ( GUCEF_NULL != msg )
        {
            newBytesWritten = 0;
            if ( CPubSubMsgBinarySerializer::Serialize( options, *msg, currentTargetOffset, target, newBytesWritten ) )
            {
                (*n) = currentTargetOffset;
                currentTargetOffset += newBytesWritten;
                bytesWritten += newBytesWritten;
            }
            else
                return false;
        }
        ++i; ++n;
    }

    // Write the footer
    newBytesWritten = 0;
    bool wroteFooter = SerializeFooter( index, currentTargetOffset, target, newBytesWritten );
    currentTargetOffset += newBytesWritten;
    bytesWritten += newBytesWritten;
    if ( !wroteFooter )
        return false;

    return true;
}


/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::Serialize( const CPubSubMsgBinarySerializerOptions& options ,
                                                const TBasicPubSubMsgVector& msgs                ,
                                                UInt32 currentTargetOffset                       ,
                                                CORE::CDynamicBuffer& target                     ,
                                                UInt32& bytesWritten                             )
{GUCEF_TRACE;

    return SerializeMsgCollection< TBasicPubSubMsgVector >( options, msgs, currentTargetOffset, target, bytesWritten );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::Serialize( const CPubSubMsgBinarySerializerOptions& options     ,
                                                const CPubSubClientTopic::TPubSubMsgsRefVector& msgs ,
                                                UInt32 currentTargetOffset                           ,
                                                CORE::CDynamicBuffer& target                         ,
                                                UInt32& bytesWritten                                 )
{GUCEF_TRACE;

    return SerializeMsgCollection< CPubSubClientTopic::TPubSubMsgsRefVector >( options, msgs, currentTargetOffset, target, bytesWritten );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::IndexRebuildScan( TMsgOffsetIndex& index             ,
                                                       const CORE::CDynamicBuffer& source ,
                                                       UInt32& bytesRead                  ,
                                                       bool hdrToFtrOrderedIndex          )
{GUCEF_TRACE;

    CPubSubMsgBinarySerializerOptions options;
    if ( !DeserializeHeader( options, source, bytesRead ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:IndexRebuildScan: Failed to read container header" );
        return false;
    }

    TMsgOffsetIndex tmpIndex;
    while ( bytesRead < source.GetDataSize() )
    {
        UInt32 msgBytesRead = 0;
        CBasicPubSubMsg msg;
        if ( CPubSubMsgBinarySerializer::Deserialize( options, true, msg, bytesRead, source, msgBytesRead ) )
        {
            if ( hdrToFtrOrderedIndex )
                index.push_back( bytesRead );
            else
                tmpIndex.push_back( bytesRead );

            bytesRead += msgBytesRead;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:IndexRebuildScan: Failed to deserialize msg with " +
                CORE::ToString( msgBytesRead ) + " bytes read from offset " + CORE::ToString( bytesRead ) + ". This will be considered the end" );
            break;
        }
    }

    if ( !hdrToFtrOrderedIndex )
    {
        TMsgOffsetIndex::reverse_iterator i = tmpIndex.rbegin();
        while ( i != tmpIndex.rend() )
        {
            index.push_back( bytesRead );
            ++i;
        }
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:IndexRebuildScan: Index rebuild discovered " + CORE::SizeTToString( index.size() ) + " messages" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::Deserialize( CPubSubClientTopic::TPubSubMsgsRefVector& msgs ,
                                                  bool linkWherePossible                         ,
                                                  const TMsgOffsetIndex& index                   ,
                                                  const CORE::CDynamicBuffer& source             ,
                                                  bool& isCorrupted                              )
{GUCEF_TRACE;

    isCorrupted = false;
    CORE::UInt32 bytesRead = 0;
    CPubSubMsgBinarySerializerOptions options;
    if ( !DeserializeHeader( options, source, bytesRead ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:Deserialize: Failed to read container header" );
        isCorrupted = true;
        return false;
    }

    if ( msgs.size() <= index.size() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:Deserialize: The given message storage was not properly pre-allocated ( " +
                CORE::SizeTToString( msgs.size() ) + " vs " + CORE::SizeTToString( index.size() ) + ")" );
        isCorrupted = false;
        return false;
    }

    CORE::UInt32 msgIndex = 0;
    CPubSubClientTopic::TPubSubMsgsRefVector::iterator m = msgs.begin();
    TMsgOffsetIndex::const_iterator i = index.begin();
    while ( i != index.end() )
    {
        CORE::UInt32 offsetOfMsg = (*i);
        CIPubSubMsg* msg = (*m);
        if ( GUCEF_NULL != msg )
        {
            if ( !CPubSubMsgBinarySerializer::Deserialize( options, linkWherePossible, *msg, offsetOfMsg, source, bytesRead ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Failed to deserialize msg at index " + CORE::ToString( msgIndex ) + " and offset " + CORE::ToString( offsetOfMsg ) );
                isCorrupted = true;
                return false;
            }
        }
        ++i; ++m; ++msgIndex;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::Deserialize( TBasicPubSubMsgVector& msgs        ,
                                                  bool linkWherePossible             ,
                                                  const TMsgOffsetIndex& index       ,
                                                  const CORE::CDynamicBuffer& source ,
                                                  bool& isCorrupted                  ,
                                                  bool bestEffortIsOk                )
{GUCEF_TRACE;

    isCorrupted = false;
    CORE::UInt32 bytesRead = 0;
    CPubSubMsgBinarySerializerOptions options;
    if ( !DeserializeHeader( options, source, bytesRead ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:Deserialize: Failed to read container header" );
        isCorrupted = true;
        return false;
    }

    CORE::UInt32 msgIndex = 0;
    TMsgOffsetIndex::const_iterator i = index.begin();
    while ( i != index.end() )
    {
        CORE::UInt32 offsetOfMsg = (*i);

        msgs.push_back( CBasicPubSubMsg() );
        CBasicPubSubMsg& msg = msgs.back();

        if ( !CPubSubMsgBinarySerializer::Deserialize( options, linkWherePossible, msg, offsetOfMsg, source, bytesRead ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Failed to deserialize msg at index " + CORE::ToString( msgIndex ) + " and offset " + CORE::ToString( offsetOfMsg ) );
            isCorrupted = true;

            if ( !bestEffortIsOk )
                return false;
        }
        ++i; ++msgIndex;
    }

    return !isCorrupted;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( TBasicPubSubMsgVector& msgs  ,
                                                             bool linkWherePossible       ,
                                                             CORE::CDynamicBuffer& source ,
                                                             bool& isCorrupted            ,
                                                             bool bestEffortIsOk          )
{GUCEF_TRACE;

    TMsgOffsetIndex index;
    return DeserializeWithRebuild( msgs, linkWherePossible, index, source, isCorrupted, bestEffortIsOk );
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeWithRebuild( TBasicPubSubMsgVector& msgs  ,
                                                             bool linkWherePossible       ,
                                                             TMsgOffsetIndex& index       ,
                                                             CORE::CDynamicBuffer& source ,
                                                             bool& isCorrupted            ,
                                                             bool bestEffortIsOk          )
{GUCEF_TRACE;

    bool performedRebuild = false;
    if ( index.empty() )
    {
        CORE::UInt32 bytesRead = 0;
        if ( !DeserializeFooter( index, source, bytesRead, true ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeWithRebuild: Failed to deserialize pubsub msg container footer. Will attempt index rebuild" );

            // Without a reliable index sourced from the footer 'best effort' deserialize makes no sense because it relies on a good index to skip 'bad' messages
            bestEffortIsOk = false;

            index.clear();
            if ( IndexRebuildScan( index, source, bytesRead, true ) )
            {
                performedRebuild = true;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeWithRebuild: Rebuild of index completed" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeWithRebuild: Failed to rebuild index" );
                return false;
            }
        }
    }

    msgs.reserve( index.size() );
    if ( !Deserialize( msgs, true, index, source, isCorrupted, bestEffortIsOk ) )
    {
        // Have we already tried a rebuild?
        // If we already did, no point in trying again
        if ( !performedRebuild )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeWithRebuild: Failed to deserialize messages. isCorrupted=" +
                CORE::ToString( isCorrupted ) + ". Will attempt index rebuild");

            index.clear();
            CORE::UInt32 bytesRead = 0;
            if ( IndexRebuildScan( index, source, bytesRead, true ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeWithRebuild: Rebuild of index completed" );

                // We never use 'best effort' on the second pass since best effort relies on a reliable index having existed in the first place
                bestEffortIsOk = false;

                bool secondPassIsCorrupted = false;
                return Deserialize( msgs, true, index, source, secondPassIsCorrupted, bestEffortIsOk );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeWithRebuild: Failed to rebuild index" );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeWithRebuild: Failed to deserialize after we already performed an index rebuild" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeFirstAndLastMsgDateTime( CORE::CDateTime& firstMsgDt        ,
                                                                         CORE::CDateTime& lastMsgDt         ,
                                                                         const CORE::CDynamicBuffer& source ,
                                                                         bool& isSupported                  ,
                                                                         bool& isCorrupted                  )
{GUCEF_TRACE;

    isSupported = false;
    isCorrupted = false;
    CORE::UInt32 bytesRead = 0;
    CPubSubMsgBinarySerializerOptions options;
    if ( !DeserializeHeader( options, source, bytesRead ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeFirstAndLastMsgDateTime: Failed to read container header" );
        isCorrupted = true;
        return false;
    }

    isSupported = options.msgDateTimeIncluded;
    if ( !isSupported )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeFirstAndLastMsgDateTime: Selected storage options preclude msg datetime thus cannot obtain msg dt" );
        return false;
    }

    UInt32 indexSize = 0;
    UInt32 firstMsgOffset = 0;
    UInt32 lastMsgOffset = 0;
    if ( !DeserializeFirstAndLastMsgOffsetFromFooter( source               ,
                                                      source.GetDataSize() ,
                                                      indexSize            ,
                                                      firstMsgOffset       ,
                                                      lastMsgOffset        ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeFirstAndLastMsgDateTime: Failed to read first and last msg offsets from footer" );
        return false;
    }

    bytesRead = 0;
    CBasicPubSubMsg msg;
    if ( !CPubSubMsgBinarySerializer::Deserialize( options, true, msg, firstMsgOffset, source, bytesRead ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Failed to deserialize first msg at offset " + CORE::ToString( firstMsgOffset ) );
        isCorrupted = true;
        return false;
    }
    firstMsgDt = msg.GetMsgDateTime();

    if ( !CPubSubMsgBinarySerializer::Deserialize( options, true, msg, lastMsgOffset, source, bytesRead ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Failed to deserialize last msg at offset " + CORE::ToString( lastMsgOffset ) );
        isCorrupted = true;
        return false;
    }
    lastMsgDt = msg.GetMsgDateTime();

    return true;
}
/*-------------------------------------------------------------------------*/

bool
CPubSubMsgContainerBinarySerializer::DeserializeMsgAtIndex( CIPubSubMsg& msg                   ,
                                                            bool linkWherePossible             ,
                                                            const CORE::CDynamicBuffer& source ,
                                                            UInt32 msgIndex                    ,
                                                            bool fromStart                     ,
                                                            bool& isCorrupted                  )
{GUCEF_TRACE;

    isCorrupted = false;
    CORE::UInt32 bytesRead = 0;
    CPubSubMsgBinarySerializerOptions options;
    if ( !DeserializeHeader( options, source, bytesRead ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Failed to read container header" );
        isCorrupted = true;
        return false;
    }

    // Read footer in hdr-to-ftr order (hdrToFtrOrderedIndex=true) so index[0]=first msg, index[size-1]=last msg
    TMsgOffsetIndex index;
    if ( !DeserializeFooter( index, source, bytesRead, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Failed to read container footer, this file is corrupted" );
        isCorrupted = true;
        return false;
    }

    if ( index.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Footer claims the container is empty" );
        // This does not count as corruption since the caller may very well just have the wrong index, attempting to load a msg from a placeholder/token resource
        return false;
    }

    if ( msgIndex >= index.size() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Per footer the index is out of range. Actual Nr of items: " + CORE::SizeTToString( index.size() ) );
        isCorrupted = true;
        return false;
    }

    // The footer index is in hdr-to-ftr order (index[0]=first msg, index[size-1]=last msg)
    // fromStart=true  + msgIndex=0 → first message  → index[0]
    // fromStart=false + msgIndex=0 → last message   → index[size-1]
    UInt32 actualIndex = (UInt32) ( fromStart ? msgIndex : (index.size()-1) - msgIndex );
    CORE::UInt32 offsetOfLastMsg = index[ actualIndex ];
    if ( !CPubSubMsgBinarySerializer::Deserialize( options, linkWherePossible, msg, offsetOfLastMsg, source, bytesRead ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSubMsgContainerBinarySerializer:DeserializeMsgAtIndex: Failed to deserialize msg at index " + CORE::ToString( actualIndex ) + " and offset " + CORE::ToString( offsetOfLastMsg ) );
        isCorrupted = true;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
