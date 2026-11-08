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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#include "gucefCORE_CVariantBinarySerializer.h"
#define GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CVARIANTBINARYSERIALIZER_H ? */

#include "gucefPUBSUB_CPubSubBookmarkBinarySerializer.h"

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

const CORE::CString CPubSubBookmarkBinarySerializer::MagicText = "PUBSUBBOOKMARK";
const CORE::UInt8   CPubSubBookmarkBinarySerializer::CurrentFormatVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubBookmarkBinarySerializer::CPubSubBookmarkBinarySerializer( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubBookmarkBinarySerializer::~CPubSubBookmarkBinarySerializer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool 
CPubSubBookmarkBinarySerializer::Serialize( const CPubSubBookmark& bookmark ,
                                            UInt32 currentTargetOffset      , 
                                            CORE::CDynamicBuffer& target    , 
                                            UInt32& bytesWritten            )
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

    // Write the DateTime of the bookmark as milliseconds since Unix Epoch in UTC
    // In most applications Unix Epoch is not UTC and leap seconds are not taken into account
    // However since the source is of higher fidelity we can retain accuracy as long as the same method is
    // used to deserialize as well. There is a problem with any DateTime which predates Unix epoch
    // this is not considered an issue here due to the intended timestamp style usage
    UInt64 msSinceUnixEpochInUtc = bookmark.GetBookmarkDateTime().ToUnixEpochBasedTicksInMillisecs();
    newBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(msSinceUnixEpochInUtc), &msSinceUnixEpochInUtc );
    currentTargetOffset += newBytesWritten;
    bytesWritten += newBytesWritten;
    if ( newBytesWritten != sizeof(msSinceUnixEpochInUtc) )
        return false;
        
    // Write the type of bookmark we are dealing with
    Int32 bmType = bookmark.GetBookmarkType();
    newBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(bmType), &bmType );
    bytesWritten += newBytesWritten;
    currentTargetOffset += newBytesWritten;
    if ( newBytesWritten != sizeof(bmType) )
        return false;

    // Write the bookmark data itself using the variant binary serializer
    UInt32 varBinSize = 0;
    if ( !CORE::CVariantBinarySerializer::Serialize( bookmark.GetBookmarkData(), currentTargetOffset, target, varBinSize ) )
        return false;
    currentTargetOffset += varBinSize;
    bytesWritten += varBinSize;
                 
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubBookmarkBinarySerializer::Serialize( const CPubSubBookmark& bookmark ,
                                            CORE::CDynamicBuffer& target    )
{GUCEF_TRACE;

    UInt32 bytesWritten = 0;
    return Serialize( bookmark, 0, target, bytesWritten );
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubBookmarkBinarySerializer::Deserialize( CPubSubBookmark& bookmark          ,
                                              UInt32 currentSourceOffset         ,
                                              bool linkWherePossible             ,
                                              const CORE::CDynamicBuffer& source ,
                                              bool& isCorrupted                  ,
                                              UInt32& bytesRead                  )
{GUCEF_TRACE;

    bytesRead = 0;
    isCorrupted = false;
    
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
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSubBookmarkBinarySerializer:Deserialize: magic text verification failed" );
            return false;
        }

        UInt8 formatVersion = source.ReadValue< UInt8 >( currentSourceOffset );
        bytesRead += sizeof(formatVersion);
        currentSourceOffset += sizeof(formatVersion);
        if ( CurrentFormatVersion != formatVersion )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSubBookmarkBinarySerializer:Deserialize: Incompatible format version" );
            return false;
        }

        // Read the DateTime of the bookmark which is milliseconds since Unix epoch in UTC IF
        // this class's Serialize() was used. Commonly used Unix epoch variables do NOT have a  
        // timezone associated nor does the Unix epoch offset in a lot of implementations take
        // leap seconds into account. Hence choose carefully to ensure consistency in interpretation
        UInt64 msSinceUnixEpochInUtc = source.ReadValue< UInt64 >( currentSourceOffset );
        currentSourceOffset += sizeof(msSinceUnixEpochInUtc);
        bytesRead += sizeof(msSinceUnixEpochInUtc);
        bookmark.GetBookmarkDateTime().FromUnixEpochBasedTicksInMillisecs( msSinceUnixEpochInUtc );

        Int32 bmType = source.ReadValue< Int32 >( currentSourceOffset );
        bytesRead += sizeof(bmType);
        currentSourceOffset += sizeof(bmType);
        bookmark.SetBookmarkType( (CPubSubBookmark::TBookmarkType) bmType );

        UInt32 varByteSize = 0;
        if ( !CORE::CVariantBinarySerializer::Deserialize( bookmark.GetBookmarkData(), currentSourceOffset, source, linkWherePossible, varByteSize ) )
            return false;
        currentSourceOffset += varByteSize;
        bytesRead += varByteSize;

        return true;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "PubSubBookmarkBinarySerializer:Deserialize: Exception encountered: " + CORE::ToString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubBookmarkBinarySerializer::Deserialize( CPubSubBookmark& bookmark          ,
                                              bool linkWherePossible             ,
                                              const CORE::CDynamicBuffer& source )
{GUCEF_TRACE;

    UInt32 bytesRead = 0;
    bool isCorrupted = false;
    return Deserialize( bookmark, 0, linkWherePossible, source, isCorrupted, bytesRead );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
