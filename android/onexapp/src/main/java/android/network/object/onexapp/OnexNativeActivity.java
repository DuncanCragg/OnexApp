
package network.object.onexapp;

import java.util.List;

import android.Manifest;
import android.os.*;
import android.net.Uri;
import android.view.inputmethod.*;
import android.view.ViewGroup.LayoutParams;
import android.view.KeyEvent;
import android.widget.*;
import android.text.*;
import android.content.*;
import android.content.pm.*;
import android.provider.Settings;
import android.app.NativeActivity;
import android.util.Log;
import android.app.*;

import androidx.core.content.ContextCompat;
import androidx.core.app.ActivityCompat;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;

/**  NativeActivity wrapper to offer Java API functions not available in JNI land.
  */
public class OnexNativeActivity extends NativeActivity implements KeyEvent.Callback {

    static OnexNativeActivity self=null;

    public static final String LOGNAME = "Onex OnexNativeActivity";

    private static final int REQUEST_SELECT_DEVICE = 1;
    private static final int REQUEST_ENABLE_BT = 2;

    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState); Log.d(LOGNAME, "onCreate");
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
        super.onRestart(); Log.d(LOGNAME, "onRestart");
    }

    @Override
    public void onStart(){
        super.onStart(); Log.d(LOGNAME, "onStart");
    }

    static private int fileRWPerms = 0; // 0 waiting, 1 approved, -1 denied

    private void checkAndRequestFileRWPermission() {

        if(Build.VERSION.SDK_INT >= 30) {

            if(Environment.isExternalStorageManager()){
                Log.d(LOGNAME, "API 30+ has file RW permission");
                fileRWPerms=1;
                return;
            }
            try {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                intent.addCategory("android.intent.category.DEFAULT");
                intent.setData(Uri.parse(String.format("package:%s",getApplicationContext().getPackageName())));
                startActivityForResult(intent, 5544);

            } catch (Exception e) {
                Intent intent = new Intent();
                intent.setAction(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                startActivityForResult(intent, 5544);
            }

        } else {

            int readext = ContextCompat.checkSelfPermission(OnexNativeActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE);
            int writext = ContextCompat.checkSelfPermission(OnexNativeActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE);

            if(readext==PackageManager.PERMISSION_GRANTED &&
               writext==PackageManager.PERMISSION_GRANTED    ){

                Log.d(LOGNAME, "API 29 has file RW permission");
                fileRWPerms=1;
                return;
            }
            ActivityCompat.requestPermissions(OnexNativeActivity.this, new String[]{ Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE }, 5544);
        }
    }

    @Override
    public void onResume(){
        super.onResume(); Log.d(LOGNAME, "onResume");

        // relies on an onResume from the finishing of the file RW permissions dialogues

        if(fileRWPerms==0) checkAndRequestFileRWPermission();

        restartEternal();
    }

    @Override
    public void onPause(){
        super.onPause(); Log.d(LOGNAME, "onPause");
    }

    @Override
    public void onStop(){
        super.onStop(); Log.d(LOGNAME, "onStop");
    }

    @Override
    public void onDestroy(){
        super.onDestroy(); Log.d(LOGNAME, "onDestroy");
        self=null;
    }

    static public void restartEternal(){
        if(self==null){
            Log.d(LOGNAME, "calling restartEternal without a running activity");
            return;
        }
        if(fileRWPerms!=1){
            Log.d(LOGNAME, "calling restartEternal without file RW permissions");
            return;
        }
        Intent intent = new Intent("network.object.onexapp.eternal.restart").setFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
        PackageManager packageManager = self.getPackageManager();
        List<ResolveInfo> broadcastReceivers = packageManager.queryBroadcastReceivers(intent, 0);
        for(ResolveInfo broadcastReceiver: broadcastReceivers) {
            ComponentName cn = new ComponentName(broadcastReceiver.activityInfo.packageName, broadcastReceiver.activityInfo.name);
            Log.d(LOGNAME, "restartEternal on "+cn);
            intent.setComponent(cn);
            self.sendBroadcast(intent);
        }
    }

    // -----------------------------------------------------------

    static private BluetoothAdapter bluetoothAdapter = null;

    static public void selectBLEMac(){
        Log.d(LOGNAME, "selectBLEMac()");
        if(self==null){
          Log.d(LOGNAME, "calling selectBLEMac without a running activity");
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
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if(requestCode==5544){
            fileRWPerms= -1;
            if (grantResults.length >= 1) {

                int readext = grantResults[0];
                int writext = grantResults[1];

                if(readext==PackageManager.PERMISSION_GRANTED &&
                   writext==PackageManager.PERMISSION_GRANTED    ){

                    Log.d(LOGNAME, "API 29 file RW permission granted");
                    fileRWPerms=1;
                }
            }
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {

      super.onActivityResult(requestCode, resultCode, data);

      Log.d(LOGNAME, "onActivityResult()");

      if(requestCode==5544) {
          fileRWPerms= -1;
          if(Build.VERSION.SDK_INT >= 30) {
              if(Environment.isExternalStorageManager()) {
                  Log.d(LOGNAME, "API 30+ file RW permission granted");
                  fileRWPerms=1;
              }
          }
          return;
      }

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
      EternalService.delay(50);
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

