/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H
#include "authpluginANDROID_CAndroidAuthBridge.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H ? */

#include "authpluginANDROID_JniGlue.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

#include <jni.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The Java class, its methods and the virtual machine: filled by nativeInit() */
struct SJavaSide
{
    JavaVM* vm;
    jclass bridgeClass;
    jmethodID isGooglePlayAvailable;
    jmethodID isAmazonAvailable;
    jmethodID googleSignIn;
    jmethodID amazonSignIn;
    jmethodID cancelRequest;
    jmethodID googleSignOut;
    jmethodID amazonSignOut;
    jmethodID openCustomTab;

    SJavaSide( void )
        : vm( GUCEF_NULL ), bridgeClass( GUCEF_NULL ), isGooglePlayAvailable( GUCEF_NULL ), isAmazonAvailable( GUCEF_NULL ), googleSignIn( GUCEF_NULL )
        , amazonSignIn( GUCEF_NULL ), cancelRequest( GUCEF_NULL ), googleSignOut( GUCEF_NULL ), amazonSignOut( GUCEF_NULL ), openCustomTab( GUCEF_NULL )
    {}
};

/** The environment of the current thread: a thread of the JVM has one, any other thread is attached for the time of a call */
class CJniScope
{
    public:

    JNIEnv* env;

    explicit CJniScope( JavaVM* vm )
        : env( GUCEF_NULL ), m_vm( vm ), m_attached( false )
    {
        if ( GUCEF_NULL == m_vm )
            return;
        jint result = m_vm->GetEnv( reinterpret_cast< void** >( &env ), JNI_VERSION_1_6 );
        if ( JNI_EDETACHED == result )
        {
            if ( JNI_OK == m_vm->AttachCurrentThread( &env, GUCEF_NULL ) )
                m_attached = true;
            else
                env = GUCEF_NULL;
        }
        else
        if ( JNI_OK != result )
        {
            env = GUCEF_NULL;
        }
    }

    ~CJniScope()
    {
        if ( m_attached )
            m_vm->DetachCurrentThread();
    }

    private:

    CJniScope( const CJniScope& src );
    CJniScope& operator=( const CJniScope& src );

    JavaVM* m_vm;
    bool m_attached;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static SJavaSide g_java;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** True, and the exception is cleared, when the call threw: the exception text can hold what was asked and is not looked at */
static bool
CallFailed( JNIEnv* env )
{GUCEF_TRACE;

    if ( env->ExceptionCheck() )
    {
        env->ExceptionClear();
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AndroidJni: A call into the Java side threw an exception" );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ToCString( JNIEnv* env, jstring text )
{GUCEF_TRACE;

    CORE::CString result;
    if ( GUCEF_NULL == text )
        return result;
    const char* chars = env->GetStringUTFChars( text, GUCEF_NULL );
    if ( GUCEF_NULL != chars )
    {
        result = chars;
        env->ReleaseStringUTFChars( text, chars );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static bool
JniIsGooglePlayAvailable( void* context )
{GUCEF_TRACE;

    CJniScope scope( g_java.vm );
    if ( GUCEF_NULL == scope.env )
        return false;
    jboolean available = scope.env->CallStaticBooleanMethod( g_java.bridgeClass, g_java.isGooglePlayAvailable );
    return !CallFailed( scope.env ) && JNI_TRUE == available;
}

/*-------------------------------------------------------------------------*/

static bool
JniIsAmazonAvailable( void* context )
{GUCEF_TRACE;

    CJniScope scope( g_java.vm );
    if ( GUCEF_NULL == scope.env )
        return false;
    jboolean available = scope.env->CallStaticBooleanMethod( g_java.bridgeClass, g_java.isAmazonAvailable );
    return !CallFailed( scope.env ) && JNI_TRUE == available;
}

/*-------------------------------------------------------------------------*/

static bool
JniGoogleSignIn( void* context, UInt32 requestId, EAndroidSignInMode mode, const char* serverClientId, bool useCredentialManager, const char* nonce )
{GUCEF_TRACE;

    CJniScope scope( g_java.vm );
    if ( GUCEF_NULL == scope.env )
        return false;
    JNIEnv* env = scope.env;
    jstring clientId = env->NewStringUTF( GUCEF_NULL != serverClientId ? serverClientId : "" );
    jstring nonceText = env->NewStringUTF( GUCEF_NULL != nonce ? nonce : "" );
    jboolean started = env->CallStaticBooleanMethod( g_java.bridgeClass, g_java.googleSignIn, (jint) requestId, (jint) mode, clientId,
                                                     useCredentialManager ? JNI_TRUE : JNI_FALSE, nonceText );
    env->DeleteLocalRef( clientId );
    env->DeleteLocalRef( nonceText );
    return !CallFailed( env ) && JNI_TRUE == started;
}

/*-------------------------------------------------------------------------*/

static bool
JniAmazonSignIn( void* context, UInt32 requestId, EAndroidSignInMode mode, const char* scopes )
{GUCEF_TRACE;

    CJniScope scope( g_java.vm );
    if ( GUCEF_NULL == scope.env )
        return false;
    JNIEnv* env = scope.env;
    jstring scopeText = env->NewStringUTF( GUCEF_NULL != scopes ? scopes : "" );
    jboolean started = env->CallStaticBooleanMethod( g_java.bridgeClass, g_java.amazonSignIn, (jint) requestId, (jint) mode, scopeText );
    env->DeleteLocalRef( scopeText );
    return !CallFailed( env ) && JNI_TRUE == started;
}

/*-------------------------------------------------------------------------*/

static void
JniCancelRequest( void* context, UInt32 requestId )
{GUCEF_TRACE;

    CJniScope scope( g_java.vm );
    if ( GUCEF_NULL == scope.env )
        return;
    scope.env->CallStaticVoidMethod( g_java.bridgeClass, g_java.cancelRequest, (jint) requestId );
    CallFailed( scope.env );
}

/*-------------------------------------------------------------------------*/

static void
JniGoogleSignOut( void* context )
{GUCEF_TRACE;

    CJniScope scope( g_java.vm );
    if ( GUCEF_NULL == scope.env )
        return;
    scope.env->CallStaticVoidMethod( g_java.bridgeClass, g_java.googleSignOut );
    CallFailed( scope.env );
}

/*-------------------------------------------------------------------------*/

static void
JniAmazonSignOut( void* context )
{GUCEF_TRACE;

    CJniScope scope( g_java.vm );
    if ( GUCEF_NULL == scope.env )
        return;
    scope.env->CallStaticVoidMethod( g_java.bridgeClass, g_java.amazonSignOut );
    CallFailed( scope.env );
}

/*-------------------------------------------------------------------------*/

static bool
JniOpenCustomTab( void* context, const char* url )
{GUCEF_TRACE;

    CJniScope scope( g_java.vm );
    if ( GUCEF_NULL == scope.env )
        return false;
    JNIEnv* env = scope.env;
    jstring urlText = env->NewStringUTF( GUCEF_NULL != url ? url : "" );
    jboolean opened = env->CallStaticBooleanMethod( g_java.bridgeClass, g_java.openCustomTab, urlText );
    env->DeleteLocalRef( urlText );
    return !CallFailed( env ) && JNI_TRUE == opened;
}

/*-------------------------------------------------------------------------*/

bool
IsJavaSideInitialized( void )
{GUCEF_TRACE;

    return GUCEF_NULL != g_java.bridgeClass;
}

/*-------------------------------------------------------------------------*/

void
ReleaseJavaSide( void )
{GUCEF_TRACE;

    CAndroidAuthBridge::Instance()->SetShim( TAndroidShim() );
    if ( GUCEF_NULL != g_java.bridgeClass )
    {
        CJniScope scope( g_java.vm );
        if ( GUCEF_NULL != scope.env )
            scope.env->DeleteGlobalRef( g_java.bridgeClass );
    }
    g_java = SJavaSide();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      JNI ENTRIES                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF::AUTHPLUGIN::ANDROIDAUTH;

/** The Java side is up: the table of functions of the bridge is filled and the providers can be used */
extern "C" JNIEXPORT void JNICALL
Java_com_vanvelzensoftware_gucefauth_GucefAuthBridge_nativeInit( JNIEnv* env, jclass bridgeClass )
{GUCEF_TRACE;

    if ( IsJavaSideInitialized() )
        return;

    SJavaSide java;
    if ( JNI_OK != env->GetJavaVM( &java.vm ) )
        return;
    java.bridgeClass = static_cast< jclass >( env->NewGlobalRef( bridgeClass ) );
    java.isGooglePlayAvailable = env->GetStaticMethodID( bridgeClass, "isGooglePlayAvailable", "()Z" );
    java.isAmazonAvailable = env->GetStaticMethodID( bridgeClass, "isAmazonAvailable", "()Z" );
    java.googleSignIn = env->GetStaticMethodID( bridgeClass, "googleSignIn", "(IILjava/lang/String;ZLjava/lang/String;)Z" );
    java.amazonSignIn = env->GetStaticMethodID( bridgeClass, "amazonSignIn", "(IILjava/lang/String;)Z" );
    java.cancelRequest = env->GetStaticMethodID( bridgeClass, "cancelRequest", "(I)V" );
    java.googleSignOut = env->GetStaticMethodID( bridgeClass, "googleSignOut", "()V" );
    java.amazonSignOut = env->GetStaticMethodID( bridgeClass, "amazonSignOut", "()V" );
    java.openCustomTab = env->GetStaticMethodID( bridgeClass, "openCustomTab", "(Ljava/lang/String;)Z" );

    if ( env->ExceptionCheck() || GUCEF_NULL == java.isGooglePlayAvailable || GUCEF_NULL == java.isAmazonAvailable || GUCEF_NULL == java.googleSignIn ||
         GUCEF_NULL == java.amazonSignIn || GUCEF_NULL == java.cancelRequest || GUCEF_NULL == java.googleSignOut || GUCEF_NULL == java.amazonSignOut ||
         GUCEF_NULL == java.openCustomTab )
    {
        env->ExceptionClear();
        env->DeleteGlobalRef( java.bridgeClass );
        GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "AndroidJni: GucefAuthBridge does not have the methods that the plugin expects, the Java sources and the plugin do not match" );
        return;
    }

    g_java = java;

    TAndroidShim shim;
    shim.context = &g_java;
    shim.isGooglePlayAvailable = &JniIsGooglePlayAvailable;
    shim.isAmazonAvailable = &JniIsAmazonAvailable;
    shim.googleSignIn = &JniGoogleSignIn;
    shim.amazonSignIn = &JniAmazonSignIn;
    shim.cancelRequest = &JniCancelRequest;
    shim.googleSignOut = &JniGoogleSignOut;
    shim.amazonSignOut = &JniAmazonSignOut;
    shim.openCustomTab = &JniOpenCustomTab;
    CAndroidAuthBridge::Instance()->SetShim( shim );
    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "AndroidJni: The Java side of the auth plugin is initialized" );
}

/*-------------------------------------------------------------------------*/

/** The end of a request of the Java side, on any thread of the JVM */
extern "C" JNIEXPORT void JNICALL
Java_com_vanvelzensoftware_gucefauth_GucefAuthBridge_nativeOnSignInResult( JNIEnv* env, jclass bridgeClass, jint requestId, jint status,
                                                                         jstring authCode, jstring idToken, jstring accessToken, jstring accountId,
                                                                         jstring displayName, jstring avatarUrl, jstring message )
{GUCEF_TRACE;

    SAndroidAuthResult result;
    result.requestId = static_cast< GUCEF::UInt32 >( requestId );
    result.status = status >= ANDROIDAUTH_OK && status <= ANDROIDAUTH_ERROR ? static_cast< EAndroidAuthStatus >( status ) : ANDROIDAUTH_ERROR;
    if ( GUCEF_NULL != authCode )
        result.authCode.Set( ToCString( env, authCode ) );
    if ( GUCEF_NULL != idToken )
        result.idToken.Set( ToCString( env, idToken ) );
    if ( GUCEF_NULL != accessToken )
        result.accessToken.Set( ToCString( env, accessToken ) );
    result.accountId = ToCString( env, accountId );
    result.displayName = ToCString( env, displayName );
    result.avatarUrl = ToCString( env, avatarUrl );
    result.message = ToCString( env, message );
    CAndroidAuthBridge::Instance()->DeliverResult( result );
}

/*-------------------------------------------------------------------------*/

/** The address Android handed to the app at the end of a login in a browser (app link or custom scheme) */
extern "C" JNIEXPORT jboolean JNICALL
Java_com_vanvelzensoftware_gucefauth_GucefAuthBridge_onAuthRedirect( JNIEnv* env, jclass bridgeClass, jstring url )
{GUCEF_TRACE;

    if ( GUCEF_NULL == url )
        return JNI_FALSE;
    return CAndroidAuthBridge::Instance()->DeliverRedirect( ToCString( env, url ) ) ? JNI_TRUE : JNI_FALSE;
}

#else /* not Android */

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

bool
IsJavaSideInitialized( void )
{GUCEF_TRACE;

    return false;
}

void
ReleaseJavaSide( void )
{GUCEF_TRACE;

    // Whatever table was installed by a test stays with the test
}

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

#endif /* GUCEF_PLATFORM_ANDROID ? */

/*-------------------------------------------------------------------------*/
