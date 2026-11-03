package org.orca;

import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;
import android.text.InputType;
import android.util.Base64;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import androidx.browser.customtabs.CustomTabsIntent;
import java.nio.ByteBuffer;
import java.security.KeyStore;
import java.util.Arrays;
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;

/**
 * NativeActivity with the few services the engine needs from Java: soft
 * keyboard text, links the app is opened with, browser sign-in pages, window
 * insets and Keystore-encrypted credentials. The engine runs on its own thread
 * (android_main); native callbacks only copy data into its queues.
 */
public class OrcaActivity extends NativeActivity {
    static { System.loadLibrary("main"); }

    static native void nativeText(String text);
    static native void nativeKey(int keyCode);
    static native void nativeUrl(String url);
    static native void nativeInsets(float left, float top, float right, float bottom, float keyboard);

    private InputView input;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        input = new InputView(this);
        addContentView(input, new ViewGroup.LayoutParams(1, 1));
        View decor = getWindow().getDecorView();
        decor.setOnApplyWindowInsetsListener((view, insets) -> {
            reportInsets(insets);
            return view.onApplyWindowInsets(insets);
        });
        deliver(getIntent());
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        deliver(intent);
    }

    private static void deliver(Intent intent) {
        Uri data = intent != null ? intent.getData() : null;
        if (data != null) nativeUrl(data.toString());
    }

    private void reportInsets(WindowInsets insets) {
        float density = getResources().getDisplayMetrics().density;
        float left, top, right, bottom, keyboard;
        if (Build.VERSION.SDK_INT >= 30) {
            android.graphics.Insets bars = insets.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.displayCutout());
            android.graphics.Insets ime = insets.getInsets(WindowInsets.Type.ime());
            left = bars.left; top = bars.top; right = bars.right; bottom = bars.bottom;
            keyboard = Math.max(0, ime.bottom - bars.bottom);
        } else {
            left = insets.getSystemWindowInsetLeft(); top = insets.getSystemWindowInsetTop();
            right = insets.getSystemWindowInsetRight(); bottom = insets.getStableInsetBottom();
            keyboard = Math.max(0, insets.getSystemWindowInsetBottom() - bottom);
        }
        nativeInsets(left / density, top / density, right / density, bottom / density, keyboard / density);
    }

    // ── called from the engine thread ──────────────────────────────────

    public void showKeyboard(boolean visible, int type) {
        runOnUiThread(() -> {
            InputMethodManager manager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
            if (visible) {
                input.setType(type);
                input.requestFocus();
                manager.restartInput(input);
                manager.showSoftInput(input, 0);
            } else {
                manager.hideSoftInputFromWindow(input.getWindowToken(), 0);
                input.clearFocus();
            }
        });
    }

    /** Opens a sign-in page in a Custom Tab, or the default browser. */
    public boolean openUrl(String url) {
        Uri uri = Uri.parse(url);
        if (!"https".equals(uri.getScheme())) return false;
        runOnUiThread(() -> {
            try {
                new CustomTabsIntent.Builder().setShowTitle(true).build().launchUrl(this, uri);
            } catch (RuntimeException unavailable) {
                startActivity(new Intent(Intent.ACTION_VIEW, uri));
            }
        });
        return true;
    }

    // Credentials: AES-GCM with a non-exportable Keystore key; ciphertext in private preferences.
    private static final String KEY_ALIAS = "orca.credentials";

    private SharedPreferences store() { return getSharedPreferences("orca-credentials", MODE_PRIVATE); }

    private static String name(byte[] key) { return Base64.encodeToString(key, Base64.NO_WRAP | Base64.URL_SAFE); }

    private static SecretKey secret() throws Exception {
        KeyStore keys = KeyStore.getInstance("AndroidKeyStore");
        keys.load(null);
        if (keys.containsAlias(KEY_ALIAS)) return ((KeyStore.SecretKeyEntry) keys.getEntry(KEY_ALIAS, null)).getSecretKey();
        KeyGenerator generator = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore");
        generator.init(new KeyGenParameterSpec.Builder(KEY_ALIAS, KeyProperties.PURPOSE_ENCRYPT | KeyProperties.PURPOSE_DECRYPT)
            .setBlockModes(KeyProperties.BLOCK_MODE_GCM).setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
            .setKeySize(256).build());
        return generator.generateKey();
    }

    public byte[] credentialGet(byte[] key) throws Exception {
        String stored = store().getString(name(key), null);
        if (stored == null) return null;
        byte[] data = Base64.decode(stored, Base64.NO_WRAP);
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        cipher.init(Cipher.DECRYPT_MODE, secret(), new GCMParameterSpec(128, data, 0, 12));
        return cipher.doFinal(data, 12, data.length - 12);
    }

    public boolean credentialSet(byte[] key, byte[] value) throws Exception {
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        cipher.init(Cipher.ENCRYPT_MODE, secret());
        byte[] iv = cipher.getIV(), sealed = cipher.doFinal(value);
        byte[] data = ByteBuffer.allocate(iv.length + sealed.length).put(iv).put(sealed).array();
        Arrays.fill(value, (byte) 0);
        return store().edit().putString(name(key), Base64.encodeToString(data, Base64.NO_WRAP)).commit();
    }

    public boolean credentialDelete(byte[] key) {
        return store().edit().remove(name(key)).commit();
    }

    /**
     * A one-pixel view that owns the IME connection. Composing text is sent
     * as it changes (replacing the previous composition with backspaces), so
     * fields update while the user types, as with a hardware keyboard.
     */
    static final class InputView extends View {
        private int type;

        InputView(Context context) {
            super(context);
            setFocusable(true);
            setFocusableInTouchMode(true);
        }

        void setType(int type) { this.type = type; }

        @Override public boolean onCheckIsTextEditor() { return true; }

        @Override
        public InputConnection onCreateInputConnection(EditorInfo info) {
            switch (type) {
                case 1: info.inputType = InputType.TYPE_CLASS_NUMBER; break;
                case 2: info.inputType = InputType.TYPE_CLASS_PHONE; break;
                case 3: info.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS; break;
                case 4: info.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI; break;
                case 5: info.inputType = InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_DECIMAL; break;
                case 6: info.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD; break;
                default: info.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_CAP_SENTENCES; break;
            }
            info.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI | EditorInfo.IME_ACTION_DONE;
            return new BaseInputConnection(this, false) {
                private String composing = "";

                private void replace(CharSequence text) {
                    String next = text == null ? "" : text.toString();
                    int common = 0;
                    while (common < composing.length() && common < next.length() && composing.charAt(common) == next.charAt(common)) common++;
                    for (int i = composing.codePointCount(common, composing.length()); i > 0; i--) nativeKey(KeyEvent.KEYCODE_DEL);
                    if (common < next.length()) nativeText(next.substring(common));
                    composing = next;
                }

                @Override public boolean setComposingText(CharSequence text, int position) { replace(text); return true; }
                @Override public boolean commitText(CharSequence text, int position) { replace(text); composing = ""; return true; }
                @Override public boolean finishComposingText() { composing = ""; return true; }

                @Override
                public boolean deleteSurroundingText(int before, int after) {
                    for (int i = 0; i < before; i++) nativeKey(KeyEvent.KEYCODE_DEL);
                    for (int i = 0; i < after; i++) nativeKey(KeyEvent.KEYCODE_FORWARD_DEL);
                    return true;
                }

                @Override
                public boolean sendKeyEvent(KeyEvent event) {
                    if (event.getAction() != KeyEvent.ACTION_DOWN) return true;
                    int code = event.getKeyCode();
                    if (code == KeyEvent.KEYCODE_DEL || code == KeyEvent.KEYCODE_ENTER || code == KeyEvent.KEYCODE_FORWARD_DEL) nativeKey(code);
                    else if (event.getUnicodeChar() != 0) nativeText(new String(Character.toChars(event.getUnicodeChar())));
                    return true;
                }

                @Override
                public boolean performEditorAction(int action) {
                    nativeKey(KeyEvent.KEYCODE_ENTER);
                    return true;
                }
            };
        }
    }
}
