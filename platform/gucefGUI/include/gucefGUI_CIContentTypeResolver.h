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
#ifndef GUCEF_GUI_CICONTENTTYPERESOLVER_H
#define GUCEF_GUI_CICONTENTTYPERESOLVER_H

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
 *  What the application says about the kind of the things a CContentBrowserForm lists: a type name for a path (shown in
 *  the type column, and what the type filter of the form picks from) and the name of a picture for it, a registered
 *  image of the GUI context (CGUIContext::RegisterImage()), shown in a tile of the thumbnail view until a thumbnail
 *  arrives, or instead of one. The form asks for every entry it lists, on the GUI thread: answer from what the path and
 *  the extension say, without reading the content.
 *
 *  The resolver is not owned by the form and has to be there for as long as the form has it (SetContentTypeResolver( NULL )
 *  lets go).
 */
class GUCEF_GUI_PUBLIC_CPP CIContentTypeResolver
{
    public:

    CIContentTypeResolver( void );

    virtual ~CIContentTypeResolver();

    /** The name of the type of what the path names, "Texture" or "Folder": what a person reads; empty for no opinion (the form says what it knows of the extension) */
    virtual CString GetTypeName( const CString& path, bool isDirectory ) = 0;

    /** The name of a registered image for the type, empty for none */
    virtual CString GetIconName( const CString& path, bool isDirectory );

    /**
     *  Whether the file is a sidecar: a metadata file that belongs to another file (a Unity .meta, a Godot .import). The content browser
     *  hides sidecars unless it is told to show them (SetShowSidecars()). The default has none.
     */
    virtual bool IsSidecar( const CString& path, bool isDirectory );

    private:

    CIContentTypeResolver( const CIContentTypeResolver& src );
    CIContentTypeResolver& operator=( const CIContentTypeResolver& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CICONTENTTYPERESOLVER_H ? */
