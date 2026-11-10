/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#define GUCEF_CORE_CGLOBPATTERN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  A glob pattern as file dialogs and shells know them, matched against a whole name or path:
 *
 *      *       any characters except '/'
 *      **      any characters including '/'; "**" followed by '/' also matches no directory at all ("**" + "/" + "*.cue" matches "a.cue")
 *      ?       one character except '/'
 *      [abc]   one of the characters; [a-z] a range; [!a-z] or [^a-z] none of them
 *      {a,b}   one of the alternatives, which can hold patterns themselves
 *      \x      the character x itself
 *
 *  Characters are Unicode code points (the text is UTF-8). Case is ignored unless asked for.
 */
class GUCEF_CORE_PUBLIC_CPP CGlobPattern
{
    public:

    CGlobPattern( void );

    explicit CGlobPattern( const CString& pattern, bool caseSensitive = false );

    /** False when the pattern is not valid (an open '[' or '{'); such a pattern matches nothing */
    bool SetPattern( const CString& pattern, bool caseSensitive = false );

    const CString& GetPattern( void ) const;

    bool IsValid( void ) const;

    bool IsCaseSensitive( void ) const;

    bool IsMatch( const CString& text ) const;

    static bool IsMatch( const CString& pattern, const CString& text, bool caseSensitive = false );

    /** Whether the text has characters with a meaning in a glob pattern: * ? [ { */
    static bool HasGlobCharacters( const CString& text );

    private:

    CString m_pattern;
    bool m_caseSensitive;
    bool m_valid;
    CString::StringVector m_alternatives;   /**< the pattern with its {a,b} alternatives expanded */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */
