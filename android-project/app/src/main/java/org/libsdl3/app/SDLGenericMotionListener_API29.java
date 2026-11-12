package org.libsdl3.app;

import android.annotation.SuppressLint;
import android.view.InputDevice;

@SuppressLint("NewApi") // prevent lint Error: Call requires API level 29 (current min is 21): android.view.InputDevice#isExternal [NewApi]
public class SDLGenericMotionListener_API29 extends SDLGenericMotionListener_API26 {
    @Override
    public int getPenDeviceType(InputDevice penDevice)
    {
        if (penDevice == null) {
            return SDL_PEN_DEVICE_TYPE_UNKNOWN;
        }

        return penDevice.isExternal() ? SDL_PEN_DEVICE_TYPE_INDIRECT : SDL_PEN_DEVICE_TYPE_DIRECT;
    }
}
