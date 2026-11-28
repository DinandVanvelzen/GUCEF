/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXXMLCALLBACKS_H
#define TABLECODECPLUGIN_XLSX_CXLSXXMLCALLBACKS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDOCUMENTPARSERCALLBACKS_H
#include "gucefCORE_CIDocumentParserCallbacks.h"
#define GUCEF_CORE_CIDOCUMENTPARSERCALLBACKS_H
#endif /* GUCEF_CORE_CIDOCUMENTPARSERCALLBACKS_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

#ifndef TABLECODECPLUGIN_XLSX_MACROS_H
#include "tablecodecpluginXLSX_macros.h"
#define TABLECODECPLUGIN_XLSX_MACROS_H
#endif /* TABLECODECPLUGIN_XLSX_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Base of the parsers of the XML parts of a package: it keeps the first error of the parse, so that the parser of a part
 *  can say what is wrong with its content (a status and a text), and the error the XML reader reports for the syntax.
 *  Once a failure is noted the events that follow are to be ignored: IsFailed().
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxXmlCallbacks : public CORE::CIDocumentParserCallbacks
{
    public:

    CXlsxXmlCallbacks( void );

    virtual ~CXlsxXmlCallbacks() GUCEF_VIRTUAL_OVERRIDE;

    bool IsFailed( void ) const;

    CORE::TDataTableStatus GetStatus( void ) const;

    const CORE::CString& GetError( void ) const;

    /** The reason of the XML reader for a document that is not well formed */
    const CORE::CString& GetSyntaxError( void ) const;

    /** Notes a failure, the first one stays */
    void Fail( CORE::TDataTableStatus status, const CORE::CString& error );

    /** Called when a failure is noted, for the parser to cut the rest of the part off */
    virtual void OnFailed( void );

    virtual void OnDocumentBegin( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnDocumentEnd( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeChildrenBegin( const CORE::CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeChildrenEnd( const CORE::CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnParseError( CORE::Int32 errorCode, const CORE::CString& description ) GUCEF_VIRTUAL_OVERRIDE;

    /** The text of the node id without its namespace prefix: "x:row" is "row". Valid as long as the variant */
    static const char* LocalName( const CORE::CVariant& name, UInt32& length );

    /**
     *  The text of a value of the XML reader without a copy when the variant holds it as text; else the text is made in the holder.
     *  The pointer is valid as long as the variant and the holder are
     */
    static void TextOf( const CORE::CVariant& value, CORE::CString& holder, const char*& text, UInt32& length );

    /** Equal to a text without a namespace prefix */
    static bool IsName( const char* name, UInt32 nameLength, const char* expected );

    private:

    CORE::TDataTableStatus m_status;
    CORE::CString m_error;
    CORE::CString m_syntaxError;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_XLSX_CXLSXXMLCALLBACKS_H ? */
