
package network.object.onexapp;

import java.io.ByteArrayOutputStream;

import android.os.*;
import android.view.*;
import android.view.inputmethod.*;
import android.view.ViewGroup.LayoutParams;
import android.view.KeyEvent;
import android.view.View.*;
import android.widget.*;
import android.text.*;
import android.content.Context;
import android.app.NativeActivity;
import android.hardware.usb.*;
import android.content.*;
import android.app.*;

import com.felhr.usbserial.UsbSerialInterface;
import com.felhr.usbserial.UsbSerialDevice;

/**  NativeActivity wrapper to offer Java API functions not available in JNI land.
  */
public class OnexNativeActivity extends NativeActivity implements KeyEvent.Callback {

    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState); System.out.println("onCreate");
        setUpKeyboardView();
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
        addContentView(kbdView, new ViewGroup.LayoutParams(10, 10));
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

    public static native void onSerialRecv(String b);

    private UsbSerialInterface.UsbReadCallback recvCB = new UsbSerialInterface.UsbReadCallback() {
        ByteArrayOutputStream buff = new ByteArrayOutputStream();
        @Override
        public void onReceivedData(byte[] data) {
          try{
            buff.write(data);
            String chars = buff.toString("UTF-8");
            int x = chars.lastIndexOf('\n');
            if(x == -1) return;
            String newChars = chars.substring(0,x+1);
            buff.reset();
            buff.write(chars.substring(x+1).getBytes());
            onSerialRecv(newChars);
          }catch(Exception e){}
        }
    };

    UsbSerialDevice serialPort = null;

    @Override
    protected void onNewIntent(Intent intent){
      super.onNewIntent(intent);
      if(!"android.hardware.usb.action.USB_DEVICE_ATTACHED".equalsIgnoreCase(intent.getAction())) return;
      UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
      UsbManager usbManager = getSystemService(UsbManager.class);
      final UsbDeviceConnection connection = usbManager.openDevice(device);
      UsbInterface interf = device.getInterface(0);
      connection.claimInterface(interf, true);
      serialPort = UsbSerialDevice.createUsbSerialDevice(device, connection);
      if(serialPort == null) { System.out.println("No serial port!"); return; }
      if(!serialPort.open()) { System.out.println("Could not open serial port!"); return; }
      serialPort.setBaudRate(9600);
      serialPort.setDataBits(UsbSerialInterface.DATA_BITS_8);
      serialPort.setStopBits(UsbSerialInterface.STOP_BITS_1);
      serialPort.setParity(UsbSerialInterface.PARITY_NONE);
      serialPort.setFlowControl(UsbSerialInterface.FLOW_CONTROL_OFF);
      serialPort.read(recvCB);
    }

    public void serialSend(String chars)
    {
      if(serialPort!=null) try{ serialPort.write(chars.getBytes("UTF-8")); }catch(Exception e){}
    }

    // -----------------------------------------------------------

    public static native void onAlarmRecv(String uid);

    public void showNotification(String title, String text){
      showNotificationStatic(this, title, text);
    }

    public void setAlarm(long when, String uid){
      Intent intent = new Intent("Onex.Alarm");
      intent.putExtra("UID", uid);
      PendingIntent pendingIntent = PendingIntent.getBroadcast(this, 54321, intent, 0);
      AlarmManager am=(AlarmManager)getSystemService(Context.ALARM_SERVICE);
      am.set(AlarmManager.RTC_WAKEUP, when*1000, pendingIntent);
    }

    static public void alarmReceived(Context context, Intent intent){
      String uid=intent.getStringExtra("UID");
      showNotificationStatic(context, "alarm", uid);
    }

    static public void showNotificationStatic(Context context, String title, String text){
      PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, new Intent(context, OnexNativeActivity.class), PendingIntent.FLAG_UPDATE_CURRENT);
      Notification.Builder notifbuilder = new Notification.Builder(context);
      notifbuilder.setContentTitle(title)
                  .setContentText(text)
                  .setSmallIcon(R.drawable.icon)
                  .setContentIntent(pendingIntent);
      NotificationManager notifMgr = (NotificationManager)context.getSystemService(context.NOTIFICATION_SERVICE);
      notifMgr.notify(12345, notifbuilder.build());
    }

    // -----------------------------------------------------------
}

