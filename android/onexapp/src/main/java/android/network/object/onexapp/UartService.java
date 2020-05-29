
/*
 * Copyright (c) 2015, Nordic Semiconductor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package network.object.onexapp;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import java.util.List;
import java.util.ArrayList;
import java.util.UUID;
import java.util.Arrays;
import java.lang.Math;

/**
 * Service for managing connection and data communication with a GATT server hosted on a
 * given Bluetooth LE device.
 */
public class UartService extends Service {
    private final static String LOGNAME = "BLE UART";

    private BluetoothManager bluetoothManager;
    private BluetoothAdapter bluetoothAdapter;
    private String bluetoothDeviceAddress;
    private BluetoothGatt bluetoothGATT;
    private int connectionState = STATE_DISCONNECTED;

    private static final int STATE_DISCONNECTED = 0;
    private static final int STATE_CONNECTING = 1;
    private static final int STATE_CONNECTED = 2;

    public final static String ACTION_GATT_CONNECTED = "com.nordicsemi.nrfUART.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED = "com.nordicsemi.nrfUART.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED = "com.nordicsemi.nrfUART.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_AVAILABLE = "com.nordicsemi.nrfUART.ACTION_DATA_AVAILABLE";
    public final static String ACTION_UART_CONNECTED = "com.nordicsemi.nrfUART.ACTION_UART_CONNECTED";
    public final static String EXTRA_DATA = "com.nordicsemi.nrfUART.EXTRA_DATA";

    public static final UUID TX_POWER_UUID = UUID.fromString("00001804-0000-1000-8000-00805f9b34fb");
    public static final UUID TX_POWER_LEVEL_UUID = UUID.fromString("00002a07-0000-1000-8000-00805f9b34fb");
    public static final UUID CCCD = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    public static final UUID FIRMWARE_REVISON_UUID = UUID.fromString("00002a26-0000-1000-8000-00805f9b34fb");
    public static final UUID DIS_UUID = UUID.fromString("0000180a-0000-1000-8000-00805f9b34fb");
    public static final UUID RX_SERVICE_UUID = UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
    public static final UUID RX_CHAR_UUID = UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
    public static final UUID TX_CHAR_UUID = UUID.fromString("6e400003-b5a3-f393-e0a9-e50e24dcca9e");


    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.d(LOGNAME, "onConnectionStateChange: "+status+"/"+newState);
            String intentAction;

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                intentAction = ACTION_GATT_CONNECTED;
                connectionState = STATE_CONNECTED;
                broadcastUpdate(intentAction);
                Log.i(LOGNAME, "Connected to GATT server. Attempting to start service discovery");
                bluetoothGATT.discoverServices();

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                intentAction = ACTION_GATT_DISCONNECTED;
                connectionState = STATE_DISCONNECTED;
                broadcastUpdate(intentAction);
                Log.i(LOGNAME, "Disconnected from GATT server.");
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.w(LOGNAME, "services discovered");
                broadcastUpdate(ACTION_GATT_SERVICES_DISCOVERED);
                enableTXNotification();
            } else {
                Log.w(LOGNAME, "onServicesDiscovered received: " + status);
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt,
                                      BluetoothGattDescriptor descriptor,
                                      int status){
            Log.d(LOGNAME, "onDescriptorWrite callback: "+status+"/"+BluetoothGatt.GATT_SUCCESS);
            byte[] value=descriptor.getValue();
            if(status==BluetoothGatt.GATT_SUCCESS && Arrays.equals(value, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)){
                broadcastUpdate(ACTION_UART_CONNECTED);
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt,
                                          BluetoothGattCharacteristic characteristic,
                                          int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
              writeAChunk();
            }
            else {
              Log.w(LOGNAME, "onCharacteristicWrite "+status+"!=SUCCESS");
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
        }
    };

    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        sendBroadcast(intent);
    }

    private void broadcastUpdate(final String action,
                                 final BluetoothGattCharacteristic characteristic) {
        final Intent intent = new Intent(action);

        if (TX_CHAR_UUID.equals(characteristic.getUuid())) {
            intent.putExtra(EXTRA_DATA, characteristic.getValue());
        }
        sendBroadcast(intent);
    }

    public class LocalBinder extends Binder {
        UartService getService() {
            return UartService.this;
        }
    }

    private final IBinder binder = new LocalBinder();

    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        // After using a given device, you should make sure that BluetoothGatt.close() is called
        // such that resources are cleaned up properly.  In this particular example, close() is
        // invoked when the UI is disconnected from the Service.
        close();
        return super.onUnbind(intent);
    }

    /**
     * Initializes a reference to the local Bluetooth adapter.
     *
     * @return Return true if the initialization is successful.
     */
    public boolean initialize() {
        // For API level 18 and above, get a reference to BluetoothAdapter through
        // BluetoothManager.
        if (bluetoothManager == null) {
            bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (bluetoothManager == null) {
                Log.e(LOGNAME, "Unable to initialize BluetoothManager.");
                return false;
            }
        }

        bluetoothAdapter = bluetoothManager.getAdapter();
        if (bluetoothAdapter == null) {
            Log.e(LOGNAME, "Unable to obtain a BluetoothAdapter.");
            return false;
        }

        return true;
    }

    /**
     * Connects to the GATT server hosted on the Bluetooth LE device.
     *
     * @param address The device address of the destination device.
     *
     * @return Return true if the connection is initiated successfully. The connection result
     *         is reported asynchronously through the
     *         {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     *         callback.
     */
    public boolean connect(final String address) {
        if (bluetoothAdapter == null || address == null) {
            Log.w(LOGNAME, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        if (bluetoothDeviceAddress != null && address.equals(bluetoothDeviceAddress) && bluetoothGATT != null) {
            Log.d(LOGNAME, "Trying to use an existing bluetoothGATT for connection.");
            if (bluetoothGATT.connect()) {
                connectionState = STATE_CONNECTING;
                return true;
            } else {
                return false;
            }
        }

        final BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
        if (device == null) {
            Log.w(LOGNAME, "Device not found.  Unable to connect.");
            return false;
        }
        // We want to directly connect to the device, so we are setting the autoConnect parameter to false.
        bluetoothGATT = device.connectGatt(this, false, gattCallback);
        Log.d(LOGNAME, "Trying to create a new connection.");
        bluetoothDeviceAddress = address;
        connectionState = STATE_CONNECTING;
        return true;
    }

    /**
     * Disconnects an existing connection or cancel a pending connection. The disconnection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public void disconnect() {
        if (bluetoothAdapter == null || bluetoothGATT == null) {
            Log.w(LOGNAME, "BluetoothAdapter not initialized");
            return;
        }
        bluetoothGATT.disconnect();
       // bluetoothGATT.close();
    }

    /**
     * After using a given BLE device, the app must call this method to ensure resources are
     * released properly.
     */
    public void close() {
        if (bluetoothGATT == null) {
            return;
        }
        Log.w(LOGNAME, "bluetoothGATT closed");
        bluetoothDeviceAddress = null;
        bluetoothGATT.close();
        bluetoothGATT = null;
    }

    /**
     * Request a read on a given {@code BluetoothGattCharacteristic}. The read result is reported
     * asynchronously through the {@code BluetoothGattCallback#onCharacteristicRead(android.bluetooth.BluetoothGatt, android.bluetooth.BluetoothGattCharacteristic, int)}
     * callback.
     *
     * @param characteristic The characteristic to read from.
     */
    public void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (bluetoothAdapter == null || bluetoothGATT == null) {
            Log.w(LOGNAME, "BluetoothAdapter not initialized");
            return;
        }
        bluetoothGATT.readCharacteristic(characteristic);
    }

    /**
     * Enables or disables notification on a give characteristic.
     *
    */

    /**
     * Enable Notification on TX characteristic
     *
     * @return
     */
    public void enableTXNotification()
    {
        if (bluetoothGATT == null) {
            Log.e(LOGNAME, "bluetoothGATT null");
            return;
        }
        BluetoothGattService RxService = bluetoothGATT.getService(RX_SERVICE_UUID);
        if (RxService == null) {
            Log.e(LOGNAME, "Rx service not found!");
            return;
        }
        BluetoothGattCharacteristic TxChar = RxService.getCharacteristic(TX_CHAR_UUID);
        if (TxChar == null) {
            Log.e(LOGNAME, "Tx characteristic not found!");
            return;
        }
        boolean ok=bluetoothGATT.setCharacteristicNotification(TxChar,true);
        if(!ok) Log.d(LOGNAME, "setCharacteristicNotification failed");

        BluetoothGattDescriptor descriptor = TxChar.getDescriptor(CCCD);
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        bluetoothGATT.writeDescriptor(descriptor);
    }

    private ArrayList<byte[]> writeChunks=new ArrayList<byte[]>();

    private boolean writesInProgress=false;

    public synchronized void write(byte[] value){
      if(connectionState!=STATE_CONNECTED){
        Log.d(LOGNAME, "write when not connected");
        writesInProgress=false;
        return;
      }
      if(writeChunks.size() >100){
        Log.d(LOGNAME, "Write buffer full!");
        if(!writesInProgress) writeAChunk();
        return;
      }
      int MAX_TX_LEN=20;
      int s=0;
      int e;
      while(s<value.length){
        e=s+Math.min(value.length-s, MAX_TX_LEN);
        byte[] slice = Arrays.copyOfRange(value, s, e);
        writeChunks.add(slice);
        s=e;
      }
      if(!writesInProgress) writeAChunk();
    }

    private synchronized void writeAChunk(){
      if(writeChunks.size()==0){
        writesInProgress=false;
        return;
      }
      byte[] slice = (byte[])writeChunks.get(0);
      boolean ok=writeRXCharacteristic(slice);
      if(ok){
        writeChunks.remove(0);
        writesInProgress=true;
      }
      else{
        Log.d(LOGNAME, "Can't send "+new String(slice));
        writesInProgress=false;
      }
    }

    public boolean writeRXCharacteristic(byte[] value)
    {
        if(connectionState!=STATE_CONNECTED){
            Log.d(LOGNAME, "writeRXCharacteristic when disconnected ");
            return false;
        }
        if (bluetoothGATT == null) {
            Log.e(LOGNAME, "UART GATT not there");
            return false;
        }
        BluetoothGattService RxService = bluetoothGATT.getService(RX_SERVICE_UUID);
        if (RxService == null) {
            Log.e(LOGNAME, "Rx service not found!");
            return false;
        }
        BluetoothGattCharacteristic RxChar = RxService.getCharacteristic(RX_CHAR_UUID);
        if (RxChar == null) {
            Log.e(LOGNAME, "Rx characteristic not found!");
            return false;
        }

        RxChar.setValue(value);
        boolean status = bluetoothGATT.writeCharacteristic(RxChar);

        return status;
    }

    /**
     * Retrieves a list of supported GATT services on the connected device. This should be
     * invoked only after {@code BluetoothGatt#discoverServices()} completes successfully.
     *
     * @return A {@code List} of supported services.
     */
    public List<BluetoothGattService> getSupportedGattServices() {
        if (bluetoothGATT == null) return null;
        return bluetoothGATT.getServices();
    }
}
