/*
 *  guidriverImGui_TestApp: tests of the gucefGUI driver library of Dear ImGui
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

#ifndef GUIDRIVERIMGUI_TESTSUPPORT_H
#define GUIDRIVERIMGUI_TESTSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUIDRIVERIMGUI_H
#include "guidriverImGui.h"
#define GUIDRIVERIMGUI_H
#endif /* GUIDRIVERIMGUI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUITEST {

typedef CORE::CString CString;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A Dear ImGui driver without a GPU renderer: its contexts serve textures in memory and capture with the software rasterizer */
class CTestImGuiDriver : public GUIDRIVERIMGUI::CImGuiDriver
{
    public:

    CTestImGuiDriver( void );

    virtual ~CTestImGuiDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverName( void ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  A window without a screen of 640 x 480 pixels that records what the GUI layers ask of the pointer of the system and
 *  that can be activated and deactivated
 */
class CRecordingWindow : public GUI::CWindowContext
{
    public:

    UInt32 shapeCalls;
    GUI::TCursorShape lastShape;
    bool supportsShapes;
    bool active;

    CRecordingWindow( void );

    virtual UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetProperty( const CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSystemCursorVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSystemCursorShape( GUI::TCursorShape shape ) GUCEF_VIRTUAL_OVERRIDE;

    /** The window becomes active or not, and says so as the window drivers do */
    void SetActive( bool isActive );
};

/*-------------------------------------------------------------------------*/

/** A frame builder that calls a function of a test in every frame */
class CTestFrameBuilder : public GUIDRIVERIMGUI::CImGuiFrameBuilder
{
    public:

    typedef void ( *TBuildFunction )( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs, void* userData );

    CTestFrameBuilder( TBuildFunction function, void* userData );

    void SetFunction( TBuildFunction function, void* userData );

    protected:

    virtual void OnBuildFrame( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    TBuildFunction m_function;
    void* m_userData;
};

/*-------------------------------------------------------------------------*/

/** Counts the events of the notifiers it observes */
class CEventCounter : public CORE::CObserver
{
    public:

    typedef GUCEF::map< CORE::CEvent, UInt32 > TCountMap;

    TCountMap counts;

    CEventCounter( void );

    UInt32 Count( const CORE::CEvent& eventId ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The driver of the tests, made at the first use */
CTestImGuiDriver& GetTestDriver( void );

/** Destroys the driver of the tests, at the end */
void DestroyTestDriver( void );

/** Loads a plugin from the directory of the test application */
bool LoadTestPlugin( const char* moduleName, const char* pluginGroup );

/** The FreeType font rasterizer, which every ImGui context needs */
bool LoadFreeTypeFontPlugin( void );

/** An ImGui context of 640 x 480 logical pixels on the given window (none by default); NULL when it cannot be made */
GUI::TGuiContextPtr CreateImGuiContext( GUI::TWindowContextPtr window = GUI::TWindowContextPtr() );

/** The ImGui context of a context pointer */
GUIDRIVERIMGUI::CImGuiContext* AsImGui( const GUI::TGuiContextPtr& context );

IMAGE::TPixelMapPtr MakeSolidImage( UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a );

/** A white atlas with one cell per glyph for the printable ASCII code points; every glyph is a full block */
GUI::TBitmapFontPtr MakeTestFont( Int32 width, Int32 height );

GUI::CColour PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y );

/** Same colour, give or take a little */
bool Near( const GUI::CColour& a, const GUI::CColour& b, Int32 tolerance );

/** The test suites */
void PerformImGuiModuleTests( void );
void PerformImGuiFrameTests( void );
void PerformImGuiInputTests( void );
void PerformImGuiFontTests( void );
void PerformImGuiTextureTests( void );
void PerformImGuiRenderTests( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUITEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_TESTSUPPORT_H ? */
