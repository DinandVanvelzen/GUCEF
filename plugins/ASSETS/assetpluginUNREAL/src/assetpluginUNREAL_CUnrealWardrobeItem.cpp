/*
 *  assetpluginUNREAL: a wardrobe item of a MetaHuman character: the outfit it puts on and its hidden face map
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H
#include "assetpluginUNREAL_CUnrealWardrobeItem.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALWARDROBEITEM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The first node with the name in the tree below 'node' (depth first), NULL when there is none */
static const CORE::CDataNode*
FindBelow( const CORE::CDataNode& node, const CORE::CString& name, UInt32 depth )
{GUCEF_TRACE;

    if ( depth > 12 )
        return GUCEF_NULL;
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
    {
        if ( (*i)->GetName() == name )
            return *i;
        const CORE::CDataNode* found = FindBelow( *(*i), name, depth + 1 );
        if ( GUCEF_NULL != found )
            return found;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** The object path a property holds: the resolved path of an object reference, or the text of a soft object path */
static CORE::CString
ObjectPathIn( const CORE::CDataNode& node, UInt32 depth )
{GUCEF_TRACE;

    const CORE::CString path = node.GetAttributeValue( "path" ).AsString();
    if ( !path.IsNULLOrEmpty() )
        return path;
    const CORE::CString value = node.GetValue().AsString();
    if ( value.Length() > 1 && '/' == value[ 0 ] )
        return value;
    if ( depth > 6 )
        return CORE::CString::Empty;
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
    {
        const CORE::CString below = ObjectPathIn( *(*i), depth + 1 );
        if ( !below.IsNULLOrEmpty() )
            return below;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

static void
ReadSetting( const CORE::CDataNode& settings, const char* name, bool& has, Float32& value )
{GUCEF_TRACE;

    const CORE::CDataNode* node = FindBelow( settings, name, 0 );
    if ( GUCEF_NULL != node && !node->GetValue().IsNULLOrEmpty() )
    {
        has = true;
        value = node->GetValue().AsFloat32( value );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealWardrobeItem::CUnrealWardrobeItem( void )
    : name()
    , packageName()
    , principalAsset()
    , bodyHiddenFaceMap()
    , hasMaxCullValue( false )
    , maxCullValue( 0.0f )
    , hasMinKeepValue( false )
    , minKeepValue( 0.0f )
    , hasMaxShrinkDistance( false )
    , maxShrinkDistance( 0.0f )
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealWardrobeItem::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealWardrobeItem::Read( CUnrealPackage& package, UInt32 exportIndex )
{GUCEF_TRACE;

    *this = CUnrealWardrobeItem();
    const CUnrealPackage::TExportVector& exports = package.GetExports();
    if ( exportIndex >= exports.size() )
    {
        m_error = "The export does not exist";
        return false;
    }
    name = exports[ exportIndex ].objectName;
    packageName = package.GetSummary().packageName;

    // the item, then the pipelines below it (the editor pipeline of an outfit names the hidden face map)
    for ( size_t e=0; e<exports.size(); ++e )
    {
        const CORE::CString className = package.GetClassName( exports[ e ] );
        if ( e != exportIndex && className.HasSubstr( "Pipeline" ) < 0 )
            continue;
        CUnrealArchiveReader reader;
        if ( !package.OpenExportRange( (UInt32) e, reader ) )
        {
            if ( e == exportIndex )
            {
                m_error = "The data of the wardrobe item cannot be opened";
                return false;
            }
            continue;
        }
        CORE::CDataNode root( "root" );
        CUnrealPropertyReader properties( package );
        if ( !properties.ReadExport( reader, root, className ) )
        {
            if ( e == exportIndex )
            {
                m_error = "The properties of the wardrobe item cannot be read: " + reader.GetError();
                return false;
            }
            continue;
        }
        if ( e == exportIndex )
        {
            const CORE::CDataNode* principal = FindBelow( root, "PrincipalAsset", 0 );
            if ( GUCEF_NULL != principal )
                principalAsset = ObjectPathIn( *principal, 0 );
        }
        // UE 5.8: the struct BodyHiddenFaceMapTexture (texture and settings); UE 5.6: the texture as BodyHiddenFaceMap
        const CORE::CDataNode* map = FindBelow( root, "BodyHiddenFaceMapTexture", 0 );
        if ( GUCEF_NULL == map )
            map = FindBelow( root, "BodyHiddenFaceMap", 0 );
        if ( GUCEF_NULL != map && bodyHiddenFaceMap.IsNULLOrEmpty() )
        {
            const CORE::CDataNode* texture = FindBelow( *map, "Texture", 0 );
            bodyHiddenFaceMap = ObjectPathIn( GUCEF_NULL != texture ? *texture : *map, 0 );
            ReadSetting( *map, "MaxCullValue", hasMaxCullValue, maxCullValue );
            ReadSetting( *map, "MinKeepValue", hasMinKeepValue, minKeepValue );
            ReadSetting( *map, "MaxShrinkDistance", hasMaxShrinkDistance, maxShrinkDistance );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
