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

#ifndef GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#define GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"    
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Basic serializer for a container of generalized Pub Sub messages
 */
class GUCEF_PUBSUB_EXPORT_CPP CPubSubMsgContainerBinarySerializer
{
    public:
    
    typedef GUCEF::vector< UInt32 >                   TMsgOffsetIndex;
    typedef CBasicPubSubMsg::TBasicPubSubMsgVector  TBasicPubSubMsgVector;

    static const CORE::CString  MagicText;
    static const CORE::UInt8    CurrentFormatVersion;

    static bool SerializeHeader( const CPubSubMsgBinarySerializerOptions& options ,
                                 CORE::CIOAccess& dest                            ,
                                 UInt32& bytesWritten                             );

    static bool SerializeHeader( const CPubSubMsgBinarySerializerOptions& options ,
                                 UInt32 currentTargetOffset                       , 
                                 CORE::CDynamicBuffer& target                     , 
                                 UInt32& bytesWritten                             );

    static bool DeserializeHeader( CPubSubMsgBinarySerializerOptions& options ,
                                   CORE::CIOAccess& source                    ,
                                   UInt32& bytesRead                          );

    static bool DeserializeHeader( CPubSubMsgBinarySerializerOptions& options ,
                                   UInt32 currentSourceOffset                 , 
                                   const CORE::CDynamicBuffer& source         , 
                                   UInt32& bytesRead                          );

    static bool DeserializeHeader( CPubSubMsgBinarySerializerOptions& options ,
                                   const CORE::CDynamicBuffer& source         , 
                                   UInt32& bytesRead                          );

    static bool SerializeFooter( const TMsgOffsetIndex& index ,
                                 CORE::CIOAccess& dest        ,
                                 UInt32& bytesWritten         );

    static bool SerializeFooter( const TMsgOffsetIndex& index ,
                                 UInt32 currentTargetOffset   , 
                                 CORE::CDynamicBuffer& target , 
                                 UInt32& bytesWritten         );

    static bool DeserializeFooter( TMsgOffsetIndex& index             ,
                                   UInt32 currentSourceOffset         , 
                                   const CORE::CDynamicBuffer& source , 
                                   UInt32& bytesRead                  ,
                                   bool hdrToFtrOrderedIndex          );

    static bool DeserializeFooter( TMsgOffsetIndex& index             ,
                                   const CORE::CDynamicBuffer& source , 
                                   UInt32& bytesRead                  ,
                                   bool hdrToFtrOrderedIndex          );

    static bool DeserializeFirstAndLastMsgOffsetFromFooter( const CORE::CDynamicBuffer& source , 
                                                            UInt32 currentSourceOffset         , 
                                                            UInt32& indexSize                  ,
                                                            UInt32& firstMsgOffset             ,
                                                            UInt32& lastMsgOffset              );
    
    /**
     *  If you have a partially written container with a header but no valid 
     *  footer this function can be used to rebuild an index which you could optionally
     *  use to append a new footer at the end of the file with the discovered messages
     *  by passing the index to SerializeFooter() with hdrToFtrOrderedIndex=true
     */
    static bool IndexRebuildScan( TMsgOffsetIndex& index             ,
                                  const CORE::CDynamicBuffer& source , 
                                  UInt32& bytesRead                  ,
                                  bool hdrToFtrOrderedIndex          );

    static bool Serialize( const CPubSubMsgBinarySerializerOptions& options     ,
                           const CPubSubClientTopic::TPubSubMsgsRefVector& msgs ,
                           UInt32 currentTargetOffset                           , 
                           CORE::CDynamicBuffer& target                         , 
                           UInt32& bytesWritten                                 );

    static bool Serialize( const CPubSubMsgBinarySerializerOptions& options ,
                           const TBasicPubSubMsgVector& msgs                ,
                           UInt32 currentTargetOffset                       , 
                           CORE::CDynamicBuffer& target                     , 
                           UInt32& bytesWritten                             );

    /**
     *  Deserializes all the messages from the source buffer using the already 
     *  deserialized index. It is assumed that you already have a pre-allocated 
     *  set of IPubSubMsg based message objects passed in msgs based on the 
     *  previously deserialized footer of which you also need to pass the index
     */
    static bool Deserialize( CPubSubClientTopic::TPubSubMsgsRefVector& msgs ,
                             bool linkWherePossible                         ,
                             const TMsgOffsetIndex& index                   ,
                             const CORE::CDynamicBuffer& source             ,
                             bool& isCorrupted                              );

    static bool Deserialize( TBasicPubSubMsgVector& msgs        ,
                             bool linkWherePossible             ,
                             const TMsgOffsetIndex& index       ,
                             const CORE::CDynamicBuffer& source ,
                             bool& isCorrupted                  ,
                             bool bestEffortIsOk                );


    /**
     *  Same as the more basic variant of but adds optional index (footer) deserialization
     *  with auto index rebuild and the same for deserializing the messages themselves 
     *  if corruption is detected
     */
    static bool DeserializeWithRebuild( TBasicPubSubMsgVector& msgs  ,
                                        bool linkWherePossible       ,
                                        TMsgOffsetIndex& index       ,
                                        CORE::CDynamicBuffer& source ,
                                        bool& isCorrupted            ,
                                        bool bestEffortIsOk          );

    /**
     *  Same as the other version of DeserializeWithRebuild except you dont need to pass
     *  index storage if you dont care about retaining/using that information
     */
    static bool DeserializeWithRebuild( TBasicPubSubMsgVector& msgs  ,
                                        bool linkWherePossible       ,
                                        CORE::CDynamicBuffer& source ,
                                        bool& isCorrupted            ,
                                        bool bestEffortIsOk          );

    /**
     *  If you only need to read a specific message such as the last message this is a
     *  useful helper function to do so. Please do not use this function to iterate all
     *  messages as it would be rather inefficient, code your own iteration for that
     *
     *  Note that if isCorrupted is true you may wish to think about using IndexRebuildScan()
     *  to effect repairs
     */
    static bool DeserializeMsgAtIndex( CIPubSubMsg& msg                   ,
                                       bool linkWherePossible             ,
                                       const CORE::CDynamicBuffer& source ,
                                       UInt32 msgIndex                    ,
                                       bool fromStart                     ,
                                       bool& isCorrupted                  );

    static bool DeserializeFirstAndLastMsgDateTime( CORE::CDateTime& firstMsgDt        ,
                                                    CORE::CDateTime& lastMsgDt         ,
                                                    const CORE::CDynamicBuffer& source ,
                                                    bool& isSupported                  ,
                                                    bool& isCorrupted                  );
    
    private:

    template < class MsgCollectionType >
    static bool SerializeMsgCollection( const CPubSubMsgBinarySerializerOptions& options ,
                                        const MsgCollectionType& msgs                    ,
                                        UInt32 currentTargetOffset                       , 
                                        CORE::CDynamicBuffer& target                     , 
                                        UInt32& bytesWritten                             );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H ? */
