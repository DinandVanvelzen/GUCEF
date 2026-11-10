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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GUI_CGUIDRIVER_H
#include "gucefGUI_CGUIDriver.h"
#define GUCEF_GUI_CGUIDRIVER_H
#endif /* GUCEF_GUI_CGUIDRIVER_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CFormLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Version 1: the first gucefGUI layout format */
const UInt32 CFormLayout::CurrentVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


namespace {

/** The descriptor of a property of a type, or NULL */
const CPropertyDescriptor*
FindDescriptor( const CString& type, const CString& name )
{
    return CGuiGlobal::Instance()->GetWidgetTypeRegistry().FindProperty( type, name );
}

/** The Property child with the given name, NULL when none */
CORE::CDataNode*
FindPropertyNode( CORE::CDataNode& node, const CString& name )
{
    CORE::CDataNode::iterator i = node.Begin();
    while ( i != node.End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Property" && child->GetAttributeValue( "name" ).AsString() == name )
            return child;
    }
    return GUCEF_NULL;
}

}

CFormLayout::SWidgetSource::SWidgetSource( void )
    : node( GUCEF_NULL )
    , hintProperties()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CFormLayout::FindLayoutRoot( const CORE::CDataNode& tree )
{GUCEF_TRACE;

    if ( tree.GetName() == "GUILayout" || tree.GetAttributeValue( "format" ).AsString() == "GUILayout" )
        return &tree;
    return tree.FindChild( "GUILayout" );
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode*
CFormLayout::FindLayoutRoot( CORE::CDataNode& tree )
{GUCEF_TRACE;

    if ( tree.GetName() == "GUILayout" || tree.GetAttributeValue( "format" ).AsString() == "GUILayout" )
        return &tree;
    return tree.FindChild( "GUILayout" );
}

/*-------------------------------------------------------------------------*/

bool
CFormLayout::LoadTree( CORE::CIOAccess& input, const CString& codecType, CORE::CDataNode& tree )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecType, codec, false ) || codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FormLayout: No DStore codec for \"" << codecType << "\" is loaded" );
        return false;
    }
    tree.Clear();
    return codec->BuildDataTree( &tree, &input );
}

/*-------------------------------------------------------------------------*/

bool
CFormLayout::LoadTree( const CString& path, CORE::CDataNode& tree )
{GUCEF_TRACE;

    CString codecType = CORE::ExtractFileExtention( path ).Lowercase();
    if ( CORE::FileExists( path ) )
    {
        CORE::CFileAccess file( path, "rb" );
        if ( !file.IsValid() )
            return false;
        return LoadTree( file, codecType, tree );
    }
    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( path );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FormLayout: Cannot open " << path );
        return false;
    }
    return LoadTree( *file->GetAccess(), codecType, tree );
}

/*-------------------------------------------------------------------------*/

bool
CFormLayout::SaveTree( const CORE::CDataNode& tree, CORE::CIOAccess& output, const CString& codecType )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecType, codec, false ) || codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FormLayout: No DStore codec for \"" << codecType << "\" is loaded" );
        return false;
    }
    return codec->StoreDataTree( &tree, &output );
}

/*-------------------------------------------------------------------------*/

bool
CFormLayout::SaveTree( const CORE::CDataNode& tree, const CString& path )
{GUCEF_TRACE;

    CORE::CFileAccess file( path, "wb" );
    if ( !file.IsValid() )
        return false;
    return SaveTree( tree, file, CORE::ExtractFileExtention( path ).Lowercase() );
}

/*-------------------------------------------------------------------------*/

bool
CFormLayout::ApplyResources( CGUIContext& context, const CORE::CDataNode& layoutRoot )
{GUCEF_TRACE;

    const CORE::CDataNode* resources = layoutRoot.FindChild( "Resources" );
    if ( GUCEF_NULL == resources )
        return true;

    bool success = true;
    CORE::CDataNode::const_iterator i = resources->ConstBegin();
    while ( i != resources->ConstEnd() )
    {
        const CORE::CDataNode* resource = (*i);
        ++i;
        if ( resource->GetName() == "Image" )
        {
            CString alias = resource->GetAttributeValue( "alias" ).AsString();
            CString path = resource->GetAttributeValue( "path" ).AsString();
            IMAGE::TPixelMapPtr pixels;
            UInt32 revision = 0;
            if ( alias.IsNULLOrEmpty() || !context.ResolveImage( path, pixels, revision ) || !context.RegisterImage( alias, pixels ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FormLayout: Image resource \"" << alias << "\" from " << path << " is not available" );
                success = false;
            }
        }
        else
        if ( resource->GetName() == "Font" )
        {
            CString path = resource->GetAttributeValue( "path" ).AsString();
            if ( !path.IsNULLOrEmpty() && ( GUCEF_NULL == context.GetDriver() || !context.GetDriver()->LoadFontFromAsset( path ) ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FormLayout: Font resource " << path << " is not available" );
                success = false;
            }
        }
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CFormLayout::ReadPropertyValue( const CORE::CDataNode& propertyNode, CString& value )
{GUCEF_TRACE;

    // every value, lists too ('\n' between the entries), is in the 'value' attribute
    const CORE::CDataNode::TKeyValuePair* attribute = propertyNode.GetAttribute( "value" );
    if ( GUCEF_NULL == attribute )
    {
        value.Clear();
        return false;
    }
    value = attribute->second.AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFormLayout::WritePropertyValue( CORE::CDataNode& propertyNode, const CString& name, const CString& value )
{GUCEF_TRACE;

    propertyNode.SetAttribute( "name", name );
    propertyNode.SetAttribute( "value", value );
}

/*-------------------------------------------------------------------------*/

void
CFormLayout::InitLayout( CORE::CDataNode& tree )
{GUCEF_TRACE;

    tree.Clear();
    tree.SetName( "GUILayout" );
    tree.SetAttribute( "format", "GUILayout" );
    tree.SetAttribute( "version", CORE::UInt32ToString( CurrentVersion ) );
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode*
CFormLayout::AddWidgetNode( CORE::CDataNode& parentNode ,
                            const CString& type         ,
                            const CString& name         ,
                            const CString& position     ,
                            const CString& size         )
{GUCEF_TRACE;

    CORE::CDataNode* node = parentNode.AddChild( "Widget" );
    node->SetAttribute( "type", type );
    node->SetAttribute( "name", name );
    if ( !position.IsNULLOrEmpty() )
        node->SetAttribute( "position", position );
    if ( !size.IsNULLOrEmpty() )
        node->SetAttribute( "size", size );
    return node;
}

/*-------------------------------------------------------------------------*/

void
CFormLayout::SetWidgetNodeProperty( CORE::CDataNode& widgetNode, EWidgetProperty property, const CString& value )
{GUCEF_TRACE;

    if ( EWidgetProperty_Unknown == property || (UInt32) property >= (UInt32) EWidgetProperty_Count )
        return;
    SetWidgetNodeProperty( widgetNode, WidgetPropertyToString( property ), value );
}

/*-------------------------------------------------------------------------*/

void
CFormLayout::SetWidgetNodeProperty( CORE::CDataNode& widgetNode, const CString& name, const CString& value )
{GUCEF_TRACE;

    CORE::CDataNode::iterator i = widgetNode.Begin();
    while ( i != widgetNode.End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Property" && child->GetAttributeValue( "name" ).AsString() == name )
        {
            WritePropertyValue( *child, name, value );
            return;
        }
    }
    CORE::CDataNode* propertyNode = widgetNode.AddChild( "Property" );
    WritePropertyValue( *propertyNode, name, value );
}

/*-------------------------------------------------------------------------*/

CWidget*
CFormLayout::Build( CGUIContext& context, CORE::CDataNode& widgetNode, CWidget* parent, TWidgetSourceMap& sources )
{GUCEF_TRACE;

    CString type = widgetNode.GetAttributeValue( "type" ).AsString();
    CString name = widgetNode.GetAttributeValue( "name" ).AsString();
    CWidget* widget = context.CreateWidget( type, name, parent );
    if ( GUCEF_NULL == widget )
        return GUCEF_NULL;

    TWidgetSource& source = sources[ widget ];
    source.node = &widgetNode;

    CString position = widgetNode.GetAttributeValue( "position" ).AsString();
    if ( !position.IsNULLOrEmpty() )
        widget->SetProperty( EWidgetProperty_Position, position );
    CString size = widgetNode.GetAttributeValue( "size" ).AsString();
    if ( !size.IsNULLOrEmpty() )
        widget->SetProperty( EWidgetProperty_Size, size );

    CString driverName = GUCEF_NULL != context.GetDriver() ? context.GetDriver()->GetDriverName() : CString::Empty;
    CORE::CDataNode::iterator i = widgetNode.Begin();
    while ( i != widgetNode.End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Property" )
        {
            CString value;
            ReadPropertyValue( *child, value );
            widget->SetProperty( child->GetAttributeValue( "name" ).AsString(), value );
        }
        else
        if ( child->GetName() == "BackendHints" && child->GetAttributeValue( "driver" ).AsString() == driverName )
        {
            CORE::CDataNode::iterator h = child->Begin();
            while ( h != child->End() )
            {
                CORE::CDataNode* hint = (*h);
                ++h;
                if ( hint->GetName() != "Property" )
                    continue;
                CString hintName = hint->GetAttributeValue( "name" ).AsString();
                CString value;
                ReadPropertyValue( *hint, value );
                widget->SetProperty( hintName, value );
                source.hintProperties.insert( hintName );
            }
        }
    }

    i = widgetNode.Begin();
    while ( i != widgetNode.End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Widget" )
            Build( context, *child, widget, sources );
    }
    return widget;
}

/*-------------------------------------------------------------------------*/

void
CFormLayout::UpdateNode( const CWidget& widget, CORE::CDataNode& widgetNode, TWidgetSourceMap& sources )
{GUCEF_TRACE;

    TWidgetSource& source = sources[ &widget ];
    source.node = &widgetNode;

    widgetNode.SetAttribute( "type", widget.GetType() );
    widgetNode.SetAttribute( "name", widget.GetName() );

    CString value;
    if ( widget.GetExplicitProperty( EWidgetProperty_Position, value ) )
        widgetNode.SetAttribute( "position", value );
    else
        widgetNode.DelAttribute( "position" );
    if ( widget.GetExplicitProperty( EWidgetProperty_Size, value ) && !value.IsNULLOrEmpty() )
        widgetNode.SetAttribute( "size", value );
    else
        widgetNode.DelAttribute( "size" );

    // properties: what is set on the widget and differs from the default, stored types only
    const CWidget::TPropertyMap& explicitProperties = widget.GetExplicitProperties();
    CWidget::TPropertyMap::const_iterator p = explicitProperties.begin();
    while ( p != explicitProperties.end() )
    {
        const CString& name = (*p).first;
        const CString& propertyValue = (*p).second;
        ++p;
        if ( name == "Position" || name == "Size" || source.hintProperties.find( name ) != source.hintProperties.end() )
            continue;
        const CPropertyDescriptor* descriptor = FindDescriptor( widget.GetType(), name );
        bool stored = GUCEF_NULL == descriptor || descriptor->HasFlag( CPropertyDescriptor::FLAG_STORED );
        bool isDefault = GUCEF_NULL != descriptor && descriptor->defaultValue == propertyValue;
        CORE::CDataNode* propertyNode = FindPropertyNode( widgetNode, name );
        if ( !stored || isDefault )
        {
            GUCEF_DELETE propertyNode;
            continue;
        }
        if ( GUCEF_NULL == propertyNode )
            propertyNode = widgetNode.AddChild( "Property" );
        WritePropertyValue( *propertyNode, name, propertyValue );
    }

    // Property nodes for properties that are no longer set
    CORE::CDataNode::iterator i = widgetNode.Begin();
    GUCEF::vector< CORE::CDataNode* > stale;
    while ( i != widgetNode.End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Property" && explicitProperties.find( child->GetAttributeValue( "name" ).AsString() ) == explicitProperties.end() )
            stale.push_back( child );
    }

    // children: update the nodes of known children, add nodes for new ones, drop nodes of children that are gone
    GUCEF::set< CORE::CDataNode* > used;
    const CWidget::TWidgetVector& children = widget.GetChildren();
    for ( UInt32 c=0; c<(UInt32)children.size(); ++c )
    {
        const CWidget* child = children[ c ];
        TWidgetSourceMap::iterator s = sources.find( child );
        if ( s != sources.end() && GUCEF_NULL != (*s).second.node && (*s).second.node->GetParent() == &widgetNode )
        {
            UpdateNode( *child, *(*s).second.node, sources );
            used.insert( (*s).second.node );
        }
        else
        {
            WriteWidget( *child, widgetNode, sources );
            used.insert( sources[ child ].node );
        }
    }
    i = widgetNode.Begin();
    while ( i != widgetNode.End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Widget" && used.find( child ) == used.end() )
            stale.push_back( child );
    }
    for ( UInt32 s=0; s<(UInt32)stale.size(); ++s )
        GUCEF_DELETE stale[ s ];
}

/*-------------------------------------------------------------------------*/

void
CFormLayout::WriteWidget( const CWidget& widget, CORE::CDataNode& parentNode, TWidgetSourceMap& sources )
{GUCEF_TRACE;

    CORE::CDataNode* node = parentNode.AddChild( "Widget" );
    UpdateNode( widget, *node, sources );
}

/*-------------------------------------------------------------------------*/

void
CFormLayout::ValidateWidget( const CORE::CDataNode& widgetNode, const CString& path, TStringVector& problems )
{GUCEF_TRACE;

    CWidgetTypeRegistry& registry = CGuiGlobal::Instance()->GetWidgetTypeRegistry();
    CString type = widgetNode.GetAttributeValue( "type" ).AsString();
    CString name = widgetNode.GetAttributeValue( "name" ).AsString();
    CString here = path.IsNULLOrEmpty() ? name : path + '/' + name;
    if ( !registry.IsRegistered( type ) )
    {
        problems.push_back( here + ": unknown widget type \"" + type + "\"" );
        return;
    }

    CPropertyDescriptor positionDescriptor( "Position", CPropertyDescriptor::VALUETYPE_UVECTOR, CString::Empty, 0, CString::Empty, CString::Empty );
    CString normalized;
    CString position = widgetNode.GetAttributeValue( "position" ).AsString();
    if ( !position.IsNULLOrEmpty() && !positionDescriptor.Validate( position, normalized ) )
        problems.push_back( here + ": position \"" + position + "\" is not a position" );
    CString size = widgetNode.GetAttributeValue( "size" ).AsString();
    if ( !size.IsNULLOrEmpty() && !positionDescriptor.Validate( size, normalized ) )
        problems.push_back( here + ": size \"" + size + "\" is not a size" );

    GUCEF::set< CString > childNames;
    CORE::CDataNode::const_iterator i = widgetNode.ConstBegin();
    while ( i != widgetNode.ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Property" )
        {
            CString propertyName = child->GetAttributeValue( "name" ).AsString();
            const CPropertyDescriptor* descriptor = registry.FindProperty( type, propertyName );
            if ( GUCEF_NULL == descriptor )
            {
                problems.push_back( here + ": type " + type + " has no property \"" + propertyName + "\"" );
                continue;
            }
            CString value;
            ReadPropertyValue( *child, value );
            if ( !value.IsNULLOrEmpty() && !descriptor->Validate( value, normalized ) )
                problems.push_back( here + ": \"" + value + "\" is no valid value for " + propertyName + " (" + CPropertyDescriptor::ValueTypeToString( descriptor->valueType ) + ")" );
        }
        else
        if ( child->GetName() == "Widget" )
        {
            CString childName = child->GetAttributeValue( "name" ).AsString();
            if ( !childName.IsNULLOrEmpty() && !childNames.insert( childName ).second )
                problems.push_back( here + ": more than one child is called \"" + childName + "\"" );
            ValidateWidget( *child, here, problems );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CFormLayout::Validate( const CORE::CDataNode& layoutRoot, TStringVector& problems )
{GUCEF_TRACE;

    UInt32 before = (UInt32) problems.size();
    const CORE::CDataNode* root = FindLayoutRoot( layoutRoot );
    if ( GUCEF_NULL == root )
    {
        problems.push_back( "no GUILayout node" );
        return false;
    }
    UInt32 version = CORE::StringToUInt32( root->GetAttributeValue( "version" ).AsString(), 1 );
    if ( version > CurrentVersion )
        problems.push_back( "layout version " + CORE::UInt32ToString( version ) + " is newer than " + CORE::UInt32ToString( CurrentVersion ) );

    UInt32 widgets = 0;
    CORE::CDataNode::const_iterator i = root->ConstBegin();
    while ( i != root->ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Widget" )
        {
            ++widgets;
            ValidateWidget( *child, CString::Empty, problems );
        }
    }
    if ( 0 == widgets )
        problems.push_back( "the layout has no widget" );
    return problems.size() == before;
}

/*-------------------------------------------------------------------------*/

UInt32
CFormLayout::SimplifyWidget( CORE::CDataNode& widgetNode )
{GUCEF_TRACE;

    UInt32 removed = 0;
    CString type = widgetNode.GetAttributeValue( "type" ).AsString();
    GUCEF::vector< CORE::CDataNode* > drop;
    CORE::CDataNode::iterator i = widgetNode.Begin();
    while ( i != widgetNode.End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Property" )
        {
            const CPropertyDescriptor* descriptor = FindDescriptor( type, child->GetAttributeValue( "name" ).AsString() );
            CString value, normalizedValue, normalizedDefault;
            ReadPropertyValue( *child, value );
            if ( GUCEF_NULL != descriptor && ( value == descriptor->defaultValue ||
                 ( descriptor->Validate( value, normalizedValue ) && descriptor->Validate( descriptor->defaultValue, normalizedDefault ) && normalizedValue == normalizedDefault ) ) )
                drop.push_back( child );
        }
        else
        if ( child->GetName() == "Widget" )
            removed += SimplifyWidget( *child );
    }
    for ( UInt32 d=0; d<(UInt32)drop.size(); ++d )
        GUCEF_DELETE drop[ d ];
    removed += (UInt32) drop.size();

    // a position at the origin is the default
    CString position = widgetNode.GetAttributeValue( "position" ).AsString();
    CUVector vec;
    if ( !position.IsNULLOrEmpty() && CUVector::FromString( position, vec ) && vec == CUVector() )
    {
        widgetNode.DelAttribute( "position" );
        ++removed;
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

UInt32
CFormLayout::Simplify( CORE::CDataNode& layoutRoot )
{GUCEF_TRACE;

    CORE::CDataNode* root = FindLayoutRoot( layoutRoot );
    if ( GUCEF_NULL == root )
        return 0;
    UInt32 removed = 0;
    CORE::CDataNode::iterator i = root->Begin();
    while ( i != root->End() )
    {
        CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Widget" )
            removed += SimplifyWidget( *child );
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

bool
CFormLayout::Migrate( CORE::CDataNode& layoutRoot, TStringVector& notes )
{GUCEF_TRACE;

    CORE::CDataNode* root = FindLayoutRoot( layoutRoot );
    if ( GUCEF_NULL == root )
        return false;
    const CORE::CDataNode::TKeyValuePair* versionAttribute = root->GetAttribute( "version" );
    UInt32 version = GUCEF_NULL == versionAttribute ? 0 : CORE::StringToUInt32( versionAttribute->second.AsString(), 0 );
    if ( version > CurrentVersion )
    {
        notes.push_back( "layout version " + CORE::UInt32ToString( version ) + " is newer than this gucefGUI (" + CORE::UInt32ToString( CurrentVersion ) + "), left as it is" );
        return false;
    }
    if ( 0 == version )
    {
        // a layout without a version is read as version 1, the first one
        notes.push_back( "added version 1" );
    }
    root->SetAttribute( "version", CORE::UInt32ToString( CurrentVersion ) );
    return true;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
