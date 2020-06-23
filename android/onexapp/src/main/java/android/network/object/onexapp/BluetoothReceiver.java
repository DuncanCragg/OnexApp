package network.object.onexapp;

import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.bluetooth.BluetoothAdapter;

public class BluetoothReceiver extends BroadcastReceiver {
    private static String LOGNAME="Onex BluetoothReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if(!BluetoothAdapter.ACTION_STATE_CHANGED.equals(intent.getAction())) return;
        if(intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1)==BluetoothAdapter.STATE_OFF){
          EternalService.bluetoothOff();
        }
        else
        if(intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1)==BluetoothAdapter.STATE_ON){
          EternalService.bluetoothOn();
        }
    }
}

