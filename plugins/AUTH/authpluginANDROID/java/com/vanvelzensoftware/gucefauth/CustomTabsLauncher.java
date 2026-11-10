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
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.net.Uri;

import androidx.browser.customtabs.CustomTabsIntent;

/**
 * Opens a url in a Custom Tab (androidx.browser:browser), the system browser of the person with its sessions, never a
 * web view of the app. Where no browser supports Custom Tabs the url goes to the default browser.
 */
final class CustomTabsLauncher
{
    private CustomTabsLauncher()
    {
    }

    static boolean open(Activity activity, String url)
    {
        Uri uri = Uri.parse(url);
        try
        {
            new CustomTabsIntent.Builder().build().launchUrl(activity, uri);
            return true;
        }
        catch (ActivityNotFoundException e)
        {
            try
            {
                activity.startActivity(new Intent(Intent.ACTION_VIEW, uri));
                return true;
            }
            catch (ActivityNotFoundException second)
            {
                return false;
            }
        }
    }
}
