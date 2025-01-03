//
// Created by maks on 18.10.2023.
//
#include <malloc.h>
#include <string.h>
#include "SDL_video.h"
#include <android/log.h>
#include "osm_bridge.h"
#include "SDL_androidwindow.h"

#define STATE_RENDERER_ALIVE 0
#define STATE_RENDERER_NEW_WINDOW 1

static const char* g_LogTag = "GLBridge";
static __thread osm_render_window_t* currentBundle;
static  basic_render_window_t* mainWindowBundle;
// a tiny buffer for rendering when there's nowhere t render
static char no_render_buffer[4];
static int _swapInterval;
static bool hasSetNoRendererBuffer = false;
static int contextsIdGenerator;

// Its not in a .h file because it is not supposed to be used outsife of this file.
void setNativeWindowSwapInterval(struct ANativeWindow* nativeWindow, int swapInterval);

bool osm_init(void) {
    if(!dlsym_OSMesa()) return false;
    return true;
}

osm_render_window_t* osm_get_current() {
    return currentBundle;
}

osm_render_window_t* osm_init_context(osm_render_window_t* share) {
    osm_render_window_t* render_window = malloc(sizeof(osm_render_window_t));
    if(render_window == NULL) return NULL;
    render_window->id = contextsIdGenerator;
    contextsIdGenerator++;
    memset(render_window, 0, sizeof(osm_render_window_t));
    OSMesaContext osmesa_share = NULL;
    if(share != NULL) osmesa_share = share->context;
    OSMesaContext context = OSMesaCreateContext_p(GL_RGBA, osmesa_share);
    if(context == NULL) {
        free(render_window);
        return NULL;
    }
    render_window->context = context;
    return render_window;
}

void osm_set_no_render_buffer(ANativeWindow_Buffer* buffer) {
    buffer->bits = &no_render_buffer;
    buffer->width = 1;
    buffer->height = 1;
    buffer->stride = 0;
}

void osm_swap_surfaces(osm_render_window_t* bundle) {
    if(bundle->nativeSurface != NULL && bundle->newNativeSurface != bundle->nativeSurface) {
        if(!bundle->disable_rendering) {
            __android_log_print(ANDROID_LOG_INFO, g_LogTag, "Unlocking for cleanup...");
            ANativeWindow_unlockAndPost(bundle->nativeSurface);
        }
        ANativeWindow_release(bundle->nativeSurface);
    }
    if(bundle->newNativeSurface != NULL) {
        __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "Switching to new native surface");
        bundle->nativeSurface = bundle->newNativeSurface;
        bundle->newNativeSurface = NULL;
        ANativeWindow_acquire(bundle->nativeSurface);
        ANativeWindow_setBuffersGeometry(bundle->nativeSurface, 0, 0, WINDOW_FORMAT_RGBX_8888);
        bundle->disable_rendering = false;
        return;
    }else {
        __android_log_print(ANDROID_LOG_ERROR, g_LogTag,
                            "No new native surface, switching to dummy framebuffer");
        bundle->nativeSurface = NULL;
        osm_set_no_render_buffer(&bundle->buffer);
        bundle->disable_rendering = true;
    }

}

void osm_release_window() {
    currentBundle->newNativeSurface = NULL;
    osm_swap_surfaces(currentBundle);
}

void osm_apply_current_ll() {
    ANativeWindow_Buffer* buffer = &currentBundle->buffer;
    OSMesaMakeCurrent_p(currentBundle->context, buffer->bits, GL_UNSIGNED_BYTE, buffer->width, buffer->height);
    if(buffer->stride != currentBundle->last_stride)
        OSMesaPixelStore_p(OSMESA_ROW_LENGTH, buffer->stride);
    currentBundle->last_stride = buffer->stride;
}

void osm_make_current(SDL_Window *window,osm_render_window_t* bundle) {
    if(bundle == NULL) {
        //technically this does nothing as its not possible to unbind a context in OSMesa
        OSMesaMakeCurrent_p(NULL, NULL, 0, 0, 0);
        currentBundle = NULL;
        return;
    }
    bool hasSetMainWindow = false;
    currentBundle = bundle;
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    if(mainWindowBundle == NULL) {
        mainWindowBundle = (basic_render_window_t*) bundle;
        __android_log_print(ANDROID_LOG_INFO, g_LogTag, "Main window bundle is now %p", mainWindowBundle);
        mainWindowBundle->newNativeSurface = data->native_window;
        hasSetMainWindow = true;
    }
    if(bundle->nativeSurface == NULL) {
        //prepare the buffer for our first render!
        osm_swap_surfaces(bundle);
        if(hasSetMainWindow) mainWindowBundle->state = STATE_RENDERER_ALIVE;
    }
    if (!hasSetNoRendererBuffer)
    {
        osm_set_no_render_buffer(&bundle->buffer);
        hasSetNoRendererBuffer = true;
    }
    osm_apply_current_ll();
    OSMesaPixelStore_p(OSMESA_Y_UP,0);
}


void osm_swap_buffers() {
    if(currentBundle->state == STATE_RENDERER_NEW_WINDOW) {
        osm_swap_surfaces(currentBundle);
        currentBundle->state = STATE_RENDERER_ALIVE;
    }

    if(currentBundle->nativeSurface != NULL && !currentBundle->disable_rendering)
        if(ANativeWindow_lock(currentBundle->nativeSurface, &currentBundle->buffer, NULL) != 0)
            osm_release_window();

    osm_apply_current_ll();
    glFinish_p(); // this will force osmesa to write the last rendered image into the buffer

    if(currentBundle->nativeSurface != NULL && !currentBundle->disable_rendering)
        if(ANativeWindow_unlockAndPost(currentBundle->nativeSurface) != 0)
            osm_release_window();
}

void osm_setup_window(SDL_Window *window) {
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    if(mainWindowBundle != NULL) {
        __android_log_print(ANDROID_LOG_INFO, g_LogTag, "Main window bundle is not NULL, changing state");
        mainWindowBundle->state = STATE_RENDERER_NEW_WINDOW;
        mainWindowBundle->newNativeSurface = data->native_window;
    }
}

void osm_swap_interval(int swapInterval) {
    if(mainWindowBundle != NULL && mainWindowBundle->nativeSurface != NULL) {
        _swapInterval = swapInterval;
        setNativeWindowSwapInterval(mainWindowBundle->nativeSurface, swapInterval);
    }
}

int MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    if (window && context) {
        osm_make_current(window,context);
        return 0;
    } else {
        return -1;
    }
}

int GetSwapInterval (_THIS){
    return _swapInterval;
}

int SetSwapInterval (_THIS,int swapInterval){
    osm_swap_interval(swapInterval);
    return 0;
}

SDL_GLContext GetCurrentContext (void){
    return osm_get_current();
}
SDL_GLContext CreateGLContext (_THIS,SDL_Window *window){
    return osm_init_context(NULL);
}

void SetupWindow (SDL_Window *window){
    osm_setup_window (window);
}

int SwapWindow(_THIS, SDL_Window *window){
    osm_swap_buffers();
    return 0;
}

void DestroyContext (SDL_GLContext context){
    if (context!=NULL){
        osm_render_window_t *contextRenderWindow = (osm_render_window_t *) context;
        if (contextRenderWindow->id <0){
            return;
        }
        osm_render_window_t *currentContextWindow = (osm_render_window_t *) GetCurrentContext();
        if (currentContextWindow!=NULL && contextRenderWindow->id == currentContextWindow->id){
            OSMesaMakeCurrent_p(NULL, NULL, 0, 0, 0);
            currentBundle = NULL;
        }
        OSMesaDestroyContext_p(contextRenderWindow->context);
    }
}
