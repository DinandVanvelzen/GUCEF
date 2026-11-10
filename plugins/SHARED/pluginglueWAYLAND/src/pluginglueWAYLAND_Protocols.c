/* Generated from the Wayland protocol XML files with a scanner that makes the output of wayland-scanner (public-code and
 * client-header) with request wrappers that call through a function table, so no libwayland-client is linked.
 * The copyright notices of the protocol files follow. Do not edit by hand. */

/* ---- wayland (wayland.xml) ----

    Copyright © 2008-2011 Kristian Høgsberg
    Copyright © 2010-2011 Intel Corporation
    Copyright © 2012-2013 Collabora, Ltd.

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice (including the
    next paragraph) shall be included in all copies or substantial
    portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
    BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
  
 */

/* ---- xdg_shell (xdg-shell.xml) ----

    Copyright © 2008-2013 Kristian Høgsberg
    Copyright © 2013      Rafael Antognolli
    Copyright © 2013      Jasper St. Pierre
    Copyright © 2010-2013 Intel Corporation
    Copyright © 2015-2017 Samsung Electronics Co., Ltd
    Copyright © 2015-2017 Red Hat Inc.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- viewporter (viewporter.xml) ----

    Copyright © 2013-2016 Collabora, Ltd.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- fractional_scale_v1 (fractional-scale-v1.xml) ----

    Copyright © 2022 Kenny Levinsen

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- xdg_decoration_unstable_v1 (xdg-decoration-unstable-v1.xml) ----

    Copyright © 2018 Simon Ser

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- cursor_shape_v1 (cursor-shape-v1.xml) ----

    Copyright 2018 The Chromium Authors
    Copyright 2023 Simon Ser

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- xdg_output_unstable_v1 (xdg-output-unstable-v1.xml) ----

    Copyright © 2017 Red Hat Inc.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

/* ---- xdg_activation_v1 (xdg-activation-v1.xml) ----

    Copyright © 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright © 2020 Carlos Garnacho <carlosg@gnome.org>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
  
 */

#include "pluginglueWAYLAND_Protocols.h"

#include <stddef.h>

/* ---- wayland ---- */

extern const struct wl_interface gucef_wl_registry_interface;
extern const struct wl_interface gucef_wl_callback_interface;
extern const struct wl_interface gucef_wl_compositor_interface;
extern const struct wl_interface gucef_wl_shm_pool_interface;
extern const struct wl_interface gucef_wl_shm_interface;
extern const struct wl_interface gucef_wl_buffer_interface;
extern const struct wl_interface gucef_wl_surface_interface;
extern const struct wl_interface gucef_wl_seat_interface;
extern const struct wl_interface gucef_wl_pointer_interface;
extern const struct wl_interface gucef_wl_keyboard_interface;
extern const struct wl_interface gucef_wl_touch_interface;
extern const struct wl_interface gucef_wl_output_interface;
extern const struct wl_interface gucef_wl_region_interface;

static const struct wl_interface* gucef_wayland_types[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_wl_surface_interface,
    &gucef_wl_region_interface,
    &gucef_wl_buffer_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_wl_shm_pool_interface,
    NULL,
    NULL,
    NULL,
    &gucef_wl_buffer_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_wl_callback_interface,
    &gucef_wl_region_interface,
    &gucef_wl_region_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_wl_callback_interface,
    &gucef_wl_output_interface,
    &gucef_wl_output_interface,
    NULL,
    NULL,
    &gucef_wl_pointer_interface,
    &gucef_wl_keyboard_interface,
    &gucef_wl_touch_interface,
    NULL,
    NULL,
    NULL,
    &gucef_wl_surface_interface,
    NULL,
    NULL,
    NULL,
    &gucef_wl_surface_interface,
    NULL,
    NULL,
    NULL,
    &gucef_wl_surface_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_wl_surface_interface,
    NULL,
    NULL,
    &gucef_wl_surface_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_wl_surface_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const struct wl_message gucef_wl_registry_requests[] = {
    { "bind", "usun", gucef_wayland_types + 0 },
};

static const struct wl_message gucef_wl_registry_events[] = {
    { "global", "usu", gucef_wayland_types + 4 },
    { "global_remove", "u", gucef_wayland_types + 7 },
};

const struct wl_interface gucef_wl_registry_interface = {
    "wl_registry", 1,
    1, gucef_wl_registry_requests,
    2, gucef_wl_registry_events,
};

static const struct wl_message gucef_wl_callback_events[] = {
    { "done", "u", gucef_wayland_types + 8 },
};

const struct wl_interface gucef_wl_callback_interface = {
    "wl_callback", 1,
    0, NULL,
    1, gucef_wl_callback_events,
};

static const struct wl_message gucef_wl_compositor_requests[] = {
    { "create_surface", "n", gucef_wayland_types + 9 },
    { "create_region", "n", gucef_wayland_types + 10 },
    { "release", "7", gucef_wayland_types + 11 },
};

const struct wl_interface gucef_wl_compositor_interface = {
    "wl_compositor", 7,
    3, gucef_wl_compositor_requests,
    0, NULL,
};

static const struct wl_message gucef_wl_shm_pool_requests[] = {
    { "create_buffer", "niiiiu", gucef_wayland_types + 11 },
    { "destroy", "", gucef_wayland_types + 17 },
    { "resize", "i", gucef_wayland_types + 17 },
};

const struct wl_interface gucef_wl_shm_pool_interface = {
    "wl_shm_pool", 3,
    3, gucef_wl_shm_pool_requests,
    0, NULL,
};

static const struct wl_message gucef_wl_shm_requests[] = {
    { "create_pool", "nhi", gucef_wayland_types + 18 },
    { "release", "2", gucef_wayland_types + 21 },
};

static const struct wl_message gucef_wl_shm_events[] = {
    { "format", "u", gucef_wayland_types + 21 },
};

const struct wl_interface gucef_wl_shm_interface = {
    "wl_shm", 3,
    2, gucef_wl_shm_requests,
    1, gucef_wl_shm_events,
};

static const struct wl_message gucef_wl_buffer_requests[] = {
    { "destroy", "", gucef_wayland_types + 22 },
};

static const struct wl_message gucef_wl_buffer_events[] = {
    { "release", "", gucef_wayland_types + 22 },
};

const struct wl_interface gucef_wl_buffer_interface = {
    "wl_buffer", 1,
    1, gucef_wl_buffer_requests,
    1, gucef_wl_buffer_events,
};

static const struct wl_message gucef_wl_surface_requests[] = {
    { "destroy", "", gucef_wayland_types + 22 },
    { "attach", "?oii", gucef_wayland_types + 22 },
    { "damage", "iiii", gucef_wayland_types + 25 },
    { "frame", "n", gucef_wayland_types + 29 },
    { "set_opaque_region", "?o", gucef_wayland_types + 30 },
    { "set_input_region", "?o", gucef_wayland_types + 31 },
    { "commit", "", gucef_wayland_types + 32 },
    { "set_buffer_transform", "2i", gucef_wayland_types + 32 },
    { "set_buffer_scale", "3i", gucef_wayland_types + 33 },
    { "damage_buffer", "4iiii", gucef_wayland_types + 34 },
    { "offset", "5ii", gucef_wayland_types + 38 },
    { "get_release", "7n", gucef_wayland_types + 40 },
};

static const struct wl_message gucef_wl_surface_events[] = {
    { "enter", "o", gucef_wayland_types + 41 },
    { "leave", "o", gucef_wayland_types + 42 },
    { "preferred_buffer_scale", "6i", gucef_wayland_types + 43 },
    { "preferred_buffer_transform", "6u", gucef_wayland_types + 44 },
};

const struct wl_interface gucef_wl_surface_interface = {
    "wl_surface", 7,
    12, gucef_wl_surface_requests,
    4, gucef_wl_surface_events,
};

static const struct wl_message gucef_wl_seat_requests[] = {
    { "get_pointer", "n", gucef_wayland_types + 45 },
    { "get_keyboard", "n", gucef_wayland_types + 46 },
    { "get_touch", "n", gucef_wayland_types + 47 },
    { "release", "5", gucef_wayland_types + 48 },
};

static const struct wl_message gucef_wl_seat_events[] = {
    { "capabilities", "u", gucef_wayland_types + 48 },
    { "name", "2s", gucef_wayland_types + 49 },
};

const struct wl_interface gucef_wl_seat_interface = {
    "wl_seat", 11,
    4, gucef_wl_seat_requests,
    2, gucef_wl_seat_events,
};

static const struct wl_message gucef_wl_pointer_requests[] = {
    { "set_cursor", "u?oii", gucef_wayland_types + 50 },
    { "release", "3", gucef_wayland_types + 54 },
};

static const struct wl_message gucef_wl_pointer_events[] = {
    { "enter", "uoff", gucef_wayland_types + 54 },
    { "leave", "uo", gucef_wayland_types + 58 },
    { "motion", "uff", gucef_wayland_types + 60 },
    { "button", "uuuu", gucef_wayland_types + 63 },
    { "axis", "uuf", gucef_wayland_types + 67 },
    { "frame", "5", gucef_wayland_types + 70 },
    { "axis_source", "5u", gucef_wayland_types + 70 },
    { "axis_stop", "5uu", gucef_wayland_types + 71 },
    { "axis_discrete", "5ui", gucef_wayland_types + 73 },
    { "axis_value120", "8ui", gucef_wayland_types + 75 },
    { "axis_relative_direction", "9uu", gucef_wayland_types + 77 },
    { "warp", "11ff", gucef_wayland_types + 79 },
};

const struct wl_interface gucef_wl_pointer_interface = {
    "wl_pointer", 11,
    2, gucef_wl_pointer_requests,
    12, gucef_wl_pointer_events,
};

static const struct wl_message gucef_wl_keyboard_requests[] = {
    { "release", "3", gucef_wayland_types + 81 },
};

static const struct wl_message gucef_wl_keyboard_events[] = {
    { "keymap", "uhu", gucef_wayland_types + 81 },
    { "enter", "uoa", gucef_wayland_types + 84 },
    { "leave", "uo", gucef_wayland_types + 87 },
    { "key", "uuuu", gucef_wayland_types + 89 },
    { "modifiers", "uuuuu", gucef_wayland_types + 93 },
    { "repeat_info", "4ii", gucef_wayland_types + 98 },
};

const struct wl_interface gucef_wl_keyboard_interface = {
    "wl_keyboard", 11,
    1, gucef_wl_keyboard_requests,
    6, gucef_wl_keyboard_events,
};

static const struct wl_message gucef_wl_touch_requests[] = {
    { "release", "3", gucef_wayland_types + 100 },
};

static const struct wl_message gucef_wl_touch_events[] = {
    { "down", "uuoiff", gucef_wayland_types + 100 },
    { "up", "uui", gucef_wayland_types + 106 },
    { "motion", "uiff", gucef_wayland_types + 109 },
    { "frame", "", gucef_wayland_types + 113 },
    { "cancel", "", gucef_wayland_types + 113 },
    { "shape", "6iff", gucef_wayland_types + 113 },
    { "orientation", "6if", gucef_wayland_types + 116 },
};

const struct wl_interface gucef_wl_touch_interface = {
    "wl_touch", 11,
    1, gucef_wl_touch_requests,
    7, gucef_wl_touch_events,
};

static const struct wl_message gucef_wl_output_requests[] = {
    { "release", "3", gucef_wayland_types + 118 },
};

static const struct wl_message gucef_wl_output_events[] = {
    { "geometry", "iiiiissi", gucef_wayland_types + 118 },
    { "mode", "uiii", gucef_wayland_types + 126 },
    { "done", "2", gucef_wayland_types + 130 },
    { "scale", "2i", gucef_wayland_types + 130 },
    { "name", "4s", gucef_wayland_types + 131 },
    { "description", "4s", gucef_wayland_types + 132 },
};

const struct wl_interface gucef_wl_output_interface = {
    "wl_output", 4,
    1, gucef_wl_output_requests,
    6, gucef_wl_output_events,
};

static const struct wl_message gucef_wl_region_requests[] = {
    { "destroy", "", gucef_wayland_types + 133 },
    { "add", "iiii", gucef_wayland_types + 133 },
    { "subtract", "iiii", gucef_wayland_types + 137 },
};

const struct wl_interface gucef_wl_region_interface = {
    "wl_region", 7,
    3, gucef_wl_region_requests,
    0, NULL,
};

/* ---- xdg_shell ---- */

extern const struct wl_interface gucef_xdg_wm_base_interface;
extern const struct wl_interface gucef_xdg_positioner_interface;
extern const struct wl_interface gucef_xdg_surface_interface;
extern const struct wl_interface gucef_xdg_toplevel_interface;
extern const struct wl_interface gucef_xdg_popup_interface;

static const struct wl_interface* gucef_xdg_shell_types[] = {
    &gucef_xdg_positioner_interface,
    &gucef_xdg_surface_interface,
    &gucef_wl_surface_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_xdg_toplevel_interface,
    &gucef_xdg_popup_interface,
    &gucef_xdg_surface_interface,
    &gucef_xdg_positioner_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_xdg_toplevel_interface,
    NULL,
    NULL,
    &gucef_wl_seat_interface,
    NULL,
    NULL,
    NULL,
    &gucef_wl_seat_interface,
    NULL,
    &gucef_wl_seat_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_wl_output_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &gucef_wl_seat_interface,
    NULL,
    &gucef_xdg_positioner_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const struct wl_message gucef_xdg_wm_base_requests[] = {
    { "destroy", "", gucef_xdg_shell_types + 0 },
    { "create_positioner", "n", gucef_xdg_shell_types + 0 },
    { "get_xdg_surface", "no", gucef_xdg_shell_types + 1 },
    { "pong", "u", gucef_xdg_shell_types + 3 },
};

static const struct wl_message gucef_xdg_wm_base_events[] = {
    { "ping", "u", gucef_xdg_shell_types + 4 },
};

const struct wl_interface gucef_xdg_wm_base_interface = {
    "xdg_wm_base", 7,
    4, gucef_xdg_wm_base_requests,
    1, gucef_xdg_wm_base_events,
};

static const struct wl_message gucef_xdg_positioner_requests[] = {
    { "destroy", "", gucef_xdg_shell_types + 5 },
    { "set_size", "ii", gucef_xdg_shell_types + 5 },
    { "set_anchor_rect", "iiii", gucef_xdg_shell_types + 7 },
    { "set_anchor", "u", gucef_xdg_shell_types + 11 },
    { "set_gravity", "u", gucef_xdg_shell_types + 12 },
    { "set_constraint_adjustment", "u", gucef_xdg_shell_types + 13 },
    { "set_offset", "ii", gucef_xdg_shell_types + 14 },
    { "set_reactive", "3", gucef_xdg_shell_types + 16 },
    { "set_parent_size", "3ii", gucef_xdg_shell_types + 16 },
    { "set_parent_configure", "3u", gucef_xdg_shell_types + 18 },
};

const struct wl_interface gucef_xdg_positioner_interface = {
    "xdg_positioner", 7,
    10, gucef_xdg_positioner_requests,
    0, NULL,
};

static const struct wl_message gucef_xdg_surface_requests[] = {
    { "destroy", "", gucef_xdg_shell_types + 19 },
    { "get_toplevel", "n", gucef_xdg_shell_types + 19 },
    { "get_popup", "n?oo", gucef_xdg_shell_types + 20 },
    { "set_window_geometry", "iiii", gucef_xdg_shell_types + 23 },
    { "ack_configure", "u", gucef_xdg_shell_types + 27 },
};

static const struct wl_message gucef_xdg_surface_events[] = {
    { "configure", "u", gucef_xdg_shell_types + 28 },
};

const struct wl_interface gucef_xdg_surface_interface = {
    "xdg_surface", 7,
    5, gucef_xdg_surface_requests,
    1, gucef_xdg_surface_events,
};

static const struct wl_message gucef_xdg_toplevel_requests[] = {
    { "destroy", "", gucef_xdg_shell_types + 29 },
    { "set_parent", "?o", gucef_xdg_shell_types + 29 },
    { "set_title", "s", gucef_xdg_shell_types + 30 },
    { "set_app_id", "s", gucef_xdg_shell_types + 31 },
    { "show_window_menu", "ouii", gucef_xdg_shell_types + 32 },
    { "move", "ou", gucef_xdg_shell_types + 36 },
    { "resize", "ouu", gucef_xdg_shell_types + 38 },
    { "set_max_size", "ii", gucef_xdg_shell_types + 41 },
    { "set_min_size", "ii", gucef_xdg_shell_types + 43 },
    { "set_maximized", "", gucef_xdg_shell_types + 45 },
    { "unset_maximized", "", gucef_xdg_shell_types + 45 },
    { "set_fullscreen", "?o", gucef_xdg_shell_types + 45 },
    { "unset_fullscreen", "", gucef_xdg_shell_types + 46 },
    { "set_minimized", "", gucef_xdg_shell_types + 46 },
};

static const struct wl_message gucef_xdg_toplevel_events[] = {
    { "configure", "iia", gucef_xdg_shell_types + 46 },
    { "close", "", gucef_xdg_shell_types + 49 },
    { "configure_bounds", "4ii", gucef_xdg_shell_types + 49 },
    { "wm_capabilities", "5a", gucef_xdg_shell_types + 51 },
};

const struct wl_interface gucef_xdg_toplevel_interface = {
    "xdg_toplevel", 7,
    14, gucef_xdg_toplevel_requests,
    4, gucef_xdg_toplevel_events,
};

static const struct wl_message gucef_xdg_popup_requests[] = {
    { "destroy", "", gucef_xdg_shell_types + 52 },
    { "grab", "ou", gucef_xdg_shell_types + 52 },
    { "reposition", "3ou", gucef_xdg_shell_types + 54 },
};

static const struct wl_message gucef_xdg_popup_events[] = {
    { "configure", "iiii", gucef_xdg_shell_types + 56 },
    { "popup_done", "", gucef_xdg_shell_types + 60 },
    { "repositioned", "3u", gucef_xdg_shell_types + 60 },
};

const struct wl_interface gucef_xdg_popup_interface = {
    "xdg_popup", 7,
    3, gucef_xdg_popup_requests,
    3, gucef_xdg_popup_events,
};

/* ---- viewporter ---- */

extern const struct wl_interface gucef_wp_viewporter_interface;
extern const struct wl_interface gucef_wp_viewport_interface;

static const struct wl_interface* gucef_viewporter_types[] = {
    &gucef_wp_viewport_interface,
    &gucef_wl_surface_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const struct wl_message gucef_wp_viewporter_requests[] = {
    { "destroy", "", gucef_viewporter_types + 0 },
    { "get_viewport", "no", gucef_viewporter_types + 0 },
};

const struct wl_interface gucef_wp_viewporter_interface = {
    "wp_viewporter", 1,
    2, gucef_wp_viewporter_requests,
    0, NULL,
};

static const struct wl_message gucef_wp_viewport_requests[] = {
    { "destroy", "", gucef_viewporter_types + 2 },
    { "set_source", "ffff", gucef_viewporter_types + 2 },
    { "set_destination", "ii", gucef_viewporter_types + 6 },
};

const struct wl_interface gucef_wp_viewport_interface = {
    "wp_viewport", 1,
    3, gucef_wp_viewport_requests,
    0, NULL,
};

/* ---- fractional_scale_v1 ---- */

extern const struct wl_interface gucef_wp_fractional_scale_manager_v1_interface;
extern const struct wl_interface gucef_wp_fractional_scale_v1_interface;

static const struct wl_interface* gucef_fractional_scale_v1_types[] = {
    &gucef_wp_fractional_scale_v1_interface,
    &gucef_wl_surface_interface,
    NULL,
};

static const struct wl_message gucef_wp_fractional_scale_manager_v1_requests[] = {
    { "destroy", "", gucef_fractional_scale_v1_types + 0 },
    { "get_fractional_scale", "no", gucef_fractional_scale_v1_types + 0 },
};

const struct wl_interface gucef_wp_fractional_scale_manager_v1_interface = {
    "wp_fractional_scale_manager_v1", 1,
    2, gucef_wp_fractional_scale_manager_v1_requests,
    0, NULL,
};

static const struct wl_message gucef_wp_fractional_scale_v1_requests[] = {
    { "destroy", "", gucef_fractional_scale_v1_types + 2 },
};

static const struct wl_message gucef_wp_fractional_scale_v1_events[] = {
    { "preferred_scale", "u", gucef_fractional_scale_v1_types + 2 },
};

const struct wl_interface gucef_wp_fractional_scale_v1_interface = {
    "wp_fractional_scale_v1", 1,
    1, gucef_wp_fractional_scale_v1_requests,
    1, gucef_wp_fractional_scale_v1_events,
};

/* ---- xdg_decoration_unstable_v1 ---- */

extern const struct wl_interface gucef_zxdg_decoration_manager_v1_interface;
extern const struct wl_interface gucef_zxdg_toplevel_decoration_v1_interface;

static const struct wl_interface* gucef_xdg_decoration_unstable_v1_types[] = {
    &gucef_zxdg_toplevel_decoration_v1_interface,
    &gucef_xdg_toplevel_interface,
    NULL,
    NULL,
};

static const struct wl_message gucef_zxdg_decoration_manager_v1_requests[] = {
    { "destroy", "", gucef_xdg_decoration_unstable_v1_types + 0 },
    { "get_toplevel_decoration", "no", gucef_xdg_decoration_unstable_v1_types + 0 },
};

const struct wl_interface gucef_zxdg_decoration_manager_v1_interface = {
    "zxdg_decoration_manager_v1", 2,
    2, gucef_zxdg_decoration_manager_v1_requests,
    0, NULL,
};

static const struct wl_message gucef_zxdg_toplevel_decoration_v1_requests[] = {
    { "destroy", "", gucef_xdg_decoration_unstable_v1_types + 2 },
    { "set_mode", "u", gucef_xdg_decoration_unstable_v1_types + 2 },
    { "unset_mode", "", gucef_xdg_decoration_unstable_v1_types + 3 },
};

static const struct wl_message gucef_zxdg_toplevel_decoration_v1_events[] = {
    { "configure", "u", gucef_xdg_decoration_unstable_v1_types + 3 },
};

const struct wl_interface gucef_zxdg_toplevel_decoration_v1_interface = {
    "zxdg_toplevel_decoration_v1", 2,
    3, gucef_zxdg_toplevel_decoration_v1_requests,
    1, gucef_zxdg_toplevel_decoration_v1_events,
};

/* ---- cursor_shape_v1 ---- */

extern const struct wl_interface gucef_wp_cursor_shape_manager_v1_interface;
extern const struct wl_interface gucef_wp_cursor_shape_device_v1_interface;

static const struct wl_interface* gucef_cursor_shape_v1_types[] = {
    &gucef_wp_cursor_shape_device_v1_interface,
    &gucef_wl_pointer_interface,
    &gucef_wp_cursor_shape_device_v1_interface,
    NULL,
    NULL,
    NULL,
};

static const struct wl_message gucef_wp_cursor_shape_manager_v1_requests[] = {
    { "destroy", "", gucef_cursor_shape_v1_types + 0 },
    { "get_pointer", "no", gucef_cursor_shape_v1_types + 0 },
    { "get_tablet_tool_v2", "no", gucef_cursor_shape_v1_types + 2 },
};

const struct wl_interface gucef_wp_cursor_shape_manager_v1_interface = {
    "wp_cursor_shape_manager_v1", 2,
    3, gucef_wp_cursor_shape_manager_v1_requests,
    0, NULL,
};

static const struct wl_message gucef_wp_cursor_shape_device_v1_requests[] = {
    { "destroy", "", gucef_cursor_shape_v1_types + 4 },
    { "set_shape", "uu", gucef_cursor_shape_v1_types + 4 },
};

const struct wl_interface gucef_wp_cursor_shape_device_v1_interface = {
    "wp_cursor_shape_device_v1", 2,
    2, gucef_wp_cursor_shape_device_v1_requests,
    0, NULL,
};

/* ---- xdg_output_unstable_v1 ---- */

extern const struct wl_interface gucef_zxdg_output_manager_v1_interface;
extern const struct wl_interface gucef_zxdg_output_v1_interface;

static const struct wl_interface* gucef_xdg_output_unstable_v1_types[] = {
    &gucef_zxdg_output_v1_interface,
    &gucef_wl_output_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const struct wl_message gucef_zxdg_output_manager_v1_requests[] = {
    { "destroy", "", gucef_xdg_output_unstable_v1_types + 0 },
    { "get_xdg_output", "no", gucef_xdg_output_unstable_v1_types + 0 },
};

const struct wl_interface gucef_zxdg_output_manager_v1_interface = {
    "zxdg_output_manager_v1", 3,
    2, gucef_zxdg_output_manager_v1_requests,
    0, NULL,
};

static const struct wl_message gucef_zxdg_output_v1_requests[] = {
    { "destroy", "", gucef_xdg_output_unstable_v1_types + 2 },
};

static const struct wl_message gucef_zxdg_output_v1_events[] = {
    { "logical_position", "ii", gucef_xdg_output_unstable_v1_types + 2 },
    { "logical_size", "ii", gucef_xdg_output_unstable_v1_types + 4 },
    { "done", "", gucef_xdg_output_unstable_v1_types + 6 },
    { "name", "2s", gucef_xdg_output_unstable_v1_types + 6 },
    { "description", "2s", gucef_xdg_output_unstable_v1_types + 7 },
};

const struct wl_interface gucef_zxdg_output_v1_interface = {
    "zxdg_output_v1", 3,
    1, gucef_zxdg_output_v1_requests,
    5, gucef_zxdg_output_v1_events,
};

/* ---- xdg_activation_v1 ---- */

extern const struct wl_interface gucef_xdg_activation_v1_interface;
extern const struct wl_interface gucef_xdg_activation_token_v1_interface;

static const struct wl_interface* gucef_xdg_activation_v1_types[] = {
    &gucef_xdg_activation_token_v1_interface,
    NULL,
    &gucef_wl_surface_interface,
    NULL,
    &gucef_wl_seat_interface,
    NULL,
    &gucef_wl_surface_interface,
    NULL,
};

static const struct wl_message gucef_xdg_activation_v1_requests[] = {
    { "destroy", "", gucef_xdg_activation_v1_types + 0 },
    { "get_activation_token", "n", gucef_xdg_activation_v1_types + 0 },
    { "activate", "so", gucef_xdg_activation_v1_types + 1 },
};

const struct wl_interface gucef_xdg_activation_v1_interface = {
    "xdg_activation_v1", 1,
    3, gucef_xdg_activation_v1_requests,
    0, NULL,
};

static const struct wl_message gucef_xdg_activation_token_v1_requests[] = {
    { "set_serial", "uo", gucef_xdg_activation_v1_types + 3 },
    { "set_app_id", "s", gucef_xdg_activation_v1_types + 5 },
    { "set_surface", "o", gucef_xdg_activation_v1_types + 6 },
    { "commit", "", gucef_xdg_activation_v1_types + 7 },
    { "destroy", "", gucef_xdg_activation_v1_types + 7 },
};

static const struct wl_message gucef_xdg_activation_token_v1_events[] = {
    { "done", "s", gucef_xdg_activation_v1_types + 7 },
};

const struct wl_interface gucef_xdg_activation_token_v1_interface = {
    "xdg_activation_token_v1", 1,
    5, gucef_xdg_activation_token_v1_requests,
    1, gucef_xdg_activation_token_v1_events,
};

