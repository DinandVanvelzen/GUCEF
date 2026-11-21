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

#ifndef GUCEF_GUI_CLAYOUTSTATE_H
#define GUCEF_GUI_CLAYOUTSTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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

/**
 *  The arrangement of a window as the user shaped it: the ratio of every CSplitter and whether every CFoldablePanel is folded,
 *  each by the name of its widget (so the names of these widgets have to be unique in a window). A state is read from a widget tree
 *  with Capture(), put back with Apply(), and written to and read from the settings of an application (CDataNode, so JSON and the
 *  other codecs) with Save() and Load().
 *
 *  A state is partial by nature: Apply() leaves widgets that the state does not mention as they are and ignores entries without a
 *  widget (a layout of another version of the window), and Load() skips what is not understood (an unknown node, an entry without
 *  a name or a ratio that is no number), so a settings file of an older or newer version still gives what it can.
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutState
{
    public:

    typedef GUCEF::map< CString, Float32 > TRatioMap;
    typedef GUCEF::map< CString, bool > TFoldedMap;

    CLayoutState( void );

    CLayoutState( const CLayoutState& src );

    CLayoutState& operator=( const CLayoutState& src );

    ~CLayoutState();

    bool operator==( const CLayoutState& other ) const;
    bool operator!=( const CLayoutState& other ) const;

    void Clear( void );

    bool IsEmpty( void ) const;

    /** The ratio (0 to 1) of the splitter with this name */
    void SetSplitRatio( const CString& splitterName, Float32 ratio );
    bool TryGetSplitRatio( const CString& splitterName, Float32& ratio ) const;
    bool RemoveSplitRatio( const CString& splitterName );

    /** Whether the foldable panel with this name is folded */
    void SetFolded( const CString& panelName, bool folded );
    bool TryGetFolded( const CString& panelName, bool& folded ) const;
    bool RemoveFolded( const CString& panelName );

    const TRatioMap& GetSplitRatios( void ) const;
    const TFoldedMap& GetFolded( void ) const;

    /** Replaces the content with the splitters and foldable panels of the tree below (and including) the widget; the number of entries */
    UInt32 Capture( const CWidget& root );

    /** Sets the splitters and foldable panels below (and including) the widget that the state has an entry for; the number of widgets set */
    UInt32 Apply( CWidget& root ) const;

    /** Adds the entries as child nodes of the node: Splitter (name, ratio) and Panel (name, folded) */
    bool Save( CORE::CDataNode& node ) const;

    /** Replaces the content with the entries of the child nodes; false when the node holds no entry at all */
    bool Load( const CORE::CDataNode& node );

    private:

    TRatioMap m_ratios;
    TFoldedMap m_folded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTSTATE_H ? */
