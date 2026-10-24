# gucefGAMEGUI

Module `platform/gucefGAMEGUI`, namespace `GUCEF::GAMEGUI`, files `gucefGAMEGUI_*.h/.cpp`. Prebaked forms for rapid game development on gucefGUI: the multiplayer screens (game browser, lobby, chat, …) on gucefMULTIPLAY, and in time other forms games commonly need. Depends on gucefMT, gucefCORE, gucefGUI and gucefMULTIPLAY (and what they bring). Tests: `tests/gucefGAMEGUI_TestApp` on the `Null` GUI driver.

## 1. Conventions

The forms follow gucefGUI's prebaked forms (`CMessageBoxForm`, `CSettingsForm`, `CPathFieldForm`):
* a form is `C<Thing>Form : GUI::CFormEx`;
* `Build()` makes its layout in code: `BuildLayout( CDataNode& )` with `GUI::CFormLayout::AddWidgetNode`, then `LoadLayout`;
* `OnPostLayoutLoad()` finds the widgets and subscribes to their events;
* static `CORE::CEvent`s with `RegisterEvents()`, registered by `CGameGuiGlobal` (set up by the module);
* embeddable sub-forms take a placeholder widget (`Build( GUI::CWidget* placeholder )`) as `CPathFieldForm` does;
* the forms are designed for `GUI::BuildDarkTheme()` and look right on both GUI drivers through the shared widget painter.

The forms talk only to the gucefMULTIPLAY interfaces (`CGameBrowser`, `CILobbyService`, `CILobby`, `CIChatService`, `CPingService`), never to a particular backend.

## 2. Forms

| form | what it shows / does |
|---|---|
| `CServerBrowserForm` | GridView: name, kind (lobby / in progress), players, map, mode, ping, lock, source; sortable. Filter bar (text, kind, hide full, hide passworded, max ping). Buttons Join, Spectate, Refresh, Create, Direct connect. Bound to a `CGameBrowser`. Events `JoinRequestedEvent`, `SpectateRequestedEvent`, `CreateRequestedEvent`, `DirectConnectRequestedEvent` (with the listing) |
| `CCreateLobbyForm` | name, password, max players, spectators, LAN / internet, service; the schema's settings. `CreateRequestedEvent` with `CLobbyCreateParams` |
| `CLobbyForm` | slots GridView (slot, team, player, faction, colour, ready, latency); combos for own faction / colour; settings panel; ready and spectate checkboxes; Start / Force start / Vote start; Leave; embedded chat; countdown line. Bound to a `CILobby`; buttons enabled by role and phase. `LaunchEvent` passes the launch description on |
| `CLobbySettingsPanelForm` | embeddable; makes a control per `CLobbySettingDef` (checkbox, spinner, combobox, editbox), enabled when the member may change it, "propose" when the policy is VOTE |
| `CChatForm` | embeddable; a TabControl with a tab per channel and per direct conversation, the message Textbox, the members Listbox (popup menu: direct message), the input Editbox. Bound to a `CIChatService` (and optionally one channel only, as in the lobby) |
| `CVotePromptForm` | modal: what is proposed by whom, Yes / No, the time left |
| `CConnectingForm` | the hand-over: which endpoint is tried, relay fallback, transfer progress, Cancel |
| `CDirectConnectForm` | address (host:port or URI) and password; `ConnectRequestedEvent` |
| `CPlayerProfileForm` | display name (and the user id, read only); `ProfileChangedEvent` |

## 3. Tests

Every form is built on the `Null` driver at 1280 x 960 with the test font and the dark theme, against `CTestLobbyService` / `CTestChatService` / a browser fed by a test listing provider (test doubles in the test app):
* rows appear and update;
* filters and sorting;
* buttons enabled by role and phase;
* a vote prompt appears and casts;
* chat messages show in their tab;
* events carry the right data.
