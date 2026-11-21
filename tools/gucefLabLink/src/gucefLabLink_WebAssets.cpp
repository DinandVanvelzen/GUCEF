/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#include "gucefLabLink_WebAssets.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** index.html */
static const char IndexHtml[] =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "<meta charset=\"utf-8\">\n"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
    "<title>Lab hub</title>\n"
    "<link rel=\"stylesheet\" href=\"/app.css\">\n"
    "<script src=\"/app.js\" defer></script>\n"
    "</head>\n"
    "<body>\n"
    "<header>\n"
    "  <h1 id=\"hub-name\">Lab hub</h1>\n"
    "  <nav id=\"who\" hidden>\n"
    "    <span id=\"who-name\"></span>\n"
    "    <button type=\"button\" id=\"show-nodes\">Nodes</button>\n"
    "    <button type=\"button\" id=\"show-users\" hidden>Users</button>\n"
    "    <button type=\"button\" id=\"logout\">Sign out</button>\n"
    "  </nav>\n"
    "</header>\n"
    "<p id=\"plain-warning\" class=\"warning\" hidden>This site is served over plain HTTP: what is typed here, passwords included, crosses the network unencrypted. Use HTTPS with the certificate of the cluster.</p>\n"
    "<main>\n"
    "  <section id=\"login-view\" hidden>\n"
    "    <form id=\"login-form\" autocomplete=\"on\">\n"
    "      <h2>Sign in</h2>\n"
    "      <label>User name <input name=\"username\" autocomplete=\"username\" maxlength=\"64\"></label>\n"
    "      <label>Password <input name=\"password\" type=\"password\" autocomplete=\"current-password\" maxlength=\"1024\"></label>\n"
    "      <label class=\"inline\"><input name=\"stay\" type=\"checkbox\" value=\"1\"> Stay signed in</label>\n"
    "      <details>\n"
    "        <summary>Sign in with the access token</summary>\n"
    "        <label>Token <input name=\"token\" type=\"password\" autocomplete=\"off\"></label>\n"
    "      </details>\n"
    "      <button type=\"submit\">Sign in</button>\n"
    "      <p id=\"login-error\" class=\"error\" role=\"alert\"></p>\n"
    "    </form>\n"
    "  </section>\n"
    "  <section id=\"nodes-view\" hidden>\n"
    "    <p id=\"status-line\" class=\"muted\"></p>\n"
    "    <div class=\"scroll\">\n"
    "      <table id=\"nodes\">\n"
    "        <thead>\n"
    "          <tr><th>Name</th><th>OS</th><th>State</th><th>Last online</th><th>GPU / VRAM</th><th>Can run</th><th>Hosts</th><th>Workloads</th><th>Agent</th></tr>\n"
    "        </thead>\n"
    "        <tbody></tbody>\n"
    "      </table>\n"
    "    </div>\n"
    "  </section>\n"
    "  <section id=\"users-view\" hidden>\n"
    "    <h2>Users</h2>\n"
    "    <table id=\"users\">\n"
    "      <thead><tr><th>Name</th><th>Role</th><th>State</th><th>Created</th><th></th></tr></thead>\n"
    "      <tbody></tbody>\n"
    "    </table>\n"
    "    <form id=\"add-user-form\">\n"
    "      <h3>Add a user</h3>\n"
    "      <label>User name <input name=\"name\" autocomplete=\"off\" maxlength=\"64\" required></label>\n"
    "      <label>Role <select name=\"role\"><option value=\"viewer\">viewer</option><option value=\"admin\">admin</option></select></label>\n"
    "      <label>Password <input name=\"password\" type=\"password\" autocomplete=\"new-password\" maxlength=\"1024\" required></label>\n"
    "      <button type=\"submit\">Add</button>\n"
    "      <p id=\"users-message\" class=\"error\" role=\"alert\"></p>\n"
    "    </form>\n"
    "  </section>\n"
    "</main>\n"
    "</body>\n"
    "</html>\n";

/** app.js */
static const char AppJs[] =
    "'use strict';\n"
    "(function () {\n"
    "  var POLL_MS = 5000;\n"
    "  var timer = null;\n"
    "  var session = null;\n"
    "\n"
    "  function $(id) { return document.getElementById(id); }\n"
    "\n"
    "  function el(tag, cls, text) {\n"
    "    var e = document.createElement(tag);\n"
    "    if (cls) { e.className = cls; }\n"
    "    if (text !== undefined && text !== null) { e.textContent = String(text); }\n"
    "    return e;\n"
    "  }\n"
    "\n"
    "  function api(method, path, form) {\n"
    "    var options = { method: method, credentials: 'same-origin', cache: 'no-store', headers: { 'X-LabLink-Csrf': '1' } };\n"
    "    if (form) {\n"
    "      options.headers['Content-Type'] = 'application/x-www-form-urlencoded';\n"
    "      options.body = new URLSearchParams(form).toString();\n"
    "    }\n"
    "    return fetch(path, options).then(function (r) {\n"
    "      return r.text().then(function (t) {\n"
    "        var body = null;\n"
    "        try { body = t ? JSON.parse(t) : null; } catch (e) { body = null; }\n"
    "        return { status: r.status, body: body };\n"
    "      });\n"
    "    });\n"
    "  }\n"
    "\n"
    "  function errorOf(answer, fallback) {\n"
    "    return (answer.body && answer.body.error) ? answer.body.error : fallback;\n"
    "  }\n"
    "\n"
    "  function show(view) {\n"
    "    ['login-view', 'nodes-view', 'users-view'].forEach(function (id) { $(id).hidden = (id !== view); });\n"
    "  }\n"
    "\n"
    "  function duration(secs) {\n"
    "    if (secs < 60) { return secs + ' s'; }\n"
    "    if (secs < 3600) { return Math.floor(secs / 60) + ' min'; }\n"
    "    if (secs < 172800) { return Math.floor(secs / 3600) + ' h'; }\n"
    "    return Math.floor(secs / 86400) + ' days';\n"
    "  }\n"
    "\n"
    "  function when(unixSecs) {\n"
    "    return unixSecs ? new Date(unixSecs * 1000).toLocaleString() : 'never';\n"
    "  }\n"
    "\n"
    "  function gib(mib) { return (mib / 1024).toFixed(1) + ' GiB'; }\n"
    "\n"
    "  function stateCell(node, hubTime) {\n"
    "    var td = el('td');\n"
    "    var main;\n"
    "    if (node.status === 'online') {\n"
    "      var busy = node.nodeState === 'busy' || node.nodeState === 'throttled';\n"
    "      main = el('div', busy ? 'state-busy' : 'state-online', busy ? 'online, busy with its user' : 'online now');\n"
    "      if (node.nodeState && node.nodeState !== 'available' && !busy) { main.textContent = 'online, ' + node.nodeState; }\n"
    "    } else if (node.status === 'offline') {\n"
    "      main = el('div', 'state-offline', 'offline since ' + when(node.lastSeen));\n"
    "      main.title = duration(Math.max(0, hubTime - node.lastSeen)) + ' ago';\n"
    "    } else {\n"
    "      main = el('div', 'state-unknown', 'unknown (no report since the hub started)');\n"
    "    }\n"
    "    td.appendChild(main);\n"
    "    if (!node.verified) {\n"
    "      var flag = el('div', 'flag', 'unverified');\n"
    "      flag.title = 'Reports with the lab secret, not with an approval: never given work';\n"
    "      td.appendChild(flag);\n"
    "    }\n"
    "    if (node.duplicateInstanceRefusals > 0) {\n"
    "      td.appendChild(el('div', 'alarm', 'second instance refused ' + node.duplicateInstanceRefusals + ' times (cloned disk?)'));\n"
    "    }\n"
    "    return td;\n"
    "  }\n"
    "\n"
    "  function listCell(items, render) {\n"
    "    var td = el('td');\n"
    "    if (!items || items.length === 0) { td.appendChild(el('span', 'muted', '-')); }\n"
    "    (items || []).forEach(function (item) { td.appendChild(render(item)); });\n"
    "    return td;\n"
    "  }\n"
    "\n"
    "  function nodeRow(node, hubTime) {\n"
    "    var tr = el('tr');\n"
    "    var name = el('td');\n"
    "    name.appendChild(el('div', null, node.name || node.machineId));\n"
    "    name.appendChild(el('div', 'muted', node.address));\n"
    "    name.title = node.machineId;\n"
    "    tr.appendChild(name);\n"
    "    tr.appendChild(el('td', null, [node.os, node.arch].filter(Boolean).join(' / ')));\n"
    "    tr.appendChild(stateCell(node, hubTime));\n"
    "    var last = el('td', null, node.status === 'online' ? 'now' : when(node.lastSeen));\n"
    "    if (node.lastSeen) { last.title = 'first seen ' + when(node.firstSeen); }\n"
    "    tr.appendChild(last);\n"
    "    tr.appendChild(listCell(node.gpus, function (g) {\n"
    "      return el('div', null, g.name + ' ' + gib(g.vramTotalMiB) + ' (' + gib(g.vramFreeMiB) + ' free, ' + g.utilizationPercent + '%)');\n"
    "    }));\n"
    "    tr.appendChild(listCell(node.capabilities, function (c) { return el('span', 'chip', c); }));\n"
    "    tr.appendChild(listCell(node.hosts, function (h) { return el('div', null, h.name + ': ' + h.state + (h.detail ? ' (' + h.detail + ')' : '')); }));\n"
    "    tr.appendChild(listCell(node.workloads, function (w) {\n"
    "      var text = w.kind + ' ' + (w.id || '') + ' ' + w.state;\n"
    "      if (w.model) { text += ', ' + w.model; }\n"
    "      if (w.endpoint) { text += ' at ' + w.endpoint; }\n"
    "      return el('div', null, text);\n"
    "    }));\n"
    "    tr.appendChild(el('td', null, node.agentVersion));\n"
    "    return tr;\n"
    "  }\n"
    "\n"
    "  function stopPolling() {\n"
    "    if (timer !== null) { clearTimeout(timer); timer = null; }\n"
    "  }\n"
    "\n"
    "  function signedOut() {\n"
    "    stopPolling();\n"
    "    session = null;\n"
    "    $('who').hidden = true;\n"
    "    show('login-view');\n"
    "  }\n"
    "\n"
    "  function poll() {\n"
    "    stopPolling();\n"
    "    if (document.hidden) { timer = setTimeout(poll, POLL_MS); return; }\n"
    "    api('GET', '/api/v1/nodes').then(function (answer) {\n"
    "      if (answer.status === 401) { signedOut(); return; }\n"
    "      if (answer.status !== 200 || !answer.body) {\n"
    "        $('status-line').textContent = 'The hub did not answer (' + answer.status + '); trying again.';\n"
    "      } else {\n"
    "        var data = answer.body;\n"
    "        $('hub-name').textContent = 'Lab hub ' + data.hub;\n"
    "        var body = $('nodes').tBodies[0];\n"
    "        while (body.firstChild) { body.removeChild(body.firstChild); }\n"
    "        var online = 0;\n"
    "        data.nodes.forEach(function (n) { if (n.status === 'online') { online++; } body.appendChild(nodeRow(n, data.hubTime)); });\n"
    "        $('status-line').textContent = data.nodes.length + ' nodes, ' + online + ' online. Updated ' + new Date().toLocaleTimeString() + '.';\n"
    "      }\n"
    "      timer = setTimeout(poll, POLL_MS);\n"
    "    }, function () {\n"
    "      $('status-line').textContent = 'The hub cannot be reached; trying again.';\n"
    "      timer = setTimeout(poll, POLL_MS);\n"
    "    });\n"
    "  }\n"
    "\n"
    "  function signedIn(s) {\n"
    "    session = s;\n"
    "    $('who').hidden = false;\n"
    "    $('who-name').textContent = s.viaToken ? 'token (admin)' : s.user + ' (' + s.role + ')';\n"
    "    $('show-users').hidden = (s.role !== 'admin');\n"
    "    $('hub-name').textContent = 'Lab hub ' + s.hub;\n"
    "    show('nodes-view');\n"
    "    poll();\n"
    "  }\n"
    "\n"
    "  function loadUsers() {\n"
    "    api('GET', '/api/v1/users').then(function (answer) {\n"
    "      if (answer.status === 401) { signedOut(); return; }\n"
    "      var body = $('users').tBodies[0];\n"
    "      while (body.firstChild) { body.removeChild(body.firstChild); }\n"
    "      if (answer.status !== 200 || !answer.body) { $('users-message').textContent = errorOf(answer, 'The users cannot be read.'); return; }\n"
    "      answer.body.users.forEach(function (u) {\n"
    "        var tr = el('tr');\n"
    "        tr.appendChild(el('td', null, u.name));\n"
    "        tr.appendChild(el('td', null, u.role));\n"
    "        tr.appendChild(el('td', u.disabled ? 'flag' : null, u.disabled ? 'disabled' : 'enabled'));\n"
    "        tr.appendChild(el('td', null, when(u.created)));\n"
    "        var actions = el('td');\n"
    "        var button = el('button', null, u.disabled ? 'Enable' : 'Disable');\n"
    "        button.type = 'button';\n"
    "        button.addEventListener('click', function () {\n"
    "          api('POST', u.disabled ? '/api/v1/users/enable' : '/api/v1/users/disable', { name: u.name }).then(function (a) {\n"
    "            $('users-message').textContent = a.status === 200 ? '' : errorOf(a, 'That did not work.');\n"
    "            loadUsers();\n"
    "          });\n"
    "        });\n"
    "        actions.appendChild(button);\n"
    "        tr.appendChild(actions);\n"
    "        body.appendChild(tr);\n"
    "      });\n"
    "    });\n"
    "  }\n"
    "\n"
    "  function start() {\n"
    "    $('plain-warning').hidden = (location.protocol !== 'http:');\n"
    "\n"
    "    $('login-form').addEventListener('submit', function (event) {\n"
    "      event.preventDefault();\n"
    "      var form = event.target;\n"
    "      var fields = new FormData(form);\n"
    "      $('login-error').textContent = '';\n"
    "      api('POST', '/api/v1/login', fields).then(function (answer) {\n"
    "        form.elements.password.value = '';\n"
    "        form.elements.token.value = '';\n"
    "        if (answer.status === 200 && answer.body) { signedIn(answer.body); return; }\n"
    "        $('login-error').textContent = errorOf(answer, 'The login did not work (' + answer.status + ').');\n"
    "      }, function () { $('login-error').textContent = 'The hub cannot be reached.'; });\n"
    "    });\n"
    "\n"
    "    $('logout').addEventListener('click', function () {\n"
    "      api('POST', '/api/v1/logout').then(signedOut, signedOut);\n"
    "    });\n"
    "\n"
    "    $('show-nodes').addEventListener('click', function () { show('nodes-view'); poll(); });\n"
    "\n"
    "    $('show-users').addEventListener('click', function () { stopPolling(); show('users-view'); loadUsers(); });\n"
    "\n"
    "    $('add-user-form').addEventListener('submit', function (event) {\n"
    "      event.preventDefault();\n"
    "      var form = event.target;\n"
    "      api('POST', '/api/v1/users/add', new FormData(form)).then(function (answer) {\n"
    "        form.elements.password.value = '';\n"
    "        $('users-message').textContent = answer.status === 200 ? '' : errorOf(answer, 'The user was not added (' + answer.status + ').');\n"
    "        if (answer.status === 200) { form.elements.name.value = ''; }\n"
    "        loadUsers();\n"
    "      });\n"
    "    });\n"
    "\n"
    "    document.addEventListener('visibilitychange', function () {\n"
    "      if (!document.hidden && session && !$('nodes-view').hidden) { poll(); }\n"
    "    });\n"
    "\n"
    "    api('GET', '/api/v1/session').then(function (answer) {\n"
    "      if (answer.status === 200 && answer.body) { signedIn(answer.body); } else { signedOut(); }\n"
    "    }, signedOut);\n"
    "  }\n"
    "\n"
    "  document.addEventListener('DOMContentLoaded', start);\n"
    "}());\n";

/** app.css */
static const char AppCss[] =
    ":root { color-scheme: light dark; --fg: #1d2330; --bg: #f6f7f9; --card: #ffffff; --line: #d9dde4; --muted: #5c6576; --accent: #2457c5; --ok: #1d7a3e; --warn: #9a5b00; --bad: #b3261e; }\n"
    "@media (prefers-color-scheme: dark) { :root { --fg: #e4e7ec; --bg: #14171c; --card: #1c2027; --line: #313743; --muted: #9aa3b2; --accent: #7aa2ff; --ok: #5cc47f; --warn: #e0a03a; --bad: #ff7a70; } }\n"
    "[hidden] { display: none !important; }\n"
    "* { box-sizing: border-box; }\n"
    "body { margin: 0; font: 14px/1.45 system-ui, -apple-system, \"Segoe UI\", Roboto, sans-serif; color: var(--fg); background: var(--bg); }\n"
    "header { display: flex; align-items: center; justify-content: space-between; gap: 1em; padding: 0.6em 1.2em; background: var(--card); border-bottom: 1px solid var(--line); }\n"
    "h1 { font-size: 1.2em; margin: 0; }\n"
    "h2 { font-size: 1.1em; }\n"
    "nav { display: flex; align-items: center; gap: 0.5em; }\n"
    "main { padding: 1em 1.2em; }\n"
    "button { font: inherit; padding: 0.3em 0.8em; border: 1px solid var(--line); border-radius: 4px; background: var(--card); color: var(--fg); cursor: pointer; }\n"
    "button:hover { border-color: var(--accent); }\n"
    "input, select { font: inherit; padding: 0.3em; border: 1px solid var(--line); border-radius: 4px; background: var(--bg); color: var(--fg); }\n"
    "form { max-width: 26em; display: flex; flex-direction: column; gap: 0.6em; }\n"
    "label { display: flex; flex-direction: column; gap: 0.2em; }\n"
    "label.inline { flex-direction: row; align-items: center; gap: 0.4em; }\n"
    ".warning { margin: 0; padding: 0.6em 1.2em; background: #fff3cd; color: #5c4400; border-bottom: 1px solid #e0c46c; }\n"
    ".error { color: var(--bad); min-height: 1.2em; margin: 0; }\n"
    ".muted { color: var(--muted); }\n"
    ".scroll { overflow-x: auto; }\n"
    "table { border-collapse: collapse; width: 100%; background: var(--card); }\n"
    "th, td { text-align: left; vertical-align: top; padding: 0.4em 0.6em; border-bottom: 1px solid var(--line); }\n"
    "th { font-weight: 600; color: var(--muted); white-space: nowrap; }\n"
    "td div + div { margin-top: 0.15em; }\n"
    ".chip { display: inline-block; margin: 0 0.25em 0.2em 0; padding: 0 0.45em; border: 1px solid var(--line); border-radius: 0.8em; font-size: 0.9em; }\n"
    ".state-online { color: var(--ok); font-weight: 600; }\n"
    ".state-busy { color: var(--warn); font-weight: 600; }\n"
    ".state-offline { color: var(--muted); }\n"
    ".state-unknown { color: var(--muted); font-style: italic; }\n"
    ".flag { color: var(--warn); }\n"
    ".alarm { color: var(--bad); }\n"
    "#users { max-width: 46em; margin-bottom: 1.5em; }\n";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
CWebAssets::Find( const GUCEF::CORE::CString& name, GUCEF::UInt32& length, const char*& mediaType )
{GUCEF_TRACE;

    struct SAsset { const char* name; const char* content; GUCEF::UInt32 length; const char* mediaType; };
    static const SAsset assets[] =
    {
        { "index.html", IndexHtml, (GUCEF::UInt32) ( sizeof( IndexHtml ) - 1 ), "text/html; charset=utf-8" },
        { "app.js", AppJs, (GUCEF::UInt32) ( sizeof( AppJs ) - 1 ), "text/javascript; charset=utf-8" },
        { "app.css", AppCss, (GUCEF::UInt32) ( sizeof( AppCss ) - 1 ), "text/css; charset=utf-8" }
    };
    for ( size_t i=0; i<sizeof( assets ) / sizeof( assets[ 0 ] ); ++i )
    {
        if ( name == assets[ i ].name )
        {
            length = assets[ i ].length;
            mediaType = assets[ i ].mediaType;
            return assets[ i ].content;
        }
    }
    length = 0;
    mediaType = GUCEF_NULL;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
