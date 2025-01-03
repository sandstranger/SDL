//
// Created by maks on 18.10.2023.
//
#include <android/native_window.h>
#include <stdbool.h>
#include "osmesa_loader.h"
#include "../SDL_sysvideo.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

typedef struct {
    OSMesaContext context;
    int32_t last_stride;
    int id;
} xxx3_osm_render_window_t;

bool osm_init(void);
int GetSwapInterval (_THIS);
int SetSwapInterval (_THIS,int swapInterval);
SDL_GLContext GetCurrentContext (void);
SDL_GLContext CreateGLContext (_THIS,SDL_Window *window);
int SwapWindow(_THIS, SDL_Window *window);
int MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context);
void DestroyContext(_THIS,SDL_GLContext context);
