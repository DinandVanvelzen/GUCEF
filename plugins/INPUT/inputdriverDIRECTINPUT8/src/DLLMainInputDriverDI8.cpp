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

/*
 *      API for a Input driver plugin module
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define DIRECTINPUT_VERSION 0x0800
//#define UNICODE       // enable to compile as UNICODE, default is ANSI key codes

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#include <stdlib.h>

/* You need to have the DirectX SDK installed for the following dx headers */
#include <dinput.h>

#include "DLLMainInputDriverDI8.h"

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
#define DRIVER_NAME       "DirectInput8\0"
#define DRIVER_COPYRIGHT  "Copyright (C) Dinand Vanvelzen. 2002 - 2005.  All rights reserved.\0"

#define MOUSE_BUFFERSIZE    128  // number or DINPUT_BUFFERSIZE
#define KEYBOARD_BUFFERSIZE 64   // number or DINPUT_BUFFERSIZE

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define BUTTONBOOL( data ) ( ( ( (data) & 0x80 ) ? 1 : 0 ) )
#define KEYBOOL( data )    ( ( ( (data) & 0x80 ) ? 1 : 0 ) )
#define BUTTONINDEX( id )  ( ( (id)-DIMOFS_BUTTON0 ) )

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SDI8Data
{
        /* system data */
        HWND hWnd;
        HINSTANCE hInstance;
        
        /* DI Object */
        LPDIRECTINPUT8 pDI;
        
        /* DI Device Capabilities */
        DIDEVCAPS DIDevCaps;                
        
        /* 8 Element Array, TRUE if Button [n] is up */
       // BOOL* bMouseUp;

        /* Mouse vars */
        POINT* pMousePos;
        Int32 mouseWheelVar;        

        /* Cursor HotSpot */
        POINT* pHotSpot;
        
        /* Mouse Handle */
        HANDLE hMouse;
        HRESULT hr;        

        /* DI Device Objects */
        LPDIRECTINPUTDEVICE8 pDIJoy;
        LPDIRECTINPUTDEVICE8 pDIKeybrd;
        LPDIRECTINPUTDEVICE8 pDIMouse;
        
        /* Device event buffers */        
        DIDEVICEOBJECTDATA mouseBuffer[ MOUSE_BUFFERSIZE ];
        DIDEVICEOBJECTDATA keyboardBuffer[ KEYBOARD_BUFFERSIZE ];

        /* Device State Buffers */
        DIJOYSTATE2 js;
        UInt8 keybuffer[ 256 ];
        UInt8 keystatebuffer[ 256 ];
        UInt32 keyModState;
        DWORD dwElements;
        
        /* context event callbacks */
        TInputCallbacks callbacks;
};
typedef struct SDI8Data TDI8Data;
         
/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

HRESULT InitDI8( TDI8Data* data );
void ShutdownDI8( TDI8Data* data );
BOOL __stdcall EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
BOOL __stdcall EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );

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

BOOL __stdcall
EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi , 
                  VOID* pContext                       )
{
        TDI8Data* data = (TDI8Data*) pContext;        

        DIPROPRANGE diprg; 
        diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        diprg.diph.dwHow        = DIPH_BYID; 
        diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin              = -1000; 
        diprg.lMax              = +1000; 

        // Set the range for the axis
        if( FAILED( data->pDIJoy->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
        {
                return DIENUM_STOP;
        }                

        return DIENUM_CONTINUE;
}

/*---------------------------------------------------------------------------*/

BOOL __stdcall 
EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance , 
                       VOID* pContext                       )
{
        TDI8Data* data = (TDI8Data*) pContext;
        HRESULT hr;

        hr = data->pDI->CreateDevice( pdidInstance->guidInstance, &data->pDIJoy, NULL );

        if ( FAILED( hr ) ) 
        {
                return DIENUM_CONTINUE;
        }                

        return DIENUM_STOP;
}                       

/*---------------------------------------------------------------------------*/

HRESULT 
InitDI8( TDI8Data* data )
{	
        DIPROPDWORD  dipdw; 
        dipdw.diph.dwSize = sizeof(DIPROPDWORD); 
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        dipdw.diph.dwObj = 0; 
        dipdw.diph.dwHow = DIPH_DEVICE; 
        dipdw.dwData = MOUSE_BUFFERSIZE;

        /* Create The DI Object */
        if( DI_OK != DirectInput8Create( data->hInstance              ,  
                                         DIRECTINPUT_VERSION          , 
                                         ((REFIID)IID_IDirectInput8)  , 
                                         ((void**)&data->pDI)         , 
                                         NULL                         ) )
        {                                         
                return E_FAIL;
        }                
	
	/* Set up keyboard input */
	if( FAILED( data->pDI->CreateDevice( GUID_SysKeyboard , 
	                                     &data->pDIKeybrd , 
	                                     NULL             ) ) )
        {	                                     
		return E_FAIL;
        }
        
	if( FAILED( data->pDIKeybrd->SetDataFormat( &c_dfDIKeyboard ) ) )
	{
	        return E_FAIL;
        }
                 
        dipdw.dwData = KEYBOARD_BUFFERSIZE;
        if( FAILED( data->pDIKeybrd->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
        {
                return E_FAIL;
        }        	        

        if( FAILED( data->pDIKeybrd->SetCooperativeLevel( data->hWnd                            , 
                                                          DISCL_FOREGROUND | DISCL_NONEXCLUSIVE ) ) )
        {                                                          
                return E_FAIL;
        }                

        if ( data->pDIKeybrd ) 
        {
                data->pDIKeybrd->Acquire(); 
        }                

        /* Set up mouse input */

        if( FAILED( data->pDI->CreateDevice( GUID_SysMouse   , 
                                             &data->pDIMouse , 
                                             NULL            ) ) )
        {                                             
                return E_FAIL;
        }                

        if( FAILED( data->pDIMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
        {
                return E_FAIL;
        }
         
        dipdw.dwData = MOUSE_BUFFERSIZE; 
        if( FAILED( data->pDIMouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
        {
                return E_FAIL;
        }

        if( FAILED( data->pDIMouse->SetCooperativeLevel( data->hWnd                         , 
                                                         DISCL_FOREGROUND | DISCL_EXCLUSIVE ) ) )
        {                                                         
                return E_FAIL;
        }                

        data->hMouse = CreateEvent( 0, 0, 0, 0 );

        if ( data->hMouse == NULL )
        {
                ShutdownDI8( data );
                return 0;
        }

        data->hr = data->pDIMouse->SetEventNotification( data->hMouse );

        if ( FAILED( data->hr ) )
        {
                ShutdownDI8( data );
                return 0;
        }

        if ( FAILED( data->hr ) )
        {
                ShutdownDI8( data );
                return 0;
        }

        if ( data->pDIMouse ) 
        {
                data->pDIMouse->Acquire();
        }                

	/*Set up Joystick input (Only the first joy) */
   /*
        if( FAILED( data->pDI->EnumDevices( DI8DEVCLASS_GAMECTRL  , 
                                            EnumJoysticksCallback , 
                                            (VOID*) data          , 
                                            DIEDFL_ATTACHEDONLY   ) ) )
        {
                return E_FAIL;
        }                

        if( FAILED( data->pDIJoy->SetDataFormat( &c_dfDIJoystick2 ) ) )
        {
                return E_FAIL;
        }                

        if( FAILED( data->pDIJoy->SetCooperativeLevel( data->hWnd                         ,  
                                                       DISCL_EXCLUSIVE | DISCL_FOREGROUND ) ) )
        {                                                       
                return E_FAIL;
        }                

        data->DIDevCaps.dwSize = sizeof( DIDEVCAPS );
        if ( FAILED( data->pDIJoy->GetCapabilities( &data->DIDevCaps ) ) )
        {
                return E_FAIL;
        }                

        if ( FAILED( data->pDIJoy->EnumObjects( EnumAxesCallback , 
                                                (VOID*) data     , 
                                                DIDFT_AXIS       ) ) )
        {                                                
                return E_FAIL;
        }                

        if( data->pDIJoy ) 
        {
                data->pDIJoy->Acquire();
        }
                    */
	return S_OK;
}

/*---------------------------------------------------------------------------*/

void
ShutdownDI8( TDI8Data* data )
{
        if ( data->pDI )
        {
                if ( data->pDIKeybrd )
                {
                        data->pDIKeybrd->Unacquire();
                        data->pDIKeybrd->Release();
                        data->pDIKeybrd = NULL;
                }

                if ( data->pDIMouse )
                {
                        data->pDIMouse->Unacquire();
                        data->pDIMouse->Release();
                        data->pDIMouse = NULL;
                }
        
                if ( data->pDIJoy )
                {
                        data->pDIJoy->Unacquire();
                        data->pDIJoy->Release();
                        data->pDIJoy = NULL;
                }
                data->pDI->Release();
                data->pDI = NULL;
        }	
}

/*---------------------------------------------------------------------------*/

HRESULT
ProcessMouseDI8( TDI8Data* data )
{
        DIDEVICEOBJECTDATA* eventptr;
        UInt32 i=0;
        data->hr = data->pDIMouse->Acquire();
        /* Check for mouse input and fill buffer */
        DWORD dwItems( MOUSE_BUFFERSIZE );
        data->hr = data->pDIMouse->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ) ,
                                                  &data->mouseBuffer[ 0 ]      ,  /* our event buffer */
                                                  &dwItems                     ,  /* in the buffer size, out the read count */
                                                  0                            ); /* remove items when read */ 

        if ( data->hr == DIERR_INPUTLOST )
        {        
                /* aquire the mouse, we may have lost our connection since the last call */
                data->hr = data->pDIMouse->Acquire();
                
                /* Check for mouse input and fill buffer */
                DWORD dwItems( MOUSE_BUFFERSIZE );
                data->hr = data->pDIMouse->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ) ,
                                                          &data->mouseBuffer[ 0 ]      ,  /* our event buffer */
                                                          &dwItems                     ,  /* in the buffer size, out the read count */
                                                          0                            ); /* remove items when read */ 
        }                
        
        if ( SUCCEEDED( data->hr ) )
        {
               
                
                for ( i=0; i<dwItems; ++i )
                {
                        eventptr = &data->mouseBuffer[ i ];
                        
                        switch ( eventptr->dwOfs ) 
                        {
                                case DIMOFS_X :
                                {
                                        data->pMousePos->x += eventptr->dwData; 
                                        data->callbacks.onMouseMove( data->callbacks.userData, i, data->pMousePos->x, data->pMousePos->y, eventptr->dwData, 0 );
                                        break;
                                }
                                case DIMOFS_Y :
                                {
                                        data->pMousePos->y += eventptr->dwData; 
                                        data->callbacks.onMouseMove( data->callbacks.userData, i, data->pMousePos->x, data->pMousePos->y, 0, eventptr->dwData );
                                        break;
                                }                                
                                case DIMOFS_Z : /* mousewheel */
                                {
                                        Int32 delta( 0 );
                                        if ( BUTTONBOOL( eventptr->dwData ) )
                                        {
                                                ++data->mouseWheelVar;
                                                delta = 1;
                                        }
                                        else
                                        {
                                                --data->mouseWheelVar;
                                                delta = -1;
                                        }
                                        data->callbacks.onMouseVarChanged( data->callbacks.userData, i, 0, data->mouseWheelVar, delta );
                                        break;
                                }
                                case DIMOFS_BUTTON0 :
                                case DIMOFS_BUTTON1 :
                                case DIMOFS_BUTTON2 :
                                case DIMOFS_BUTTON3 :
                                case DIMOFS_BUTTON4 :
                                case DIMOFS_BUTTON5 :
                                case DIMOFS_BUTTON6 :
                                case DIMOFS_BUTTON7 :
                                {
                                        if ( BUTTONBOOL( eventptr->dwData ) )
                                        {
                                                data->callbacks.onMouseButtonDown( data->callbacks.userData, i, BUTTONINDEX( (int) eventptr->dwOfs ) );
                                        }
                                        else
                                        {
                                                data->callbacks.onMouseButtonUp( data->callbacks.userData, i, BUTTONINDEX( (int) eventptr->dwOfs ) );
                                        }
                                }
                        }
                }
                return S_OK;
        }
        else
        {
                if ( data->hr == DIERR_INPUTLOST )
                {
                        data->hr = data->pDIMouse->Acquire();
                }
                return E_FAIL;                       
        }                                
}

/*---------------------------------------------------------------------------*/

/**
 *      Utility function for converting from virtual DI key values into
 *      keyboard scancodes.
 *      
 *      @return 0 on failure 1 on success
 */
UInt32
MapDIKeyToScancode( UINT keyCode   ,
                    UINT *scanCode )
{
        HKL hklKeyboardLayout = GetKeyboardLayout(0); // 0 means current thread
        // This seemingly cannot fail
        // If this value is cached then the application must respond to WM_INPUTLANGCHANGE

        /*
         *      Notes on second arg beeing 0:
         *            uCode is a virtual-key code and is translated into a scan code. 
         *            If it is a virtual-key code that does not distinguish between left- and right-hand keys, 
         *            the left-hand scan code is returned. If there is no translation, the function returns 0.
         */
        UINT uiScanCode = MapVirtualKeyEx( keyCode           ,
                                           0                 , // Convert DIK_ code to scan code
                                           hklKeyboardLayout );
        if( 0 == uiScanCode )
        {
                // Error converting to a scancode
                return 0;
        }
        *scanCode = uiScanCode;
        return 1;
}

/*---------------------------------------------------------------------------*/

UInt32
MapScanCodeToChar( UINT scanCode  ,
                   BYTE *charCode )
{

        HKL hklKeyboardLayout = GetKeyboardLayout(0); // 0 means current thread
        // This seemingly cannot fail
        // If this value is cached then the application must respond to WM_INPUTLANGCHANGE

        /* 
         *      Notes on second arg beeing 2:
         *            uCode is a virtual-key code and is translated into an unshifted character value in 
         *            the low-order word of the return value. Dead keys (diacritics) are indicated by 
         *            setting the top bit of the return value. If there is no translation, the function returns 0.
         */
        UINT uiCharCode = MapVirtualKeyEx( scanCode          ,
                                           2                 , // Convert scan code to char code
                                           hklKeyboardLayout );
        if( 0 == uiCharCode )
        {
                // Error converting to a char
                return 0;
        }
        *charCode = ((BYTE)uiCharCode);
        return 1;                                           
}

/*---------------------------------------------------------------------------*/

HRESULT
ProcessKeyboardDI8( TDI8Data* data )
{        
        DIDEVICEOBJECTDATA* eventptr;
        UInt32 i=0;

        /* Check for keyboard input and fill buffer */
        DWORD dwItems( KEYBOARD_BUFFERSIZE );
        data->hr = data->pDIKeybrd->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ) ,
                                                   &data->keyboardBuffer[ 0 ]   ,  /* our event buffer */
                                                   &dwItems                     ,  /* in the buffer size, out the read count */
                                                   0                            ); /* remove items when read */
        
        data->hr = data->pDIKeybrd->Acquire();
        
        if ( SUCCEEDED( data->hr ) )
        {
                for ( i=0; i<dwItems; ++i )
                {
                        UINT scanCode(0);
                        eventptr = &data->keyboardBuffer[ i ];
                        MapDIKeyToScancode( eventptr->dwOfs, &scanCode );

                        if ( KEYBOOL( eventptr->dwData ) )
                        { 
                              //  data->callbacks.onKeyboardKeyDown( data->callbacks.userData, scanCode );
                              
                           /*     UINT scanCode(0);
                                BYTE charCode(0);
                                if ( 0 != MapDIKeyToScancode( eventptr->dwOfs, &scanCode ) )
                                {
                                        if ( 0 != MapScanCodeToChar( scanCode, &charCode ) )
                                        {
                                                data->callbacks.onKeyboardKeyDown( data->callbacks.userData, charCode );
                                        }
                                } */
                                                            
                               
                              data->callbacks.onKeyboardKeyDown( data->callbacks.userData, 0, (KeyCode)scanCode, data->keyModState );
                        }
                        else
                        {
                               // UINT scanCode(0);
                              //  MapDIKeyToScancode( eventptr->dwOfs, &scanCode );                                 
                              //  data->callbacks.onKeyboardKeyUp( data->callbacks.userData, scanCode );
                              
                              data->callbacks.onKeyboardKeyUp( data->callbacks.userData, 0, (KeyCode)scanCode, data->keyModState );
                        }
                }
                return S_OK;
        }
        else
        {
                if ( data->hr == DIERR_INPUTLOST )
                {
                        // we lost contact with the device, attempt to recover
                        data->hr = data->pDIKeybrd->Acquire();
                        while( data->hr == DIERR_INPUTLOST )
                        {
                                data->hr = data->pDIKeybrd->Acquire();
                        }
                }
                else
                if ( data->hr == DI_BUFFEROVERFLOW )
                {
                        /* Check for keyboard input and fill buffer */
                        UInt8 keystatebuffer[ 256 ];
                        data->hr = data->pDIKeybrd->GetDeviceState( 256                    ,
                                                                   (LPVOID) &keystatebuffer ); 
                                                
                        if ( SUCCEEDED( data->hr ) ) 
                        { 
                                /* compare the current state with our buffered state and attempt to compensate event wise */
                                UInt32 i;
                                for ( i=0; i<256; ++i )
                                {
                                        if ( keystatebuffer[ i ] != data->keystatebuffer[ i ] )
                                        {
                                                if ( KEYBOOL( keystatebuffer[ i ] ) )
                                                {
                                                        data->keybuffer[ i ] = 1;
                                                        data->callbacks.onKeyboardKeyDown( data->callbacks.userData, 0, (KeyCode)i, data->keyModState );        
                                                }
                                                else
                                                {
                                                        data->keybuffer[ i ] = 0;
                                                        data->callbacks.onKeyboardKeyUp( data->callbacks.userData, 0, (KeyCode)i, data->keyModState );
                                                }                                                        
                                        }
                                }          
                        }                        
                }
                return E_FAIL;                        
        }      
}

/*---------------------------------------------------------------------------*/

HRESULT 
ProcessJoystickDI8( TDI8Data* data )
{
        /* Get joystick state and fill buffer */
    /*    if( FAILED( data->pDIJoy->Poll() ) )  
        {
                while( data->pDIJoy->Acquire() == DIERR_INPUTLOST ) 
                {}

                return E_FAIL; 
        }

        if ( FAILED( data->pDIJoy->GetDeviceState( sizeof(DIJOYSTATE2) ,  
                                                   &data->js           ) ) )
        {
                return E_FAIL;
        }       */         

        return S_OK;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Init( void** plugdata    ,
                      const char*** args ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        *plugdata = NULL;
        return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
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
        TDI8Data* data = (TDI8Data*) contextdata;
        
        if ( FAILED( ProcessMouseDI8( data ) )    ||
             FAILED( ProcessKeyboardDI8( data ) ) ||
             FAILED( ProcessJoystickDI8( data ) ) )
        {
                return 0;
        }             
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
        TDI8Data* data = (TDI8Data*) malloc( sizeof( TDI8Data ) );
        *contextdata = data;
        
        #pragma warning( disable: 4312 )
        data->hWnd = (HWND) ParseArgListItemUInt32( argc, argv, "HWND" );
        data->hInstance = (HINSTANCE) ParseArgListItemUInt32( argc, argv, "HINSTANCE" );
        
        data->pDI = NULL;
        data->pDIKeybrd	= NULL;
        data->pDIMouse = NULL;
        data->pDIJoy = NULL;
        data->pMousePos	= (POINT*) malloc( sizeof( POINT ) );
        data->pMousePos->x = 0l;
        data->pMousePos->y = 0l;
        data->mouseWheelVar = 0;
        data->pHotSpot = (POINT*) malloc( sizeof( POINT ) );
        data->pHotSpot->x = 0l;
        data->pHotSpot->y = 0l;
        data->dwElements = 16;
        data->callbacks = *callbacks;
        
        if ( FAILED( InitDI8( data ) ) )
        {
                free( data );
                *contextdata = NULL;
                return 0;        
        }
        
        // initialize the keyboard key buffers
        memset( data->keystatebuffer, 0, 256 );
        memset( data->keybuffer, 0, 256 );
        INPUTDRIVERPLUG_GetKeyBoardKeyStates( plugdata, data );
                
        return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_DestroyContext( void* plugdata    , 
                                void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TDI8Data* data = (TDI8Data*) contextdata;

        ShutdownDI8( data );
        
        free( data->pMousePos );
        free( data->pHotSpot );
        
        free( data );
        
        return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetMousePos( void* plugdata    , 
                             void* contextdata , 
                             UInt32* xpos      , 
                             UInt32* ypos      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TDI8Data* data = (TDI8Data*) contextdata;
        
        *xpos = data->pMousePos->x;
        *ypos = data->pMousePos->y;
        
     /* normalized window coords: 
     
        RECT rect;
        GetWindowRect( data->hWnd , &rect );
        *xpos = 1.0f - ( rect.right / data->pMousePos->x );
        *ypos = 1.0f - ( rect.bottom / data->pMousePos->y ); */
        
        return 1;
}

/*---------------------------------------------------------------------------*/

UInt8* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetKeyBoardKeyStates( void* plugdata    , 
                                      void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TDI8Data* data = (TDI8Data*) contextdata;
        
        /* Check for keyboard input and fill buffer */
        data->hr = data->pDIKeybrd->GetDeviceState( 256                            ,
                                                    (LPVOID) &data->keystatebuffer ); 
        if ( FAILED( data->hr ) ) 
        { 
                data->pDIKeybrd->Acquire();
                return NULL;                
        }
        
        for ( UInt32 i=0; i<256; ++i )
        {
                // convert binary mask into a simple 0 or 1 flag,
                // this assures independance of the DX implementation.
                data->keybuffer[ i ] = KEYBOOL( data->keystatebuffer[ i ] );
        }
        return data->keybuffer;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetMouseButtonPressedState( void* plugdata           , 
                                            void* contextdata        , 
                                            const UInt32 buttonindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        if ( buttonindex < 8 )
        {
                TDI8Data* data = (TDI8Data*) contextdata;        
                DIMOUSESTATE2 dims2;
                
                ZeroMemory( &dims2, sizeof(DIMOUSESTATE2) );
                data->hr = data->pDIMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims2 );
                
                if ( FAILED( data->hr ) ) 
                { 
                        data->pDIMouse->Acquire();
                        return 0;                
                }
                return dims2.rgbButtons[ buttonindex ];
        }                             
        return FALSE;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetKeyboardKeyPressedState( void* plugdata        , 
                                            void* contextdata     , 
                                            const UInt32 keyindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        if ( keyindex < 256 )
        {
                return ( (TDI8Data*) contextdata )->keybuffer[ keyindex ] != 0;
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
        return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetDeviceVarState( void* plugdata          , 
                                   void* contextdata       , 
                                   const UInt32 deviceid   , 
                                   const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        return 1;
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

/*-------------------------------------------------------------------------*/


/*
-----------------------------

IDirectInputDevice8::GetDeviceData Method

Retrieves buffered data from the device.

Syntax

HRESULT GetDeviceData(      
    DWORD cbObjectData,
    LPDIDEVICEOBJECTDATA rgdod,
    LPDWORD pdwInOut,
    DWORD dwFlags
);

Parameters

    cbObjectData
        Size of the DIDEVICEOBJECTDATA structure, in bytes. 
    rgdod
        Array of DIDEVICEOBJECTDATA structures to receive the buffered data. The number of elements in this array must be equal to the value of the pdwInOut parameter. If this parameter is NULL, the buffered data is not stored anywhere, but all other side effects take place. 
    pdwInOut
        On entry, the number of elements in the array pointed to by the rgdod parameter. On exit, the number of elements actually obtained. 
    dwFlags
        Flags that control the manner in which data is obtained. This value can be 0 or the following flag.

        DIGDD_PEEK
            Do not remove the items from the buffer. A subsequent IDirectInputDevice8::GetDeviceData call will read the same data. Normally, data is removed from the buffer after it is read. 

Return Value

    If the method succeeds, the return value is DI_OK or DI_BUFFEROVERFLOW.

    If the method fails, the return value can be one of the following error values.

    DIERR_INPUTLOST	Access to the input device has been lost. It must be reacquired.
    DIERR_INVALIDPARAM	An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard Component Object Model (COM) return value.
    DIERR_NOTACQUIRED	The operation cannot be performed unless the device is acquired.
    DIERR_NOTBUFFERED	The device is not buffered. Set the DIPROP_BUFFERSIZE property to enable buffering.
    DIERR_NOTINITIALIZED	The object has not been initialized.
    
    
--------------------------------

IDirectInputDevice8::Acquire Method

Obtains access to the input device.

Syntax

HRESULT Acquire(VOID);

Return Value

    If the method succeeds, the return value is DI_OK, or S_FALSE if the device was already acquired.

    If the method fails, the return value can be one of the following error values.

    DIERR_INVALIDPARAM	An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard Component Object Model (COM) return value.
    DIERR_NOTINITIALIZED	The object has not been initialized.
    DIERR_OTHERAPPHASPRIO	Another application has a higher priority level, preventing this call from succeeding. This value is equal to the E_ACCESSDENIED standard COM return value. This error can be returned when an application has only foreground access to a device but is attempting to acquire the device while in the background.

Remarks

    Before a device can be acquired, a data format must be set by using the IDirectInputDevice8::SetDataFormat method or IDirectInputDevice8::SetActionMap method. If the data format has not been set, IDirectInputDevice8::Acquire returns DIERR_INVALIDPARAM.

    Devices must be acquired before calling the IDirectInputDevice8::GetDeviceState or IDirectInputDevice8::GetDeviceData methods for that device.

    Device acquisition does not use a reference count. Therefore, if an application calls the IDirectInputDevice8::Acquire method twice, then calls the IDirectInputDevice8::Unacquire method once, the device is unacquired.

    If IDirectInputDevice8::BuildActionMap succeeds but no actions have been mapped, a subsequent call to IDirectInputDevice8::SetActionMap will return DI_OK but a call to IDirectInputDevice8::Acquire will fail with DIERR_INVALIDPARAM.
        
*/