/*
 *  guidriverImGuiOpenGL_TestApp: tests of the gucefGUI driver ImGui (OpenGL)
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

#ifndef GUIDRIVERIMGUIGL_TESTSCENES_H
#define GUIDRIVERIMGUIGL_TESTSCENES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "guidriverImGuiOpenGL_TestSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIGLTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

extern const GUI::CColour Black;
extern const GUI::CColour Red;
extern const GUI::CColour Green;
extern const GUI::CColour White;
extern const GUI::CColour Yellow;

/** A picture with a different colour for every pixel */
IMAGE::TPixelMapPtr MakeGradientImage( UInt32 width, UInt32 height );

/** The picture blown up 'factor' times with every pixel a block */
IMAGE::TPixelMapPtr Blocks( const IMAGE::CPixelMap& source, UInt32 factor );

/** A root widget over the whole context, shown */
GUI::CWidget* Screen( GUI::CGUIContext* context );

/** Panels, borders, clipping, opacity, scaled and tinted pictures, text, lines and points in one picture (as gucefGUI_TestApp draws it) */
void BuildMixedScene( GUI::CGUIContext* context );

/** At a render scale of 2: pictures and a font of density 2 next to ones of density 1, a fill, a line and text */
void BuildDensityScene( GUI::CGUIContext* context );

/** The pixels that differ, outside an optional rectangle that is left out; logs the first difference */
UInt32 CountDifferences( const IMAGE::TPixelMapPtr& expected, const IMAGE::TPixelMapPtr& actual, Int32 tolerance, const GUI::CRect& leftOut, const char* what );

/**
 *  The pixels that differ by more than the tolerance in some channel, as a per mille of all pixels; 'maxDifference' is the
 *  largest difference of one channel. 1000 when the pictures cannot be compared
 */
UInt32 DifferentPerMille( const IMAGE::TPixelMapPtr& expected, const IMAGE::TPixelMapPtr& actual, Int32 tolerance, UInt32& maxDifference );

/** A window with the usual widgets of Dear ImGui, the same in every context; needs the image "Checker" (BuildMixedScene) */
void BuildScriptedWindow( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs, void* userData );

/** Drawing straight into the foreground draw list, to test the rules of the rasterizers; needs the image "Ramp" */
void BuildRasterizerProbes( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs, void* userData );

/** More vertices than 16-bit indices can address */
void BuildManyRects( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs, void* userData );

/** Fills a registered image of the context with one colour in place and tells the context that it changed */
bool RecolourImage( GUI::CGUIContext* context, const char* alias, UInt8 r, UInt8 g, UInt8 b, UInt8 a );

/** What BuildButtonWindow() recorded */
struct SButtonState
{
    float minX, minY, maxX, maxY;   /**< the rectangle of the button, in logical pixels */
    bool hovered;
    UInt32 clicks;
    UInt32 frames;

    SButtonState( void ) : minX( 0 ), minY( 0 ), maxX( 0 ), maxY( 0 ), hovered( false ), clicks( 0 ), frames( 0 ) {}
};

/** A window with one button at the top left corner of the picture, which records where it is, whether it is hovered and how often it was clicked; userData is a SButtonState */
void BuildButtonWindow( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs, void* userData );

/** Fills only (no text, no pictures): panels with borders, a clipped child, an opacity, the retained scene whose edges a present mode shows exactly */
void BuildFillScene( GUI::CGUIContext* context );

/** Compares the draw data of two contexts: lists, vertices, indices, commands and clip rectangles */
bool SameDrawData( const ImDrawData& a, const ImDrawData& b );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIGLTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_TESTSCENES_H ? */
