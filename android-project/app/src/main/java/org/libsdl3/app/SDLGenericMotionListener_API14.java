package org.libsdl3.app;

import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;

public class SDLGenericMotionListener_API14 implements View.OnGenericMotionListener {
    public static final int SDL_PEN_DEVICE_TYPE_UNKNOWN = 0;
    public static final int SDL_PEN_DEVICE_TYPE_DIRECT = 1;
    public static final int SDL_PEN_DEVICE_TYPE_INDIRECT = 2;

    // Generic Motion (mouse hover, joystick...) events go here
    @Override
    public boolean onGenericMotion(View v, MotionEvent event) {
        if (event.getSource() == InputDevice.SOURCE_JOYSTICK)
            return SDLControllerManager.handleJoystickMotionEvent(event);

        float x, y;
        int action = event.getActionMasked();
        int pointerCount = event.getPointerCount();
        boolean consumed = false;

        for (int i = 0; i < pointerCount; i++) {
            int toolType = event.getToolType(i);

            if (toolType == MotionEvent.TOOL_TYPE_MOUSE) {
                switch (action) {
                    case MotionEvent.ACTION_SCROLL:
                        x = event.getAxisValue(MotionEvent.AXIS_HSCROLL, i);
                        y = event.getAxisValue(MotionEvent.AXIS_VSCROLL, i);
                        SDLActivity.onNativeMouse(0, action, x, y, false);
                        consumed = true;
                        break;

                    case MotionEvent.ACTION_HOVER_MOVE:
                        x = getEventX(event, i);
                        y = getEventY(event, i);

                        SDLActivity.onNativeMouse(0, action, x, y, checkRelativeEvent(event));
                        consumed = true;
                        break;

                    default:
                        break;
                }
            } else if (toolType == MotionEvent.TOOL_TYPE_STYLUS || toolType == MotionEvent.TOOL_TYPE_ERASER) {
                switch (action) {
                    case MotionEvent.ACTION_HOVER_ENTER:
                    case MotionEvent.ACTION_HOVER_MOVE:
                    case MotionEvent.ACTION_HOVER_EXIT:
                        x = event.getX(i);
                        y = event.getY(i);
                        float p = event.getPressure(i);
                        if (p > 1.0f) {
                            // may be larger than 1.0f on some devices
                            // see the documentation of getPressure(i)
                            p = 1.0f;
                        }

                        // BUTTON_STYLUS_PRIMARY is 2^5, so shift by 4, and apply SDL_PEN_INPUT_DOWN/SDL_PEN_INPUT_ERASER_TIP
                        int buttons = (event.getButtonState() >> 4) | (1 << (toolType == MotionEvent.TOOL_TYPE_STYLUS ? 0 : 30));

                        SDLActivity.onNativePen(event.getPointerId(i), getPenDeviceType(event.getDevice()), buttons, action, x, y, p);
                        consumed = true;
                        break;
                }
            }
        }

        return consumed;
    }

    boolean supportsRelativeMouse() {
        return false;
    }

    public boolean inRelativeMode() {
        return false;
    }

    boolean setRelativeMouseEnabled(boolean enabled) {
        return false;
    }

    void reclaimRelativeMouseModeIfNeeded() {

    }

    boolean checkRelativeEvent(MotionEvent event) {
        return inRelativeMode();
    }

    public float getEventX(MotionEvent event, int pointerIndex) {
        return event.getX(pointerIndex);
    }

    public float getEventY(MotionEvent event, int pointerIndex) {
        return event.getY(pointerIndex);
    }

    public int getPenDeviceType(InputDevice penDevice) {
        return SDL_PEN_DEVICE_TYPE_UNKNOWN;
    }
}
