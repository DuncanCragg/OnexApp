// THANKS to https://fabcirablog.weebly.com/blog/creating-a-never-ending-background-service-in-android-gt-7

package network.object.onexapp;

import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class EternalService extends android.app.Service {
    private static String LOGNAME = "EternalService";

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(LOGNAME, "*onBind");
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);

        Log.d(LOGNAME, "*onStartCommand");

        // this may be called many times

        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(LOGNAME, "*onDestroy");
        sendBroadcast(new Intent("network.object.onexapp.eternal.restart"));
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        super.onTaskRemoved(rootIntent);
        Log.d(LOGNAME, "*onTaskRemoved");
        sendBroadcast(new Intent("network.object.onexapp.eternal.restart"));
    }
}
