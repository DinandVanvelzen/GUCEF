/*
 *  GucefLogService: Independent logging service for GUCEF
 *
 *  Copyright (C) 2002 - 2009.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_MFILE_H
#include "MFILE.h"
#define GUCEF_CORE_MFILE_H
#endif /* GUCEF_CORE_MFILE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const bool defaultShowConsoleState = true;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SBlockMatch;
typedef struct SBlockMatch TBlockMatch;

struct SRepeatValueInfo
{
    unsigned char repeatValue;
    unsigned long sequenceLength;
    unsigned long offsetInBlock;
};
typedef struct SRepeatValueInfo TRepeatValueInfo;

struct SBlockMatch
{
    size_t sizeOfBlock;
    unsigned long offsetInFile;
    unsigned long subBlockCount;
};

struct SSourceBlock
{
    TBlockMatch blockMatch;
    TRepeatValueInfo repeatValueInfo;
};
typedef struct SSourceBlock TSourceBlock;

typedef std::vector< TBlockMatch > TBlockMatchVector;
typedef std::map< unsigned long, TBlockMatch > TBlockMatchMap;

struct SBlockMatchCombo
{
    TSourceBlock sourceBlock;
    TBlockMatchVector matchedBlocks;
};
typedef struct SBlockMatchCombo TBlockMatchCombo;

typedef std::map< unsigned long, TBlockMatchCombo > TBlockMatchComboMap;

struct SSamsungDeltaHeader
{
    CORE::UInt16 field1[ 3 ];
    CORE::UInt32 field2;
    CORE::UInt32 field3;

    CORE::UInt8 field4[ 6 ];

};
typedef struct SSamsungDeltaHeader TSamsungDeltaHeader;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
ParseParams( const int argc                 ,
             char* argv[]                   ,
             CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    keyValueList.DeleteAll();
    GUCEF::CORE::CString argString;
    if ( argc > 0 )
    {
        argString = *argv;

        // Combine the argument strings back into a single string because we don't want to use
        // a space as the seperator
        for ( int i=1; i<argc; ++i )
        {
            argString += ' ' + CORE::CString( argv[ i ] );
        }

        // Parse the param list based on the ' symbol
        keyValueList.SetMultiple( argString, '*' );
    }
}

/*-------------------------------------------------------------------------*/

void
FindIdenticalBlock( CORE::MFILE* file                   ,
                    void* data                          ,
                    unsigned int blockSize              ,
                    TBlockMatchVector& blockMatchVector ,
                    bool preciseMatching                )
{GUCEF_TRACE;

    void* fileBlock = new unsigned char[ blockSize ];
    memset( fileBlock, 0, blockSize );
    size_t actuallyRead = 0;

    mfseek( file, 0, SEEK_SET );
    while ( 0 == CORE::mfeof( file ) )
    {
        actuallyRead = mfread( fileBlock, 1, blockSize, file );
        if ( actuallyRead > 0 )
        {
            if ( 0 == memcmp( data, fileBlock, actuallyRead ) )
            {
                TBlockMatch blockMatch;
                blockMatch.offsetInFile = CORE::mftell( file )-actuallyRead;
                blockMatch.sizeOfBlock = actuallyRead;
                blockMatch.subBlockCount = 1;
                blockMatchVector.push_back( blockMatch );

                //GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Found matching block at offset " + CORE::UInt32ToString( blockMatch.offsetInFile ) );
            }
        }
        else
        {
            break;
        }

        memset( fileBlock, 0, blockSize );

        if ( preciseMatching )
        {
            // Dont just jump at fixed block sized but shift the
            // fixed block 1 byte per iteration. This of course takes a lot
            // longer but it can better identify matches
            CORE::UInt32 pos = CORE::mftell( file );
            pos -= ( actuallyRead - 1);
            CORE::mfsetpos( file, pos );
        }
    }

    delete []fileBlock;
    fileBlock = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
MergeAdjecentBlockMatches( const TBlockMatchVector& blockMatches ,
                           TBlockMatchVector& mergedBlockMatches )
{GUCEF_TRACE;

    TBlockMatchVector::const_iterator i = blockMatches.begin();
    while ( i != blockMatches.end() )
    {
        const TBlockMatch& matchedBlock = (*i);
        TBlockMatchVector::const_iterator n = i;
        ++n;

        TBlockMatch mergedBlock = matchedBlock;

        while ( n != blockMatches.end() )
        {
            const TBlockMatch& matchedBlock2 = (*n);
            if ( mergedBlock.offsetInFile + mergedBlock.sizeOfBlock == matchedBlock2.offsetInFile )
            {
                // blocks are adjecent
                mergedBlock.sizeOfBlock += matchedBlock2.sizeOfBlock;
                ++mergedBlock.subBlockCount;
                ++n;
            }
            else
            {
                // end of adjecency
                i = n;
                break;
            }
        }
        mergedBlockMatches.push_back( mergedBlock );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
FindLongestRepeatingSequence( void* testBuffer       ,
                              size_t bufferSize      ,
                              TRepeatValueInfo& info )
{GUCEF_TRACE;

    info.offsetInBlock = 0;
    info.repeatValue = 0;
    info.sequenceLength = 0;

    unsigned char* byteBuffer = (unsigned char*) testBuffer;

    CORE::UInt32 offset = 0;
    CORE::UInt32 longestSequence = 0;
    CORE::UInt32 repeatValue = 0;

    while ( offset < bufferSize )
    {
        unsigned char testValue = byteBuffer[ offset ];
        CORE::UInt32 thisStartOffset = offset;

        ++offset;
        while ( ( offset < bufferSize )             &&
                ( testValue == byteBuffer[ offset ] ) )
        {
            ++offset;
        }

        CORE::UInt32 sequenceLength = offset - thisStartOffset;
        if ( longestSequence < sequenceLength )
        {
            info.sequenceLength = sequenceLength;
            info.repeatValue = testValue;
            info.offsetInBlock = thisStartOffset;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
FindIdenticalBlocks( CORE::MFILE* file1                      ,
                     CORE::MFILE* file2                      ,
                     TBlockMatchComboMap& blockMatchComboMap ,
                     const size_t testBlockSizeInBytes       ,
                     bool preciseMatching                    )
{GUCEF_TRACE;

    void* prevTestBuffer = new char[ testBlockSizeInBytes ];
    size_t prevOffsetInSrcFile = 0;

    void* testBuffer = new char[ testBlockSizeInBytes ];
    memset( testBuffer, 0, testBlockSizeInBytes );
    memset( prevTestBuffer, 0, testBlockSizeInBytes );
              //int a=0;
    size_t offsetInSrcFile = 0;
    while ( 0 == CORE::mfeof( file1 ) )
    {           //  if ( a>200)break;++a;
        size_t actuallyRead = CORE::mfread( testBuffer           ,
                                            1                    ,
                                            testBlockSizeInBytes ,
                                            file1                );

        // only check this after the first cycle
        bool performMatchingWithOtherFile = true;
        if ( 0 != offsetInSrcFile )
        {
            // to prevent lengthy cycling trough padding data check
            // if this block is the same as the last block
            if ( 0 == memcmp( prevTestBuffer, testBuffer, actuallyRead ) )
            {
                // Check to see if matched where added for the previous block which is aparently
                // identical to this block
                TBlockMatchComboMap::iterator n = blockMatchComboMap.find( prevOffsetInSrcFile );
                if ( n != blockMatchComboMap.end() )
                {
                    TBlockMatch& prevSourceBlock = (*n).second.sourceBlock.blockMatch;

                    // check to make sure this block is adjecent with the last block
                    if ( prevSourceBlock.offsetInFile + prevSourceBlock.sizeOfBlock == offsetInSrcFile )
                    {
                        // Source blocks are identical and adjecent
                        // just grow the previous source block to include this block
                        prevSourceBlock.sizeOfBlock += actuallyRead;
                        ++prevSourceBlock.subBlockCount;
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Source block at offset " + CORE::UInt32ToString( offsetInSrcFile ) +
                            " matches the preceding block, we will grow that block to include this block, skipping matching block search."
                            "The new block is " + CORE::UInt32ToString( prevSourceBlock.sizeOfBlock ) + " bytes in size" );
                        performMatchingWithOtherFile = false;
                    }
                    else
                    {
                        // Source blocks are identical but not adjecent
                        // Add the block into the list with the same matches as the preceding block
                        // no need to do the same matching work twice
                        TBlockMatchCombo newCombo;
                        newCombo.sourceBlock.blockMatch.offsetInFile = offsetInSrcFile;
                        newCombo.sourceBlock.blockMatch.sizeOfBlock = actuallyRead;
                        newCombo.sourceBlock.blockMatch.subBlockCount = 1;
                        newCombo.sourceBlock.repeatValueInfo = (*n).second.sourceBlock.repeatValueInfo;
                        newCombo.matchedBlocks = (*n).second.matchedBlocks;
                        blockMatchComboMap[ offsetInSrcFile ] = newCombo;
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Source block at offset " + CORE::UInt32ToString( offsetInSrcFile ) + " matches the preceding block but we dont , we will grow that block to include this block, skipping matching block search" );
                        performMatchingWithOtherFile = false;
                    }
                }
                else
                {
                    // blocks are identical but the last block did not have any matches
                    // we wont find any matches for this block either then
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Source block at offset " + CORE::UInt32ToString( offsetInSrcFile ) + " matches a previous block for which no match was found, skipping matching block search" );
                    performMatchingWithOtherFile = false;
                }
            }
        }
        memcpy( prevTestBuffer, testBuffer, actuallyRead );
        prevOffsetInSrcFile = offsetInSrcFile;

        if ( performMatchingWithOtherFile )
        {
            TRepeatValueInfo repeatInfo;
            FindLongestRepeatingSequence( testBuffer   ,
                                          actuallyRead ,
                                          repeatInfo   );

            if ( 0 != repeatInfo.sequenceLength )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "The longest repeating sequence in this block is " +
                       CORE::UInt32ToString( repeatInfo.sequenceLength ) + " with value " + CORE::UInt8ToString( repeatInfo.repeatValue ) +
                       " starting at offset " + CORE::UInt32ToString( repeatInfo.offsetInBlock ) + " in the block" );
            }
            TBlockMatchVector blockMatches;
            FindIdenticalBlock( file2           ,
                                testBuffer      ,
                                actuallyRead    ,
                                blockMatches    ,
                                preciseMatching );

            if ( !blockMatches.empty() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Found " + CORE::UInt32ToString( blockMatches.size() ) + " matching blocks for block at offset " + CORE::UInt32ToString( offsetInSrcFile ) );

                TBlockMatchVector mergedBlockMatches;
                MergeAdjecentBlockMatches( blockMatches, mergedBlockMatches );
                blockMatches.clear();

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "The matching blocks where merged where adjecent resulting in " + CORE::UInt32ToString( mergedBlockMatches.size() ) + " merged matching blocks" );

                TBlockMatchCombo newCombo;
                newCombo.sourceBlock.blockMatch.offsetInFile = offsetInSrcFile;
                newCombo.sourceBlock.blockMatch.sizeOfBlock = actuallyRead;
                newCombo.sourceBlock.blockMatch.subBlockCount = 1;
                newCombo.sourceBlock.repeatValueInfo = repeatInfo;
                newCombo.matchedBlocks = mergedBlockMatches;
                blockMatchComboMap[ offsetInSrcFile ] = newCombo;
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Could not find a matching block for block at offset " + CORE::UInt32ToString( offsetInSrcFile ) );
            }
        }
        offsetInSrcFile += actuallyRead;
    }

    delete []prevTestBuffer;
    prevTestBuffer = GUCEF_NULL;
    delete [](char*)testBuffer;
    testBuffer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
FindIdenticalBlocks( const CORE::CString& file1         ,
                     const CORE::CString& file2         ,
                     TBlockMatchComboMap& blockMatchMap ,
                     const size_t testBlockSizeInBytes  ,
                     bool preciseMatching               )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Starting search for identical blocks between file1 (" + file1 + ") and file 2 (" + file2 + ")" );

    CORE::CDynamicBuffer file1Buffer;
    if ( !file1Buffer.LoadContentFromFile( file1 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + file1 );
        return;
    }

    CORE::CDynamicBuffer file2Buffer;
    if ( !file2Buffer.LoadContentFromFile( file2 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + file2 );
        return;
    }

    CORE::MFILE* file1Ptr = CORE::mfcreate( file1Buffer.GetBufferPtr(), file1Buffer.GetDataSize() );
    CORE::MFILE* file2Ptr = CORE::mfcreate( file2Buffer.GetBufferPtr(), file2Buffer.GetDataSize() );

    if ( GUCEF_NULL != file1Ptr && GUCEF_NULL != file2Ptr )
    {
        FindIdenticalBlocks( file1Ptr             ,
                             file2Ptr             ,
                             blockMatchMap        ,
                             testBlockSizeInBytes ,
                             preciseMatching      );
    }
    if ( GUCEF_NULL != file1Ptr )
    {
        CORE::mfdestroy( file1Ptr );
    }
    if ( GUCEF_NULL != file2Ptr )
    {
        CORE::mfdestroy( file2Ptr );
    }
}

/*-------------------------------------------------------------------------*/

void
WriteBlockMatchesAsTextFile( const TBlockMatchComboMap& blockMatchComboMap ,
                             const CORE::CString& filename                 )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Writing matched blocks to text file at " + filename );

    FILE* fileOut = fopen( filename.C_String(), "w" );
    if ( GUCEF_NULL == fileOut )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to write matched blocks to text file at " + filename );
        return;
    }

    CORE::CString textFileContent;
    TBlockMatchComboMap::const_iterator i = blockMatchComboMap.begin();
    while ( i != blockMatchComboMap.end() )
    {
        const TRepeatValueInfo& repeatValueInfo = (*i).second.sourceBlock.repeatValueInfo;
        const TBlockMatch& sourceBlock = (*i).second.sourceBlock.blockMatch;
        const TBlockMatchVector& blockMatchVector = (*i).second.matchedBlocks;

        textFileContent =

        "\n\n#Source block:\n"
        "offset: " + CORE::UInt32ToString( sourceBlock.offsetInFile ) + "\n"
        "block size: " + CORE::UInt32ToString( sourceBlock.sizeOfBlock ) + "\n"
        "sub block count: " + CORE::UInt32ToString( sourceBlock.subBlockCount ) + "\n"
        "longest repeat value sequence: value=" + CORE::UInt32ToString( repeatValueInfo.repeatValue ) + " length="
            + CORE::UInt32ToString( repeatValueInfo.sequenceLength ) + " offset=" + CORE::UInt32ToString( repeatValueInfo.offsetInBlock ) + "\n"
        "#Block matches: (match count=" + CORE::UInt32ToString( blockMatchVector.size() ) + ")\n";

        fwrite( textFileContent.C_String(), 1, textFileContent.Length(), fileOut );

        int m = 1;
        TBlockMatchVector::const_iterator n = blockMatchVector.begin();
        while ( n != blockMatchVector.end() )
        {
            const TBlockMatch& matchedBlock = (*n);

            textFileContent =

            "-- Match " + CORE::Int32ToString( m ) + "\n"
            "offset: " + CORE::UInt32ToString( matchedBlock.offsetInFile ) + "\n"
            "block size: " + CORE::UInt32ToString( matchedBlock.sizeOfBlock ) + "\n"
            "sub block count: " + CORE::UInt32ToString( matchedBlock.subBlockCount ) + "\n";

            fwrite( textFileContent.C_String(), 1, textFileContent.Length(), fileOut );

            ++n;++m;
        }
        ++i;
    }
    fclose( fileOut );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully wrote matched blocks to text file at " + filename );
}

/*-------------------------------------------------------------------------*/

bool
AreBlocksOverlapping( const TBlockMatch& blockA  ,
                      const TBlockMatch& blockB  ,
                      TBlockMatch& combinedBlock )
{GUCEF_TRACE;

    if ( ( blockA.offsetInFile >= blockB.offsetInFile )                                           &&
         ( blockA.offsetInFile + blockA.sizeOfBlock >= blockB.offsetInFile + blockB.sizeOfBlock ) &&
         ( blockA.offsetInFile <= blockB.offsetInFile + blockB.sizeOfBlock )                       )
    {
        // Blocks overlap as:
        //  A
        //    B
        combinedBlock.offsetInFile = blockB.offsetInFile;
        combinedBlock.sizeOfBlock = ( blockA.offsetInFile + blockA.sizeOfBlock ) - combinedBlock.offsetInFile;
        combinedBlock.subBlockCount = 1;
        return true;
    }
    if ( ( blockA.offsetInFile <= blockB.offsetInFile )                                           &&
         ( blockA.offsetInFile + blockA.sizeOfBlock <= blockB.offsetInFile + blockB.sizeOfBlock ) &&
         ( blockB.offsetInFile <= blockA.offsetInFile + blockA.sizeOfBlock )                       )
    {
        // Blocks overlap as:
        //    B
        //  A
        combinedBlock.offsetInFile = blockA.offsetInFile;
        combinedBlock.sizeOfBlock = ( blockB.offsetInFile + blockB.sizeOfBlock ) - combinedBlock.offsetInFile;
        combinedBlock.subBlockCount = 1;
        return true;
    }
    if ( ( blockA.offsetInFile <= blockB.offsetInFile )                                          &&
         ( blockA.offsetInFile + blockA.sizeOfBlock >= blockB.offsetInFile + blockB.sizeOfBlock ) )
    {
        // Block A encompasses B
        combinedBlock = blockA;
        combinedBlock.subBlockCount = 1;
        return true;
    }
    if ( ( blockA.offsetInFile >= blockB.offsetInFile )                                          &&
         ( blockA.offsetInFile + blockA.sizeOfBlock <= blockB.offsetInFile + blockB.sizeOfBlock ) )
    {
        // Block B encompasses A
        combinedBlock = blockB;
        combinedBlock.subBlockCount = 1;
        return true;
    }
    else return false;
}

/*-------------------------------------------------------------------------*/

void
MergeBlockMatches( TBlockMatchMap& blocks                   ,
                   const TBlockMatchVector& blocksToMergeIn )
{GUCEF_TRACE;

    TBlockMatchVector::const_iterator i = blocksToMergeIn.begin();
    while ( i != blocksToMergeIn.end() )
    {
        const TBlockMatch& blockToMergeIn = (*i);
        bool blockNeedsToBeAdded = true;

        // We will have to look for matches
        TBlockMatchMap::iterator n = blocks.begin();
        while ( n != blocks.end() )
        {
            TBlockMatch& matchedBlock = (*n).second;

            TBlockMatch combinedBlock;
            if ( AreBlocksOverlapping( matchedBlock, blockToMergeIn, combinedBlock ) )
            {
                // Overwrite with the combined block thus including the new block
                (*n).second = combinedBlock;
                blockNeedsToBeAdded = false;
                break;
            }
            ++n;
        }

        if ( blockNeedsToBeAdded )
        {
            blocks[ blockToMergeIn.offsetInFile ] = blockToMergeIn;
        }

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
MergeAllTargetBlocks( const TBlockMatchComboMap& blockMatchComboMap ,
                      TBlockMatchMap& otherBlocks                   )
{GUCEF_TRACE;

    TBlockMatchComboMap::const_iterator i = blockMatchComboMap.begin();
    while ( i != blockMatchComboMap.end() )
    {
        MergeBlockMatches( otherBlocks, (*i).second.matchedBlocks );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
DetermineMatchGaps( const TBlockMatchComboMap& blockMatchComboMap ,
                    TBlockMatchVector& unmatchedSourceBlocks      ,
                    TBlockMatchVector& unmatchedOtherBlocks       )
{GUCEF_TRACE;

    TBlockMatchComboMap::const_iterator i = blockMatchComboMap.begin();
    while ( i != blockMatchComboMap.end() )
    {
        const TBlockMatch& prevSourceBlock = (*i).second.sourceBlock.blockMatch;
        ++i;
        if ( i == blockMatchComboMap.end() ) break;
        const TBlockMatch& sourceBlock = (*i).second.sourceBlock.blockMatch;

        if ( prevSourceBlock.offsetInFile + prevSourceBlock.sizeOfBlock != sourceBlock.offsetInFile )
        {
            // We are missing a segment which aparently could not be matched
            // This is an indicator that parts of the data from these blocks could be replaced/erased
            if ( prevSourceBlock.offsetInFile + prevSourceBlock.sizeOfBlock < sourceBlock.offsetInFile )
            {
                TBlockMatch unmatchedBlock;
                unmatchedBlock.offsetInFile = prevSourceBlock.offsetInFile + prevSourceBlock.sizeOfBlock;
                unmatchedBlock.sizeOfBlock = sourceBlock.offsetInFile - unmatchedBlock.offsetInFile;
                unmatchedBlock.subBlockCount = 1;

                unmatchedSourceBlocks.push_back( unmatchedBlock );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Error while determining source block match gaps, the blocks are not properly merged and/or ordered before entering this phase" );
            }
        }
    }

    // Now we merge all the matched blocks in the target file so we can easily
    // locate gaps
    TBlockMatchMap otherBlocks;
    MergeAllTargetBlocks( blockMatchComboMap, otherBlocks );

    // Locate the gaps in our list of matched target blocks
    TBlockMatchMap::iterator n = otherBlocks.begin();
    while ( n != otherBlocks.end() )
    {
        const TBlockMatch& prevBlock = (*n).second;
        ++n;
        if ( n == otherBlocks.end() ) break;
        const TBlockMatch& block = (*n).second;

        if ( prevBlock.offsetInFile + prevBlock.sizeOfBlock != block.offsetInFile )
        {
            // We are missing a segment which aparently could not be matched
            // This is an indicator that parts of the data from these blocks could be replaced/erased
            if ( prevBlock.offsetInFile + prevBlock.sizeOfBlock < block.offsetInFile )
            {
                TBlockMatch unmatchedBlock;
                unmatchedBlock.offsetInFile = prevBlock.offsetInFile + prevBlock.sizeOfBlock;
                unmatchedBlock.sizeOfBlock = block.offsetInFile - unmatchedBlock.offsetInFile;
                unmatchedBlock.subBlockCount = 1;

                unmatchedOtherBlocks.push_back( unmatchedBlock );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Error while determining target block match gaps, the blocks are not properly merged and/or ordered before entering this phase" );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
ConvertBytesToHexStringLines( const void* byteBuffer  ,
                              CORE::UInt32 bufferSize ,
                              bool addSpaces          ,
                              bool addHexPrefix       )
{GUCEF_TRACE;

    CORE::CString hexString;
    CORE::UInt32 remainingBytes = bufferSize;
    CORE::UInt32 sectionSize = 0;

    do
    {
        sectionSize = remainingBytes > 16 ? 16 : remainingBytes;

        hexString += CORE::ConvertBytesToHexString( (const char*)byteBuffer + (bufferSize-remainingBytes) ,
                                                    sectionSize                                           ,
                                                    addSpaces                                             ,
                                                    addHexPrefix                                          ) + '\n';
        remainingBytes -= sectionSize;
    }
    while ( remainingBytes > 0 );

    return hexString;
}

/*-------------------------------------------------------------------------*/

void
WriteBlocksAsHexInTextFile( const TBlockMatchVector& blocks ,
                            const CORE::CString& inputFile  ,
                            const CORE::CString& outputFile )
{GUCEF_TRACE;

    CORE::CDynamicBuffer fileBuffer;
    if ( !fileBuffer.LoadContentFromFile( inputFile ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + inputFile );
        return;
    }

    FILE* outFile = fopen( outputFile.C_String(), "w" );
    if ( GUCEF_NULL == outFile )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to open text file for writing at " + outputFile );
        return;
    }

    CORE::UInt32 n=0;
    CORE::UInt32 totalBlockBytes = 0;
    CORE::CString content;
    TBlockMatchVector::const_iterator i = blocks.begin();
    while ( i != blocks.end() )
    {
        const TBlockMatch& blockmatch = (*i);

        content =
        "\n\n#block " + CORE::UInt32ToString( n ) + ":\n"
        "offset: " + CORE::UInt32ToString( blockmatch.offsetInFile ) + "\n"
        "block size: " + CORE::UInt32ToString( blockmatch.sizeOfBlock ) + "\n"
        "sub block count: " + CORE::UInt32ToString( blockmatch.subBlockCount ) + "\n"
        "hex:\n" + ConvertBytesToHexStringLines( fileBuffer.GetConstBufferPtr( blockmatch.offsetInFile ) ,
                                                 blockmatch.sizeOfBlock                                  ,
                                                 true                                                    ,
                                                 false                                                   );

        totalBlockBytes += blockmatch.sizeOfBlock;
        fwrite( content.C_String(), 1, content.Length(), outFile );

        ++i;++n;
    }

    content = "\n\nTotal bytes for all blocks: " + CORE::UInt32ToString( totalBlockBytes );
    fwrite( content.C_String(), 1, content.Length(), outFile );

    fclose( outFile );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully wrote block data as hex to text file at " + outputFile );
}

/*-------------------------------------------------------------------------*/

void
WriteBlockMatchGapsAsTextFile( const TBlockMatchVector& unmatchedBlocks ,
                               const CORE::CString& outputFile          )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Writing match gaps to text file at " + outputFile );

    FILE* outFile = fopen( outputFile.C_String(), "w" );
    if ( GUCEF_NULL == outFile )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to write unmatched block gaps to text file at " + outputFile );
        return;
    }

    CORE::UInt32 n = 0;
    CORE::UInt32 totalBlockBytes = 0;
    CORE::CString content;
    TBlockMatchVector::const_iterator i = unmatchedBlocks.begin();
    while ( i != unmatchedBlocks.end() )
    {
        const TBlockMatch& blockmatch = (*i);

        content =
        "\n\n#Unmatched block " + CORE::UInt32ToString( n ) + ":\n"
        "offset: " + CORE::UInt32ToString( blockmatch.offsetInFile ) + "\n"
        "block size: " + CORE::UInt32ToString( blockmatch.sizeOfBlock ) + "\n"
        "sub block count: " + CORE::UInt32ToString( blockmatch.subBlockCount ) + "\n";

        totalBlockBytes += blockmatch.sizeOfBlock;

        fwrite( content.C_String(), 1, content.Length(), outFile );

        ++i;++n;
    }

    content = "\n\nTotal bytes for all blocks: " + CORE::UInt32ToString( totalBlockBytes );
    fwrite( content.C_String(), 1, content.Length(), outFile );

    fclose( outFile );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully wrote unmatched block gaps to text file at " + outputFile );
}

/*-------------------------------------------------------------------------*/

void
MatchSpecificBlock( void* buffer                     ,
                    unsigned long bufferSize         ,
                    CORE::MFILE* searchFile          ,
                    TBlockMatchVector& matchedBlocks )
{GUCEF_TRACE;

    CORE::CDynamicBuffer searchBuffer( bufferSize, true );

    CORE::UInt32 offsetInFile = 0;
    while ( 0 == CORE::mfeof( searchFile ) )
    {
        CORE::mfsetpos( searchFile, offsetInFile );
        CORE::UInt32 actuallyRead = CORE::mfread( searchBuffer.GetBufferPtr(), 1, bufferSize, searchFile );
        if ( 0 == memcmp( searchBuffer.GetBufferPtr(), buffer, actuallyRead ) )
        {
            TBlockMatch match;
            match.offsetInFile = offsetInFile;
            match.sizeOfBlock = actuallyRead;
            match.subBlockCount = 1;
            matchedBlocks.push_back( match );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Found matching block at offset " +
                  CORE::UInt32ToString( match.offsetInFile ) + " with size " + CORE::UInt32ToString( match.sizeOfBlock ) );
        }

        // We want to be very precise in this matching so we will move 1 byte
        // and then compare again
        ++offsetInFile;
    }
    CORE::mfsetpos( searchFile, 0 );
}

/*-------------------------------------------------------------------------*/

void
MatchSpecificBlocks( const TBlockMatchVector& blocksToMatch ,
                     CORE::MFILE* sourceFile                ,
                     CORE::MFILE* searchFile                ,
                     TBlockMatchComboMap& matches           )
{GUCEF_TRACE;

    CORE::CDynamicBuffer sourceBuffer;

    TBlockMatchVector::const_iterator i = blocksToMatch.begin();
    while ( i != blocksToMatch.end() )
    {
        const TBlockMatch& blockMatch = (*i);

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Attempting to match a specific block at offset " +
              CORE::UInt32ToString( blockMatch.offsetInFile ) + " with size " + CORE::UInt32ToString( blockMatch.sizeOfBlock ) );

        sourceBuffer.SetDataSize( blockMatch.sizeOfBlock );
        CORE::mfseek( sourceFile, blockMatch.offsetInFile, SEEK_SET );
        CORE::mfread( sourceBuffer.GetBufferPtr(), blockMatch.sizeOfBlock, 1, sourceFile );

        TBlockMatchVector matchedBlocks;
        MatchSpecificBlock( sourceBuffer.GetBufferPtr() ,
                            blockMatch.sizeOfBlock      ,
                            searchFile                  ,
                            matchedBlocks               );

        if ( !matchedBlocks.empty() )
        {
            TBlockMatchCombo& matchCombo = matches[ blockMatch.offsetInFile ];
            matchCombo.matchedBlocks = matchedBlocks;
            FindLongestRepeatingSequence( sourceBuffer.GetBufferPtr()            ,
                                          blockMatch.sizeOfBlock                 ,
                                          matchCombo.sourceBlock.repeatValueInfo );
            matchCombo.sourceBlock.blockMatch = blockMatch;
        }

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
MatchSpecificBlocks( const TBlockMatchVector& blocksToMatch ,
                     const CORE::CString& sourceFilePath    ,
                     const CORE::CString& searchFilePath    ,
                     TBlockMatchComboMap& matches           )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Starting search for specific blocks between file1 (" + sourceFilePath + ") and file 2 (" + searchFilePath + ")" );

    CORE::CDynamicBuffer sourceFileBuffer;
    if ( !sourceFileBuffer.LoadContentFromFile( sourceFilePath ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + sourceFilePath );
        return;
    }

    CORE::CDynamicBuffer searchFileBuffer;
    if ( !searchFileBuffer.LoadContentFromFile( searchFilePath ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + searchFilePath );
        return;
    }

    CORE::MFILE* file1Ptr = CORE::mfcreate( sourceFileBuffer.GetBufferPtr(), sourceFileBuffer.GetDataSize() );
    CORE::MFILE* file2Ptr = CORE::mfcreate( searchFileBuffer.GetBufferPtr(), searchFileBuffer.GetDataSize() );

    if ( GUCEF_NULL != file1Ptr && GUCEF_NULL != file2Ptr )
    {
        MatchSpecificBlocks( blocksToMatch ,
                             file1Ptr      ,
                             file2Ptr      ,
                             matches       );
    }
    if ( GUCEF_NULL != file1Ptr )
    {
        CORE::mfdestroy( file1Ptr );
    }
    if ( GUCEF_NULL != file2Ptr )
    {
        CORE::mfdestroy( file2Ptr );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetUniqueLogFilename( const CORE::CString& logfileName )
{GUCEF_TRACE;

    CORE::UInt32 i=0;
    CORE::CString testPath;
    do
    {
        testPath = logfileName;
        CORE::AppendToPath( testPath, "FileDiff_Log(" + CORE::UInt32ToString( i ) + ").txt" );
        ++i;
    }
    while ( CORE::FileExists( testPath ) );

    return testPath;
}

/*-------------------------------------------------------------------------*/

void
LookForValues( const CORE::CString& filename )
{GUCEF_TRACE;

    CORE::Int32 testVals[ 6 ] = { 125, 1484, 6901, 7797, 8684, 8838 };

    CORE::CDynamicBuffer fileBuffer;
    if ( !fileBuffer.LoadContentFromFile( filename ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + filename );
        return;
    }

    CORE::Int16 testValue = 0;
    for ( CORE::UInt32 i=0; i<fileBuffer.GetDataSize(); ++i )
    {
        if ( i+sizeof(testValue) <= fileBuffer.GetDataSize() )
        {
            testValue = fileBuffer.AsConstType< CORE::Int16 >( i );
            for ( CORE::UInt32 n=0; n<6; ++n )
            {
                if ( testVals[ n ] == testValue )
                {
                 int a=0;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
GenerateDelta( CORE::CDynamicBuffer& file1Buffer  ,
               CORE::CDynamicBuffer& file2Buffer  ,
               const CORE::CString& deltaFilename )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

void
ReadSamsungDeltaHeader( TSamsungDeltaHeader& header   ,
                        const CORE::CString& filename )
{
    CORE::CDynamicBuffer fileBuffer;
    if ( !fileBuffer.LoadContentFromFile( filename ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + filename );
        return;
    }

    const char* dataBuffer = (const char*) fileBuffer.GetConstBufferPtr( 0 );

    memcpy( &header.field1[ 0 ], dataBuffer + 0, 2 );
    memcpy( &header.field1[ 1 ], dataBuffer + 2, 2 );
    memcpy( &header.field1[ 2 ], dataBuffer + 4, 2 );
    memcpy( &header.field2,      dataBuffer + 6, 4 );
    memcpy( &header.field3,      dataBuffer + 10, 4 );
    memcpy( &header.field3,      dataBuffer + 10, 4 );

    memcpy( &header.field4[ 0 ], dataBuffer + 96, 1 );
    memcpy( &header.field4[ 1 ], dataBuffer + 97, 1 );
    memcpy( &header.field4[ 2 ], dataBuffer + 98, 1 );
    memcpy( &header.field4[ 3 ], dataBuffer + 99, 1 );
    memcpy( &header.field4[ 4 ], dataBuffer + 100, 1 );
    memcpy( &header.field4[ 5 ], dataBuffer + 101, 1 );


}

/*-------------------------------------------------------------------------*/

void
SimpleBlockDiff( TBlockMatchVector& differentBlocks ,
                 const CORE::CString& filename1     ,
                 const CORE::CString& filename2     )
{GUCEF_TRACE;

    CORE::CDynamicBuffer file1Buffer;
    if ( !file1Buffer.LoadContentFromFile( filename1 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + filename1 );
        return;
    }
    CORE::CDynamicBuffer file2Buffer;
    if ( !file2Buffer.LoadContentFromFile( filename2 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load file: " + filename2 );
        return;
    }

    TBlockMatch mismatchedBlock;
    bool matching = true;
    for ( CORE::UInt32 i=0; i<file1Buffer.GetDataSize(); ++i )
    {
        CORE::UInt8 file1Byte = file1Buffer.AsConstType< CORE::Int8 >( i );
        CORE::UInt8 file2Byte = file2Buffer.AsConstType< CORE::Int8 >( i );

        if ( file1Byte != file2Byte && matching )
        {
            matching = false;
            mismatchedBlock.offsetInFile = i;
        }
        else
        if ( file1Byte == file2Byte && !matching )
        {
            matching = true;
            mismatchedBlock.sizeOfBlock = i - mismatchedBlock.offsetInFile;
            mismatchedBlock.subBlockCount = 1;
            differentBlocks.push_back( mismatchedBlock );
        }
    }
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "This tool was compiled on: " __DATE__ " @ " __TIME__ );

    #ifdef GUCEF_CALLSTACK_TRACING
    CORE::GUCEF_LogStackTo( "GucefLogServiceApp_Callstack.cvs" );
    CORE::GUCEF_SetStackLoggingInCvsFormat( 1 );
    CORE::GUCEF_SetStackLogging( 1 );
    #endif /* GUCEF_CALLSTACK_TRACING ? */

    // Parse the application parameters
    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );

    CORE::CString outputDir = keyValueList.GetValueAlways( "outputDir" );
    if ( outputDir.IsNULLOrEmpty() )
    {
        outputDir = CORE::RelativePath( "$MODULEDIR$" );
    }

    CORE::CString logFilename;
    if ( outputDir.IsNULLOrEmpty() )
    {
        logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
    }
    else
    {
        logFilename = outputDir;
    }
    logFilename = GetUniqueLogFilename( logFilename );
    CORE::CFileAccess logFileAccess( logFilename, "w" );

    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    // Do we want to display the console window?
    CORE::CPlatformNativeConsoleLogger* consoleOut = GUCEF_NULL;
    bool showConsole = defaultShowConsoleState;
    if ( keyValueList.HasKey( "showConsole" ) )
    {
        showConsole = CORE::StringToBool( keyValueList.GetValue( "showConsole" ) );
    }
    if ( showConsole )
    {
        consoleOut = new CORE::CPlatformNativeConsoleLogger();
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut->GetLogger() );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileDiff: Enabled console output" );
    }

    // Flush bootstrap logging now that we attached all our loggers
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    CORE::CString file1Path = keyValueList.GetValueAlways( "file1" );
    CORE::CString file2Path = keyValueList.GetValueAlways( "file2" );

    TSamsungDeltaHeader header;
    ReadSamsungDeltaHeader( header, file1Path );

    if ( !file1Path.IsNULLOrEmpty() && !file2Path.IsNULLOrEmpty() )
    {
        //TBlockMatchVector mismatchedBlocks;
        //SimpleBlockDiff( mismatchedBlocks, file1Path, file2Path );

        //CORE::CString outputfilePath = outputDir;
        //CORE::AppendToPath( outputfilePath, "SimpleBlockDiff.txt" );
        //WriteBlockMatchGapsAsTextFile( mismatchedBlocks, outputfilePath );
        //
        //outputfilePath = outputDir;
        //CORE::AppendToPath( outputfilePath, "File1DiffBlocksHex.txt" );
        //WriteBlocksAsHexInTextFile( mismatchedBlocks, file1Path, outputfilePath );

        //outputfilePath = outputDir;
        //CORE::AppendToPath( outputfilePath, "File2DiffBlocksHex.txt" );
        //WriteBlocksAsHexInTextFile( mismatchedBlocks, file2Path, outputfilePath );

        //size_t testBlockSizeInBytes = 128;
        //CORE::CString testBlockSizeStr = keyValueList.GetValueAlways( "testBlockSizeInBytes" );
        //if ( !testBlockSizeStr.IsNULLOrEmpty() )
        //{
        //    testBlockSizeInBytes = CORE::StringToUInt32( testBlockSizeStr );
        //}

        //CORE::CString outputMatchfile = keyValueList.GetValueAlways( "outputMatchFile" );
        //if ( outputMatchfile.IsNULLOrEmpty() )
        //{
        //    outputMatchfile = outputDir;
        //    CORE::AppendToPath( outputMatchfile, "MatchedBlocks_blockSize(" + CORE::UInt32ToString( testBlockSizeInBytes ) + ").txt" );
        //}
        //
        //bool preciseMatching = false;
        //CORE::CString preciseMatchingStr = keyValueList.GetValueAlways( "preciseMatching" );
        //if ( !preciseMatchingStr.IsNULLOrEmpty() )
        //{
        //    preciseMatching = CORE::StringToBool( preciseMatchingStr );
        //}
        //
        //TBlockMatchComboMap blockMatchComboMap;
        //FindIdenticalBlocks( file1Path            ,
        //                     file2Path            ,
        //                     blockMatchComboMap   ,
        //                     testBlockSizeInBytes ,
        //                     preciseMatching      );
        //
        //WriteBlockMatchesAsTextFile( blockMatchComboMap ,
        //                             outputMatchfile    );
        //
        //TBlockMatchVector unmatchedSourceBlocks;
        //TBlockMatchVector unmatchedOtherBlocks;
        //DetermineMatchGaps( blockMatchComboMap    ,
        //                    unmatchedSourceBlocks ,
        //                    unmatchedOtherBlocks  );
        //
        //CORE::CString outputUnmatchedSourcefile = keyValueList.GetValueAlways( "outputUnmatchedSourcefile" );
        //if ( outputUnmatchedSourcefile.IsNULLOrEmpty() )
        //{
        //    outputUnmatchedSourcefile = outputDir;
        //    CORE::AppendToPath( outputUnmatchedSourcefile, "UnmatchedSourceBlocks_blockSize(" + CORE::UInt32ToString( testBlockSizeInBytes ) + ").txt" );
        //}
        //CORE::CString outputUnmatchedTargetfile = keyValueList.GetValueAlways( "outputUnmatchedTargetfile" );
        //if ( outputUnmatchedTargetfile.IsNULLOrEmpty() )
        //{
        //    outputUnmatchedTargetfile = outputDir;
        //    CORE::AppendToPath( outputUnmatchedTargetfile, "UnmatchedTargetBlocks_blockSize(" + CORE::UInt32ToString( testBlockSizeInBytes ) + ").txt" );
        //}
        //
        //WriteBlockMatchGapsAsTextFile( unmatchedSourceBlocks, outputUnmatchedSourcefile );
        //WriteBlockMatchGapsAsTextFile( unmatchedOtherBlocks, outputUnmatchedTargetfile );
        //
        //CORE::CString deltafilePath = keyValueList.GetValueAlways( "deltafile" );
        //if ( !deltafilePath.IsNULLOrEmpty() )
        //{
        //    TBlockMatchComboMap matchedSourceBlocks;
        //    MatchSpecificBlocks( unmatchedSourceBlocks ,
        //                         file1Path             ,
        //                         deltafilePath         ,
        //                         matchedSourceBlocks   );

        //    CORE::CString outputMatchedSourcefile = keyValueList.GetValueAlways( "outputMatchedSourcefileDelta" );
        //    if ( outputUnmatchedSourcefile.IsNULLOrEmpty() )
        //    {
        //        outputUnmatchedSourcefile = outputDir;
        //        CORE::AppendToPath( outputUnmatchedSourcefile, "MatchedSourceDeltaBlocks_blockSize(" + CORE::UInt32ToString( testBlockSizeInBytes ) + ").txt" );
        //    }

        //    WriteBlockMatchesAsTextFile( matchedSourceBlocks       ,
        //                                 outputUnmatchedSourcefile );
        //
        //    TBlockMatchComboMap matchedTargetBlocks;
        //    MatchSpecificBlocks( unmatchedOtherBlocks ,
        //                         file2Path            ,
        //                         deltafilePath        ,
        //                         matchedTargetBlocks  );
        //
        //    CORE::CString outputMatchedTargetfile = keyValueList.GetValueAlways( "outputMatchedTargetfileDelta" );
        //    if ( outputMatchedTargetfile.IsNULLOrEmpty() )
        //    {
        //        outputMatchedTargetfile = outputDir;
        //        CORE::AppendToPath( outputMatchedTargetfile, "MatchedTargetDeltaBlocks_blockSize(" + CORE::UInt32ToString( testBlockSizeInBytes ) + ").txt" );
        //    }
        //
        //    WriteBlockMatchesAsTextFile( matchedTargetBlocks     ,
        //                                 outputMatchedTargetfile );
        //}
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "input error, file1 and file2 params must be given" );
    }

    delete consoleOut;
    consoleOut = GUCEF_NULL;

    return 1;
}
GUCEF_OSMAIN_END

/*---------------------------------------------------------------------------*/
