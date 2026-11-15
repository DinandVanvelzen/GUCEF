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

#ifndef GUCEF_GUI_CTOOLTIPSERVICE_H
#define GUCEF_GUI_CTOOLTIPSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_CIHELPSOURCE_H
#include "gucefGUI_CIHelpSource.h"
#define GUCEF_GUI_CIHELPSOURCE_H
#endif /* GUCEF_GUI_CIHELPSOURCE_H ? */

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

class CWidget;

/**
 *  The tooltips of one GUI context (CGUIContext::GetTooltipService()): when the pointer has rested on a part of a widget
 *  for the show delay, the overlay pass of the painter draws the tooltip of that part, one at a time.
 *
 *  Rest: a move of the pointer while no tooltip shows, and a change of the widget under it, start the rest again. A mouse
 *  button, the wheel or a key hides the tooltip until the pointer moves again. While a tooltip shows, a move within the
 *  widget keeps it shown and its text follows the part under the pointer. The time is the time of the context (the sum of
 *  the frame times), so a test drives it with RenderFrame().
 *
 *  Text: the tooltip of the widget's part (CWidget::GetTooltipAt()), else the brief of the help topic of the part
 *  (CWidget::GetHelpTopicAt(), else the HelpTopic property of the nearest ancestor that has one) from the help source; when
 *  that topic has a long explainer in the source and a hint is set, the hint is the last line. A widget that shows its own
 *  text shortened with "..." (a label, a button, a check box, a combo box, a group box caption, a window title, see
 *  CWidgetPainter::IsOwnTextElided()) has its whole text as the tooltip, above its own tooltip when it has one.
 */
class GUCEF_GUI_PUBLIC_CPP CTooltipService
{
    public:

    enum EDefaults
    {
        DefaultShowDelayInMs = 1000 ,
        DefaultMaxWidth = 480       ,   /**< logical pixels, the box included */
        DefaultOffsetX = 12         ,   /**< the box starts this far right of the pointer */
        DefaultOffsetY = 16         ,   /**< and this far below it */
        DefaultPaddingX = 4         ,
        DefaultPaddingY = 2
    };

    CTooltipService( void );

    /*
     *  Settings
     */

    /** Off: no tooltip shows; the widgets still answer GetTooltipAt() */
    void SetEnabled( bool enabled );
    bool IsEnabled( void ) const;

    /** How long the pointer rests before a tooltip shows, in milliseconds of context time; the tooltip never shows earlier */
    void SetShowDelayInMs( UInt32 delayInMs );
    UInt32 GetShowDelayInMs( void ) const;

    /** The widest a tooltip box is, in logical pixels; longer text wraps. Never wider than the picture */
    void SetMaxWidth( Int32 maxWidth );
    Int32 GetMaxWidth( void ) const;

    void SetColours( const CColour& background, const CColour& border, const CColour& text );
    const CColour& GetBackgroundColour( void ) const;
    const CColour& GetBorderColour( void ) const;
    const CColour& GetTextColour( void ) const;

    /** Where the briefs of help topics come from; NULL: only the tooltips of the widgets */
    void SetHelpSource( const THelpSourcePtr& source );
    const THelpSourcePtr& GetHelpSource( void ) const;

    /** The line added under a tooltip whose help topic has a long explainer (for example "F1: more about this"); empty: none */
    void SetMoreHint( const CString& hint );
    const CString& GetMoreHint( void ) const;

    /*
     *  State, told by the context
     */

    /** The pointer moved: while no tooltip shows the rest starts again; a suppression by input ends */
    void OnPointerMoved( UInt64 nowInMs );

    /** The widget under the pointer changed: the rest starts again and no tooltip shows */
    void OnHoverChanged( UInt64 nowInMs );

    /** A mouse button, the wheel or a key: the tooltip hides until the pointer moves again */
    void OnInputPressed( void );

    /** Whether a tooltip may show now: enabled, not suppressed, and rested for the delay */
    bool IsDue( UInt64 nowInMs ) const;

    /** What the last overlay pass did: whether it drew a tooltip */
    void SetShowing( bool showing );
    bool IsShowing( void ) const;

    /** When the pointer started to rest, in context time */
    UInt64 GetRestStartInMs( void ) const;

    /*
     *  Content
     */

    /** The help topic of the point of the widget: GetHelpTopicAt(), else the HelpTopic property of the nearest ancestor */
    CString FindHelpTopicAt( const CWidget& widget, Int32 localX, Int32 localY ) const;

    /** The text of the tooltip at the point of the widget (relative to its top left), as described for the class; empty for none */
    CString GetTooltipText( const CWidget& widget, Int32 localX, Int32 localY ) const;

    /**
     *  The document a reader opens for a help id: the id when the source has a document of it, else the topic its brief names
     *  when the source has that document; empty without a source or a document
     */
    CString ResolveTopic( const CString& helpId ) const;

    private:

    bool m_enabled;
    UInt32 m_showDelayInMs;
    Int32 m_maxWidth;
    CColour m_background;
    CColour m_border;
    CColour m_text;
    THelpSourcePtr m_helpSource;
    CString m_moreHint;
    UInt64 m_restStartInMs;
    bool m_suppressed;
    bool m_showing;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTOOLTIPSERVICE_H ? */
