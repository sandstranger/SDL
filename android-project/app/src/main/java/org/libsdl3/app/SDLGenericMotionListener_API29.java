package org.libsdl3.app;

import android.view.InputDevice;

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
