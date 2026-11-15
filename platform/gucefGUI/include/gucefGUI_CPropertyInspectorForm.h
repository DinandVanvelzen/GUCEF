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
#ifndef GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#define GUCEF_GUI_CPROPERTYINSPECTORFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CITABLEVIEWMODEL_H
#include "gucefGUI_CITableViewModel.h"
#define GUCEF_GUI_CITABLEVIEWMODEL_H
#endif /* GUCEF_GUI_CITABLEVIEWMODEL_H ? */

#ifndef GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#include "gucefGUI_CPropertyRowDescriptor.h"
#define GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#endif /* GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H ? */

#ifndef GUCEF_GUI_CPATHFIELDFORM_H
#include "gucefGUI_CPathFieldForm.h"
#define GUCEF_GUI_CPATHFIELDFORM_H
#endif /* GUCEF_GUI_CPATHFIELDFORM_H ? */

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

class CGridView;
class CPropertyInspectorModel;

/**
 *  The values of a CORE::CDataNode tree as a list to look at and change: a grid with the columns name and value, and rows
 *  that fold for every node that has attributes or nodes below it. Each value is edited with the editor of its kind, by
 *  the grid in place: text, a number in its range, a bool as a check box, a choice in a list, a colour, a vector of 2 to 4
 *  numbers, and a path with a CPathFieldForm below the list, where a button opens the dialog that chooses it.
 *
 *  Paths. A value is found by its path in the tree: the names of the nodes from the root, separated by '/', then '@' and
 *  the name of the attribute ("Material/Surface@Roughness"; the attributes of the root start with '@': "@Name"). A node
 *  that has a value only (no attributes and no nodes below it) is a value itself and has the path of the node
 *  ("Material/Roughness"). Of several nodes with the same name below one node the first has the name, the next ones the
 *  name with "[1]", "[2]" ... ("Layer", "Layer[1]"). A node with attributes or nodes below it is a group, its path is the
 *  path of the node.
 *
 *  Descriptors. The application gives a CPropertyRowDescriptor per path (SetDescriptor()): the kind of the value, its
 *  range, choices, whether it is read only, a tooltip and a unit. A value without a descriptor is shown as text that
 *  cannot be changed.
 *
 *  The form works on a copy of the tree. SetData( node ) copies the tree and remembers where it came from: Apply() writes
 *  the copy back to it, Revert() takes it again; with SetLiveEditing( true ) every change goes to the source at once.
 *  SetDataCopy() takes a tree without a source: GetData() gives the result. A change is checked against the descriptor:
 *  SetValue() and the editors refuse a value that does not fit, with the reason. The value is written to the tree in the
 *  type the attribute had (a number stays a number).
 *
 *  Events: PropertyChangedEvent (TPropertyChangedEventData: the path, the old and the new value as text) for every change,
 *  AppliedEvent (no data) when the copy was written back to the source. A value the user changes in the grid also routes
 *  ValueChangedEvent on the grid, as the widgets of the other forms do.
 *
 *  Keys: arrows move the cursor, F2 or Enter edits the value, Space flips a check box, Escape cancels an edit; on a group
 *  row Enter and Space fold and unfold it, Right opens it and Left closes it.
 */
class GUCEF_GUI_PUBLIC_CPP CPropertyInspectorForm : public CFormEx
{
    public:

    static const CORE::CEvent PropertyChangedEvent;
    static const CORE::CEvent AppliedEvent;

    static void RegisterEvents( void );

    /** The data of PropertyChangedEvent */
    struct GUCEF_GUI_PUBLIC_CPP SPropertyChange
    {
        CString path;
        CString oldValue;
        CString newValue;
    };
    typedef struct SPropertyChange TPropertyChange;
    typedef CORE::CTCloneableObj< TPropertyChange > TPropertyChangedEventData;

    typedef GUCEF::map< CString, CPropertyRowDescriptor > TDescriptorMap;

    CPropertyInspectorForm( void );

    explicit CPropertyInspectorForm( CIGUIContext* context );

    virtual ~CPropertyInspectorForm();

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "Properties" );

    /** Makes the form in the place of 'placeholder', which it fills (as a settings page is built in its pane) */
    bool Build( CWidget* placeholder );

    /*
     *  Descriptors
     */

    void SetDescriptor( const CString& path, const CPropertyRowDescriptor& descriptor );

    void SetDescriptors( const TDescriptorMap& descriptors );

    void ClearDescriptors( void );

    /** The descriptor of a path, NULL when there is none */
    const CPropertyRowDescriptor* GetDescriptor( const CString& path ) const;

    /*
     *  The data
     */

    /** Shows a copy of the tree; Apply() writes the copy back to 'source', which has to stay while the form has it (NULL: no source) */
    void SetData( CORE::CDataNode* source );

    /** Shows a copy of the tree without a source: the result is in GetData() */
    void SetDataCopy( const CORE::CDataNode& data );

    /** The copy the form works on */
    const CORE::CDataNode& GetData( void ) const;

    /** Whether the copy differs from what was set or applied last */
    bool IsModified( void ) const;

    /** Writes the copy back to the source and notifies AppliedEvent; false without a source */
    bool Apply( void );

    /** Takes the source again, undoing the changes that were not applied; false without a source */
    bool Revert( void );

    /** With live editing every change is written to the source at once (and AppliedEvent follows it) */
    void SetLiveEditing( bool liveEditing );

    bool IsLiveEditing( void ) const;

    /*
     *  Rows and values
     */

    /** The rows shown now: groups and values, the ones below a folded group not counted */
    UInt32 GetRowCount( void ) const;

    /** The path of a row (see above), empty for a row that is not there */
    CString GetRowPath( UInt32 row ) const;

    /** The row of a path, -1 when it is not shown */
    Int32 FindRow( const CString& path ) const;

    bool IsGroupRow( UInt32 row ) const;

    /** The text of the value of a path as it is in the tree, empty when there is none */
    CString GetValue( const CString& path ) const;

    /**
     *  Changes a value as an edit does: refused with the reason in 'problem' (may be NULL) for a path without a value, a read
     *  only value, or a text that does not fit the descriptor. Notifies PropertyChangedEvent when the value changed
     */
    bool SetValue( const CString& path, const CString& text, CString* problem = GUCEF_NULL );

    /** Folds or unfolds a group (also the groups that are not shown now); false for a path that is no group */
    bool SetGroupExpanded( const CString& path, bool expanded );

    bool IsGroupExpanded( const CString& path ) const;

    /** Puts the cursor on the value of a path (the group row of a group); false for a row that is not shown */
    bool SelectPath( const CString& path );

    /** The path of the row with the cursor, empty when there is none */
    CString GetSelectedPath( void ) const;

    /** The grid, for what is not here: its columns, its scrolling */
    CGridView* GetGrid( void );

    /** The path field below the list: shown while the cursor is on an editable path; NULL before the form is built */
    CPathFieldForm* GetPathField( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPreLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CPropertyInspectorModel;

    typedef CORE::CTEventHandlerFunctor< CPropertyInspectorForm > TEventCallback;

    void OnApplyActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRevertActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGridItemClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGridKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGridMouseMove( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGridCursorChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPathChosen( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPathEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    void RebuildRows( void );
    void ShowButtons( void );
    void UpdatePathEditor( void );
    bool ChangeValue( const CString& path, const CString& text, CString* problem, bool byUser );
    bool ToggleGroup( Int32 row );
    void WriteToSource( void );

    CPropertyInspectorForm( const CPropertyInspectorForm& src );
    CPropertyInspectorForm& operator=( const CPropertyInspectorForm& src );

    private:

    CORE::CDataNode m_data;
    CORE::CDataNode* m_source;
    TDescriptorMap m_descriptors;
    GUCEF::set< CString > m_collapsed;
    CPropertyInspectorModel* m_model;           /**< owned by the grid through a shared pointer, which this form keeps too */
    TTableViewModelPtr m_modelPtr;
    CPathFieldForm* m_pathField;                /**< made when the layout is loaded, in the place of the strip below the list */
    bool m_liveEditing;
    bool m_modified;
    bool m_updatingWidgets;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CPROPERTYINSPECTORFORM_H ? */
