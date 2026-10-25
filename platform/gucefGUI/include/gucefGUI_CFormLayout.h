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

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#define GUCEF_GUI_CFORMLAYOUT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

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

class CGUIContext;

/**
 *  gucefGUI's own layout format, the same for every backend: a DataNode tree that any DStore codec reads
 *  and writes (XML and JSON alike).
 *
 *      GUILayout format="GUILayout" version="1"
 *          Resources
 *              Image alias="..." path="..."        an image file (VFS path) under a name
 *              Font name="..." path="..."          a font file for the driver
 *          Widget type="..." name="..." position="rel,px rel,px" size="rel,px rel,px"
 *              Property name="..." value="..."     a string list holds its entries in 'value', one per line
 *              BackendHints driver="..."           properties only that driver reads
 *                  Property name="..." value="..."
 *              Widget ...                          children, in drawing order
 *
 *  The 'format' attribute identifies the layout where a codec keeps no name for the top node (JSON).
 *  Nodes are written so every codec keeps them: nothing depends on the value of a node, and the nodes
 *  of the format have at least two attributes or children.
 *  'version' only goes up for changes older code cannot read; new widget types or properties do not
 *  change it. Nodes and properties gucefGUI does not know are kept, so loading and saving a layout loses
 *  nothing. A saved layout only holds values that differ from the defaults.
 */
class GUCEF_GUI_PUBLIC_CPP CFormLayout
{
    public:

    /** The layout version this gucefGUI writes and the newest it reads */
    static const UInt32 CurrentVersion;

    typedef GUCEF::vector< CString > TStringVector;
    typedef GUCEF::set< CString > TStringSet;

    /** What a built widget came from: its node and the properties that came from backend hints */
    struct SWidgetSource
    {
        CORE::CDataNode* node;
        TStringSet hintProperties;

        SWidgetSource( void );
    };
    typedef struct SWidgetSource TWidgetSource;
    typedef GUCEF::map< const CWidget*, TWidgetSource > TWidgetSourceMap;

    /** The layout node: the tree itself or a child of it, named "GUILayout" or with format="GUILayout"; NULL when there is none */
    static const CORE::CDataNode* FindLayoutRoot( const CORE::CDataNode& tree );

    static CORE::CDataNode* FindLayoutRoot( CORE::CDataNode& tree );

    /** Reads a file (from disk, else from the VFS) with the DStore codec its extension names */
    static bool LoadTree( const CString& path, CORE::CDataNode& tree );

    static bool LoadTree( CORE::CIOAccess& input, const CString& codecType, CORE::CDataNode& tree );

    /** Writes a file (to disk) with the DStore codec its extension names */
    static bool SaveTree( const CORE::CDataNode& tree, const CString& path );

    static bool SaveTree( const CORE::CDataNode& tree, CORE::CIOAccess& output, const CString& codecType );

    /** Registers the images and loads the fonts of the Resources node */
    static bool ApplyResources( CGUIContext& context, const CORE::CDataNode& layoutRoot );

    /** Makes the widget of a "Widget" node and everything below it */
    static CWidget* Build( CGUIContext& context, CORE::CDataNode& widgetNode, CWidget* parent, TWidgetSourceMap& sources );

    /**
     *  Writes the state of a built widget (and its children) back into its node: attributes and Property
     *  nodes for every value that differs from the default, new nodes for new children, and away with the
     *  nodes of children that are gone. Everything else in the node stays as it was.
     */
    static void UpdateNode( const CWidget& widget, CORE::CDataNode& widgetNode, TWidgetSourceMap& sources );

    /** Makes a fresh layout tree for a widget (and its children) that was not loaded from one */
    static void WriteWidget( const CWidget& widget, CORE::CDataNode& parentNode, TWidgetSourceMap& sources );

    /** Checks a layout against the registered widget types; every problem found is added as a line of text */
    static bool Validate( const CORE::CDataNode& layoutRoot, TStringVector& problems );

    /** Removes Property nodes and attributes whose value equals the default; returns how many went */
    static UInt32 Simplify( CORE::CDataNode& layoutRoot );

    /** Brings an older layout to the current version; notes say what was changed */
    static bool Migrate( CORE::CDataNode& layoutRoot, TStringVector& notes );

    /*
     *  Building a layout in code, as the forms gucefGUI comes with do: the tree can be loaded with
     *  CForm::LoadLayout() and saved to a file like any other layout
     */

    /** Makes 'tree' an empty layout of the current version */
    static void InitLayout( CORE::CDataNode& tree );

    /** Adds a Widget node to a layout or to a Widget node; position and size are UDim pairs, empty ones are left out */
    static CORE::CDataNode* AddWidgetNode( CORE::CDataNode& parentNode                 ,
                                           const CString& type                         ,
                                           const CString& name                         ,
                                           const CString& position = CString::Empty    ,
                                           const CString& size = CString::Empty        );

    /**
     *  Sets a property of a Widget node, replacing the value it had. The node is a config tree and holds text, as the
     *  format does: the name of the property is written as the text of the id, the value is text. A standard property is
     *  named by its EWidgetProperty, any other property (a backend hint, a property of a widget type an application
     *  registered) by its name.
     */
    static void SetWidgetNodeProperty( CORE::CDataNode& widgetNode, EWidgetProperty property, const CString& value );
    static void SetWidgetNodeProperty( CORE::CDataNode& widgetNode, const CString& name, const CString& value );

    private:

    static void ValidateWidget( const CORE::CDataNode& widgetNode, const CString& path, TStringVector& problems );
    static UInt32 SimplifyWidget( CORE::CDataNode& widgetNode );
    static bool ReadPropertyValue( const CORE::CDataNode& propertyNode, CString& value );
    static void WritePropertyValue( CORE::CDataNode& propertyNode, const CString& name, const CString& value );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */
