/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#include "gucefCORE_CTokenizer.h"

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

CTokenizer::CTokenSequence::CTokenSequence( void )
    : CTSharedObjCreator< CTokenSequence, MT::CNoLock >( this )
    , tokens()
    , sequences()
    , ordering()
    , sequenceOpenToken()
    , sequenceCloseToken()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTokenizer::CTokenSequence::CTokenSequence( const CTokenSequence& src )
    : CTSharedObjCreator< CTokenSequence, MT::CNoLock >( this )
    , tokens( src.tokens )
    , sequences( src.sequences )
    , ordering( src.ordering )
    , sequenceOpenToken( src.sequenceOpenToken )
    , sequenceCloseToken( src.sequenceCloseToken )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTokenizer::CTokenSequence::~CTokenSequence()
{GUCEF_TRACE;

    tokens.clear();
    sequences.clear();
    ordering.clear();
    sequenceOpenToken.Clear();
    sequenceCloseToken.Clear();
}

/*-------------------------------------------------------------------------*/

CTokenizer::CTokenSequence&
CTokenizer::CTokenSequence::operator=( const CTokenSequence& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        tokens = src.tokens;
        sequences = src.sequences;
        ordering = src.ordering;
        sequenceOpenToken = src.sequenceOpenToken;
        sequenceCloseToken = src.sequenceCloseToken;
    }

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::FindNextGroupMatch( Int32Vector& availOpenGroupTokens       ,
                                Int32Vector& availCloseGroupTokens      ,
                                Int32PairVector& matchedGroupTokenPairs )
{GUCEF_TRACE;
    
    // the tokens in the given arrays are assumed to occur in the order they were found in the input string
    // ordered by index, so we can just iterate through them and check if we find a match

    size_t nrOfOpens = 0;
    size_t nrOfCloses = 0;
    size_t max = availOpenGroupTokens.size() > availCloseGroupTokens.size() ? availCloseGroupTokens.size() : availOpenGroupTokens.size();

    Int32Vector::iterator closeIter = availCloseGroupTokens.begin();
    while ( nrOfOpens < max || nrOfCloses < max )
    {
        size_t openIndex = nrOfOpens < max ? nrOfOpens : max-1;
        size_t closeIndex = nrOfCloses < max ? nrOfCloses : max-1;
        Int32 currentOpen = availOpenGroupTokens[ openIndex ];
        Int32 currentClose = availCloseGroupTokens[ closeIndex ];
        
        // We need to check if we find another open before a close
        // if so this indicates nesting and it means we need to find that many closes before we can match the current open

        bool isNextTokenAnOpen = ( nrOfOpens < max ) && ( currentOpen < currentClose );
        if ( isNextTokenAnOpen )
        {
            ++nrOfOpens;
        }
        else
        {
            ++nrOfCloses;
        }

        if ( nrOfOpens == nrOfCloses )
        {
            matchedGroupTokenPairs.push_back( Int32Pair( availOpenGroupTokens[ 0 ], currentClose ) );
            availOpenGroupTokens.erase( availOpenGroupTokens.begin() );
            availCloseGroupTokens.erase( closeIter );
            return true;
        }
        else
        {
            if ( !isNextTokenAnOpen )
                ++closeIter;
        }
    }

    // No match found, bad nesting / syntax
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::FindNextGroup( Int32 currentIndex                            ,
                           const Int32PairVector& matchedGroupTokenPairs ,
                           size_t& nextGroupIndex                        )
{GUCEF_TRACE;

    nextGroupIndex = 0;

    bool foundAtLeastOne = false;
    size_t closestGroupIndex = 0;
    Int32 closestGroupOpenTokenIndex = GUCEF_INT32MAX;
    for ( size_t i=0; i<matchedGroupTokenPairs.size(); ++i )
    {
        const Int32Pair& currentGroup = matchedGroupTokenPairs[ i ];
        
        // Check if the group is even relevant since we parse left to right
        // We may have already passed this group
        if ( currentGroup.first > currentIndex || currentIndex < 0 )
        {
            Int32 distanceToGroup = currentIndex < 0 ? currentGroup.first : currentGroup.first - currentIndex;
            if ( distanceToGroup >= 0 && distanceToGroup < closestGroupOpenTokenIndex )
            {
                closestGroupOpenTokenIndex = distanceToGroup;
                closestGroupIndex = i;
                foundAtLeastOne = true;
            }
        }
    }

    if ( foundAtLeastOne )
    {
        nextGroupIndex = closestGroupIndex;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::FindFirstGroup( const Int32PairVector& matchedGroupTokenPairs ,
                            size_t& firstGroupIndex                       )
{GUCEF_TRACE;

    return FindNextGroup( -1, matchedGroupTokenPairs, firstGroupIndex );
}

/*-------------------------------------------------------------------------*/

bool 
CTokenizer::FindDirectlyContainedGroups( const Int32PairVector& matchedGroupTokenPairs ,
                                         size_t currentGroupIndex                      ,
                                         Int32Vector& directlyContainedGroupsIndices   )
{GUCEF_TRACE;

    const Int32Pair& currentGroup = matchedGroupTokenPairs[ currentGroupIndex ];
    for ( size_t i=0; i<matchedGroupTokenPairs.size(); ++i )
    {
        if ( i != currentGroupIndex )
        {
            const Int32Pair& otherGroup = matchedGroupTokenPairs[ i ];
            if ( otherGroup.first > currentGroup.first && otherGroup.second < currentGroup.second )
            {
                // This group is contained within the current group
                // Question now is if its directly contained or not as it could be nested in yet another group

                bool directlyContained = true;
                for ( size_t j=0; j<matchedGroupTokenPairs.size(); ++j )
                {
                    if ( j != i && j != currentGroupIndex )
                    {
                        const Int32Pair& otherOtherGroup = matchedGroupTokenPairs[ j ];
                        if ( otherOtherGroup.first > currentGroup.first && otherOtherGroup.second < currentGroup.second )
                        {
                            if ( otherOtherGroup.first > otherGroup.first && otherOtherGroup.second < otherGroup.second )
                            {
                                // This group is nested in another group
                                directlyContained = false;
                                break;
                            }
                        }
                    }
                }

                if ( directlyContained )
                {
                    directlyContainedGroupsIndices.push_back( (Int32) i );
                }
            }
        }
    }

    return !directlyContainedGroupsIndices.empty();
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::FindLongestSegmentAtStart( const CString& inputString                  ,
                                       const CString::StringVector& searchSegments ,
                                       size_t& longestMatchSegmentIndex            )
{GUCEF_TRACE;

    bool foundMatch = false;
    longestMatchSegmentIndex = 0;

    UInt32 longestMatch = 0;
    for ( size_t i=0; i<searchSegments.size(); ++i )
    {
        const CString& searchStr = searchSegments[ i ];
        if ( inputString.StartsWith( searchStr ) )
        {
            if ( searchStr.Length() > longestMatch )
            {
                longestMatch = searchStr.Length();
                longestMatchSegmentIndex = i;
                foundMatch = true;
            }
        }
    }

    return foundMatch;
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::ParseTokens( const CString& inputString                    , 
                         const CString::StringVector& predefinedTokens ,
                         const CString::StringVector& variableNames    ,
                         CTokenSequencePtr tokenSequence               ,
                         CString::StringVector& errors                 )
{GUCEF_TRACE;

    if ( tokenSequence.IsNULL() )
    {
        errors.push_back( "TokenSequence is NULL" );
        return false;
    }
    
    // We need to tokenize the input string based on the predefined tokens and variable names
    // At this point no consideration is given to grouping tokens, we just tokenize the string
    // based on the predefined tokens and variable names if any otherwise purely whitespace
    // we parse the string left to right and create a sequence of tokens

    // First split purely based on whitespace
    CString::StringVector strElements = inputString.ParseElements( ' ', false );

    if ( predefinedTokens.empty() && variableNames.empty() )
    {
        // We only have whitespace to split on
        // This means we have our sequence of tokens
        tokenSequence->tokens = strElements;
        for ( size_t i=0; i<strElements.size(); ++i )
        {
            CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN, i };
            tokenSequence->ordering.push_back( tokenOrderPair );
        }
        return true;
    }

    // Now we need to check each element and see if it is a predefined token or a variable name
    // which is potentially combined into a single string element
    // We need to try and find the longest possible match left to right
    // If we find a match we need to split the element into the matched token and the remaining string
    for ( size_t i=0; i<strElements.size(); ++i )
    {
        CString remainder;
        CString element = strElements[ i ];
        while ( !element.IsNULLOrEmpty() )
        {
            // pre-defined tokens take priority matching wise over variable names
            // this is because a predefined token should be considered fixed syntax whereas a variable name
            // can be anything and should only be considered a variable name if no predefined token matches as part of that text
            size_t indexOfLongestMatch = 0;
            if ( FindLongestSegmentAtStart( element, predefinedTokens, indexOfLongestMatch ) )
            {
                // Our match is a predefined token
                const CString& matchedToken = predefinedTokens[ indexOfLongestMatch ];
                element = element.CutChars( matchedToken.Length(), true, 0 );

                if ( !remainder.IsNULLOrEmpty() )
                {
                    // We have a dynamically discovered token in front of this token
                    // We need to add this to the sequence first
                    tokenSequence->tokens.push_back( remainder );
                    CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN, tokenSequence->tokens.size()-1 };
                    tokenSequence->ordering.push_back( tokenOrderPair );
                    remainder.Clear();
                }

                tokenSequence->tokens.push_back( matchedToken );
                CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN, tokenSequence->tokens.size()-1 };
                tokenSequence->ordering.push_back( tokenOrderPair );
            }
            else
            if ( FindLongestSegmentAtStart( element, variableNames, indexOfLongestMatch ) )
            {
                // Our match is a variable name
                const CString& matchedToken = variableNames[ indexOfLongestMatch ];
                element = element.CutChars( matchedToken.Length(), true, 0 );

                if ( !remainder.IsNULLOrEmpty() )
                {
                    // We have a dynamically discovered token in front of this token
                    // We need to add this to the sequence first
                    tokenSequence->tokens.push_back( remainder );
                    CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN, tokenSequence->tokens.size()-1 };
                    tokenSequence->ordering.push_back( tokenOrderPair );
                    remainder.Clear();
                }

                tokenSequence->tokens.push_back( matchedToken );
                CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN, tokenSequence->tokens.size()-1 };
                tokenSequence->ordering.push_back( tokenOrderPair );
            }
            else
            {
                // No matches found against anything pre-defined
                // As such we will go character by character and look for other known tokens further down the line
                remainder += element[ 0 ];
                element = element.CutChars( 1, true, 0 );
            }
        }
        if ( !remainder.IsNULLOrEmpty() )
        {
            // We have a dynamically discovered token in front of this token
            // We need to add this to the sequence first
            tokenSequence->tokens.push_back( remainder );
            CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_TOKEN, tokenSequence->tokens.size()-1 };
            tokenSequence->ordering.push_back( tokenOrderPair );
            remainder.Clear();
        }
    }
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::FindTightestEnvelopingGroup( const Int32PairVector& allGroupTokenPairs ,
                                         const Int32Pair& envelopedGroup           ,
                                         Int32Pair& tightestEnvelopingGroup        )
{GUCEF_TRACE;

    // We need to find the tightest enveloping group for the given enveloping group
    // This means we need to find the group that is the closest to the enveloping group
    // but still contains it

    Int32Pair bestTightestEnvelopingGroup = { -1, -1 };
    Int32PairVector::const_iterator groupIter = allGroupTokenPairs.begin();
    while ( groupIter != allGroupTokenPairs.end() )
    {
        const Int32Pair& currentGroup = *groupIter;
        
        // Ignore self match
        if ( currentGroup != envelopedGroup )
        {
            // Does this group envelop the given group?
            if ( currentGroup.first < envelopedGroup.first && currentGroup.second > envelopedGroup.second )
            {
                // The given group is enveloped by this group
                // Check if this group is tighter than the current tightest group
                if ( bestTightestEnvelopingGroup.first != -1 && bestTightestEnvelopingGroup.second != -1 )
                {
                    if ( currentGroup.first > tightestEnvelopingGroup.first && currentGroup.second < tightestEnvelopingGroup.second )
                    {
                        // This group is tighter than the current tightest group
                        tightestEnvelopingGroup = currentGroup;
                    }                
                }
                else
                {
                    // init, first tightest group found
                    bestTightestEnvelopingGroup = currentGroup;
                }
            }
        }
        ++groupIter;
    }

    if ( bestTightestEnvelopingGroup.first != -1 && bestTightestEnvelopingGroup.second != -1 )
    {
        tightestEnvelopingGroup = bestTightestEnvelopingGroup;
        return true;
    }
    
    // No enveloping group found
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::ProcessGroupContent( const CString& inputString                    ,
                                 const CString::StringVector& predefinedTokens ,
                                 const CString::StringVector& variableNames    ,
                                 const CString& groupOpenToken                 ,
                                 const CString& groupCloseToken                ,
                                 const Int32PairVector& matchedGroupTokenPairs ,
                                 CTokenSequencePtr currentSequence             ,
                                 size_t currentSeqGroupIndex                   ,
                                 CString::StringVector& errors                 )
{GUCEF_TRACE;

    const Int32Pair& currentGroup = matchedGroupTokenPairs[ currentSeqGroupIndex ];

    Int32Vector directlyContainedGroupsIndices;
    if ( FindDirectlyContainedGroups( matchedGroupTokenPairs, currentSeqGroupIndex, directlyContainedGroupsIndices ) )
    {
        // Because we have nested groups we will have to differentiate between what is part of the current group and what is part of the nested groups
        // The directly nested groups will be in the vector in order from left to right
        Int32 lastGroupEnd = currentGroup.first;
        for ( size_t i=0; i<directlyContainedGroupsIndices.size(); ++i )
        {
            Int32 nestedGroupIndex = directlyContainedGroupsIndices[ i ];
            const Int32Pair& nestedGroup = matchedGroupTokenPairs[ nestedGroupIndex ];

            if ( nestedGroup.first > lastGroupEnd + (Int32) groupCloseToken.Length() )
            {
                // We have a section of the input string between the current group and the nested group
                // We need to parse this first
                CString strSectionBetweenGroups = inputString.SubstrFromRange( lastGroupEnd + groupCloseToken.Length(), nestedGroup.first );
                if ( !ParseTokens( strSectionBetweenGroups , 
                                   predefinedTokens        , 
                                   variableNames           , 
                                   currentSequence         , 
                                   errors                  ) )
                {       
                    return false;
                }
            }

            // Now we need to process the nested group
            CTokenSequencePtr nestedGroupSequence = CTokenSequence::CreateSharedObj();
            if ( nestedGroupSequence.IsNULL() )
            {
                errors.push_back( "Failed to create nested group node" );
                return false;
            }

            nestedGroupSequence->sequenceOpenToken = groupOpenToken;
            nestedGroupSequence->sequenceCloseToken = groupCloseToken;

            if ( !ProcessGroupContent( inputString            ,
                                       predefinedTokens       ,
                                       variableNames          ,
                                       groupOpenToken         ,
                                       groupCloseToken        ,
                                       matchedGroupTokenPairs ,
                                       nestedGroupSequence    ,
                                       nestedGroupIndex       ,
                                       errors                 ) )
            {
                return false;
            }

            currentSequence->sequences.push_back( nestedGroupSequence );
            CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_SEQUENCE, currentSequence->sequences.size()-1 };
            currentSequence->ordering.push_back( tokenOrderPair );

            lastGroupEnd = nestedGroup.second + groupCloseToken.Length();
        }
        return true;
    }
    else
    {
        // There are no groups within this group
        // If you dont have any directly nested groups you locally also have no indirectly nested groups, you have none
        // So we can process the content of this group as a sequence of tokens
        CString groupContent = inputString.SubstrFromRange( currentGroup.first + currentSequence->sequenceOpenToken.Length(), currentGroup.second );
        return ParseTokens( groupContent     , 
                            predefinedTokens , 
                            variableNames    , 
                            currentSequence  , 
                            errors           );
    }
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::ParseTokensAndGroups( const CString& inputString                    , 
                                  const CString::StringVector& predefinedTokens ,
                                  const CString::StringVector& variableNames    ,
                                  const CString& groupOpenToken                 ,
                                  const CString& groupCloseToken                ,
                                  CTokenSequencePtr& tokenSequence              ,
                                  CString::StringVector& errors                 )
{GUCEF_TRACE;

    Int32Vector openTokenIndices;
    Int32Vector closeTokenIndices;

    // Since we are already dealing with groups lets keep it consistent and make sure
    // that if there is an implicit outer group we add it for parsing purposes
    CString stringToParse;
    bool addedImplicitGroupOpenCloseTokens = false;
    if ( inputString.StartsWith( groupOpenToken ) && inputString.EndsWith( groupCloseToken ) )
    {
        stringToParse = inputString;
    }
    else
    {
        stringToParse = groupOpenToken + inputString + groupCloseToken;
        addedImplicitGroupOpenCloseTokens = true;
    }

    // first collect all the group open and close tokens
    Int32 groupOpenIndex = 0;
    do
    {
        groupOpenIndex = stringToParse.HasSubstr( groupOpenToken, groupOpenIndex, true );
        if ( groupOpenIndex >= 0 )
        {
            openTokenIndices.push_back( groupOpenIndex );
            groupOpenIndex += groupOpenToken.Length();
        }
    } 
    while ( groupOpenIndex >= 0 );

    Int32 groupCloseIndex = 0;
    do
    {
        groupCloseIndex = stringToParse.HasSubstr( groupCloseToken, groupCloseIndex, true );
        if ( groupCloseIndex >= 0 )
        {
            closeTokenIndices.push_back( groupCloseIndex );
            groupCloseIndex += groupCloseToken.Length();
        }
    }
    while ( groupCloseIndex >= 0 );

    // Perform some basic checks to see if we have a valid grouping structure
    if ( openTokenIndices.size() != closeTokenIndices.size() )
    {
        errors.push_back( "Number of groupOpenTokens does not match number of groupCloseTokens. groupOpenToken=" +
             groupOpenToken + " groupCloseToken=" + groupCloseToken );
        return false;
    }

    // Early out to save some effort if possible:
    // If we had no open or close tokens or just the outer ones we are dealing with a single implicit or explicit group
    if ( openTokenIndices.size() == 1 && closeTokenIndices.size() == 1 )
    {
        CTokenSequencePtr node = CTokenSequence::CreateSharedObj();        
        if ( !addedImplicitGroupOpenCloseTokens )
        {
            // Since the string actually had open and close tokens we will state as such for the sequence node
            node->sequenceOpenToken = groupOpenToken;
            node->sequenceCloseToken = groupCloseToken;
        }
        CString innerString = stringToParse.SubstrFromRange( groupOpenToken.Length(), closeTokenIndices[ 0 ] );
        if ( ParseTokens( innerString      , 
                          predefinedTokens , 
                          variableNames    , 
                          node             , 
                          errors           ) )
        {       
            if ( tokenSequence.IsNULL() )
            {
                tokenSequence = node;
            }
            else
            {
                tokenSequence->sequences.push_back( node );
                CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_SEQUENCE, tokenSequence->sequences.size()-1 };
                tokenSequence->ordering.push_back( tokenOrderPair );
            }
            return true;
        }
        return false;
    }

    // Now try to match which open and close tokens belong together
    Int32PairVector matchedGroupTokenPairs;
    matchedGroupTokenPairs.reserve( ( openTokenIndices.size() / 2 ) + 1 );
    while ( !openTokenIndices.empty() && !closeTokenIndices.empty() )
    {
        if ( !FindNextGroupMatch( openTokenIndices       ,
                                  closeTokenIndices      ,
                                  matchedGroupTokenPairs ) )
        {
            errors.push_back( "No matching groupCloseToken found for groupOpenToken at index " + ToString( *openTokenIndices.begin() ) +
                    ". groupOpenToken=" + groupOpenToken + " groupCloseToken=" + groupCloseToken );
            return false;
        }
    }

    // Now that we have all the matched open and close tokens we can start constructing the sequences from the groups
    // Note that the grouping pairs are already in the correct order since they were found in order in the input string
    // However we need to check if a group is enveloped by another group

    size_t groupIndex = 0;
    if ( !FindFirstGroup( matchedGroupTokenPairs, groupIndex ) || matchedGroupTokenPairs.empty() )
    {
        errors.push_back( "No valid group found in input string but grouping tokens are present" );
        return false;
    }    
    // Note: this must be a copy, not a reference: matchedGroupTokenPairs can be reassigned below which frees its storage
    Int32Pair firstGroup = matchedGroupTokenPairs[ groupIndex ];

    if ( firstGroup.first > 0 || firstGroup.second < ((Int32) stringToParse.Length())-1 )
    {
        // We have groups but no singular parent group
        // add the artificial singular parent group to the string
        
        stringToParse = groupOpenToken + stringToParse + groupCloseToken;
        addedImplicitGroupOpenCloseTokens = true;

        // We will now artificially create a parent group to hold all the groups
        // This is to ensure that we have a single root node for the sequences

        Int32PairVector allGroupTokenPairs; 
        Int32Pair rootGroup = { 0, stringToParse.Length() - groupCloseToken.Length() };
        allGroupTokenPairs.push_back( rootGroup );
        
        // Now shift all the pre-existing groups to the right by the length of the groupOpenToken
        // in antcipation of the decorated input string
        
        for ( size_t i=0; i<matchedGroupTokenPairs.size(); ++i )
        {
            Int32Pair group = matchedGroupTokenPairs[ i ];
            group.first += groupOpenToken.Length();
            group.second += groupOpenToken.Length();
            allGroupTokenPairs.push_back( group );
        }
        matchedGroupTokenPairs = allGroupTokenPairs;

        groupIndex = 0;
        firstGroup = matchedGroupTokenPairs[ groupIndex ];
    }

    CTokenSequencePtr currentSequence = CTokenSequence::CreateSharedObj();
    if ( currentSequence.IsNULL() )
    {
        errors.push_back( "Failed to create root node for group" );
        return false;
    }
    
    if ( !addedImplicitGroupOpenCloseTokens )
    {
        // Since the root sequence actually had open and close tokens we will state as such for the root node
        // otherwise we leave it blank on the sequence even through we did add them temporarily to help with parsing
        currentSequence->sequenceOpenToken = groupOpenToken;
        currentSequence->sequenceCloseToken = groupCloseToken;
    }

    // Hook up our first sequence to the input sequence
    if ( tokenSequence.IsNULL() )
    {
        tokenSequence = currentSequence;
    }
    else
    {
        tokenSequence->sequences.push_back( currentSequence );
        CTokenSequence::TTokenSequencePair tokenOrderPair = { CTokenSequence::TOKENSEQELEMENTTYPE_SEQUENCE, tokenSequence->sequences.size()-1 };
        tokenSequence->ordering.push_back( tokenOrderPair );
    }
    
    // Now that we have a root node we can start processing the groups recursively
    return ProcessGroupContent( stringToParse                  ,
                                predefinedTokens               ,
                                variableNames                  ,
                                groupOpenToken                 ,
                                groupCloseToken                ,
                                matchedGroupTokenPairs         ,
                                currentSequence                ,
                                groupIndex                     ,                                
                                errors                         );
}

/*-------------------------------------------------------------------------*/

bool
CTokenizer::Tokenize( const CString& inputString                    , 
                      const CString::StringVector& predefinedTokens ,
                      const CString::StringVector& variableNames    ,
                      const CString& groupOpenToken                 ,
                      const CString& groupCloseToken                ,
                      CTokenSequencePtr& tokenSequence              ,
                      CString::StringVector& errors                 )
{GUCEF_TRACE;

    tokenSequence.Unlink();

    // When tokenizing we dont want any line endings so we remove them
    CString simplifiedInputStr = inputString.RemoveChar( '\n' );
    simplifiedInputStr = simplifiedInputStr.RemoveChar( '\r' );
        
    // We also dont want tabs, it will just complicate things
    simplifiedInputStr = simplifiedInputStr.ReplaceChar( '\t', ' ' );

    // Also get rid of any multiple spaces
    simplifiedInputStr = simplifiedInputStr.CompactRepeatingChar( ' ' );

    // Get rid of any leading and trailing whitespace
    simplifiedInputStr = simplifiedInputStr.Trim( true ).Trim( false );

    // If we have tokens defined for a grouping concept we need to perform extra work
    // to correctly process groups as their own sequences in order to preserve the correct
    // precedence order
    if ( !groupOpenToken.IsNULLOrEmpty() && !groupCloseToken.IsNULLOrEmpty() )
    {
        return ParseTokensAndGroups( simplifiedInputStr ,
                                     predefinedTokens   ,
                                     variableNames      ,
                                     groupOpenToken     ,
                                     groupCloseToken    ,
                                     tokenSequence      ,
                                     errors             ) && !tokenSequence.IsNULL();
    }
    
    // If no group open and close tokens are defined the entire string is considered a single sequence of tokens
    // Left to right precedence is observed and parsed into the sequence
    return ParseTokens( simplifiedInputStr , 
                        predefinedTokens   , 
                        variableNames      , 
                        tokenSequence      , 
                        errors             ) && !tokenSequence.IsNULL();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
