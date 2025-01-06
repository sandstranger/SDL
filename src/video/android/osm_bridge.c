//
// Created by maks on 18.10.2023.
//
#include <malloc.h>
#include <string.h>
#include "SDL_video.h"
#include <android/log.h>
#include "osm_bridge.h"
#include "SDL_androidwindow.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "SDL_androidvideo.h"
#include "SDL_surface.h"
#include "SDL.h"
#include "jni.h"
#include <stdbool.h>
#include "osmesa_loader.h"
#include "../SDL_sysvideo.h"
#include "stb_image_write.h"

typedef struct {
    OSMesaContext context;
    int32_t last_stride;
} xxx3_osm_render_window_t;

static ANativeWindow *nativeSurface;
static ANativeWindow_Buffer buffer;
static xxx3_osm_render_window_t *xxx3_osm;
static volatile bool hasCleaned = false;
static bool hasSetNoRendererBuffer = false;
static char xxx3_no_render_buffer[4];
static const char* osm_LogTag = "[ XXX3 OSM Bridge ]";
static int32_t last_stride;
static volatile GLubyte* *abuffer;
static bool bufferWasCreated;
static SDL_Window *sdlWindow;
static SDL_Renderer* renderer;
static SDL_Texture* renderTexture;
extern void load_vulkan(void);
static void *correctedBuffer = NULL;
static SDL_Surface *surface;

void* xxx3OsmCreateContext() {
    xxx3_osm_render_window_t *renderWindow = malloc(sizeof(xxx3_osm_render_window_t));
    if (!renderWindow)
    {
        printf("%s Failed to allocate memory for xxx3_osm\n", osm_LogTag);
        return NULL;
    }

    memset(renderWindow, 0, sizeof(xxx3_osm_render_window_t));

    printf("%s generating context\n", osm_LogTag);

    OSMesaContext context = OSMesaCreateContext_p(OSMESA_RGBA, NULL);
    if (context == NULL) {
        printf("%s OSMesaContext is Null!!!\n", osm_LogTag);
        return NULL;
    }

    renderWindow->context = context;
    printf("%s context = %p\n", osm_LogTag, context);
    return renderWindow;
}
void UpdateSdlRender(GLubyte *buffer, int width, int height, int stride) {
    if (!correctedBuffer) {
        correctedBuffer = malloc(width * height * 4);
    }
    for (int y = 0; y < height; y++) {
        memcpy(correctedBuffer + (y * width * 4), buffer + (y * stride * 4), width * 4);
    }

    if (SDL_UpdateTexture(renderTexture, NULL, correctedBuffer, width * 4) != 0) {
        __android_log_print(ANDROID_LOG_VERBOSE, "ERROR", "SDL_UpdateTexture failed: %s", SDL_GetError());
    }

    if (SDL_RenderClear(renderer) != 0) {
        __android_log_print(ANDROID_LOG_VERBOSE, "ERROR", "SDL_RenderClear failed: %s", SDL_GetError());
    }

    if (SDL_RenderCopy(renderer, renderTexture, NULL, NULL) != 0) {
        __android_log_print(ANDROID_LOG_VERBOSE, "ERROR", "SDL_RenderCopy failed: %s", SDL_GetError());
    }
    SDL_RenderPresent(renderer);
}

void xxx3_osm_set_no_render_buffer(ANativeWindow_Buffer* buf) {
    buffer.width = sdlWindow->w;
    buffer.height = sdlWindow->h;
    buf->bits = xxx3_no_render_buffer;
    buf->stride = 0;
}

void xxx2_osm_apply_current_ll(ANativeWindow_Buffer* buf) {
    if (!bufferWasCreated) {
        abuffer = malloc(sdlWindow->w * sdlWindow->h * 6);
        bufferWasCreated = true;
    }
        OSMesaMakeCurrent_p(xxx3_osm->context,
                                abuffer,
                                GL_UNSIGNED_BYTE,
                                sdlWindow->w,
                                sdlWindow->h);

    if (buf->stride != last_stride)
        OSMesaPixelStore_p(OSMESA_ROW_LENGTH, buf->stride);
    last_stride = buf->stride;
}

void xxx3OsmMakeCurrent(SDL_Window *window) {
    if (hasCleaned){
        return;
    }

    if (!hasCleaned)
    {
        sdlWindow = window;
        renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, sdlWindow->w, sdlWindow->h);
        if (!renderer) {
            __android_log_print(ANDROID_LOG_VERBOSE, osm_LogTag, "Renderer creation failed: %s\n", SDL_GetError());
            SDL_Quit();
        }

        SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
        printf("%s making current\n", osm_LogTag);
        nativeSurface = data->zinkWindow;
        ANativeWindow_acquire(nativeSurface);
        ANativeWindow_setBuffersGeometry(nativeSurface, 0, 0, WINDOW_FORMAT_RGBX_8888);
        ANativeWindow_lock(nativeSurface, &buffer, NULL);
    }

    if (!hasSetNoRendererBuffer)
    {
        hasSetNoRendererBuffer = true;
        xxx3_osm_set_no_render_buffer(&buffer);
    }
    xxx2_osm_apply_current_ll(&buffer);
    OSMesaPixelStore_p(OSMESA_Y_UP, 0);

    if (!hasCleaned)
    {
        printf("%s vendor: %s\n", osm_LogTag, glGetString_p(GL_VENDOR));
        printf("%s renderer: %s\n", osm_LogTag, glGetString_p(GL_RENDERER));
        glClear_p(GL_COLOR_BUFFER_BIT);
        glClearColor_p(0.4f, 0.4f, 0.4f, 1.0f);  // Установить цвет фона
        glFinish_p();
        ANativeWindow_unlockAndPost(nativeSurface);
        hasCleaned = true;
    }
}

void xxx3OsmSwapBuffers() {
    if (!hasCleaned) return;
    ANativeWindow_lock(nativeSurface,&buffer, NULL);
    if (xxx3_osm) {
      xxx2_osm_apply_current_ll(&buffer);
    }
    glFinish_p();
    ANativeWindow_unlockAndPost(nativeSurface);
    UpdateSdlRender(abuffer,sdlWindow->w,sdlWindow->h,buffer.stride);
}

void SetupOsMContext(SDL_Window *window)
{
    if (window) {
        xxx3OsmMakeCurrent(window);
    }
}

bool osm_init(void) {
    load_vulkan();
    dlsym_OSMesa();
    xxx3_osm = xxx3OsmCreateContext();
    return true;
}

void SwapOsmBuffers (void){
    xxx3OsmSwapBuffers();
}
