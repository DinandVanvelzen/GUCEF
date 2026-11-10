/*
 * Copyright (C) Dinand Vanvelzen. 2002 - 2005.  All rights reserved.
 *
 * All source code herein is the property of Dinand Vanvelzen. You may not sell
 * or otherwise commercially exploit the source or things you created based on
 * the source.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL DINAND VANVELZEN BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF 
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT 
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windowsx.h>
#include <windows.h>
#include <winuser.h>

#include "DLLMainInputDriverMSWINMSG.h"

#ifndef GUCEFINPUT_MACROS_H
#include "gucefINPUT_macros.h"    /* gucefINPUT macros, used here for the export and callspec macros */
#define GUCEFINPUT_MACROS_H
#endif /* GUCEFINPUT_MACROS_H ? */ 

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace INPUT {
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const TVersion version = { 1, 0, 0, 0 };
#define DRIVER_NAME       "MSWINMSG\0"
#define DRIVER_COPYRIGHT  "Copyright (C) Dinand Vanvelzen. 2002 - 2011. License: LGPLv3.\0"

#define MAX_NR_OF_CONTEXTS      32
#define KEYBOARD_DEVICEID       0
#define MOUSE_DEVICEID          1


/*
 *  The following are key codes which are not available on all windows versions but might still be secretly supported
 *  We add them here so that if the codes are sent out we can deal with them in a uniform manner
 */
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL          0x020A      
#endif
#ifndef VK_BROWSER_BACK
#define VK_BROWSER_BACK        0xA6
#endif
#ifndef VK_BROWSER_FORWARD
#define VK_BROWSER_FORWARD     0xA7
#endif
#ifndef VK_BROWSER_REFRESH
#define VK_BROWSER_REFRESH     0xA8
#endif
#ifndef VK_BROWSER_STOP
#define VK_BROWSER_STOP        0xA9
#endif
#ifndef VK_BROWSER_SEARCH
#define VK_BROWSER_SEARCH      0xAA
#endif
#ifndef VK_BROWSER_FAVORITES
#define VK_BROWSER_FAVORITES   0xAB
#endif
#ifndef VK_BROWSER_HOME
#define VK_BROWSER_HOME        0xAC
#endif
#ifndef VK_VOLUME_MUTE
#define VK_VOLUME_MUTE         0xAD
#endif
#ifndef VK_VOLUME_DOWN
#define VK_VOLUME_DOWN         0xAE
#endif
#ifndef VK_VOLUME_UP
#define VK_VOLUME_UP           0xAF
#endif
#ifndef VK_MEDIA_NEXT_TRACK
#define VK_MEDIA_NEXT_TRACK    0xB0
#endif
#ifndef VK_MEDIA_PREV_TRACK
#define VK_MEDIA_PREV_TRACK    0xB1
#endif
#ifndef VK_MEDIA_STOP
#define VK_MEDIA_STOP          0xB2
#endif
#ifndef VK_MEDIA_PLAY_PAUSE
#define VK_MEDIA_PLAY_PAUSE    0xB3
#endif
#ifndef VK_LAUNCH_MAIL
#define VK_LAUNCH_MAIL         0xB4
#endif
#ifndef VK_LAUNCH_MEDIA_SELECT
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#endif
#ifndef VK_LAUNCH_APP1
#define VK_LAUNCH_APP1         0xB6
#endif
#ifndef VK_LAUNCH_APP2
#define VK_LAUNCH_APP2         0xB7
#endif
#ifndef VK_XBUTTON1
#define VK_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#endif
#ifndef VK_XBUTTON2
#define VK_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */
#endif
#ifndef VK_YEN
#define VK_YEN       13
#endif
#ifndef VK_GRAVE
#define VK_GRAVE    192
#endif
#ifndef VK_POWER
#define VK_POWER    WM_POWER
#endif

/*
 *  Fake key for testing
 */
#ifndef VK_UNKNOWN
#define VK_UNKNOWN  0x00
#endif

/*
 *  The following are key codes for the ASCII values
 */
#ifndef VK_0
#define VK_0    '0'
#endif
#ifndef VK_1
#define VK_1    '1'
#endif
#ifndef VK_2
#define VK_2    '2'
#endif
#ifndef VK_3
#define VK_3    '3'
#endif
#ifndef VK_4
#define VK_4    '4'
#endif
#ifndef VK_5
#define VK_5    '5'
#endif
#ifndef VK_6
#define VK_6    '6'
#endif
#ifndef VK_7
#define VK_7    '7'
#endif
#ifndef VK_8
#define VK_8    '8'
#endif
#ifndef VK_9
#define VK_9    '9'
#endif
#ifndef VK_A
#define VK_A    'A'
#endif
#ifndef VK_B
#define VK_B    'B'
#endif
#ifndef VK_C
#define VK_C    'C'
#endif
#ifndef VK_D
#define VK_D    'D'
#endif
#ifndef VK_E
#define VK_E    'E'
#endif
#ifndef VK_F
#define VK_F    'F'
#endif
#ifndef VK_G
#define VK_G    'G'
#endif
#ifndef VK_H
#define VK_H    'H'
#endif
#ifndef VK_I
#define VK_I    'I'
#endif
#ifndef VK_J
#define VK_J    'J'
#endif
#ifndef VK_K
#define VK_K    'K'
#endif
#ifndef VK_L
#define VK_L    'L'
#endif
#ifndef VK_M
#define VK_M    'M'
#endif
#ifndef VK_N
#define VK_N    'N'
#endif
#ifndef VK_O
#define VK_O    'O'
#endif
#ifndef VK_P
#define VK_P    'P'
#endif
#ifndef VK_Q
#define VK_Q    'Q'
#endif
#ifndef VK_R
#define VK_R    'R'
#endif
#ifndef VK_S
#define VK_S    'S'
#endif
#ifndef VK_T
#define VK_T    'T'
#endif
#ifndef VK_U
#define VK_U    'U'
#endif
#ifndef VK_V
#define VK_V    'V'
#endif
#ifndef VK_W
#define VK_W    'W'
#endif
#ifndef VK_X
#define VK_X    'X'
#endif
#ifndef VK_Y
#define VK_Y    'Y'
#endif
#ifndef VK_Z
#define VK_Z    'Z'
#endif


/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SContextData
{
    HWND hWnd;
    WNDPROC PrevWindowProcedurePtr;
    TInputCallbacks callbacks;
    BYTE inUse;
    BYTE keyStateBuffer[ 256 ];
    UInt32 keyModState;
    Int32 mouseXPos;
    Int32 mouseYPos;
    Int32 mouseWheelPos;           /* the wheel in notches */
    Int32 mouseWheelRemainder;     /* wheel movement short of a notch (high resolution wheels) */
    Int32 mouseHWheelPos;
    Int32 mouseHWheelRemainder;
    WCHAR highSurrogate;           /* the first half of a character outside the BMP typed in two WM_CHAR messages */
    BYTE doGrab;           /* grab the cursor and confine it to the window ? */
    BYTE showCursor;       /* show the O/S cursor ? */
         
};
typedef struct SContextData TContextData;

/*---------------------------------------------------------------------------*/
        
struct SDriverData
{
    TContextData contextList[ MAX_NR_OF_CONTEXTS ];
    UInt32 contextCount;
};
typedef struct SDriverData TDriverData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_64BIT
  #define MSWIN_BITTARGET_GWL_WNDPROC       GWLP_WNDPROC
#else
  #define MSWIN_BITTARGET_GWL_WNDPROC       GWL_WNDPROC
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static TDriverData* driverData = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
GetArgListItem( const int argc    ,
                const char** argv ,
                const char* key   )
{
    int i;
    for ( i=0; i<argc-1; ++i )
    {
        if ( strcmp( argv[ i ], key ) == 0 )
        {
            return argv[ i+1 ];
        }
        ++i;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32
ParseArgListItemUInt32( const int argc    ,
                        const char** argv ,
                        const char* key   )
{
    const char* value = GetArgListItem( argc, argv, key );
    if ( NULL != value )
    {
        UInt32 result = 0UL;
        sscanf( value, "%d", &result );
        return result;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/

void*
ParseArgListItemPointer( const int argc    ,
                         const char** argv ,
                         const char* key   )
{
    const char* value = GetArgListItem( argc, argv, key );
    if ( NULL != value )
    {
        void* result = NULL;
        sscanf( value, "%p", &result );
        return result;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void
UpdateKeyModState( TContextData* data )
{         
    GetKeyState( VK_SHIFT ) & 0x80 ? GUCEF_SETBITXON( data->keyModState, KEYMOD_SHIFT_BIT ) : GUCEF_SETBITXOFF( data->keyModState, KEYMOD_SHIFT_BIT );
    GetKeyState( VK_MENU ) & 0x80 ? GUCEF_SETBITXON( data->keyModState, KEYMOD_ALT_BIT ) : GUCEF_SETBITXOFF( data->keyModState, KEYMOD_ALT_BIT );
    GetKeyState( VK_CONTROL ) & 0x80 ? GUCEF_SETBITXON( data->keyModState, KEYMOD_CTRL_BIT ) : GUCEF_SETBITXOFF( data->keyModState, KEYMOD_CTRL_BIT );     
}

/*---------------------------------------------------------------------------*/

TContextData*
GetFreeContext( void )
{
    if ( driverData->contextCount < MAX_NR_OF_CONTEXTS )
    {
        UInt32 i;
        for ( i=0; i<MAX_NR_OF_CONTEXTS; ++i )
        {
            if ( 0 == driverData->contextList[ i ].inUse )
            {
                 driverData->contextList[ i ].inUse = 1;
                 return &driverData->contextList[ i ];
            }
        }
        return NULL;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

void
ReleaseContext( TContextData* context )
{
    UInt32 i;
    for ( i=0; i<MAX_NR_OF_CONTEXTS; ++i )
    {
        if ( &driverData->contextList[ i ] == context )
        {
            memset( context, 0, sizeof( TContextData ) );
        }
    }
}

/*---------------------------------------------------------------------------*/

TContextData*
LookupDataForHWND( HWND whnd )
{
        UInt32 i, checked = 0;
        for ( i=0; i<MAX_NR_OF_CONTEXTS; ++i )
        {
                if ( 1 == driverData->contextList[ i ].inUse )
                {
                        ++checked;
                        if ( driverData->contextList[ i ].hWnd == whnd )
                        {
                                return &driverData->contextList[ i ];
                        }
                }
                
                if ( checked == driverData->contextCount )
                {
                        return NULL;
                }                        
        }
        return NULL;        
}

/*---------------------------------------------------------------------------*/

BOOL
ClientRectToScreenRect( HWND hwnd, RECT* rect )
{
    POINT screenPos;
    screenPos.x = rect->left;
    screenPos.y = rect->top;
    if ( ClientToScreen( hwnd, &screenPos ) )
    {
	    rect->left = screenPos.x;
	    rect->top = screenPos.y;
	        
            screenPos.x = rect->right;
	    screenPos.y = rect->bottom;
	        
	    if ( ClientToScreen( hwnd, &screenPos ) )
	    {
	        rect->right = screenPos.x;
	        rect->bottom = screenPos.y;
	                
	        return 1;
	    }	        		                
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32
GrabWindowCursorInput( TContextData* data ,
                       const BYTE doGrab  )
{
	if ( data->doGrab == doGrab ) return 0;
	
	if ( doGrab != 0 ) 
	{	        			
		RECT screenRect;
		if ( GetClientRect( data->hWnd, &screenRect ) )
		{
		        /*
		         *      Now we have to translate to window coordinates
		         */
		        if ( ClientRectToScreenRect( data->hWnd, &screenRect ) )
		        { 
		                if ( ClipCursor( &screenRect ) )
		                {
		                        /*      
		                         *      successfull cursor grab ,
		                         *      we now place the cursor in the center of the window
		                         */
		                        POINT cursorPos, screenPos;
		                        cursorPos.x = ( screenRect.right - screenRect.left ) / 2;
		                        cursorPos.y = ( screenRect.bottom - screenRect.top ) / 2;
		                        screenPos = cursorPos; 
		                        if ( ClientToScreen( data->hWnd, &screenPos ) )
		                        {
		                                if ( SetCursorPos( screenPos.x, screenPos.y ) )
		                                {
		                                        data->doGrab = 1;
		                                        
		                                        /*
		                                         *      Send an event to the user so he can adjust for the cursor warp
		                                         */
		                                        data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, cursorPos.x, cursorPos.y, cursorPos.x - data->mouseXPos, cursorPos.y - data->mouseYPos );
                                                        data->mouseXPos = cursorPos.x;
                                                        data->mouseYPos = cursorPos.y;
                                                        
		                                        return 1;
                                                }
                                        }
                                }                                        
		        }
        }
        }
    else
    {
		/*
		 *      Allow the cursor to move anywhere
		 */
		ClipCursor( NULL );		
		data->doGrab = 0;	
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

KeyCode
MapControlKeyToASide( const WPARAM wparam, const LPARAM lparam )
{
    if ( wparam == VK_CONTROL )
    {
        UINT scancode = MapVirtualKey( VK_RCONTROL, MAPVK_VK_TO_VSC );
        if ( ( ( lparam & 0x0011D0001 ) >> 16 ) == scancode )
        {
            return INPUT::KEYCODE_LCONTROL;
        } 
        return INPUT::KEYCODE_RCONTROL;
    }
    return INPUT::KEYCODE_UNASSIGNED;
}

/*---------------------------------------------------------------------------*/

KeyCode
MapShiftKeyToASide( const WPARAM wparam, const LPARAM lparam )
{
    if ( wparam == VK_SHIFT )
    {
        UINT scancode = MapVirtualKey( VK_RSHIFT, MAPVK_VK_TO_VSC );
        if ( ( ( lparam & 0x01FF0000 ) >> 16 ) == scancode )
        {
            return INPUT::KEYCODE_RSHIFT;
        } 
        return INPUT::KEYCODE_LSHIFT;
    }
    return INPUT::KEYCODE_UNASSIGNED;
}

/*---------------------------------------------------------------------------*/

KeyCode
MapKeyScanCode( const WPARAM wparam, const LPARAM lparam )
{
    switch ( wparam )
    {
        case VK_UNKNOWN : return INPUT::KEYCODE_UNASSIGNED;
        case VK_ESCAPE : return INPUT::KEYCODE_ESCAPE;
        case VK_1 : return INPUT::KEYCODE_1;
        case VK_2 : return INPUT::KEYCODE_2;
        case VK_3 : return INPUT::KEYCODE_3;
        case VK_4 : return INPUT::KEYCODE_4;
        case VK_5 : return INPUT::KEYCODE_5;
        case VK_6 : return INPUT::KEYCODE_6;
        case VK_7 : return INPUT::KEYCODE_7;
        case VK_8 : return INPUT::KEYCODE_8;
        case VK_9 : return INPUT::KEYCODE_9;
        case VK_0 : return INPUT::KEYCODE_0;
        case VK_OEM_MINUS : return INPUT::KEYCODE_MINUS;
        case VK_OEM_PLUS : return INPUT::KEYCODE_EQUALS;
        case VK_BACK : return INPUT::KEYCODE_BACK;
        case VK_TAB : return INPUT::KEYCODE_TAB;
        case VK_Q : return INPUT::KEYCODE_Q;
        case VK_W : return INPUT::KEYCODE_W;
        case VK_E : return INPUT::KEYCODE_E;
        case VK_R : return INPUT::KEYCODE_R;
        case VK_T : return INPUT::KEYCODE_T;
        case VK_Y : return INPUT::KEYCODE_Y;
        case VK_U : return INPUT::KEYCODE_U;
        case VK_I : return INPUT::KEYCODE_I;
        case VK_O : return INPUT::KEYCODE_O;
        case VK_P : return INPUT::KEYCODE_P;
        case VK_OEM_4 : return INPUT::KEYCODE_LBRACKET;
        case VK_OEM_6 : return INPUT::KEYCODE_RBRACKET;
        case VK_RETURN : return INPUT::KEYCODE_RETURN;
        case VK_CONTROL : return MapControlKeyToASide( wparam, lparam );
        case VK_A : return INPUT::KEYCODE_A;
        case VK_S : return INPUT::KEYCODE_S;
        case VK_D : return INPUT::KEYCODE_D;
        case VK_F : return INPUT::KEYCODE_F;
        case VK_G : return INPUT::KEYCODE_G;
        case VK_H : return INPUT::KEYCODE_H;
        case VK_J : return INPUT::KEYCODE_J;
        case VK_K : return INPUT::KEYCODE_K;
        case VK_L : return INPUT::KEYCODE_L;
        case VK_OEM_1 : return INPUT::KEYCODE_SEMICOLON;
        case VK_OEM_7 : return INPUT::KEYCODE_APOSTROPHE; 
        case VK_GRAVE : return INPUT::KEYCODE_GRAVE; 
        case VK_SHIFT : return MapShiftKeyToASide( wparam, lparam );
        case VK_OEM_5 : return INPUT::KEYCODE_BACKSLASH; 
        case VK_Z : return INPUT::KEYCODE_Z;
        case VK_X : return INPUT::KEYCODE_X;
        case VK_C : return INPUT::KEYCODE_C;
        case VK_V : return INPUT::KEYCODE_V;
        case VK_B : return INPUT::KEYCODE_B;
        case VK_N : return INPUT::KEYCODE_N;
        case VK_M : return INPUT::KEYCODE_M;
        case VK_OEM_COMMA : return INPUT::KEYCODE_COMMA;
        case VK_OEM_PERIOD : return INPUT::KEYCODE_PERIOD;
        case VK_OEM_2 : return INPUT::KEYCODE_SLASH;
        case VK_MULTIPLY : return INPUT::KEYCODE_MULTIPLY;
        case VK_LMENU : return INPUT::KEYCODE_LMENU;
        case VK_SPACE : return INPUT::KEYCODE_SPACE;
        case VK_CAPITAL : return INPUT::KEYCODE_CAPITAL;
        case VK_F1 : return INPUT::KEYCODE_F1;
        case VK_F2 : return INPUT::KEYCODE_F2;
        case VK_F3 : return INPUT::KEYCODE_F3;
        case VK_F4 : return INPUT::KEYCODE_F4;
        case VK_F5 : return INPUT::KEYCODE_F5;
        case VK_F6 : return INPUT::KEYCODE_F6;
        case VK_F7 : return INPUT::KEYCODE_F7;
        case VK_F8 : return INPUT::KEYCODE_F8;
        case VK_F9 : return INPUT::KEYCODE_F9;
        case VK_F10 : return INPUT::KEYCODE_F10;
        case VK_NUMLOCK : return INPUT::KEYCODE_NUMLOCK;
        case VK_SCROLL : return INPUT::KEYCODE_SCROLL;
        case VK_NUMPAD7 : return INPUT::KEYCODE_NUMPAD7;
        case VK_NUMPAD8 : return INPUT::KEYCODE_NUMPAD8;
        case VK_NUMPAD9 : return INPUT::KEYCODE_NUMPAD9;
        case VK_SUBTRACT : return INPUT::KEYCODE_SUBTRACT;
        case VK_NUMPAD4 : return INPUT::KEYCODE_NUMPAD4;
        case VK_NUMPAD5 : return INPUT::KEYCODE_NUMPAD5;
        case VK_NUMPAD6 : return INPUT::KEYCODE_NUMPAD6;
        case VK_ADD : return INPUT::KEYCODE_ADD;
        case VK_NUMPAD1 : return INPUT::KEYCODE_NUMPAD1;
        case VK_NUMPAD2 : return INPUT::KEYCODE_NUMPAD2;
        case VK_NUMPAD3 : return INPUT::KEYCODE_NUMPAD3;   
        case VK_NUMPAD0 : return INPUT::KEYCODE_NUMPAD0;
        case VK_DECIMAL : return INPUT::KEYCODE_DECIMAL;
        //case VK_LWIN : return INPUT::KEYCODE_LEFTWINKEY;
        //case VK_RWIN : return INPUT::KEYCODE_RIGHTWINKEY;
        case VK_OEM_102 : return INPUT::KEYCODE_OEM_102;
        case VK_F11 : return INPUT::KEYCODE_F11;
        case VK_F12 : return INPUT::KEYCODE_F12;
        case VK_F13 : return INPUT::KEYCODE_F13;
        case VK_F14 : return INPUT::KEYCODE_F14;
        case VK_F15 : return INPUT::KEYCODE_F15;
        case VK_KANA : return INPUT::KEYCODE_KANA;
        //case VK_OEM_2 : return INPUT::KEYCODE_ABNT_C1;
        case VK_CONVERT : return INPUT::KEYCODE_CONVERT;
        case VK_NONCONVERT : return INPUT::KEYCODE_NOCONVERT;
        //case VK_YEN : return INPUT::KEYCODE_YEN;
        //case VK_OEM_PERIOD : return INPUT::KEYCODE_ABNT_C2;
        case VK_OEM_NEC_EQUAL : return INPUT::KEYCODE_NUMPADEQUALS;
        case VK_MEDIA_PREV_TRACK : return INPUT::KEYCODE_PREVTRACK;
        case VK_ATTN : return INPUT::KEYCODE_AT;
        //case VK_OEM_COMMA : return INPUT::KEYCODE_COLON;
        //case VK_OEM_MINUS : return INPUT::KEYCODE_UNDERLINE;
        case VK_KANJI : return INPUT::KEYCODE_KANJI;
        case VK_MEDIA_STOP : return INPUT::KEYCODE_STOP;
        case VK_OEM_AX : return INPUT::KEYCODE_AX;
        //case VK_UNLABELED : return INPUT::KEYCODE_UNLABELED;
        case VK_MEDIA_NEXT_TRACK : return INPUT::KEYCODE_NEXTTRACK;
        //case VK_NUMPADENTER : return INPUT::KEYCODE_NUMPADENTER;
        case VK_VOLUME_MUTE : return INPUT::KEYCODE_MUTE;
        //case VK_CALCULATOR : return INPUT::KEYCODE_CALCULATOR;
        case VK_MEDIA_PLAY_PAUSE : return INPUT::KEYCODE_PLAYPAUSE;
        //case VK_MEDIA_STOP : return INPUT::KEYCODE_MEDIASTOP;
        case VK_VOLUME_DOWN : return INPUT::KEYCODE_VOLUMEDOWN;
        case VK_VOLUME_UP : return INPUT::KEYCODE_VOLUMEUP;
        case VK_BROWSER_HOME : return INPUT::KEYCODE_WEBHOME;
        //case VK_DECIMAL : return INPUT::KEYCODE_NUMPADCOMMA;
        case VK_DIVIDE : return INPUT::KEYCODE_DIVIDE;
        case VK_SNAPSHOT : return INPUT::KEYCODE_SYSRQ;
        case VK_RMENU : return INPUT::KEYCODE_RMENU;
        case VK_PAUSE : return INPUT::KEYCODE_PAUSE;
        case VK_HOME : return INPUT::KEYCODE_HOME;
        case VK_UP : return INPUT::KEYCODE_UP;
        case VK_PRIOR : return INPUT::KEYCODE_PGUP;
        case VK_LEFT : return INPUT::KEYCODE_LEFT;
        case VK_RIGHT : return INPUT::KEYCODE_RIGHT;
        case VK_END : return INPUT::KEYCODE_END;
        case VK_DOWN : return INPUT::KEYCODE_DOWN;
        case VK_NEXT : return INPUT::KEYCODE_PGDOWN;
        case VK_INSERT : return INPUT::KEYCODE_INSERT;
        case VK_DELETE : return INPUT::KEYCODE_DELETE;
        case VK_LWIN : return INPUT::KEYCODE_LWIN;
        case VK_RWIN : return INPUT::KEYCODE_RWIN;
        case VK_APPS : return INPUT::KEYCODE_APPS;
        //case VK_POWER : return INPUT::KEYCODE_POWER;
        case VK_SLEEP : return INPUT::KEYCODE_SLEEP;
        //case VK_WAKE : return INPUT::KEYCODE_WAKE;
        case VK_BROWSER_SEARCH : return INPUT::KEYCODE_WEBSEARCH;
        case VK_BROWSER_FAVORITES : return INPUT::KEYCODE_WEBFAVORITES;
        case VK_BROWSER_REFRESH : return INPUT::KEYCODE_WEBREFRESH;
        case VK_BROWSER_STOP : return INPUT::KEYCODE_WEBSTOP;
        case VK_BROWSER_FORWARD  : return INPUT::KEYCODE_WEBFORWARD;
        case VK_BROWSER_BACK : return INPUT::KEYCODE_WEBBACK;
        case VK_LAUNCH_APP1 : return INPUT::KEYCODE_MYCOMPUTER;
        case VK_LAUNCH_MAIL : return INPUT::KEYCODE_MAIL;
        case VK_LAUNCH_MEDIA_SELECT : return INPUT::KEYCODE_MEDIASELECT;
        default :
        {
            return INPUT::KEYCODE_UNASSIGNED;
        }
    }
}

/*---------------------------------------------------------------------------*/

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL  0x020E
#endif

/*---------------------------------------------------------------------------*/

/* Passes a typed character on as UTF-8 text; 'unit' is a UTF-16 code unit of a WM_CHAR of a unicode window */
static void
SendTypedCharacter( TContextData* data, WCHAR unit )
{
    if ( GUCEF_NULL == data->callbacks.onKeyboardText )
        return;

    UInt32 codePoint = unit;
    if ( unit >= 0xD800 && unit <= 0xDBFF )
    {
        data->highSurrogate = unit;
        return;
    }
    if ( unit >= 0xDC00 && unit <= 0xDFFF )
    {
        if ( 0 == data->highSurrogate )
            return;
        codePoint = 0x10000 + ( ( (UInt32) data->highSurrogate - 0xD800 ) << 10 ) + ( (UInt32) unit - 0xDC00 );
    }
    data->highSurrogate = 0;

    /* control characters (Return, Backspace, Escape, Tab, Delete) are keys, not text */
    if ( codePoint < 32 || 127 == codePoint )
        return;

    char utf8[ 5 ];
    if ( codePoint < 0x80 )
    {
        utf8[ 0 ] = (char) codePoint;
        utf8[ 1 ] = 0;
    }
    else
    if ( codePoint < 0x800 )
    {
        utf8[ 0 ] = (char) ( 0xC0 | ( codePoint >> 6 ) );
        utf8[ 1 ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
        utf8[ 2 ] = 0;
    }
    else
    if ( codePoint < 0x10000 )
    {
        utf8[ 0 ] = (char) ( 0xE0 | ( codePoint >> 12 ) );
        utf8[ 1 ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        utf8[ 2 ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
        utf8[ 3 ] = 0;
    }
    else
    {
        utf8[ 0 ] = (char) ( 0xF0 | ( codePoint >> 18 ) );
        utf8[ 1 ] = (char) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) );
        utf8[ 2 ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        utf8[ 3 ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
        utf8[ 4 ] = 0;
    }
    data->callbacks.onKeyboardText( data->callbacks.userData, KEYBOARD_DEVICEID, utf8 );
}

/*---------------------------------------------------------------------------*/

/* Turns wheel movement in WHEEL_DELTA units into whole notches and reports those as mouse variable 'variable' */
static void
SendWheel( TContextData* data, UInt32 variable, Int32 wheelDelta, Int32& remainder, Int32& position )
{
    remainder += wheelDelta;
    Int32 notches = remainder / WHEEL_DELTA;
    if ( 0 == notches )
        return;
    remainder -= notches * WHEEL_DELTA;
    position += notches;
    data->callbacks.onMouseVarChanged( data->callbacks.userData, MOUSE_DEVICEID, variable, position, notches );
}

/*---------------------------------------------------------------------------*/

/* Passes a side button (WM_XBUTTONDOWN / WM_XBUTTONUP) on as button 3 or 4 */
static void
SendSideButton( TContextData* data, WPARAM wparam, LPARAM lparam, bool pressed )
{
    Int32 xPos = GET_X_LPARAM( lparam );
    Int32 yPos = GET_Y_LPARAM( lparam );
    data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
    data->mouseXPos = xPos;
    data->mouseYPos = yPos;

    UInt32 button = XBUTTON1 == GET_XBUTTON_WPARAM( wparam ) ? 3 : 4;
    if ( pressed )
        data->callbacks.onMouseButtonDown( data->callbacks.userData, MOUSE_DEVICEID, button );
    else
        data->callbacks.onMouseButtonUp( data->callbacks.userData, MOUSE_DEVICEID, button );
}

/*---------------------------------------------------------------------------*/

LRESULT
InputDriverProcessMSWINMSG( HWND whnd     , 
                            UINT message  , 
                            WPARAM wparam , 
                            LPARAM lparam )
{
    TContextData* data = LookupDataForHWND( whnd );
    if ( NULL == data ) return 0;
        
    switch ( message )
    {
        case WM_CHAR :
        {
            /*
             *      the key down & action has been translated into a char: this is the text the user typed
             *      A unicode window gets UTF-16 code units, others a character of the ANSI code page
             */
            if ( ::GetFocus() == data->hWnd )
            {
                if ( ::IsWindowUnicode( data->hWnd ) )
                {
                    SendTypedCharacter( data, (WCHAR) wparam );
                }
                else
                {
                    char ansi = (char) wparam;
                    WCHAR wide[ 2 ] = { 0, 0 };
                    if ( 1 == ::MultiByteToWideChar( CP_ACP, 0, &ansi, 1, wide, 2 ) )
                        SendTypedCharacter( data, wide[ 0 ] );
                }
            }
            break;
        }
        case WM_POWER :
        {
            data->callbacks.onKeyboardKeyDown( data->callbacks.userData, KEYBOARD_DEVICEID, INPUT::KEYCODE_POWER, data->keyModState );
            break;
        }
        case WM_KEYUP :
        {
            if ( ::GetFocus() != data->hWnd )
            {
                break;
            }

            /* 
             *      In non-ASCII mode we should only use this and ignore WM_CHAR
             */
            switch ( wparam )
            {
                case VK_SHIFT :
                {
                    GUCEF_SETBITXOFF( data->keyModState, KEYMOD_SHIFT_BIT );    
                    break;                                        
                }
                case VK_MENU :
                {
                    /* this is the alt key */
                    GUCEF_SETBITXOFF( data->keyModState, KEYMOD_ALT_BIT );
                    break;                                        
                } 
                case VK_CONTROL :
                {
                    GUCEF_SETBITXOFF( data->keyModState, KEYMOD_CTRL_BIT );
                    break;                                        
                }
                case VK_XBUTTON1 :
                {
                    UInt32 xPos = GET_X_LPARAM( lparam );
                    UInt32 yPos = GET_Y_LPARAM( lparam );
                    data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
                    data->mouseXPos = xPos;
                    data->mouseYPos = yPos;  
                        
                    data->callbacks.onMouseButtonUp( data->callbacks.userData, MOUSE_DEVICEID, 3 );
                    break;
                }
                case VK_XBUTTON2 :
                {
                    UInt32 xPos = GET_X_LPARAM( lparam );
                    UInt32 yPos = GET_Y_LPARAM( lparam );
                    data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
                    data->mouseXPos = xPos;
                    data->mouseYPos = yPos;  
                        
                    data->callbacks.onMouseButtonUp( data->callbacks.userData, MOUSE_DEVICEID, 4 );
                    break;
                }                                                               
                default :
                {
                    break;
                }
            }                        

            data->keyStateBuffer[ wparam ] = 0;                          /* , data->keyModState <- modifiers */
            data->callbacks.onKeyboardKeyUp( data->callbacks.userData, KEYBOARD_DEVICEID, MapKeyScanCode( wparam, lparam ), data->keyModState );
            break;
        }
        case WM_KEYDOWN :
        {
            if ( ::GetFocus() != data->hWnd )
            {
                break;
            }

            switch ( wparam )
            {
                case VK_CAPITAL :
                {
                    /* the capital or CAPS key is a toggle key and should be handled as such */
                    GUCEF_GETBITX( data->keyModState, KEYMOD_CAPSLOCK_BIT ) ? GUCEF_SETBITXOFF( data->keyModState, KEYMOD_CAPSLOCK_BIT ) : GUCEF_SETBITXON( data->keyModState, KEYMOD_CAPSLOCK_BIT );
                    break;
                }
                case VK_SCROLL :
                {
                    /* The scroll lock key is a toggle key and should be treated as such*/
                    GUCEF_GETBITX( data->keyModState, KEYMOD_SCROLLLOCK_BIT ) ? GUCEF_SETBITXOFF( data->keyModState, KEYMOD_SCROLLLOCK_BIT ) : GUCEF_SETBITXON( data->keyModState, KEYMOD_SCROLLLOCK_BIT );
                    break; 
                }
                case VK_NUMLOCK :
                {
                    /* The numlock lock key is a toggle key and should be treated as such*/
                    GUCEF_GETBITX( data->keyModState, KEYMOD_NUMLOCK_BIT ) ? GUCEF_SETBITXOFF( data->keyModState, KEYMOD_NUMLOCK_BIT ) : GUCEF_SETBITXON( data->keyModState, KEYMOD_NUMLOCK_BIT );
                    break; 
                }
                case VK_SHIFT :
                {
                    GUCEF_SETBITXON( data->keyModState, KEYMOD_SHIFT_BIT );
                    break;                                        
                }
                case VK_MENU :
                {
                    GUCEF_SETBITXON( data->keyModState, KEYMOD_ALT_BIT );
                    break;                                        
                } 
                case VK_CONTROL :
                {
                    GUCEF_SETBITXON( data->keyModState, KEYMOD_CTRL_BIT );
                    break;                                        
                }                                                               
                case VK_XBUTTON1 :
                {
                    UInt32 xPos = GET_X_LPARAM( lparam );
                    UInt32 yPos = GET_Y_LPARAM( lparam );
                    data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
                    data->mouseXPos = xPos;
                    data->mouseYPos = yPos;  
                        
                    data->callbacks.onMouseButtonDown( data->callbacks.userData, MOUSE_DEVICEID, 3 );
                    break;
                }
                case VK_XBUTTON2 :
                {
                    UInt32 xPos = GET_X_LPARAM( lparam );
                    UInt32 yPos = GET_Y_LPARAM( lparam );
                    data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
                    data->mouseXPos = xPos;
                    data->mouseYPos = yPos;  
                        
                    data->callbacks.onMouseButtonDown( data->callbacks.userData, MOUSE_DEVICEID, 4 );
                    break;
                }
                default :
                {
                    break;
                }
            }                        
            data->keyStateBuffer[ wparam ] = 1;                           /* , data->keyModState <- modifiers */
            data->callbacks.onKeyboardKeyDown( data->callbacks.userData, KEYBOARD_DEVICEID, MapKeyScanCode( wparam, lparam ), data->keyModState ); 
            break;               
        }
        case WM_MOUSEMOVE :
        {
            if ( ::GetFocus() == data->hWnd )
            {
                UInt32 xPos = GET_X_LPARAM( lparam );
                UInt32 yPos = GET_Y_LPARAM( lparam );
                data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
                data->mouseXPos = xPos;
                data->mouseYPos = yPos;  
            }
            break;
        }
        case WM_MOUSEWHEEL :
        case WM_MOUSEHWHEEL :
        {
            if ( ::GetFocus() == data->hWnd )
            {
                /* the wheel messages have the pointer in screen coordinates */
                POINT pos;
                pos.x = GET_X_LPARAM( lparam );
                pos.y = GET_Y_LPARAM( lparam );
                ::ScreenToClient( data->hWnd, &pos );
                data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, pos.x, pos.y, pos.x - data->mouseXPos, pos.y - data->mouseYPos );
                data->mouseXPos = pos.x;
                data->mouseYPos = pos.y;

                /* away from the user and tilting right are positive */
                Int32 wheelDelta = (short) HIWORD( wparam );
                if ( WM_MOUSEWHEEL == message )
                    SendWheel( data, 0, wheelDelta, data->mouseWheelRemainder, data->mouseWheelPos );
                else
                    SendWheel( data, 1, wheelDelta, data->mouseHWheelRemainder, data->mouseHWheelPos );
            }
            break;
        }
        case WM_XBUTTONDOWN :
        {
            SendSideButton( data, wparam, lparam, true );
            break;
        }
        case WM_XBUTTONUP :
        {
            SendSideButton( data, wparam, lparam, false );
            break;
        }
        case WM_LBUTTONDOWN :
        {
            UInt32 xPos = GET_X_LPARAM( lparam );
            UInt32 yPos = GET_Y_LPARAM( lparam );
            data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
            data->mouseXPos = xPos;
            data->mouseYPos = yPos;  
                        
            data->callbacks.onMouseButtonDown( data->callbacks.userData, MOUSE_DEVICEID, 0 );
            break;
        }
        case WM_LBUTTONUP :
        {
            WORD xPos = GET_X_LPARAM( lparam );
            WORD yPos = GET_Y_LPARAM( lparam );
            data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
            data->mouseXPos = xPos;
            data->mouseYPos = yPos;  
                        
            data->callbacks.onMouseButtonUp( data->callbacks.userData, MOUSE_DEVICEID, 0 );
            break;
        }                
        case WM_MBUTTONDOWN :
        {
            WORD xPos = GET_X_LPARAM( lparam );
            WORD yPos = GET_Y_LPARAM( lparam );
            data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
            data->mouseXPos = xPos;
            data->mouseYPos = yPos;  
                        
            data->callbacks.onMouseButtonDown( data->callbacks.userData, MOUSE_DEVICEID, 2 );
            break;
        }
        case WM_MBUTTONUP :
        {
            WORD xPos = GET_X_LPARAM( lparam );
            WORD yPos = GET_Y_LPARAM( lparam );
            data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
            data->mouseXPos = xPos;
            data->mouseYPos = yPos; 
                        
            data->callbacks.onMouseButtonUp( data->callbacks.userData, MOUSE_DEVICEID, 2 );
            break;
        }                                 
        case WM_RBUTTONDOWN :
        {
            WORD xPos = GET_X_LPARAM( lparam );
            WORD yPos = GET_Y_LPARAM( lparam );
            data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
            data->mouseXPos = xPos;
            data->mouseYPos = yPos;  
                        
            data->callbacks.onMouseButtonDown( data->callbacks.userData, MOUSE_DEVICEID, 1 );
            break;
        }
        case WM_RBUTTONUP :
        {
            WORD xPos = GET_X_LPARAM( lparam );
            WORD yPos = GET_Y_LPARAM( lparam );
            data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, xPos, yPos, xPos - data->mouseXPos, yPos - data->mouseYPos );
            data->mouseXPos = xPos;
            data->mouseYPos = yPos; 
                        
            data->callbacks.onMouseButtonUp( data->callbacks.userData, MOUSE_DEVICEID, 1 );
            break;   
        }                
        case WM_ACTIVATE :
        {
            if ( ( WA_ACTIVE == LOWORD( wparam ) )      ||
                 ( WA_CLICKACTIVE == LOWORD( wparam ) ) )
            {
                GrabWindowCursorInput( data, data->doGrab );
                ::ShowCursor( data->showCursor );
            }
            else
            {
                // WA_INACTIVE
                GrabWindowCursorInput( data, 0 );
                ::ShowCursor( 1 );
            }
            break;
        }
        default : 
        {
            break;
        }                
    }        
        
    return CallWindowProc( data->PrevWindowProcedurePtr ,
                           whnd                         ,
                           message                      ,
                           wparam                       ,
                           lparam                       );
}                   

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Init( void** plugdata    ,
                      const char*** args ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( NULL != driverData )
    {
        /* we should not get here, init called more then once ? */
        return 1;
    }
    *plugdata = driverData = ( TDriverData* ) malloc( sizeof( TDriverData ) );
        
    if ( NULL == driverData )
    {
        /* allocation failed */
        return 0;
    }
        
    memset( driverData, 0, sizeof( TDriverData ) );
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    free( driverData );
    driverData = NULL;
    return 1;                        
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Name( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return DRIVER_NAME;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return DRIVER_COPYRIGHT;
}

/*---------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return &version;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Update( void* plugdata    , 
                        void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        //if ( plugdata && contextdata )
        //{
        //        TContextData* data = (TContextData*) contextdata;
        //        
        //        //OpenInputDesktop(
        //        
        //        POINT cursorPos;
        //        if ( GetCursorPos( &cursorPos ) )
        //        {                        
        //                if ( ScreenToClient( data->hWnd, &cursorPos ) )
        //                {
        //                        if ( ( cursorPos.x != data->mouseXPos ) ||
        //                             ( cursorPos.y != data->mouseYPos )  )
        //                        {                             
        //                                data->callbacks.onMouseMove( data->callbacks.userData, MOUSE_DEVICEID, cursorPos.x, cursorPos.y, cursorPos.x - data->mouseXPos, cursorPos.y - data->mouseYPos );
        //                                data->mouseXPos = cursorPos.x;
        //                                data->mouseYPos = cursorPos.y;                        
        //                        }
        //                }                                                                
        //        }
        //        
        //}
        return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_CreateContext( void* plugdata                   ,
                               void** contextdata               ,
                               int argc                         ,
                               const char** argv                ,
                               const TInputCallbacks* callbacks ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TContextData* data;
    *contextdata = data = GetFreeContext();
        
    /* make sure we didnt hit the context maximum */
    if ( GUCEF_NULL == data ) return 0;

    data->callbacks = *callbacks; /* copy our callbacks */
    data->inUse = 1;              /* we are using this context */
    data->doGrab = 0;             /* default win32 is no input cursor grabbing */
    data->showCursor = 1;         /* default win32 is to always show the cursor */
    data->mouseWheelPos = 0;
    data->mouseWheelRemainder = 0;
    data->mouseHWheelPos = 0;
    data->mouseHWheelRemainder = 0;
    data->highSurrogate = 0;
        
    /* 
        *      get the window handle, this is always passed for MSWIN input drivers 
        */
    #pragma warning( disable: 4312 )
    data->hWnd = (HWND) ParseArgListItemPointer( argc, argv, "-WINDOW" );
    BYTE grabCursor = ParseArgListItemUInt32( argc, argv, "-GRABCURSOR" ) != 0;
    data->showCursor = ParseArgListItemUInt32( argc, argv, "-HIDECURSOR" ) == 0;

    if ( GUCEF_NULL == data->hWnd )
    {
        /* We need a valid window handle in order to hook up a input context */
        ReleaseContext( data );
        *contextdata = GUCEF_NULL;
        return 0;
    }

    /*
     *      Retrieve a pointer to the current window message handler
     *      we will have to restore this link if the context is destroyed !!!
     */
    data->PrevWindowProcedurePtr = (WNDPROC) GetWindowLongPtr( data->hWnd                  ,
                                                               MSWIN_BITTARGET_GWL_WNDPROC );

    /*
     *      use per instance window sub-classing 
     *      we redirect windows messages to our function which will process 
     *      and then forward the messages
     */
    #pragma warning( disable: 4244 ) // 'argument' : conversion from 'LONG_PTR' to 'LONG', possible loss of data
    SetWindowLongPtr( data->hWnd                            ,
                      MSWIN_BITTARGET_GWL_WNDPROC           ,
                      (LONG_PTR) InputDriverProcessMSWINMSG );

       /*
        *  Set mouse confinement and mouse visibility
        */
    GrabWindowCursorInput( data, grabCursor );
    ::ShowCursor( data->showCursor ); 
        
    callbacks->onKeyboardAttached( callbacks->userData, KEYBOARD_DEVICEID );
    callbacks->onMouseAttached( callbacks->userData, MOUSE_DEVICEID );
                               
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_DestroyContext( void* plugdata    , 
                                void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TContextData* data = (TContextData*) contextdata;

    /*
     *  Restore the link to the old window handler
     */        
    #pragma warning( disable: 4244 ) // 'argument' : conversion from 'LONG_PTR' to 'LONG', possible loss of data
    SetWindowLongPtr( data->hWnd                              ,
                      MSWIN_BITTARGET_GWL_WNDPROC             ,
                      (LONG_PTR) data->PrevWindowProcedurePtr );
                                        
    /* make sure we release our grab on the cursor */
    GrabWindowCursorInput( data, 0 );

    data->callbacks.onKeyboardDetached( data->callbacks.userData, KEYBOARD_DEVICEID );
    data->callbacks.onMouseDetached( data->callbacks.userData, MOUSE_DEVICEID );

    ReleaseContext( data );
        
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetMousePos( void* plugdata    , 
                             void* contextdata , 
                             UInt32* xpos      , 
                             UInt32* ypos      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TContextData* data = (TContextData*) contextdata;
        return 1;
}

/*---------------------------------------------------------------------------*/

UInt8* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetKeyBoardKeyStates( void* plugdata    , 
                                      void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TContextData* data = (TContextData*) contextdata;
        return data->keyStateBuffer;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetMouseButtonPressedState( void* plugdata           , 
                                            void* contextdata        , 
                                            const UInt32 buttonindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TContextData* data = (TContextData*) contextdata;                         
        return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetKeyboardKeyPressedState( void* plugdata        , 
                                            void* contextdata     , 
                                            const UInt32 keyindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TContextData* data = (TContextData*) contextdata;
        if ( keyindex < 256 )
        {
                return data->keyStateBuffer[ keyindex ];                
        }
        return 0;                
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetDeviceBoolState( void* plugdata          , 
                                    void* contextdata       , 
                                    const UInt32 deviceid   , 
                                    const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        /* not supported */
        return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetDeviceVarState( void* plugdata          , 
                                   void* contextdata       , 
                                   const UInt32 deviceid   , 
                                   const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        /* not supported */
        return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace INPUT */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*---------------------------------------------------------------------------*/
