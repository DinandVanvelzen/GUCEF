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
import android.os.CancellationSignal;

import androidx.core.content.ContextCompat;
import androidx.credentials.ClearCredentialStateRequest;
import androidx.credentials.Credential;
import androidx.credentials.CredentialManager;
import androidx.credentials.CredentialManagerCallback;
import androidx.credentials.CustomCredential;
import androidx.credentials.GetCredentialRequest;
import androidx.credentials.GetCredentialResponse;
import androidx.credentials.exceptions.ClearCredentialException;
import androidx.credentials.exceptions.GetCredentialCancellationException;
import androidx.credentials.exceptions.GetCredentialException;
import androidx.credentials.exceptions.NoCredentialException;

import com.google.android.libraries.identity.googleid.GetGoogleIdOption;
import com.google.android.libraries.identity.googleid.GetSignInWithGoogleOption;
import com.google.android.libraries.identity.googleid.GoogleIdTokenCredential;
import com.google.android.libraries.identity.googleid.GoogleIdTokenParsingException;

/**
 * The Google id token with Credential Manager (androidx.credentials, androidx.credentials:credentials-play-services-auth,
 * com.google.android.libraries.identity.googleid:googleid). Silent: the accounts that authorized the app before, chosen
 * automatically (GetGoogleIdOption). Interactive: the "Sign in with Google" sheet (GetSignInWithGoogleOption).
 * The nonce goes into the token; the C++ side checks it.
 */
final class GoogleCredentialAuth
{
    interface Callback
    {
        void onToken(String idToken, String displayName, String avatarUrl);

        void onFailure(int status, String message);
    }

    private GoogleCredentialAuth()
    {
    }

    static void getIdToken(Activity activity, boolean interactive, String serverClientId, String nonce, final Callback callback)
    {
        GetCredentialRequest.Builder request = new GetCredentialRequest.Builder();
        if (interactive)
        {
            GetSignInWithGoogleOption.Builder option = new GetSignInWithGoogleOption.Builder(serverClientId);
            if (nonce != null && !nonce.isEmpty())
            {
                option.setNonce(nonce);
            }
            request.addCredentialOption(option.build());
        }
        else
        {
            GetGoogleIdOption.Builder option = new GetGoogleIdOption.Builder()
                .setFilterByAuthorizedAccounts(true)
                .setServerClientId(serverClientId)
                .setAutoSelectEnabled(true);
            if (nonce != null && !nonce.isEmpty())
            {
                option.setNonce(nonce);
            }
            request.addCredentialOption(option.build());
        }

        CredentialManager.create(activity).getCredentialAsync(
            activity,
            request.build(),
            new CancellationSignal(),
            ContextCompat.getMainExecutor(activity),
            new CredentialManagerCallback<GetCredentialResponse, GetCredentialException>()
            {
                @Override
                public void onResult(GetCredentialResponse response)
                {
                    Credential credential = response.getCredential();
                    if (credential instanceof CustomCredential
                        && GoogleIdTokenCredential.TYPE_GOOGLE_ID_TOKEN_CREDENTIAL.equals(credential.getType()))
                    {
                        try
                        {
                            GoogleIdTokenCredential google = GoogleIdTokenCredential.createFrom(credential.getData());
                            callback.onToken(google.getIdToken(), google.getDisplayName(),
                                             google.getProfilePictureUri() != null ? google.getProfilePictureUri().toString() : null);
                        }
                        catch (GoogleIdTokenParsingException e)
                        {
                            callback.onFailure(GucefAuthBridge.STATUS_ERROR, "The Google id token credential cannot be read");
                        }
                    }
                    else
                    {
                        callback.onFailure(GucefAuthBridge.STATUS_ERROR, "Credential Manager gave a credential that is not a Google id token");
                    }
                }

                @Override
                public void onError(GetCredentialException e)
                {
                    if (e instanceof GetCredentialCancellationException)
                    {
                        callback.onFailure(GucefAuthBridge.STATUS_CANCELLED, "The person closed the Google sign in");
                    }
                    else
                    if (e instanceof NoCredentialException)
                    {
                        callback.onFailure(GucefAuthBridge.STATUS_NO_CREDENTIAL, "No Google account authorized this app yet");
                    }
                    else
                    {
                        callback.onFailure(GucefAuthBridge.STATUS_ERROR, "Credential Manager failed: " + e.getClass().getSimpleName());
                    }
                }
            });
    }

    /** Forgets the credential state of the app, so that the next silent login does not pick an account by itself */
    static void clearState(Activity activity)
    {
        CredentialManager.create(activity).clearCredentialStateAsync(
            new ClearCredentialStateRequest(),
            new CancellationSignal(),
            ContextCompat.getMainExecutor(activity),
            new CredentialManagerCallback<Void, ClearCredentialException>()
            {
                @Override
                public void onResult(Void result)
                {
                }

                @Override
                public void onError(ClearCredentialException e)
                {
                }
            });
    }
}
