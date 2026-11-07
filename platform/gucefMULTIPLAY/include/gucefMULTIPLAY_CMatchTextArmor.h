/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHTEXTARMOR_H
#define GUCEF_MULTIPLAY_CMATCHTEXTARMOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum EMatchArmorResult
{
    MATCHARMOR_OK              = 0 ,
    MATCHARMOR_NO_BLOCK        = 1 ,    /**< no block of the wanted match (and revision) is in the text */
    MATCHARMOR_TOO_LARGE       = 2 ,    /**< the text, or a block in it, is above the cap */
    MATCHARMOR_BAD_HEADER      = 3 ,
    MATCHARMOR_BAD_ENCODING    = 4 ,    /**< the base64 text is not valid */
    MATCHARMOR_TRUNCATED       = 5 ,    /**< the block has no end line, or the text is shorter than the header says */
    MATCHARMOR_BAD_CHECKSUM    = 6 ,    /**< the text was damaged on its way: ask for it to be sent again */
    MATCHARMOR_BAD_HASH        = 7
};
typedef enum EMatchArmorResult TMatchArmorResult;

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchArmorResultToString( TMatchArmorResult result );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The container bytes (CMatchContainer) as plain text that survives being sent in the body of a mail:
 *
 *      -----BEGIN <label>-----
 *      Match: <match id>
 *      Revision: <number>
 *      Length: <bytes of the container>
 *      SHA-256: <hex of the container>
 *      Version: 1
 *
 *      <base64, 76 characters per line>
 *      Checksum: <CRC-24 of the container bytes, 6 hex digits>
 *      -----END <label>-----
 *
 *  The writer refuses containers above the caps (a container is at most 2 MB, the text at most 512 KB). The
 *  reader copes with what mail does to text: reply quote prefixes (any run of '>' and blanks in front of a
 *  line), trailing blanks, CRLF, LF or CR line ends, lines folded to another width, a human text before and after
 *  the block, and several blocks in one text (a thread of replies): the block of the wanted match id and revision
 *  is taken, the highest revision when no revision is wanted. A block that was damaged is never repaired: the
 *  CRC-24 and the SHA-256 decide, and the result tells the player to ask for a new copy.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchTextArmor
{
    public:

    static const UInt32 MaxEncodedTextSize = 512 * 1024;
    static const UInt32 MaxScannedTextSize = 4 * 1024 * 1024;       /**< of the whole text handed to the reader, quotes and all */
    static const UInt32 LineWidth = 76;

    /** @param label the words in the marker lines, for example "MATCH TURN" */
    explicit CMatchTextArmor( const CORE::CString& label = "MATCH TURN" );

    CMatchTextArmor( const CMatchTextArmor& src );

    ~CMatchTextArmor();

    CMatchTextArmor& operator=( const CMatchTextArmor& src );

    const CORE::CString& GetLabel( void ) const;

    /** The text for the container; false when the container or the text would be above the caps */
    bool Encode( const CORE::CString& matchId, UInt32 revision, const void* container, UInt32 containerSize, CORE::CString& text ) const;

    /**
     *  Finds the wanted block in a text and decodes it
     *
     *  @param wantedMatchId the match to take the block of, empty for any
     *  @param wantedRevision the revision, 0 for the highest that decodes
     *  @param container receives the container bytes
     *  @param foundRevision the revision of the block taken
     *  @return MATCHARMOR_OK, or why the best candidate block was refused
     */
    TMatchArmorResult Decode( const void* text                  ,
                              UInt32 textSize                   ,
                              const CORE::CString& wantedMatchId,
                              UInt32 wantedRevision             ,
                              CORE::CDynamicBuffer& container   ,
                              UInt32* foundRevision = GUCEF_NULL ) const;

    /** CRC-24 as used by OpenPGP armor */
    static UInt32 Crc24( const void* data, UInt32 size );

    private:

    CORE::CString m_label;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHTEXTARMOR_H ? */
