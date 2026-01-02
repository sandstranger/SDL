/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#ifdef SDL_VIDEO_DRIVER_ANDROID

#include <android/log.h>

#include "SDL_hints.h"
#include "SDL_events.h"
#include "SDL_androidtouch.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_touch_c.h"
#include "../../core/android/SDL_android.h"
#include "uthash.h"

#define ACTION_DOWN 0
#define ACTION_UP   1
#define ACTION_MOVE 2
/* #define ACTION_CANCEL 3 */
/* #define ACTION_OUTSIDE 4 */
#define ACTION_POINTER_DOWN 5
#define ACTION_POINTER_UP   6

typedef struct {
    int64_t key;
    bool value;
    UT_hash_handle hh;
} LongMap;

static LongMap *map = NULL;

static void put_long(int64_t key, bool value) {
    LongMap *e;
    HASH_FIND(hh, map, &key, sizeof(int64_t), e);
    if (!e) {
        e = malloc(sizeof(*e));
        e->key = key;
        HASH_ADD(hh, map, key, sizeof(int64_t), e);
    }
    e->value = value;
}

static bool get_long(int64_t key, bool *out_value) {
    LongMap *e;
    HASH_FIND(hh, map, &key, sizeof(int64_t), e);
    if (!e) {
        return false;
    }
    *out_value = e->value;
    return true;
}

static void remove_long(int64_t key) {
    LongMap *e;
    HASH_FIND(hh, map, &key, sizeof(int64_t), e);
    if (e) {
        HASH_DEL(map, e);
        free(e);
    }
}

static bool contains_long(int64_t key) {
    LongMap *e;
    HASH_FIND(hh, map, &key, sizeof(int64_t), e);
    return e != NULL;
}

void Android_InitTouch(void)
{
    /* Add all touch devices */
    Android_JNI_InitTouch();
}

void Android_QuitTouch(void)
{
}

void Android_OnTouch(SDL_Window *window, int touch_device_id_in, int pointer_finger_id_in, int action,
                     float x, float y, float p, bool invokePressEvents)
{
    SDL_TouchID touchDeviceId = 0;
    SDL_FingerID fingerId = 0;

    if (!window) {
        return;
    }

    touchDeviceId = (SDL_TouchID)touch_device_id_in;
    if (SDL_AddTouch(touchDeviceId, SDL_TOUCH_DEVICE_DIRECT, "") < 0) {
        SDL_Log("error: can't add touch %s, %d", __FILE__, __LINE__);
    }

    fingerId = (SDL_FingerID)pointer_finger_id_in;
    switch (action) {
    case ACTION_DOWN:
    case ACTION_POINTER_DOWN:
        if (contains_long(fingerId)){
            bool savedInvokePressEventsState;
            get_long(fingerId, &savedInvokePressEventsState);
            SDL_SendTouch(touchDeviceId, fingerId, window,
                          SDL_FALSE, 0, 0, 0, savedInvokePressEventsState);
            remove_long(fingerId);
        }

        if (!contains_long(fingerId)){
            put_long(fingerId, invokePressEvents);
        }

        SDL_SendTouch(touchDeviceId, fingerId, window, SDL_TRUE, x, y, p, invokePressEvents);
        break;

    case ACTION_MOVE:
        SDL_SendTouchMotion(touchDeviceId, fingerId, window, x, y, p);
        break;

    case ACTION_UP:
    case ACTION_POINTER_UP:
        if (contains_long(fingerId)){
            remove_long(fingerId);
        }
        SDL_SendTouch(touchDeviceId, fingerId, window, SDL_FALSE, x, y, p, invokePressEvents);
        break;

    default:
        break;
    }
}

#endif /* SDL_VIDEO_DRIVER_ANDROID */

/* vi: set ts=4 sw=4 expandtab: */
