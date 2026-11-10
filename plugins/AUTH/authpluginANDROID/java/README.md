# Java side of authpluginANDROID

Plain source files for the package `com.vanvelzensoftware.gucefauth`. They are not built by the GUCEF build (that needs
the Android SDK and a JDK): copy the directory `com` into the `src/main/java` of the Android module of the app, or
add this directory as a source set. The C++ side is `authpluginANDROID`.

These sources are not yet built or tested against the Android SDK, a JDK or a device.
They are small and follow the documented APIs of Play Games Services v2, Credential Manager, Login with Amazon and
Custom Tabs; the places that depend on the version of a library are marked `VERSION` in the code.

## Classes

| class | what it does |
|---|---|
| `GucefAuthBridge` | loads the plugin library, is called by the C++ providers through JNI and answers through `nativeOnSignInResult`; `onAuthRedirect` hands redirects to the plugin |
| `GooglePlayGamesAuth` | Play Games Services v2: `isAuthenticated`, `signIn`, `requestServerSideAccess` |
| `GoogleCredentialAuth` | Credential Manager: the Google id token (silent with authorized accounts, "Sign in with Google" when interactive) |
| `AmazonLoginAuth` | Login with Amazon: `getToken` (silent), `authorize` (interactive), `User.fetch` |
| `CustomTabsLauncher` | opens a url in a Custom Tab; the browser launcher of the plugin uses it |
| `AuthRedirectActivity` | receives the app link or custom scheme redirect and hands it to the plugin |

## Dependencies of the Android module

Versions are the ones the app picks; the plugin only uses the interfaces named below.

```gradle
dependencies {
    // Google sign in with Play Games Services v2 (GooglePlayGamesAuth)
    implementation 'com.google.android.gms:play-services-games-v2:<version>'

    // Credential Manager and Sign in with Google (GoogleCredentialAuth)
    implementation 'androidx.credentials:credentials:<version>'
    implementation 'androidx.credentials:credentials-play-services-auth:<version>'
    implementation 'com.google.android.libraries.identity.googleid:googleid:<version>'

    // Custom Tabs (CustomTabsLauncher)
    implementation 'androidx.browser:browser:<version>'

    // Login with Amazon (AmazonLoginAuth), Fire OS: the library comes from the Amazon developer portal and is not
    // distributed with GUCEF. Put login-with-amazon-sdk.jar in app/libs and add:
    implementation files('libs/login-with-amazon-sdk.jar')
}
```

An app that only uses some of the features leaves the others out: the missing library is noticed when the feature is used
and the provider reports that it is not available.

## Manifest

The redirect of the browser flows (OAuth/OIDC with `redirectMode` `custom-scheme`) reaches the app as an app link or a
custom scheme. Register the redirect uri that is registered at the provider (the scheme and host are the app's):

```xml
<activity
    android:name="com.vanvelzensoftware.gucefauth.AuthRedirectActivity"
    android:exported="true"
    android:launchMode="singleTask"
    android:noHistory="true">
    <intent-filter>
        <action android:name="android.intent.action.VIEW" />
        <category android:name="android.intent.category.DEFAULT" />
        <category android:name="android.intent.category.BROWSABLE" />
        <data android:scheme="com.example.game" android:host="oauth2redirect" />
    </intent-filter>
</activity>
```

Login with Amazon also wants its `assets/api_key.txt` and the `AuthorizationActivity` entries of its documentation.
Play Games Services needs `<meta-data android:name="com.google.android.gms.games.APP_ID" android:value="@string/app_id" />`.

## Wiring in the app

```java
// Activity.onCreate
GucefAuthBridge.init(this);

// Activity.onResume
GucefAuthBridge.onResume(this);
```

Load the plugin after that (`authpluginANDROID` in the bootstrap plugins): the C++ side finds the Java side by the time
the plugin loads. The library name in `GucefAuthBridge` (`authpluginANDROID`) is the name of the library of the build
without `lib` and `.so`; a debug build may end in `_d`.

In the settings of the application (`Auth/Providers/google`, `Auth/Providers/amazon`):

```json
{
    "Auth": {
        "Providers": {
            "google": { "clientId": "<web client id of the game's server>", "Parameters": { "useCredentialManager": "true", "proofKind": "auth-code" } },
            "amazon": { "clientId": "<client id of the security profile, only needed on the server that verifies>", "Parameters": { "amazonScopes": "profile" } }
        }
    }
}
```

The OAuth/OIDC engine asks for the browser with `CORE::OpenUrlInSystemBrowser()`; the plugin installs a handler that
calls `GucefAuthBridge.openCustomTab()`, and it hands the redirect that `AuthRedirectActivity` receives to
`IAuthFlowEngine::SubmitRedirectUrl()` of the engine through `CEngineRedirectReceiver`. An application that wants to take
the redirect itself registers its own `CAndroidRedirectReceiver` with `CAndroidAuthBridge::SetRedirectReceiver()`.

## What the Java side answers with

`GucefAuthBridge.report()` calls `nativeOnSignInResult( requestId, status, authCode, idToken, accessToken, accountId,
displayName, avatarUrl, message )`. The numbers of `status` are the constants `STATUS_*` of `GucefAuthBridge` and the
values of `EAndroidAuthStatus` in `authpluginANDROID_AndroidShim.h`; keep them equal. The auth code, the id token and
the access token are credentials: the Java side does not log them.
