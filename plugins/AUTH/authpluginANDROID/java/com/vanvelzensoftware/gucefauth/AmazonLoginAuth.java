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

import com.amazon.identity.auth.device.AuthError;
import com.amazon.identity.auth.device.api.Listener;
import com.amazon.identity.auth.device.api.authorization.AuthCancellation;
import com.amazon.identity.auth.device.api.authorization.AuthorizationManager;
import com.amazon.identity.auth.device.api.authorization.AuthorizeListener;
import com.amazon.identity.auth.device.api.authorization.AuthorizeRequest;
import com.amazon.identity.auth.device.api.authorization.AuthorizeResult;
import com.amazon.identity.auth.device.api.authorization.Scope;
import com.amazon.identity.auth.device.api.authorization.ScopeFactory;
import com.amazon.identity.auth.device.api.authorization.User;
import com.amazon.identity.auth.device.api.workflow.RequestContext;

/**
 * Login with Amazon (the Login with Amazon SDK for Android, login-with-amazon-sdk.jar). The app is identified by the
 * API key in assets/api_key.txt that Amazon issues for the app's package and signing key.
 * Silent: AuthorizationManager.getToken() only gives a token for an app that was authorized before.
 * Interactive: AuthorizationManager.authorize(); the answer comes to the RequestContext, which needs onResume().
 * Either way the user id comes from User.fetch(). The scopes are given as names ("profile", "profile:user_id").
 */
final class AmazonLoginAuth
{
    private static RequestContext sRequestContext;

    private AmazonLoginAuth()
    {
    }

    static boolean isAvailable()
    {
        // The class is there when the library is part of the app
        return AuthorizationManager.class != null;
    }

    static void onResume()
    {
        if (sRequestContext != null)
        {
            sRequestContext.onResume();
        }
    }

    private static Scope[] toScopes(String scopes)
    {
        String[] names = scopes == null || scopes.trim().isEmpty() ? new String[] { "profile" } : scopes.trim().split("\\s+");
        Scope[] result = new Scope[names.length];
        for (int i = 0; i < names.length; ++i)
        {
            result[i] = ScopeFactory.scopeNamed(names[i]);
        }
        return result;
    }

    static void signIn(final Activity activity, final int requestId, final boolean interactive, String scopeNames)
    {
        final Scope[] scopes = toScopes(scopeNames);

        if (!interactive)
        {
            AuthorizationManager.getToken(activity, scopes, new Listener<AuthorizeResult, AuthError>()
            {
                @Override
                public void onSuccess(AuthorizeResult result)
                {
                    if (result.getAccessToken() == null)
                    {
                        GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_NOT_SIGNED_IN, "The app was not authorized for Login with Amazon yet");
                    }
                    else
                    {
                        fetchUser(activity, requestId, result.getAccessToken());
                    }
                }

                @Override
                public void onError(AuthError error)
                {
                    GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_ERROR, "Login with Amazon failed: " + error.getType());
                }
            });
            return;
        }

        sRequestContext = RequestContext.create(activity);
        sRequestContext.registerListener(new AuthorizeListener()
        {
            @Override
            public void onSuccess(AuthorizeResult result)
            {
                if (result.getAccessToken() == null)
                {
                    GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_DENIED, "Login with Amazon gave no access token");
                }
                else
                {
                    fetchUser(activity, requestId, result.getAccessToken());
                }
            }

            @Override
            public void onError(AuthError error)
            {
                GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_ERROR, "Login with Amazon failed: " + error.getType());
            }

            @Override
            public void onCancel(AuthCancellation cancellation)
            {
                GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_CANCELLED, "The person closed Login with Amazon");
            }
        });
        AuthorizationManager.authorize(new AuthorizeRequest.Builder(sRequestContext).addScopes(scopes).build());
    }

    private static void fetchUser(Activity activity, final int requestId, final String accessToken)
    {
        User.fetch(activity, new Listener<User, AuthError>()
        {
            @Override
            public void onSuccess(User user)
            {
                GucefAuthBridge.report(requestId, GucefAuthBridge.STATUS_OK, null, null, accessToken, user.getUserId(), user.getUserName(), null, null);
            }

            @Override
            public void onError(AuthError error)
            {
                GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_ERROR, "The Login with Amazon profile could not be read: " + error.getType());
            }
        });
    }

    static void signOut(Activity activity)
    {
        AuthorizationManager.signOut(activity, new Listener<Void, AuthError>()
        {
            @Override
            public void onSuccess(Void result)
            {
            }

            @Override
            public void onError(AuthError error)
            {
            }
        });
    }
}
