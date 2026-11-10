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
import android.app.Application;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.games.AuthenticationResult;
import com.google.android.gms.games.GamesSignInClient;
import com.google.android.gms.games.PlayGames;
import com.google.android.gms.games.PlayGamesSdk;
import com.google.android.gms.games.gamessignin.AuthResponse;
import com.google.android.gms.games.gamessignin.AuthScope;

import java.util.Arrays;

/**
 * Google sign in for the game with Play Games Services v2 (com.google.android.gms:play-services-games-v2):
 * GamesSignInClient.isAuthenticated() for the silent login, signIn() when the person is asked, and
 * requestServerSideAccess() for the one time server auth code that the game's server exchanges. Play Games does not say
 * who the person is: the id token of Credential Manager (GoogleCredentialAuth) is where the account id comes from.
 *
 * VERSION: requestServerSideAccess() with a list of AuthScope (AuthResponse, in the package
 * com.google.android.gms.games.gamessignin) asks for the openid scope next to the games scope, so that the server gets
 * an id token when it exchanges the code. A version of the library that only has requestServerSideAccess(String, boolean),
 * which returns the code as a String, has to use that form; the server then gets no id token from the exchange and
 * verifies the id token of the login instead (setting proofKind "id-token").
 */
final class GooglePlayGamesAuth
{
    private GooglePlayGamesAuth()
    {
    }

    static void initialize(Application application)
    {
        PlayGamesSdk.initialize(application);
    }

    static boolean isAvailable(Activity activity)
    {
        return GoogleApiAvailability.getInstance().isGooglePlayServicesAvailable(activity) == ConnectionResult.SUCCESS;
    }

    static void signIn(final Activity activity, final int requestId, final boolean interactive, final String serverClientId,
                       final boolean useCredentialManager, final String nonce)
    {
        final GamesSignInClient client = PlayGames.getGamesSignInClient(activity);

        client.isAuthenticated().addOnCompleteListener(task ->
        {
            boolean authenticated = task.isSuccessful() && task.getResult() != null && task.getResult().isAuthenticated();
            if (authenticated)
            {
                requestCode(activity, requestId, interactive, serverClientId, useCredentialManager, nonce);
            }
            else
            if (!interactive)
            {
                GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_NOT_SIGNED_IN, "Play Games has no signed in player");
            }
            else
            {
                client.signIn().addOnCompleteListener(signInTask ->
                {
                    AuthenticationResult result = signInTask.isSuccessful() ? signInTask.getResult() : null;
                    if (result != null && result.isAuthenticated())
                    {
                        requestCode(activity, requestId, interactive, serverClientId, useCredentialManager, nonce);
                    }
                    else
                    {
                        GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_CANCELLED, "The player did not sign in to Play Games");
                    }
                });
            }
        });
    }

    private static void requestCode(final Activity activity, final int requestId, final boolean interactive, final String serverClientId,
                                    final boolean useCredentialManager, final String nonce)
    {
        PlayGames.getGamesSignInClient(activity)
            .requestServerSideAccess(serverClientId, false, Arrays.asList(AuthScope.OPEN_ID, AuthScope.PROFILE))
            .addOnCompleteListener(task ->
            {
                AuthResponse response = task.isSuccessful() ? task.getResult() : null;
                final String authCode = response != null ? response.getAuthCode() : null;
                if (authCode == null || authCode.isEmpty())
                {
                    GucefAuthBridge.fail(requestId, GucefAuthBridge.STATUS_ERROR, "Play Games gave no server auth code");
                    return;
                }
                if (!useCredentialManager)
                {
                    GucefAuthBridge.report(requestId, GucefAuthBridge.STATUS_OK, authCode, null, null, null, null, null, null);
                    return;
                }
                GoogleCredentialAuth.getIdToken(activity, interactive, serverClientId, nonce, new GoogleCredentialAuth.Callback()
                {
                    @Override
                    public void onToken(String idToken, String displayName, String avatarUrl)
                    {
                        GucefAuthBridge.report(requestId, GucefAuthBridge.STATUS_OK, authCode, idToken, null, null, displayName, avatarUrl, null);
                    }

                    @Override
                    public void onFailure(int status, String message)
                    {
                        GucefAuthBridge.fail(requestId, status, message);
                    }
                });
            });
    }
}
