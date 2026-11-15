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

#ifndef GUCEF_GUI_CLAYOUTDEBUGOVERLAY_H
#define GUCEF_GUI_CLAYOUTDEBUGOVERLAY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINT_H
#include "gucefGUI_CLayoutLint.h"
#define GUCEF_GUI_CLAYOUTLINT_H
#endif /* GUCEF_GUI_CLAYOUTLINT_H ? */

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
 *  The layout lint while the application runs: every context has one (CGUIContext::GetLayoutDebugOverlay()), off by
 *  default. On, it runs the lint after a layout pass (and at most once a second otherwise, for text that changed without
 *  a layout) and, in overlay mode, draws the findings over everything: an outline in red for errors and amber for
 *  warnings, a band over the part of a text that is cut, and the rule id. In log mode it writes the findings to the log
 *  once per change of what it found, never per frame.
 *
 *  Switched on with SetMode(), with the environment variable GUCEF_GUI_LAYOUT_LINT ("overlay", "log", "overlay,log";
 *  "on" is "overlay") when the context is made, or with the developer console command "gui.lint on|off|log|overlay".
 *  Nothing runs while it is off.
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutDebugOverlay
{
    public:

    enum EMode
    {
        MODE_OFF        = 0 ,
        MODE_OVERLAY    = 1 ,
        MODE_LOG        = 2
    };

    /** How often the lint runs again without a layout pass, in milliseconds of context time */
    enum { RelintIntervalInMs = 1000 };

    CLayoutDebugOverlay( void );

    /** MODE_* flags */
    void SetMode( UInt32 mode );
    UInt32 GetMode( void ) const;
    bool IsOn( void ) const;

    /** The mode a text names: "off", "on", "overlay", "log", "overlay,log" (also "all", "both"); false for a text it cannot read */
    static bool ParseMode( const CString& text, UInt32& mode );

    /** Sets the mode from GUCEF_GUI_LAYOUT_LINT when it is set; false when it is not set or cannot be read */
    bool ApplyEnvironment( void );

    /** The options of the lint runs (the defaults; layoutFirst is off, the overlay runs inside a frame) */
    CLayoutLintOptions& GetOptions( void );

    /** Runs the lint when it is due and adds the drawing of the findings (overlay mode); called by CWidgetPainter::PaintOverlay() */
    void Paint( CGUIContext& context, CDrawSurface::TCommandVector& commands );

    /** What the last run found */
    const TLayoutLintFindingVector& GetFindings( void ) const;

    /** Makes the next Paint() run the lint */
    void Invalidate( void );

    private:

    void Relint( CGUIContext& context );

    UInt32 m_mode;
    CLayoutLint m_lint;
    CLayoutLintOptions m_options;
    TLayoutLintFindingVector m_findings;
    bool m_dirty;
    UInt32 m_lintedRevision;
    UInt64 m_lintedTime;
    CString m_loggedReport;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTDEBUGOVERLAY_H ? */
