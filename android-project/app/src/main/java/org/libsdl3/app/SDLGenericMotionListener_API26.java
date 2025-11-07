package org.libsdl3.app;

import android.os.Build;
import android.view.InputDevice;
import android.view.MotionEvent;

public class SDLGenericMotionListener_API26 extends SDLGenericMotionListener_API24 {
    // Generic Motion (mouse hover, joystick...) events go here
    private boolean mRelativeModeEnabled;

    @Override
    boolean supportsRelativeMouse() {
        return (!SDLActivity.isDeXMode() || Build.VERSION.SDK_INT >= 27 /* Android 8.1 (O_MR1) */);
    }

    @Override
    public boolean inRelativeMode() {
        return mRelativeModeEnabled;
    }

    @Override
    boolean setRelativeMouseEnabled(boolean enabled) {

        if (Build.VERSION.SDK_INT < 26 /* Android 8.0 (O) */) {
            /* Silence 'lint' warning */
            return false;
        }

        if (!SDLActivity.isDeXMode() || Build.VERSION.SDK_INT >= 27 /* Android 8.1 (O_MR1) */) {
            if (enabled) {
                SDLActivity.getContentView().requestPointerCapture();
            } else {
                SDLActivity.getContentView().releasePointerCapture();
            }
            mRelativeModeEnabled = enabled;
            return true;
        } else {
            return false;
        }
    }

    @Override
    void reclaimRelativeMouseModeIfNeeded() {

        if (Build.VERSION.SDK_INT < 26 /* Android 8.0 (O) */) {
            /* Silence 'lint' warning */
            return;
        }

        if (mRelativeModeEnabled && !SDLActivity.isDeXMode()) {
            SDLActivity.getContentView().requestPointerCapture();
        }
    }

    @Override
    boolean checkRelativeEvent(MotionEvent event) {
        if (Build.VERSION.SDK_INT < 26 /* Android 8.0 (O) */) {
            /* Silence 'lint' warning */
            return false;
        }
        return event.getSource() == InputDevice.SOURCE_MOUSE_RELATIVE;
    }

    @Override
    public float getEventX(MotionEvent event, int pointerIndex) {
        // Relative mouse in capture mode will only have relative for X/Y
        return event.getX(pointerIndex);
    }

    @Override
    public float getEventY(MotionEvent event, int pointerIndex) {
        // Relative mouse in capture mode will only have relative for X/Y
        return event.getY(pointerIndex);
    }
}
