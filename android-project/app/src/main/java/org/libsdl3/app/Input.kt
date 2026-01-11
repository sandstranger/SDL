@file:JvmName("Input")

package org.libsdl3.app

import android.view.KeyEvent
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

private const val ESCAPE_KEYCODE = 4
const val INPUT_DELAY_MILLIS : Long = 50

private val inputCoroutineScope = CoroutineScope(Dispatchers.Default)

fun onKeyDown(keyCode: Int,startDelayMS : Long = 0, delayBeforeKeyRelease : Long = 0,
              repeatCount : Int = 1) {
    if (startDelayMS == 0L && delayBeforeKeyRelease == 0L){
        onKeyDown(keyCode)
    }
    else{
        inputCoroutineScope.launch {
            onKeyDownTask(keyCode, startDelayMS, delayBeforeKeyRelease,repeatCount)
        }
    }
}

internal fun onEscapeBtnClicked (keyCode : Int, event : KeyEvent) {
    if (event.action == KeyEvent.ACTION_DOWN && keyCode == ESCAPE_KEYCODE) {
        onKeyDown(KeyEvent.KEYCODE_ESCAPE, delayBeforeKeyRelease = INPUT_DELAY_MILLIS)
    }
}

suspend fun onKeyDownTask(keyCode: Int,startDelayMS : Long = 0L, delayBeforeKeyRelease : Long = 0L,
                                  repeatCount : Int = 1){
    for (i in 0 until repeatCount) {
        if (startDelayMS > 0) {
            delay(startDelayMS)
        }
        SDLActivity.onNativeKeyDown(keyCode)
        if (delayBeforeKeyRelease > 0) {
            delay(delayBeforeKeyRelease)
        }
        SDLActivity.onNativeKeyUp(keyCode)
    }
}

private fun onKeyDown (keyCode: Int){
    SDLActivity.onNativeKeyDown(keyCode)
    SDLActivity.onNativeKeyUp(keyCode)
}