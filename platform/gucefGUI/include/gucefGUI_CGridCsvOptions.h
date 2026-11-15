/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CGRIDCSVOPTIONS_H
#define GUCEF_GUI_CGRIDCSVOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How the CSV export of a grid writes and when it runs. The text is UTF-8; fields are quoted as RFC 4180 says (a field
 *  with the delimiter, a quote, a CR or an LF, or a blank at its start or end, is in quotes, quotes inside doubled); an
 *  empty cell is an empty field.
 */
class GUCEF_GUI_PUBLIC_CPP CGridCsvOptions
{
    public:

    enum ELineEnding
    {
        LINEENDING_CRLF = 0 ,   /**< what RFC 4180 says */
        LINEENDING_LF       ,
        LINEENDING_CR
    };

    char delimiter;                 /**< ',' by default; '\t' for tab separated, ';' for spreadsheets of a decimal comma; not a quote, CR, LF or NUL */
    ELineEnding lineEnding;
    bool byteOrderMark;             /**< a UTF-8 byte order mark in front, for spreadsheets that need it to read UTF-8; false by default */
    bool includeHeader;             /**< a first row of the column titles; true by default */
    bool rawValues;                 /**< the values of the model as plain text instead of the text the grid shows (no grouping, prefix, suffix or rounding); false by default */
    UInt32 batchRowCount;           /**< rows built and written at a time, the bound of the memory an export takes; 4096 by default, 1 to 1048576 */
    UInt64 immediateCellLimit;      /**< the export flow of a grid writes up to this many cells (rows times columns) at once, larger tables in slices per frame; 200000 by default */
    UInt32 cellsPerFrame;           /**< cells a slice writes per frame of the GUI; 50000 by default */
    bool useThreadPool;             /**< the export flow of a grid with a model writes on a pool thread; only for a model that may be read from another thread while the export runs. False by default */
    CString threadPoolName;         /**< the pool of the asynchronous export; empty for the default pool */
    bool showResultMessage;         /**< the export flow tells the user what came of it in a message box; true by default */
    CString suggestedFileName;      /**< the name the file save dialog starts with; empty for the name of the grid widget, else "table". ".csv" is added when missing */

    CGridCsvOptions( void );

    /** False, with the reason, for a delimiter that cannot be one or a batch size out of range */
    bool Validate( CString& error ) const;

    /** The line ending as the CSV table codec of gucefCORE names it: "crlf", "lf" or "cr" */
    CString GetLineEndingCodecText( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCSVOPTIONS_H ? */
