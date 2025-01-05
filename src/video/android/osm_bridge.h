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
