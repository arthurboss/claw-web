package com.arthurboss.clawmobile;

import android.os.Bundle;
import androidx.activity.OnBackPressedCallback;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import com.getcapacitor.BridgeActivity;

public class MainActivity extends BridgeActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        hideSystemBars();
        setupBackButton();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        // Re-hide after any system UI interruption (e.g. a swipe revealing
        // the bars temporarily) brings focus back to the window.
        if (hasFocus) hideSystemBars();
    }

    // True fullscreen: draw edge-to-edge and hide both system bars, letting
    // the user swipe to reveal them temporarily (matches the web manifest's
    // display: fullscreen behavior).
    private void hideSystemBars() {
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
        WindowInsetsControllerCompat controller =
            WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        controller.hide(WindowInsetsCompat.Type.systemBars());
        controller.setSystemBarsBehavior(
            WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
    }

    // The game is a single-page canvas app with no WebView history, so the
    // default back behavior (BridgeActivity falling back to finish()) would
    // quit instantly on any back press/gesture. Route it to the same Escape
    // key the touch pause button and keyboard already use, so back opens the
    // game's own pause menu instead of exiting.
    private void setupBackButton() {
        getOnBackPressedDispatcher().addCallback(this, new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                String js =
                    "window.dispatchEvent(new KeyboardEvent('keydown',{key:'Escape',code:'Escape',keyCode:27,which:27,bubbles:true,cancelable:true}));"
                  + "window.dispatchEvent(new KeyboardEvent('keyup',{key:'Escape',code:'Escape',keyCode:27,which:27,bubbles:true,cancelable:true}));";
                getBridge().getWebView().evaluateJavascript(js, null);
            }
        });
    }
}
