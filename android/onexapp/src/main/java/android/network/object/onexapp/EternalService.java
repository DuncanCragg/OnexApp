// THANKS to https://fabcirablog.weebly.com/blog/creating-a-never-ending-background-service-in-android-gt-7

package network.object.onexapp;

import java.util.List;
import java.io.ByteArrayOutputStream;

import android.os.*;
import android.view.inputmethod.*;
import android.view.ViewGroup.LayoutParams;
import android.view.KeyEvent;
import android.widget.*;
import android.text.*;
import android.app.NativeActivity;
import android.hardware.usb.*;
import android.content.*;
import android.content.pm.*;
import android.util.Log;
import android.app.*;

import com.felhr.usbserial.UsbSerialInterface;
import com.felhr.usbserial.UsbSerialDevice;

public class EternalService extends Service {
    private static String LOGNAME = "Onex EternalService";
    public static final boolean logReadWrite = true;

    static private EternalService self=null;

    static public void delay(int ms){ try{ Thread.sleep(ms); }catch(Exception e){}; }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(LOGNAME, "*onBind");
        return null;
    }

    static private boolean initialised=false;

    static private boolean connecting=false;
    static private boolean reconnecting=false;

    static private String blemac=null;

    static public native String initOnex();
    static public native void   loopOnex();
    static public native void   setBLEMac(String blemac);
    static public native void   connectionState(String state);

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(LOGNAME, "*onCreate");

        String CHANNEL_ID = "onex-activity";

        NotificationChannel channel = new NotificationChannel(CHANNEL_ID, "Onex Activity", NotificationManager.IMPORTANCE_DEFAULT);
        ((NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE)).createNotificationChannel(channel);

        Intent intent = new Intent(this, OnexNativeActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_IMMUTABLE);

        Notification.Builder notifbuilder = new Notification.Builder(this, CHANNEL_ID);
        notifbuilder.setSmallIcon(R.drawable.icon)
                    .setContentTitle("Onex is running")
                    .setContentText("(long press for notif settings)")
                    .setContentIntent(pendingIntent)
                    .setAutoCancel(true);
        startForeground(54321, notifbuilder.build());
    }

    // onStartCommand may be called many times
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        Log.d(LOGNAME, "*onStartCommand");

        self=this;

        if(!initialised){

          System.loadLibrary("native-lib");

          new Thread(){
             @Override
             public void run(){
               Log.d(LOGNAME, "============== calling initOnex()");
               blemac=initOnex();
               Log.d(LOGNAME, "============== initOnex() returned "+blemac);

               bindToNUSService();

               Log.d(LOGNAME, "============== calling loopOnex()");
               loopOnex();
             }
          }.start();
          initialised=true;
        }
        return START_STICKY;
    }

    private NUSService nusService = null;

    private void bindToNUSService() {
        Log.d(LOGNAME, "bindToNUSService()");

        Intent bindIntent = new Intent(this, NUSService.class);
        bindService(bindIntent, serviceConnection, Context.BIND_AUTO_CREATE);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(NUSService.ACTION_GATT_CONNECTING);
        intentFilter.addAction(NUSService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(NUSService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(NUSService.ACTION_NUS_CONNECTED);
        intentFilter.addAction(NUSService.ACTION_DATA_AVAILABLE);
        intentFilter.addAction(NUSService.ACTION_GATT_DISCONNECTED);
        registerReceiver(NUSStatusChangeReceiver, intentFilter);
    }

    private ServiceConnection serviceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder rawBinder) {
            if(nusService !=null){
              Log.d(LOGNAME, "onServiceConnected(): attempt to set nusService on top of existing");
              return;
            }

            nusService = ((NUSService.LocalBinder)rawBinder).getService();

            if(!nusService.initialize()){
              Log.e(LOGNAME, "onServiceConnected(): Unable to initialize NUS service");
              return;
            }
            Log.d(LOGNAME, "onServiceConnected(): OK");

            connectBLEMac();
        }

        public void onServiceDisconnected(ComponentName classname) {
            Log.d(LOGNAME, "onServiceDisconnected()");
            connecting=false;
            reconnecting=false;
            nusService = null;
            recvBuff.reset();
        }
    };

    static public void connectBLEMac(boolean success){
      if(success) self.connectBLEMac();
      else connectionState("BLE disconnected");
    }

    private void connectBLEMac(){
      Log.d(LOGNAME, "connectBLEMac["+nusService+", "+blemac+"]");
      if(nusService==null || blemac==null) return;
      if(!nusService.connect(blemac)) Log.d(LOGNAME, "nusService.connect failed");
    }

    static public void ensureBluetoothConnecting(){
      Log.d(LOGNAME, "ensureBluetoothConnecting() connecting="+connecting+" reconnecting="+reconnecting);
      if(reconnecting) return;
      reconnecting=true;
      blemac=null; setBLEMac("");
      if(self.nusService.disconnect()){
        Log.d(LOGNAME, "ensureBluetoothConnecting(): disconnecting..");
        if(connecting){
          delay(10);
          Log.d(LOGNAME, "ensureBluetoothConnecting(): already reconnecting; closing");
          self.nusService.close();
          connecting=false;
          connectionState("BLE selection");
          OnexNativeActivity.selectBLEMac();
        }
      }else{
        Log.d(LOGNAME, "ensureBluetoothConnecting(): not yet connected");
        connectionState("BLE selection");
        OnexNativeActivity.selectBLEMac();
      }
    }

    public static void onBLEMacSelected(String bm){
      Log.d(LOGNAME, "onBLEMacSelected("+bm+")");
      if(bm.equals("00:00:00:00:00:00")){
        connecting=false;
        reconnecting=false;
        blemac=null; setBLEMac("");
        connectionState("BLE disconnected");
        return;
      }
      blemac=bm; setBLEMac(blemac);
      connectBLEMac(true);
    }

    static public void bluetoothOff(){
      Log.d(LOGNAME, "BT off");
      if(self.nusService.disconnect()){
        delay(10);
        self.nusService.close();
      }
      connecting=false;
      reconnecting=false;
      connectionState("BLE disconnected");
    }

    static public void bluetoothOn(){
      Log.d(LOGNAME, "BT on");
      if(blemac!=null){
        connectionState("BLE rescanning");
        self.nusService.scanForBLEMAC(blemac);
      }
    }

    private final BroadcastReceiver NUSStatusChangeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (action.equals(NUSService.ACTION_GATT_CONNECTING)) {
                Log.d(LOGNAME, "GATT connecting");
                connecting=true;
                connectionState("BLE connecting");
            }

            if (action.equals(NUSService.ACTION_GATT_CONNECTED)) {
                Log.d(LOGNAME, "GATT connected");
                connecting=false;
                reconnecting=false;
                connectionState("BLE connected");
            }

            if (action.equals(NUSService.ACTION_GATT_SERVICES_DISCOVERED)) {
                Log.d(LOGNAME, "GATT connected");
                connecting=false;
                reconnecting=false;
                connectionState("services connected");
            }

            if (action.equals(NUSService.ACTION_NUS_CONNECTED)) {
                Log.d(LOGNAME, "NUS connected");
                connecting=false;
                reconnecting=false;
                connectionState("Onex connected");
                asyncConnected();
            }

            if (action.equals(NUSService.ACTION_DATA_AVAILABLE)) {
                try {
                    byte[] data = intent.getByteArrayExtra(NUSService.EXTRA_DATA);
                    dataRecv(data);
                } catch (Exception e) {
                    Log.e(LOGNAME, e.toString());
                }
            }

            if (action.equals(NUSService.ACTION_GATT_DISCONNECTED)) {
                Log.d(LOGNAME, "GATT disconnected");
                recvBuff.reset();
                if(reconnecting) self.nusService.close();
                if(blemac==null){
                  connectionState("BLE selection");
                  OnexNativeActivity.selectBLEMac();
                }
                else connectBLEMac();
            }
        }
    };

    // -----------------------------------------------------------

    private static UsbSerialDevice serialPort = null;

    static public void onUSBAttached(Intent intent){
      UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
      UsbManager usbManager = self.getSystemService(UsbManager.class);
      final UsbDeviceConnection connection = usbManager.openDevice(device);
      UsbInterface interf = device.getInterface(0);
      connection.claimInterface(interf, true);
      serialPort = UsbSerialDevice.createUsbSerialDevice(device, connection);
      if(serialPort == null) { Log.e(LOGNAME, "No serial port!"); return; }
      if(!serialPort.open()) { Log.e(LOGNAME, "Could not open serial port!"); return; }
      serialPort.setBaudRate(9600);
      serialPort.setDataBits(UsbSerialInterface.DATA_BITS_8);
      serialPort.setStopBits(UsbSerialInterface.STOP_BITS_1);
      serialPort.setParity(UsbSerialInterface.PARITY_NONE);
      serialPort.setFlowControl(UsbSerialInterface.FLOW_CONTROL_OFF);
      serialPort.read(recvCB);
      asyncConnected();
    }

    static public UsbSerialInterface.UsbReadCallback recvCB = new UsbSerialInterface.UsbReadCallback() {
        @Override
        public void onReceivedData(byte[] data) { dataRecv(data); }
    };

    // -----------------------------------------------------------

    static public native void serialOnRecv(String b);

    static public void asyncConnected(){
        new Thread(){ public void run(){ serialOnRecv(null); }}.start();
    }

    static private ByteArrayOutputStream recvBuff = new ByteArrayOutputStream();

    static private void dataRecv(byte[] data) {
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

    static public void serialSend(String chars){
      if(logReadWrite) Log.d(LOGNAME, "write (" + chars + ")");
      try {
        if (self.nusService!=null){
          self.nusService.write(chars.getBytes("UTF-8"));
        }
        if(self.serialPort!=null){
          self.serialPort.write(chars.getBytes("UTF-8"));
        }
      }catch(Exception e){
        e.printStackTrace();
      }
    }

    // -----------------------------------------------------------

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(LOGNAME, "*onDestroy");

        Intent intent = new Intent("network.object.onexapp.eternal.restart").setFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
        PackageManager packageManager = getPackageManager();
        List<ResolveInfo> broadcastReceivers = packageManager.queryBroadcastReceivers(intent, 0);
        for(ResolveInfo broadcastReceiver: broadcastReceivers) {
            ComponentName cn = new ComponentName(broadcastReceiver.activityInfo.packageName, broadcastReceiver.activityInfo.name);
            intent.setComponent(cn);
            sendBroadcast(intent);
        }
        try {
            unregisterReceiver(NUSStatusChangeReceiver);
        } catch (Exception e) {
            e.printStackTrace();
        }
        unbindService(serviceConnection);
        nusService.stopSelf();
        nusService= null;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        super.onTaskRemoved(rootIntent);
        Log.d(LOGNAME, "*onTaskRemoved");

        Intent intent = new Intent("network.object.onexapp.eternal.restart").setFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
        PackageManager packageManager = getPackageManager();
        List<ResolveInfo> broadcastReceivers = packageManager.queryBroadcastReceivers(intent, 0);
        for(ResolveInfo broadcastReceiver: broadcastReceivers) {
            ComponentName cn = new ComponentName(broadcastReceiver.activityInfo.packageName, broadcastReceiver.activityInfo.name);
            intent.setComponent(cn);
            sendBroadcast(intent);
        }
    }
}
