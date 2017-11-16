
package network.object.onexapp;

import android.os.*;
import android.view.*;
import android.view.inputmethod.InputMethodManager;
import android.content.Context;
import android.app.NativeActivity;

/**  NativeActivity wrapper to offer Java API functions not available in JNI land.
  */
public class OnexNativeActivity extends NativeActivity implements KeyEvent.Callback {

    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState); System.out.println("onCreate");
        System.loadLibrary("onexapp");
    }

    @Override
    public void onRestart(){
        super.onRestart(); System.out.println("onRestart");
    }

    @Override
    public void onStart(){
        super.onStart(); System.out.println("onStart");
    }

    @Override
    public void onResume(){
        super.onResume(); System.out.println("onResume");
    }

    @Override
    public void onPause(){
        super.onPause(); System.out.println("onPause");
    }

    @Override
    public void onStop(){
        super.onStop(); System.out.println("onStop");
    }

    @Override
    public void onDestroy(){
        super.onDestroy(); System.out.println("onDestroy");
    }

    // -----------------------------------------------------------

    public void showKeyboard()
    {
        InputMethodManager imm=(InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.showSoftInput(this.getWindow().getDecorView(), InputMethodManager.SHOW_FORCED);
    }

    public void hideKeyboard()
    {
        InputMethodManager imm=(InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(this.getWindow().getDecorView().getWindowToken(), 0);
    }

    // -----------------------------------------------------------

    public static native void onKeyPress(int keyCode, String key);

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event){
      char unicodeChar = event.isPrintingKey()? (char)event.getUnicodeChar(): ' ';
      onKeyPress(keyCode, ""+unicodeChar);
      return true;
    }

    @Override
    public boolean onKeyMultiple(int keyCode, int count, KeyEvent event){
      onKeyPress(keyCode, event.getCharacters());
      return true;
    }

    @Override
    public void onBackPressed(){
        return;
    }

    // -----------------------------------------------------------
}

