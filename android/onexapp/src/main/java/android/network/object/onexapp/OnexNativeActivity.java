
package network.object.onexapp;

import java.io.ByteArrayOutputStream;

import android.os.*;
import android.view.*;
import android.view.inputmethod.InputMethodManager;
import android.content.Context;
import android.app.NativeActivity;
import android.hardware.usb.*;
import android.content.Intent;

import com.felhr.usbserial.UsbSerialInterface;
import com.felhr.usbserial.UsbSerialDevice;

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
    public static native void onKeyRelease(int keyCode);

    @Override
    public boolean dispatchKeyEvent(final KeyEvent event)
    {
      int action = event.getAction();
      if(!(action == KeyEvent.ACTION_UP || action == KeyEvent.ACTION_MULTIPLE)){
        return super.dispatchKeyEvent(event);
      }
      String chars;
      if(action == KeyEvent.ACTION_UP){
        chars = event.isPrintingKey()? ""+((char)event.getUnicodeChar()): " ";
      }
      else{
        chars = event.getCharacters();
      }
      final int keyCode = event.getKeyCode();
      onKeyPress(keyCode, chars);
      final int presstime = (keyCode == 0x43)? 50: 300;
      new Thread(){ public void run(){ try { Thread.sleep(presstime); }catch(Exception e){}; onKeyRelease(keyCode); } }.start();
      return true;
    }

    @Override
    public void onBackPressed(){
        System.out.println("Back button!");
        return;
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
}

