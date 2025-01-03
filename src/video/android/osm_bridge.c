//
// Created by maks on 18.10.2023.
//
#include <malloc.h>
#include <string.h>
#include "SDL_video.h"
#include <android/log.h>
#include "osm_bridge.h"
#include "SDL_androidwindow.h"

static ANativeWindow *nativeSurface;
static ANativeWindow_Buffer buffer;
static xxx3_osm_render_window_t *xxx3_osm;
static bool hasCleaned = false;
static bool hasSetNoRendererBuffer = false;
static char xxx3_no_render_buffer[4];
static const char* osm_LogTag = "[ XXX3 OSM Bridge ]";
static int _swapInterval;
static int contextsIdGenerator;
static int32_t last_stride;

// Its not in a .h file because it is not supposed to be used outsife of this file.
void setNativeWindowSwapInterval(struct ANativeWindow* nativeWindow, int swapInterval);

bool osm_init(void) {
    return dlsym_OSMesa();
}

void xxx3_osm_set_no_render_buffer(ANativeWindow_Buffer* buf, SDL_Window *window) {
    buf->bits = &xxx3_no_render_buffer;
    buf->width = window->w;
    buf->height = window->h;
    buf->stride = 0;
}

struct xxx3_osm_render_window_t* xxx3OsmGetCurrentContext() {
    return xxx3_osm;
}

void* xxx3OsmCreateContext() {
    xxx3_osm_render_window_t *renderWindow = malloc(sizeof(xxx3_osm_render_window_t));
    if (!renderWindow)
    {
        printf("%s Failed to allocate memory for xxx3_osm\n", osm_LogTag);
        return NULL;
    }

    memset(renderWindow, 0, sizeof(xxx3_osm_render_window_t));

    printf("%s generating context\n", osm_LogTag);

    OSMesaContext osmesa_share = NULL;
  //  if (contextSrc != NULL) osmesa_share = contextSrc;

    OSMesaContext context = OSMesaCreateContext_p(OSMESA_RGBA, osmesa_share);
    if (context == NULL) {
        printf("%s OSMesaContext is Null!!!\n", osm_LogTag);
        return NULL;
    }

    renderWindow->context = context;
    renderWindow->id = contextsIdGenerator;
    contextsIdGenerator++;
    printf("%s context = %p\n", osm_LogTag, context);
    return renderWindow;
}

void xxx3_osm_apply_current(xxx3_osm_render_window_t *renderWindow) {
    if (renderWindow!=NULL) {
        OSMesaMakeCurrent_p(renderWindow->context, buffer.bits, GL_UNSIGNED_BYTE, buffer.width,
                            buffer.height);
        if (buffer.stride != last_stride)
            OSMesaPixelStore_p(OSMESA_ROW_LENGTH, buffer.stride);
        last_stride = buffer.stride;
    }
}

void xxx3OsmMakeCurrent(SDL_Window *window,xxx3_osm_render_window_t *renderWindow) {
    if (!hasCleaned)
    {
        SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
        printf("%s making current\n", osm_LogTag);
        nativeSurface = data->native_window;
        ANativeWindow_acquire(nativeSurface);
        ANativeWindow_setBuffersGeometry(nativeSurface, 0, 0, WINDOW_FORMAT_RGBX_8888);
        ANativeWindow_lock(nativeSurface, &buffer, NULL);
    }

    if (!hasSetNoRendererBuffer)
    {
        hasSetNoRendererBuffer = true;
        xxx3_osm_set_no_render_buffer(&buffer,window);
    }
    xxx3_osm = renderWindow;
    xxx3_osm_apply_current(renderWindow);
    OSMesaPixelStore_p(OSMESA_Y_UP, 0);

    if (!hasCleaned)
    {
        hasCleaned = true;
        printf("%s vendor: %s\n", osm_LogTag, glGetString_p(GL_VENDOR));
        printf("%s renderer: %s\n", osm_LogTag, glGetString_p(GL_RENDERER));
        glClear_p(GL_COLOR_BUFFER_BIT);
        glClearColor_p(0.4f, 0.4f, 0.4f, 1.0f);
        ANativeWindow_unlockAndPost(nativeSurface);
    }
}

void xxx3OsmSwapBuffers() {
    ANativeWindow_lock(nativeSurface,&buffer, NULL);
    if (xxx3_osm) {
        xxx3_osm_apply_current(xxx3_osm);
    }
    glFinish_p();
    ANativeWindow_unlockAndPost(nativeSurface);
}

void xxx3OsmSwapInterval(int interval) {
    if (nativeSurface != NULL)
        setNativeWindowSwapInterval(nativeSurface, interval);
}


int MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    if (window && context) {
        xxx3_osm_render_window_t *renderWindow = context;
        xxx3OsmMakeCurrent(window,renderWindow);
        return 0;
    } else {
        return -1;
    }
}

int GetSwapInterval (_THIS){
    return _swapInterval;
}

int SetSwapInterval (_THIS,int swapInterval){
    xxx3OsmSwapInterval(swapInterval);
    return 0;
}

SDL_GLContext GetCurrentContext (void){
    return xxx3OsmGetCurrentContext();
}

SDL_GLContext CreateGLContext (_THIS,SDL_Window *window){
    return xxx3OsmCreateContext();
}

int SwapWindow(_THIS, SDL_Window *window){
    xxx3OsmSwapBuffers();
    return 0;
}

void DestroyContext (_THIS,SDL_GLContext context){
    if (context!=NULL){
        xxx3_osm_render_window_t *contextToDestroy = (xxx3_osm_render_window_t *) context;
        if (contextToDestroy->id <0){
            return;
        }
        xxx3_osm_render_window_t *currentContextWindow = (xxx3_osm_render_window_t *) GetCurrentContext();
        if (currentContextWindow!=NULL && contextToDestroy->id == currentContextWindow->id){
            OSMesaMakeCurrent_p(NULL, buffer.bits, GL_UNSIGNED_BYTE, buffer.width,
                                buffer.height);
            xxx3_osm = NULL;
        }
        OSMesaDestroyContext_p(contextToDestroy->context);
    }
}
