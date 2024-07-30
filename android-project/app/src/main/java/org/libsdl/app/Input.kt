package org.libsdl.app

import android.view.KeyEvent
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

private const val ESCAPE_KEYCODE = 4
private const val INPUT_DELAY_MILLIS : Long = 50

fun onKeyDown(keyCode: Int) {
    CoroutineScope(Dispatchers.Default).launch {
        onKeyDownTask(keyCode)
    }
}

internal fun onEscapeBtnClicked (keyCode : Int, event : KeyEvent) {
    if (event.action == KeyEvent.ACTION_DOWN && keyCode == ESCAPE_KEYCODE) {
        onKeyDown(KeyEvent.KEYCODE_ESCAPE)
    }
}

private suspend fun onKeyDownTask(keyCode: Int){
    SDLActivity.onNativeKeyDown(keyCode)
    delay(INPUT_DELAY_MILLIS)
    SDLActivity.onNativeKeyUp(keyCode)
}