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

#ifndef GUCEF_CORE_CTOKENIZER_H
#define GUCEF_CORE_CTOKENIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"   
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class which breaks down a given string into tokens.
 *  The tokens are externally defined and passed to the tokenizer as token definition aids
 *  The largest matching token out of a given string segment is considered the token.
 *          ie for >= if '>=' is in the token list it would be the accepted token 
 *          not '>' and '=' if those are also in the token list
 *  If no token matches the string segment the entire segment is considered a single token base on whitespace
 *  Special consideration is given to precedence of tokens as defined by grouping tokens  
 *          ie ( and ) in C++
 *  If no group open and close tokens are defined the entire string is considered a single sequence of tokens
 *  Left to right precedence is observed and parsed into the sequence
 */
class GUCEF_CORE_PUBLIC_CPP CTokenizer
{
    public:

    class GUCEF_CORE_PUBLIC_CPP CTokenSequence : public CTSharedObjCreator< CTokenSequence, MT::CNoLock >
    {
        public:

        enum ETokenSeqElementType
        {
            TOKENSEQELEMENTTYPE_UNDEFINED = 0,

            TOKENSEQELEMENTTYPE_TOKEN,
            TOKENSEQELEMENTTYPE_SEQUENCE
        };
        typedef enum ETokenSeqElementType TTokenSeqElementType;

        typedef typename CTSharedObjCreator< CTokenSequence, MT::CNoLock >::TSharedPtrType     CTokenSequencePtr;
        typedef GUCEF::vector< CTokenSequencePtr >                                             TTokenSequencePtrVector;
        typedef std::pair< TTokenSeqElementType, size_t >                                      TTokenSequencePair;
        typedef GUCEF::vector< TTokenSequencePair >                                            TTokenSequencePairVector;
        
        CTokenSequence( void );

        CTokenSequence( const CTokenSequence& src );

        virtual ~CTokenSequence();

        CTokenSequence& operator=( const CTokenSequence& src );

        CString::StringVector tokens;        
        TTokenSequencePtrVector sequences;
        TTokenSequencePairVector ordering;
        CString sequenceOpenToken;
        CString sequenceCloseToken;
    };
    typedef CTokenSequence::CTokenSequencePtr     CTokenSequencePtr;

    /**
     *  Tokenizes the input string based on the given predefined tokens, variable names and grouping tokens.
     */
    static bool Tokenize( const CString& inputString                    , 
                          const CString::StringVector& predefinedTokens ,
                          const CString::StringVector& variableNames    ,
                          const CString& groupOpenToken                 ,
                          const CString& groupCloseToken                ,
                          CTokenSequencePtr& tokenSequence              ,
                          CString::StringVector& errors                 );

    private:

    typedef std::pair< Int32, Int32 >                       Int32Pair;
    typedef GUCEF::vector< Int32Pair >                      Int32PairVector;
    typedef GUCEF::vector< Int32 >                          Int32Vector;
    typedef GUCEF::map< Int32Pair, CTokenSequencePtr >      Int32Pair2TokenGroupPtrMap;

    static bool ParseTokens( const CString& inputString                    , 
                             const CString::StringVector& predefinedTokens ,
                             const CString::StringVector& variableNames    ,
                             CTokenSequencePtr tokenSequence               ,
                             CString::StringVector& errors                 );

    static bool ParseTokensAndGroups( const CString& inputString                    , 
                                      const CString::StringVector& predefinedTokens ,
                                      const CString::StringVector& variableNames    ,
                                      const CString& groupOpenToken                 ,
                                      const CString& groupCloseToken                ,
                                      CTokenSequencePtr& tokenSequence              ,
                                      CString::StringVector& errors                 );

    static bool FindLongestSegmentAtStart( const CString& inputString                  ,
                                           const CString::StringVector& searchSegments ,
                                           size_t& longestMatchSegmentIndex            );

    static bool FindNextGroupMatch( Int32Vector& availOpenGroupTokens       ,
                                    Int32Vector& availCloseGroupTokens      ,
                                    Int32PairVector& matchedGroupTokenPairs );

    static bool FindFirstGroup( const Int32PairVector& matchedGroupTokenPairs ,
                                size_t& firstGroupIndex                       );

    static bool FindNextGroup( Int32 currentIndex                            ,
                               const Int32PairVector& matchedGroupTokenPairs ,
                               size_t& nextGroupIndex                        );

    static bool FindDirectlyContainedGroups( const Int32PairVector& matchedGroupTokenPairs ,
                                             size_t currentGroupIndex                      ,
                                             Int32Vector& directlyContainedGroupsIndices   );

    static bool ProcessGroupContent( const CString& inputString                    ,
                                     const CString::StringVector& predefinedTokens ,
                                     const CString::StringVector& variableNames    ,
                                     const CString& groupOpenToken                 ,
                                     const CString& groupCloseToken                ,
                                     const Int32PairVector& matchedGroupTokenPairs ,
                                     CTokenSequencePtr currentSequence             ,
                                     size_t currentSeqGroupIndex                   ,                                     
                                     CString::StringVector& errors                 );

    static bool FindTightestEnvelopingGroup( const Int32PairVector& allGroupTokenPairs ,
                                             const Int32Pair& envelopedGroup           ,
                                             Int32Pair& tightestEnvelopingGroup        );
    
    


};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTOKENIZER_H ? */
