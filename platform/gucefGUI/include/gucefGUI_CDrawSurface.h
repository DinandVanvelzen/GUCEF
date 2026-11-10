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

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#define GUCEF_GUI_CDRAWSURFACE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  A widget the application draws on with simple 2D commands: rectangles, lines, (parts of) images,
 *  text, points and clipping. The commands are kept as a list; every driver turns the list into its own
 *  drawing, so the same code draws on every backend. Coordinates are relative to the widget.
 *
 *  Typical use, every frame or when something changes: Clear(), the drawing calls, Commit().
 *  Properties: BackgroundColour, Clip (true: nothing is drawn outside the widget).
 */
class GUCEF_GUI_PUBLIC_CPP CDrawSurface : public CWidget
{
    public:

    enum ECommandType
    {
        COMMAND_FILLRECT    = 0 ,
        COMMAND_STROKERECT  = 1 ,
        COMMAND_LINE        = 2 ,
        COMMAND_IMAGE       = 3 ,
        COMMAND_TEXT        = 4 ,
        COMMAND_POINTS      = 5 ,
        COMMAND_PUSHCLIP    = 6 ,
        COMMAND_POPCLIP     = 7
    };

    struct SPoint
    {
        Int32 x;
        Int32 y;
    };
    typedef GUCEF::vector< struct SPoint > TPointVector;

    struct SCommand
    {
        ECommandType type;
        CRect rect;             /**< fill, stroke, the target of an image, the clip */
        CRect source;           /**< the part of the image, empty for all of it */
        Int32 x0;               /**< a line from (x0,y0) to (x1,y1); the pen position of text */
        Int32 y0;
        Int32 x1;
        Int32 y1;
        Int32 thickness;
        CColour colour;         /**< colour of fills, lines, points and text; the tint of an image */
        CString image;          /**< image name (registered with the context, or a VFS path) */
        CString font;
        CString text;
        TPointVector points;

        SCommand( void );
    };
    typedef struct SCommand TCommand;
    typedef GUCEF::vector< TCommand > TCommandVector;

    CDrawSurface( void );

    CDrawSurface( const CString& widgetTypeName );

    virtual ~CDrawSurface();

    /** Starts a new drawing */
    void Clear( void );

    void FillRect( const CRect& rect, const CColour& colour );

    void StrokeRect( const CRect& rect, const CColour& colour, Int32 thickness = 1 );

    void DrawLine( Int32 x0, Int32 y0, Int32 x1, Int32 y1, const CColour& colour, Int32 thickness = 1 );

    /** Draws the 'source' part of the image (all of it when empty) into 'target', scaled to fit, multiplied by 'tint' */
    void DrawImage( const CString& image, const CRect& target, const CRect& source = CRect(), const CColour& tint = CColour( 255, 255, 255, 255 ) );

    /** Draws one line of text with its top left at (x,y) */
    void DrawString( const CString& font, const CString& text, Int32 x, Int32 y, const CColour& colour );

    /** Single pixels in one colour */
    void DrawPoints( const TPointVector& points, const CColour& colour );

    void PushClip( const CRect& rect );

    void PopClip( void );

    /** Hands the drawing to the driver; until then the previous one stays */
    void Commit( void );

    /** The committed drawing */
    const TCommandVector& GetCommands( void ) const;

    /** Goes up with every Commit() */
    UInt32 GetRevision( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CWidget* Create( void );

    private:

    CDrawSurface( const CDrawSurface& src );
    CDrawSurface& operator=( const CDrawSurface& src );

    private:

    TCommandVector m_building;
    TCommandVector m_committed;
    UInt32 m_revision;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */
