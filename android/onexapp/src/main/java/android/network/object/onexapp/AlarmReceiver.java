package network.object.onexapp;

import android.content.*;
import android.app.*;

public class AlarmReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        OnexNativeActivity.alarmReceived(context, intent);
    }
}
