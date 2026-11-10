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

package com.vanvelzensoftware.gucefauth;

import android.app.Activity;
import android.os.Build;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.HashSet;
import java.util.Set;

/**
 * The Java side of the gucefAUTH Android plugin. The C++ providers call the static methods of this class through JNI
 * (the methods that take a request id) and are answered through nativeOnSignInResult(); the browser flows hand their
 * redirect to onAuthRedirect().
 *
 * The application calls init() from its main Activity (onCreate) and onResume() from onResume(). The class loads the
 * plugin library and tells it that the Java side is there; load the GUCEF plugins afterwards.
 *
 * Every call into an SDK is made through a class of its own (GooglePlayGamesAuth, GoogleCredentialAuth,
 * AmazonLoginAuth, CustomTabsLauncher): when the library of an SDK is not part of the app, the class cannot be loaded,
 * the error is caught here and the feature is reported as not available instead of the app crashing.
 */
public final class GucefAuthBridge
{
    private static final String TAG = "GucefAuthBridge";

    /** The values of EAndroidSignInMode in the C++ code */
    public static final int MODE_SILENT = 0;
    public static final int MODE_INTERACTIVE = 1;

    /** The values of EAndroidAuthStatus in the C++ code */
    public static final int STATUS_OK = 0;
    public static final int STATUS_CANCELLED = 1;
    public static final int STATUS_NOT_SIGNED_IN = 2;
    public static final int STATUS_NO_CREDENTIAL = 3;
    public static final int STATUS_DENIED = 4;
    public static final int STATUS_NETWORK = 5;
    public static final int STATUS_ERROR = 6;

    private static WeakReference<Activity> sActivity = new WeakReference<Activity>(null);
    private static final Set<Integer> sCancelled = new HashSet<Integer>();

    static
    {
        // The name of the plugin library of the build, without lib and .so (a debug build may end in _d)
        System.loadLibrary("authpluginANDROID");
        nativeInit();
    }

    private GucefAuthBridge()
    {
    }

    // ---- called by the application

    /** Call from the onCreate of the main Activity. Initializes Play Games Services when its library is there */
    public static void init(Activity activity)
    {
        sActivity = new WeakReference<Activity>(activity);
        try
        {
            GooglePlayGamesAuth.initialize(activity.getApplication());
        }
        catch (Throwable t)
        {
            Log.i(TAG, "Play Games Services are not part of the app");
        }
    }

    /** Call from the onResume of the Activity: Login with Amazon needs it to get the answer of the authorization */
    public static void onResume(Activity activity)
    {
        sActivity = new WeakReference<Activity>(activity);
        try
        {
            AmazonLoginAuth.onResume();
        }
        catch (Throwable t)
        {
            // Login with Amazon is not part of the app
        }
    }

    // ---- called by the C++ side (the signatures are looked up by name in authpluginANDROID_JniGlue.cpp)

    public static boolean isGooglePlayAvailable()
    {
        Activity activity = sActivity.get();
        if (activity == null)
        {
            return false;
        }
        try
        {
            return GooglePlayGamesAuth.isAvailable(activity);
        }
        catch (Throwable t)
        {
            return false;
        }
    }

    /** Login with Amazon is for Fire OS: the library has to be part of the app and the device an Amazon one */
    public static boolean isAmazonAvailable()
    {
        if (!"Amazon".equalsIgnoreCase(Build.MANUFACTURER))
        {
            return false;
        }
        try
        {
            return AmazonLoginAuth.isAvailable();
        }
        catch (Throwable t)
        {
            return false;
        }
    }

    public static boolean googleSignIn(final int requestId, final int mode, final String serverClientId, final boolean useCredentialManager, final String nonce)
    {
        final Activity activity = sActivity.get();
        if (activity == null)
        {
            return false;
        }
        forget(requestId);
        try
        {
            GooglePlayGamesAuth.signIn(activity, requestId, mode == MODE_INTERACTIVE, serverClientId, useCredentialManager, nonce);
            return true;
        }
        catch (Throwable t)
        {
            Log.w(TAG, "Google sign in could not start: " + t.getClass().getSimpleName());
            return false;
        }
    }

    public static boolean amazonSignIn(final int requestId, final int mode, final String scopes)
    {
        final Activity activity = sActivity.get();
        if (activity == null)
        {
            return false;
        }
        forget(requestId);
        try
        {
            AmazonLoginAuth.signIn(activity, requestId, mode == MODE_INTERACTIVE, scopes);
            return true;
        }
        catch (Throwable t)
        {
            Log.w(TAG, "Login with Amazon could not start: " + t.getClass().getSimpleName());
            return false;
        }
    }

    /** The request is given up: its answer is not reported any more */
    public static void cancelRequest(int requestId)
    {
        synchronized (sCancelled)
        {
            sCancelled.add(requestId);
        }
    }

    public static void googleSignOut()
    {
        Activity activity = sActivity.get();
        if (activity == null)
        {
            return;
        }
        try
        {
            GoogleCredentialAuth.clearState(activity);
        }
        catch (Throwable t)
        {
            // Credential Manager is not part of the app
        }
    }

    public static void amazonSignOut()
    {
        Activity activity = sActivity.get();
        if (activity == null)
        {
            return;
        }
        try
        {
            AmazonLoginAuth.signOut(activity);
        }
        catch (Throwable t)
        {
            // Login with Amazon is not part of the app
        }
    }

    public static boolean openCustomTab(String url)
    {
        Activity activity = sActivity.get();
        if (activity == null)
        {
            return false;
        }
        try
        {
            return CustomTabsLauncher.open(activity, url);
        }
        catch (Throwable t)
        {
            Log.w(TAG, "The browser could not be opened: " + t.getClass().getSimpleName());
            return false;
        }
    }

    // ---- used by the SDK classes

    /** Reports the end of a request to the C++ side, unless the request was given up */
    static void report(int requestId, int status, String authCode, String idToken, String accessToken, String accountId,
                       String displayName, String avatarUrl, String message)
    {
        synchronized (sCancelled)
        {
            if (sCancelled.remove(requestId))
            {
                return;
            }
        }
        nativeOnSignInResult(requestId, status, authCode, idToken, accessToken, accountId, displayName, avatarUrl, message);
    }

    static void fail(int requestId, int status, String message)
    {
        report(requestId, status, null, null, null, null, null, null, message);
    }

    /** A new request starts with a clean slate: a stale cancellation of an earlier request with the same id does not apply */
    private static void forget(int requestId)
    {
        synchronized (sCancelled)
        {
            sCancelled.remove(requestId);
        }
    }

    // ---- JNI, implemented in authpluginANDROID_JniGlue.cpp

    private static native void nativeInit();

    private static native void nativeOnSignInResult(int requestId, int status, String authCode, String idToken, String accessToken,
                                                    String accountId, String displayName, String avatarUrl, String message);

    /** Hands the address of an app link or custom scheme redirect to the plugin. True when a login took it */
    public static native boolean onAuthRedirect(String url);
}
