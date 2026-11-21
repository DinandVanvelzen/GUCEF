/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_DRN_CDRNNAMESPACECODEC_H
#define GUCEF_DRN_CDRNNAMESPACECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEENTRY_H
#include "gucefDRN_CDRNNamespaceEntry.h"
#define GUCEF_DRN_CDRNNAMESPACEENTRY_H
#endif /* GUCEF_DRN_CDRNNAMESPACEENTRY_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACESHAREINFO_H
#include "gucefDRN_CDRNNamespaceShareInfo.h"
#define GUCEF_DRN_CDRNNAMESPACESHAREINFO_H
#endif /* GUCEF_DRN_CDRNNAMESPACESHAREINFO_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEINFO_H
#include "gucefDRN_CDRNNamespaceInfo.h"
#define GUCEF_DRN_CDRNNAMESPACEINFO_H
#endif /* GUCEF_DRN_CDRNNAMESPACEINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The wire encoding of the namespace messages (DRN_RES_NS_*), hand written on CDRNWireWriter and CDRNWireReader and
 *  shared by the serving side (CDRNResourceSessionComponent) and the requesting side (CDRNNamespaceClient). Every Read
 *  function checks what it reads and returns false for anything malformed; nothing allocates by a count before the count
 *  was checked against the bytes that are left.
 *
 *  Request bodies, after the opcode, in the order written:
 *    HELLO     requestId, maxVersion, clientCaps
 *    SHARES    requestId
 *    STAT      requestId, share, count, ( path, ifRevisionNot ) * count
 *    LIST      requestId, share, dir, cursor, maxEntries, ifRevisionNot
 *  Answer (NS_ANSWER): requestId, kind, status, message, then for a status OK (LIST also UNCHANGED, which holds only the
 *  directory revision):
 *    HELLO     version, capabilities, maxStatPaths, maxPageEntries, maxAnswerBytes, listingLifetimeMs
 *    SHARES    count, ( name, capabilities, flags, maxStatPaths, maxPageEntries ) * count
 *    STAT      count, ( status, entry without name when OK ) * count, in the order of the request
 *    LIST      dirRevision, nextCursor, count, entry * count
 *  Entry: [name, only in LIST], kind u8, size varint64, modifiedUtcMs varint64, flags varint, revision string,
 *  [sha256 32 bytes when the flag is set].
 */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceCodec
{
    public:

    /** A string as its bytes with a length in front: names of a file system are carried as they are, whatever bytes they hold */
    static void WriteText( CDRNWireWriter& writer, const CORE::CString& text );
    static bool ReadText( CDRNWireReader& reader, CORE::CString& text );

    // requests

    static void WriteHelloRequest( CDRNWireWriter& writer, UInt32 requestId, UInt32 maxVersion, UInt32 clientCaps );
    static bool ReadHelloRequest( CDRNWireReader& reader, UInt32& requestId, UInt32& maxVersion, UInt32& clientCaps );

    static void WriteSharesRequest( CDRNWireWriter& writer, UInt32 requestId );
    static bool ReadSharesRequest( CDRNWireReader& reader, UInt32& requestId );

    static void WriteStatRequestHeader( CDRNWireWriter& writer, UInt32 requestId, const CORE::CString& share, UInt32 count );
    static void WriteStatRequestItem( CDRNWireWriter& writer, const CORE::CString& path, const CORE::CString& ifRevisionNot );
    static bool ReadStatRequestHeader( CDRNWireReader& reader, UInt32& requestId, CORE::CString& share, UInt32& count );
    static bool ReadStatRequestItem( CDRNWireReader& reader, CORE::CString& path, CORE::CString& ifRevisionNot );

    static void WriteListRequest( CDRNWireWriter& writer, UInt32 requestId, const CORE::CString& share, const CORE::CString& dir,
                                  const CORE::CString& cursor, UInt32 maxEntries, const CORE::CString& ifRevisionNot );
    static bool ReadListRequest( CDRNWireReader& reader, UInt32& requestId, CORE::CString& share, CORE::CString& dir,
                                 CORE::CString& cursor, UInt32& maxEntries, CORE::CString& ifRevisionNot );

    /** Only the request id of any request body: for answering a request whose rest is malformed */
    static bool PeekRequestId( const void* body, UInt32 size, UInt32& requestId );

    // answers

    static void WriteAnswerHeader( CDRNWireWriter& writer, UInt32 requestId, UInt32 kind, TDRNResourceStatus status, const CORE::CString& message );
    static bool ReadAnswerHeader( CDRNWireReader& reader, UInt32& requestId, UInt32& kind, TDRNResourceStatus& status, CORE::CString& message );

    static void WriteHelloBody( CDRNWireWriter& writer, const CDRNNamespaceInfo& info );
    static bool ReadHelloBody( CDRNWireReader& reader, CDRNNamespaceInfo& info );

    static void WriteShare( CDRNWireWriter& writer, const CDRNNamespaceShareInfo& share );
    static bool ReadShare( CDRNWireReader& reader, CDRNNamespaceShareInfo& share );

    /** A count that the bytes left can hold at least minItemBytes each, at most maxCount: false for anything else */
    static bool ReadCount( CDRNWireReader& reader, UInt32 minItemBytes, UInt32 maxCount, UInt32& count );

    /** An entry of a STAT answer: the status and, when OK, the entry without its name */
    static void WriteStatItem( CDRNWireWriter& writer, const CDRNNamespaceEntry& entry );
    static bool ReadStatItem( CDRNWireReader& reader, CDRNNamespaceEntry& entry );

    static void WriteListHeader( CDRNWireWriter& writer, const CORE::CString& dirRevision, const CORE::CString& nextCursor, UInt32 count );
    static bool ReadListHeader( CDRNWireReader& reader, CORE::CString& dirRevision, CORE::CString& nextCursor, UInt32& count );

    /** An entry of a LIST answer: with the name, which is a single name of the listed directory */
    static void WriteListEntry( CDRNWireWriter& writer, const CDRNNamespaceEntry& entry );
    static bool ReadListEntry( CDRNWireReader& reader, CDRNNamespaceEntry& entry );

    /** The largest number of bytes an answer entry can take: the budget check of a page */
    static UInt32 GetMaxListEntryBytes( void );

    private:

    CDRNNamespaceCodec( void );

    static void WriteEntryBody( CDRNWireWriter& writer, const CDRNNamespaceEntry& entry );
    static bool ReadEntryBody( CDRNWireReader& reader, CDRNNamespaceEntry& entry );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACECODEC_H ? */
