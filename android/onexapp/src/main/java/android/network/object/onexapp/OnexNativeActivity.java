
package network.object.onexapp;

import android.os.*;
import android.view.inputmethod.*;
import android.view.ViewGroup.LayoutParams;
import android.view.KeyEvent;
import android.widget.*;
import android.text.*;
import android.content.Context;
import android.app.NativeActivity;
import android.content.*;
import android.util.Log;
import android.app.*;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;

/**  NativeActivity wrapper to offer Java API functions not available in JNI land.
  */
public class OnexNativeActivity extends NativeActivity implements KeyEvent.Callback {

    static OnexNativeActivity self=null;

    public static final String LOGNAME = "OnexNativeActivity";

    private static final int REQUEST_SELECT_DEVICE = 1;
    private static final int REQUEST_ENABLE_BT = 2;

    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState); System.out.println("onCreate");
        self=this;
        setUpKeyboardView();
    }

    @Override
    public void onNewIntent(Intent intent){
      super.onNewIntent(intent);
      Log.d(LOGNAME, "onNewIntent("+intent.getAction()+")");
      if(!"android.hardware.usb.action.USB_DEVICE_ATTACHED".equalsIgnoreCase(intent.getAction())) return;
      EternalService.onUSBAttached(intent);
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
        sendBroadcast(new Intent("network.object.onexapp.eternal.restart"));
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
        self=null;
    }

    // -----------------------------------------------------------

    static private BluetoothAdapter bluetoothAdapter = null;

    static public void getBLEMac(){
        Log.d(LOGNAME, "getBLEMac()");
        if(self==null){
          Log.d(LOGNAME, "calling getBLEMac without a running activity");
          return;
        }
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
          Toast.makeText(self, "Bluetooth is not available", Toast.LENGTH_LONG).show();
          return;
        }
        if (!bluetoothAdapter.isEnabled()) {
          Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
          self.startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
        }
        else {
          Intent newIntent = new Intent(self, DeviceListActivity.class);
          self.startActivityForResult(newIntent, REQUEST_SELECT_DEVICE);
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
      Log.d(LOGNAME, "onActivityResult()");
      switch (requestCode) {
        case REQUEST_ENABLE_BT:
            if (resultCode == Activity.RESULT_OK) {
                Toast.makeText(this, "Bluetooth enabled", Toast.LENGTH_SHORT).show();
                Intent newIntent = new Intent(this, DeviceListActivity.class);
                startActivityForResult(newIntent, REQUEST_SELECT_DEVICE);
            } else {
                Log.d(LOGNAME, "Bluetooth not enabled");
                Toast.makeText(this, "Bluetooth not enabled", Toast.LENGTH_SHORT).show();
            }
            break;
        case REQUEST_SELECT_DEVICE:
            if (resultCode == Activity.RESULT_OK && data != null) {
                String blemac = data.getStringExtra(BluetoothDevice.EXTRA_DEVICE);
                Log.d(LOGNAME, "onActivityResult() select device OK: "+blemac);
                EternalService.onBLEMacSelected(blemac);
            }
            else {
                Log.d(LOGNAME, "onActivityResult() select device not OK");
                EternalService.onBLEMacSelected("00:00:00:00:00:00");
            }
            break;
        default:
            Log.e(LOGNAME, "wrong request code");
            break;
      }
    }

    // -----------------------------------------------------------

    public static final int KEY_BACKSPACE=0x0043;
    public static final int KEY_ENTER    =0x0042;
    public static final int KEY_BACK     =0x100a;

    private int keyboardType = InputType.TYPE_CLASS_TEXT;
    private boolean keyboardUp=false;

    public class KeyboardView extends EditText {
      public String prevText="";
      public KeyboardView(Context context) { super(context); }
      @Override
      public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        if(!keyboardUp){
          InputMethodManager imm=(InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
          imm.hideSoftInputFromWindow(kbdView.getWindowToken(), 0);
          return null;
        }
        InputConnection inputConnection = super.onCreateInputConnection(outAttrs);
        outAttrs.inputType |= keyboardType; // | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
        return inputConnection;
      }
      @Override
      public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        boolean r=super.onKeyPreIme(keyCode, event);
        if(keyboardUp && event.getKeyCode() == KeyEvent.KEYCODE_BACK) {
          if(event.getAction() == KeyEvent.ACTION_DOWN){ onKeyPress(KEY_BACK, 0); return true; }
          if(event.getAction() == KeyEvent.ACTION_UP) {  onKeyRelease(KEY_BACK); keyboardUp=false; return true; }
        }
        return r;
      }
    }

    private KeyboardView kbdView;

    public void delay(int ms){ try{ Thread.sleep(ms); }catch(Exception e){}; }

    public void setUpKeyboardView(){
        kbdView = new KeyboardView(this);
        kbdView.setImeOptions(EditorInfo.IME_FLAG_NO_FULLSCREEN);
        kbdView.addTextChangedListener(new TextWatcher(){
            public void onTextChanged(CharSequence cs, int start, int before, int count) {
              String currText=cs.toString();
              String prevText=kbdView.prevText;
              int i; for(i=0; i<prevText.length() && i<currText.length(); i++){
                  if(prevText.charAt(i) != currText.charAt(i)) break;
              }
              String deled=""; String added="";
              if(i< prevText.length()) deled=prevText.substring(i);
              if(i< currText.length()) added=currText.substring(i);
              int deln=deled.length();
              int addn=added.length();
              int ch;
              if(deled!="") for(int n=0; n< deln; n+=Character.charCount(ch)){ ch=deled.codePointAt(n); activateKey(KEY_BACKSPACE, 0); }
              if(added!="") for(int n=0; n< addn; n+=Character.charCount(ch)){ ch=added.codePointAt(n); activateKey(-1, ch);           }
              kbdView.prevText=currText;
            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after){}
            public void afterTextChanged(Editable s){}
        });
        addContentView(kbdView, new LayoutParams(10, 10));
    }

    public void activateKey(int keyCode, int ch){
      if(ch=='\n'){ keyCode=KEY_ENTER; ch=0; }
      onKeyPress(keyCode, ch);
      onKeyRelease(keyCode);
      delay(50);
    }

    public static native void onKeyPress(int keyCode, int key);
    public static native void onKeyRelease(int keyCode);

    public void showKeyboard(int type)
    {
        keyboardUp=true;
        keyboardType = type;
        InputMethodManager imm=(InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.restartInput(kbdView);
        imm.showSoftInput(kbdView, InputMethodManager.SHOW_FORCED);
    }

    public void hideKeyboard()
    {
        kbdView.post(new Runnable(){ public void run(){ kbdView.prevText=""; kbdView.setText(""); }});
        InputMethodManager imm=(InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(kbdView.getWindowToken(), 0);
        keyboardUp=false;
    }

    // -----------------------------------------------------------

    public void setAlarm(long when, String uid){
      Intent intent = new Intent("Onex.Alarm");
      intent.putExtra("UID", uid);
      PendingIntent pendingIntent = PendingIntent.getBroadcast(this, uid.hashCode(), intent, 0);
      AlarmManager am=(AlarmManager)getSystemService(Context.ALARM_SERVICE);
      if(when!=0) am.set(AlarmManager.RTC_WAKEUP, when*1000, pendingIntent);
      else        am.cancel(pendingIntent);
    }

    public static native void onAlarmRecv(String uid);

    static public void alarmReceived(Context context, Intent intent){
      String uid=intent.getStringExtra("UID");
      if(self!=null) self.onAlarmRecv(uid);
      else context.startActivity(new Intent(context, OnexNativeActivity.class));
    }

    public void showNotification(String title, String text){
      Log.d(LOGNAME, "showNotification!!");
      PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, new Intent(this, OnexNativeActivity.class), PendingIntent.FLAG_UPDATE_CURRENT);
      Notification.Builder notifbuilder = new Notification.Builder(this, "Notification-Channel-ID");
      notifbuilder.setContentTitle(title)
                  .setContentText(text)
                  .setSmallIcon(R.drawable.icon)
                  .setContentIntent(pendingIntent);
      NotificationManager notifMgr = (NotificationManager)this.getSystemService(this.NOTIFICATION_SERVICE);
      notifMgr.notify(12345, notifbuilder.build());
    }

    // -----------------------------------------------------------

    private void showMessage(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    // -----------------------------------------------------------
}

