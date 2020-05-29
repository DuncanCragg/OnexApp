
package network.object.onexapp;

import java.io.ByteArrayOutputStream;

import android.os.*;
import android.view.inputmethod.*;
import android.view.ViewGroup.LayoutParams;
import android.view.KeyEvent;
import android.widget.*;
import android.text.*;
import android.content.Context;
import android.app.NativeActivity;
import android.hardware.usb.*;
import android.content.*;
import android.util.Log;
import android.app.*;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;

import com.felhr.usbserial.UsbSerialInterface;
import com.felhr.usbserial.UsbSerialDevice;

/**  NativeActivity wrapper to offer Java API functions not available in JNI land.
  */
public class OnexNativeActivity extends NativeActivity implements KeyEvent.Callback {

    static OnexNativeActivity self=null;

    public static final boolean logReadWrite = true;
    public static final String LOGNAME = "OnexApp";

    private static UsbSerialDevice serialPort = null;

    private static final int REQUEST_SELECT_DEVICE = 1;
    private static final int REQUEST_ENABLE_BT = 2;

    private UartService uartService = null;

    private BluetoothAdapter bluetoothAdapter = null;

    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState); System.out.println("onCreate");
        self=this;
        setUpKeyboardView();
        System.loadLibrary("onexapp");
    }

    public static native void setBLEMac(String uid);

    private String blemac=null;

    public void onexInitialised(String blemac){

        Log.i(LOGNAME, "onexInitialised "+ blemac);

        this.blemac = blemac;

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter != null) {

          bindToUARTService();

          triggerBLE();
        }
        else Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_LONG).show();
    }


    private void triggerBLE(){
      if (!bluetoothAdapter.isEnabled()) {
        Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
      }
      else {
        if(blemac==null){
          Intent newIntent = new Intent(this, DeviceListActivity.class);
          startActivityForResult(newIntent, REQUEST_SELECT_DEVICE);
        }
        else useBLEMac();
      }
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
        try {
            unregisterReceiver(UARTStatusChangeReceiver);
        } catch (Exception e) {
            e.printStackTrace();
        }
        unbindService(serviceConnection);
        uartService.stopSelf();
        uartService= null;
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

    private void asyncConnected(){
        new Thread(){ public void run(){ serialOnRecv(null); }}.start();
    }

    public static native void serialOnRecv(String b);

    ByteArrayOutputStream recvBuff = new ByteArrayOutputStream();

    private void dataRecv(byte[] data) {
      try{
        recvBuff.write(data);
        String chars = recvBuff.toString("UTF-8");
        int x = chars.lastIndexOf('\n');
        if(x == -1) return;
        String newChars = chars.substring(0,x+1);
        recvBuff.reset();
        recvBuff.write(chars.substring(x+1).getBytes());
        if(logReadWrite) Log.d(LOGNAME, "read (" + newChars + ")" );
        serialOnRecv(newChars);
      }catch(Exception e){}
    }

    private UsbSerialInterface.UsbReadCallback recvCB = new UsbSerialInterface.UsbReadCallback() {
        @Override
        public void onReceivedData(byte[] data) { dataRecv(data); }
    };

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
      asyncConnected();
    }

    // -----------------------------------------------------------

    private ServiceConnection serviceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder rawBinder) {
            if(uartService !=null){
              Log.d(LOGNAME, "attempt to set uartService on top of existing");
              return;
            }
            uartService = ((UartService.LocalBinder)rawBinder).getService();
            Log.d(LOGNAME, "UART Service= " + uartService);
            if(uartService.initialize()){
              useBLEMac();
            }
            else Log.e(LOGNAME, "Unable to initialize UART service");
        }

        public void onServiceDisconnected(ComponentName classname) {
            uartService = null;
        }
    };

    private void bindToUARTService() {
        Intent bindIntent = new Intent(this, UartService.class);
        bindService(bindIntent, serviceConnection, Context.BIND_AUTO_CREATE);
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(UartService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(UartService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(UartService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(UartService.ACTION_UART_CONNECTED);
        intentFilter.addAction(UartService.ACTION_DATA_AVAILABLE);
        registerReceiver(UARTStatusChangeReceiver, intentFilter);
    }

    private final BroadcastReceiver UARTStatusChangeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (action.equals(UartService.ACTION_UART_CONNECTED)) {
                Log.d(LOGNAME, "UART connected");
                asyncConnected();
            }

            if (action.equals(UartService.ACTION_DATA_AVAILABLE)) {
                try {
                    byte[] data = intent.getByteArrayExtra(UartService.EXTRA_DATA);
                    dataRecv(data);
                } catch (Exception e) {
                    Log.e(LOGNAME, e.toString());
                }
            }

            if (action.equals(UartService.ACTION_GATT_DISCONNECTED)) {
                Log.d(LOGNAME, "GATT disconnected");
                recvBuff.reset();
                triggerBLE();
            }
        }
    };

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
      switch (requestCode) {

        case REQUEST_ENABLE_BT:
            if (resultCode == Activity.RESULT_OK) {
                Toast.makeText(this, "Bluetooth enabled", Toast.LENGTH_SHORT).show();
                Intent newIntent = new Intent(this, DeviceListActivity.class);
                startActivityForResult(newIntent, REQUEST_SELECT_DEVICE);
            } else {
                Log.d(LOGNAME, "BT not enabled");
                Toast.makeText(this, "Bluetooth not enabled", Toast.LENGTH_SHORT).show();
            }
            break;
        case REQUEST_SELECT_DEVICE:
            if (resultCode == Activity.RESULT_OK && data != null) {
                blemac = data.getStringExtra(BluetoothDevice.EXTRA_DEVICE);
                setBLEMac(blemac);
                useBLEMac();
            }
            break;
        default:
            Log.e(LOGNAME, "wrong request code");
            break;
      }
    }

    private void useBLEMac(){
      if(uartService==null || blemac==null) return;
      if(!uartService.connect(blemac)) Log.d(LOGNAME, "uartService.connect failed");
    }

    // -----------------------------------------------------------

    public void serialSend(String chars){
      if(logReadWrite) Log.d(LOGNAME, "write (" + chars + ")");
      try {
        if (uartService!=null){
          uartService.write(chars.getBytes("UTF-8"));
        }
        if(serialPort!=null){
          serialPort.write(chars.getBytes("UTF-8"));
        }
      }catch(Exception e){
        e.printStackTrace();
      }
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

