/*
 *  AssetViewerLib: renders assets from fixed views into contact sheets and thumbnails, with or without a window on the screen
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

#ifndef ASSETVIEWERLIB_CASSETCAPTURE_H
#define ASSETVIEWERLIB_CASSETCAPTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERSTAGE_H
#include "AssetViewerLib_CViewerStage.h"
#define ASSETVIEWERLIB_CVIEWERSTAGE_H
#endif /* ASSETVIEWERLIB_CVIEWERSTAGE_H ? */

#ifndef ASSETVIEWERLIB_CMODELVIEW_H
#include "AssetViewerLib_CModelView.h"
#define ASSETVIEWERLIB_CMODELVIEW_H
#endif /* ASSETVIEWERLIB_CMODELVIEW_H ? */

#ifndef ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#include "AssetViewerLib_CAssetTypeResolver.h"
#define ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#endif /* ASSETVIEWERLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETVIEWERLIB_CASSETMOUNTER_H
#include "AssetViewerLib_CAssetMounter.h"
#define ASSETVIEWERLIB_CASSETMOUNTER_H
#endif /* ASSETVIEWERLIB_CASSETMOUNTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The render path that the capture of the viewer (--capture) and its thumbnails share. It has a stage of its own, draws it
 *  inside a frame of an Ogre window (the window of the application, an offscreen one when there is no display) and reads the
 *  picture back; it needs no GUI and no window on the screen.
 *
 *  A contact sheet is the asset seen from four fixed views, in a grid of two by two tiles with a margin around and between them:
 *  front and side in the top row, top and three quarter below. A model is framed to fill its tile. Scenes and materials
 *  (on a sphere) are captured the same way; a texture is the picture itself, scaled to the sheet.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CAssetCapture
{
    public:

    enum ELayout
    {
        VIEWS_PER_SHEET = 4 ,
        SHEET_MARGIN = 4        /**< pixels around the tiles and between them */
    };

    /** The window whose frames draw the stage; NULL draws the frames with the Root of Ogre alone (frame listeners run, no window is drawn) */
    explicit CAssetCapture( GUIDRIVEROGRE::COgreWindowContext* window );

    ~CAssetCapture();

    /** Makes the stage with square tiles of this size */
    bool Initialize( UInt32 tileSize );

    void Shutdown( void );

    bool IsInitialized( void ) const;

    UInt32 GetTileSize( void ) const;

    CViewerStage& GetStage( void );

    /** The width and height of the sheet for tiles of this size */
    static UInt32 GetSheetSize( UInt32 tileSize );

    /** The top left pixel of tile 0 to 3 (front, side, top, three quarter) on the sheet */
    static void GetTileOrigin( UInt32 tileSize, UInt32 tileIndex, UInt32& x, UInt32& y );

    /** The colour of the sheet between the tiles */
    static void GetSheetColour( UInt8& red, UInt8& green, UInt8& blue );

    /** Draws the stage in a frame and reads the picture back */
    bool RenderStage( IMAGE::TPixelMapPtr& pixels );

    /** The four views of the mesh of a resource group, on a sheet */
    bool CaptureMeshSheet( const CORE::CString& groupName, const CORE::CString& meshName, IMAGE::TPixelMapPtr& sheet );

    /** The four views of whatever is in the stage already (the model view and the framing are the caller's) */
    bool CaptureViewsOf( CModelView* model, COrbitController& controller, IMAGE::TPixelMapPtr& sheet );

    /** The contact sheet of an asset of the mounts: a model, scene, material or texture; false for other kinds */
    bool CaptureContactSheet( CAssetMounter& mounter, const CORE::CString& vfsPath, IMAGE::TPixelMapPtr& sheet );

    /** One picture of an asset whose longest side is at most 'size': three quarter view of a model, scene or material, or the texture scaled */
    bool CaptureThumbnail( CAssetMounter& mounter, const CORE::CString& vfsPath, UInt32 size, IMAGE::TPixelMapPtr& pixels );

    /** Writes a picture as PNG to a file of the disk; false with a logged reason */
    static bool WritePng( const IMAGE::TPixelMapPtr& pixels, const CORE::CString& path );

    /** The name of the sheet file of an asset: its path with the separators turned into '_', and ".png" */
    static CORE::CString MakeSheetFileName( const CORE::CString& vfsPath );

    private:

    CAssetCapture( const CAssetCapture& src );
    CAssetCapture& operator=( const CAssetCapture& src );

    bool CaptureTextureSheet( const CORE::CString& vfsPath, UInt32 size, bool square, IMAGE::TPixelMapPtr& pixels );
    bool CaptureNonTexture( CAssetMounter& mounter, const CORE::CString& vfsPath, EAssetKind kind, bool sheetNotThumbnail, UInt32 thumbnailSize, IMAGE::TPixelMapPtr& result );

    private:

    GUIDRIVEROGRE::COgreWindowContext* m_window;
    CViewerStage m_stage;
    UInt32 m_tileSize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CASSETCAPTURE_H ? */
