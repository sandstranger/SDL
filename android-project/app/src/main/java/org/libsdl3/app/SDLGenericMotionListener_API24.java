package org.libsdl3.app;

import android.os.Build;
import android.view.MotionEvent;

public class SDLGenericMotionListener_API24 extends SDLGenericMotionListener_API14 {
    // Generic Motion (mouse hover, joystick...) events go here

    private boolean mRelativeModeEnabled;

    @Override
    boolean supportsRelativeMouse() {
        return true;
    }

    @Override
    public boolean inRelativeMode() {
        return mRelativeModeEnabled;
    }

    @Override
    boolean setRelativeMouseEnabled(boolean enabled) {
        mRelativeModeEnabled = enabled;
        return true;
    }

    @Override
    public float getEventX(MotionEvent event, int pointerIndex) {
        if (Build.VERSION.SDK_INT < 24 /* Android 7.0 (N) */) {
            /* Silence 'lint' warning */
            return 0;
        }

        if (mRelativeModeEnabled && event.getToolType(pointerIndex) == MotionEvent.TOOL_TYPE_MOUSE) {
            return event.getAxisValue(MotionEvent.AXIS_RELATIVE_X, pointerIndex);
        } else {
            return event.getX(pointerIndex);
        }
    }

    @Override
    public float getEventY(MotionEvent event, int pointerIndex) {
        if (Build.VERSION.SDK_INT < 24 /* Android 7.0 (N) */) {
            /* Silence 'lint' warning */
            return 0;
        }

        if (mRelativeModeEnabled && event.getToolType(pointerIndex) == MotionEvent.TOOL_TYPE_MOUSE) {
            return event.getAxisValue(MotionEvent.AXIS_RELATIVE_Y, pointerIndex);
        } else {
            return event.getY(pointerIndex);
        }
    }
}
