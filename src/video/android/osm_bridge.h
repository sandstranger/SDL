//
// Created by maks on 18.10.2023.
//
#include <android/native_window.h>
#include <stdbool.h>
#ifndef POJAVLAUNCHER_OSM_BRIDGE_H
#define POJAVLAUNCHER_OSM_BRIDGE_H
#include "osmesa_loader.h"
#include "../SDL_sysvideo.h"

typedef struct {
    char       state;
    struct ANativeWindow *nativeSurface;
    struct ANativeWindow *newNativeSurface;
} basic_render_window_t;

typedef struct {
    char       state;
    struct ANativeWindow *nativeSurface;
    struct ANativeWindow *newNativeSurface;
    ANativeWindow_Buffer buffer;
    int32_t last_stride;
    bool disable_rendering;
    OSMesaContext context;
} osm_render_window_t;

bool osm_init(void);
int GetSwapInterval (_THIS);
int SetSwapInterval (_THIS,int swapInterval);
SDL_GLContext GetCurrentContext ();
SDL_GLContext CreateGLContext (_THIS,SDL_Window *window);
void SetupWindow (SDL_Window *window);
int SwapWindow(_THIS, SDL_Window *window);
int MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context);
void DestroyContext();
#endif //POJAVLAUNCHER_OSM_BRIDGE_H
